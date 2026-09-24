// license:GPLv3+

#pragma once

#include "core/Settings.h"


// This class owns the application settings and exposes the settings that are effective
// for the current context: while a table session is running in a player, that table's
// override layer applies, otherwise the base application settings are used.
// Components should depend on a Settings (or this service) instead of reaching the
// settings through the application or player classes.
class SettingsService final
{
public:
   // Application wide settings (not affected by per table overrides)
   Settings &GetAppSettings() { return m_appSettings; }
   const Settings &GetAppSettings() const { return m_appSettings; }

   // Settings effective in the current context (table overrides applied while a table session is active)
   Settings &GetActiveSettings() { return m_tableOverride ? *m_tableOverride : m_appSettings; }
   const Settings &GetActiveSettings() const { return m_tableOverride ? *m_tableOverride : m_appSettings; }

   // Define the table whose settings override the application ones (set/cleared by the player around a table session)
   void SetTableOverride(Settings *settings) { m_tableOverride = settings; }
   void ClearTableOverride(Settings *settings)
   {
      assert(m_tableOverride == settings);
      m_tableOverride = nullptr;
   }

private:
   Settings m_appSettings;
   Settings *m_tableOverride = nullptr;
};

// Settings service, always defined
extern SettingsService g_settingsService;
