// license:GPLv3+

#pragma once

#include "common.h"

#include "plugins/ResURIResolver.h"
#include "plugins/PagedUIPlugin.h"
#include "plugins/VPXPlugin.h"

#include <future>


namespace DMDOverlay
{

class DMDOverlay final
{
public:
   DMDOverlay(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver, VPXTexture& dmdTex);
   ~DMDOverlay();
   // Renders one <DMD> display of the document (bounds in B2S document space); backImage is the
   // artwork the 'detect position' bounds detection searches in
   void Render(VPXRenderContext2D* context, B2S::B2SDMDDisplay* display, VPXTexture backImage);

   // HideDMD plugin setting override: -1 = no override, 0 = force visible, 1 = force hidden
   void SetEnableOverride(int enableOverride) { m_enableOverride = enableOverride; }

   // Document editing page (see plugins/PagedUIPlugin.h): fills the sheet callbacks editing the
   // given document (sheet 'context' will be the doc pointer). CommitEdit rewrites the document's
   // <DMDOverlay> element. The caller still defines the sheet name/description.
   static void InitEditSheetDef(PagedUISheetDef* sheet, B2S::B2SOverlayDoc* doc);

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
   int m_enableOverride = -1;

   bool m_stopSearching = false;
   std::future<vec4<int>> m_frameSearch;

   // Identity of the frame last uploaded to the texture, to avoid a full copy plus a GPU re-upload every frame
   DisplaySrcId m_uploadedSrc { };
   unsigned int m_uploadedFrameId = 0;
   bool m_hasUploadedFrame = false;
};

}
