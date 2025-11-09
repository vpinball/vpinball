// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class PointOfViewSettingsPage final : public InGameUIPage
{
public:
   PointOfViewSettingsPage();
   
   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Save() override;
   void ResetToStoredValues() override;
   void ResetToDefaults() override;

private:
   void BuildPage();
   void OnPointOfViewChanged();

   ViewSetup& GetCurrentViewSetup() const { return m_player->m_ptable->GetViewSetup(); }

   bool m_opened = false;
   bool m_lockScale = true;
   ViewSetup m_initialViewSetup;
   vec3 m_playerPos;
   vec3 m_initialPlayerPos;
   bool m_staticPrepassDisabled = false;
};

}
