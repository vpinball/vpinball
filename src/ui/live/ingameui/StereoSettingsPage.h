// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

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

private:
   void BuildPage();
   void OnPointOfViewChanged();
   void UpdateAnaglyphCalibrationModal();
   VPX::Properties::PropertyRegistry::PropId GetCalibratedProperty() const;
   int m_calibrationStep = -1;
   float m_backgroundOpacity = 1.f;
   StereoMode m_editedStereoMode = STEREO_OFF;
   bool m_staticPrepassDisabled = false;
};

}
