// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "CabinetRender.h"
#include "renderer/ViewSetup.h"

namespace VPX::InGameUI
{

class PointOfViewSettingsPage final : public InGameUIPage
{
public:
   PointOfViewSettingsPage();

   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsed) override;
   void Save() override;
   void ResetToStoredValues() override;
   void ResetToDefaults() override;

private:
   VPX::Properties::PropertyRegistry::PropId SelectProp(
      VPX::Properties::PropertyRegistry::PropId dt, VPX::Properties::PropertyRegistry::PropId fss, VPX::Properties::PropertyRegistry::PropId cab) const;
   void OnPointOfViewChanged();
   void UpdateDefaults();
   void BuildPage() override;

   ViewSetup& GetCurrentViewSetup() const;

   bool m_opened = false;
   bool m_lockScale = true;
   ViewSetup m_initialViewSetup;
   vec3 m_playerPos;
   bool m_staticPrepassDisabled = false;

   unsigned int m_glassNotifId = 0;

   CabinetRender m_cabinetRender;
};

}
