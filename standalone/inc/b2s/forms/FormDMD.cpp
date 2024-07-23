#include "stdafx.h"

#include "FormDMD.h"
#include "FormWindow.h"
#include "../controls/B2SPictureBox.h"

#include "../../common/WindowManager.h"

FormDMD::FormDMD()
{
   SetName("formDMD");

   m_pB2SData = B2SData::GetInstance();

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "B2SWindows"s, true)) {
      PLOGI.printf("B2S DMD window disabled");
      return;
   }

   m_pWindow = new FormWindow(this, "B2SDMD",
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDX"s, B2S_SETTINGS_DMDX),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDY"s, B2S_SETTINGS_DMDY),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDWidth"s, B2S_SETTINGS_DMDWIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDHeight"s, B2S_SETTINGS_DMDHEIGHT),
      B2S_DMD_ZORDER,
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDRotation"s, 0));
}

FormDMD::~FormDMD()
{
}

void FormDMD::OnPaint(VP::RendererGraphics* pGraphics)
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