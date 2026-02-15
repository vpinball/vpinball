// license:GPLv3+

#include "core/stdafx.h"

#include "MiscSettingsPage.h"

namespace VPX::InGameUI
{

MiscSettingsPage::MiscSettingsPage()
   : InGameUIPage("Miscellaneous Settings"s, ""s, SaveMode::Table)
{
}

void MiscSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_staticPrepassDisabled = false;
   BuildPage();
}

void MiscSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void MiscSettingsPage::RequestDynamicRendererUpdate()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->MarkShaderDirty();
   m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
}

void MiscSettingsPage::BuildPage()
{
   ClearItems();

   // Tonemapper, as a table override, if not using WCG display tonemapper
   if (!m_player->m_renderer->m_HDRforceDisableToneMapper || !m_player->m_playfieldWnd->IsWCGBackBuffer())
   {
      Settings::SetTableOverride_ToneMapper_Default(m_player->m_ptable->GetToneMapper());
      AddItem(std::make_unique<InGameUIItem>( //
         Settings::m_propTableOverride_ToneMapper, //
         [this]() { return static_cast<int>(m_player->m_renderer->m_toneMapper); }, //
         [this](int, int v) { m_player->m_renderer->m_toneMapper = static_cast<ToneMapper>(v); }));
   }

   // Exposure, always saved as a table override setting if different from table embedded value
   // FIXME this conflicts with HDR adjusting exposure
   Settings::SetTableOverride_Exposure_Default(m_player->m_ptable->GetExposure());
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propTableOverride_Exposure, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_renderer->m_exposure; }, //
      [this](float, float v)
      {
         m_player->m_renderer->m_exposure = v;
         m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
      }));

   // Difficulty, always saved as a table override setting if different from table embedded value
   Settings::SetTableOverride_Difficulty_Default(m_player->m_ptable->m_difficulty);
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propTableOverride_Difficulty, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_globalDifficulty; }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_globalDifficulty = v;
         m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
         m_difficultyNotification
            = m_player->m_liveUI->PushNotification("You have changed the difficulty level\nThis change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // Global emission scale, if not overriden on commandline
   if (!g_app->m_bgles)
   {
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(
            ""s, ""s, "Scene Lighting"s, "Select how scene lighting is evaluated"s, false, 0, 0, vector { "Table's default"s, "Custom fixed value"s, "Time of day"s }),
         [this]() { return (int) m_player->m_renderer->m_sceneLighting.GetMode(); }, // Live
         [this](Settings& settings)
         { 
            if (!settings.GetPlayer_OverrideTableEmissionScale())
               return 0;
            if (!settings.GetPlayer_DynamicDayNight())
               return 1;
            return 2;
         }, // Stored
         [this](int, int v)
         {
            m_player->m_renderer->m_sceneLighting.SetMode((Renderer::SceneLighting::Mode)v);
            RequestDynamicRendererUpdate();
            BuildPage();
         },
         [](Settings& settings)
         {
            settings.ResetPlayer_OverrideTableEmissionScale();
            settings.ResetPlayer_DynamicDayNight();
         },
         [](int v, Settings& settings, bool isTableOverride)
         {
            settings.SetPlayer_OverrideTableEmissionScale(v != 0, isTableOverride);
            settings.SetPlayer_DynamicDayNight(v == 2, isTableOverride);
         }));

      if (m_player->m_renderer->m_sceneLighting.GetMode() == Renderer::SceneLighting::Mode::User)
      {
         Settings::SetPlayer_EmissionScale_Default(m_player->m_ptable->m_globalEmissionScale);
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propPlayer_EmissionScale, 100.f, "%4.1f %%"s,  //
            [this]() { return m_player->m_renderer->m_sceneLighting.GetUserLightLevel(); },
            [this](float, float v)
            {
               m_player->m_renderer->m_sceneLighting.SetUserLightLevel(v);
               RequestDynamicRendererUpdate();
            }));
      }
      else if (m_player->m_renderer->m_sceneLighting.GetMode() == Renderer::SceneLighting::Mode::DayNight)
      {
         AddItem(std::make_unique<InGameUIItem>( //
            Settings::m_propPlayer_Latitude, 1.f, "%4.1f deg"s, //
            [this]() { return m_player->m_renderer->m_sceneLighting.GetLatitude(); }, //
            [this](float, float v) {
               m_player->m_renderer->m_sceneLighting.SetLatitude(v);
               RequestDynamicRendererUpdate();
            }));
         AddItem(std::make_unique<InGameUIItem>( //
            Settings::m_propPlayer_Longitude, 1.f, "%4.1f deg"s, //
            [this]() { return m_player->m_renderer->m_sceneLighting.GetLongitude(); }, //
            [this](float, float v) {
               m_player->m_renderer->m_sceneLighting.SetLongitude(v);
               RequestDynamicRendererUpdate();
            }));
      }
   }

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFContactors, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFContactors(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFContactors(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFKnocker, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFKnocker(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFKnocker(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFChimes, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFChimes(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFChimes(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFBell, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFBell(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFBell(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFGear, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFGear(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFGear(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFShaker, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFShaker(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFShaker(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFFlippers, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFFlippers(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFFlippers(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFTargets, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFTargets(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFTargets(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_DOFDropTargets, //
      [this]() { return m_player->m_ptable->m_settings.GetController_DOFDropTargets(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetController_DOFDropTargets(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propController_ForceDisableB2S, //
      [this]() { return m_player->m_ptable->m_settings.GetController_ForceDisableB2S(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetController_ForceDisableB2S(v, false);
         m_difficultyNotification = m_player->m_liveUI->PushNotification("This change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));
}

}
