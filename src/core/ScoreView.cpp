// license:GPLv3+

#include "core/stdafx.h"
#include "ScoreView.h"
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <iostream>

ScoreView::ScoreView()
{
}

ScoreView::~ScoreView()
{
   for (auto& image : m_images)
      delete image.second;
   m_images.clear();
}

void ScoreView::Load(const string& path)
{
   if (!std::filesystem::exists(path))
      return;
   std::error_code ec;
   if (std::filesystem::is_directory(path, ec))
   {
      for (const auto& entry : std::filesystem::directory_iterator(path))
      {
         string ext = entry.path().extension().string();
         if (!entry.is_directory() && entry.path().extension().string() == ".scv")
         {
            std::ifstream ifs(entry.path());
            Parse(entry.path().string(), ifs);
         }
      }
   }
   else
   {
      std::ifstream ifs(path);
      Parse(path, ifs);
   }
}

static string TrimLeading(const string& str, const std::string& whitespace)
{
   if (str.empty())
      return str;
   const auto strBegin = str.find_first_not_of(whitespace);
   if (strBegin == std::string::npos)
      return string();
   return string(str.cbegin() + strBegin, str.cend());
}

static string TrimTrailing(const string& str, const std::string& whitespace)
{
   if (str.empty())
      return str;
   const auto strBegin = str.cbegin();
   const auto pos = str.find_last_not_of(whitespace);
   if (pos == string::npos)
      return string();
   const auto strEnd = strBegin + pos + 1;
   return string(strBegin, strEnd);
}

void ScoreView::Parse(const string& path, std::istream& f)
{
   #define CHECK_FIELD(check) if (!(check)) { PLOGE << "Invalid field '" << key << ": " << value << "' at line " << lineIndex << " in ScoreView file " << path; return; }
   const string whitespace = " \t"s;
   Layout layout { string() };
   layout.path = path;
   layout.width = 1920.f;
   layout.height = 1080.f;
   layout.fit = ScoreView::Contain;
   std::string line;
   size_t indentSize = 0;
   unsigned int lineIndex = 0;
   size_t expectedIndent = 0;
   float fValue;
   Visual* visual = nullptr;
   const auto parseArray = [](const string& value) -> vector<float>
   {
      vector<float> array;
      size_t pos1 = value.find('[');
      if (pos1 == string::npos)
         return vector<float>();
      while (true)
      {
         size_t pos2 = value.find_first_of("],\n", pos1 + 1);
         if ((pos2 == string::npos) || (value[pos2] == '\n'))
            return vector<float>();
         float v;
         if (!try_parse_float(value.substr(pos1 + 1, pos2 - pos1 - 1), v))
            return vector<float>();
         array.push_back(v);
         if (value[pos2] == ']')
            return array;
         pos1 = pos2;
      }
   };
   while (std::getline(f, line))
   {
      lineIndex++;
      line = TrimTrailing(line, whitespace);
      if (line.empty())
         continue;
      const auto afterIndent = line.find_first_not_of(whitespace);
      if ((indentSize == 0) && (afterIndent != 0))
         indentSize = afterIndent;
      if ((indentSize != 0) && ((afterIndent % indentSize) != 0))
      {
         PLOGE << "Invalid indentation at line " << lineIndex << " in ScoreView file " << path;
         return;
      }
      size_t indent = indentSize == 0 ? 0 : afterIndent / indentSize;
      if (indent > expectedIndent)
      {
         PLOGE << "Invalid indentation (" << indent << " while expecting " << expectedIndent << " at line " << lineIndex << " in ScoreView file " << path;
         return;
      }
      if (indent < expectedIndent)
      {
         if (indent <= 1)
            visual = nullptr;
         expectedIndent = indent;
      }
      if (line[afterIndent] == '#')
         continue;
      const auto colon = line.find(':');
      if (colon == string::npos)
      {
         PLOGE << "Field is missing ':' separator at line " << lineIndex << " in ScoreView file " << path;
         return;
      }
      if (colon == afterIndent)
      {
         PLOGE << "Field is missing a key before ':' separator at line " << lineIndex << " in ScoreView file " << path;
         return;
      }
      const string key(line.cbegin() + afterIndent, line.cbegin() + colon);
      string value(line.cbegin() + colon + 1, line.cend());
      value = TrimLeading(value, whitespace);

      if (key == "Width")
      {
         CHECK_FIELD((indent == 0) && try_parse_float(value, fValue) && (fValue >= 1.f));
         layout.width = fValue;
      }
      else if (key == "Height")
      {
         CHECK_FIELD((indent == 0) && try_parse_float(value, fValue) && (fValue >= 1.f));
         layout.height = fValue;
      }
      else if (key == "Fit")
      {
         CHECK_FIELD(indent == 0);
         if (value == "Contain")
            layout.fit = ScoreView::Contain;
         else
         {
            // Unsupported fit mode
            CHECK_FIELD(false);
         }
      }
      if (key == "Visuals")
      {
         CHECK_FIELD(indent == 0);
         expectedIndent = 1;
      }
      else if (key == "- DMD")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.push_back({VisualType::DMD});
         visual = &layout.visuals.back();
         visual->srcUri = "default://dmd";
         visual->liveStyle = 1; // Default to Neon Plasma
         visual->tint = vec3(1.f, 1.f, 1.f);
         visual->glassTint = vec3(1.f, 1.f, 1.f);
         visual->glassAmbient = vec3(1.f, 1.f, 1.f);
         visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
         visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
         visual->dmdSize = int2(-1, -1);
      }
      else if (key == "- SegDisplay")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.push_back({VisualType::SegDisplay});
         visual = &layout.visuals.back();
         visual->srcUri = "";
         visual->liveStyle = 1;
         visual->tint = vec3(1.f, 1.f, 1.f);
         visual->glassTint = vec3(1.f, 1.f, 1.f);
         visual->glassAmbient = vec3(1.f, 1.f, 1.f);
         visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
         visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
         visual->nElements = -1;
         visual->segFamilyHint = Renderer::Generic;
      }
      else if (key == "- Image")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.push_back({VisualType::Image});
         visual = &layout.visuals.back();
         // Not yet supported
         CHECK_FIELD(false);
      }
      else if (key == "Rect")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         auto rect = parseArray(value);
         CHECK_FIELD(rect.size() == 4);
         visual->x = rect[0];
         visual->y = rect[1];
         visual->w = rect[2];
         visual->h = rect[3];
         visual->y = layout.height - visual->y - visual->h;
      }
      else if (key == "Pad")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr)); // Display pad inside rect
         auto rect = parseArray(value);
         CHECK_FIELD(rect.size() == 4);
         visual->glassPad.x = clamp(rect[0] / layout.height, 0.f, 1.f);
         visual->glassPad.y = clamp(rect[1] / layout.width, 0.f, 1.f);
         visual->glassPad.z = clamp(rect[2] / layout.height, 0.f, 1.f);
         visual->glassPad.w = clamp(rect[3] / layout.width, 0.f, 1.f);
         // Not yet supported
         CHECK_FIELD((visual->glassPad.x == 0.f) && (visual->glassPad.y == 0.f) && (visual->glassPad.z == 0.f) && (visual->glassPad.w == 0.f));
      }
      else if (key == "Style")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         if (visual->type == VisualType::DMD)
         {
            if (value == "Neon Plasma")
               visual->liveStyle = 1;
            else if (value == "Red LED")
               visual->liveStyle = 2;
            else if (value == "Green LED")
               visual->liveStyle = 3;
            else if (value == "Yellow LED")
               visual->liveStyle = 4;
            else if (value == "Generic Plasma")
               visual->liveStyle = 5;
            else if (value == "Generic LED")
               visual->liveStyle = 6;
            else
            {
               // Invalid style
               CHECK_FIELD(false);
            }
         }
         else if (visual->type == VisualType::SegDisplay)
         {
            if (value == "Neon Plasma")
               visual->liveStyle = 0;
            else if (value == "Blue VFD")
               visual->liveStyle = 1;
            else if (value == "Green VFD")
               visual->liveStyle = 2;
            else if (value == "Red LED")
               visual->liveStyle = 3;
            else if (value == "Green LED")
               visual->liveStyle = 4;
            else if (value == "Yellow LED")
               visual->liveStyle = 5;
            else if (value == "Generic Plasma")
               visual->liveStyle = 6;
            else if (value == "Generic LED")
               visual->liveStyle = 7;
            else
            {
               // Invalid style
               CHECK_FIELD(false);
            }
         }
      }
      else if (key == "Src")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         visual->srcUri = value;
      }
      else if (key == "Size")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr) && (visual->type == VisualType::DMD)); // DMD size
         const auto size = parseArray(value);
         CHECK_FIELD(size.size() == 2);
         visual->dmdSize.x = static_cast<int>(size[0]);
         visual->dmdSize.y = static_cast<int>(size[1]);
      }
      else if (key == "Type")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr) && (visual->type == VisualType::SegDisplay)); // Seg display hardware family hint
         if (value == "Atari")
            visual->segFamilyHint = Renderer::Atari;
         else if (value == "Bally")
            visual->segFamilyHint = Renderer::Bally;
         else if (value == "Gottlieb")
            visual->segFamilyHint = Renderer::Gottlieb;
         else if (value == "Williams")
            visual->segFamilyHint = Renderer::Williams;
         else
         {
            // Invalid segment family hint
            CHECK_FIELD(false);
         }
      }
      else if (key == "XPos")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr) && (visual->type == VisualType::SegDisplay)); // Seg display sub element offsets
         visual->xOffsets = parseArray(value);
         CHECK_FIELD(!visual->xOffsets.empty() && (visual->nElements == -1 || visual->xOffsets.size() == visual->nElements));
      }
      else if (key == "NElements")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr) && (visual->type == VisualType::SegDisplay) && try_parse_int(value, visual->nElements)); // Number of seg display elements
         CHECK_FIELD(visual->xOffsets.empty() || visual->xOffsets.size() == visual->nElements);
      }
      else if (key == "Glass")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         expectedIndent = 3;
         visual->style = value;
      }
      else if (key == "Tint")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass tint
         auto tint = parseArray(value);
         CHECK_FIELD(tint.size() == 3);
         visual->glassTint.x = tint[0];
         visual->glassTint.y = tint[1];
         visual->glassTint.z = tint[2];
      }
      else if (key == "Roughness")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr) && try_parse_float(value, fValue)); // Glass roughness
         visual->glassRoughness = fValue;
      }
      else if (key == "Image")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass image path
         visual->glassPath = value;
      }
      else if (key == "Area")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass image area
         auto rect = parseArray(value);
         CHECK_FIELD(rect.size() == 4);
         visual->glassArea.x = rect[0];
         visual->glassArea.y = rect[1];
         visual->glassArea.z = rect[2];
         visual->glassArea.w = rect[3];
      }
      else if (key == "Ambient")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass image ambient
         auto rect = parseArray(value);
         CHECK_FIELD(rect.size() == 3)
         visual->glassAmbient.x = InvsRGB(rect[0]);
         visual->glassAmbient.y = InvsRGB(rect[1]);
         visual->glassAmbient.z = InvsRGB(rect[2]);
      }
      //PLOGD << indent << ": " << key << " => " << value;
   }
   for (auto& visual : layout.visuals)
   {
      switch (visual.type)
      {
      case VisualType::DMD:
         if (visual.dmdSize.x < 0 || visual.dmdSize.y < 0)
         {
            PLOGE << "DMD display needs Size to be defined in ScoreView file " << path;
            return;
         }
         break;
      case VisualType::SegDisplay:
         if (visual.nElements == -1)
            visual.nElements = (int)visual.xOffsets.size();
         if (visual.nElements == 0)
         {
            PLOGE << "Segment display needs at least one of XPos/NElements to be defined in ScoreView file " << path;                                                                      \
            return;
         }
         if (visual.xOffsets.empty())
            for (int i = 0; i < visual.nElements; i++)
               visual.xOffsets.push_back(visual.w * static_cast<float>(i) / static_cast<float>(visual.nElements));
         break;
      }
   }
   // TODO avoid duplicates
   m_layouts.push_back(layout);
   #undef CHECK_FIELD
}

void ScoreView::Select(const VPX::RenderOutput& output)
{
   m_bestLayout = nullptr;
   Player* player = g_pplayer;
   if (player == nullptr)
      return;
   if (m_layouts.empty())
      return;

   // Evaluate output aspect ratio
   int scoreW, scoreH;
   switch (output.GetMode())
   {
   case VPX::RenderOutput::OM_WINDOW:
      scoreW = output.GetWindow()->GetBackBuffer()->GetWidth();
      scoreH = output.GetWindow()->GetBackBuffer()->GetHeight();
      break;
   case VPX::RenderOutput::OM_EMBEDDED:
      scoreW = output.GetEmbeddedWindow()->GetWidth();
      scoreH = output.GetEmbeddedWindow()->GetHeight();
      break;
   default:
      return;
   }
   const float rtAR = static_cast<float>(scoreW) / static_cast<float>(scoreH);

   // Evaluate layouts against current context
   ResURIResolver::SegDisplay segDisplay;
   BaseTexture* dmdFrame;
   for (auto& layout : m_layouts)
   {
      const float layoutAR = static_cast<float>(layout.width) / static_cast<float>(layout.height);
      layout.unfittedPixels = (rtAR > layoutAR) ? (layoutAR / rtAR) : (rtAR / layoutAR);
      layout.matchedVisuals = 0;
      layout.unmatchedVisuals = 0;
      for (auto& visual : layout.visuals)
      {
         switch (visual.type)
         {
         case VisualType::DMD:
            dmdFrame = player->m_resURIResolver.GetDisplay(visual.srcUri, nullptr);
            if ((dmdFrame == nullptr) || (dmdFrame->width() * visual.dmdSize.y != visual.dmdSize.x * dmdFrame->height()))
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals++;
            break;
         case VisualType::SegDisplay:
            segDisplay = player->m_resURIResolver.GetSegDisplay(visual.srcUri, nullptr);
            if ((segDisplay.frame == nullptr) || (segDisplay.displays.size() != visual.nElements))
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals++;
            break;
         case VisualType::Image:
            break;
         }
      }
   }

   // Select the best matching layout:
   // - without missing visual sources
   // - with a maximum of satisfied visuals
   // - with displays size matching visuals (16 alphanum vs 20 alphanum, 128x32,...)
   // - with the best matching layout aspect ratio
   m_bestLayout = &*std::max_element(m_layouts.begin(), m_layouts.end(),
      [](const Layout& a, const Layout& b) -> bool
      {
         if (a.unmatchedVisuals != b.unmatchedVisuals) // The least unmatched visuals
            return a.unmatchedVisuals > b.unmatchedVisuals;
         if (a.matchedVisuals != b.matchedVisuals) // The maximum mactched visual
            return a.matchedVisuals < b.matchedVisuals;
         if (a.unfittedPixels != b.unfittedPixels) // The one fitting the output better
            return a.unfittedPixels < b.unfittedPixels;
         return false;
      });
}

void ScoreView::LoadGlass(Visual& visual)
{
   assert((visual.type == VisualType::DMD) || (visual.type == VisualType::SegDisplay));
   if ((visual.glass == nullptr) && !visual.glassPath.empty())
   {
      auto texImage = m_images.find(visual.glassPath);
      if (texImage != m_images.end())
         visual.glass = texImage->second;
      else
      {
         std::filesystem::path path1 = std::filesystem::path(m_bestLayout->path).remove_filename();
         std::filesystem::path path2 = visual.glassPath;
         std::filesystem::path fullPath = path1 / path2;
         Texture* tex = new Texture();
         if (std::filesystem::exists(fullPath))
         {
            tex->LoadFromFile(fullPath.string());
            tex->m_alphaTestValue = (float)(-1.0 / 255.0); // Disable alpha testing
         }
         visual.glass = tex;
         m_images[visual.glassPath] = visual.glass;
      }
   }
}

void ScoreView::Render(const VPX::RenderOutput& output)
{
   Player* player = g_pplayer;
   if (player == nullptr)
      return;
   if (m_bestLayout == nullptr)
      return;

   // Evaluate output surface
   Renderer* renderer = player->m_renderer;
   RenderTarget* sceneRT = renderer->m_renderDevice->GetCurrentRenderTarget();
   RenderTarget* scoreRT = nullptr;
   int scoreX, scoreY, scoreW, scoreH;
   #ifdef ENABLE_BGFX
   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      output.GetWindow()->Show();
      scoreRT = output.GetWindow()->GetBackBuffer();
      scoreX = scoreY = 0;
      scoreW = scoreRT->GetWidth();
      scoreH = scoreRT->GetHeight();
   }
   else
   #endif
   if (output.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
   {
      output.GetEmbeddedWindow()->GetPos(scoreX, scoreY);
      scoreRT = player->m_playfieldWnd->GetBackBuffer();
      scoreW = output.GetEmbeddedWindow()->GetWidth();
      scoreH = output.GetEmbeddedWindow()->GetHeight();
      scoreY = scoreRT->GetHeight() - scoreY - scoreH;
   }
   if (scoreRT == nullptr)
      return;

   // Render layout
   const float rtAR = static_cast<float>(scoreW) / static_cast<float>(scoreH);
   const float layoutAR = static_cast<float>(m_bestLayout->width) / static_cast<float>(m_bestLayout->height);
   const float pw = 2.f * (rtAR > layoutAR ? layoutAR / rtAR : 1.f) * static_cast<float>(scoreW) / static_cast<float>(scoreRT->GetWidth());
   const float ph = 2.f * (rtAR < layoutAR ? rtAR / layoutAR : 1.f) * static_cast<float>(scoreH) / static_cast<float>(scoreRT->GetHeight());
   const float px = static_cast<float>(scoreX + scoreW / 2) / static_cast<float>(scoreRT->GetWidth()) * 2.f - 1.f - pw * 0.5f;
   const float py = static_cast<float>(scoreY + scoreH / 2) / static_cast<float>(scoreRT->GetHeight()) * 2.f - 1.f - ph * 0.5f;
   const float sx = pw / m_bestLayout->width;
   const float sy = ph / m_bestLayout->height;

   renderer->m_renderDevice->SetRenderTarget("ScoreView"s, scoreRT, true, true);
   if (scoreRT != sceneRT)
   {
      renderer->m_renderDevice->AddRenderTargetDependency(sceneRT, false);
      renderer->m_renderDevice->Clear(clearType::TARGET, 0);
   }
   else
   {
      // FIXME clear rect
   }

   for (auto& visual : m_bestLayout->visuals)
   {
      switch (visual.type)
      {
      case VisualType::DMD:
      {
         BaseTexture* frame = player->m_resURIResolver.GetDisplay(visual.srcUri, nullptr);
         if (frame == nullptr)
            continue;
         LoadGlass(visual);
         renderer->m_renderDevice->ResetRenderState();
         renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
         renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         renderer->m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         vec4 glassArea;
         if (visual.glass == nullptr || visual.glassArea.z == 0.f || visual.glassArea.w == 0.f)
            glassArea = vec4(0.f, 0.f, 1.f, 1.f);
         else
         {
            glassArea.x = visual.glassArea.x / visual.glass->m_width;
            glassArea.y = visual.glassArea.y / visual.glass->m_height;
            glassArea.z = visual.glassArea.z / visual.glass->m_width;
            glassArea.w = visual.glassArea.w / visual.glass->m_height;
         }
         renderer->SetupDMDRender(visual.liveStyle, true, visual.tint, 1.f, frame, 1.f, Renderer::ColorSpace::Reinhard_sRGB, nullptr,
            visual.glassPad, visual.glassTint, visual.glassRoughness, visual.glass, glassArea, visual.glassAmbient);
         const float vx1 = px  + visual.x * sx;
         const float vy1 = py  + visual.y * sy;
         const float vx2 = vx1 + visual.w * sx;
         const float vy2 = vy1 + visual.h * sy;
         const Vertex3D_NoTex2 vertices[4] = { 
            { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, 
            { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, 
            { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
            { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
         renderer->m_renderDevice->DrawTexturedQuad(renderer->m_renderDevice->m_DMDShader, vertices);
         break;
      }

      case VisualType::SegDisplay:
      {
         ResURIResolver::SegDisplay frame = player->m_resURIResolver.GetSegDisplay(visual.srcUri, nullptr);
         if ((frame.frame == nullptr) || (frame.displays.size() != visual.nElements))
            continue;
         LoadGlass(visual);
         renderer->m_renderDevice->ResetRenderState();
         renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
         renderer->m_renderDevice->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
         renderer->m_renderDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
         // We use max blending as segment may overlap in the glass diffuse: we retain the most lighted one which is wrong but looks ok (otherwise we would have to deal with colorspace conversions and layering between glass and emitter)
         renderer->m_renderDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_MAX);
         renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         renderer->m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         // FIXME use hardware segment style
         vec4 glassArea;
         if (visual.glass == nullptr || visual.glassArea.z == 0.f || visual.glassArea.w == 0.f)
            glassArea = vec4(0.f, 0.f, 1.f, 1.f);
         else
         {
            glassArea.x = visual.glassArea.x / visual.glass->m_width;
            glassArea.y = visual.glassArea.y / visual.glass->m_height;
            glassArea.z = visual.glassArea.z / visual.glass->m_width;
            glassArea.w = visual.glassArea.w / visual.glass->m_height;
         }
         const float elementWidth = visual.h * (24.f / 32.f); // Each segment element SDF is 24x32, fitted on visual height
         const float elementXPad = visual.h * (4.f / 32.f); // Each segment element SDF has a 4 pad around the segments for SDF range
         const float hGlassScale = glassArea.z / visual.w;
         vec4 segGlassArea = glassArea;
         segGlassArea.z = elementWidth * hGlassScale;
         const float vy1 = py + sy * visual.y;
         const float vy2 = vy1 + sy * visual.h;
         Vertex3D_NoTex2 vertices[4] = { 
            { 1.f, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
            { 0.f, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
            { 1.f, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
            { 0.f, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
         for (size_t i = 0; i < frame.displays.size(); i++)
         {
            segGlassArea.x = glassArea.x + visual.xOffsets[i] * hGlassScale;
            renderer->SetupSegmentRenderer(visual.liveStyle, true, visual.tint, 1.0f, visual.segFamilyHint, frame.displays[i], &frame.frame[i * 16], 1.f, Renderer::ColorSpace::Reinhard_sRGB, nullptr, 
               visual.glassPad, visual.glassTint, visual.glassRoughness,
               visual.glass, segGlassArea, visual.glassAmbient);
            const float vx1 = px + sx * (visual.x + visual.xOffsets[i]);
            const float vx2 = vx1 + sx * visual.h * (24.f / 32.f);
            vertices[0].x = vertices[2].x = vx2;
            vertices[1].x = vertices[3].x = vx1;
            renderer->m_renderDevice->DrawTexturedQuad(renderer->m_renderDevice->m_DMDShader, vertices);
         }
         break;
      }

      case VisualType::Image: break;
      }
   }
}
