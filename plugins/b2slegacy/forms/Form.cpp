#include "../common.h"

#include "Form.h"
#include "../utils/DMDOverlay.h"
#include "../Server.h"

namespace B2SLegacy {

Form::Form(VPXPluginAPI* vpxApi, MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData, const string& overlayType) 
   : Control(vpxApi), 
     m_msgApi(msgApi), 
     m_endpointId(endpointId),
     m_pB2SData(pB2SData)
{
   if (!overlayType.empty()) {
      m_pResURIResolver = new ResURIResolver(*msgApi, m_endpointId, true, false, false, false);
      m_dmdTex = nullptr;

      m_pDmdOverlay = new DMDOverlay(*m_pResURIResolver, m_dmdTex, nullptr, m_vpxApi);
      m_pDmdOverlay->LoadSettings(overlayType == "ScoreView");
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
      m_pDmdOverlay->UpdateBackgroundImage(GetBackgroundImage());
      m_pDmdOverlay->Render(ctx);
   }
   Control::OnPaint(ctx);
}

}
