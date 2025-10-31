// license:GPLv3+

#include "core/stdafx.h"

#include "MiscSettingsPage.h"

namespace VPX::InGameUI
{

MiscSettingsPage::MiscSettingsPage()
   : InGameUIPage("Miscellaneous Settings"s, ""s, SaveMode::Table)
{
   BuildPage();
}

void MiscSettingsPage::BuildPage()
{
   ClearItems();

   // Tonemapper, as a table override, if not using WCG display tonemapper
   if (!m_player->m_renderer->m_HDRforceDisableToneMapper || !m_player->m_playfieldWnd->IsWCGBackBuffer())
   {
      Settings::GetRegistry().Register(Settings::GetTableOverride_ToneMapper_Property()->WithDefault(m_player->m_ptable->GetToneMapper()));
      AddItem(std::make_unique<InGameUIItem>( //
         Settings::m_propTableOverride_ToneMapper, //
         [this]() { return static_cast<int>(m_player->m_renderer->m_toneMapper); }, //
         [this](int, int v) { m_player->m_renderer->m_toneMapper = static_cast<ToneMapper>(v); }));
   }

   // Exposure, always saved as a table override setting if different from table embedded value
   // FIXME this conflicts with HDR adjusting exposure
   Settings::GetRegistry().Register(Settings::GetTableOverride_Exposure_Property()->WithDefault(m_player->m_ptable->GetExposure()));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propTableOverride_Exposure, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_renderer->m_exposure; }, //
      [this](float, float v)
      {
         m_player->m_renderer->m_exposure = v;
         m_player->m_ptable->FireOptionEvent(1); // Table option changed event
      }));

   // Difficulty, always saved as a table override setting if different from table embedded value
   Settings::GetRegistry().Register(Settings::GetTableOverride_Difficulty_Property()->WithDefault(m_player->m_ptable->m_difficulty));
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propTableOverride_Difficulty, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_globalDifficulty; }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_globalDifficulty = v;
         m_player->m_ptable->FireOptionEvent(1); // Table option changed event
         m_difficultyNotification
            = m_player->m_liveUI->PushNotification("You have changed the difficulty level\nThis change will only be applied after restart."s, 5000, m_difficultyNotification);
      }));

   // Global emission scale, if not overriden on commandline
   if (!g_pvp->m_bgles)
   {
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Scene Lighting"s, "Select how scene lighting is evaluated"s, 0, 0, vector { "Table's default"s, "Custom fixed value"s, "Time of day"s }),
         [this]() { return m_dayTimeMode; }, //
         [this]() { 
            if (!m_player->m_ptable->m_settings.GetPlayer_OverrideTableEmissionScale())
               return 0;
            if (!m_player->m_ptable->m_settings.GetPlayer_DynamicDayNight())
               return 1;
            return 2;
         }, //
         [this](int, int v)
         {
            m_dayTimeMode = v;
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

      if (m_dayTimeMode == 1)
      {
         Settings::GetRegistry().Register(Settings::GetPlayer_EmissionScale_Property()->WithDefault(m_player->m_ptable->m_globalEmissionScale));
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propPlayer_EmissionScale, 100.f, "%4.1f %%"s, [this]() { return m_player->m_renderer->m_globalEmissionScale; },
            [this](float, float v)
            {
               if (!m_staticPrepassDisabled)
               {
                  m_player->m_renderer->DisableStaticPrePass(true);
                  m_staticPrepassDisabled = true;
               }
               m_player->m_renderer->m_globalEmissionScale = v;
               m_player->m_renderer->MarkShaderDirty();
               m_player->m_ptable->FireOptionEvent(1); // Table option changed event
            }));
      }
      else if (m_dayTimeMode == 2)
      {
         // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
         AddItem(std::make_unique<InGameUIItem>( //
            Settings::m_propPlayer_Latitude, 1.f, "%4.1f deg"s, //
            [this]() { return m_player->m_ptable->m_settings.GetPlayer_Latitude(); }, //
            [this](float, float v) {
               m_player->m_ptable->m_settings.SetPlayer_Latitude(v, false);
               // FIXME recompute light level
            }));
         // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
         AddItem(std::make_unique<InGameUIItem>( //
            Settings::m_propPlayer_Longitude, 1.f, "%4.1f deg"s, //
            [this]() { return m_player->m_ptable->m_settings.GetPlayer_Longitude(); }, //
            [this](float, float v) {
               m_player->m_ptable->m_settings.SetPlayer_Longitude(v, false);
               // FIXME recompute light level
            }));
      }
   }
}

}
