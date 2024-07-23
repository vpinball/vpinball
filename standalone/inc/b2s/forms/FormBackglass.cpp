#include "stdafx.h"

#include "FormBackglass.h"
#include "FormDMD.h"
#include "FormWindow.h"
#include "../classes/B2SScreen.h"
#include "../classes/B2SAnimation.h"
#include "../classes/AnimationInfo.h"
#include "../classes/B2SReelDisplay.h"
#include "../classes/LEDDisplayDigitLocation.h"
#include "../classes/PictureBoxAnimationEntry.h"
#include "../controls/B2SPictureBox.h"
#include "../controls/B2SLEDBox.h"
#include "../controls/B2SReelBox.h"
#include "../classes/ControlInfo.h"
#include "../dream7/Dream7Display.h"
#include "../../common/WindowManager.h"

#include <SDL2/SDL_image.h>
#include "tinyxml2/tinyxml2.h"

FormBackglass::FormBackglass()
{
   SetName("formBackglass"s);

   m_pB2SSettings = B2SSettings::GetInstance();
   m_pB2SData = B2SData::GetInstance();
   m_pFormDMD = NULL;
   m_pStartupTimer = NULL;
   m_pRotateTimer = NULL;
   m_rotateSlowDownSteps = 0;
   m_rotateRunTillEnd = false;
   m_rotateRunToFirstStep = false;
   m_rotateAngle = 0;
   m_rotateTimerInterval = 0;
   m_selectedLEDType = eLEDTypes_Undefined;
   m_pDarkImage4Authentic = NULL;
   m_pDarkImage4Fantasy = NULL;
   m_pTopLightImage4Authentic = NULL;
   m_pTopLightImage4Fantasy = NULL;
   m_pSecondLightImage4Authentic = NULL;
   m_pSecondLightImage4Fantasy = NULL;
   m_pTopAndSecondLightImage4Authentic = NULL;
   m_pTopAndSecondLightImage4Fantasy = NULL;
   m_topRomID4Authentic = 0;
   m_topRomIDType4Authentic = eRomIDType_NotDefined;
   m_topRomInverted4Authentic = false;
   m_secondRomID4Authentic = 0;
   m_secondRomIDType4Authentic = eRomIDType_NotDefined;
   m_secondRomInverted4Authentic = false;
   m_topRomID4Fantasy = 0;
   m_topRomIDType4Fantasy = eRomIDType_NotDefined;
   m_topRomInverted4Fantasy = false;
   m_secondRomID4Fantasy = 0;
   m_secondRomIDType4Fantasy = eRomIDType_NotDefined;
   m_secondRomInverted4Fantasy = false;
   m_pB2SAnimation = new B2SAnimation();
   m_pB2SScreen = new B2SScreen();

   // load settings
   m_pB2SSettings->Load();

   // get B2S xml and start
   LoadB2SData();

   if (!m_pB2SData->IsValid())
      return;

   // initialize screen settings
   InitB2SScreen();

   // resize images
   ResizeSomeImages();

   // show startup snippits
   ShowStartupSnippits();

   // create 'image on' timer and start it
   m_pStartupTimer = new VP::Timer(2000, std::bind(&FormBackglass::StartupTimerTick, this, std::placeholders::_1));
   m_pStartupTimer->Start();

   // create rotation timer
   m_pRotateTimer = new VP::Timer(m_rotateTimerInterval, std::bind(&FormBackglass::RotateTimerTick, this, std::placeholders::_1));

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "B2SWindows"s, true)) {
      PLOGI.printf("B2S Backglass window disabled");
      return; 
   }

   m_pWindow = new FormWindow(this, "B2SBackglass"s,
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassX"s, B2S_SETTINGS_BACKGLASSX),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassY"s, B2S_SETTINGS_BACKGLASSY),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassWidth"s, B2S_SETTINGS_BACKGLASSWIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassHeight"s, B2S_SETTINGS_BACKGLASSHEIGHT),
      B2S_BACKGLASS_ZORDER,
      pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassRotation"s, 0));
}

FormBackglass::~FormBackglass()
{
    delete m_pStartupTimer;
    delete m_pRotateTimer;
    delete m_pB2SAnimation;
    delete m_pFormDMD;
    delete m_pB2SScreen;
}

void FormBackglass::OnPaint(VP::RendererGraphics* pGraphics)
{
   if (m_pB2SSettings->IsHideB2SBackglass()) {
      Hide();
      return;
   }

   if (IsVisible() && GetBackgroundImage()) {
      // draw background image
      pGraphics->DrawImage(GetBackgroundImage(), NULL, NULL);
      // draw all visible and necessary images
      if (m_pB2SData->GetIlluminations()->size() > 0) {
         if (!m_pB2SData->IsUseZOrder()) {
            // draw all standard images
            for(const auto& [key, pIllu] : *m_pB2SData->GetIlluminations())
               DrawImage(pGraphics, pIllu);
         }
         else {
            // first of all draw all standard images
            for(const auto& [key, pIllu] : *m_pB2SData->GetIlluminations()) {
               if (pIllu->GetZOrder() == 0)
                  DrawImage(pGraphics, pIllu);
            }
            // now draw z-ordered images
            for(const auto& [key, pIllus] : *m_pB2SData->GetZOrderImages()) {
               for (int i = 0; i < pIllus.size(); i++)
                  DrawImage(pGraphics, pIllus[i]);
            }
         }
      }
   }

   Control::OnPaint(pGraphics);
}

void FormBackglass::DrawImage(VP::RendererGraphics* pGraphics, B2SPictureBox* pPicbox)
{
   if (pPicbox) {
      SDL_FRect rectf = pPicbox->GetRectangleF();
      SDL_Rect rect = { (int)rectf.x, (int)rectf.y, (int)rectf.w, (int)rectf.h };

      bool drawme = (!m_pB2SSettings->IsAllOut() && pPicbox->IsVisible());
      if (drawme && !SDL_RectEmpty(&m_pB2SScreen->GetBackglassCutOff()))
         drawme = !SDL_HasIntersection(&m_pB2SScreen->GetBackglassCutOff(), &rect);
      if (drawme && pPicbox->GetRomID() != 0 && !pPicbox->IsSetThruAnimation())
         drawme = (pPicbox->GetRomID() != GetTopRomID() || pPicbox->GetRomIDType() != GetTopRomIDType() || pPicbox->IsRomInverted() != IsTopRomInverted()) && (pPicbox->GetRomID() != GetSecondRomID() || pPicbox->GetRomIDType() != GetSecondRomIDType() || pPicbox->IsRomInverted() != IsSecondRomInverted());
      if (drawme && m_pB2SData->IsDualBackglass())
         drawme = (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == (eDualMode)m_pB2SSettings->GetCurrentDualMode());

      if (drawme) {
         if (m_pB2SData->IsOnAndOffImage()) {
            if (m_pB2SData->IsOffImageVisible() && pPicbox->GetOffImage())
               pGraphics->DrawImage(pPicbox->GetOffImage(), NULL, &rect);
            else
               pGraphics->DrawImage(pPicbox->GetBackgroundImage(), NULL, &rect);
         }
         else
            pGraphics->DrawImage(pPicbox->GetBackgroundImage(), NULL, &rect);
      }
   }
}

void FormBackglass::StartupTimerTick(VP::Timer* pTimer)
{
   m_pStartupTimer->Stop();

   // maybe show some 'startup on' images
   ShowStartupImages();

   // start autostarted animations
   m_pB2SAnimation->AutoStart();
}

void FormBackglass::RotateTimerTick(VP::Timer* pTimer)
{
   static int currentAngleS = 0;

   auto rotatingImages = m_pB2SData->GetRotatingImages();
   if (rotatingImages && (*rotatingImages)[0].find(currentAngleS) != (*rotatingImages)[0].end()) {
      auto rotatingPictureBox = m_pB2SData->GetRotatingPictureBox();
      if (rotatingPictureBox) {
         auto& pPicBox = (*rotatingPictureBox)[0];
         pPicBox->SetBackgroundImage((*rotatingImages)[0][currentAngleS]);
         pPicBox->SetVisible(true);
      }
      currentAngleS += m_rotateAngle;
      if (currentAngleS >= 360)
         currentAngleS = 0;
   }
   else {
      m_pRotateTimer->Stop();
   }

   if (m_rotateSlowDownSteps > 0) {
       int lastStep = 25;
       int add2IntervalPerStep = 3;

       if (m_rotateSlowDownSteps >= lastStep) {
          m_pRotateTimer->Stop();
          m_rotateSlowDownSteps = 0;
          m_pRotateTimer->SetInterval(m_rotateTimerInterval);
       }
       else {
          m_rotateSlowDownSteps++;
          m_pRotateTimer->SetInterval(m_pRotateTimer->GetInterval() + add2IntervalPerStep);
       }
   }
   else if (m_rotateRunTillEnd) {
      if (currentAngleS + m_rotateAngle >= 360)
         m_pRotateTimer->Stop();
   }
   else if (m_rotateRunToFirstStep) {
      if (currentAngleS == 0)
         m_pRotateTimer->Stop();
   }
}

void FormBackglass::StartAnimation(const string& szName, bool playReverse)
{
   m_pB2SAnimation->StartAnimation(szName, playReverse);
}

void FormBackglass::StopAnimation(const string& szName)
{
   m_pB2SAnimation->StopAnimation(szName);
}

void FormBackglass::StopAllAnimations()
{
   m_pB2SAnimation->StopAllAnimations();
}

bool FormBackglass::IsAnimationRunning(const string& szName)
{
   return m_pB2SAnimation->IsAnimationRunning(szName);
}

void FormBackglass::StartRotation()
{
   if (m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && m_pB2SData->GetRotatingImages() && (*m_pB2SData->GetRotatingImages())[0].size() > 0) {
      if (m_rotateAngle > 0 && m_rotateTimerInterval > 0) {
         if (m_pRotateTimer->IsEnabled())
            m_pRotateTimer->Stop();
         m_rotateSlowDownSteps = 0;
         m_pRotateTimer->SetInterval(m_rotateTimerInterval);
         m_pRotateTimer->Start();
      }
   }
}

void FormBackglass::StopRotation()
{
   if (m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && m_pB2SData->GetRotatingImages() && (*m_pB2SData->GetRotatingImages())[0].size() > 0) {
      if (m_pRotateTimer->IsEnabled()) {
         if ((*m_pB2SData->GetRotatingPictureBox())[0]->GetSnippitRotationStopBehaviour() == eSnippitRotationStopBehaviour_SpinOff)
            m_rotateSlowDownSteps = 1;
         else if ((*m_pB2SData->GetRotatingPictureBox())[0]->GetSnippitRotationStopBehaviour() == eSnippitRotationStopBehaviour_RunAnimationTillEnd)
            m_rotateRunTillEnd = true;
         else if ((*m_pB2SData->GetRotatingPictureBox())[0]->GetSnippitRotationStopBehaviour() == eSnippitRotationStopBehaviour_RunAnimationToFirstStep)
            m_rotateRunToFirstStep = true;
         else
            m_pRotateTimer->Stop();
      }
   }
}

void FormBackglass::ShowScoreDisplays()
{
   if (m_selectedLEDType == eLEDTypes_Undefined)
      m_selectedLEDType = GetLEDType();
   if (m_selectedLEDType == eLEDTypes_Dream7) {
      for (const auto& [key, pDisplay] : *m_pB2SData->GetLEDDisplays())
         pDisplay->SetVisible(true);
   }
   else if (m_selectedLEDType == eLEDTypes_Rendered) {
      for (const auto& [key, pLED] : *m_pB2SData->GetLEDs())
         pLED->SetVisible(true);
   }
}

void FormBackglass::HideScoreDisplays()
{
   if (m_selectedLEDType == eLEDTypes_Undefined)
      m_selectedLEDType = GetLEDType();
   if (m_selectedLEDType == eLEDTypes_Dream7) {
      for (const auto& [key, pDisplay] : *m_pB2SData->GetLEDDisplays())
         pDisplay->SetVisible(false);
   }
   else if (m_selectedLEDType == eLEDTypes_Rendered) {
      for (const auto& [key, pLED] : *m_pB2SData->GetLEDs())
         pLED->SetVisible(false);
   }
}

eLEDTypes FormBackglass::GetLEDType()
{
   eLEDTypes ret = eLEDTypes_Undefined;
   if (m_pB2SData->GetLEDDisplays()->size() > 0) {
      for (const auto& [key, pDisplay] : *m_pB2SData->GetLEDDisplays()) {
         if (pDisplay->IsVisible()) {
            ret = eLEDTypes_Dream7;
            break;
         }
      }
   }
   else if (m_pB2SData->GetLEDs()->size() > 0) {
      for (const auto& [key, pLED] : *m_pB2SData->GetLEDs()) {
         if (pLED->IsVisible()) {
            ret = eLEDTypes_Rendered;
            break;
         }
      }
   }
   return ret;
}

void FormBackglass::PlaySound(const string& szSoundName)
{
   PLOGW << "Not implemented";
}

void FormBackglass::StopSound(const string& szSoundName)
{
   PLOGW << "Not implemented";
}

void FormBackglass::LoadB2SData()
{
   string szFilename = find_path_case_insensitive(TitleAndPathFromFilename(m_pB2SData->GetTableFileName().c_str()) + ".directb2s");
   if (szFilename.empty()) {
      PLOGW.printf("No directb2s file found");
      return;
   }

   PLOGI.printf("directb2s file found at: %s", szFilename.c_str());

   m_pB2SData->SetBackglassFileName(szFilename);

   std::ifstream infile(szFilename);
   if (!infile.good())
      return;

   char* data = nullptr;

   try {
      tinyxml2::XMLDocument b2sTree;
      std::stringstream buffer;
      std::ifstream myFile(szFilename.c_str());
      buffer << myFile.rdbuf();
      myFile.close();

      auto xml = buffer.str();
      if (b2sTree.Parse(xml.c_str(), xml.size())) {
         PLOGE.printf("Failed to parse directb2s file: %s", szFilename.c_str());
         return;
      }
      
      // try to get into the file and read some XML
      if (!b2sTree.FirstChildElement("DirectB2SData")) {
         PLOGE.printf("Invalid directb2s file: %s", szFilename.c_str());
         return;
      }

      m_pB2SSettings->SetBackglassFileVersion(b2sTree.FirstChildElement("DirectB2SData")->Attribute("Version"));

      // current backglass version is not allowed to be larger than server version and to be smaller minimum B2S version
      if (m_pB2SSettings->GetBackglassFileVersion() > m_pB2SSettings->GetDirectB2SVersion()) {
         PLOGE.printf("B2S backglass server version (%s) doesn't match directb2s file version (%s). Please update the B2S backglass server.", 
            m_pB2SSettings->GetDirectB2SVersion().c_str(), m_pB2SSettings->GetBackglassFileVersion().c_str());
         return;
      }
      else if (m_pB2SSettings->GetBackglassFileVersion() < m_pB2SSettings->GetMinimumDirectB2SVersion()) {
         PLOGE.printf("directb2s file version (%s) doesn't match minimum directb2s version. Please update the directb2s backglass file.", 
            m_pB2SSettings->GetBackglassFileVersion().c_str());
         return;
      }

      auto topnode = b2sTree.FirstChildElement("DirectB2SData");

      bool mergeBulbs = true;
      if (topnode->FirstChildElement("MergeBulbs"))
         mergeBulbs = (topnode->FirstChildElement("MergeBulbs")->IntAttribute("Value") != 0);

      // clear all data
      m_pB2SData->ClearAll(true);

      // get some basic info
      m_pB2SData->SetTableName(topnode->FirstChildElement("Name")->Attribute("Value"));
      m_pB2SData->SetTableType(topnode->FirstChildElement("TableType")->IntAttribute("Value"));
      m_pB2SData->SetDMDType(topnode->FirstChildElement("DMDType")->IntAttribute("Value"));
      if (topnode->FirstChildElement("DMDDefaultLocation"))
         m_pB2SData->SetDMDDefaultLocation({
            topnode->FirstChildElement("DMDDefaultLocation")->IntAttribute("LocX"), topnode->FirstChildElement("DMDDefaultLocation")->IntAttribute("LocY") });
      m_pB2SData->SetGrillHeight(max(topnode->FirstChildElement("GrillHeight")->IntAttribute("Value"), 0));
      if (topnode->FirstChildElement("GrillHeight")->FindAttribute("Small") && m_pB2SData->GetGrillHeight() > 0)
         m_pB2SData->SetSmallGrillHeight(max(topnode->FirstChildElement("GrillHeight")->IntAttribute("Small"), 0));
      if (topnode->FirstChildElement("DualBackglass"))
         m_pB2SData->SetDualBackglass(topnode->FirstChildElement("DualBackglass")->IntAttribute("Value") == 1);

      // maybe set current dual mode to get for sure
      if (!m_pB2SData->IsDualBackglass())
         m_pB2SSettings->SetCurrentDualMode(eDualMode_Authentic);

      // get skip defaults
      if (topnode->FirstChildElement("LampsDefaultSkipFrames") && !m_pB2SSettings->IsGameNameFound()) 
         m_pB2SSettings->SetLampsSkipFrames(topnode->FirstChildElement("LampsDefaultSkipFrames")->IntAttribute("Value"));
      if (topnode->FirstChildElement("SolenoidsDefaultSkipFrames") && !m_pB2SSettings->IsGameNameFound()) 
         m_pB2SSettings->SetSolenoidsSkipFrames(topnode->FirstChildElement("SolenoidsDefaultSkipFrames")->IntAttribute("Value"));
      if (topnode->FirstChildElement("GIStringsDefaultSkipFrames") && !m_pB2SSettings->IsGameNameFound()) 
         m_pB2SSettings->SetGIStringsSkipFrames(topnode->FirstChildElement("GIStringsDefaultSkipFrames")->IntAttribute("Value"));
      if (topnode->FirstChildElement("LEDsDefaultSkipFrames") && !m_pB2SSettings->IsGameNameFound()) 
         m_pB2SSettings->SetLEDsSkipFrames(topnode->FirstChildElement("LEDsDefaultSkipFrames")->IntAttribute("Value"));
      if (!m_pB2SSettings->IsGameNameFound()) {
         if (m_pB2SSettings->GetLampsSkipFrames() == 0)
            m_pB2SSettings->SetLampsSkipFrames(1);
         if (m_pB2SSettings->GetSolenoidsSkipFrames() == 0)
            m_pB2SSettings->SetSolenoidsSkipFrames(3);
         if (m_pB2SSettings->GetGIStringsSkipFrames() == 0)
            m_pB2SSettings->SetGIStringsSkipFrames(3);
      }

      // get all illumination info
      std::map<string, int> roms4Authentic;
      std::map<string, int> roms4Fantasy;
      if (topnode->FirstChildElement("Illumination")) {
         for (auto innerNode = topnode->FirstChildElement("Illumination")->FirstChildElement("Bulb");  innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Bulb")) {
            string parent = innerNode->Attribute("Parent");
            int id = innerNode->IntAttribute("ID");
            int romid = 0;
            eRomIDType romidtype = (eRomIDType)0;
            int romidvalue = 0;
            bool rominverted = false;
            if (innerNode->FindAttribute("B2SID")) {
               romid = innerNode->IntAttribute("B2SID");
               if (innerNode->FindAttribute("B2SValue"))
                  romidvalue = innerNode->IntAttribute("B2SValue");
               romidtype = (eRomIDType)1;
            }
            else {
               romid = innerNode->IntAttribute("RomID");
               romidtype = (eRomIDType)innerNode->IntAttribute("RomIDType");
               if (innerNode->FindAttribute("RomInverted"))
                  rominverted = (innerNode->IntAttribute("RomInverted") == 1);
            }
            int intensity = innerNode->IntAttribute("Intensity");
            int initialstate = innerNode->IntAttribute("InitialState");
            int dualmode = 0;
            if (innerNode->FindAttribute("DualMode"))
                dualmode = innerNode->IntAttribute("DualMode");
            string name = innerNode->Attribute("Name");
            bool isimagesnippit = false;
            if (innerNode->FindAttribute("IsImageSnippit"))
               isimagesnippit = (innerNode->IntAttribute("IsImageSnippit") == 1);
            int zorder = 0;
            if (innerNode->FindAttribute("ZOrder"))
               zorder = innerNode->IntAttribute("ZOrder");
            ePictureBoxType picboxtype = ePictureBoxType_StandardImage;
            int picboxrotatesteps = 0;
            int picboxrotateinterval = 0;
            eSnippitRotationDirection picboxrotationdirection = eSnippitRotationDirection_Clockwise;
            eSnippitRotationStopBehaviour picboxrotationstopbehaviour = eSnippitRotationStopBehaviour_SpinOff;
            if (innerNode->FindAttribute("SnippitType")) {
               picboxtype = (ePictureBoxType)innerNode->IntAttribute("SnippitType");
               if (innerNode->FindAttribute("SnippitRotatingSteps"))
                  picboxrotatesteps = innerNode->IntAttribute("SnippitRotatingSteps");
               else {
                  if (innerNode->FindAttribute("SnippitRotatingAngle"))
                     picboxrotatesteps = 360 / innerNode->IntAttribute("SnippitRotatingAngle");
               }
               if (innerNode->FindAttribute("SnippitRotatingInterval"))
                  picboxrotateinterval = innerNode->IntAttribute("SnippitRotatingInterval");
               if (innerNode->FindAttribute("SnippitRotatingDirection"))
                   picboxrotationdirection = (eSnippitRotationDirection)innerNode->IntAttribute("SnippitRotatingDirection");
               if (innerNode->FindAttribute("SnippitRotatingStopBehaviour"))
                   picboxrotationstopbehaviour = (eSnippitRotationStopBehaviour)innerNode->IntAttribute("SnippitRotatingStopBehaviour");
            }
            bool visible = (innerNode->IntAttribute("Visible") == 1);
            SDL_Point loc = { innerNode->IntAttribute("LocX"), innerNode->IntAttribute("LocY") };
            SDL_Rect size = { 0, 0, innerNode->IntAttribute("Width"), innerNode->IntAttribute("Height") };
            SDL_Surface* pImage = Base64ToImage(innerNode->Attribute("Image"));
            SDL_Surface* pOffImage = NULL;
            if (innerNode->FindAttribute("OffImage"))
                pOffImage = Base64ToImage(innerNode->Attribute("OffImage"));
            B2SPictureBox* pPicbox = new B2SPictureBox();
            bool isOnBackglass = (parent == "Backglass");
            pPicbox->SetName("PictureBox" + std::to_string(id));
            pPicbox->SetGroupName(name);
            pPicbox->SetLocation(loc);
            pPicbox->SetSize(size);
            pPicbox->SetRomID(romid);
            pPicbox->SetRomIDType(romidtype);
            pPicbox->SetRomIDValue(romidvalue);
            pPicbox->SetRomInverted(rominverted);
            pPicbox->SetIntensity(intensity);
            pPicbox->SetInitialState(initialstate);
            pPicbox->SetDualMode((eDualMode)dualmode);
            pPicbox->SetBackgroundImage(pImage);
            pPicbox->SetOffImage(pOffImage);
            pPicbox->SetImageSnippit(isimagesnippit);
            pPicbox->SetSnippitRotationStopBehaviour(picboxrotationstopbehaviour);
            pPicbox->SetZOrder(zorder);
            pPicbox->SetPictureBoxType(picboxtype);

            if (isOnBackglass) {
                pPicbox->SetType(eType_2_OnBackglass);
                AddControl(pPicbox);
                // add to general collection
                m_pB2SData->GetIlluminations()->Add(pPicbox);
                // maybe add ZOrder info
                if (zorder > 0) {
                    m_pB2SData->SetUseZOrder(true);
                    m_pB2SData->GetZOrderImages()->Add(pPicbox);
                }
                if (romid > 0 && picboxtype == ePictureBoxType_StandardImage && romidtype != eRomIDType_Mech) {
                   string key = 
                      (rominverted ? "I"s : string()) + (romidtype == eRomIDType_Lamp ? "L" : romidtype == eRomIDType_Solenoid ? "S" : "GI") + 
                      std::to_string(romid);
                   if (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == eDualMode_Authentic)
                      roms4Authentic[key] += size.w * size.h;
                   if (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == eDualMode_Fantasy)
                      roms4Fantasy[key] += size.w * size.h;
                }
            }
            else {
               if (!m_pB2SSettings->IsHideB2SDMD()) {
                  CheckDMDForm();
                  pPicbox->SetType(eType_2_OnDMD);
                  m_pFormDMD->AddControl(pPicbox);
                  // add to general collection
                  m_pB2SData->GetDMDIlluminations()->Add(pPicbox);
                  // maybe add ZOrder info
                  if (zorder > 0) {
                     m_pB2SData->SetUseDMDZOrder(true);
                     m_pB2SData->GetZOrderDMDImages()->Add(pPicbox);
                  }
               }
            }
            pPicbox->BringToFront();
            pPicbox->SetVisible(false);

            // add illumination into group
            m_pB2SData->GetIlluminationGroups()->Add(pPicbox);

            // maybe do picture rotating
            if (picboxrotatesteps > 0) {
               if (picboxtype == ePictureBoxType_SelfRotatingImage && m_pB2SData->GetRotatingImages()->size() == 0) {
                  m_rotateTimerInterval = picboxrotateinterval;
                  RotateImage(pPicbox, picboxrotatesteps, picboxrotationdirection, ePictureBoxType_SelfRotatingImage);
               }
               else if (picboxtype == ePictureBoxType_MechRotatingImage && m_pB2SData->GetRotatingImages()->size() == 0) {
                  RotateImage(pPicbox, picboxrotatesteps, picboxrotationdirection, ePictureBoxType_MechRotatingImage, romidtype, romid);
               }
            }
         }
      }

      // get all score infos
      int dream7index = 1;
      int renderedandreelindex = 1;
      if (topnode->FirstChildElement("Scores")) {
         int rollinginterval = 0;
         if (topnode->FirstChildElement("Scores")->FindAttribute("ReelRollingInterval"))
            rollinginterval = topnode->FirstChildElement("Scores")->IntAttribute("ReelRollingInterval");
         for (auto innerNode = topnode->FirstChildElement("Scores")->FirstChildElement("Score"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Score")) {
            string parent = innerNode->Attribute("Parent");
            int id = innerNode->IntAttribute("ID");
            int setid = 0;
            if (innerNode->FindAttribute("ReelIlluImageSet"))
               setid = innerNode->IntAttribute("ReelIlluImageSet");
            string reeltype = innerNode->Attribute("ReelType");
            OLE_COLOR reellitcolor = String2Color(innerNode->Attribute("ReelLitColor"));
            OLE_COLOR reeldarkcolor = String2Color(innerNode->Attribute("ReelDarkColor"));
            float d7glow = innerNode->FloatAttribute("Glow") / 100.0f;
            float d7thickness = innerNode->FloatAttribute("Thickness") / 100.0f;
            float d7shear = innerNode->FloatAttribute("Shear") / 100.0f;
            int digits = innerNode->IntAttribute("Digits");
            int spacing = innerNode->IntAttribute("Spacing");
            bool hidden = false;
            if (innerNode->FindAttribute("DisplayState"))
                hidden = (innerNode->IntAttribute("DisplayState") == 1);
            SDL_Point loc = { innerNode->IntAttribute("LocX"), innerNode->IntAttribute("LocY") };
            SDL_Rect size = { 0, 0, innerNode->IntAttribute("Width"), innerNode->IntAttribute("Height") };
            int b2sstartdigit = 0;
            int b2sscoretype = 0;
            int b2splayerno = 0;
            if (innerNode->FindAttribute("B2SStartDigit"))
                b2sstartdigit = innerNode->IntAttribute("B2SStartDigit");
            if (innerNode->FindAttribute("B2SScoreType"))
                b2sscoretype = innerNode->IntAttribute("B2SScoreType");
            if (innerNode->FindAttribute("B2SPlayerNo"))
                b2splayerno = innerNode->IntAttribute("B2SPlayerNo");
            int dream7b2sstartdigit = b2sstartdigit;
            int startdigit = (b2sstartdigit > 0 ? b2sstartdigit : renderedandreelindex);
            int romid = 0;
            int romidtype = 0;
            int romidvalue = 0;
            if (innerNode->FindAttribute("ReelIlluB2SID")) {
                romid = innerNode->IntAttribute("ReelIlluB2SID");
                if (innerNode->FindAttribute("ReelIlluB2SValue"))
                    romidvalue = innerNode->IntAttribute("ReelIlluB2SValue");
                romidtype = 1;
            }
            string soundName;

            // set some tmp vars
            bool isOnBackglass = (parent == "Backglass");
            bool isDream7LEDs = string_starts_with_case_insensitive(reeltype, "dream7"s);
            bool isRenderedLEDs = string_starts_with_case_insensitive(reeltype, "rendered"s);
            bool isReels = !isDream7LEDs && !isRenderedLEDs;
            SDL_FRect glowbulb = { 0.0f, 0.0f, 0.0f, 0.0f };
            int glow = d7glow;
            string ledtype;

            // set led type
            if (isDream7LEDs)
                ledtype = reeltype.substr(9);
            else if (isRenderedLEDs)
                ledtype = reeltype.substr(11);

            // maybe get default glow value
            if (m_pB2SSettings->GetDefaultGlow() == -1)
                m_pB2SSettings->SetDefaultGlow(d7glow);

            // set preferred LED settings
            if (isRenderedLEDs || isDream7LEDs) {
                if (m_pB2SSettings->IsGameNameFound() && m_pB2SSettings->GetUsedLEDType() != eLEDTypes_Undefined) {
                   isDream7LEDs = (m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
                   isRenderedLEDs = (m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
                }
                else if (m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Undefined)
                   m_pB2SSettings->SetUsedLEDType(isDream7LEDs ? eLEDTypes_Dream7 : eLEDTypes_Rendered);
                if (m_pB2SSettings->IsGameNameFound() && m_pB2SSettings->GetGlowIndex() > -1)
                   glow = m_pB2SSettings->GetGlowIndex() * 8;
                if (m_pB2SSettings->IsGameNameFound() && m_pB2SSettings->IsGlowBulbOn())
                   glowbulb = { 0.0f, 0.0f, 0.1f, 0.4f };
            }

            // maybe create Dream 7 LED display controls
            if (isDream7LEDs || isRenderedLEDs) {
               // add some self rendered Dream7 LED segments
               Dream7Display* pLed = new Dream7Display();
               pLed->SetName("LEDDisplay" + std::to_string(id));
               pLed->SetLocation(loc);
               pLed->SetSize(size);
               if (ledtype == "7" || ledtype == "8")
                  pLed->SetType(SegmentNumberType_SevenSegment);
               else if (ledtype == "9" || ledtype == "10")
                  pLed->SetType(SegmentNumberType_TenSegment);
               else if (ledtype == "14")
                  pLed->SetType(SegmentNumberType_FourteenSegment);
               pLed->SetScaleMode(ScaleMode_Stretch);
               pLed->SetDigits(digits);
               pLed->SetSpacing(spacing * 5);
               pLed->SetHidden(hidden);
               // color settings
               pLed->SetLightColor(reellitcolor);
               pLed->SetGlassColor(RGB(min(GetRValue(reellitcolor) + 50, 255), min(GetGValue(reellitcolor) + 50, 255), min(GetBValue(reellitcolor) + 50, 255)));
               pLed->SetGlassColorCenter(RGB(min(GetRValue(reellitcolor) + 70, 255), min(GetGValue(reellitcolor) + 70, 255), min(GetBValue(reellitcolor) + 70, 255)));
               pLed->SetOffColor(reeldarkcolor);
               pLed->SetBackColor(RGB(15, 15, 15));
               pLed->SetGlassAlpha(140);
               pLed->SetGlassAlphaCenter(255);
               pLed->SetThickness(d7thickness * 1.2);
               pLed->SetShear(d7shear);
               pLed->SetGlow(glow < 3 ? glow : 3);
               if (!SDL_FRectEmpty(&glowbulb))
                  pLed->SetBulbSize(glowbulb);
               // 'TAXI' patch to shear the third LED display
               if (id == 3 && m_pB2SData->GetTableName() == "Taxi") {
                  pLed->SetAngle(4.0f);
                  pLed->SetShear(pLed->GetShear() / 2.0f);
               }

               // add control to parent
               if (isOnBackglass) {
                  AddControl(pLed);
                  // add to general collection
                  (*m_pB2SData->GetLEDDisplays())[pLed->GetName()] = pLed;
               }
               else {
                  if (!m_pB2SSettings->IsHideB2SDMD()) {
                    CheckDMDForm();
                    m_pFormDMD->AddControl(pLed);
                    // add to general collection
                    (*m_pB2SData->GetLEDDisplays())[pLed->GetName()] = pLed;
                  }
               }
               pLed->BringToFront();
               pLed->SetVisible(isDream7LEDs && !hidden);
               // add digit location info
               for (int i = 0; i < digits; i++) {
                  if (isOnBackglass || !m_pB2SSettings->IsHideB2SDMD())
                    (*m_pB2SData->GetLEDDisplayDigits())[(dream7b2sstartdigit > 0 ? dream7b2sstartdigit : dream7index) - 1] = new LEDDisplayDigitLocation(pLed, i, id);
                  dream7index++;
                  if (dream7b2sstartdigit > 0)
                     dream7b2sstartdigit++;
               }
               // add LED area
               (*m_pB2SData->GetLEDAreas())["LEDArea" + std::to_string(id)] = new LEDAreaInfo({ loc.x, loc.y, size.w, size.h }, !isOnBackglass);
               // add or update player info collection
               if (b2splayerno > 0) {
                   m_pB2SData->SetPlayerAdded(true);
                   if (!m_pB2SData->GetPlayers()->contains(b2splayerno))
                      m_pB2SData->GetPlayers()->Add(b2splayerno);
                   (*m_pB2SData->GetPlayers())[b2splayerno]->Add(new ControlInfo(startdigit, digits, eControlType_Dream7LEDDisplay, pLed));
               }
            }

            // create reel or led boxes
            int width = (size.w - (digits - 1) * spacing / 2) / digits;
            for (int i = 1; i <= digits; i++) {
               int x = loc.x + ((i - 1) * (width + spacing / 2));
               if (isRenderedLEDs || isDream7LEDs) {
                  // add some self rendered LEDs
                  B2SLEDBox* pLed = new B2SLEDBox();
                  pLed->SetID((b2sstartdigit > 0) ? b2sstartdigit : renderedandreelindex);
                  pLed->SetDisplayID(id);
                  pLed->SetName("LEDBox" + std::to_string(pLed->GetID()));
                  pLed->SetStartDigit(startdigit);
                  pLed->SetDigits(digits);
                  pLed->SetHidden(hidden);
                  pLed->SetLocation({ x, loc.y });
                  pLed->SetSize({ 0, 0, width, size.h });
                  if (ledtype == "7" || ledtype == "8")
                     pLed->SetLEDType(eLEDType_LED8);
                  else if (ledtype == "9" || ledtype == "10")
                     pLed->SetLEDType(eLEDType_LED10);
                  else if (ledtype == "14")
                     pLed->SetLEDType(eLEDType_LED14);
                  pLed->SetLitLEDSegmentColor(reellitcolor);
                  pLed->SetDarkLEDSegmentColor(reeldarkcolor);
                  pLed->SetBackColor(RGB(5, 5, 5));
                  if (isOnBackglass) {
                     pLed->SetType(eType_2_OnBackglass);
                     AddControl(pLed);
                     // add to general collection
                     (*m_pB2SData->GetLEDs())[pLed->GetName()] = pLed;
                  }
                  else {
                     if (!m_pB2SSettings->IsHideB2SDMD()) {
                        CheckDMDForm();
                        pLed->SetType(eType_2_OnDMD);
                        m_pFormDMD->AddControl(pLed);
                        // add to general collection
                        (*m_pB2SData->GetLEDs())[pLed->GetName()] = pLed;
                     }
                  }
                  pLed->BringToFront();
                  pLed->SetVisible(isRenderedLEDs && !hidden);
               }
               else if (isReels) {
                  // look for matching reel sound
                  soundName.clear();
                  if (innerNode->FindAttribute(("Sound" + std::to_string(i)).c_str())) {
                     soundName = innerNode->Attribute(("Sound" + std::to_string(i)).c_str());
                     if (soundName.empty())
                        soundName = "stille"s;
                  }
                  // add reel or LED pictures
                  B2SReelBox* pReel = new B2SReelBox();
                  pReel->SetID((b2sstartdigit > 0) ? b2sstartdigit : renderedandreelindex);
                  pReel->SetDisplayID(id);
                  pReel->SetSetID(setid);
                  pReel->SetName("ReelBox" + std::to_string(pReel->GetID()));
                  pReel->SetStartDigit(startdigit);
                  pReel->SetDigits(digits);
                  pReel->SetRomID(romid);
                  pReel->SetRomIDType((eRomIDType)romidtype);
                  pReel->SetRomIDValue(romidvalue);
                  pReel->SetLocation({ x, loc.y });
                  pReel->SetSize({ 0, 0, width, size.h });
                  pReel->SetReelType(reeltype.substr(0, reeltype.length() - 2));
                  pReel->SetScoreType((eScoreType)b2sscoretype);
                  pReel->SetSoundName(soundName);
                  if (rollinginterval >= 10)
                     pReel->SetRollingInterval(rollinginterval);
                  if (isOnBackglass) {
                    pReel->SetType(eType_2_OnBackglass);
                    AddControl(pReel);
                    m_pB2SData->GetReels()->Add(pReel);
                  }
                  else {
                    if (!m_pB2SSettings->IsHideB2SDMD()) {
                        CheckDMDForm();
                        pReel->SetType(eType_2_OnDMD);
                        m_pFormDMD->AddControl(pReel);
                        m_pB2SData->GetReels()->Add(pReel);
                    }
                  }
                  pReel->BringToFront();
                  pReel->SetVisible(!hidden);
                  // add or update reel display
                  if (!m_pB2SData->GetReelDisplays()->contains(id)) {
                     B2SReelDisplay* pReelDisplay = new B2SReelDisplay();
                     pReelDisplay->SetStartDigit(startdigit);
                     pReelDisplay->SetDigits(digits);
                     pReelDisplay->GetReels()->Add(pReel->GetID(), pReel);
                     (*m_pB2SData->GetReelDisplays())[id] = pReelDisplay;
                  }
                  else {
                     (*m_pB2SData->GetReelDisplays())[id]->GetReels()->Add(pReel->GetID(), pReel);
                  }
                  // add or update player info collection
                  if (b2splayerno > 0) {
                      m_pB2SData->SetPlayerAdded(true);
                      if (!m_pB2SData->GetPlayers()->contains(b2splayerno))
                          m_pB2SData->GetPlayers()->Add(b2splayerno);
                      (*m_pB2SData->GetPlayers())[b2splayerno]->Add(new ControlInfo(startdigit, digits, eControlType_ReelDisplay, (*m_pB2SData->GetReelDisplays())[id]));
                  }
               }

               renderedandreelindex++;
               if (b2sstartdigit > 0)
                  b2sstartdigit++;
            }

            dream7index = renderedandreelindex;
         }
      }

      // maybe get all reel images
      if (topnode->FirstChildElement("Reels")) {
         if (topnode->FirstChildElement("Reels")->FirstChildElement("Image")) {
            for (auto innerNode = topnode->FirstChildElement("Reels")->FirstChildElement("Image");  innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Image")) {
               string name = innerNode->Attribute("Name");
               SDL_Surface* pImage = Base64ToImage(innerNode->Attribute("Image"));
               if (!m_pB2SData->GetReelImages()->contains(name))
                  (*m_pB2SData->GetReelImages())[name] = pImage;
            }
         }
         else if (topnode->FirstChildElement("Reels")->FirstChildElement("Images") && topnode->FirstChildElement("Reels")->FirstChildElement("Images")->FirstChildElement("Image")) {
            for (auto innerNode = topnode->FirstChildElement("Reels")->FirstChildElement("Images")->FirstChildElement("Image");  innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Image")) {
               string name = innerNode->Attribute("Name");
               SDL_Surface* pImage = Base64ToImage(innerNode->Attribute("Image"));
               if (!m_pB2SData->GetReelImages()->contains(name))
                  (*m_pB2SData->GetReelImages())[name] = pImage;
               // maybe get the intermediate reel images
               if (innerNode->FindAttribute("CountOfIntermediates")) {
                  int countOfIntermediates = innerNode->IntAttribute("CountOfIntermediates");
                  for (int i = 1; i <= countOfIntermediates; i++) {
                     string intname = name + '_' + std::to_string(i);
                     SDL_Surface* intimage = Base64ToImage(innerNode->Attribute(("IntermediateImage" + std::to_string(i)).c_str()));
                     if (!m_pB2SData->GetReelIntermediateImages()->contains(intname))
                        (*m_pB2SData->GetReelIntermediateImages())[intname] = intimage;
                  }
               }
            }

            if (topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")) {
               if (topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("IlluminatedImage")) {
                  for (auto innerNode = topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("IlluminatedImage"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("IlluminatedImage")) {
                     string name = innerNode->Attribute("Name");
                     SDL_Surface* pImage = Base64ToImage(innerNode->Attribute("Image"));
                     if (!m_pB2SData->GetReelIlluImages()->contains(name))
                        (*m_pB2SData->GetReelIlluImages())[name] = pImage;
                     // maybe get the intermediate reel images
                     if (innerNode->FindAttribute("CountOfIntermediates")) {
                        int countOfIntermediates = innerNode->IntAttribute("CountOfIntermediates");
                        for (int i = 1; i <= countOfIntermediates; i++) {
                           string intname = name + '_' + std::to_string(i);
                           SDL_Surface* intimage = Base64ToImage(innerNode->Attribute(("IntermediateImage" + std::to_string(i)).c_str()));
                           if (!m_pB2SData->GetReelIntermediateIlluImages()->contains(intname))
                              (*m_pB2SData->GetReelIntermediateIlluImages())[intname] = intimage;
                        }
                     }
                  }
               }
               else if (topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("Set")) {
                  for (auto setnode = topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("Set"); setnode != nullptr; setnode = setnode->NextSiblingElement("Set")) {
                     int setid = setnode->IntAttribute("ID");
                     for (auto innerNode = setnode->FirstChildElement("IlluminatedImage"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("IlluminatedImage")) {
                        string name = string(innerNode->Attribute("Name")) + '_' + std::to_string(setid);
                        SDL_Surface* pImage = Base64ToImage(innerNode->Attribute("Image"));
                        if (!m_pB2SData->GetReelIlluImages()->contains(name))
                           (*m_pB2SData->GetReelIlluImages())[name] = pImage;
                        // maybe get the intermediate reel images
                        if (innerNode->FindAttribute("CountOfIntermediates")) {
                           int countOfIntermediates = innerNode->IntAttribute("CountOfIntermediates");
                           for (int i = 1; i <= countOfIntermediates; i++) {
                              string intname = name + '_' + std::to_string(i);
                              SDL_Surface* intimage = Base64ToImage(innerNode->Attribute(("IntermediateImage" + std::to_string(i)).c_str()));
                              if (!m_pB2SData->GetReelIntermediateIlluImages()->contains(intname))
                                 (*m_pB2SData->GetReelIntermediateIlluImages())[intname] = intimage;
                           }
                        }
                     }
                  }
               }
            }
         }
      }

      // maybe get all sounds
      if (topnode->FirstChildElement("Sounds")) {
         for (auto innerNode = topnode->FirstChildElement("Sounds")->FirstChildElement("Sound"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Sound")) {
            string name = innerNode->Attribute("Name");
            Sound* pSound = Base64ToWav(innerNode->Attribute("Stream"));
            if (!m_pB2SData->GetSounds()->contains(name))
               (*m_pB2SData->GetSounds())[name] = pSound;
         }
         for(const auto& [key, pReel] : *m_pB2SData->GetReels()) {
            if (m_pB2SData->GetSounds()->contains(pReel->GetSoundName()))
               pReel->SetSound((*m_pB2SData->GetSounds())[pReel->GetSoundName()]);
         }
      }

      // get background and maybe DMD image(s)
      if (topnode->FirstChildElement("Images")) {
         SDL_Surface* pOffImage = NULL;
         // backglass image
         if (topnode->FirstChildElement("Images")->FirstChildElement("BackglassOffImage")) {
            m_pB2SData->SetOnAndOffImage(true);
            // get on and off image
            pOffImage = Base64ToImage(topnode->FirstChildElement("Images")->FirstChildElement("BackglassOffImage")->Attribute("Value"));
            m_pDarkImage4Authentic = pOffImage;
            if (m_pB2SData->IsDualBackglass())
               m_pDarkImage4Fantasy = pOffImage;
            auto onimagenode = topnode->FirstChildElement("Images")->FirstChildElement("BackglassOnImage");
            SDL_Surface* pOnImage = NULL;
            if (onimagenode) {
               pOnImage = Base64ToImage(onimagenode->Attribute("Value"));
               m_pTopLightImage4Authentic = pOnImage;
               if (m_pB2SData->IsDualBackglass())
                  m_pTopLightImage4Fantasy = pOnImage;
               auto romidnode = onimagenode->FindAttribute("RomID");
               if (romidnode) {
                  m_topRomID4Authentic = romidnode->IntValue();
                  m_topRomIDType4Authentic = (eRomIDType)onimagenode->IntAttribute("RomIDType");
                  m_topRomInverted4Authentic = false;
                  switch(m_topRomIDType4Authentic) {
                     case eRomIDType_Lamp:
                        (*m_pB2SData->GetUsedRomLampIDs4Authentic())[m_topRomID4Authentic] = vector<B2SBaseBox*>{};
                        break;
                     case eRomIDType_Solenoid:
                        (*m_pB2SData->GetUsedRomSolenoidIDs4Authentic())[m_topRomID4Authentic] = vector<B2SBaseBox*>{};
                        break;
                     case eRomIDType_GIString:
                        (*m_pB2SData->GetUsedRomGIStringIDs4Authentic())[m_topRomID4Authentic] = vector<B2SBaseBox*>{};
                        break;
                     default: break;
                  }
                  if (m_pB2SData->IsDualBackglass()) {
                     m_topRomID4Fantasy = romidnode->IntValue();
                     m_topRomIDType4Fantasy = (eRomIDType)onimagenode->IntAttribute("RomIDType");
                     m_topRomInverted4Fantasy = false;
                     switch(m_topRomIDType4Authentic) {
                        case eRomIDType_Lamp:
                           (*m_pB2SData->GetUsedRomLampIDs4Fantasy())[m_topRomID4Fantasy] = vector<B2SBaseBox*>{};
                           break;
                        case eRomIDType_Solenoid:
                           (*m_pB2SData->GetUsedRomSolenoidIDs4Fantasy())[m_topRomID4Fantasy] = vector<B2SBaseBox*>{};
                           break;
                        case eRomIDType_GIString:
                           (*m_pB2SData->GetUsedRomGIStringIDs4Fantasy())[m_topRomID4Fantasy] = vector<B2SBaseBox*>{};
                           break;
                        default: break;
                     }
                  }
               }
            }
         }
         else {
            SDL_Surface* pImage = NULL;
            if (topnode->FirstChildElement("Images")->FirstChildElement("BackglassImage")) {
               pImage = Base64ToImage(topnode->FirstChildElement("Images")->FirstChildElement("BackglassImage")->Attribute("Value"));
               m_pDarkImage4Authentic = pImage;
               if (m_pB2SData->IsDualBackglass())
                  m_pDarkImage4Fantasy = pImage;
            }
        }
        // starting image is the dark image
        SetBackgroundImage(GetDarkImage());

        // DMD image
        SDL_Surface* pImage = NULL;
        if (topnode->FirstChildElement("Images")->FirstChildElement("DMDImage")) {
           pImage = Base64ToImage(topnode->FirstChildElement("Images")->FirstChildElement("DMDImage")->Attribute("Value"));
           if (pImage) {
              if (!m_pB2SSettings->IsHideB2SDMD()) {
                 CheckDMDForm();
                 m_pFormDMD->SetBackgroundImage(pImage);
              }
           }
        }

        // look for the largest bulb amount
        int top4Authentic = 0;
        string topkey4Authentic;
        int second4Authentic = 0;
        string secondkey4Authentic;
        for (auto romsize : roms4Authentic) {
           if (romsize.second > second4Authentic) {
              second4Authentic = romsize.second;
              secondkey4Authentic = romsize.first;
           }
           if (romsize.second > top4Authentic) {
              second4Authentic = top4Authentic;
              secondkey4Authentic = topkey4Authentic;
              top4Authentic = romsize.second;
              topkey4Authentic = romsize.first;
           }
        }
        int top4Fantasy = 0;
        string topkey4Fantasy;
        int second4Fantasy = 0;
        string secondkey4Fantasy;
        if (m_pB2SData->IsDualBackglass()) {
           for (auto romsize : roms4Fantasy) {
              if (romsize.second > second4Fantasy) {
                 second4Fantasy = romsize.second;
                 secondkey4Fantasy = romsize.first;
              }
              if (romsize.second > top4Fantasy) {
                 second4Fantasy = top4Fantasy;
                 secondkey4Fantasy = topkey4Fantasy;
                 top4Fantasy = romsize.second;
                 topkey4Fantasy = romsize.first;
              }
           }
        }

        // maybe draw some light images for pretty fast image changing
        if (top4Authentic >= minSize4Image && mergeBulbs) {
           // create some light images
           if (m_pTopLightImage4Authentic == NULL) {
              m_pTopLightImage4Authentic = CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, topkey4Authentic, "", m_topRomID4Authentic, m_topRomIDType4Authentic, m_topRomInverted4Authentic);
              if (second4Authentic > minSize4Image) {
                 m_pSecondLightImage4Authentic = CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, secondkey4Authentic, "", m_secondRomID4Authentic, m_secondRomIDType4Authentic, m_secondRomInverted4Authentic);
                 m_pTopAndSecondLightImage4Authentic = CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, topkey4Authentic, secondkey4Authentic);
              }
           }
           else {
              m_pSecondLightImage4Authentic = CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, topkey4Authentic, "", m_secondRomID4Authentic, m_secondRomIDType4Authentic, m_secondRomInverted4Authentic);
              m_pTopAndSecondLightImage4Authentic = CreateLightImage(m_pTopLightImage4Authentic, eDualMode_Authentic, topkey4Authentic);
           }
        }
        if (m_pB2SData->IsDualBackglass() && top4Fantasy >= minSize4Image && mergeBulbs) {
           // create some light images
           if (m_pTopLightImage4Fantasy == NULL) {
              m_pTopLightImage4Fantasy = CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy, "", m_topRomID4Fantasy, m_topRomIDType4Fantasy, m_topRomInverted4Fantasy);
              if (second4Fantasy > minSize4Image) {
                 m_pSecondLightImage4Fantasy = CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, secondkey4Fantasy, "", m_secondRomID4Fantasy, m_secondRomIDType4Fantasy, m_secondRomInverted4Fantasy);
                 m_pTopAndSecondLightImage4Fantasy = CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy, secondkey4Fantasy);
              }
           }
           else {
              m_pSecondLightImage4Fantasy = CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy, "", m_secondRomID4Fantasy, m_secondRomIDType4Fantasy, m_secondRomInverted4Fantasy);
              m_pTopAndSecondLightImage4Fantasy = CreateLightImage(m_pTopLightImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy);
           }
        }
        m_pB2SData->SetUsedTopRomIDType4Authentic(m_topRomIDType4Authentic);
        m_pB2SData->SetUsedSecondRomIDType4Authentic(m_secondRomIDType4Authentic);
        if (m_pB2SData->IsDualBackglass()) {
           m_pB2SData->SetUsedTopRomIDType4Fantasy(m_topRomIDType4Fantasy);
           m_pB2SData->SetUsedSecondRomIDType4Fantasy(m_secondRomIDType4Fantasy);
        }
        // remove top and second rom bulbs
        CheckBulbs(m_topRomID4Authentic, m_topRomIDType4Authentic, m_topRomInverted4Authentic, eDualMode_Authentic);
        CheckBulbs(m_secondRomID4Authentic, m_secondRomIDType4Authentic, m_secondRomInverted4Authentic, eDualMode_Authentic);

        if (m_pB2SData->IsDualBackglass()) {
           CheckBulbs(m_topRomID4Fantasy, m_topRomIDType4Fantasy, m_topRomInverted4Fantasy, eDualMode_Fantasy);
           CheckBulbs(m_secondRomID4Fantasy, m_secondRomIDType4Fantasy, m_secondRomInverted4Fantasy, eDualMode_Fantasy);
        }
      }

      // get all animation info
      if (topnode->FirstChildElement("Animations")) {
         for (auto innerNode = topnode->FirstChildElement("Animations")->FirstChildElement("Animation"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Animation")) {
            string name = innerNode->Attribute("Name");
            eDualMode dualmode = eDualMode_Both;
            if (innerNode->FindAttribute("DualMode"))
                dualmode = (eDualMode)innerNode->IntAttribute("DualMode");
            int interval = innerNode->IntAttribute("Interval");
            int loops = innerNode->IntAttribute("Loops");
            string idJoins = innerNode->Attribute("IDJoin");
            bool startAnimationAtBackglassStartup = (string(innerNode->Attribute("StartAnimationAtBackglassStartup")) == "1");
            eLightsStateAtAnimationStart lightsStateAtAnimationStart = eLightsStateAtAnimationStart_NoChange;
            eLightsStateAtAnimationEnd lightsStateAtAnimationEnd = eLightsStateAtAnimationEnd_InvolvedLightsOff;
            eAnimationStopBehaviour animationstopbehaviour = eAnimationStopBehaviour_StopImmediatelly;
            bool lockInvolvedLamps = false;
            bool hidescoredisplays = false;
            bool bringtofront = false;
            bool randomstart = false;
            int randomquality = 1;
            if (innerNode->FindAttribute("LightsStateAtAnimationStart"))
                lightsStateAtAnimationStart = (eLightsStateAtAnimationStart)innerNode->IntAttribute("LightsStateAtAnimationStart");
            else if (innerNode->FindAttribute("AllLightsOffAtAnimationStart"))
                lightsStateAtAnimationStart = (string(innerNode->Attribute("AllLightsOffAtAnimationStart")) == "1") ? eLightsStateAtAnimationStart_LightsOff : eLightsStateAtAnimationStart_NoChange;
            if (innerNode->FindAttribute("LightsStateAtAnimationEnd"))
                lightsStateAtAnimationEnd = (eLightsStateAtAnimationEnd)innerNode->IntAttribute("LightsStateAtAnimationEnd");
            else if (innerNode->FindAttribute("ResetLightsAtAnimationEnd"))
                lightsStateAtAnimationEnd = (string(innerNode->Attribute("ResetLightsAtAnimationEnd")) == "1") ? eLightsStateAtAnimationEnd_LightsReseted : eLightsStateAtAnimationEnd_Undefined;
            if (innerNode->FindAttribute("AnimationStopBehaviour"))
                animationstopbehaviour = (eAnimationStopBehaviour)innerNode->IntAttribute("AnimationStopBehaviour");
            else if (innerNode->FindAttribute("RunAnimationTilEnd"))
                animationstopbehaviour = (string(innerNode->Attribute("RunAnimationTilEnd")) == "1") ? eAnimationStopBehaviour_RunAnimationTillEnd : eAnimationStopBehaviour_StopImmediatelly;
            lockInvolvedLamps = (string(innerNode->Attribute("LockInvolvedLamps")) == "1");
            if (innerNode->FindAttribute("HideScoreDisplays"))
                hidescoredisplays = (string(innerNode->Attribute("HideScoreDisplays")) == "1");
            if (innerNode->FindAttribute("BringToFront"))
                bringtofront = (string(innerNode->Attribute("BringToFront")) == "1");
            if (innerNode->FindAttribute("RandomStart"))
                randomstart = (string(innerNode->Attribute("RandomStart")) == "1");
            if (randomstart && innerNode->FindAttribute("RandomQuality"))
                randomquality = innerNode->IntAttribute("RandomQuality");
            if (lightsStateAtAnimationStart == eLightsStateAtAnimationStart_Undefined)
               lightsStateAtAnimationStart = eLightsStateAtAnimationStart_NoChange;
            if (lightsStateAtAnimationEnd == eLightsStateAtAnimationEnd_Undefined)
               lightsStateAtAnimationEnd = eLightsStateAtAnimationEnd_InvolvedLightsOff;
            if (animationstopbehaviour == eAnimationStopBehaviour_Undefined)
               animationstopbehaviour = eAnimationStopBehaviour_StopImmediatelly;
            vector<PictureBoxAnimationEntry*> entries;
            for (auto stepnode = innerNode->FirstChildElement("AnimationStep"); stepnode != nullptr; stepnode = stepnode->NextSiblingElement("AnimationStep")) {
               int step = stepnode->IntAttribute("Step");
               string on = stepnode->Attribute("On");
               int waitLoopsAfterOn = stepnode->IntAttribute("WaitLoopsAfterOn");
               string off = stepnode->Attribute("Off");
               int waitLoopsAfterOff = stepnode->IntAttribute("WaitLoopsAfterOff");
               int pulseswitch = 0;
               if (stepnode->FindAttribute("PulseSwitch"))
                  pulseswitch = stepnode->IntAttribute("PulseSwitch");
               entries.push_back(new PictureBoxAnimationEntry(on, waitLoopsAfterOn, off, waitLoopsAfterOff, pulseswitch));
            }
            // maybe add animation
            if (interval > 0 && entries.size() > 0) {
                m_pB2SAnimation->AddAnimation(name, this, m_pFormDMD, dualmode, interval, loops, startAnimationAtBackglassStartup, 
                   lightsStateAtAnimationStart, lightsStateAtAnimationEnd, animationstopbehaviour, lockInvolvedLamps, hidescoredisplays,
                   bringtofront, randomstart, randomquality, entries);
                // maybe set slowdown
                if (m_pB2SSettings->GetAnimationSlowDowns()->contains(name))
                   m_pB2SAnimation->SetAnimationSlowDown(name, (*m_pB2SSettings->GetAnimationSlowDowns())[name]);
                // add join to ID
                if (!idJoins.empty()) {
                   std::istringstream iss(idJoins);
                   for (string idJoin; std::getline(iss, idJoin, ',');) {
                      if (!idJoin.empty()) {
                         int id0 = 0;
                         int id1 = 0;
                         int id2 = 0;
                         int id3 = 0;
                         if (idJoin.length() >= 1 && is_string_numeric(idJoin))
                            id0 = std::stoi(idJoin);
                         if (idJoin.length() >= 2 && is_string_numeric(idJoin.substr(1)))
                            id1 = std::stoi(idJoin.substr(1));
                         if (idJoin.length() >= 3 && is_string_numeric(idJoin.substr(2)))
                            id2 = std::stoi(idJoin.substr(2));
                         if (idJoin.length() >= 4 && is_string_numeric(idJoin.substr(3)))
                            id3 = std::stoi(idJoin.substr(3));
                         if (string_starts_with_case_insensitive(idJoin, "L"s)) {
                            AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationLampIDs() : m_pB2SData->GetUsedAnimationLampIDs();
                            if (id1 > 0)
                               for (int i = 1; i <= randomquality; i++)
                                  pAnimations->Add(id1, new AnimationInfo(name, false));
                         }
                         else if (string_starts_with_case_insensitive(idJoin, "S"s)) {
                            AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationSolenoidIDs() : m_pB2SData->GetUsedAnimationSolenoidIDs();
                            if (id1 > 0)
                               for (int i = 1; i <= randomquality; i++)
                                  pAnimations->Add(id1, new AnimationInfo(name, false));
                         }
                         else if (string_starts_with_case_insensitive(idJoin, "G"s)) {
                            AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationGIStringIDs() : m_pB2SData->GetUsedAnimationGIStringIDs();
                            if (string_starts_with_case_insensitive(idJoin, "GI"s)) {
                               if (id2 > 0)
                                  pAnimations->Add(id2, new AnimationInfo(name, false));
                            }
                            else {
                               if (id1 > 0)
                                  pAnimations->Add(id1, new AnimationInfo(name, false));
                            }
                         }
                         else if (string_starts_with_case_insensitive(idJoin, "I"s)) {
                             if (string_starts_with_case_insensitive(idJoin, "IL"s)) {
                                AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationLampIDs() : m_pB2SData->GetUsedAnimationLampIDs();
                                if (id2 > 0)
                                   pAnimations->Add(id2, new AnimationInfo(name, true));
                             }
                             else if (string_starts_with_case_insensitive(idJoin, "IS"s)) {
                                AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationSolenoidIDs() : m_pB2SData->GetUsedAnimationSolenoidIDs();
                                if (id2 > 0)
                                   pAnimations->Add(id2, new AnimationInfo(name, true));
                             }
                             else if (string_starts_with_case_insensitive(idJoin, "IG"s)) {
                                AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationGIStringIDs() : m_pB2SData->GetUsedAnimationGIStringIDs();
                                if (string_starts_with_case_insensitive(idJoin, "IGI"s)) {
                                   if (id3 > 0)
                                      pAnimations->Add(id3, new AnimationInfo(name, true));
                                }
                                else {
                                   if (id2 > 0)
                                      pAnimations->Add(id2, new AnimationInfo(name, true));
                                }
                             }
                             else {
                                AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationLampIDs() : m_pB2SData->GetUsedAnimationLampIDs();
                                if (id1 > 0)
                                   pAnimations->Add(id1, new AnimationInfo(name, true));
                             }
                         }
                         else {
                            AnimationCollection* pAnimations = randomstart ? m_pB2SData->GetUsedRandomAnimationLampIDs() : m_pB2SData->GetUsedAnimationLampIDs();
                            if (id0 > 0)
                               pAnimations->Add(id0, new AnimationInfo(name, false));
                         }
                      }
                   }
                }
            }
         }
      }

      m_pB2SData->SetValid(true);
   }

   catch (...) {
   }
}

void FormBackglass::InitB2SScreen()
{
   // initialize screen settings
   if (m_pFormDMD) {
      if (m_pB2SData->GetDMDType() == eDMDType_B2SAlwaysOnSecondMonitor)
         m_pB2SScreen->Start(this, m_pFormDMD, m_pB2SData->GetDMDDefaultLocation(), eDMDViewMode_ShowDMDOnlyAtDefaultLocation, m_pB2SData->GetGrillHeight(), m_pB2SData->GetSmallGrillHeight());
      else if (m_pB2SData->GetDMDType() == eDMDType_B2SAlwaysOnThirdMonitor)
         m_pB2SScreen->Start(this, m_pFormDMD, m_pB2SData->GetDMDDefaultLocation(), eDMDViewMode_DoNotShowDMDAtDefaultLocation, m_pB2SData->GetGrillHeight(), m_pB2SData->GetSmallGrillHeight());
      else if (m_pB2SData->GetDMDType() == eDMDType_B2SOnSecondOrThirdMonitor)
         m_pB2SScreen->Start(this, m_pFormDMD, m_pB2SData->GetDMDDefaultLocation(), eDMDViewMode_ShowDMD, m_pB2SData->GetGrillHeight(), m_pB2SData->GetSmallGrillHeight());
      else
         m_pB2SScreen->Start(this, m_pB2SData->GetGrillHeight(), m_pB2SData->GetSmallGrillHeight());
   }
   else
      m_pB2SScreen->Start(this, m_pB2SData->GetGrillHeight(), m_pB2SData->GetSmallGrillHeight());
}

void FormBackglass::ResizeSomeImages()
{
   float xResizeFactor = 1.0f;
   float yResizeFactor = 1.0f;
   if (m_pDarkImage4Authentic) {
      int width = m_pDarkImage4Authentic->w;
      int height = m_pDarkImage4Authentic->h;
      SDL_Surface* pImage = ResizeSurface(m_pDarkImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pDarkImage4Authentic);
      m_pDarkImage4Authentic = pImage;
      xResizeFactor = (float)width / (float)m_pDarkImage4Authentic->w;
      yResizeFactor = (float)height / (float)m_pDarkImage4Authentic->h;
   }
   if (m_pDarkImage4Fantasy)
      m_pDarkImage4Fantasy = m_pDarkImage4Authentic;
   if (m_pTopLightImage4Authentic) {
      SDL_Surface* pImage = ResizeSurface(m_pTopLightImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pTopLightImage4Authentic);
      m_pTopLightImage4Authentic = pImage;
   }
   if (m_pTopLightImage4Fantasy) {
      SDL_Surface* pImage = ResizeSurface(m_pTopLightImage4Fantasy, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pTopLightImage4Fantasy);
      m_pTopLightImage4Fantasy = pImage;
   }
   if (m_pSecondLightImage4Authentic) {
      SDL_Surface* pImage = ResizeSurface(m_pSecondLightImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pSecondLightImage4Authentic);
      m_pSecondLightImage4Authentic = pImage;
   }
   if (m_pSecondLightImage4Fantasy) {
      SDL_Surface* pImage = ResizeSurface(m_pSecondLightImage4Fantasy, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pSecondLightImage4Fantasy);
      m_pSecondLightImage4Fantasy = pImage;
   }
   if (m_pTopAndSecondLightImage4Authentic) {
      SDL_Surface* pImage = ResizeSurface(m_pTopAndSecondLightImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pTopAndSecondLightImage4Authentic);
      m_pTopAndSecondLightImage4Authentic = pImage;
   }
   if (m_pTopAndSecondLightImage4Fantasy) {
      SDL_Surface* pImage = ResizeSurface(m_pTopAndSecondLightImage4Fantasy, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h);
      SDL_FreeSurface(m_pTopAndSecondLightImage4Fantasy);
      m_pTopAndSecondLightImage4Fantasy = pImage;
   }
   SetBackgroundImage(GetDarkImage());

   // now resize the detail images
   if (xResizeFactor != 1.0f || yResizeFactor != 1.0f) {
      for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
         if (pPicbox->GetPictureBoxType() == ePictureBoxType_StandardImage) {
            if (pPicbox->GetBackgroundImage()) {
               SDL_FRect frect = { 0.0f, 0.0f, pPicbox->GetBackgroundImage()->w / xResizeFactor, pPicbox->GetBackgroundImage()->h / yResizeFactor };
               SDL_Rect rect = { 0, 0, (int)frect.w, (int)frect.h };
               SDL_Surface* pImage = ResizeSurface(pPicbox->GetBackgroundImage(), rect.w, rect.h);
               SDL_FreeSurface(pPicbox->GetBackgroundImage());
               pPicbox->SetBackgroundImage(pImage);
               if (pPicbox->GetOffImage()) {
                  SDL_Surface* pOffImage = ResizeSurface(pPicbox->GetOffImage(), rect.w, rect.h);
                  SDL_FreeSurface(pPicbox->GetOffImage());
                  pPicbox->SetOffImage(pImage);
               }
            }
         }
      }
   }
}

void FormBackglass::ShowStartupSnippits()
{
   // maybe show some 'startup on' snippits
   for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
      if (pPicbox->GetInitialState() == 1 && pPicbox->IsImageSnippit())
         pPicbox->SetVisible(true);
   }
}

void FormBackglass::ShowStartupImages()
{
   // maybe show some 'startup on' images
   bool topIsOn = false;
   for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
      if (pPicbox->GetInitialState() == 1 && !pPicbox->IsImageSnippit()) {
         if (GetTopRomID() > 0 && pPicbox->GetRomID() == GetTopRomID() && pPicbox->GetRomIDType() == GetTopRomIDType() && pPicbox->IsRomInverted() == IsTopRomInverted()) {
            topIsOn = true;
            if (GetTopLightImage() && GetTopLightImage() != GetBackgroundImage())
               SetBackgroundImage(GetTopLightImage());
         }
         else if (!topIsOn && GetSecondRomID() > 0 && pPicbox->GetRomID() == GetSecondRomID() && pPicbox->GetRomIDType() == GetSecondRomIDType() && pPicbox->IsRomInverted() == IsSecondRomInverted()) {
            if (GetSecondLightImage() && GetSecondLightImage() != GetBackgroundImage())
               SetBackgroundImage(GetSecondLightImage());
         }
         else
            pPicbox->SetVisible(true);
      }
   }
}

void FormBackglass::RotateImage(B2SPictureBox* pPicbox, int rotationsteps, eSnippitRotationDirection rotationdirection, ePictureBoxType type)
{
   eRomIDType romidtype = eRomIDType_NotDefined;
   int romid = 0;

   RotateImage(pPicbox, rotationsteps, rotationdirection, type, romidtype, romid);
}

void FormBackglass::RotateImage(B2SPictureBox* pPicbox, int rotationsteps, eSnippitRotationDirection rotationdirection, ePictureBoxType type, eRomIDType& romidtype, int& romid)
{
   if (pPicbox && rotationsteps > 0) {
      // store some data
      if (romid == 0)
         m_pB2SData->SetUseRotatingImage(true);
      else
         m_pB2SData->SetUseMechRotatingImage(true);

      (*m_pB2SData->GetRotatingPictureBox())[romid] = pPicbox;

      // calc rotation angle
      m_rotateAngle = 360 / rotationsteps;
      //assert(360%rotationsteps == 0);

      // rotate the image the whole circle
      int rotatingAngle = 0;
      int index = 0;
      while (rotatingAngle < 360) {
         SDL_Surface* pImage = RotateSurface(pPicbox->GetBackgroundImage(), rotationdirection == eSnippitRotationDirection_AntiClockwise ? rotatingAngle : 360 - rotatingAngle);
         (*m_pB2SData->GetRotatingImages())[romid][pPicbox->GetPictureBoxType() == ePictureBoxType_MechRotatingImage ? index : rotatingAngle] = pImage;
         rotatingAngle += m_rotateAngle;
         index++;
      }
   }
}

void FormBackglass::CheckDMDForm()
{
   if (!m_pFormDMD && !m_pB2SSettings->IsHideB2SDMD()) {
      m_pFormDMD = new FormDMD();
   }
}

SDL_Surface* FormBackglass::CreateLightImage(SDL_Surface* image, eDualMode dualmode, const string& firstromkey_)
{
   return CreateLightImage(image, dualmode, firstromkey_, string());
}

SDL_Surface* FormBackglass::CreateLightImage(SDL_Surface* image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_)
{
   int romid = 0;
   eRomIDType romidtype = eRomIDType_NotDefined;
   bool rominverted = false;
   return CreateLightImage(image, dualmode, firstromkey_, secondromkey_, romid, romidtype, rominverted);
}

SDL_Surface* FormBackglass::CreateLightImage(SDL_Surface* image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_, int& romid, eRomIDType& romidtype, bool& rominverted)
{
   string firstromkey = firstromkey_;
   string secondromkey = secondromkey_;

   int secondromid = 0;
   eRomIDType secondromidtype = eRomIDType_NotDefined;
   bool secondrominverted = false;
   if (firstromkey.substr(0, 1) == "I") {
      rominverted = true;
      firstromkey = firstromkey.substr(1);
   }
   romidtype = (firstromkey.substr(0, 1) == "S" ? eRomIDType_Solenoid : (firstromkey.substr(0, 2) == "GI" ? eRomIDType_GIString : eRomIDType_Lamp));
   romid = std::stoi((romidtype == eRomIDType_GIString ? firstromkey.substr(2) : firstromkey.substr(1)));
   if (!secondromkey.empty()) {
      if (secondromkey.substr(0, 1) == "I") {
         secondrominverted = true;
         secondromkey = secondromkey.substr(1);
      }
      secondromidtype = (secondromkey.substr(0, 1) == "S" ? eRomIDType_Solenoid : (secondromkey.substr(0, 2) == "GI" ? eRomIDType_GIString : eRomIDType_Lamp));
      secondromid = std::stoi((secondromidtype == eRomIDType_GIString ? secondromkey.substr(2) : secondromkey.substr(1)));
   }
   // create image copy
   SDL_Surface* pImage = ResizeSurface(image, image->w, image->h);
   // draw matching bulbs into image
   for (auto& pControl : *GetControls()) {
      if (auto pPicbox = dynamic_cast<B2SPictureBox*>(pControl)) {
         if (pPicbox->GetRomID() == romid && pPicbox->GetRomIDType() == romidtype && pPicbox->IsRomInverted() == rominverted && (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == dualmode)) {
            SDL_Rect rect = { pPicbox->GetLocation().x, pPicbox->GetLocation().y, pPicbox->GetSize().w, pPicbox->GetSize().h };
            SDL_BlitScaled(pPicbox->GetBackgroundImage(), NULL, pImage, &rect);
         }
      }
   }
   // maybe draw second matching bulbs into image
   if (!secondromkey.empty()) {
      for (auto& pControl : *GetControls()) {
         if (auto pPicbox = dynamic_cast<B2SPictureBox*>(pControl)) {
            if (pPicbox->GetRomID() == secondromid && pPicbox->GetRomIDType() == secondromidtype && pPicbox->IsRomInverted() == secondrominverted && (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == dualmode)) {
               SDL_Rect rect = { pPicbox->GetLocation().x, pPicbox->GetLocation().y, pPicbox->GetSize().w, pPicbox->GetSize().h };
               SDL_BlitScaled(pPicbox->GetBackgroundImage(), NULL, pImage, &rect);
            }
         }
      }
   }
   return pImage;
}

void FormBackglass::CheckBulbs(int romid, eRomIDType romidtype, bool rominverted, eDualMode dualmode)
{
   if (romid > 0 && romidtype != eRomIDType_NotDefined) {
      std::map<int, vector<B2SBaseBox*>>* pUsedRomIDs = NULL;
      if (romidtype == eRomIDType_Lamp)
         pUsedRomIDs = (dualmode == eDualMode_Fantasy ? m_pB2SData->GetUsedRomLampIDs4Fantasy() : m_pB2SData->GetUsedRomLampIDs4Authentic());
      else if (romidtype == eRomIDType_Solenoid)
         pUsedRomIDs = (dualmode == eDualMode_Fantasy ? m_pB2SData->GetUsedRomSolenoidIDs4Fantasy() : m_pB2SData->GetUsedRomSolenoidIDs4Authentic());
      else if (romidtype == eRomIDType_GIString)
         pUsedRomIDs = (dualmode == eDualMode_Fantasy ? m_pB2SData->GetUsedRomGIStringIDs4Fantasy() : m_pB2SData->GetUsedRomGIStringIDs4Authentic());
      if (pUsedRomIDs->find(romid) != pUsedRomIDs->end()) {
         pUsedRomIDs->erase(romid);
         for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
            if (pPicbox->GetRomID() == romid && pPicbox->GetRomIDType() == romidtype && pPicbox->IsRomInverted() != rominverted && (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == dualmode))
               m_pB2SData->GetIlluminations()->Add(pPicbox, dualmode);
         }
      }
   }
}

SDL_Surface* FormBackglass::RotateSurface(SDL_Surface* source, int angle)
{
   SDL_LockSurface(source);

   SDL_Surface* const destination = SDL_CreateRGBSurface(0, source->w, source->h, source->format->BitsPerPixel, source->format->Rmask, source->format->Gmask, source->format->Bmask, source->format->Amask);

   const float radians = -(float)angle * (float)(M_PI / 180.0);
   const float cosine = cosf(radians);
   const float sine = sinf(radians);

   const float center_x = destination->w / 2.0f;
   const float center_y = destination->h / 2.0f;

   const UINT32* const __restrict src = ((UINT32*)source->pixels);
   UINT32* const __restrict dest = ((UINT32*)destination->pixels);

   for (int y = 0; y < destination->h; ++y) {
      const float xoffs = center_x - center_x * cosine + ((float)y - center_y) * sine;
      const float yoffs = center_y + center_x * sine   + ((float)y - center_y) * cosine;

      for (int x = 0; x < destination->w; ++x) {
         const int old_x = (int)(round(xoffs + (float)x * cosine));
         const int old_y = (int)(round(yoffs - (float)x * sine));

         if (/*old_x >= 0 &&*/ (unsigned int)old_x < (unsigned int)source->w && /*old_y >= 0 &&*/ (unsigned int)old_y < (unsigned int)source->h)
            dest[y * destination->w + x] = src[old_y * source->w + old_x];
         else
            dest[y * destination->w + x] = 0; //!!?
      }
   }

   SDL_UnlockSurface(source);

   return destination;
}

SDL_Surface* FormBackglass::ResizeSurface(SDL_Surface* original, int newWidth, int newHeight)
{
    SDL_Surface* newSurface = SDL_CreateRGBSurface(0, newWidth, newHeight, original->format->BitsPerPixel,
       original->format->Rmask, original->format->Gmask, original->format->Bmask, original->format->Amask);

    if (!newSurface)
       return NULL;

    SDL_BlitScaled(original, NULL, newSurface, NULL);

    return newSurface;
}

SDL_Surface* FormBackglass::Base64ToImage(const string& image)
{
   vector<unsigned char> imageData = base64_decode(image);
   SDL_RWops* rwops = SDL_RWFromConstMem(imageData.data(), imageData.size());

   if (!rwops)
      return NULL;

   SDL_Surface* pImage = IMG_Load_RW(rwops, 0);
   SDL_RWclose(rwops);

   return pImage;
}

Sound* FormBackglass::Base64ToWav(const string& data)
{
   return new Sound(base64_decode(data));
}

OLE_COLOR FormBackglass::String2Color(const string& color)
{
   std::istringstream ss(color);
   string token;
   vector<int> colorValues;

   while (std::getline(ss, token, '.'))
      colorValues.push_back(std::stoi(token));

   if (colorValues.size() == 3)
      return RGB(colorValues[0], colorValues[1], colorValues[2]);

   return RGB(0, 0, 0);
}
