// license:GPLv3+

#include "Nixie.h"

#include "plugins/ControllerPlugin.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace Nixie
{

static inline float InvsRGB(const float x) { return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f)); }

Nixie::Nixie(const MsgPluginAPI* api, unsigned int endpointId, VPXPluginAPI* vpxApi)
   : m_resURIResolver(*api, endpointId, true, true, false)
   , m_vpxApi(vpxApi)
   , m_msgApi(api)
   , m_endpointId(endpointId)
{
   m_onDmdChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdChangedMsgId, OnResChanged, this);
   m_onSegChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSegChangedMsgId, OnResChanged, this);
}

Nixie::~Nixie()
{
   m_msgApi->UnsubscribeMsg(m_onSegChangedMsgId, OnResChanged, this);
   m_msgApi->ReleaseMsgID(m_onSegChangedMsgId);
   m_msgApi->UnsubscribeMsg(m_onDmdChangedMsgId, OnResChanged, this);
   m_msgApi->ReleaseMsgID(m_onDmdChangedMsgId);
   for (auto& image : m_images)
      m_vpxApi->DeleteTexture(image.second);
   for (auto& layout : m_layouts)
      for (auto& visual : layout.visuals)
         if (visual.dmdTex)
            m_vpxApi->DeleteTexture(visual.dmdTex);
}

void Nixie::OnResChanged(const unsigned int msgId, void* userData, void* msgData) { static_cast<Nixie*>(userData)->m_invalidBestLayout = true; }

void Nixie::Load(const std::filesystem::path& path)
{
   if (!std::filesystem::exists(path))
      return;
   std::error_code ec;
   if (std::filesystem::is_directory(path, ec))
   {
      for (const auto& entry : std::filesystem::directory_iterator(path))
      {
         if (!entry.is_directory() && lowerCase(entry.path().extension()) == ".nix")
         {
            std::ifstream ifs(entry.path());
            Parse(entry.path());
         }
      }
   }
   else
   {
      Parse(path);
   }
}

void Nixie::Parse(const std::filesystem::path& path)
{
   std::vector<PinballPlugin::NixieFile::Layout> parsed;
   string error;
   if (!PinballPlugin::NixieFile::Parse(path, parsed, error))
   {
      LOGE(error);
      return;
   }
   // Adapt the file semantics to the render ones: authored rects are converted from the
   // top left origin of the layout space to the bottom up render space, and the glass
   // ambient lighting is converted from sRGB to linear
   vector<Layout> layouts;
   layouts.reserve(parsed.size());
   for (const PinballPlugin::NixieFile::Layout& parsedLayout : parsed)
   {
      Layout& layout = layouts.emplace_back();
      layout.path = parsedLayout.path;
      layout.window = parsedLayout.window;
      layout.width = parsedLayout.width;
      layout.height = parsedLayout.height;
      layout.fit = parsedLayout.fit;
      layout.visuals.reserve(parsedLayout.visuals.size());
      for (const PinballPlugin::NixieFile::Visual& parsedVisual : parsedLayout.visuals)
      {
         Visual& visual = layout.visuals.emplace_back(parsedVisual);
         visual.y = layout.height - visual.y - visual.h;
         visual.glassAmbient.x = InvsRGB(visual.glassAmbient.x);
         visual.glassAmbient.y = InvsRGB(visual.glassAmbient.y);
         visual.glassAmbient.z = InvsRGB(visual.glassAmbient.z);
      }
   }
   // Only keep the layouts of a fully valid file, so that a broken file falls back to the next layout source
   m_invalidBestLayout = true;
   m_layouts.insert(m_layouts.end(), layouts.begin(), layouts.end());
}

VPXTexture Nixie::LoadTexture(const std::string& relPath)
{
   // Cached by resolved path, as layouts loaded from different folders may use the same relative path for different files
   const std::filesystem::path fullPath = (m_bestLayout->path.parent_path() / PluginStrings::PathFromUTF8(relPath)).lexically_normal();
   const string key = PluginStrings::PathToUTF8(fullPath);
   const auto texImage = m_images.find(key);
   if (texImage != m_images.end())
      return texImage->second;
   VPXTexture texture = nullptr;
   std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
   if (file.is_open())
   {
      const std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      std::vector<uint8_t> buffer((size_t)size);
      file.read(reinterpret_cast<char*>(buffer.data()), size);
      file.close();
      texture = m_vpxApi->CreateTexture(buffer.data(), static_cast<int>(size));
   }
   else
   {
      LOGE("Missing texture file: " + PluginStrings::PathToUTF8(fullPath));
   }
   m_images[key] = texture;
   return texture;
}

void Nixie::LoadGlass(Visual& visual)
{
   assert((visual.type == VisualType::DMD) || (visual.type == VisualType::Screen) || (visual.type == VisualType::SegDisplay));
   if ((visual.glass == nullptr) && !visual.glassPath.empty())
      visual.glass = LoadTexture(visual.glassPath);
}

void Nixie::Select(const VPXWindowId window, const float scoreW, const float scoreH)
{
   m_bestLayout = nullptr;

   // Evaluate output aspect ratio
   const float rtAR = scoreW / scoreH;

   // Evaluate layouts targeting this window against current context
   PinballPlugin::ResURIResolver::SegDisplayState segDisplay;
   PinballPlugin::ResURIResolver::DisplayState display;
   vector<Layout*> layouts;
   for (auto& layout : m_layouts)
   {
      if (layout.window != window)
         continue;
      layouts.push_back(&layout);
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
            if ((display.source == nullptr) || (display.source->width * visual.displaySize.y != visual.displaySize.x * display.source->height)
               || ((display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_UNKNOWN
                  && (display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_NEON_PLASMA
                  && (display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_RED_LED
                  && (display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_RGB_LED))
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals += 10; // To favor DMD over alphanumeric seg displays
            break;

         case VisualType::Screen:
            display = m_resURIResolver.GetDisplayState(visual.srcUri);
            if ((display.source == nullptr) || (display.source->width * visual.displaySize.y != visual.displaySize.x * display.source->height)
               || ((display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_UNKNOWN
                  && (display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY
                  && (display.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) != CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY))
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals++;
            break;

         case VisualType::SegDisplay:
            segDisplay = m_resURIResolver.GetSegDisplayState(visual.srcUri);
            if ((segDisplay.source == nullptr) || (segDisplay.source->nElements != visual.nElements))
               layout.unmatchedVisuals++;
            else
               layout.matchedVisuals++;
            break;

         case VisualType::Picture: break;
         }
      }
   }

   // Select the best matching layout:
   // - without missing visual sources
   // - with a maximum of satisfied visuals
   // - with displays size matching visuals (16 alphanum vs 20 alphanum, 128x32,...)
   // - with the best matching layout aspect ratio
   if (layouts.empty())
      return;
   m_bestLayout = *std::max_element(layouts.begin(), layouts.end(),
      [](const Layout* a, const Layout* b) -> bool
      {
         if (a->unmatchedVisuals != b->unmatchedVisuals) // The least unmatched visuals
            return a->unmatchedVisuals > b->unmatchedVisuals;
         if (a->matchedVisuals != b->matchedVisuals) // The maximum matched visual
            return a->matchedVisuals < b->matchedVisuals;
         if (a->unfittedPixels != b->unfittedPixels) // The one fitting the output better
            return a->unfittedPixels < b->unfittedPixels;
         return false;
      });

   if (m_bestLayout->unmatchedVisuals > 0)
      m_bestLayout = nullptr;
}

bool Nixie::Render(VPXRenderContext2D* ctx)
{
   if (m_invalidBestLayout)
   {
      m_invalidBestLayout = false;
      Select(ctx->window, ctx->outWidth, ctx->outHeight);
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
      case VisualType::Screen:
      {
         PinballPlugin::ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState(visual.srcUri);
         if (dmd.state.frame == nullptr)
            continue;
         LoadGlass(visual);
         if (!visual.hasUploadedFrame || (visual.dmdTex == nullptr) || (dmd.state.frameId != visual.uploadedFrameId) || (*dmd.source != visual.uploadedSrc))
         {
            m_vpxApi->UpdateTexture(&visual.dmdTex, dmd.source->width, dmd.source->height,
               dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F       ? VPXTextureFormat::VPXTEXFMT_BW32F
                  : dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565 ? VPXTextureFormat::VPXTEXFMT_sRGB565
                                                                            : VPXTextureFormat::VPXTEXFMT_sRGB8,
               dmd.state.frame);
            visual.uploadedSrc = *dmd.source;
            visual.uploadedFrameId = dmd.state.frameId;
            visual.hasUploadedFrame = true;
         }
         vec4 glassArea;
         if (visual.glass == nullptr || visual.glassArea.z == 0.f || visual.glassArea.w == 0.f)
            glassArea = vec4(0.f, 0.f, 1.f, 1.f);
         else
         {
            VPXTextureInfo* texInfo = m_vpxApi->GetTextureInfo(visual.glass);
            glassArea.x = visual.glassArea.x / static_cast<float>(texInfo->width);
            glassArea.y = visual.glassArea.y / static_cast<float>(texInfo->height);
            glassArea.z = visual.glassArea.z / static_cast<float>(texInfo->width);
            glassArea.w = visual.glassArea.w / static_cast<float>(texInfo->height);
         }
         VPXDisplayRenderStyle style;
         if (visual.type == VisualType::DMD)
            style = static_cast<VPXDisplayRenderStyle>(visual.liveStyle);
         else
         {
            switch (visual.liveStyle)
            {
            case 1: style = VPXDMDStyle_Pixelated; break;
            case 2: style = VPXDMDStyle_CRT; break;
            default: style = VPXDMDStyle_Smoothed; break;
            }
         }
         ctx->DrawDisplay(ctx, style,
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
         PinballPlugin::ResURIResolver::SegDisplayState frame = m_resURIResolver.GetSegDisplayState(visual.srcUri);
         if ((frame.state.frame == nullptr) || (frame.source->nElements != visual.nElements) || (frame.source->nElements > CTLPI_SEG_MAX_DISP_ELEMENTS))
            continue;
         LoadGlass(visual);
         vec4 glassArea;
         if (visual.glass == nullptr || visual.glassArea.z == 0.f || visual.glassArea.w == 0.f)
            glassArea = vec4(0.f, 0.f, 1.f, 1.f);
         else
         {
            VPXTextureInfo* texInfo = m_vpxApi->GetTextureInfo(visual.glass);
            glassArea.x = visual.glassArea.x / static_cast<float>(texInfo->width);
            glassArea.y = visual.glassArea.y / static_cast<float>(texInfo->height);
            glassArea.z = visual.glassArea.z / static_cast<float>(texInfo->width);
            glassArea.w = visual.glassArea.w / static_cast<float>(texInfo->height);
         }
         const float elementW = visual.h * (24.f / 32.f); // TODO allow to adjust. For the time being, each segment element SDF is 24x32, fitted on visual height
         const float hGlassScale = glassArea.z / visual.w;
         vec4 segGlassArea = glassArea;
         segGlassArea.z = elementW * hGlassScale;
         VPXSegDisplayHint hint = VPXSegDisplayHint::Generic;
         VPXSegDisplayRenderStyle style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
         if (visual.liveStyle == -1)
         {
            if ((frame.source->hardware & CTLPI_SEG_HARDWARE_FAMILY_MASK) == CTLPI_SEG_HARDWARE_NEON_PLASMA)
               style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
            else if ((frame.source->hardware & CTLPI_SEG_HARDWARE_FAMILY_MASK) == CTLPI_SEG_HARDWARE_VFD_GREEN)
               style = VPXSegDisplayRenderStyle::VPXSegStyle_GreenVFD;
            else if ((frame.source->hardware & CTLPI_SEG_HARDWARE_FAMILY_MASK) == CTLPI_SEG_HARDWARE_VFD_BLUE)
            {
               style = VPXSegDisplayRenderStyle::VPXSegStyle_BlueVFD;
               if (frame.source->hardware == CTLPI_SEG_HARDWARE_GTS1_4DIGIT //
                  || frame.source->hardware == CTLPI_SEG_HARDWARE_GTS1_6DIGIT //
                  || frame.source->hardware == CTLPI_SEG_HARDWARE_GTS80A_7DIGIT //
                  || frame.source->hardware == CTLPI_SEG_HARDWARE_GTS80B_20DIGIT //
               )
                  hint = VPXSegDisplayHint::Gottlieb;
            }
         }
         else
         {
            style = static_cast<VPXSegDisplayRenderStyle>(visual.liveStyle);
         }
         for (size_t i = 0; i < frame.source->nElements; i++)
         {
            segGlassArea.x = glassArea.x + visual.xOffsets[i] * hGlassScale;
            ctx->DrawSegDisplay(ctx, style, hint,
               // First layer: glass
               visual.glass, visual.glassTint.x, visual.glassTint.y, visual.glassTint.z, visual.glassRoughness, // Glass texture, tint and roughness
               segGlassArea.x, segGlassArea.y, segGlassArea.z, segGlassArea.w, // Glass texture coordinates (inside overall glass texture, cut for each element)
               visual.glassAmbient.x, visual.glassAmbient.y, visual.glassAmbient.z, // Glass lighting from room
               // Second layer: emitter
               frame.source->elementType[i], frame.state.frame + 16 * i, visual.tint.x, visual.tint.y, visual.tint.z, 1.f,
               1.f, // Segment emitter, emitter tint, emitter brightness, emitter alpha
               visual.glassPad.x, visual.glassPad.y, visual.glassPad.z, visual.glassPad.w, // Emitter padding (from glass border)
               // Render quad
               px + visual.x + visual.xOffsets[i], py + visual.y, elementW, visual.h);
         }
         break;
      }

      case VisualType::Picture:
      {
         if (visual.pictureTex == nullptr)
            visual.pictureTex = LoadTexture(visual.srcUri);
         if (visual.pictureTex == nullptr)
            continue;
         const VPXTextureInfo* texInfo = m_vpxApi->GetTextureInfo(visual.pictureTex);
         ctx->DrawImage(ctx, visual.pictureTex, //
            visual.tint.x, visual.tint.y, visual.tint.z, 1.f, // tint color and alpha
            0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), // whole image
            0.f, 0.f, 0.f, // no pivot point nor rotation
            px + visual.x, py + visual.y, visual.w, visual.h); // Render quad
         break;
      }
      }
   }
   return true;
}

}
