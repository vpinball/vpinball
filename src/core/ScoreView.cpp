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

string TrimLeading(const string& str, const std::string& whitespace)
{
   if (str.empty())
      return str;
   const auto strBegin = str.find_first_not_of(whitespace);
   if (strBegin == std::string::npos)
      return "";
   return string(str.cbegin() + strBegin, str.cend());
}

string TrimTrailing(const string& str, const std::string& whitespace)
{
   if (str.empty())
      return str;
   const auto strBegin = str.cbegin();
   const auto pos = str.find_last_not_of(whitespace);
   if (pos == string::npos)
      return "";
   const auto strEnd = strBegin + pos + 1;
   return string(strBegin, strEnd);
}

void ScoreView::Parse(const string& path, std::istream& f)
{
   #define CHECK_FIELD(check) if (!(check)) { PLOGE << "Invalid field '" << key << ": " << value << "' at line " << lineIndex << " in ScoreView file " << path; return; }
   const string whitespace = " \t";
   Layout layout { "" };
   layout.path = path;
   layout.width = 1920.f;
   layout.height = 1080.f;
   std::string line;
   size_t indentSize = 0;
   unsigned int lineIndex = 0;
   int expectedIndent = 0;
   float fValue;
   Visual* visual = nullptr;
   const auto parseArray = [](const string& value, int size) -> vector<float>
   {
      vector<float> array(size);
      array.resize(size);
      int pos1 = value.find('[');
      if (pos1 == string::npos)
         return vector<float>();
      for (int i = 0; i < size; i++)
      {
         int pos2 = value.find(i == size -1 ? ']' : ',', pos1 + 1);
         if (pos2 == string::npos)
            return vector<float>();
         if (!try_parse_float(value.substr(pos1 + 1, pos2 - pos1 - 1), array[i]))
            return vector<float>();
         pos1 = pos2;
      }
      return array;
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
         // visual = nullptr;
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
      if (key == "Visuals")
      {
         CHECK_FIELD(indent == 0);
         expectedIndent = 1;
      }
      else if (key == "- DMD")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.resize(layout.visuals.size() + 1);
         visual = &layout.visuals.back();
         visual->type = VisualType::DMD;
         visual->srcUri = "default://dmd";
         visual->liveStyle = 1; // Default to Neon Plasma
         visual->tint = vec3(1.f, 1.f, 1.f);
         visual->glassAmbient = vec3(1.f, 1.f, 1.f);
      }
      else if (key == "- SegDisplay")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.resize(layout.visuals.size() + 1);
         visual = &layout.visuals.back();
         visual->type = VisualType::SegDisplay;
         visual->srcUri = "";
         visual->liveStyle = 1;
         visual->tint = vec3(1.f, 1.f, 1.f);
         visual->glassAmbient = vec3(1.f, 1.f, 1.f);
      }
      else if (key == "- Image")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.resize(layout.visuals.size() + 1);
         visual = &layout.visuals.back();
         visual->type = VisualType::Image;
      }
      else if (key == "Rect")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         auto rect = parseArray(value, 4);
         CHECK_FIELD(rect.size() == 4);
         visual->x = rect[0];
         visual->y = rect[1];
         visual->w = rect[2];
         visual->h = rect[3];
         visual->y = layout.height - visual->y - visual->h;
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
         }
         else if (visual->type == VisualType::SegDisplay)
         {
            if (value == "Neon Plasma")
               visual->liveStyle = 0;
            if (value == "Blue VFD")
               visual->liveStyle = 1;
            if (value == "Green VFD")
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
         }
      }
      else if (key == "Src")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         visual->srcUri = value;
      }
      else if (key == "Glass")
      {
         CHECK_FIELD((indent == 2) && (visual != nullptr));
         expectedIndent = 3;
         visual->style = value;
      }
      else if (key == "Pad")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass pad
         auto rect = parseArray(value, 4);
         CHECK_FIELD(rect.size() == 4);
         visual->glassPadT = clamp(rect[0] / layout.height, 0.f, 1.f);
         visual->glassPadL = clamp(rect[1] / layout.width, 0.f, 1.f);
         visual->glassPadB = clamp(rect[2] / layout.height, 0.f, 1.f);
         visual->glassPadR = clamp(rect[3] / layout.width, 0.f, 1.f);
      }
      else if (key == "Image")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass image
         visual->glassPath = value;
      }
      else if (key == "Ambient")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr)); // Glass ambient
         auto rect = parseArray(value, 3);
         CHECK_FIELD(rect.size() == 3)
         visual->glassAmbient.x = rect[0] / 255.f;
         visual->glassAmbient.y = rect[1] / 255.f;
         visual->glassAmbient.z = rect[2] / 255.f;
      }
      else if (key == "Roughness")
      {
         CHECK_FIELD((indent == 3) && (visual != nullptr) && try_parse_float(value, fValue)); // Glass roughness
         visual->glassRoughness = fValue;
      }
      //PLOGD << indent << ": " << key << " => " << value;
   }
   // TODO avoid duplicates
   m_layouts.push_back(layout);
   #undef CHECK_FIELD
}

void ScoreView::Select()
{
   m_bestLayout = nullptr;
   Player* player = g_pplayer;
   if (player == nullptr)
      return;
   if (m_layouts.empty())
      return;

   // Evaluate layouts against current context
   for (auto& layout : m_layouts)
   {
      layout.matchedVisuals = 0;
      layout.unmatchedVisuals = 0;
      for (auto& visual : layout.visuals)
      {
         switch (visual.type)
         {
         case VisualType::DMD:
            if (player->m_resURIResolver.GetDisplay(visual.srcUri, nullptr) == nullptr)
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals++;
            break;
         case VisualType::SegDisplay:
            if (player->m_resURIResolver.GetSegDisplay(visual.srcUri, nullptr).frame == nullptr)
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
   // - with displays size matching visuals (16 alphanum vs 20 alphanum, ...)
   // - with the best matching layout aspect ratio
   m_bestLayout = &*std::max_element(m_layouts.begin(), m_layouts.end(),
      [](const Layout& a, const Layout& b) -> bool
      {
         if (a.unmatchedVisuals != b.unmatchedVisuals)
            return a.unmatchedVisuals > b.unmatchedVisuals;
         if (a.matchedVisuals != b.matchedVisuals)
            return a.matchedVisuals < b.matchedVisuals;
         // TODO implement display count matching
         // TODO implement aspect ratio matching
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
            tex->LoadFromFile(fullPath.string().c_str());
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
         renderer->m_renderDevice->SetRenderTarget("DMDView"s, scoreRT, true, true);
         if (scoreRT != sceneRT)
            renderer->m_renderDevice->AddRenderTargetDependency(sceneRT, false);
         renderer->SetupDMDRender(visual.liveStyle, true, visual.tint, 1.f, frame, 1.f, Renderer::ColorSpace::Reinhard_sRGB, nullptr,
            visual.glass, visual.glassAmbient, visual.glassRoughness, visual.glassPadL, visual.glassPadR, visual.glassPadT, visual.glassPadB);
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
         if (frame.frame == nullptr)
            continue;
         LoadGlass(visual);
         renderer->m_renderDevice->ResetRenderState();
         renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
         renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         renderer->m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         renderer->m_renderDevice->SetRenderTarget("DMDView"s, scoreRT, true, true);
         if (scoreRT != sceneRT)
            renderer->m_renderDevice->AddRenderTargetDependency(sceneRT, false);
         float div = 1.f / frame.displays.size();
         for (int i = 0; i < frame.displays.size(); i++)
         {
            renderer->SetupAlphaSegRender(visual.liveStyle, true, visual.tint, 1.0f, frame.displays[i], &frame.frame[i * 16], 1.f, Renderer::ColorSpace::Reinhard_sRGB, nullptr, 
               visual.glass, visual.glassAmbient, visual.glassRoughness, visual.glassPadL, visual.glassPadR, visual.glassPadT, visual.glassPadB);
            const float vx1 = px + visual.x * sx + i * visual.w * div * sx;
            const float vy1 = py + visual.y * sy;
            const float vx2 = vx1 + visual.w * div * sx;
            const float vy2 = vy1 + visual.h * sy;
            const Vertex3D_NoTex2 vertices[4] = { 
               { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, 
               { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, 
               { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
               { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
            renderer->m_renderDevice->DrawTexturedQuad(renderer->m_renderDevice->m_DMDShader, vertices);
         }
         break;
      }

      case VisualType::Image: break;
      }
   }
}