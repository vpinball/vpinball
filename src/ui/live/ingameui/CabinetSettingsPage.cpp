// license:GPLv3+

#include "core/stdafx.h"

#include "CabinetSettingsPage.h"

namespace VPX::InGameUI
{

CabinetSettingsPage ::CabinetSettingsPage()
   : InGameUIPage("Cabinet Settings"s, "Defines the real world pinball cabinet size and player position.\nThis is needed to correctly compute virtual to real world visuals and sounds."s,
        SaveMode::Global)
{
}

void CabinetSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_staticPrepassDisabled = false;
   const Settings& settings = GetSettings();
   m_playerPos.x = settings.GetPlayer_ScreenPlayerX();
   m_playerPos.y = settings.GetPlayer_ScreenPlayerY();
   m_playerPos.z = settings.GetPlayer_ScreenPlayerZ();
   BuildPage();
}

void CabinetSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void CabinetSettingsPage::OnPointOfViewChanged()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->InitLayout();
}

void CabinetSettingsPage::ResetToDefaults()
{
   InGameUIPage::ResetToDefaults();

   if (ViewSetup& viewSetup = GetCurrentViewSetup(); viewSetup.mMode == VLM_WINDOW)
   {
      const PinTable* table = m_player->m_ptable;
      const float screenInclination = table->m_settings.GetPlayer_ScreenInclination();
      viewSetup.SetViewPosFromPlayerPosition(table, m_playerPos, screenInclination);
   }
   OnPointOfViewChanged();
   BuildPage();
}

void CabinetSettingsPage::BuildPage()
{
   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet dimensions"s));

   // Update defaults to user selected values, as defaults do not mean that much here ?

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenWidth(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenWidth(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenHeight, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenHeight(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenHeight(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenInclination, 1.f, "%4.2f deg"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_ScreenInclination(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_ScreenInclination(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarWidth, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_LockbarWidth(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_LockbarWidth(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarHeight, 1.f, "%4.1f cm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_LockbarHeight(); }, //
      [this](float, float v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetPlayer_LockbarHeight(v, false);
      }));

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Player position"s));

   Settings::SetPlayer_ScreenPlayerX_Default(0.f);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerX, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.x; }, //
      [this](float, float v)
      {
         m_playerPos.x = v;
         const float screenInclination = m_player->m_ptable->m_settings.GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      }));

   Settings::SetPlayer_ScreenPlayerY_Default(-10.f);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerY, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.y; }, //
      [this](float, float v)
      {
         m_playerPos.y = v;
         const float screenInclination = m_player->m_ptable->m_settings.GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      }));

   Settings::SetPlayer_ScreenPlayerZ_Default(70.f);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerZ, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.z; }, //
      [this](float, float v)
      {
         m_playerPos.z = v;
         const float screenInclination = m_player->m_ptable->m_settings.GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      }));
}

void CabinetSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);
   if ((m_player->m_ptable->GetViewMode() == ViewSetupID::BG_FULLSCREEN) && (m_player->m_ptable->GetViewSetup().mMode == VLM_WINDOW))
      m_cabinetRender.Render(
         ImVec4(GetWindowPos().x, GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y, GetWindowSize().x, min(GetWindowSize().x, GetWindowPos().y - 2.f * ImGui::GetStyle().ItemSpacing.y)),
         m_player->m_ptable, m_playerPos);
}

}
