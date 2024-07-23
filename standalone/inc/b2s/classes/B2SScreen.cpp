#include "stdafx.h"

#include "B2SScreen.h"
#include "../forms/Form.h"
#include "../forms/FormBackglass.h"
#include "../forms/FormDMD.h"
#include "../controls/B2SPictureBox.h"
#include "../controls/B2SReelBox.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"

B2SScreen::B2SScreen()
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

   m_pB2SData = B2SData::GetInstance();
   m_pB2SSettings = B2SSettings::GetInstance();

   ReadB2SSettingsFromFile();
}

B2SScreen::~B2SScreen()
{
}

void B2SScreen::Start(Form* pFormBackglass)
{
   // here we go with one form for the backglass and no DMD and no grill
   Start(pFormBackglass, NULL, { 0, 0 }, eDMDViewMode_NoDMD, 0, 0);

   Show();
}

void B2SScreen::Start(Form* pFormBackglass, int backglassGrillHeight, int smallBackglassGrillHeight)
{
   // here we go with one form for the backglass and no DMD but with grill
   Start(pFormBackglass, NULL, { 0, 0 }, eDMDViewMode_NoDMD, backglassGrillHeight, smallBackglassGrillHeight);
}

void B2SScreen::Start(Form* pFormBackglass, Form* pFormDMD, SDL_Point defaultDMDLocation)
{
   // here we go with two forms for the backglass and DMD and default values for the DMD view mode and the grill height
   Start(pFormBackglass, pFormDMD, defaultDMDLocation, eDMDViewMode_ShowDMD, 0, 0);
}

void B2SScreen::Start(Form* pFormBackglass, Form* pFormDMD, SDL_Point defaultDMDLocation, eDMDViewMode dmdViewMode, int backglassGrillHeight, int smallBackglassGrillHeight)
{
   // here we go with one or two forms for the backglass and the DMD
   m_pFormBackglass = pFormBackglass;
   m_pFormDMD = pFormDMD;

   // get all backglass settings
   GetB2SSettings(defaultDMDLocation, dmdViewMode, backglassGrillHeight, smallBackglassGrillHeight);

   // show all the backglass stuff
   Show();
}

void B2SScreen::ReadB2SSettingsFromFile()
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   m_backglassSize = { 0, 0,
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassWidth"s, B2S_SETTINGS_BACKGLASSWIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassHeight"s, B2S_SETTINGS_BACKGLASSHEIGHT) };

   m_backglassLocation = { 
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassX"s, B2S_SETTINGS_BACKGLASSX),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassY"s, B2S_SETTINGS_BACKGLASSY) };
  
   m_dmdSize = { 0, 0,
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDWidth"s, B2S_SETTINGS_DMDWIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDHeight"s, B2S_SETTINGS_DMDHEIGHT) };

   m_dmdLocation = {
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDX"s, B2S_SETTINGS_DMDX),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDY"s, B2S_SETTINGS_DMDY) };

   m_dmdFlipY = pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDMDFlipY"s, false);
}

void B2SScreen::GetB2SSettings(SDL_Point defaultDMDLocation, eDMDViewMode dmdViewMode, int backglassGrillHeight, int backglassSmallGrillHeight)
{
   FormBackglass* pFormBackglass = (FormBackglass*)m_pFormBackglass;

   m_dmdViewMode = dmdViewMode;

   // show or do not show the grill and do some more DMD stuff
   bool showTheGrill = (m_dmdLocation.x == 0 && m_dmdLocation.y == 0);
   if (m_pB2SSettings->GetHideGrill() == B2SSettingsCheckedState_Unchecked)
      showTheGrill = true;
   else if (m_pB2SSettings->GetHideGrill() == B2SSettingsCheckedState_Checked)
      showTheGrill = false;

   if (showTheGrill) {
      // show the grill
      m_dmdAtDefaultLocation = true;

      // use default values for DMD location
      if ((defaultDMDLocation.x != 0 || defaultDMDLocation.y != 0) && m_dmdViewMode != eDMDViewMode_DoNotShowDMDAtDefaultLocation)
         m_dmdLocation = defaultDMDLocation;

      // maybe do some corrections since there is a small grill
      if (backglassSmallGrillHeight > 0 && pFormBackglass->GetDarkImage()) {
         if (m_dmdLocation.y > 0)
            m_dmdLocation.y -= backglassSmallGrillHeight;

         m_backglassSmallGrillHeight = backglassSmallGrillHeight;

         m_backglassCutOff = { 0, pFormBackglass->GetDarkImage()->h - backglassGrillHeight - backglassSmallGrillHeight, pFormBackglass->GetDarkImage()->w, backglassSmallGrillHeight };

         // shrink some images to remove the small grill
         pFormBackglass->SetDarkImage4Authentic(CutOutImage(pFormBackglass->GetDarkImage4Authentic(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetTopLightImage4Authentic())
            pFormBackglass->SetTopLightImage4Authentic(CutOutImage(pFormBackglass->GetTopLightImage4Authentic(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetSecondLightImage4Authentic())
            pFormBackglass->SetSecondLightImage4Authentic(CutOutImage(pFormBackglass->GetSecondLightImage4Authentic(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetTopAndSecondLightImage4Authentic())
            pFormBackglass->SetTopAndSecondLightImage4Authentic(CutOutImage(pFormBackglass->GetTopAndSecondLightImage4Authentic(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetDarkImage4Fantasy())
            pFormBackglass->SetDarkImage4Fantasy(CutOutImage(pFormBackglass->GetDarkImage4Fantasy(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetTopLightImage4Fantasy())
            pFormBackglass->SetTopLightImage4Fantasy(CutOutImage(pFormBackglass->GetTopLightImage4Fantasy(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetSecondLightImage4Fantasy())
            pFormBackglass->SetSecondLightImage4Fantasy(CutOutImage(pFormBackglass->GetSecondLightImage4Fantasy(), backglassGrillHeight, backglassSmallGrillHeight));
         if (pFormBackglass->GetTopAndSecondLightImage4Fantasy())
            pFormBackglass->SetTopAndSecondLightImage4Fantasy(CutOutImage(pFormBackglass->GetTopAndSecondLightImage4Fantasy(), backglassGrillHeight, backglassSmallGrillHeight));

         // set background image and new backglass form height (without grill)
         pFormBackglass->SetBackgroundImage(pFormBackglass->GetDarkImage());
         pFormBackglass->SetSize({ 0, 0, pFormBackglass->GetDarkImage()->w, pFormBackglass->GetDarkImage()->h});
      }
   }
   else {
      // do not show the grill (if possible)
      m_dmdAtDefaultLocation = false;

      // maybe hide grill
      if (backglassGrillHeight > 0 && pFormBackglass->GetDarkImage()) {
         m_backglassGrillHeight = backglassGrillHeight;
         m_backglassSmallGrillHeight = backglassSmallGrillHeight;

         m_backglassCutOff = { 0, pFormBackglass->GetDarkImage()->h - backglassGrillHeight, pFormBackglass->GetDarkImage()->w, backglassGrillHeight };

         // shrink some images to remove the grill
         pFormBackglass->SetDarkImage4Authentic(ResizeImage(pFormBackglass->GetDarkImage4Authentic(), backglassGrillHeight));
         if (pFormBackglass->GetTopLightImage4Authentic())
            pFormBackglass->SetTopLightImage4Authentic(ResizeImage(pFormBackglass->GetTopLightImage4Authentic(), backglassGrillHeight));
         if (pFormBackglass->GetSecondLightImage4Authentic())
            pFormBackglass->SetSecondLightImage4Authentic(ResizeImage(pFormBackglass->GetSecondLightImage4Authentic(), backglassGrillHeight));
         if (pFormBackglass->GetTopAndSecondLightImage4Authentic())
            pFormBackglass->SetTopAndSecondLightImage4Authentic(ResizeImage(pFormBackglass->GetTopAndSecondLightImage4Authentic(), backglassGrillHeight));
         if (pFormBackglass->GetDarkImage4Fantasy())
            pFormBackglass->SetDarkImage4Fantasy(ResizeImage(pFormBackglass->GetDarkImage4Fantasy(), backglassGrillHeight));
         if (pFormBackglass->GetTopLightImage4Fantasy())
            pFormBackglass->SetTopLightImage4Fantasy(ResizeImage(pFormBackglass->GetTopLightImage4Fantasy(), backglassGrillHeight));
         if (pFormBackglass->GetSecondLightImage4Fantasy())
            pFormBackglass->SetSecondLightImage4Fantasy(ResizeImage(pFormBackglass->GetSecondLightImage4Fantasy(), backglassGrillHeight));
         if (pFormBackglass->GetTopAndSecondLightImage4Fantasy())
            pFormBackglass->SetTopAndSecondLightImage4Fantasy(ResizeImage(pFormBackglass->GetTopAndSecondLightImage4Fantasy(), backglassGrillHeight));

         // set background image and new backglass form height (without grill)
         pFormBackglass->SetBackgroundImage(pFormBackglass->GetDarkImage());
         pFormBackglass->SetSize({ 0, 0, pFormBackglass->GetDarkImage()->w, pFormBackglass->GetDarkImage()->h});
      }

      // maybe rotate DMD image
      if (m_dmdFlipY && m_pFormDMD && m_pFormDMD->GetBackgroundImage())
         m_pFormDMD->SetBackgroundImage(FlipImage(m_pFormDMD->GetBackgroundImage()));
   }
}

void B2SScreen::Show()
{
   // first of all get the info whether the DMD is to be shown or not

   m_dmdToBeShown = (m_pFormDMD && !SDL_RectEmpty(&m_dmdSize) && !(m_dmdLocation.x == 0 && m_dmdLocation.y == 0) &&
        ((m_dmdViewMode == eDMDViewMode_ShowDMD) ||
         (m_dmdViewMode == eDMDViewMode_ShowDMDOnlyAtDefaultLocation && m_dmdAtDefaultLocation) ||
         (m_dmdViewMode == eDMDViewMode_DoNotShowDMDAtDefaultLocation && !m_dmdAtDefaultLocation)));

   //  set forms to background image size
   if (m_pFormBackglass && m_pFormBackglass->GetBackgroundImage())
      m_pFormBackglass->SetSize({ 0, 0, m_pFormBackglass->GetBackgroundImage()->w, m_pFormBackglass->GetBackgroundImage()->h});
   if (m_pFormDMD && m_pFormDMD->GetBackgroundImage())
      m_pFormDMD->SetSize({ 0, 0, m_pFormDMD->GetBackgroundImage()->w, m_pFormDMD->GetBackgroundImage()->h});

   // calculate backglass rescale factors
   float rescaleBackglassX = (float)m_pFormBackglass->GetWidth() / (float)m_backglassSize.w;
   float rescaleBackglassY = (float)m_pFormBackglass->GetHeight() / (float)m_backglassSize.h;
   if (m_pFormBackglass->GetBackgroundImage()) {
      rescaleBackglassX = (float)m_pFormBackglass->GetBackgroundImage()->w / (float)m_backglassSize.w;
      rescaleBackglassY = (float)m_pFormBackglass->GetBackgroundImage()->h / (float)m_backglassSize.h;
   }

   // maybe rescale the location and the size because this is the default and therefore it has to be done
   float rescaleDMDX = 1.0f;
   float rescaleDMDY = 1.0f;
   if (m_dmdToBeShown) {
      if (m_dmdAtDefaultLocation) {
         m_dmdSize = m_pFormDMD->GetSize();
         if (rescaleBackglassX != 1.0f || rescaleBackglassY != 1.0f) {
            m_dmdLocation = { (int)(m_dmdLocation.x * rescaleBackglassX), (int)(m_dmdLocation.y * rescaleBackglassY) };
            m_dmdSize = { 0, 0, (int)(m_dmdSize.w * rescaleBackglassX), (int)(m_dmdSize.h * rescaleBackglassY) };
         }
      }

      // calculate DMD rescale factors
      rescaleDMDX = (float)m_pFormDMD->GetWidth() / (float)m_dmdSize.w;
      rescaleDMDY = (float)m_pFormDMD->GetHeight() / (float)m_dmdSize.h;
      if (m_pFormDMD->GetBackgroundImage()) {
         rescaleDMDX = (float)m_pFormDMD->GetBackgroundImage()->w / (float)m_dmdSize.w;
         rescaleDMDY = (float)m_pFormDMD->GetBackgroundImage()->h / (float)m_dmdSize.h;
      }
   }

   // move and scale all picked objects
   ScaleAllControls(rescaleBackglassX, rescaleBackglassY, rescaleDMDX, rescaleDMDY);

   // show the backglass form
   m_pFormBackglass->SetSize(m_backglassSize);
   m_pFormBackglass->Show();

   // bring backglass screen to the front
   if (m_pB2SSettings->IsFormToFront())
      m_pFormBackglass->SetTopMost(true);
   m_pFormBackglass->BringToFront();

   // maybe show DMD form
   if (m_dmdToBeShown) {
      m_pFormDMD->SetSize(m_dmdSize);
      // show the DMD form
      m_pFormDMD->Show();
      m_pFormDMD->BringToFront();
      m_pFormDMD->SetTopMost(true);
   }
}

void B2SScreen::ScaleAllControls(float rescaleX, float rescaleY, float rescaleDMDX, float rescaleDMDY)
{
   // get scale info for all picked objects and scale some of them
   for(const auto& [key, pReelbox] : *m_pB2SData->GetReels()) {
      bool isOnDMD = (pReelbox->GetParent() && pReelbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pReelbox, isOnDMD ? rescaleDMDX : rescaleX, isOnDMD ? rescaleDMDY : rescaleY, isOnDMD);
   }
   for(const auto& [key, pLedbox] : *m_pB2SData->GetLEDs()) {
      bool isOnDMD = (pLedbox->GetParent() && pLedbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pLedbox, isOnDMD ? rescaleDMDX : rescaleX, isOnDMD ? rescaleDMDY : rescaleY, isOnDMD, isOnDMD && m_dmdFlipY && !m_dmdAtDefaultLocation);
   }
   for(const auto& [key, pDream7Display] : *m_pB2SData->GetLEDDisplays()) {
      bool isOnDMD = (pDream7Display->GetParent() && pDream7Display->GetParent()->GetName() == "formDMD");
      ScaleControl(pDream7Display, isOnDMD ? rescaleDMDX : rescaleX, isOnDMD ? rescaleDMDY : rescaleY, isOnDMD, isOnDMD && m_dmdFlipY && !m_dmdAtDefaultLocation);
   }
   for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
      bool isOnDMD = (pPicbox->GetParent() && pPicbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pPicbox, rescaleX, rescaleY, isOnDMD);
   }
   for(const auto& [key, pPicbox] : *m_pB2SData->GetDMDIlluminations()) {
      bool isOnDMD = (pPicbox->GetParent() && pPicbox->GetParent()->GetName() == "formDMD");
      ScaleControl(pPicbox, rescaleDMDX, rescaleDMDY, isOnDMD, m_dmdFlipY && !m_dmdAtDefaultLocation);
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
          pLedArea->SetRect({ (int)(pLedArea->GetRect().x / rescaleX), (int)(pLedArea->GetRect().y / rescaleY), (int)(pLedArea->GetRect().w / rescaleX), (int)(pLedArea->GetRect().h / rescaleY) });
      }
   }

   // and now recalc the backglass cut off rectangle
   if (!SDL_RectEmpty(&m_backglassCutOff)) {
      m_backglassCutOff = { (int)(roundf(m_backglassCutOff.x / rescaleX)), (int)(roundf(m_backglassCutOff.y / rescaleY)),
       (int)(roundf(m_backglassCutOff.w / rescaleX)), (int)(roundf(m_backglassCutOff.h / rescaleY)) };
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
      pControl->SetRectangleF({ pControl->GetLeft() / rescaleX, pControl->GetTop() / rescaleY, pControl->GetWidth() / rescaleX, pControl->GetHeight() / rescaleY });

   // scale not more than the LED and reel boxes
   if (dynamic_cast<B2SLEDBox*>(pControl) || dynamic_cast<B2SReelBox*>(pControl)) {
       pControl->SetLocation({ (int)pControl->GetRectangleF().x, (int)pControl->GetRectangleF().y });
       pControl->SetSize({ 0, 0, (int)pControl->GetRectangleF().w, (int)pControl->GetRectangleF().h });
       B2SLEDBox* pLedBox = dynamic_cast<B2SLEDBox*>(pControl);
       if (pLedBox)
          pLedBox->OnResize();
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
            if (pPicbox->GetBackgroundImage())
               pPicbox->SetBackgroundImage(FlipImage(pPicbox->GetBackgroundImage()));
            if (pPicbox->GetOffImage())
               pPicbox->SetOffImage(FlipImage(pPicbox->GetOffImage()));
         }
      }
   }
}

void B2SScreen::ScaleControl(Dream7Display* pControl, float rescaleX, float rescaleY, bool isOnDMD, bool flipY)
{
   // store current LED location
   SDL_FRect rectF = { pControl->GetLeft() / rescaleX, pControl->GetTop() / rescaleY, pControl->GetWidth() / rescaleX, pControl->GetHeight() / rescaleY };

   // scale LED display
   pControl->SetLocation({ (int)(pControl->GetLeft() / rescaleX), (int)(pControl->GetTop() / rescaleY) });
   pControl->SetSize({ 0, 0, (int)(pControl->GetWidth() / rescaleX), (int)(pControl->GetHeight() / rescaleY) });
   pControl->SetSpacing(pControl->GetSpacing() / rescaleX);
   pControl->SetOffsetWidth((int)(pControl->GetOffsetWidth() / rescaleX));
   pControl->SetThickness(pControl->GetThickness() / rescaleX);
   pControl->SegmentDisplaySizeChanged();

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

SDL_Surface* B2SScreen::CutOutImage(SDL_Surface* pSourceImage, int grillheight, int smallgrillheight)
{
    SDL_Surface* pImageWithoutSmallGrill = SDL_CreateRGBSurface(0, pSourceImage->w, pSourceImage->h - smallgrillheight, pSourceImage->format->BitsPerPixel,
        pSourceImage->format->Rmask, pSourceImage->format->Gmask, pSourceImage->format->Bmask, pSourceImage->format->Amask);

    SDL_Surface* pImageBackglass = PartFromImage(pSourceImage, { 0, 0, pSourceImage->w, pSourceImage->h - grillheight - smallgrillheight });
    SDL_Surface* pImageGrill = PartFromImage(pSourceImage, { 0, pSourceImage->h - grillheight, pSourceImage->w, grillheight });

    SDL_BlitSurface(pImageBackglass, NULL, pImageWithoutSmallGrill, NULL);

    SDL_Rect dest = { 0, pImageBackglass->h, pImageGrill->w, pImageGrill->h };
    SDL_BlitSurface(pImageGrill, NULL, pImageWithoutSmallGrill, &dest);

    SDL_FreeSurface(pImageBackglass);
    SDL_FreeSurface(pImageGrill);

    return pImageWithoutSmallGrill;
}

SDL_Surface* B2SScreen::PartFromImage(SDL_Surface* pSourceImage, SDL_Rect rect)
{
   SDL_Surface* pImagePart = SDL_CreateRGBSurface(0, rect.w, rect.h, pSourceImage->format->BitsPerPixel,
      pSourceImage->format->Rmask, pSourceImage->format->Gmask, pSourceImage->format->Bmask, pSourceImage->format->Amask);

   if (!pImagePart)
      return NULL;

   SDL_BlitSurface(pSourceImage, &rect, pImagePart, NULL);

   return pImagePart;
}

SDL_Surface* B2SScreen::ResizeImage(SDL_Surface* pSourceImage, int grillheight)
{
   SDL_Surface* pImageWithoutGrill = SDL_CreateRGBSurface(0, pSourceImage->w, pSourceImage->h - grillheight, pSourceImage->format->BitsPerPixel,
      pSourceImage->format->Rmask, pSourceImage->format->Gmask, pSourceImage->format->Bmask, pSourceImage->format->Amask);

   if (!pImageWithoutGrill)
      return NULL;

   SDL_BlitSurface(pSourceImage, NULL, pImageWithoutGrill, NULL);

   return pImageWithoutGrill;
}

SDL_Surface* B2SScreen::FlipImage(SDL_Surface* pSourceImage)
{
   SDL_Surface* pFlippedImage = SDL_CreateRGBSurface(0, pSourceImage->w, pSourceImage->h, pSourceImage->format->BitsPerPixel,
      pSourceImage->format->Rmask, pSourceImage->format->Gmask, pSourceImage->format->Bmask, pSourceImage->format->Amask);

   if (!pFlippedImage)
      return NULL;

   SDL_LockSurface(pSourceImage);
   SDL_LockSurface(pFlippedImage);

   int bpp = pSourceImage->format->BytesPerPixel;
   for (int y = 0; y < pSourceImage->h; ++y) {
      for (int x = 0; x < pSourceImage->w; ++x) {
         UINT8* src_pixel = (UINT8*)pSourceImage->pixels + y * pSourceImage->pitch + x * bpp;
         UINT8* dst_pixel = (UINT8*)pFlippedImage->pixels + (pFlippedImage->h - y - 1) * pFlippedImage->pitch + x * bpp;
         memcpy(dst_pixel, src_pixel, bpp);
      }
   }

   SDL_UnlockSurface(pSourceImage);
   SDL_UnlockSurface(pFlippedImage);

   return pFlippedImage;
}