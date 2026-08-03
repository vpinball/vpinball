// license:GPLv3+

#pragma once

#include <future>

#include "common.h"
#include "plugins/ResURIResolver.h"

namespace B2S {

class B2SDMDOverlay final
{
public:
   B2SDMDOverlay(ResURIResolver& resURIResolver, VPXTexture& dmdTex, VPXTexture backImage);
   ~B2SDMDOverlay();
   void Render(VPXRenderContext2D* context);

   static void RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId);
   void LoadSettings(bool isScoreView);

   void UpdateBackgroundImage(VPXTexture backImage);

private:
   ivec4 SearchDmdSubFrame(VPXTexture image, float dmdAspectRatio) const;

   ResURIResolver& m_resURIResolver;
   VPXTexture& m_dmdTex;
   // Gates the per-frame display pull so unchanged frames are not re-uploaded
   // into m_dmdTex at render rate. Per-overlay even though m_dmdTex is shared.
   ResURIResolver::DisplayUploadGate m_displayUploadGate;

   ivec4 m_frame;
   bool m_enable = false;

   bool m_detectDmdFrame = false;
   VPXTexture m_backImage = nullptr;
   CtlResId m_detectSrcId {};
   bool m_stopSearching = false;
   std::future<ivec4> m_frameSearch;
};

}
