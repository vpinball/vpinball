#pragma once

#include "../controls/Control.h"
#include "../DMDOverlay/DMDOverlay.h"
#include "plugins/ResURIResolver.h"

namespace B2SLegacy {

class B2SData;
class Server;

class Form : public Control
{
public:
   Form(VPXPluginAPI* vpxApi, const MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData, const string& overlayType = ""s);
   ~Form() override;

   void Show();
   void Hide();
   void SetTopMost(bool topMost) { m_topMost = topMost; }
   bool IsTopMost() const { return m_topMost; }
   B2SData* GetB2SData() const { return m_pB2SData; }
   void OnPaint(VPXRenderContext2D* const ctx) override;

   // Document space of the authored <DMD> displays: the artwork texture at its native resolution
   // (the displayed background may be grill-cut and resized) and the document band that was cut
   // away from it (empty when the whole artwork is rendered)
   virtual VPXTexture GetOverlayDocImage() const { return GetBackgroundImage(); }
   virtual SDL_Rect GetOverlayDocCut() const { return { 0, 0, 0, 0 }; }

protected:
   const MsgPluginAPI* m_msgApi = nullptr;
   B2SData* m_pB2SData = nullptr;
   uint32_t m_endpointId = 0;

private:
   PinballPlugin::ResURIResolver* m_pResURIResolver = nullptr;
   vector<std::unique_ptr<DMDOverlay::Renderer>> m_dmdOverlays; // One per <DMD> display of the edited document, in enumeration order
   int m_dmdDisplayTarget = 0; // View rendered by this form: 0 = backglass, 1 = score view
   bool m_topMost = false;
};

}
