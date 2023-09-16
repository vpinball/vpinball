#include "stdafx.h"

#include "B2SScreen.h"
#include "../forms/FormBackglass.h"
#include "../forms/FormDMD.h"
#include "../controls/B2SPictureBox.h"
#include "../controls/B2SReelBox.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"

B2SScreen::B2SScreen(B2SData* pB2SData)
{
   m_pFormBackglass = NULL;
   m_pFormDMD = NULL;
   m_playfieldSize = { 0, 0, 0, 0 };
   m_backglassMonitor = "";
   m_backglassSize = { 0, 0, 0, 0 };
   m_backglassLocation = { 0, 0 };
   m_backglassGrillHeight = 0;
   m_backglassSmallGrillHeight = 0;
   m_dmdSize = { 0, 0, 0, 0 };
   m_dmdLocation = { 0, 0 };
   m_dmdFlipY = false;
   m_dmdAtDefaultLocation = true;
   m_dmdViewMode = eDMDViewMode_NotDefined;
   m_backgroundSize = { 0, 0, 0, 0 };
   m_backgroundLocation = { 0, 0 };
   m_backgroundPath = "";
   m_backglassCutOff = { 0, 0, 0, 0 };
   m_dmdToBeShown = false;
   m_startBackground = false;

   m_pB2SData = pB2SData;

   m_pSurface = NULL;

   ReadB2SSettingsFromFile();
}

B2SScreen::~B2SScreen()
{
   if (m_pSurface)
      SDL_FreeSurface(m_pSurface);
}

void B2SScreen::Start(FormBackglass* pFormBackglass, int backglassGrillHeight, int smallBackglassGrillHeight)
{
   m_pFormBackglass = pFormBackglass;

   Show();
}

void B2SScreen::ReadB2SSettingsFromFile()
{
   m_backglassLocation = { 
      LoadValueWithDefault(regKey[RegName::Standalone], "B2SX"s, 0), 
      LoadValueWithDefault(regKey[RegName::Standalone], "B2SY"s, 0) };
  
   m_backglassSize = { 0, 0,
      LoadValueWithDefault(regKey[RegName::Standalone], "B2SWidth"s, 0), 
      LoadValueWithDefault(regKey[RegName::Standalone], "B2SHeight"s, 0) };
}

void B2SScreen::GetB2SSettings()
{
}

void B2SScreen::Show()
{
   // set forms to background image size
   if (m_pFormBackglass->GetBackgroundImage()) {
      m_pFormBackglass->SetSize( { 0, 0, m_pFormBackglass->GetBackgroundImage()->w, m_pFormBackglass->GetBackgroundImage()->h});
   }

   // calculate backglass rescale factors
   float rescaleBackglassX = (float)m_pFormBackglass->GetWidth() / (float)m_backglassSize.w;
   float rescaleBackglassY = (float)m_pFormBackglass->GetHeight() / (float)m_backglassSize.h;

   float rescaleDMDX = 1;
   float rescaleDMDY = 1;

   ScaleAllControls(rescaleBackglassX, rescaleBackglassY, rescaleDMDX, rescaleDMDY);

   m_pSurface = SDL_CreateRGBSurfaceWithFormat(0, m_backglassSize.w, m_backglassSize.h, 32, SDL_PIXELFORMAT_RGBA32);

   g_pplayer->SetB2SRenderCallback(std::bind(&B2SScreen::Render, this, std::placeholders::_1));
}

void B2SScreen::Render(SDL_Renderer* pRenderer)
{
   m_pFormBackglass->OnPaint(m_pSurface);

   SDL_Texture* pTexture = SDL_CreateTextureFromSurface(pRenderer, m_pSurface);

   SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
   SDL_RenderClear(pRenderer);

   if (pTexture) {
      SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);
      SDL_RenderPresent(pRenderer);
   }
}

void B2SScreen::ScaleAllControls(float rescaleX, float rescaleY, float rescaleDMDX, float rescaleDMDY)
{
   // get scale info for all picked objects and scale some of them
   for(const auto& [key, pReelbox] : *m_pB2SData->GetReels()) {
      bool isOnDMD = (pReelbox->GetParent() && pReelbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pReelbox, rescaleX, rescaleY, isOnDMD);
   }
   for(const auto& [key, pLedbox] : *m_pB2SData->GetLEDs()) {
      bool isOnDMD = (pLedbox->GetParent() && pLedbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pLedbox, rescaleX, rescaleY, isOnDMD);
   }
   for(const auto& [key, pDream7Display] : *m_pB2SData->GetLEDDisplays()) {
      bool isOnDMD = (pDream7Display->GetParent() && pDream7Display->GetParent()->GetName() == "formDMD");
      ScaleControl(pDream7Display, rescaleX, rescaleY, isOnDMD);
   }
   for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
      bool isOnDMD = (pPicbox->GetParent() && pPicbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pPicbox, rescaleX, rescaleY, isOnDMD);
   }
   for(const auto& [key, pPicbox] : *m_pB2SData->GetDMDIlluminations()) {
      bool isOnDMD = (pPicbox->GetParent() && pPicbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pPicbox, rescaleX, rescaleY, isOnDMD);
   }

   // get scale info for the LED areas
   for (const auto& [key, pLedArea] : *m_pB2SData->GetLEDAreas()) {
      if (pLedArea->IsOnDMD()) {
         if (m_pFormDMD) {
            float y = pLedArea->GetRect().y / rescaleDMDY;
            if (m_dmdFlipY && !m_dmdAtDefaultLocation) {
               y = m_pFormDMD->GetHeight() / rescaleDMDY - y - pLedArea->GetRect().h / rescaleDMDY;
            }
            pLedArea->SetRect({ (int)(pLedArea->GetRect().x / rescaleDMDX), (int)y, (int)(pLedArea->GetRect().w / rescaleDMDX), (int)(pLedArea->GetRect().h / rescaleDMDY) });
         }
      }
      else {
          pLedArea->SetRect({ (int)(pLedArea->GetRect().x / rescaleDMDX), (int)(pLedArea->GetRect().y / rescaleDMDY), (int)(pLedArea->GetRect().w / rescaleDMDX), (int)(pLedArea->GetRect().h / rescaleDMDY) });
      }
   }

   // and now recalc the backglass cut off rectangle
   if (!SDL_RectEmpty(&m_backglassCutOff)) {
      m_backglassCutOff = { (int)(m_backglassCutOff.x / rescaleX), (int)(m_backglassCutOff.y / rescaleY),
       (int)(m_backglassCutOff.w / rescaleX), (int)(m_backglassCutOff.h / rescaleY) };
   }
}

void B2SScreen::ScaleControl(B2SBaseBox* pControl, float rescaleX, float rescaleY, bool isOnDMD, bool flipY)
{
   if (!isOnDMD && !SDL_RectEmpty(&m_backglassCutOff)) {
      if (SDL_HasIntersection(&m_backglassCutOff, &pControl->GetRect()))
         pControl->SetRectangleF({ 0, 0, 0, 0 });
      else if (m_backglassCutOff.y < pControl->GetTop())
         pControl->SetRectangleF({ pControl->GetLeft() / rescaleX, (pControl->GetTop() - m_backglassSmallGrillHeight) / rescaleY, pControl->GetWidth() / rescaleX, pControl->GetHeight() / rescaleY });
      else
         pControl->SetRectangleF({ pControl->GetLeft() / rescaleX, pControl->GetTop() / rescaleY, pControl->GetWidth() / rescaleX, pControl->GetHeight() / rescaleY });
   }
   else
      pControl->SetRectangleF({ (float)pControl->GetLeft() / rescaleX, (float)pControl->GetTop() / rescaleY, (float)pControl->GetWidth() / rescaleX, (float)pControl->GetHeight() / rescaleY });

   // scale not more than the LED and reel boxes
   if (dynamic_cast<B2SLEDBox*>(pControl) || dynamic_cast<B2SReelBox*>(pControl)) {
       pControl->SetLocation({ (int)pControl->GetRectangleF().x, (int)pControl->GetRectangleF().y });
       pControl->SetSize({ 0, 0, (int)pControl->GetRectangleF().w, (int)pControl->GetRectangleF().h });
   }

   // maybe flip DMD images
   if (flipY && m_pFormDMD) {
      if (dynamic_cast<B2SLEDBox*>(pControl)) {
      }
      else {
         B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pControl);
         if (pPicbox) {
            // set new top location
            float newY = m_pFormDMD->GetHeight() / rescaleY - pPicbox->GetRectangleF().y - pPicbox->GetRectangleF().h;
            pPicbox->SetRectangleF({ pPicbox->GetRectangleF().x, newY, pPicbox->GetRectangleF().w, pPicbox->GetRectangleF().h });
            // flip the images
            if (pPicbox->GetBackgroundImage()) {
               // picbox.BackgroundImage.RotateFlip(RotateFlipType.RotateNoneFlipY)
            }
            if (pPicbox->GetOffImage()) {
               // picbox.OffImage.RotateFlip(RotateFlipType.RotateNoneFlipY)
            }
         }
      }
   }
}

void B2SScreen::ScaleControl(Dream7Display* pControl, float rescaleX, float rescaleY, bool isOnDMD, bool flipY)
{
   // store current LED location
   SDL_FRect rectF = { (float)pControl->GetLeft() / rescaleX, (float)pControl->GetTop() / rescaleY, (float)pControl->GetWidth() / rescaleX, (float)pControl->GetHeight() / rescaleY };

   // scale LED display
    pControl->SetLocation({ (int)(pControl->GetLeft() / rescaleX), (int)(pControl->GetTop() / rescaleY) });
    pControl->SetSize({ 0, 0, (int)(pControl->GetWidth() / rescaleX), (int)(pControl->GetHeight() / rescaleY) });
    pControl->SetSpacing(pControl->GetSpacing() / rescaleX);
    pControl->SetOffsetWidth(pControl->GetOffsetWidth() / rescaleX);
    pControl->SetThickness(pControl->GetThickness() / rescaleX);

    // maybe flip LEDs
    if (flipY) {
        pControl->SetMirrored(true);
        // set new top location
        if (m_pFormDMD) {
            float newY = m_pFormDMD->GetHeight() / rescaleY - rectF.y - rectF.h;
            pControl->SetLocation({ (int)rectF.x, (int)newY });
        }
    }
}
