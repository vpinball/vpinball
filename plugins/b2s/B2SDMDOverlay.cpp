// license:GPLv3+

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

void B2SDMDOverlay::LoadSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId, bool isScoreView)
{
   if (isScoreView)
   {
      msgApi->RegisterSetting(endpointId, &m_scoreViewDMDOverlayProp);
      msgApi->RegisterSetting(endpointId, &m_scoreViewDMDAutoPosProp);
      msgApi->RegisterSetting(endpointId, &m_scoreViewDMDXProp);
      msgApi->RegisterSetting(endpointId, &m_scoreViewDMDYProp);
      msgApi->RegisterSetting(endpointId, &m_scoreViewDMDWProp);
      msgApi->RegisterSetting(endpointId, &m_scoreViewDMDHProp);
      m_enable = m_scoreViewDMDOverlayProp.boolDef.val != 0;
      m_detectDmdFrame = m_scoreViewDMDAutoPosProp.boolDef.val != 0;
      m_frame.x = m_scoreViewDMDXProp.intDef.val != 0;
      m_frame.y = m_scoreViewDMDYProp.intDef.val != 0;
      m_frame.z = m_scoreViewDMDWProp.intDef.val != 0;
      m_frame.w = m_scoreViewDMDHProp.intDef.val != 0;
   }
   else
   {
      msgApi->RegisterSetting(endpointId, &m_backglassDMDOverlayProp);
      msgApi->RegisterSetting(endpointId, &m_backglassDMDAutoPosProp);
      msgApi->RegisterSetting(endpointId, &m_backglassDMDXProp);
      msgApi->RegisterSetting(endpointId, &m_backglassDMDYProp);
      msgApi->RegisterSetting(endpointId, &m_backglassDMDWProp);
      msgApi->RegisterSetting(endpointId, &m_backglassDMDHProp);
      m_enable = m_backglassDMDOverlayProp.boolDef.val != 0;
      m_detectDmdFrame = m_backglassDMDAutoPosProp.boolDef.val != 0;
      m_frame.x = m_backglassDMDXProp.intDef.val != 0;
      m_frame.y = m_backglassDMDYProp.intDef.val != 0;
      m_frame.z = m_backglassDMDWProp.intDef.val != 0;
      m_frame.w = m_backglassDMDHProp.intDef.val != 0;
   }
}

void B2SDMDOverlay::Render(VPXRenderContext2D* ctx)
{
   if (!m_enable)
      return;

   ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState("ctrl://default/display"s);
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

   if (m_frame.z == 0 || m_frame.w == 0)
      return;

   switch (dmd.source->frameFormat)
   {
   case CTLPI_DISPLAY_FORMAT_LUM8: UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_BW, dmd.state.frame); break;
   case CTLPI_DISPLAY_FORMAT_SRGB888: UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_sRGB8, dmd.state.frame); break;
   case CTLPI_DISPLAY_FORMAT_SRGB565: UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_sRGB565, dmd.state.frame); break;
   default: return;
   }

   vec4 glassArea;
   vec4 glassAmbient(1.f, 1.f, 1.f, 1.f);
   vec4 glassTint(1.f, 1.f, 1.f, 1.f);
   vec4 glassPad;
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

ivec4 B2SDMDOverlay::SearchDmdSubFrame(VPXTexture image, float dmdAspectRatio) const
{
   ivec4 subFrame;

   const VPXTextureInfo* const texInfo = GetTextureInfo(image);
   if (texInfo == nullptr)
      return subFrame;

   unsigned int pos_step;
   switch (texInfo->format)
   {
   case VPXTEXFMT_BW: pos_step = 1; break;
   case VPXTEXFMT_sRGB8: pos_step = 3; break;
   case VPXTEXFMT_sRGBA8: pos_step = 4; break;
   default: pos_step = 0;
   }

   // Find the largest dark rectangle in the background image
   float maxHeuristic = 0.f;
   std::stack<int> st;
   vector<int> heights(texInfo->width, 0); // height of empty columns above each pixels in the row as we scan them downward
   unsigned int pos = 0;
   for (unsigned int y = 0; y < texInfo->height; ++y)
   {
      for (unsigned int x = 0; x < texInfo->width; ++x,pos+=pos_step)
      {
         uint8_t lum = 0;
         switch (texInfo->format)
         {
         case VPXTEXFMT_BW: lum = texInfo->data[pos]; break;
         case VPXTEXFMT_sRGB8: lum = static_cast<uint8_t>(0.299f * texInfo->data[pos] + 0.587f * texInfo->data[pos + 1] + 0.114f * texInfo->data[pos + 2]); break;
         case VPXTEXFMT_sRGBA8: lum = static_cast<uint8_t>(0.299f * texInfo->data[pos] + 0.587f * texInfo->data[pos + 1] + 0.114f * texInfo->data[pos + 2]); break;
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
            const float heuristic = static_cast<float>(height * width) * sqrtf(arMatch);
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
