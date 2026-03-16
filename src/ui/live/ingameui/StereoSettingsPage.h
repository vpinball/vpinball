// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "renderer/Anaglyph.h"

namespace VPX::InGameUI
{

class StereoSettingsPage final : public InGameUIPage
{
public:
   StereoSettingsPage();
   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsedS) override;
   void SelectNextItem() override;
   void SelectPrevItem() override;
   void AdjustItem(float direction, bool isInitialPress) override;
   bool IsFlipperNavNeeded() const override { return m_calibrationStep >= 0; }

private:
   void BuildPage();
   void OnPointOfViewChanged();

   void RenderAnaglyphInformations() const;

   void UpdateAnaglyphCalibrationModal();
   VPX::Properties::PropertyRegistry::PropId GetCalibratedProperty() const;
   int m_calibrationStep = -1;
   float m_backgroundOpacity = 1.f;
   uint32_t m_lastUpdateMs = 0;
   uint32_t m_pressStartMs = 0;

   void NotifyDirectSave();
   bool m_notificationDone = false;

   StereoMode m_editedStereoMode = STEREO_OFF;
   bool m_staticPrepassDisabled = false;
   Anaglyph m_anaglyph;
};

}
