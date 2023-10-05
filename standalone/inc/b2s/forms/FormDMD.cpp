#include "stdafx.h"

#include "FormDMD.h"
#include "../controls/B2SPictureBox.h"

FormDMD::FormDMD()
{
   SetName("formDMD");

   m_pB2SData = B2SData::GetInstance();
}

FormDMD::~FormDMD()
{
}

void FormDMD::OnPaint(VP::Graphics* pGraphics)
{
   if (IsVisible() && GetBackgroundImage()) {
      // draw background image
      pGraphics->DrawImage(GetBackgroundImage(), NULL, NULL);

      if (m_pB2SData->GetDMDIlluminations()->size() > 0) {
         if (!m_pB2SData->IsUseZOrder()) {
            // draw all standard images
            for(const auto& [key, pIllu] : *m_pB2SData->GetDMDIlluminations()) {
               if (pIllu->IsVisible()) {
                    SDL_Rect rect = { (int)pIllu->GetRectangleF().x, (int)pIllu->GetRectangleF().y, (int)pIllu->GetRectangleF().w, (int)pIllu->GetRectangleF().h };
                    pGraphics->DrawImage(pIllu->GetBackgroundImage(), NULL, &rect);
               }
            }
         }
         else {
            // first of all draw zorderd images
            for(const auto& [key, pIllus] : *m_pB2SData->GetZOrderDMDImages()) {
               for (int i = 0; i < pIllus.size(); i++) {
                  if (pIllus[i]->IsVisible()) {
                     SDL_Rect rect = { (int)pIllus[i]->GetRectangleF().x, (int)pIllus[i]->GetRectangleF().y, (int)pIllus[i]->GetRectangleF().w, (int)pIllus[i]->GetRectangleF().h };
                     pGraphics->DrawImage(pIllus[i]->GetBackgroundImage(), NULL, &rect);
                  }
               }
            }
            // now draw all standard images
            for(const auto& [key, pIllu] : *m_pB2SData->GetDMDIlluminations()) {
               if (pIllu->IsVisible() && pIllu->GetZOrder() == 0) {
                  SDL_Rect rect = { (int)pIllu->GetRectangleF().x, (int)pIllu->GetRectangleF().y, (int)pIllu->GetRectangleF().w, (int)pIllu->GetRectangleF().h };
                  pGraphics->DrawImage(pIllu->GetBackgroundImage(), NULL, &rect);
               }
            }
         }
      }
   }

   Control::OnPaint(pGraphics);
}