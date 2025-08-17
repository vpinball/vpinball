#include "../common.h"

#include "FormDMD.h"
#include "../Server.h"
#include "../controls/B2SPictureBox.h"
#include "../utils/VPXGraphics.h"
#include "../utils/DMDOverlay.h"
#include "LoggingPlugin.h"

namespace B2SLegacy {

FormDMD::FormDMD(VPXPluginAPI* vpxApi, MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData)
   : Form(vpxApi, msgApi, endpointId, pB2SData, "Scoreview")
{
   SetName("formDMD");
}

FormDMD::~FormDMD()
{
}

void FormDMD::OnPaint(VPXRenderContext2D* const ctx)
{
   if (IsVisible() && GetBackgroundImage()) {
      VPXGraphics::DrawImage(m_vpxApi, ctx, GetBackgroundImage(), NULL, NULL);

      if (!m_pB2SData->GetDMDIlluminations()->empty()) {
         if (!m_pB2SData->IsUseZOrder()) {
            for(const auto& [key, pIllu] : *m_pB2SData->GetDMDIlluminations()) {
               if (pIllu->IsVisible()) {
                  SDL_Rect rect = { (int)pIllu->GetRectangleF().x, (int)pIllu->GetRectangleF().y, (int)pIllu->GetRectangleF().w, (int)pIllu->GetRectangleF().h };
                  VPXGraphics::DrawImage(m_vpxApi, ctx, pIllu->GetBackgroundImage(), NULL, &rect);
               }
            }
         }
         else {
            for(const auto& [key, pIllus] : *m_pB2SData->GetZOrderDMDImages()) {
               for (int i = 0; i < (int)pIllus.size(); i++) {
                  if (pIllus[i]->IsVisible()) {
                     SDL_Rect rect = { (int)pIllus[i]->GetRectangleF().x, (int)pIllus[i]->GetRectangleF().y, (int)pIllus[i]->GetRectangleF().w, (int)pIllus[i]->GetRectangleF().h };
                     VPXGraphics::DrawImage(m_vpxApi, ctx, pIllus[i]->GetBackgroundImage(), NULL, &rect);
                  }
               }
            }
            for(const auto& [key, pIllu] : *m_pB2SData->GetDMDIlluminations()) {
               if (pIllu->IsVisible() && pIllu->GetZOrder() == 0) {
                  SDL_Rect rect = { (int)pIllu->GetRectangleF().x, (int)pIllu->GetRectangleF().y, (int)pIllu->GetRectangleF().w, (int)pIllu->GetRectangleF().h };
                  VPXGraphics::DrawImage(m_vpxApi, ctx, pIllu->GetBackgroundImage(), NULL, &rect);
               }
            }
         }
      }
   }

   Form::OnPaint(ctx);
}


}
