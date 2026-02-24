// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "CabinetRender.h"

namespace VPX::InGameUI
{

class CabinetSettingsPage final : public InGameUIPage
{
public:
   CabinetSettingsPage();

   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsed) override;
   void ResetToDefaults() override;

private:
   void BuildPage();
   void OnPointOfViewChanged();

   ViewSetup& GetCurrentViewSetup() const { return m_player->m_ptable->GetViewSetup(); }

   vec3 m_playerPos;
   bool m_staticPrepassDisabled = false;
   unsigned int m_delayApplyNotifId = 0;

   CabinetRender m_cabinetRender;
};

}
