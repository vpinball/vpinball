// license:GPLv3+

#pragma once

#include "plugins/ResURIResolver.h"
#include "plugins/VPXPlugin.h"

#include <future>


namespace DMDOverlay
{

class DMDOverlay final
{
public:
   DMDOverlay(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver, VPXTexture& dmdTex, VPXTexture backImage);
   ~DMDOverlay();
   void Render(VPXRenderContext2D* context);

   static void RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId);
   void LoadSettings(bool isScoreView);

   void UpdateBackgroundImage(VPXTexture backImage);

private:
   template <class T> class vec4 final
   {
   public:
      constexpr vec4() { }
      constexpr vec4(T px, T py, T pz, T pw)
         : x(px)
         , y(py)
         , z(pz)
         , w(pw)
      {
      }

      T x = static_cast<T>(0), y = static_cast<T>(0), z = static_cast<T>(0), w = static_cast<T>(0);
   };

   vec4<int> SearchDmdSubFrame(VPXTexture image, const VPXTextureInfo* const texInfo, float dmdAspectRatio) const;

   PinballPlugin::ResURIResolver& m_resURIResolver;
   VPXTexture& m_dmdTex;
   const VPXPluginAPI* const m_vpxApi;

   vec4<int> m_frame;
   bool m_enable = false;

   bool m_detectDmdFrame = false;
   VPXTexture m_backImage = nullptr;
   CtlResId m_detectSrcId { };
   bool m_stopSearching = false;
   std::future<vec4<int>> m_frameSearch;

   // Identity of the frame last uploaded to the texture, to avoid a full copy plus a GPU re-upload every frame
   DisplaySrcId m_uploadedSrc { };
   unsigned int m_uploadedFrameId = 0;
   bool m_hasUploadedFrame = false;
};

}
