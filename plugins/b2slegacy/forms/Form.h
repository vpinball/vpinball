#pragma once

#include "../controls/Control.h"
#include "core/ResURIResolver.h"

namespace B2SLegacy {

class B2SData;
class Server;
class DMDOverlay;

class Form : public Control
{
public:
   Form(VPXPluginAPI* vpxApi, MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData, const string& overlayType = "");
   ~Form();

   void Show();
   void Hide();
   void SetTopMost(bool topMost) { m_topMost = topMost; }
   bool IsTopMost() const { return m_topMost; }
   B2SData* GetB2SData() const { return m_pB2SData; }
   void OnPaint(VPXRenderContext2D* const ctx) override;

protected:
   MsgPluginAPI* m_msgApi = nullptr;
   B2SData* m_pB2SData = nullptr;
   uint32_t m_endpointId = 0;

private:
   ResURIResolver* m_pResURIResolver = nullptr;
   VPXTexture m_dmdTex = nullptr;
   DMDOverlay* m_pDmdOverlay = nullptr;
   bool m_topMost = false;
};

}
