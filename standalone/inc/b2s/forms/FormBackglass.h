#pragma once

#include "Form.h"
#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../../common/Timer.h"

class B2SScreen;
class B2SAnimation;
class FormDMD;

class FormBackglass : public Form
{
public:
   FormBackglass();
   ~FormBackglass();

   void OnPaint(VP::RendererGraphics* pGraphics) override;

   void StartAnimation(const string& szName, bool playReverse = false);
   void StopAnimation(const string& szName);
   void StopAllAnimations();
   bool IsAnimationRunning(const string& szName);
   void StartRotation();
   void StopRotation();
   void ShowScoreDisplays();
   void HideScoreDisplays();
   void PlaySound(const string& szSoundName);
   void StopSound(const string& szSoundName);
   SDL_Surface* GetTopLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pTopLightImage4Fantasy : m_pTopLightImage4Authentic; }
   SDL_Surface* GetTopLightImage4Authentic() const { return m_pTopLightImage4Authentic; }
   void SetTopLightImage4Authentic(SDL_Surface* pTopLightImage4Authentic) { m_pTopLightImage4Authentic = pTopLightImage4Authentic; }
   SDL_Surface* GetTopLightImage4Fantasy() const { return m_pTopLightImage4Fantasy; }
   void SetTopLightImage4Fantasy(SDL_Surface* pTopLightImage4Fantasy) { m_pTopLightImage4Fantasy = pTopLightImage4Fantasy; }
   SDL_Surface* GetSecondLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pSecondLightImage4Fantasy : m_pSecondLightImage4Authentic; }
   SDL_Surface* GetSecondLightImage4Authentic() const { return m_pSecondLightImage4Authentic; }
   void SetSecondLightImage4Authentic(SDL_Surface* pSecondLightImage4Authentic) { m_pSecondLightImage4Authentic = pSecondLightImage4Authentic; }
   SDL_Surface* GetSecondLightImage4Fantasy() const { return m_pSecondLightImage4Fantasy; }
   void SetSecondLightImage4Fantasy(SDL_Surface* pSecondLightImage4Fantasy) { m_pSecondLightImage4Fantasy = pSecondLightImage4Fantasy; }
   SDL_Surface* GetTopAndSecondLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pTopAndSecondLightImage4Fantasy : m_pTopAndSecondLightImage4Authentic; }
   SDL_Surface* GetTopAndSecondLightImage4Authentic() const { return m_pTopAndSecondLightImage4Authentic; }
   void SetTopAndSecondLightImage4Authentic(SDL_Surface* pTopAndSecondLightImage4Authentic) { m_pTopAndSecondLightImage4Authentic = pTopAndSecondLightImage4Authentic; }
   SDL_Surface* GetTopAndSecondLightImage4Fantasy() const { return m_pTopAndSecondLightImage4Fantasy; }
   void SetTopAndSecondLightImage4Fantasy(SDL_Surface* pTopAndSecondLightImage4Fantasy) { m_pTopAndSecondLightImage4Fantasy = pTopAndSecondLightImage4Fantasy; }
   SDL_Surface* GetDarkImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pDarkImage4Fantasy : m_pDarkImage4Authentic; }
   SDL_Surface* GetDarkImage4Authentic() const { return m_pDarkImage4Authentic; }
   void SetDarkImage4Authentic(SDL_Surface* pDarkImage4Authentic) { m_pDarkImage4Authentic = pDarkImage4Authentic; }
   SDL_Surface* GetDarkImage4Fantasy() const { return m_pDarkImage4Fantasy; }
   void SetDarkImage4Fantasy(SDL_Surface* pDarkImage4Fantasy) { m_pDarkImage4Fantasy = pDarkImage4Fantasy; }
   int GetTopRomID() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomID4Fantasy : m_topRomID4Authentic; }
   int GetTopRomIDType() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomIDType4Fantasy : m_topRomIDType4Authentic; }
   bool IsTopRomInverted() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomInverted4Fantasy : m_topRomInverted4Authentic; }
   int GetSecondRomID() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_secondRomID4Fantasy : m_secondRomID4Authentic; }
   eRomIDType GetSecondRomIDType() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_secondRomIDType4Fantasy : m_secondRomIDType4Authentic; }
   bool IsSecondRomInverted() { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_secondRomInverted4Fantasy : m_secondRomInverted4Authentic; }

private:
   void DrawImage(VP::RendererGraphics* pGraphics, B2SPictureBox* pPicbox);
   void StartupTimerTick(VP::Timer* pTimer);
   void RotateTimerTick(VP::Timer* pTimer);
   eLEDTypes GetLEDType();
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
   SDL_Surface* RotateSurface(SDL_Surface* source, int angle);

   const int minSize4Image = 300000;

   B2SSettings* m_pB2SSettings;
   B2SData* m_pB2SData;
   B2SScreen* m_pB2SScreen;
   B2SAnimation* m_pB2SAnimation;
   FormDMD* m_pFormDMD;
   VP::Timer* m_pStartupTimer;
   VP::Timer* m_pRotateTimer;
   int m_rotateSlowDownSteps;
   bool m_rotateRunTillEnd;
   bool m_rotateRunToFirstStep;
   int m_rotateAngle;
   int m_rotateTimerInterval;
   eLEDTypes m_selectedLEDType;
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
