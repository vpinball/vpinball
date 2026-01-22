#include "../common.h"
#include <cmath>

#include "B2SScreen.h"
#include "../utils/VPXGraphics.h"
#include "../forms/Form.h"
#include "../forms/FormBackglass.h"
#include "../forms/FormDMD.h"
#include "../controls/B2SPictureBox.h"
#include "../controls/B2SReelBox.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"
#include "../classes/LEDAreaInfo.h"

namespace B2SLegacy {

MSGPI_INT_VAL_SETTING(backglassWidthProp, "B2SBackglassWidth", "B2SBackglassWidth", "", true, 0, 16384, 1024);
MSGPI_INT_VAL_SETTING(backglassHeightProp, "B2SBackglassHeight", "B2SBackglassHeight", "", true, 0, 16384, 768);
MSGPI_INT_VAL_SETTING(backglassXProp, "BackglassX", "BackglassX", "", true, 0, 16384, 0);
MSGPI_INT_VAL_SETTING(backglassYProp, "BackglassY", "BackglassY", "", true, 0, 16384, 0);

MSGPI_INT_VAL_SETTING(dmdWidthProp, "B2SDMDWidth", "B2SDMDWidth", "", true, 0, 16384, 512);
MSGPI_INT_VAL_SETTING(dmdHeightProp, "B2SDMDHeight", "B2SDMDHeight", "", true, 0, 16384, 128);
MSGPI_INT_VAL_SETTING(dmdXProp, "DMDX", "DMDX", "", true, 0, 16384, 0);
MSGPI_INT_VAL_SETTING(dmdYProp, "DMDY", "DMDY", "", true, 0, 16384, 0);
MSGPI_BOOL_VAL_SETTING(dmdFlipYProp, "B2SDMDFlipY", "B2SDMDFlipY", "", true, false);

B2SScreen::B2SScreen(B2SData* pB2SData, MsgPluginAPI* msgApi, VPXPluginAPI* vpxApi, unsigned int endpointId)
   : m_pB2SData(pB2SData),
     m_msgApi(msgApi),
     m_vpxApi(vpxApi),
     m_endpointId(endpointId),
     m_pB2SSettings(pB2SData->GetB2SSettings())
{
   m_pFormBackglass = nullptr;
   m_pFormDMD = nullptr;
   m_playfieldSize = { 0, 0, 0, 0 };
   m_backglassMonitor.clear();
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
   m_backgroundPath.clear();
   m_backglassCutOff = { 0, 0, 0, 0 };
   m_dmdToBeShown = false;
   m_rescaleBackglass = { 0, 0, 1.0f, 1.0f };

   ReadB2SSettingsFromFile();
}

B2SScreen::~B2SScreen()
{
}

void B2SScreen::Start(Form* pFormBackglass)
{
   // here we go with one form for the backglass and no DMD and no grill
   Start(pFormBackglass, nullptr, { 0, 0 }, eDMDViewMode_NoDMD, 0, 0);

   Show();
}

void B2SScreen::Start(Form* pFormBackglass, int backglassGrillHeight, int smallBackglassGrillHeight)
{
   // here we go with one form for the backglass and no DMD but with grill
   Start(pFormBackglass, nullptr, { 0, 0 }, eDMDViewMode_NoDMD, backglassGrillHeight, smallBackglassGrillHeight);
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
   m_msgApi->RegisterSetting(m_endpointId, &backglassWidthProp);
   m_msgApi->RegisterSetting(m_endpointId, &backglassHeightProp);
   m_msgApi->RegisterSetting(m_endpointId, &backglassXProp);
   m_msgApi->RegisterSetting(m_endpointId, &backglassYProp);

   m_backglassSize = { 0, 0, backglassWidthProp_Val, backglassHeightProp_Val };
   m_backglassLocation = { backglassXProp_Val, backglassYProp_Val };

   m_msgApi->RegisterSetting(m_endpointId, &dmdWidthProp);
   m_msgApi->RegisterSetting(m_endpointId, &dmdHeightProp);
   m_msgApi->RegisterSetting(m_endpointId, &dmdXProp);
   m_msgApi->RegisterSetting(m_endpointId, &dmdYProp);
   m_msgApi->RegisterSetting(m_endpointId, &dmdFlipYProp);

   m_dmdSize = { 0, 0, dmdWidthProp_Val, dmdHeightProp_Val };
   m_dmdLocation = { dmdXProp_Val, dmdYProp_Val };

   m_dmdFlipY = dmdFlipYProp_Val;
}

void B2SScreen::GetB2SSettings(SDL_Point defaultDMDLocation, eDMDViewMode dmdViewMode, int backglassGrillHeight, int backglassSmallGrillHeight)
{
   FormBackglass* pFormBackglass = (FormBackglass*)m_pFormBackglass;

   m_dmdViewMode = dmdViewMode;

   // VB uses CheckedState_Indeterminate
   // show or do not show the grill and do some more DMD stuff
   // bool showTheGrill = (m_dmdLocation.x == 0 && m_dmdLocation.y == 0);
   // if (GetHideGrill() == CheckState.Unchecked) showTheGrill = true;
   // else if (GetHideGrill() == CheckState.Checked) showTheGrill = false;
   bool showTheGrill = !m_pB2SSettings->IsHideGrill();

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

         m_backglassCutOff = { 0, static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->height - backglassGrillHeight - backglassSmallGrillHeight), static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->width), backglassSmallGrillHeight };

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
         pFormBackglass->SetSize({ 0, 0, static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->width), static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->height) });
      }
   }
   else {
      // do not show the grill (if possible)
      m_dmdAtDefaultLocation = false;

      // maybe hide grill
      if (backglassGrillHeight > 0 && pFormBackglass->GetDarkImage()) {
         m_backglassGrillHeight = backglassGrillHeight;
         m_backglassSmallGrillHeight = backglassSmallGrillHeight;

         m_backglassCutOff = { 0, static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->height - backglassGrillHeight), static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->width), backglassGrillHeight };

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
         pFormBackglass->SetSize({ 0, 0, static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->width), static_cast<int>(m_vpxApi->GetTextureInfo(pFormBackglass->GetDarkImage())->height) });
      }

      // maybe rotate DMD image
      if (m_dmdFlipY && m_pFormDMD && m_pFormDMD->GetBackgroundImage()) {
         m_pFormDMD->SetBackgroundImage(FlipImage(m_pFormDMD->GetBackgroundImage()));
      }
   }
}

void B2SScreen::Show()
{
   // first of all get the info whether the DMD is to be shown or not
   m_dmdToBeShown = (m_pFormDMD && !SDL_RectEmpty(&m_dmdSize) && /* FIXME !(m_dmdLocation.x == 0 && m_dmdLocation.y == 0) && */
        ((m_dmdViewMode == eDMDViewMode_ShowDMD) ||
         (m_dmdViewMode == eDMDViewMode_ShowDMDOnlyAtDefaultLocation && m_dmdAtDefaultLocation) ||
         (m_dmdViewMode == eDMDViewMode_DoNotShowDMDAtDefaultLocation && !m_dmdAtDefaultLocation)));

   //  set forms to background image size
   if (m_pFormBackglass && m_pFormBackglass->GetBackgroundImage())
      m_pFormBackglass->SetSize({ 0, 0, static_cast<int>(m_vpxApi->GetTextureInfo(m_pFormBackglass->GetBackgroundImage())->width), static_cast<int>(m_vpxApi->GetTextureInfo(m_pFormBackglass->GetBackgroundImage())->height) });

   if (m_pFormDMD && m_pFormDMD->GetBackgroundImage())
      m_pFormDMD->SetSize({ 0, 0, static_cast<int>(m_vpxApi->GetTextureInfo(m_pFormDMD->GetBackgroundImage())->width), static_cast<int>(m_vpxApi->GetTextureInfo(m_pFormDMD->GetBackgroundImage())->height) });

   // calculate backglass rescale factors
   if (m_pFormBackglass->GetBackgroundImage()) {
      m_rescaleBackglass = { 0, 0,
         (float)m_vpxApi->GetTextureInfo(m_pFormBackglass->GetBackgroundImage())->width / (float)m_backglassSize.w,
         (float)m_vpxApi->GetTextureInfo(m_pFormBackglass->GetBackgroundImage())->height / (float)m_backglassSize.h };
   }
   else {
      m_rescaleBackglass = { 0, 0,
         (float)m_pFormBackglass->GetWidth() / (float)m_backglassSize.w,
         (float)m_pFormBackglass->GetHeight() / (float)m_backglassSize.h };
   }

   // maybe rescale the location and the size because this is the default and therefore it has to be done
   float rescaleDMDX = 1.0f;
   float rescaleDMDY = 1.0f;
   if (m_dmdToBeShown) {
      if (m_dmdAtDefaultLocation) {
         m_dmdSize = m_pFormDMD->GetSize();
         if (m_rescaleBackglass.w != 1.0f || m_rescaleBackglass.h != 1.0f) {
            m_dmdLocation = { (int)((float)m_dmdLocation.x * m_rescaleBackglass.w), (int)((float)m_dmdLocation.y * m_rescaleBackglass.h) };
            m_dmdSize = { 0, 0, (int)((float)m_dmdSize.w * m_rescaleBackglass.w), (int)((float)m_dmdSize.h * m_rescaleBackglass.h) };
         }
      }

      // calculate DMD rescale factors
      rescaleDMDX = (float)m_pFormDMD->GetWidth() / (float)m_dmdSize.w;
      rescaleDMDY = (float)m_pFormDMD->GetHeight() / (float)m_dmdSize.h;
      if (m_pFormDMD->GetBackgroundImage()) {
         rescaleDMDX = (float)m_vpxApi->GetTextureInfo(m_pFormDMD->GetBackgroundImage())->width / (float)m_dmdSize.w;
         rescaleDMDY = (float)m_vpxApi->GetTextureInfo(m_pFormDMD->GetBackgroundImage())->height / (float)m_dmdSize.h;
      }
   }

   // move and scale all picked objects
   ScaleAllControls(m_rescaleBackglass.w, m_rescaleBackglass.h, rescaleDMDX, rescaleDMDY);

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
            float y = (float)pLedArea->GetRect().y / rescaleDMDY;
            if (m_dmdFlipY && !m_dmdAtDefaultLocation) {
               y = (float)m_pFormDMD->GetHeight() / rescaleDMDY - y - (float)pLedArea->GetRect().h / rescaleDMDY;
            }
            pLedArea->SetRect({ (int)((float)pLedArea->GetRect().x / rescaleDMDX), (int)y, (int)((float)pLedArea->GetRect().w / rescaleDMDX), (int)((float)pLedArea->GetRect().h / rescaleDMDY) });
         }
      }
      else {
          pLedArea->SetRect({ (int)((float)pLedArea->GetRect().x / rescaleX), (int)((float)pLedArea->GetRect().y / rescaleY), (int)((float)pLedArea->GetRect().w / rescaleX), (int)((float)pLedArea->GetRect().h / rescaleY) });
      }
   }

   // and now recalc the backglass cut off rectangle
   if (!SDL_RectEmpty(&m_backglassCutOff)) {
      m_backglassCutOff = { (int)(roundf((float)m_backglassCutOff.x / rescaleX)), (int)(roundf((float)m_backglassCutOff.y / rescaleY)),
       (int)(roundf((float)m_backglassCutOff.w / rescaleX)), (int)(roundf((float)m_backglassCutOff.h / rescaleY)) };
   }
}

void B2SScreen::ScaleControl(B2SBaseBox* pControl, float rescaleX, float rescaleY, bool isOnDMD, bool flipY)
{
   if (!isOnDMD && !SDL_RectEmpty(&m_backglassCutOff)) {
      if (SDL_HasRectIntersection(&m_backglassCutOff, &pControl->GetRect()))
         pControl->SetRectangleF({ 0, 0, 0, 0 });
      else if (m_backglassCutOff.y < pControl->GetTop())
         pControl->SetRectangleF({ (float)pControl->GetLeft() / rescaleX, (float)(pControl->GetTop() - m_backglassSmallGrillHeight) / rescaleY, (float)pControl->GetWidth() / rescaleX, (float)pControl->GetHeight() / rescaleY });
      else
         pControl->SetRectangleF({ (float)pControl->GetLeft() / rescaleX, (float)pControl->GetTop() / rescaleY, (float)pControl->GetWidth() / rescaleX, (float)pControl->GetHeight() / rescaleY });
   }
   else
      pControl->SetRectangleF({ (float)pControl->GetLeft() / rescaleX, (float)pControl->GetTop() / rescaleY, (float)pControl->GetWidth() / rescaleX, (float)pControl->GetHeight() / rescaleY });

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
            float newY = (float)m_pFormDMD->GetHeight() / rescaleY - pPicbox->GetRectangleF().y - pPicbox->GetRectangleF().h;
            pPicbox->SetRectangleF({ pPicbox->GetRectangleF().x, newY, pPicbox->GetRectangleF().w, pPicbox->GetRectangleF().h });
            // flip the images
            if (pPicbox->GetBackgroundImage()) {
               pPicbox->SetBackgroundImage(FlipImage(pPicbox->GetBackgroundImage()));
            }
            if (pPicbox->GetOffImage()) {
               pPicbox->SetOffImage(FlipImage(pPicbox->GetOffImage()));
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
   pControl->SetLocation({ (int)((float)pControl->GetLeft() / rescaleX), (int)((float)pControl->GetTop() / rescaleY) });
   pControl->SetSize({ 0, 0, (int)((float)pControl->GetWidth() / rescaleX), (int)((float)pControl->GetHeight() / rescaleY) });
   pControl->SetSpacing(pControl->GetSpacing() / rescaleX);
   pControl->SetOffsetWidth((int)((float)pControl->GetOffsetWidth() / rescaleX));
   pControl->SetThickness(pControl->GetThickness() / rescaleX);
   pControl->SegmentDisplaySizeChanged();

   // maybe flip LEDs
   if (flipY) {
      pControl->SetMirrored(true);
      // set new top location
      if (m_pFormDMD) {
         float newY = (float)m_pFormDMD->GetHeight() / rescaleY - rectF.y - rectF.h;
         pControl->SetLocation({ (int)rectF.x, (int)newY });
      }
   }
}

VPXTexture B2SScreen::CutOutImage(VPXTexture pSourceImage, int grillheight, int smallgrillheight)
{
   if (!pSourceImage)
      return nullptr;

   SDL_Surface* sourceSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pSourceImage);
   if (!sourceSurface)
      return nullptr;

   SDL_Surface* pImageWithoutSmallGrill = SDL_CreateSurface(sourceSurface->w, sourceSurface->h - smallgrillheight, sourceSurface->format);
   if (!pImageWithoutSmallGrill) {
      SDL_DestroySurface(sourceSurface);
      return nullptr;
   }

   SDL_Rect backglassRect = { 0, 0, sourceSurface->w, sourceSurface->h - grillheight - smallgrillheight };
   SDL_Surface* pImageBackglass = SDL_CreateSurface(backglassRect.w, backglassRect.h, sourceSurface->format);
   if (pImageBackglass) {
      SDL_BlitSurface(sourceSurface, &backglassRect, pImageBackglass, NULL);
      SDL_BlitSurface(pImageBackglass, NULL, pImageWithoutSmallGrill, NULL);
   }

   SDL_Rect grillRect = { 0, sourceSurface->h - grillheight, sourceSurface->w, grillheight };
   SDL_Surface* pImageGrill = SDL_CreateSurface(grillRect.w, grillRect.h, sourceSurface->format);
   if (pImageGrill) {
      SDL_BlitSurface(sourceSurface, &grillRect, pImageGrill, NULL);
      SDL_Rect dest = { 0, pImageBackglass ? pImageBackglass->h : 0, pImageGrill->w, pImageGrill->h };
      SDL_BlitSurface(pImageGrill, NULL, pImageWithoutSmallGrill, &dest);
   }

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, pImageWithoutSmallGrill);

   SDL_DestroySurface(sourceSurface);
   SDL_DestroySurface(pImageWithoutSmallGrill);
   if (pImageBackglass) SDL_DestroySurface(pImageBackglass);
   if (pImageGrill) SDL_DestroySurface(pImageGrill);

   return result;
}

VPXTexture B2SScreen::PartFromImage(VPXTexture pSourceImage, SDL_Rect rect)
{
   if (!pSourceImage)
      return nullptr;

   SDL_Surface* sourceSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pSourceImage);
   if (!sourceSurface)
      return nullptr;

   SDL_Surface* partSurface = SDL_CreateSurface(rect.w, rect.h, sourceSurface->format);
   if (!partSurface) {
      SDL_DestroySurface(sourceSurface);
      return nullptr;
   }

   SDL_BlitSurface(sourceSurface, &rect, partSurface, NULL);

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, partSurface);

   SDL_DestroySurface(sourceSurface);
   SDL_DestroySurface(partSurface);

   return result;
}

VPXTexture B2SScreen::ResizeImage(VPXTexture pSourceImage, int grillheight)
{
   if (!pSourceImage)
      return nullptr;

   SDL_Surface* sourceSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pSourceImage);
   if (!sourceSurface)
      return nullptr;

   SDL_Surface* resizedSurface = SDL_CreateSurface(sourceSurface->w, sourceSurface->h - grillheight, sourceSurface->format);
   if (!resizedSurface) {
      SDL_DestroySurface(sourceSurface);
      return nullptr;
   }

   SDL_BlitSurface(sourceSurface, NULL, resizedSurface, NULL);

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, resizedSurface);

   SDL_DestroySurface(sourceSurface);
   SDL_DestroySurface(resizedSurface);

   return result;
}

VPXTexture B2SScreen::FlipImage(VPXTexture pSourceImage)
{
   if (!pSourceImage)
      return nullptr;

   SDL_Surface* sourceSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pSourceImage);
   if (!sourceSurface)
      return nullptr;

   SDL_Surface* flippedSurface = SDL_CreateSurface(sourceSurface->w, sourceSurface->h, sourceSurface->format);
   if (!flippedSurface) {
      SDL_DestroySurface(sourceSurface);
      return nullptr;
   }

   SDL_LockSurface(sourceSurface);
   SDL_LockSurface(flippedSurface);

   const int bpp = SDL_GetPixelFormatDetails(sourceSurface->format)->bytes_per_pixel;
   for (int y = 0; y < sourceSurface->h; ++y) {
      uint8_t* src_pixel = (uint8_t*)sourceSurface->pixels + y * sourceSurface->pitch;
      uint8_t* dst_pixel = (uint8_t*)flippedSurface->pixels + (flippedSurface->h - y - 1) * flippedSurface->pitch;
      memcpy(dst_pixel, src_pixel, sourceSurface->w * bpp);
   }

   SDL_UnlockSurface(sourceSurface);
   SDL_UnlockSurface(flippedSurface);

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, flippedSurface);

   SDL_DestroySurface(sourceSurface);
   SDL_DestroySurface(flippedSurface);

   return result;
}

}
