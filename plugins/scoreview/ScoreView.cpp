// license:GPLv3+

#include "ScoreView.h"

#include "plugins/ControllerPlugin.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ScoreView {

static inline float InvsRGB(const float x) { return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f)); }

ScoreView::ScoreView(MsgPluginAPI* api, unsigned int endpointId, VPXPluginAPI* vpxApi)
   : m_resURIResolver(*api, endpointId, true, true, false, false)
   , m_vpxApi(vpxApi)
   , m_msgApi(api)
   , m_endpointId(endpointId)
{
   m_onDmdChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdChangedMsgId, OnResChanged, this);
   m_onSegChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSegChangedMsgId, OnResChanged, this);
}

ScoreView::~ScoreView()
{
   m_msgApi->UnsubscribeMsg(m_onSegChangedMsgId, OnResChanged);
   m_msgApi->ReleaseMsgID(m_onSegChangedMsgId);
   m_msgApi->UnsubscribeMsg(m_onDmdChangedMsgId, OnResChanged);
   m_msgApi->ReleaseMsgID(m_onDmdChangedMsgId);
   for (auto& image : m_images)
      m_vpxApi->DeleteTexture(image.second);
   for (auto& layout : m_layouts)
      for (auto& visual : layout.visuals)
         if (visual.dmdTex)
            m_vpxApi->DeleteTexture(visual.dmdTex);
}

void ScoreView::OnResChanged(const unsigned int msgId, void* userData, void* msgData)
{
   static_cast<ScoreView*>(userData)->m_invalidBestLayout = true;
}

void ScoreView::Load(const string& path)
{
   const std::filesystem::path p(path);
   if (!std::filesystem::exists(p))
      return;
   std::error_code ec;
   if (std::filesystem::is_directory(p, ec))
   {
      for (const auto& entry : std::filesystem::directory_iterator(p))
      {
         if (!entry.is_directory() && entry.path().extension().string() == ".scv")
         {
            std::ifstream ifs(entry.path());
            Parse(entry.path(), ifs);
         }
      }
   }
   else
   {
      std::ifstream ifs(path);
      Parse(p, ifs);
   }
}

void ScoreView::Parse(const std::filesystem::path& path, std::istream& content)
{
   #define CHECK_FIELD(check) if (!(check)) { LOGE("Invalid field '%s: %s' at line %d in ScoreView file %s", key.c_str(), value.c_str(), lineIndex, path.c_str()); return; }
   static const string whitespace = " \t"s;
   Layout layout { string() };
   layout.path = path;
   layout.width = 1920.f;
   layout.height = 1080.f;
   layout.fit = ScoreView::Contain;
   {
   Visual* visual = nullptr;
   const auto parseArray = [](const string& value) -> vector<float>
   {
      size_t pos1 = value.find('[');
      if (pos1 == string::npos)
         return vector<float>();
      vector<float> array;
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
   std::string line;
   size_t indentSize = 0;
   unsigned int lineIndex = 0;
   size_t expectedIndent = 0;
   float fValue;
   while (std::getline(content, line))
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
         LOGE("Invalid indentation at line %d in ScoreView file %s", lineIndex, path.c_str());
         return;
      }
      size_t indent = indentSize == 0 ? 0 : afterIndent / indentSize;
      if (indent > expectedIndent)
      {
         LOGE("Invalid indentation (%d while expecting %d at line %d in ScoreView file %s", indent, expectedIndent, lineIndex, path.c_str());
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
         LOGE("Field is missing ':' separator at line %s in ScoreView file %s", lineIndex, path.c_str());
         return;
      }
      if (colon == afterIndent)
      {
         LOGE("Field is missing a key before ':' separator at line %d in ScoreView file %s", lineIndex, path.c_str());
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
         visual->srcUri = "ctrl://default/display";
         visual->liveStyle = 1; // Default to Neon Plasma
         visual->tint = vec3(1.f, 1.f, 1.f);
         visual->glassTint = vec3(1.f, 1.f, 1.f);
         visual->glassAmbient = vec3(1.f, 1.f, 1.f);
         visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
         visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
         visual->dmdSize = vec2i(-1, -1);
      }
      else if (key == "- SegDisplay")
      {
         CHECK_FIELD(indent == 1);
         expectedIndent = indent + 1;
         layout.visuals.push_back({VisualType::SegDisplay});
         visual = &layout.visuals.back();
         visual->srcUri = string();
         visual->liveStyle = 1;
         visual->tint = vec3(1.f, 1.f, 1.f);
         visual->glassTint = vec3(1.f, 1.f, 1.f);
         visual->glassAmbient = vec3(1.f, 1.f, 1.f);
         visual->glassPad = vec4(0.f, 0.f, 0.f, 0.f);
         visual->glassArea = vec4(0.f, 0.f, 0.f, 0.f);
         visual->nElements = -1;
         visual->segFamilyHint = VPXSegDisplayHint::Generic;
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
            visual->segFamilyHint = VPXSegDisplayHint::Atari;
         else if (value == "Bally")
            visual->segFamilyHint = VPXSegDisplayHint::Bally;
         else if (value == "Gottlieb")
            visual->segFamilyHint = VPXSegDisplayHint::Gottlieb;
         else if (value == "Williams")
            visual->segFamilyHint = VPXSegDisplayHint::Williams;
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
   }
   for (auto& visual : layout.visuals)
   {
      switch (visual.type)
      {
      case VisualType::DMD:
         if (visual.dmdSize.x < 0 || visual.dmdSize.y < 0)
         {
            LOGE("DMD display needs Size to be defined in ScoreView file %s", path.c_str());
            return;
         }
         break;
      case VisualType::SegDisplay:
         if (visual.nElements == -1)
            visual.nElements = (int)visual.xOffsets.size();
         if (visual.nElements == 0)
         {
            LOGE("Segment display needs at least one of XPos/NElements to be defined in ScoreView file %s", path.c_str());
            return;
         }
         if (visual.xOffsets.empty())
         {
            visual.xOffsets.reserve(visual.nElements);
            for (int i = 0; i < visual.nElements; i++)
               visual.xOffsets.push_back(visual.w * static_cast<float>(i) / static_cast<float>(visual.nElements));
         }
         break;
      }
   }
   // TODO avoid duplicates
   m_invalidBestLayout = true;
   m_layouts.push_back(layout);
   #undef CHECK_FIELD
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
         const std::filesystem::path fullPath = m_bestLayout->path.remove_filename() / visual.glassPath;
         std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
         if (file.is_open())
         {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(size);
            file.read(reinterpret_cast<char*>(buffer.data()), size);
            file.close();
            visual.glass = m_vpxApi->CreateTexture(buffer.data(), static_cast<int>(size));
         }
         else
         {
            LOGE("Missing glass file: %s", fullPath.c_str());
            visual.glass = nullptr;
         }
         m_images[visual.glassPath] = visual.glass;
      }
   }
}

void ScoreView::Select(const float scoreW, const float scoreH)
{
   m_bestLayout = nullptr;
   if (m_layouts.empty())
      return;

   // Evaluate output aspect ratio
   const float rtAR = scoreW / scoreH;

   // Evaluate layouts against current context
   ResURIResolver::SegDisplayState segDisplay;
   ResURIResolver::DisplayState display;
   for (auto& layout : m_layouts)
   {
      const float layoutAR = layout.width / layout.height;
      layout.unfittedPixels = (rtAR > layoutAR) ? (layoutAR / rtAR) : (rtAR / layoutAR);
      layout.matchedVisuals = 0;
      layout.unmatchedVisuals = 0;
      for (const auto& visual : layout.visuals)
      {
         switch (visual.type)
         {
         case VisualType::DMD:
            display = m_resURIResolver.GetDisplayState(visual.srcUri);
            if ((display.source == nullptr) || (display.source->width * visual.dmdSize.y != visual.dmdSize.x * display.source->height))
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals += 10; // To favor DMD over alphanumeric seg displays
            break;
         case VisualType::SegDisplay:
            segDisplay = m_resURIResolver.GetSegDisplayState(visual.srcUri);
            if ((segDisplay.source == nullptr) || (segDisplay.source->nElements != visual.nElements))
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
         if (a.matchedVisuals != b.matchedVisuals) // The maximum matched visual
            return a.matchedVisuals < b.matchedVisuals;
         if (a.unfittedPixels != b.unfittedPixels) // The one fitting the output better
            return a.unfittedPixels < b.unfittedPixels;
         return false;
      });

   if (m_bestLayout->unmatchedVisuals > 0)
      m_bestLayout = nullptr;
}

bool ScoreView::Render(VPXRenderContext2D* ctx)
{
   if (m_invalidBestLayout)
   {
      m_invalidBestLayout = false;
      Select(ctx->outWidth, ctx->outHeight);
   }

   if (m_bestLayout == nullptr)
      return false;

   // Fit the layout inside the output
   const float outAR = ctx->outWidth / ctx->outHeight;
   const float layoutAR = m_bestLayout->width / m_bestLayout->height;
   float px = 0.f, py = 0.f;
   if (outAR > layoutAR)
   {
      ctx->srcWidth = m_bestLayout->height * outAR;
      ctx->srcHeight = m_bestLayout->height;
      px = 0.5f * (ctx->srcWidth - m_bestLayout->width);
   }
   else
   {
      ctx->srcWidth = m_bestLayout->width;
      ctx->srcHeight = m_bestLayout->width / outAR;
      py = 0.5f * (ctx->srcHeight - m_bestLayout->height);
   }

   for (auto& visual : m_bestLayout->visuals)
   {
      switch (visual.type)
      {
      case VisualType::DMD:
      {
         ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState(visual.srcUri);
         if (dmd.state.frame == nullptr)
            continue;
         LoadGlass(visual);
         m_vpxApi->UpdateTexture(&visual.dmdTex, dmd.source->width, dmd.source->height,
              dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_LUM8    ? VPXTextureFormat::VPXTEXFMT_BW
            : dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565 ? VPXTextureFormat::VPXTEXFMT_sRGB565
                                                                      : VPXTextureFormat::VPXTEXFMT_sRGB8,
            dmd.state.frame);
         vec4 glassArea;
         if (visual.glass == nullptr || visual.glassArea.z == 0.f || visual.glassArea.w == 0.f)
            glassArea = vec4(0.f, 0.f, 1.f, 1.f);
         else
         {
            int gWidth, gHeight;
            m_vpxApi->GetTextureInfo(visual.glass, &gWidth, &gHeight);
            glassArea.x = visual.glassArea.x / static_cast<float>(gWidth);
            glassArea.y = visual.glassArea.y / static_cast<float>(gHeight);
            glassArea.z = visual.glassArea.z / static_cast<float>(gWidth);
            glassArea.w = visual.glassArea.w / static_cast<float>(gHeight);
         }
         ctx->DrawDisplay(ctx, static_cast<VPXDisplayRenderStyle>(visual.liveStyle),
            // First layer: glass
            visual.glass, visual.glassTint.x, visual.glassTint.y, visual.glassTint.z, visual.glassRoughness, // Glass texture, tint and roughness
            glassArea.x, glassArea.y, glassArea.z, glassArea.w, // Glass texture coordinates (inside overall glass texture)
            visual.glassAmbient.x, visual.glassAmbient.y, visual.glassAmbient.z, // Glass lighting from room
            // Second layer: emitter
            visual.dmdTex, visual.tint.x, visual.tint.y, visual.tint.z, 1.f, 1.f, // DMD emitter, emitter tint, emitter brightness, emitter alpha
            visual.glassPad.x, visual.glassPad.y, visual.glassPad.z, visual.glassPad.w, // Emitter padding (from glass border)
            // Render quad
            px + visual.x, py + visual.y, visual.w, visual.h);
         break;
      }

      case VisualType::SegDisplay:
      {
         ResURIResolver::SegDisplayState frame = m_resURIResolver.GetSegDisplayState(visual.srcUri);
         if ((frame.state.frame == nullptr) || (frame.source->nElements != visual.nElements))
            continue;
         LoadGlass(visual);
         vec4 glassArea;
         if (visual.glass == nullptr || visual.glassArea.z == 0.f || visual.glassArea.w == 0.f)
            glassArea = vec4(0.f, 0.f, 1.f, 1.f);
         else
         {
            int gWidth, gHeight;
            m_vpxApi->GetTextureInfo(visual.glass, &gWidth, &gHeight);
            glassArea.x = visual.glassArea.x / static_cast<float>(gWidth);
            glassArea.y = visual.glassArea.y / static_cast<float>(gHeight);
            glassArea.z = visual.glassArea.z / static_cast<float>(gWidth);
            glassArea.w = visual.glassArea.w / static_cast<float>(gHeight);
         }
         const float elementW = visual.h * (24.f / 32.f); // TODO allow to adjust. For the time being, each segment element SDF is 24x32, fitted on visual height
         const float hGlassScale = glassArea.z / visual.w;
         vec4 segGlassArea = glassArea;
         segGlassArea.z = elementW * hGlassScale;
         for (size_t i = 0; i < frame.source->nElements; i++)
         {
            segGlassArea.x = glassArea.x + visual.xOffsets[i] * hGlassScale;
            // FIXME use hardware segment style
            ctx->DrawSegDisplay(ctx, static_cast<VPXSegDisplayRenderStyle>(visual.liveStyle), VPXSegDisplayHint::Generic,
               // First layer: glass
               visual.glass, visual.glassTint.x, visual.glassTint.y, visual.glassTint.z, visual.glassRoughness, // Glass texture, tint and roughness
               segGlassArea.x, segGlassArea.y, segGlassArea.z, segGlassArea.w, // Glass texture coordinates (inside overall glass texture, cut for each element)
               visual.glassAmbient.x, visual.glassAmbient.y, visual.glassAmbient.z, // Glass lighting from room
               // Second layer: emitter
               frame.source->elementType[i], frame.state.frame + 16 * i, visual.tint.x, visual.tint.y, visual.tint.z, 1.f, 1.f, // Segment emitter, emitter tint, emitter brightness, emitter alpha
               visual.glassPad.x, visual.glassPad.y, visual.glassPad.z, visual.glassPad.w, // Emitter padding (from glass border)
               // Render quad
               px + visual.x + visual.xOffsets[i], py + visual.y, elementW, visual.h);
         }
         break;
      }

      case VisualType::Image: break;
      }
   }
   return true;
}

}
