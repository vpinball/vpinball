// license:GPLv3+

#include "core/stdafx.h"

#include "MiscSettingsPage.h"

namespace VPX::InGameUI
{

MiscSettingsPage::MiscSettingsPage()
   : InGameUIPage("Miscellaneous Settings"s, ""s, SaveMode::Table)
{
   // Global emission scale, if not overriden on commandline nor as an automatic day/night based on lat/lon, defined as a table override against the table original value
   const bool isDayNightOverriden = m_player->m_ptable->m_settings.LoadValueBool(Settings::Player, "OverrideTableEmissionScale"s);
   const bool isAutomaticDayNight = m_player->m_ptable->m_settings.LoadValueBool(Settings::Player, "DynamicDayNight"s);
   if (!g_pvp->m_bgles && !(isDayNightOverriden && isAutomaticDayNight))
   {
      AddItem(std::make_unique<InGameUIItem>(
         "Day/Night"s, "Select between daylight or night time lighting"s, 0.f, 100.f, 0.1f, 100.f * m_player->m_ptable->m_globalEmissionScale, "%4.1f %%"s,
         [this]() { return 100.f * m_player->m_renderer->m_globalEmissionScale; },
         [this](float prev, float v)
         {
            m_player->m_renderer->m_globalEmissionScale = v / 100.f;
            m_player->m_renderer->MarkShaderDirty();
            m_player->m_ptable->FireOptionEvent(1); // Table option changed event
            if (!m_staticPrepassDisabled)
            {
               m_player->m_renderer->DisableStaticPrePass(true);
               m_staticPrepassDisabled = true;
            }
         },
         [](Settings& settings)
         {
            settings.DeleteValue(Settings::Player, "OverrideTableEmissionScale"s);
            settings.DeleteValue(Settings::Player, "EmissionScale"s);
         },
         [this](float v, Settings& settings, bool isTableOverride)
         {
            assert(isTableOverride);
            if (abs(v - 100.f * m_player->m_ptable->m_globalEmissionScale) >= 0.1f)
            {
               settings.SaveValue(Settings::Player, "OverrideTableEmissionScale"s, true, isTableOverride);
               settings.SaveValue(Settings::Player, "EmissionScale"s, v / 100.f, isTableOverride);
            }
         }));
   }

   // Tonemapper, as a table override, if not using WCG display tonemapper
   if (!m_player->m_renderer->m_HDRforceDisableToneMapper || !m_player->m_playfieldWnd->IsWCGBackBuffer())
   {
      AddItem(std::make_unique<InGameUIItem>(
         "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s,
#ifdef ENABLE_BGFX
         vector({ "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s, "AgX Punchy"s }),
#else
         vector({ "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s }),
#endif
         static_cast<int>(m_player->m_ptable->GetToneMapper()), [this]() { return static_cast<int>(m_player->m_renderer->m_toneMapper); },
         [this](int prev, int v) { m_player->m_renderer->m_toneMapper = static_cast<ToneMapper>(v); },
         [](Settings& settings) { settings.DeleteValue(Settings::Player, "ToneMapper"s); },
         [this](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, "ToneMapper"s, v, isTableOverride); }));
   }

   // Exposure, always saved as a table override setting if different from table embedded value
   AddItem(std::make_unique<InGameUIItem>(
      "Camera Exposure"s, "Overall brightness of the rendered scene"s, 0.f, 200.f, 0.1f, 100.f * m_player->m_ptable->GetExposure(), "%4.1f %%"s,
      [this]() { return 100.f * m_player->m_renderer->m_exposure; },
      [this](float prev, float v)
      {
         m_player->m_renderer->m_exposure = v / 100.f;
         m_player->m_ptable->FireOptionEvent(1); // Table option changed event
      },
      [](Settings& settings) { settings.DeleteValue(Settings::Player, "Exposure"s); },
      [this](float v, Settings& settings, bool isTableOverride)
      {
         if (abs(v - 100.f * m_player->m_ptable->GetExposure()) >= 0.1f)
            settings.SaveValue(Settings::TableOverride, "Exposure"s, v / 100.f, isTableOverride);
      }));

   // Difficulty, always saved as a table override setting if different from table embedded value
   AddItem(std::make_unique<InGameUIItem>(
      "Difficulty"s, "Overall difficulty (slope, flipper size, trajectories scattering,...)"s, 0.f, 100.f, 0.1f, 100.f * m_player->m_ptable->m_difficulty, "%4.1f %%"s,
      [this]() { return 100.f * m_player->m_ptable->m_globalDifficulty; },
      [this](float prev, float v)
      {
         m_player->m_ptable->m_globalDifficulty = v / 100.f;
         m_player->m_ptable->FireOptionEvent(1); // Table option changed event
      },
      [](Settings& settings) { settings.DeleteValue(Settings::Player, "Difficulty"s); },
      [this](float v, Settings& settings, bool isTableOverride)
      {
         m_player->m_liveUI->PushNotification("You have changed the difficulty level\nThis change will only be applied after restart."s, 10000);
         if (abs(v - 100.f * m_player->m_ptable->m_difficulty) >= 0.1f)
            settings.SaveValue(Settings::TableOverride, "Difficulty"s, v / 100.f, isTableOverride);
      }));
}

}
