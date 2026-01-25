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

B2SDMDOverlay::~B2SDMDOverlay()
{
   m_stopSearching = true;
   if (m_frameSearch.valid())
      m_frameSearch.get();
}

void B2SDMDOverlay::RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId)
{
   msgApi->RegisterSetting(endpointId, &backglassDMDOverlayProp);
   msgApi->RegisterSetting(endpointId, &backglassDMDAutoPosProp);
   msgApi->RegisterSetting(endpointId, &backglassDMDXProp);
   msgApi->RegisterSetting(endpointId, &backglassDMDYProp);
   msgApi->RegisterSetting(endpointId, &backglassDMDWProp);
   msgApi->RegisterSetting(endpointId, &backglassDMDHProp);
   msgApi->RegisterSetting(endpointId, &scoreViewDMDOverlayProp);
   msgApi->RegisterSetting(endpointId, &scoreViewDMDAutoPosProp);
   msgApi->RegisterSetting(endpointId, &scoreViewDMDXProp);
   msgApi->RegisterSetting(endpointId, &scoreViewDMDYProp);
   msgApi->RegisterSetting(endpointId, &scoreViewDMDWProp);
   msgApi->RegisterSetting(endpointId, &scoreViewDMDHProp);
}

void B2SDMDOverlay::LoadSettings(bool isScoreView)
{
   if (isScoreView)
   {
      m_enable = scoreViewDMDOverlayProp_Val != 0;
      m_detectDmdFrame = scoreViewDMDAutoPosProp_Val != 0;
      if (!m_detectDmdFrame)
      {
         m_frame.x = scoreViewDMDXProp_Val;
         m_frame.y = scoreViewDMDYProp_Val;
         m_frame.z = scoreViewDMDWProp_Val;
         m_frame.w = scoreViewDMDHProp_Val;
      }
   }
   else
   {
      m_enable = backglassDMDOverlayProp_Val != 0;
      m_detectDmdFrame = backglassDMDAutoPosProp_Val != 0;
      if (!m_detectDmdFrame)
      {
         m_frame.x = backglassDMDXProp_Val;
         m_frame.y = backglassDMDYProp_Val;
         m_frame.z = backglassDMDWProp_Val;
         m_frame.w = backglassDMDHProp_Val;
      }
   }
}

void B2SDMDOverlay::UpdateBackgroundImage(VPXTexture backImage)
{
   if (m_backImage != backImage)
   {
      m_backImage = backImage;
      if (m_detectDmdFrame)
      {
         m_frame = ivec4();
         m_detectSrcId.id = 0;
      }
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
   case CTLPI_DISPLAY_FORMAT_LUM32F: UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_BW32F, dmd.state.frame); break;
   case CTLPI_DISPLAY_FORMAT_SRGB888: UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_sRGB8, dmd.state.frame); break;
   case CTLPI_DISPLAY_FORMAT_SRGB565: UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_sRGB565, dmd.state.frame); break;
   default: return;
   }

   vec4 glassArea(0.f, 0.f, 0.f, 0.f);
   vec4 glassAmbient(1.f, 1.f, 1.f, 1.f);
   vec4 glassTint(1.f, 1.f, 1.f, 1.f);
   vec4 glassPad(0.f, 0.f, 0.f, 0.f);
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
   const VPXTextureInfo* const texInfo = GetTextureInfo(image);
   if (texInfo == nullptr)
      return ivec4();

   unsigned int pos_step;
   switch (texInfo->format)
   {
   case VPXTEXFMT_BW32F: pos_step = 1; break;
   case VPXTEXFMT_sRGB8: pos_step = 3; break;
   case VPXTEXFMT_sRGBA8: pos_step = 4; break;
   default: pos_step = 0;
   }

   // Heuristic to select large rectangles, favoring screen centered ones
   auto heuristic = [texW = static_cast<float>(texInfo->width)](const ivec4& frame)
   { return static_cast<float>(frame.z) / texW - 1.f * fabs(0.5f - static_cast<float>(frame.x + frame.z / 2) / texW); };

   const int padding = (2 * texInfo->width) / 1920;
   float lumMin = 0.f;
   float lumMax = 256.f;
   ivec4 searchFrame(0, 0, texInfo->width, texInfo->height);
   ivec4 bestFrame;
   for (int search = 0; search < 7; search++)
   {
      const float lumLimit = (lumMin + lumMax) * 0.5f;

      LOGD("DMD area search thr: %f area is %d,%d %dx%d", lumLimit, searchFrame.x, searchFrame.y, searchFrame.z, searchFrame.w);

      // Find the largest dark rectangle in the background image
      ivec4 subFrame;
      ivec4 searchSubFrame;
      std::stack<int> st;
      vector<int> heights(texInfo->width, 0); // height of empty columns above each pixels in the row as we scan them downward
      for (int y = searchFrame.y; y < (searchFrame.y + searchFrame.w); ++y)
      {
         // If disabled while searching, just abort
         if (m_stopSearching)
            return ivec4();
         unsigned int pos = (y * texInfo->width + searchFrame.x) * pos_step;
         for (int x = searchFrame.x; x < (searchFrame.x + searchFrame.z); ++x, pos += pos_step)
         {
            float lum = 0;
            switch (texInfo->format)
            {
            case VPXTEXFMT_BW32F:
               lum = 255.f * static_cast<float*>(texInfo->data)[pos];
               break;

            case VPXTEXFMT_sRGB8:
            case VPXTEXFMT_sRGBA8:
               lum = 0.299f * static_cast<float>(static_cast<uint8_t*>(texInfo->data)[pos]) + 0.587f * static_cast<float>(static_cast<uint8_t*>(texInfo->data)[pos + 1]) + 0.114f * static_cast<float>(static_cast<uint8_t*>(texInfo->data)[pos + 2]);
               break;

            default: return ivec4();
            }
            if (lum < lumLimit)
               heights[x] += 1;
            else
               heights[x] = 0;
         }

         // Evaluate each rectangle that can be formed with the heights of the columns
         for (int x = searchFrame.x; x <= searchFrame.x + searchFrame.z; ++x)
         {
            while (!st.empty() && (x == (searchFrame.x + searchFrame.z) || heights[st.top()] > heights[x]))
            {
               const int height = heights[st.top()];
               st.pop();
               const int width = st.empty() ? (x - searchFrame.x) : (x - st.top() - 1);
               if (width > 6 * padding && height > 3 * padding)
               {
                  ivec4 unpaddedFrame;
                  unpaddedFrame.x = st.empty() ? searchFrame.x : st.top() + 1;
                  unpaddedFrame.y = y - height + 1;
                  unpaddedFrame.z = width;
                  unpaddedFrame.w = height;
                  // Extends search frame for next pass to include this area even if not selected
                  if (searchSubFrame.x == 0)
                     searchSubFrame = unpaddedFrame;
                  else
                  {
                     ivec4 newSearchFrame;
                     newSearchFrame.x = std::min(searchSubFrame.x, unpaddedFrame.x);
                     newSearchFrame.y = std::min(searchSubFrame.y, unpaddedFrame.y);
                     newSearchFrame.z = std::max(searchSubFrame.x + searchSubFrame.z, unpaddedFrame.x + unpaddedFrame.z) - newSearchFrame.x;
                     newSearchFrame.w = std::max(searchSubFrame.y + searchSubFrame.w, unpaddedFrame.y + unpaddedFrame.w) - newSearchFrame.y;
                     searchSubFrame = newSearchFrame;
                  }
                  // Add some padding and fit to searched aspect ratio
                  ivec4 frame;
                  frame.x = unpaddedFrame.x + padding;
                  frame.y = unpaddedFrame.y + padding;
                  frame.z = unpaddedFrame.z - 2 * padding;
                  frame.w = unpaddedFrame.w - 2 * padding;
                  if (const float ar = static_cast<float>(frame.z) / static_cast<float>(frame.w); ar > dmdAspectRatio)
                  {
                     const int w = static_cast<int>(static_cast<float>(frame.w) * dmdAspectRatio);
                     frame.x += (frame.z - w) / 2;
                     frame.z = w;
                  }
                  else
                  {
                     const int h = static_cast<int>(static_cast<float>(frame.z) / dmdAspectRatio);
                     frame.y += (frame.w - h) / 2;
                     frame.w = h;
                  }
                  // Selected area must be large enough (at least 1/3 of the backglass width)
                  if ((3 * frame.z >= static_cast<int>(texInfo->width)) && (heuristic(frame) > heuristic(subFrame)))
                     subFrame = frame;
               }
            }
            if (x < (searchFrame.x + searchFrame.z))
               st.push(x);
         }
      }

      LOGD("DMD area search %f -> %f, thr: %f lead to %d,%d %dx%d Heur:%f, PrevHeur: %f", lumMin, lumMax, lumLimit, subFrame.x, subFrame.y, subFrame.z, subFrame.w,
         heuristic(subFrame), heuristic(bestFrame));

      // There are no heuristic between luminance levels, this can lead to unwanted behavior (for example drifting insde a box)
      if (subFrame.w > 0) // && (bestFrame.w == 0 || (heuristic(subFrame) > 0.6f * heuristic(bestFrame))))
      {
         lumMax = lumLimit;
         bestFrame = subFrame;
         searchFrame = searchSubFrame;
      }
      else
      {
         lumMin = lumLimit;
      }
   }

   bestFrame.y = texInfo->height - 1 - bestFrame.y - bestFrame.w;

   return bestFrame;
}

}
