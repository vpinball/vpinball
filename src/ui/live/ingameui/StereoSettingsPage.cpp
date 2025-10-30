// license:GPLv3+

#include "core/stdafx.h"

#include "StereoSettingsPage.h"

namespace VPX::InGameUI
{

StereoSettingsPage::StereoSettingsPage()
   : InGameUIPage("Stereo Settings"s, ""s, SaveMode::Both)
{
   BuildPage();
}

void StereoSettingsPage::BuildPage()
{
   ClearItems();

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   // This is a bit hacky as we can change the stereo mode at runtime if already doing stereo, but not switch it on/off
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3D, //
      [this]()
      {
         const bool stereoRT = m_player->m_renderer->m_stereo3D != STEREO_OFF;
         const bool stereoSel = m_player->m_ptable->m_settings.GetPlayer_Stereo3D() != STEREO_OFF;
         return (stereoRT != stereoSel) ? m_player->m_ptable->m_settings.GetPlayer_Stereo3D() : m_player->m_renderer->m_stereo3D;
      }, //
      [this](int, int v) {
         const bool stereoRT = m_player->m_renderer->m_stereo3D != STEREO_OFF;
         const bool stereoSel = v != STEREO_OFF;
         if (stereoRT != stereoSel)
         {
            m_player->m_liveUI->PushNotification("Toggling stereo rendering will be applied after restarting the game", 5000);
            m_player->m_ptable->m_settings.SetPlayer_Stereo3D((StereoMode)v, false);
         }
         else
         {
            m_player->m_renderer->m_stereo3D = (StereoMode)v;
            m_player->m_renderer->UpdateStereoShaderState();
            BuildPage();
         }
      }));

   if (m_player->m_renderer->m_stereo3D == STEREO_OFF)
      return;

   // FIXME this will conflict with rendering, so just disabled for now
   /* AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_Stereo3DEnabled, //
      [this]() { return m_player->m_renderer->m_stereo3Denabled; }, //
      [this](bool v) {
         m_player->m_renderer->m_stereo3Denabled = v;
         m_player->m_renderer->InitLayout();
         m_player->m_renderer->UpdateStereoShaderState();
         if (m_player->m_renderer->IsUsingStaticPrepass())
         {
            m_player->m_renderer->DisableStaticPrePass(true);
            m_player->m_renderer->DisableStaticPrePass(false);
         }
      })); */

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DEyeSeparation, 1.f, "%4.1f mm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DEyeSeparation(); }, //
      [this](float, float v) {m_player->m_ptable->m_settings.SetPlayer_Stereo3DEyeSeparation(v, false); }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DBrightness, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DBrightness(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayer_Stereo3DBrightness(v, false); }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DSaturation, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DSaturation(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayer_Stereo3DSaturation(v, false); }));

   if (!IsAnaglyphStereoMode(m_player->m_renderer->m_stereo3D))
      return;

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DDefocus, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DDefocus(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayer_Stereo3DDefocus(v, false); }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DLeftContrast, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DLeftContrast(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayer_Stereo3DLeftContrast(v, false); }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DRightContrast, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DRightContrast(); }, //
      [this](float, float v) { m_player->m_ptable->m_settings.SetPlayer_Stereo3DRightContrast(v, false); }));
}

}
