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
   FormBackglass(VPXPluginAPI* vpxApi, MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData);
   ~FormBackglass() override;

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
   eRomIDType GetTopRomIDType() const { return m_pB2SSettings->GetCurrentDualMode() == eDualMode_Fantasy ? m_topRomIDType4Fantasy : m_topRomIDType4Authentic; }
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
   VPXTexture Base64ToImage(const char* image);
   Sound* Base64ToWav(const char* data);
   uint32_t String2Color(const string& color);
   VPXTexture ResizeTexture(VPXTexture original, int newWidth, int newHeight);
   VPXTexture RotateTexture(VPXTexture source, int angle);

   static constexpr int minSize4Image = 300000;

   B2SSettings* m_pB2SSettings = nullptr;
   B2SScreen* m_pB2SScreen = nullptr;
   B2SAnimation* m_pB2SAnimation = nullptr;
   FormDMD* m_pFormDMD = nullptr;
   Timer* m_pStartupTimer = nullptr;
   Timer* m_pRotateTimer = nullptr;
   int m_rotateSlowDownSteps = 0;
   bool m_rotateRunTillEnd = false;
   bool m_rotateRunToFirstStep = false;
   int m_rotateAngle = 0;
   int m_rotateTimerInterval = 0;
   eLEDTypes m_selectedLEDType = eLEDTypes_Undefined;
   VPXTexture m_pDarkImage4Authentic = nullptr;
   VPXTexture m_pTopLightImage4Authentic = nullptr;
   VPXTexture m_pSecondLightImage4Authentic = nullptr;
   VPXTexture m_pTopAndSecondLightImage4Authentic = nullptr;
   VPXTexture m_pDarkImage4Fantasy = nullptr;
   VPXTexture m_pTopLightImage4Fantasy = nullptr;
   VPXTexture m_pSecondLightImage4Fantasy = nullptr;
   VPXTexture m_pTopAndSecondLightImage4Fantasy = nullptr;
   int m_topRomID4Authentic = 0;
   eRomIDType m_topRomIDType4Authentic = eRomIDType_NotDefined;
   bool m_topRomInverted4Authentic = false;
   int m_secondRomID4Authentic = 0;
   eRomIDType m_secondRomIDType4Authentic = eRomIDType_NotDefined;
   bool m_secondRomInverted4Authentic = false;
   int m_topRomID4Fantasy = 0;
   eRomIDType m_topRomIDType4Fantasy = eRomIDType_NotDefined;
   bool m_topRomInverted4Fantasy = false;
   int m_secondRomID4Fantasy = 0;
   eRomIDType m_secondRomIDType4Fantasy = eRomIDType_NotDefined;
   bool m_secondRomInverted4Fantasy = false;

   bool m_valid = false;
};

}
