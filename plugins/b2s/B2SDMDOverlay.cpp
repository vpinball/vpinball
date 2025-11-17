// license:GPLv3+

#include "common.h"

#include "B2SDMDOverlay.h"

#include <cmath>
#include <vector>
#include <stack>
#include <algorithm>

namespace B2S {

MSGPI_BOOL_VAL_SETTING(scoreViewDMDOverlayProp, "ScoreViewDMDOverlay", "ScoreView DMD Overlay", "Enable a DMD overlay on the Score View", true, false);
MSGPI_BOOL_VAL_SETTING(scoreViewDMDAutoPosProp, "ScoreViewDMDAutoPos", "ScoreView DMD Automatic position", "Enable automatic DMD bounds detection", true, false);
MSGPI_INT_VAL_SETTING(scoreViewDMDXProp, "ScoreViewDMDX", "ScoreView DMD X position", "DMD overlay X position", true, 0, 0xFFFF, 0);
MSGPI_INT_VAL_SETTING(scoreViewDMDYProp, "ScoreViewDMDY", "ScoreView DMD Y position", "DMD overlay Y position", true, 0, 0xFFFF, 0);
MSGPI_INT_VAL_SETTING(scoreViewDMDWProp, "ScoreViewDMDW", "ScoreView DMD width", "DMD overlay width", true, 0, 0xFFFF, 0);
MSGPI_INT_VAL_SETTING(scoreViewDMDHProp, "ScoreViewDMDH", "ScoreView DMD height", "DMD overlay height", true, 0, 0xFFFF, 0);

MSGPI_BOOL_VAL_SETTING(backglassDMDOverlayProp, "BackglassDMDOverlay", "Backglass DMD Overlay", "Enable a DMD overlay on the Backglass", true, false);
MSGPI_BOOL_VAL_SETTING(backglassDMDAutoPosProp, "BackglassDMDAutoPos", "Backglass DMD Automatic position", "Enable automatic DMD bounds detection", true, false);
MSGPI_INT_VAL_SETTING(backglassDMDXProp, "BackglassDMDX", "Backglass DMD X position", "DMD overlay X position", true, 0, 0xFFFF, 0);
MSGPI_INT_VAL_SETTING(backglassDMDYProp, "BackglassDMDY", "Backglass DMD Y position", "DMD overlay Y position", true, 0, 0xFFFF, 0);
MSGPI_INT_VAL_SETTING(backglassDMDWProp, "BackglassDMDW", "Backglass DMD width", "DMD overlay width", true, 0, 0xFFFF, 0);
MSGPI_INT_VAL_SETTING(backglassDMDHProp, "BackglassDMDH", "Backglass DMD height", "DMD overlay height", true, 0, 0xFFFF, 0);

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
      msgApi->RegisterSetting(endpointId, &scoreViewDMDOverlayProp);
      msgApi->RegisterSetting(endpointId, &scoreViewDMDAutoPosProp);
      msgApi->RegisterSetting(endpointId, &scoreViewDMDXProp);
      msgApi->RegisterSetting(endpointId, &scoreViewDMDYProp);
      msgApi->RegisterSetting(endpointId, &scoreViewDMDWProp);
      msgApi->RegisterSetting(endpointId, &scoreViewDMDHProp);
      m_enable = scoreViewDMDOverlayProp_Val != 0;
      m_detectDmdFrame = scoreViewDMDAutoPosProp_Val != 0;
      m_frame.x = scoreViewDMDXProp_Val;
      m_frame.y = scoreViewDMDYProp_Val;
      m_frame.z = scoreViewDMDWProp_Val;
      m_frame.w = scoreViewDMDHProp_Val;
   }
   else
   {
      msgApi->RegisterSetting(endpointId, &backglassDMDOverlayProp);
      msgApi->RegisterSetting(endpointId, &backglassDMDAutoPosProp);
      msgApi->RegisterSetting(endpointId, &backglassDMDXProp);
      msgApi->RegisterSetting(endpointId, &backglassDMDYProp);
      msgApi->RegisterSetting(endpointId, &backglassDMDWProp);
      msgApi->RegisterSetting(endpointId, &backglassDMDHProp);
      m_enable = backglassDMDOverlayProp_Val != 0;
      m_detectDmdFrame = backglassDMDAutoPosProp_Val != 0;
      m_frame.x = backglassDMDXProp_Val;
      m_frame.y = backglassDMDYProp_Val;
      m_frame.z = backglassDMDWProp_Val;
      m_frame.w = backglassDMDHProp_Val;
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
