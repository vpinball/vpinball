// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class PointOfViewSettingsPage : public InGameUIPage
{
public:
   PointOfViewSettingsPage();
   
   void Open() override;
   void Close() override;
   void Save() override;
   void ResetToInitialValues() override;

private:
   void BuildPage();
   void OnPointOfViewChanged();

   ViewSetup& GetCurrentViewSetup() const { return m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set]; }

   bool m_opened = false;
   bool m_lockScale = true;
   ViewSetup m_initialViewSetup;
   vec3 m_playerPos;
   vec3 m_initialPlayerPos;
   bool m_staticPrepassDisabled = false;
};

};