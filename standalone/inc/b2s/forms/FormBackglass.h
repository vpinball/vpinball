#pragma once

#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../classes/Timer.h"
#include "../controls/Control.h"

class B2SScreen;
class FormDMD;

class FormBackglass : public Control
{
public:
   FormBackglass();
   ~FormBackglass();

   void OnPaint(SDL_Surface* pSurface) override;

   void StartRotation();
   void StopRotation();
   bool isVisibleStateSet() const { return m_visibleStateSet; }
   void setVisibleStateSet(bool visibleStateSet) { m_visibleStateSet = visibleStateSet; }
   bool isLastTopVisible() const { return m_lastTopVisible; }
   void setLastTopVisible(bool lastTopVisible) { m_lastTopVisible = lastTopVisible; }
   bool isLastSecondVisible() const { return m_lastSecondVisible; }
   void setLastSecondVisible(bool lastSecondVisible) { m_lastSecondVisible = lastSecondVisible; }
   SDL_Surface* GetBackgroundImage() const { return m_pBackgroundImage; }
   void SetBackgroundImage(SDL_Surface* pBackgroundImage) { m_pBackgroundImage = pBackgroundImage; Invalidate(); }
   SDL_Surface* GetDarkImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_pDarkImage4Fantasy : m_pDarkImage4Authentic; }
   SDL_Surface* GetTopLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_pTopLightImage4Fantasy : m_pTopLightImage4Authentic; }
   SDL_Surface* GetSecondLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_pSecondLightImage4Fantasy : m_pSecondLightImage4Authentic; }
   SDL_Surface* GetTopAndSecondLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_pTopAndSecondLightImage4Fantasy : m_pTopAndSecondLightImage4Authentic; }
   int GetTopRomID() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_topRomID4Fantasy : m_topRomID4Authentic; }
   int GetTopRomIDType() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_topRomIDType4Fantasy : m_topRomIDType4Authentic; }
   bool IsTopRomInverted() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_topRomInverted4Fantasy : m_topRomInverted4Authentic; }  
   int GetSecondRomID() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_secondRomID4Fantasy : m_secondRomID4Authentic; }  
   eRomIDType GetSecondRomIDType() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_secondRomIDType4Fantasy : m_secondRomIDType4Authentic; }
   bool IsSecondRomInverted() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_2_Fantasy ? m_secondRomInverted4Fantasy : m_secondRomInverted4Authentic; }

private:
   void DrawImage(SDL_Surface* pSurface, B2SPictureBox* pPicbox);
   static void StartupTimerTick(void* param);
   static void RotateTimerTick(void* param);
   void LoadB2SData();
   void InitB2SScreen();
   void ResizeSomeImages();
   void ShowStartupSnippits();
   void ShowStartupImages();
   void RotateImage(B2SPictureBox* pPicbox, int rotationsteps, eSnippitRotationDirection rotationdirection, ePictureBoxType type);
   void RotateImage(B2SPictureBox* pPicbox, int rotationsteps, eSnippitRotationDirection rotationdirection, ePictureBoxType type, eRomIDType& romidtype, int& romid);
   void CheckDMDForm();
   SDL_Surface* CreateLightImage(SDL_Surface* image, eDualMode dualmode, const string& firstromkey_);
   SDL_Surface* CreateLightImage(SDL_Surface* image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_);
   SDL_Surface* CreateLightImage(SDL_Surface* image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_, int& romid, eRomIDType& romidtype, bool& rominverted);
   void CheckBulbs(int romid, eRomIDType romidtype, bool rominverted, eDualMode dualmode);
   SDL_Surface* Base64ToImage(const string& image);
   Sound* Base64ToWav(const string& data);

   OLE_COLOR String2Color(const string& color);

   SDL_Surface* ResizeSurface(SDL_Surface* original, int newWidth, int newHeight);
   SDL_Surface* RotateSurface(SDL_Surface* source, double angle);

   const int minSize4Image = 300000;

   B2SSettings* m_pB2SSettings;
   B2SData* m_pB2SData;
   B2SScreen* m_pB2SScreen;
   //B2SLED* m_pB2SLED;
   //B2SAnimation* m_pB2SAnimation;
   FormDMD* m_pFormDMD;
   B2S::Timer* m_pStartupTimer;
   B2S::Timer* m_pRotateTimer;
   int m_rotateSlowDownSteps;
   bool m_rotateRunTillEnd;
   bool m_rotateRunToFirstStep;
   int m_rotateSteps;
   float m_rotateAngle;
   int m_rotateTimerInterval;
   bool m_visibleStateSet;
   bool m_lastTopVisible;
   bool m_lastSecondVisible;
   std::map<string, int> m_animations;
   string m_lastRandomStartedAnimation;
   int m_rotation;
   SDL_Surface* m_pBackgroundImage;
   eLEDType m_selectedLEDType;
   SDL_Surface* m_pDarkImage4Authentic;
   SDL_Surface* m_pTopLightImage4Authentic;
   SDL_Surface* m_pSecondLightImage4Authentic;
   SDL_Surface* m_pTopAndSecondLightImage4Authentic;
   SDL_Surface* m_pDarkImage4Fantasy;
   SDL_Surface* m_pTopLightImage4Fantasy;
   SDL_Surface* m_pSecondLightImage4Fantasy;
   SDL_Surface* m_pTopAndSecondLightImage4Fantasy;
   int m_topRomID4Authentic;
   eRomIDType m_topRomIDType4Authentic;
   bool m_topRomInverted4Authentic;
   int m_secondRomID4Authentic;
   eRomIDType m_secondRomIDType4Authentic;
   bool m_secondRomInverted4Authentic;
   int m_topRomID4Fantasy;
   eRomIDType m_topRomIDType4Fantasy;
   bool m_topRomInverted4Fantasy;
   int m_secondRomID4Fantasy;
   eRomIDType m_secondRomIDType4Fantasy;
   bool m_secondRomInverted4Fantasy;
};
