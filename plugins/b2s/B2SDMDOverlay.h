// license:GPLv3+

#pragma once

#include <future>

#include "common.h"
#include "ResURIResolver.h"

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
};

}
