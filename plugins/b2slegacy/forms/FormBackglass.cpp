#include "FormBackglass.h"

#include <fstream>
#include <cmath>
#include <tinyxml2/tinyxml2.h>

#include "FormDMD.h"
#include "../common.h"
#include "../Server.h"
#include "../utils/DMDOverlay.h"
#include "../classes/B2SVersionInfo.h"
#include "../classes/B2SScreen.h"
#include "../classes/B2SAnimation.h"
#include "../classes/AnimationInfo.h"
#include "../classes/B2SReelDisplay.h"
#include "../classes/LEDDisplayDigitLocation.h"
#include "../classes/PictureBoxAnimationEntry.h"
#include "../classes/ControlInfo.h"
#include "../classes/LEDAreaInfo.h"
#include "../controls/B2SPictureBox.h"
#include "../controls/B2SLEDBox.h"
#include "../controls/B2SReelBox.h"
#include "../dream7/Dream7Display.h"
#include "../utils/VPXGraphics.h"
#include "../utils/VPXGraphics.h"

namespace B2SLegacy {

#include <exception>

FormBackglass::FormBackglass(VPXPluginAPI* vpxApi, MsgPluginAPI* msgApi,uint32_t endpointId, B2SData* pB2SData)
   : Form(vpxApi, msgApi, endpointId, pB2SData, "Backglass"s),
     m_pB2SSettings(pB2SData->GetB2SSettings())
{
   SetName("formBackglass"s);

   m_pFormDMD = nullptr;
   m_pStartupTimer = nullptr;
   m_pRotateTimer = nullptr;
   m_rotateSlowDownSteps = 0;
   m_rotateRunTillEnd = false;
   m_rotateRunToFirstStep = false;
   m_rotateAngle = 0;
   m_rotateTimerInterval = 0;
   m_selectedLEDType = eLEDTypes_Undefined;
   m_pDarkImage4Authentic = nullptr;
   m_pDarkImage4Fantasy = nullptr;
   m_pTopLightImage4Authentic = nullptr;
   m_pTopLightImage4Fantasy = nullptr;
   m_pSecondLightImage4Authentic = nullptr;
   m_pSecondLightImage4Fantasy = nullptr;
   m_pTopAndSecondLightImage4Authentic = nullptr;
   m_pTopAndSecondLightImage4Fantasy = nullptr;
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
   m_pB2SScreen = new B2SScreen(m_pB2SData, m_msgApi, m_vpxApi, endpointId);

   // load settings
   m_pB2SSettings->Load();

   // get B2S xml and start
   try {
      LoadB2SData();
   }

   catch(...) {
      return;
   }

   m_valid = true;
}

void FormBackglass::Start() {
   if (!m_valid)
      return;

   // initialize screen settings
   InitB2SScreen();

   // Do not resize images yet. First render we will know the size of the backglass
   ResizeSomeImages();

   // show startup snippits
   ShowStartupSnippits();

   // create 'image on' timer and start it
   m_pStartupTimer = new Timer(2000, std::bind(&FormBackglass::StartupTimerTick, this, std::placeholders::_1));
   m_pStartupTimer->Start();

   // create rotation timer
   m_pRotateTimer = new Timer(m_rotateTimerInterval, std::bind(&FormBackglass::RotateTimerTick, this, std::placeholders::_1));
}

FormBackglass::~FormBackglass()
{
   delete m_pStartupTimer;
   delete m_pRotateTimer;
   delete m_pB2SAnimation;
   delete m_pFormDMD;
   delete m_pB2SScreen;

   if (m_pDarkImage4Authentic)
      m_vpxApi->DeleteTexture(m_pDarkImage4Authentic);
   if (m_pTopLightImage4Authentic)
      m_vpxApi->DeleteTexture(m_pTopLightImage4Authentic);
   if (m_pSecondLightImage4Authentic)
      m_vpxApi->DeleteTexture(m_pSecondLightImage4Authentic);
   if (m_pTopAndSecondLightImage4Authentic)
      m_vpxApi->DeleteTexture(m_pTopAndSecondLightImage4Authentic);
   if (m_pDarkImage4Fantasy)
      m_vpxApi->DeleteTexture(m_pDarkImage4Fantasy);
   if (m_pTopLightImage4Fantasy)
      m_vpxApi->DeleteTexture(m_pTopLightImage4Fantasy);
   if (m_pSecondLightImage4Fantasy)
      m_vpxApi->DeleteTexture(m_pSecondLightImage4Fantasy);
   if (m_pTopAndSecondLightImage4Fantasy)
      m_vpxApi->DeleteTexture(m_pTopAndSecondLightImage4Fantasy);
}

void FormBackglass::OnPaint(VPXRenderContext2D* const ctx)
{
   if (m_pB2SSettings->IsHideB2SBackglass())
      return;

   if (IsVisible() && GetBackgroundImage()) {
      // draw background image
      VPXGraphics::DrawImage(m_vpxApi, ctx, GetBackgroundImage(), NULL, NULL);
      // draw all visible and necessary images
      if (!m_pB2SData->GetIlluminations()->empty()) {
         if (!m_pB2SData->IsUseZOrder()) {
            // draw all standard images
            for(const auto& [key, pIllu] : *m_pB2SData->GetIlluminations())
               DrawImage(ctx, pIllu);
         }
         else {
            // first of all draw all standard images
            for(const auto& [key, pIllu] : *m_pB2SData->GetIlluminations()) {
               if (pIllu->GetZOrder() == 0)
                  DrawImage(ctx, pIllu);
            }
            // now draw z-ordered images
            for(const auto& [key, pIllus] : *m_pB2SData->GetZOrderImages()) {
               for (size_t i = 0; i < pIllus.size(); i++)
                  DrawImage(ctx, pIllus[i]);
            }
         }
      }
   }

   Form::OnPaint(ctx);
}


void FormBackglass::DrawImage(VPXRenderContext2D* const ctx, B2SPictureBox* pPicbox)
{
   if (pPicbox) {
      SDL_FRect rectf = pPicbox->GetRectangleF();
      SDL_Rect rect = { (int)rectf.x, (int)rectf.y, (int)rectf.w, (int)rectf.h };

      bool drawme = (!m_pB2SSettings->IsAllOut() && pPicbox->IsVisible());
      if (drawme && !SDL_RectEmpty(&m_pB2SScreen->GetBackglassCutOff()))
         drawme = !SDL_HasRectIntersection(&m_pB2SScreen->GetBackglassCutOff(), &rect);
      if (drawme && pPicbox->GetRomID() != 0 && !pPicbox->IsSetThruAnimation())
         drawme = (pPicbox->GetRomID() != GetTopRomID() || pPicbox->GetRomIDType() != GetTopRomIDType() || pPicbox->IsRomInverted() != IsTopRomInverted()) && (pPicbox->GetRomID() != GetSecondRomID() || pPicbox->GetRomIDType() != GetSecondRomIDType() || pPicbox->IsRomInverted() != IsSecondRomInverted());
      if (drawme && m_pB2SData->IsDualBackglass())
         drawme = (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == (eDualMode)m_pB2SSettings->GetCurrentDualMode());

      if (drawme) {
         if (m_pB2SData->IsOnAndOffImage()) {
            if (m_pB2SData->IsOffImageVisible() && pPicbox->GetOffImage())
               VPXGraphics::DrawImage(m_vpxApi, ctx, pPicbox->GetOffImage(), NULL, &rect);
         }
         else
            VPXGraphics::DrawImage(m_vpxApi, ctx, pPicbox->GetBackgroundImage(), NULL, &rect);
      }
   }
}

void FormBackglass::StartupTimerTick(Timer* pTimer)
{
   m_pStartupTimer->Stop();

   // maybe show some 'startup on' images
   ShowStartupImages();

   // start autostarted animations
   m_pB2SAnimation->AutoStart();
}

void FormBackglass::RotateTimerTick(Timer* pTimer)
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
   else
      m_pRotateTimer->Stop();

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
   if (m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && m_pB2SData->GetRotatingImages() && !(*m_pB2SData->GetRotatingImages())[0].empty()) {
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
   if (m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && m_pB2SData->GetRotatingImages() && !(*m_pB2SData->GetRotatingImages())[0].empty()) {
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

eLEDTypes FormBackglass::GetLEDType() const
{
   eLEDTypes ret = eLEDTypes_Undefined;
   if (!m_pB2SData->GetLEDDisplays()->empty()) {
      for (const auto& [key, pDisplay] : *m_pB2SData->GetLEDDisplays()) {
         if (pDisplay->IsVisible()) {
            ret = eLEDTypes_Dream7;
            break;
         }
      }
   }
   else if (!m_pB2SData->GetLEDs()->empty()) {
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
   LOGW("Not implemented");
}

void FormBackglass::StopSound(const string& szSoundName)
{
   LOGW("Not implemented");
}

const SDL_FRect& FormBackglass::GetScaleFactor() const
{
   return m_pB2SScreen->GetRescaleBackglass();
}

void FormBackglass::LoadB2SData()
{
   const std::filesystem::path tablePath(m_pB2SData->GetTableFileName());
   const std::filesystem::path b2sFilename = find_case_insensitive_file_path(tablePath.parent_path() / tablePath.filename().replace_extension(".directb2s"));
   if (b2sFilename.empty()) {
      LOGD("No directb2s file found");
      throw std::exception();
   }

   LOGI("directb2s file found at: %s", b2sFilename.string().c_str());

   m_pB2SData->SetBackglassFileName(b2sFilename.string());

   std::ifstream infile(b2sFilename);
   if (!infile.good())
      throw std::exception();

   tinyxml2::XMLDocument b2sTree;
   std::stringstream buffer;
   std::ifstream myFile(b2sFilename);
   buffer << myFile.rdbuf();
   myFile.close();

   auto xml = buffer.str();
   if (b2sTree.Parse(xml.c_str(), xml.size())) {
      LOGE("Failed to parse directb2s file: %s", b2sFilename.string().c_str());
      throw std::exception();
   }

   if (!b2sTree.FirstChildElement("DirectB2SData")) {
      LOGE("Invalid directb2s file: %s", b2sFilename.string().c_str());
      throw std::exception();
   }

   m_pB2SSettings->SetBackglassFileVersion(b2sTree.FirstChildElement("DirectB2SData")->Attribute("Version"));

   // current backglass version is not allowed to be larger than server version and to be smaller minimum B2S version
   if (m_pB2SSettings->GetBackglassFileVersion() > string(B2S_VERSION_STRING)) {
      LOGE("B2S backglass server version (%s) doesn't match directb2s file version (%s). Please update the B2S backglass server.",
         B2S_VERSION_STRING, m_pB2SSettings->GetBackglassFileVersion().c_str());
      return;
   }
   else if (m_pB2SSettings->GetBackglassFileVersion() < m_pB2SSettings->GetMinimumDirectB2SVersion()) {
      LOGE("directb2s file version (%s) doesn't match minimum directb2s version. Please update the directb2s backglass file.",
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
   m_pB2SData->SetGrillHeight(std::max(topnode->FirstChildElement("GrillHeight")->IntAttribute("Value"), 0));
   if (topnode->FirstChildElement("GrillHeight")->FindAttribute("Small") && m_pB2SData->GetGrillHeight() > 0)
      m_pB2SData->SetSmallGrillHeight(std::max(topnode->FirstChildElement("GrillHeight")->IntAttribute("Small"), 0));
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
         //bool visible = (innerNode->IntAttribute("Visible") == 1);
         SDL_Point loc = { innerNode->IntAttribute("LocX"), innerNode->IntAttribute("LocY") };
         SDL_Rect size = { 0, 0, innerNode->IntAttribute("Width"), innerNode->IntAttribute("Height") };
         VPXTexture pImage = Base64ToImage(innerNode->Attribute("Image"));
         VPXTexture pOffImage = nullptr;
         if (innerNode->FindAttribute("OffImage"))
            pOffImage = Base64ToImage(innerNode->Attribute("OffImage"));
         if (picboxtype == ePictureBoxType_StandardImage) {
            // Events of overlapping pictures get merged #76, crop image transparency
            VPXTexture pCroppedTexture = CropImageToTransparency(pImage, pOffImage, loc, size);
            if (pCroppedTexture) {
               if (pImage)
                  m_vpxApi->DeleteTexture(pImage);
               pImage = pCroppedTexture;
            }
         }
         B2SPictureBox* pPicbox = new B2SPictureBox(m_vpxApi, m_pB2SData);
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
            if (picboxtype == ePictureBoxType_SelfRotatingImage && m_pB2SData->GetRotatingImages()->empty()) {
               m_rotateTimerInterval = picboxrotateinterval;
               RotateImage(pPicbox, picboxrotatesteps, picboxrotationdirection, ePictureBoxType_SelfRotatingImage);
            }
            else if (picboxtype == ePictureBoxType_MechRotatingImage && m_pB2SData->GetRotatingImages()->empty()) {
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
         uint32_t reellitcolor = String2Color(innerNode->Attribute("ReelLitColor"));
         uint32_t reeldarkcolor = String2Color(innerNode->Attribute("ReelDarkColor"));
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
         float glow = d7glow;
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
            Dream7Display* pLed = new Dream7Display(m_vpxApi);
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
            pLed->SetGlassColor(RGB(std::min(GetRValue(reellitcolor) + 50, 255), std::min(GetGValue(reellitcolor) + 50, 255), std::min(GetBValue(reellitcolor) + 50, 255)));
            pLed->SetGlassColorCenter(RGB(std::min(GetRValue(reellitcolor) + 70, 255), std::min(GetGValue(reellitcolor) + 70, 255), std::min(GetBValue(reellitcolor) + 70, 255)));
            pLed->SetOffColor(reeldarkcolor);
            pLed->SetBackColor(RGB(15, 15, 15));
            pLed->SetGlassAlpha(140);
            pLed->SetGlassAlphaCenter(255);
            pLed->SetThickness(d7thickness * 1.2f);
            pLed->SetShear(d7shear);
            pLed->SetGlow(glow < 3.f ? glow : 3.f);
            if (!SDL_RectEmptyFloat(&glowbulb))
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
               B2SLEDBox* pLed = new B2SLEDBox(m_vpxApi, m_pB2SData);
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
               B2SReelBox* pReel = new B2SReelBox(m_vpxApi, m_pB2SData);
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
            VPXTexture pImage = Base64ToImage(innerNode->Attribute("Image"));
            if (!m_pB2SData->GetReelImages()->contains(name))
               (*m_pB2SData->GetReelImages())[name] = pImage;
            else
               m_vpxApi->DeleteTexture(pImage);
         }
      }
      else if (topnode->FirstChildElement("Reels")->FirstChildElement("Images") && topnode->FirstChildElement("Reels")->FirstChildElement("Images")->FirstChildElement("Image")) {
         for (auto innerNode = topnode->FirstChildElement("Reels")->FirstChildElement("Images")->FirstChildElement("Image");  innerNode != nullptr; innerNode = innerNode->NextSiblingElement("Image")) {
            string name = innerNode->Attribute("Name");
            VPXTexture pImage = Base64ToImage(innerNode->Attribute("Image"));
            if (!m_pB2SData->GetReelImages()->contains(name))
               (*m_pB2SData->GetReelImages())[name] = pImage;
            else
               m_vpxApi->DeleteTexture(pImage);
            // maybe get the intermediate reel images
            if (innerNode->FindAttribute("CountOfIntermediates")) {
               int countOfIntermediates = innerNode->IntAttribute("CountOfIntermediates");
               for (int i = 1; i <= countOfIntermediates; i++) {
                  string intname = name + '_' + std::to_string(i);
                  VPXTexture pIntImage = Base64ToImage(innerNode->Attribute(("IntermediateImage" + std::to_string(i)).c_str()));
                  if (!m_pB2SData->GetReelIntermediateImages()->contains(intname))
                     (*m_pB2SData->GetReelIntermediateImages())[intname] = pIntImage;
                  else
                     m_vpxApi->DeleteTexture(pIntImage);
               }
            }
         }

         if (topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")) {
            if (topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("IlluminatedImage")) {
               for (auto innerNode = topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("IlluminatedImage"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("IlluminatedImage")) {
                  string name = innerNode->Attribute("Name");
                  VPXTexture pImage = Base64ToImage(innerNode->Attribute("Image"));
                  if (!m_pB2SData->GetReelIlluImages()->contains(name))
                     (*m_pB2SData->GetReelIlluImages())[name] = pImage;
                  else
                     m_vpxApi->DeleteTexture(pImage);
                  // maybe get the intermediate reel images
                  if (innerNode->FindAttribute("CountOfIntermediates")) {
                     int countOfIntermediates = innerNode->IntAttribute("CountOfIntermediates");
                     for (int i = 1; i <= countOfIntermediates; i++) {
                        string intname = name + '_' + std::to_string(i);
                        VPXTexture pIntImage = Base64ToImage(innerNode->Attribute(("IntermediateImage" + std::to_string(i)).c_str()));
                        if (!m_pB2SData->GetReelIntermediateIlluImages()->contains(intname))
                           (*m_pB2SData->GetReelIntermediateIlluImages())[intname] = pIntImage;
                        else
                           m_vpxApi->DeleteTexture(pIntImage);
                     }
                  }
               }
            }
            else if (topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("Set")) {
               for (auto setnode = topnode->FirstChildElement("Reels")->FirstChildElement("IlluminatedImages")->FirstChildElement("Set"); setnode != nullptr; setnode = setnode->NextSiblingElement("Set")) {
                  int setid = setnode->IntAttribute("ID");
                  for (auto innerNode = setnode->FirstChildElement("IlluminatedImage"); innerNode != nullptr; innerNode = innerNode->NextSiblingElement("IlluminatedImage")) {
                     string name = string(innerNode->Attribute("Name")) + '_' + std::to_string(setid);
                     VPXTexture pImage = Base64ToImage(innerNode->Attribute("Image"));
                     if (!m_pB2SData->GetReelIlluImages()->contains(name))
                        (*m_pB2SData->GetReelIlluImages())[name] = pImage;
                     else
                        m_vpxApi->DeleteTexture(pImage);
                     // maybe get the intermediate reel images
                     if (innerNode->FindAttribute("CountOfIntermediates")) {
                        int countOfIntermediates = innerNode->IntAttribute("CountOfIntermediates");
                        for (int i = 1; i <= countOfIntermediates; i++) {
                           string intname = name + '_' + std::to_string(i);
                           VPXTexture pIntImage = Base64ToImage(innerNode->Attribute(("IntermediateImage" + std::to_string(i)).c_str()));
                           if (!m_pB2SData->GetReelIntermediateIlluImages()->contains(intname))
                              (*m_pB2SData->GetReelIntermediateIlluImages())[intname] = pIntImage;
                           else
                              m_vpxApi->DeleteTexture(pIntImage);
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
      // backglass image
      if (topnode->FirstChildElement("Images")->FirstChildElement("BackglassOffImage")) {
         m_pB2SData->SetOnAndOffImage(true);
         // get on and off image
         VPXTexture pOffImage = Base64ToImage(topnode->FirstChildElement("Images")->FirstChildElement("BackglassOffImage")->Attribute("Value"));
         if (pOffImage) {
            SetDarkImage4Authentic(VPXGraphics::DuplicateTexture(m_vpxApi, pOffImage));
            if (m_pB2SData->IsDualBackglass())
               SetDarkImage4Fantasy(VPXGraphics::DuplicateTexture(m_vpxApi, pOffImage));
            m_vpxApi->DeleteTexture(pOffImage);
         }
         auto onimagenode = topnode->FirstChildElement("Images")->FirstChildElement("BackglassOnImage");
         if (onimagenode) {
            VPXTexture pOnImage = Base64ToImage(onimagenode->Attribute("Value"));
            if (pOnImage) {
               SetTopLightImage4Authentic(VPXGraphics::DuplicateTexture(m_vpxApi, pOnImage));
               if (m_pB2SData->IsDualBackglass())
                  SetTopLightImage4Fantasy(VPXGraphics::DuplicateTexture(m_vpxApi, pOnImage));
               m_vpxApi->DeleteTexture(pOnImage);
            }
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
         VPXTexture pImage = nullptr;
         if (topnode->FirstChildElement("Images")->FirstChildElement("BackglassImage")) {
            pImage = Base64ToImage(topnode->FirstChildElement("Images")->FirstChildElement("BackglassImage")->Attribute("Value"));
            if (pImage) {
               SetDarkImage4Authentic(VPXGraphics::DuplicateTexture(m_vpxApi, pImage));
               if (m_pB2SData->IsDualBackglass())
                  SetDarkImage4Fantasy(VPXGraphics::DuplicateTexture(m_vpxApi, pImage));
               m_vpxApi->DeleteTexture(pImage);
            }
         }
      }
      // starting image is the dark image
      SetBackgroundImage(GetDarkImage());

      // DMD image
      VPXTexture pImage = nullptr;
      if (topnode->FirstChildElement("Images")->FirstChildElement("DMDImage")) {
         pImage = Base64ToImage(topnode->FirstChildElement("Images")->FirstChildElement("DMDImage")->Attribute("Value"));
         if (pImage) {
            if (!m_pB2SSettings->IsHideB2SDMD()) {
               CheckDMDForm();
               m_pFormDMD->SetBackgroundImage(pImage);
            }
            else
               m_vpxApi->DeleteTexture(pImage);
         }
      }

      // look for the largest bulb amount
      int top4Authentic = 0;
      string topkey4Authentic;
      int second4Authentic = 0;
      string secondkey4Authentic;
      for (const auto& romsize : roms4Authentic) {
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
         for (const auto& romsize : roms4Fantasy) {
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
         if (m_pTopLightImage4Authentic == nullptr) {
            SetTopLightImage4Authentic(CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, topkey4Authentic, ""s, m_topRomID4Authentic, m_topRomIDType4Authentic, m_topRomInverted4Authentic));
            if (second4Authentic > minSize4Image) {
               SetSecondLightImage4Authentic(CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, secondkey4Authentic, ""s, m_secondRomID4Authentic, m_secondRomIDType4Authentic, m_secondRomInverted4Authentic));
               SetTopAndSecondLightImage4Authentic(CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, topkey4Authentic, secondkey4Authentic));
            }
         }
         else {
            SetSecondLightImage4Authentic(CreateLightImage(m_pDarkImage4Authentic, eDualMode_Authentic, topkey4Authentic, ""s, m_secondRomID4Authentic, m_secondRomIDType4Authentic, m_secondRomInverted4Authentic));
            SetTopAndSecondLightImage4Authentic(CreateLightImage(m_pTopLightImage4Authentic, eDualMode_Authentic, topkey4Authentic));
         }
      }
      if (m_pB2SData->IsDualBackglass() && top4Fantasy >= minSize4Image && mergeBulbs) {
         // create some light images
         if (m_pTopLightImage4Fantasy == nullptr) {
            SetTopLightImage4Fantasy(CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy, ""s, m_topRomID4Fantasy, m_topRomIDType4Fantasy, m_topRomInverted4Fantasy));
            if (second4Fantasy > minSize4Image) {
               SetSecondLightImage4Fantasy(CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, secondkey4Fantasy, ""s, m_secondRomID4Fantasy, m_secondRomIDType4Fantasy, m_secondRomInverted4Fantasy));
               SetTopAndSecondLightImage4Fantasy(CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy, secondkey4Fantasy));
            }
         }
         else {
            SetSecondLightImage4Fantasy(CreateLightImage(m_pDarkImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy, ""s, m_secondRomID4Fantasy, m_secondRomIDType4Fantasy, m_secondRomInverted4Fantasy));
            SetTopAndSecondLightImage4Fantasy(CreateLightImage(m_pTopLightImage4Fantasy, eDualMode_Fantasy, topkey4Fantasy));
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
         bool startAnimationAtBackglassStartup = (innerNode->Attribute("StartAnimationAtBackglassStartup") == "1"s);
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
            lightsStateAtAnimationStart = (innerNode->Attribute("AllLightsOffAtAnimationStart") == "1"s) ? eLightsStateAtAnimationStart_LightsOff : eLightsStateAtAnimationStart_NoChange;
         if (innerNode->FindAttribute("LightsStateAtAnimationEnd"))
            lightsStateAtAnimationEnd = (eLightsStateAtAnimationEnd)innerNode->IntAttribute("LightsStateAtAnimationEnd");
         else if (innerNode->FindAttribute("ResetLightsAtAnimationEnd"))
            lightsStateAtAnimationEnd = (innerNode->Attribute("ResetLightsAtAnimationEnd") == "1"s) ? eLightsStateAtAnimationEnd_LightsReseted : eLightsStateAtAnimationEnd_Undefined;
         if (innerNode->FindAttribute("AnimationStopBehaviour"))
            animationstopbehaviour = (eAnimationStopBehaviour)innerNode->IntAttribute("AnimationStopBehaviour");
         else if (innerNode->FindAttribute("RunAnimationTilEnd"))
            animationstopbehaviour = (innerNode->Attribute("RunAnimationTilEnd") == "1"s) ? eAnimationStopBehaviour_RunAnimationTillEnd : eAnimationStopBehaviour_StopImmediatelly;
         lockInvolvedLamps = (innerNode->Attribute("LockInvolvedLamps") == "1"s);
         if (innerNode->FindAttribute("HideScoreDisplays"))
            hidescoredisplays = (innerNode->Attribute("HideScoreDisplays") == "1"s);
         if (innerNode->FindAttribute("BringToFront"))
            bringtofront = (innerNode->Attribute("BringToFront") == "1"s);
         if (innerNode->FindAttribute("RandomStart"))
            randomstart = (innerNode->Attribute("RandomStart") == "1"s);
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
            //int step = stepnode->IntAttribute("Step");
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
         if (interval > 0 && !entries.empty()) {
            m_pB2SAnimation->AddAnimation(m_pB2SData, name, this, m_pFormDMD, dualmode, interval, loops, startAnimationAtBackglassStartup,
               lightsStateAtAnimationStart, lightsStateAtAnimationEnd, animationstopbehaviour, lockInvolvedLamps, hidescoredisplays,
               bringtofront, randomstart, randomquality, entries);
            // maybe set slowdown
            const auto& it = m_pB2SSettings->GetAnimationSlowDowns()->find(name);
            if (it != m_pB2SSettings->GetAnimationSlowDowns()->end())
               m_pB2SAnimation->SetAnimationSlowDown(name, it->second);
            // add join to ID
            if (!idJoins.empty()) {
               std::istringstream iss(idJoins);
               for (string idJoin; std::getline(iss, idJoin, ',');) {
                  if (!idJoin.empty()) {
                     int id0 = 0;
                     int id1 = 0;
                     int id2 = 0;
                     int id3 = 0;
                     if (!idJoin.empty() && is_string_numeric(idJoin))
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
      unsigned int width = m_vpxApi->GetTextureInfo(m_pDarkImage4Authentic)->width;
      unsigned int height = m_vpxApi->GetTextureInfo(m_pDarkImage4Authentic)->height;
      SetDarkImage4Authentic(ResizeTexture(m_pDarkImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
      xResizeFactor = (float)width / (float)m_vpxApi->GetTextureInfo(m_pDarkImage4Authentic)->width;
      yResizeFactor = (float)height / (float)m_vpxApi->GetTextureInfo(m_pDarkImage4Authentic)->height;
   }
   if (m_pDarkImage4Fantasy)
      SetDarkImage4Fantasy(m_pDarkImage4Authentic ? VPXGraphics::DuplicateTexture(m_vpxApi, m_pDarkImage4Authentic) : nullptr);
   if (m_pTopLightImage4Authentic)
      SetTopLightImage4Authentic(ResizeTexture(m_pTopLightImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
   if (m_pTopLightImage4Fantasy)
      SetTopLightImage4Fantasy(ResizeTexture(m_pTopLightImage4Fantasy, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
   if (m_pSecondLightImage4Authentic)
      SetSecondLightImage4Authentic(ResizeTexture(m_pSecondLightImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
   if (m_pSecondLightImage4Fantasy)
      SetSecondLightImage4Fantasy(ResizeTexture(m_pSecondLightImage4Fantasy, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
   if (m_pTopAndSecondLightImage4Authentic)
      SetTopAndSecondLightImage4Authentic(ResizeTexture(m_pTopAndSecondLightImage4Authentic, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
   if (m_pTopAndSecondLightImage4Fantasy)
      SetTopAndSecondLightImage4Fantasy(ResizeTexture(m_pTopAndSecondLightImage4Fantasy, m_pB2SScreen->GetBackglassSize().w, m_pB2SScreen->GetBackglassSize().h));
   SetBackgroundImage(GetDarkImage());

   // now resize the detail images
   if (xResizeFactor != 1.0f || yResizeFactor != 1.0f) {
      for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
         if (pPicbox->GetPictureBoxType() == ePictureBoxType_StandardImage) {
            VPXTexture pImage = pPicbox->GetBackgroundImage();
            if (pImage) {
               SDL_FRect frect = { 0.0f, 0.0f, (float)m_vpxApi->GetTextureInfo(pImage)->width / xResizeFactor, (float)m_vpxApi->GetTextureInfo(pImage)->height / yResizeFactor };
               SDL_Rect rect = { 0, 0, (int)frect.w, (int)frect.h };
               pPicbox->SetBackgroundImage(ResizeTexture(pImage, rect.w, rect.h));
               if (pPicbox->GetOffImage())
                  pPicbox->SetOffImage(ResizeTexture(pPicbox->GetOffImage(), rect.w, rect.h));
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
         VPXTexture pImage = RotateTexture(pPicbox->GetBackgroundImage(), rotationdirection == eSnippitRotationDirection_AntiClockwise ? rotatingAngle : 360 - rotatingAngle);
         (*m_pB2SData->GetRotatingImages())[romid][pPicbox->GetPictureBoxType() == ePictureBoxType_MechRotatingImage ? index : rotatingAngle] = pImage;
         rotatingAngle += m_rotateAngle;
         index++;
      }
   }
}

void FormBackglass::CheckDMDForm()
{
   if (!m_pFormDMD && !m_pB2SSettings->IsHideB2SDMD())
      m_pFormDMD = new FormDMD(m_vpxApi, m_msgApi, m_endpointId, m_pB2SData);
}

VPXTexture FormBackglass::CreateLightImage(VPXTexture image, eDualMode dualmode, const string& firstromkey_)
{
   return CreateLightImage(image, dualmode, firstromkey_, string());
}

VPXTexture FormBackglass::CreateLightImage(VPXTexture image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_)
{
   int romid = 0;
   eRomIDType romidtype = eRomIDType_NotDefined;
   bool rominverted = false;
   return CreateLightImage(image, dualmode, firstromkey_, secondromkey_, romid, romidtype, rominverted);
}

VPXTexture FormBackglass::CreateLightImage(VPXTexture image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_, int& romid, eRomIDType& romidtype, bool& rominverted)
{
   SDL_Surface* pSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, image);
   if (!pSurface)
      return nullptr;

   string firstromkey = firstromkey_;
   string secondromkey = secondromkey_;

   int secondromid = 0;
   eRomIDType secondromidtype = eRomIDType_NotDefined;
   bool secondrominverted = false;
   if (firstromkey.starts_with('I')) {
      rominverted = true;
      firstromkey = firstromkey.substr(1);
   }
   romidtype = (firstromkey.starts_with('S') ? eRomIDType_Solenoid : (firstromkey.starts_with("GI") ? eRomIDType_GIString : eRomIDType_Lamp));
   romid = std::stoi((romidtype == eRomIDType_GIString ? firstromkey.substr(2) : firstromkey.substr(1)));
   if (!secondromkey.empty()) {
      if (secondromkey.starts_with('I')) {
         secondrominverted = true;
         secondromkey = secondromkey.substr(1);
      }
      secondromidtype = (secondromkey.starts_with('S') ? eRomIDType_Solenoid : (secondromkey.starts_with("GI") ? eRomIDType_GIString : eRomIDType_Lamp));
      secondromid = std::stoi((secondromidtype == eRomIDType_GIString ? secondromkey.substr(2) : secondromkey.substr(1)));
   }

   // create image copy
   VPXTexture duplicateTexture = ResizeTexture(image, pSurface->w, pSurface->h);
   SDL_Surface* pImage = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, duplicateTexture);

   // draw matching bulbs into image
   for (auto& pControl : *GetControls()) {
      if (auto pPicbox = dynamic_cast<B2SPictureBox*>(pControl)) {
         if (pPicbox->GetRomID() == romid && pPicbox->GetRomIDType() == romidtype && pPicbox->IsRomInverted() == rominverted && (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == dualmode)) {
            SDL_Rect rect = { pPicbox->GetLocation().x, pPicbox->GetLocation().y, pPicbox->GetSize().w, pPicbox->GetSize().h };
            SDL_Surface* picboxSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pPicbox->GetBackgroundImage());
            if (picboxSurface) {
               SDL_BlitSurfaceScaled(picboxSurface, NULL, pImage, &rect, SDL_SCALEMODE_NEAREST);
               SDL_DestroySurface(picboxSurface);
            }
         }
      }
   }
   // maybe draw second matching bulbs into image
   if (!secondromkey.empty()) {
      for (auto& pControl : *GetControls()) {
         if (auto pPicbox = dynamic_cast<B2SPictureBox*>(pControl)) {
            if (pPicbox->GetRomID() == secondromid && pPicbox->GetRomIDType() == secondromidtype && pPicbox->IsRomInverted() == secondrominverted && (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == dualmode)) {
               SDL_Rect rect = { pPicbox->GetLocation().x, pPicbox->GetLocation().y, pPicbox->GetSize().w, pPicbox->GetSize().h };
               SDL_Surface* picboxSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pPicbox->GetBackgroundImage());
               if (picboxSurface) {
                  SDL_BlitSurfaceScaled(picboxSurface, NULL, pImage, &rect, SDL_SCALEMODE_NEAREST);
                  SDL_DestroySurface(picboxSurface);
               }
            }
         }
      }
   }

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, pImage);

   SDL_DestroySurface(pSurface);
   SDL_DestroySurface(pImage);
   if (duplicateTexture)
      m_vpxApi->DeleteTexture(duplicateTexture);

   return result;
}

void FormBackglass::CheckBulbs(int romid, eRomIDType romidtype, bool rominverted, eDualMode dualmode)
{
   if (romid > 0 && romidtype != eRomIDType_NotDefined) {
      std::map<int, vector<B2SBaseBox*>>* pUsedRomIDs = nullptr;
      if (romidtype == eRomIDType_Lamp)
         pUsedRomIDs = (dualmode == eDualMode_Fantasy ? m_pB2SData->GetUsedRomLampIDs4Fantasy() : m_pB2SData->GetUsedRomLampIDs4Authentic());
      else if (romidtype == eRomIDType_Solenoid)
         pUsedRomIDs = (dualmode == eDualMode_Fantasy ? m_pB2SData->GetUsedRomSolenoidIDs4Fantasy() : m_pB2SData->GetUsedRomSolenoidIDs4Authentic());
      else if (romidtype == eRomIDType_GIString)
         pUsedRomIDs = (dualmode == eDualMode_Fantasy ? m_pB2SData->GetUsedRomGIStringIDs4Fantasy() : m_pB2SData->GetUsedRomGIStringIDs4Authentic());
      auto itr = pUsedRomIDs->find(romid);
      if (itr != pUsedRomIDs->end()) {
         pUsedRomIDs->erase(itr);
         for(const auto& [key, pPicbox] : *m_pB2SData->GetIlluminations()) {
            if (pPicbox->GetRomID() == romid && pPicbox->GetRomIDType() == romidtype && pPicbox->IsRomInverted() != rominverted && (pPicbox->GetDualMode() == eDualMode_Both || pPicbox->GetDualMode() == dualmode))
               m_pB2SData->GetIlluminations()->Add(pPicbox, dualmode);
         }
      }
   }
}

VPXTexture FormBackglass::RotateTexture(VPXTexture source, int angle)
{
   SDL_Surface* pSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, source);
   if (!pSurface)
      return nullptr;

   SDL_LockSurface(pSurface);
   const int sw = pSurface->w;
   const int sh = pSurface->h;
   SDL_Surface* const destination = SDL_CreateSurface(sw, sh, pSurface->format);
   const float radians = -(float)angle * (float)(M_PI / 180.0);
   const float cosine = cosf(radians);
   const float sine = sinf(radians);
   const int dw = destination->w;
   const int dh = destination->h;
   const float center_x = (float)dw / 2.0f;
   const float center_y = (float)dh / 2.0f;
   const uint32_t* const __restrict src = ((uint32_t*)pSurface->pixels);
   uint32_t* const __restrict dest = ((uint32_t*)destination->pixels);
   for (int y = 0; y < dh; ++y) {
      const float xoffs = center_x - center_x * cosine + ((float)y - center_y) * sine;
      const float yoffs = center_y + center_x * sine   + ((float)y - center_y) * cosine;
      for (int x = 0; x < dw; ++x) {
         const int old_x = (int)(round(xoffs + (float)x * cosine));
         const int old_y = (int)(round(yoffs - (float)x * sine));
         if (/*old_x >= 0 &&*/ (unsigned int)old_x < (unsigned int)sw && /*old_y >= 0 &&*/ (unsigned int)old_y < (unsigned int)sh)
            dest[y * dw + x] = src[old_y * sw + old_x];
         else
            dest[y * dw + x] = 0; //!!?
      }
   }
   SDL_UnlockSurface(pSurface);

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, destination);
   SDL_DestroySurface(pSurface);
   SDL_DestroySurface(destination);

   return result;
}

VPXTexture FormBackglass::ResizeTexture(VPXTexture original, int newWidth, int newHeight)
{
   SDL_Surface* pSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, original);
   if (!pSurface)
      return nullptr;

   SDL_Surface* resizedSurface = SDL_CreateSurface(newWidth, newHeight, pSurface->format);
   if (!resizedSurface) {
      SDL_DestroySurface(pSurface);
      return nullptr;
   }

   SDL_BlitSurfaceScaled(pSurface, nullptr, resizedSurface, nullptr, SDL_SCALEMODE_LINEAR);

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, resizedSurface);

   SDL_DestroySurface(pSurface);
   SDL_DestroySurface(resizedSurface);

   return result;
}

SDL_Rect FormBackglass::GetBoundingRectangle(VPXTexture pImage)
{
   SDL_Surface* sourceSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pImage);
   if (!sourceSurface)
      return { 0, 0, 0, 0 };

   SDL_Surface* surface = SDL_ConvertSurface(sourceSurface, SDL_PIXELFORMAT_RGBA32);
   SDL_DestroySurface(sourceSurface);
   if (!surface)
      return { 0, 0, 0, 0 };

   SDL_LockSurface(surface);

   const uint32_t* const __restrict pixels = static_cast<uint32_t*>(surface->pixels);
   const int pitch = surface->pitch / (int)sizeof(uint32_t);
   const SDL_PixelFormatDetails* const pfd = SDL_GetPixelFormatDetails(surface->format);
   SDL_Palette* const pal = SDL_GetSurfacePalette(surface);

   const int w = surface->w;
   const int h = surface->h;

   int minX = w;
   int minY = h;
   int maxX = 0;
   int maxY = 0;

   bool foundNonTransparent = false;

   for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
         uint8_t r,g,b,alpha;
         SDL_GetRGBA(pixels[y * pitch + x], pfd, pal, &r, &g, &b, &alpha);

         if (alpha) {
            foundNonTransparent = true;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
         }
      }
   }

   SDL_UnlockSurface(surface);

   SDL_Rect result;
   if (!foundNonTransparent)
      result = { 0, 0, 0, 0 };
   else
      result = { minX, minY, maxX - minX + 1, maxY - minY + 1 };

   SDL_DestroySurface(surface);

   return result;
}

VPXTexture FormBackglass::CropImageToTransparency(VPXTexture pImage, VPXTexture pOffImage, SDL_Point& loc, SDL_Rect& size)
{
   SDL_Surface* pImageSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pImage);
   if (!pImageSurface)
      return nullptr;

   SDL_Rect boundingRect = GetBoundingRectangle(pImage);
   if (boundingRect.w == 0 && boundingRect.h == 0) {
      SDL_DestroySurface(pImageSurface);
      return nullptr;
   }

   SDL_Surface* pCroppedImage = SDL_CreateSurface(boundingRect.w, boundingRect.h, pImageSurface->format);
   if (!pCroppedImage) {
      SDL_DestroySurface(pImageSurface);
      return nullptr;
   }

   SDL_Surface* pOffImageSurface = nullptr;
   if (pOffImage) {
      pOffImageSurface = VPXGraphics::VPXTextureToSDLSurface(m_vpxApi, pOffImage);
      if (pOffImageSurface) {
         SDL_Rect offboundingRect = GetBoundingRectangle(pOffImage);
         if (offboundingRect.w == 0 && offboundingRect.h == 0) {
            SDL_DestroySurface(pCroppedImage);
            SDL_DestroySurface(pImageSurface);
            SDL_DestroySurface(pOffImageSurface);
            return nullptr;
         }

         SDL_Rect unionRect;
         SDL_GetRectUnion(&boundingRect, &offboundingRect, &unionRect);
         boundingRect = unionRect;

         SDL_Rect srcRect = boundingRect;
         SDL_Rect dstRect = { 0, 0, boundingRect.w, boundingRect.h };
         SDL_BlitSurfaceScaled(pOffImageSurface, &srcRect, pCroppedImage, &dstRect, SDL_SCALEMODE_NEAREST);
      }
   }

   SDL_Rect srcRect = boundingRect;
   SDL_Rect dstRect = { 0, 0, boundingRect.w, boundingRect.h };
   SDL_BlitSurfaceScaled(pImageSurface, &srcRect, pCroppedImage, &dstRect, SDL_SCALEMODE_NEAREST);

   size.w = size.w * boundingRect.w / pImageSurface->w;
   size.h = size.h * boundingRect.h / pImageSurface->h;
   loc.x += boundingRect.x;
   loc.y += boundingRect.y;

   VPXTexture result = VPXGraphics::SDLSurfaceToVPXTexture(m_vpxApi, pCroppedImage);

   SDL_DestroySurface(pImageSurface);
   if (pOffImageSurface)
      SDL_DestroySurface(pOffImageSurface);
   SDL_DestroySurface(pCroppedImage);

   return result;
}

VPXTexture FormBackglass::Base64ToImage(const char* image)
{
   const size_t image_len = strlen(image);
   vector<uint8_t> decoded = base64_decode(image, image_len);
   if (decoded.empty()) {
      LOGE("Base64ToImage: Failed to decode Base64 data");
      return nullptr;
   }

   VPXTexture pImage = m_vpxApi->CreateTexture(decoded.data(), static_cast<int>(decoded.size()));
   if (!pImage) {
      size_t len = std::min<size_t>(image_len, 40);
      LOGE("Base64ToImage: Failed to create texture from data: %s", string(image, len).c_str());
   }

   return pImage;
}

Sound* FormBackglass::Base64ToWav(const char* data)
{
   vector<uint8_t> decoded = base64_decode(data, strlen(data));
   return new Sound(std::move(decoded));
}

uint32_t FormBackglass::String2Color(const string& color)
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

}
