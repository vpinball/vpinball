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
   void Update(float elapsedS) override;
   void CollectPanes(vector<InGameUIPane>& panes) override;
   void SelectNextItem() override;
   void SelectPrevItem() override;
   void AdjustItem(float direction, bool isInitialPress) override;
   bool IsFlipperNavNeeded() const override { return m_calibrationStep >= 0; }

private:
   void BuildPage() override;
   void OnPointOfViewChanged();

   void RenderAnaglyphInformations(const ImVec4& rect) const;

   void UpdateAnaglyphCalibrationModal();
   VPX::Properties::PropertyRegistry::PropId GetCalibratedProperty() const;
   int m_calibrationStep = -1;
   float m_backgroundOpacity = 1.f;
   uint64_t m_lastUpdateNs = 0; // SDL_GetTicksNS() of the last held adjustment step
   uint64_t m_pressStartMs = 0;

   void NotifyDirectSave();
   bool m_notificationDone = false;

   StereoMode m_editedStereoMode = STEREO_OFF;
   bool m_staticPrepassDisabled = false;
   Anaglyph m_anaglyph;
};

}
