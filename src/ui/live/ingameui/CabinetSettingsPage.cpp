// license:GPLv3+

#include "core/stdafx.h"
#include "CabinetSettingsPage.h"

#include "core/player.h"
#include "renderer/Renderer.h"
#include "ui/live/LiveUI.h"

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
   m_player->m_renderer->InvalidateLayout();
}

void CabinetSettingsPage::ResetToDefaults()
{
   InGameUIPage::ResetToDefaults();

   if (ViewSetup& viewSetup = GetCurrentViewSetup(); viewSetup.mMode == VLM_WINDOW)
   {
      const PinTable* table = m_player->m_ptable;
      const float screenInclination = table->GetSettings().GetPlayer_ScreenInclination();
      viewSetup.SetViewPosFromPlayerPosition(table, m_playerPos, screenInclination);
   }
   OnPointOfViewChanged();
   RequestRebuild();
}

void CabinetSettingsPage::BuildPage()
{
   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cabinet dimensions"s));

   // Update defaults to user selected values, as defaults do not mean that much here ?

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenWidth, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_ScreenWidth(); }, //
      [this](float, float v)
      {
         NotifySavedForNextStart();
         g_settingsService.GetActiveSettings().SetPlayer_ScreenWidth(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenHeight, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_ScreenHeight(); }, //
      [this](float, float v)
      {
         NotifySavedForNextStart();
         g_settingsService.GetActiveSettings().SetPlayer_ScreenHeight(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenInclination, 1.f, "%4.2f deg"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_ScreenInclination(); }, //
      [this](float, float v)
      {
         NotifySavedForNextStart();
         g_settingsService.GetActiveSettings().SetPlayer_ScreenInclination(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarWidth, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_LockbarWidth(); }, //
      [this](float, float v)
      {
         NotifySavedForNextStart();
         g_settingsService.GetActiveSettings().SetPlayer_LockbarWidth(v, false);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LockbarHeight, 1.f, "%4.1f cm"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_LockbarHeight(); }, //
      [this](float, float v)
      {
         NotifySavedForNextStart();
         g_settingsService.GetActiveSettings().SetPlayer_LockbarHeight(v, false);
      }));

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Player position"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerX, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.x; }, //
      [this](float, float v)
      {
         m_playerPos.x = v;
         const float screenInclination = g_settingsService.GetActiveSettings().GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerY, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.y; }, //
      [this](float, float v)
      {
         m_playerPos.y = v;
         const float screenInclination = g_settingsService.GetActiveSettings().GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ScreenPlayerZ, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.z; }, //
      [this](float, float v)
      {
         m_playerPos.z = v;
         const float screenInclination = g_settingsService.GetActiveSettings().GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      }));
}

void CabinetSettingsPage::CollectPanes(vector<InGameUIPane>& panes)
{
   InGameUIPage::CollectPanes(panes);

   if ((m_player->m_ptable->GetViewMode() == ViewSetupID::BG_FULLSCREEN) && (m_player->m_ptable->GetViewSetup().mMode == VLM_WINDOW))
   {
      InGameUIPane pane;
      pane.id = "cabinet"s;
      pane.measure = [](float availW, float availH)
      {
         const float spacing = ImGui::GetStyle().ItemSpacing.y;
         return ImVec2(availW, min(availW, availH - spacing));
      };
      // CabinetRender expects its rect as (x, bottomY, w, h)
      pane.render = [this](const ImVec4& rect) { m_cabinetRender.Render(ImVec4(rect.x, rect.y + rect.w, rect.z, rect.w), m_player->m_ptable, m_playerPos); };
      panes.push_back(std::move(pane));
   }
}

}
