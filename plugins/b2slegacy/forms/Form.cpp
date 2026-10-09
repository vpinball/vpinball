#include "../common.h"

#include "Form.h"
#include "../utils/DMDOverlay.h"
#include "../Server.h"

namespace B2SLegacy {

Form::Form(VPXPluginAPI* vpxApi, const MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData, const string& overlayType) 
   : Control(vpxApi), 
     m_msgApi(msgApi), 
     m_endpointId(endpointId),
     m_pB2SData(pB2SData)
{
   if (!overlayType.empty()) {
      m_pResURIResolver = new PinballPlugin::ResURIResolver(*msgApi, m_endpointId, true, false, false);
      m_isScoreView = overlayType == "ScoreView";
      m_pDmdOverlay = new DMDOverlay::DMDOverlay(m_vpxApi, *m_pResURIResolver, m_dmdTex, nullptr);
   }
}

Form::~Form()
{
   delete m_pDmdOverlay;
   delete m_pResURIResolver;
   if (m_dmdTex && m_vpxApi)
      m_vpxApi->DeleteTexture(m_dmdTex);
}

void Form::Show()
{
}

void Form::Hide()
{
}

void Form::OnPaint(VPXRenderContext2D* const ctx)
{
   if (m_pDmdOverlay) {
      m_pDmdOverlay->LoadSettings(m_isScoreView);
      m_pDmdOverlay->UpdateBackgroundImage(GetBackgroundImage());
      m_pDmdOverlay->Render(ctx);
   }
   Control::OnPaint(ctx);
}

}
