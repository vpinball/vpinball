#pragma once

#include "Form.h"
#include "../classes/B2SSettings.h"
#include "../classes/B2SData.h"
#include "../utils/Timer.h"

namespace B2SLegacy {

class B2SScreen;
class B2SAnimation;
class FormDMD;
class Server;

class FormBackglass final : public Form
{
public:
   FormBackglass(MsgPluginAPI* msgApi, VPXPluginAPI* vpxApi, B2SData* pB2SData);
   ~FormBackglass();

   void Start();
   void OnPaint(VPXRenderContext2D* const ctx) override;
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
   const SDL_FRect& GetScaleFactor() const;
   VPXTexture GetTopLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pTopLightImage4Fantasy : m_pTopLightImage4Authentic; }
   VPXTexture GetTopLightImage4Authentic() const { return m_pTopLightImage4Authentic; }
   void SetTopLightImage4Authentic(VPXTexture pTopLightImage4Authentic) { if (m_pTopLightImage4Authentic) m_vpxApi->DeleteTexture(m_pTopLightImage4Authentic); m_pTopLightImage4Authentic = pTopLightImage4Authentic; }
   VPXTexture GetTopLightImage4Fantasy() const { return m_pTopLightImage4Fantasy; }
   void SetTopLightImage4Fantasy(VPXTexture pTopLightImage4Fantasy) { if (m_pTopLightImage4Fantasy) m_vpxApi->DeleteTexture(m_pTopLightImage4Fantasy); m_pTopLightImage4Fantasy = pTopLightImage4Fantasy; }
   VPXTexture GetSecondLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pSecondLightImage4Fantasy : m_pSecondLightImage4Authentic; }
   VPXTexture GetSecondLightImage4Authentic() const { return m_pSecondLightImage4Authentic; }
   void SetSecondLightImage4Authentic(VPXTexture pSecondLightImage4Authentic) { if (m_pSecondLightImage4Authentic) m_vpxApi->DeleteTexture(m_pSecondLightImage4Authentic); m_pSecondLightImage4Authentic = pSecondLightImage4Authentic; }
   VPXTexture GetSecondLightImage4Fantasy() const { return m_pSecondLightImage4Fantasy; }
   void SetSecondLightImage4Fantasy(VPXTexture pSecondLightImage4Fantasy) { if (m_pSecondLightImage4Fantasy) m_vpxApi->DeleteTexture(m_pSecondLightImage4Fantasy); m_pSecondLightImage4Fantasy = pSecondLightImage4Fantasy; }
   VPXTexture GetTopAndSecondLightImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pTopAndSecondLightImage4Fantasy : m_pTopAndSecondLightImage4Authentic; }
   VPXTexture GetTopAndSecondLightImage4Authentic() const { return m_pTopAndSecondLightImage4Authentic; }
   void SetTopAndSecondLightImage4Authentic(VPXTexture pTopAndSecondLightImage4Authentic) { if (m_pTopAndSecondLightImage4Authentic) m_vpxApi->DeleteTexture(m_pTopAndSecondLightImage4Authentic); m_pTopAndSecondLightImage4Authentic = pTopAndSecondLightImage4Authentic; }
   VPXTexture GetTopAndSecondLightImage4Fantasy() const { return m_pTopAndSecondLightImage4Fantasy; }
   void SetTopAndSecondLightImage4Fantasy(VPXTexture pTopAndSecondLightImage4Fantasy) { if (m_pTopAndSecondLightImage4Fantasy) m_vpxApi->DeleteTexture(m_pTopAndSecondLightImage4Fantasy); m_pTopAndSecondLightImage4Fantasy = pTopAndSecondLightImage4Fantasy; }
   VPXTexture GetDarkImage() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_pDarkImage4Fantasy : m_pDarkImage4Authentic; }
   VPXTexture GetDarkImage4Authentic() const { return m_pDarkImage4Authentic; }
   void SetDarkImage4Authentic(VPXTexture pDarkImage4Authentic) { if (m_pDarkImage4Authentic) m_vpxApi->DeleteTexture(m_pDarkImage4Authentic); m_pDarkImage4Authentic = pDarkImage4Authentic; }
   VPXTexture GetDarkImage4Fantasy() const { return m_pDarkImage4Fantasy; }
   void SetDarkImage4Fantasy(VPXTexture pDarkImage4Fantasy) { if (m_pDarkImage4Fantasy) m_vpxApi->DeleteTexture(m_pDarkImage4Fantasy); m_pDarkImage4Fantasy = pDarkImage4Fantasy; }
   int GetTopRomID() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomID4Fantasy : m_topRomID4Authentic; }
   int GetTopRomIDType() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomIDType4Fantasy : m_topRomIDType4Authentic; }
   bool IsTopRomInverted() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomInverted4Fantasy : m_topRomInverted4Authentic; }
   int GetSecondRomID() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_secondRomID4Fantasy : m_secondRomID4Authentic; }
   eRomIDType GetSecondRomIDType() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_secondRomIDType4Fantasy : m_secondRomIDType4Authentic; }
   bool IsSecondRomInverted() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_secondRomInverted4Fantasy : m_secondRomInverted4Authentic; }
   FormDMD* GetFormDMD() const { return m_pFormDMD; }
   B2SScreen* GetB2SScreen() const { return m_pB2SScreen; }
   void ResizeSomeImages();
   bool IsValid() const { return m_valid; }

private:
   void DrawImage(VPXRenderContext2D* const ctx, B2SPictureBox* pPicbox);
   void StartupTimerTick(Timer* pTimer);
   void RotateTimerTick(Timer* pTimer);
   eLEDTypes GetLEDType() const;
   void LoadB2SData();
   void InitB2SScreen();

   void ShowStartupSnippits();
   void ShowStartupImages();
   void RotateImage(B2SPictureBox* pPicbox, int rotationsteps, eSnippitRotationDirection rotationdirection, ePictureBoxType type);
   void RotateImage(B2SPictureBox* pPicbox, int rotationsteps, eSnippitRotationDirection rotationdirection, ePictureBoxType type, eRomIDType& romidtype, int& romid);
   void CheckDMDForm();
   VPXTexture CreateLightImage(VPXTexture image, eDualMode dualmode, const string& firstromkey_);
   VPXTexture CreateLightImage(VPXTexture image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_);
   VPXTexture CreateLightImage(VPXTexture image, eDualMode dualmode, const string& firstromkey_, const string& secondromkey_, int& romid, eRomIDType& romidtype, bool& rominverted);
   void CheckBulbs(int romid, eRomIDType romidtype, bool rominverted, eDualMode dualmode);
   SDL_Rect GetBoundingRectangle(VPXTexture pImage);
   VPXTexture CropImageToTransparency(VPXTexture pImage, VPXTexture pOffImage, SDL_Point& loc, SDL_Rect& size);
   VPXTexture Base64ToImage(const string& image);
   Sound* Base64ToWav(const string& data);
   uint32_t String2Color(const string& color);
   VPXTexture ResizeTexture(VPXTexture original, int newWidth, int newHeight);
   VPXTexture RotateTexture(VPXTexture source, int angle);

   static constexpr int minSize4Image = 300000;

   B2SSettings* m_pB2SSettings;
   B2SScreen* m_pB2SScreen;
   B2SAnimation* m_pB2SAnimation;
   FormDMD* m_pFormDMD;
   Timer* m_pStartupTimer;
   Timer* m_pRotateTimer;
   int m_rotateSlowDownSteps;
   bool m_rotateRunTillEnd;
   bool m_rotateRunToFirstStep;
   int m_rotateAngle;
   int m_rotateTimerInterval;
   eLEDTypes m_selectedLEDType;
   VPXTexture m_pDarkImage4Authentic;
   VPXTexture m_pTopLightImage4Authentic;
   VPXTexture m_pSecondLightImage4Authentic;
   VPXTexture m_pTopAndSecondLightImage4Authentic;
   VPXTexture m_pDarkImage4Fantasy;
   VPXTexture m_pTopLightImage4Fantasy;
   VPXTexture m_pSecondLightImage4Fantasy;
   VPXTexture m_pTopAndSecondLightImage4Fantasy;
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

   bool m_valid = false;
};

}
