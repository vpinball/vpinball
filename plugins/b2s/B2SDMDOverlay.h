// license:GPLv3+

#pragma once

#include <future>

#include "common.h"
#include "core/ResURIResolver.h"

namespace B2S {

class B2SDMDOverlay final
{
public:
   B2SDMDOverlay(ResURIResolver& resURIResolver, VPXTexture& dmdTex, VPXTexture backImage);
   void LoadSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId, bool isScoreView);
   void Render(VPXRenderContext2D* context);

private:
   ivec4 SearchDmdSubFrame(VPXTexture image, float dmdAspectRatio) const;

   ResURIResolver& m_resURIResolver;
   VPXTexture& m_dmdTex;

   ivec4 m_frame;
   bool m_enable = false;

   bool m_detectDmdFrame = false;
   VPXTexture m_backImage;
   CtlResId m_detectSrcId {};
   std::future<ivec4> m_frameSearch;

   MSGPI_BOOL_SETTING(m_scoreViewDMDOverlayProp, "ScoreViewDMDOverlay", "ScoreView DMD Overlay", "Enable a DMD overlay on the Score View", true, false);
   MSGPI_BOOL_SETTING(m_scoreViewDMDAutoPosProp, "ScoreViewDMDAutoPos", "ScoreView DMD Automatic position", "Enable automatic DMD bounds detection", true, false);
   MSGPI_INT_SETTING(m_scoreViewDMDXProp, "ScoreViewDMDX", "ScoreView DMD X position", "DMD overlay X position", true, 0, 0xFFFF, 0);
   MSGPI_INT_SETTING(m_scoreViewDMDYProp, "ScoreViewDMDY", "ScoreView DMD Y position", "DMD overlay Y position", true, 0, 0xFFFF, 0);
   MSGPI_INT_SETTING(m_scoreViewDMDWProp, "ScoreViewDMDW", "ScoreView DMD width", "DMD overlay width", true, 0, 0xFFFF, 0);
   MSGPI_INT_SETTING(m_scoreViewDMDHProp, "ScoreViewDMDH", "ScoreView DMD height", "DMD overlay height", true, 0, 0xFFFF, 0);

   MSGPI_BOOL_SETTING(m_backglassDMDOverlayProp, "BackglassDMDOverlay", "Backglass DMD Overlay", "Enable a DMD overlay on the Backglass", true, false);
   MSGPI_BOOL_SETTING(m_backglassDMDAutoPosProp, "BackglassDMDAutoPos", "Backglass DMD Automatic position", "Enable automatic DMD bounds detection", true, false);
   MSGPI_INT_SETTING(m_backglassDMDXProp, "BackglassDMDX", "Backglass DMD X position", "DMD overlay X position", true, 0, 0xFFFF, 0);
   MSGPI_INT_SETTING(m_backglassDMDYProp, "BackglassDMDY", "Backglass DMD Y position", "DMD overlay Y position", true, 0, 0xFFFF, 0);
   MSGPI_INT_SETTING(m_backglassDMDWProp, "BackglassDMDW", "Backglass DMD width", "DMD overlay width", true, 0, 0xFFFF, 0);
   MSGPI_INT_SETTING(m_backglassDMDHProp, "BackglassDMDH", "Backglass DMD height", "DMD overlay height", true, 0, 0xFFFF, 0);
};

}
