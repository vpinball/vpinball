#include "common.h"

#include "B2SDMDOverlay.h"

#include <cmath>
#include <vector>
#include <stack>
#include <algorithm>

namespace B2S {

B2SDMDOverlay::B2SDMDOverlay(ResURIResolver& resURIResolver, VPXTexture& dmdTex, VPXTexture backImage)
   : m_resURIResolver(resURIResolver)
   , m_dmdTex(dmdTex)
   , m_backImage(backImage)
{
}

void B2SDMDOverlay::LoadSettings(MsgPluginAPI* const msgApi, const string& pluginId, const string& prefix)
{
   char buf[32];
   msgApi->GetSetting(pluginId.c_str(), (prefix + "DMDOverlay").c_str(), buf, sizeof(buf));
   m_enable = atoi(buf) != 0;
   msgApi->GetSetting(pluginId.c_str(), (prefix + "DMDAutoPos").c_str(), buf, sizeof(buf));
   m_detectDmdFrame = atoi(buf) != 0;
   msgApi->GetSetting(pluginId.c_str(), (prefix + "DMDX").c_str(), buf, sizeof(buf));
   m_frame.x = atoi(buf);
   msgApi->GetSetting(pluginId.c_str(), (prefix + "DMDY").c_str(), buf, sizeof(buf));
   m_frame.y = atoi(buf);
   msgApi->GetSetting(pluginId.c_str(), (prefix + "DMDWidth").c_str(), buf, sizeof(buf));
   m_frame.z = atoi(buf);
   msgApi->GetSetting(pluginId.c_str(), (prefix + "DMDHeight").c_str(), buf, sizeof(buf));
   m_frame.w = atoi(buf);
}

void B2SDMDOverlay::Render(VPXRenderContext2D* ctx)
{
   if (!m_enable)
      return;

   ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState("ctrl://default/display");
   if (dmd.state.frame == nullptr)
      return;

   // When DMD source change, search for the DMD sub frame as it depends on the DMD source aspect ratio
   if (m_detectDmdFrame && m_backImage && m_detectSrcId.id != dmd.source->id.id)
   {
      m_frame = ivec4();
      m_detectSrcId.id = dmd.source->id.id;
      const float ar = static_cast<float>(dmd.source->width) / static_cast<float>(dmd.source->height);
      m_frameSearch = std::async(std::launch::async, [this, ar]() { return SearchDmdSubFrame(m_backImage, ar); });
   }

   if (m_frameSearch.valid() && m_frameSearch.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      m_frame = m_frameSearch.get();

   if (m_frame.z == 0.f || m_frame.w == 0.f)
      return;

   UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height,
      dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_LUM8         ? VPXTextureFormat::VPXTEXFMT_BW
         : dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565 ? VPXTextureFormat::VPXTEXFMT_sRGB565
                                                                   : VPXTextureFormat::VPXTEXFMT_sRGB8,
      dmd.state.frame);

   vec4 glassArea, glassAmbient(1.f, 1.f, 1.f, 1.f), glassTint(1.f, 1.f, 1.f, 1.f), glassPad;
   vec4 dmdTint(1.f, 1.f, 1.f, 1.f);
   ctx->DrawDisplay(ctx, VPXDisplayRenderStyle::VPXDMDStyle_Plasma,
      // First layer: glass
      nullptr, glassTint.x, glassTint.y, glassTint.z, 0.f, // Glass texture, tint and roughness
      glassArea.x, glassArea.y, glassArea.z, glassArea.w, // Glass texture coordinates (inside overall glass texture)
      glassAmbient.x, glassAmbient.y, glassAmbient.z, // Glass lighting from room
      // Second layer: emitter
      m_dmdTex, dmdTint.x, dmdTint.y, dmdTint.z, 1.f, 1.f, // DMD emitter, emitter tint, emitter brightness, emitter alpha
      glassPad.x, glassPad.y, glassPad.z, glassPad.w, // Emitter padding (from glass border)
      // Render quad
      static_cast<float>(m_frame.x), static_cast<float>(m_frame.y), static_cast<float>(m_frame.z), static_cast<float>(m_frame.w));
}

ivec4 B2SDMDOverlay::SearchDmdSubFrame(VPXTexture image, float dmdAspectRatio)
{
   ivec4 subFrame;

   VPXTextureInfo* texInfo = GetTextureInfo(image);
   if (texInfo == nullptr)
      return subFrame;

   // Find the largest dark rectangle in the background image
   float maxHeuristic = 0.f;
   float selectedAspectRatio = 1.f;
   std::stack<int> st;
   vector<int> heights(texInfo->width, 0); // height of empty columns above each pixels in the row as we scan them downward
   for (unsigned int y = 0; y < texInfo->height; ++y)
   {
      for (unsigned int x = 0; x < texInfo->width; ++x)
      {
         uint8_t lum = 0;
         const int pos = y * texInfo->width + x;
         switch (texInfo->format)
         {
         case VPXTEXFMT_BW: lum = texInfo->data[pos]; break;
         case VPXTEXFMT_sRGB8: lum = static_cast<uint8_t>(0.299f * texInfo->data[pos * 3] + 0.587f * texInfo->data[pos * 3 + 1] + 0.114f * texInfo->data[pos * 3 + 2]); break;
         case VPXTEXFMT_sRGBA8: lum = static_cast<uint8_t>(0.299f * texInfo->data[pos * 4] + 0.587f * texInfo->data[pos * 4 + 1] + 0.114f * texInfo->data[pos * 4 + 2]); break;
         default: return subFrame;
         }
         if (lum < 8)
            heights[x] += 1;
         else
            heights[x] = 0;
      }

      // Evaluate each rectangle that can be formed with the heights of the columns
      for (unsigned int x = 0; x <= texInfo->width; ++x)
      {
         while (!st.empty() && (x == texInfo->width || heights[st.top()] > heights[x]))
         {
            const int height = heights[st.top()];
            st.pop();
            const int width = st.empty() ? x : x - st.top() - 1;
            const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            const float arMatch = aspectRatio < dmdAspectRatio ? aspectRatio / dmdAspectRatio : dmdAspectRatio / aspectRatio;
            const float heuristic = height * width * powf(arMatch, 0.5f);
            if (heuristic > maxHeuristic)
            {
               maxHeuristic = heuristic;
               subFrame.x = st.empty() ? 0 : st.top() + 1;
               subFrame.y = texInfo->height - y - 1;
               subFrame.z = width;
               subFrame.w = height;
            }
         }
         if (x < texInfo->width)
            st.push(x);
      }
   }

   // Do not select a too small area
   if (static_cast<unsigned int>(subFrame.z * subFrame.w) < texInfo->width * texInfo->height / 16)
      subFrame = ivec4();

   return subFrame;
}

}