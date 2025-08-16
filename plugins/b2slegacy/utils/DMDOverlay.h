#pragma once

#include <future>

#include "../common.h"
#include "core/ResURIResolver.h"

namespace B2SLegacy {
   
class DMDOverlay final
{
public:
   DMDOverlay(ResURIResolver& resURIResolver, VPXTexture& dmdTex, VPXTexture backImage, VPXPluginAPI* vpxApi);
   void LoadSettings(MsgPluginAPI* const msgApi, const string& pluginId, const string& prefix);
   void Render(VPXRenderContext2D* context);
   void UpdateBackgroundImage(VPXTexture backImage);

private:
   ivec4 SearchDmdSubFrame(VPXTexture image, float dmdAspectRatio);

   ResURIResolver& m_resURIResolver;
   VPXTexture& m_dmdTex;
   VPXPluginAPI* m_vpxApi;

   bool m_enable = false;
   ivec4 m_frame;

   bool m_detectDmdFrame = false;
   VPXTexture m_backImage;
   CtlResId m_detectSrcId { 0 };
   std::future<ivec4> m_frameSearch;
};

}