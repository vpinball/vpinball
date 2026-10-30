// license:GPLv3+

#include "core/stdafx.h"
#include "core/SettingsService.h"

SettingsService g_settingsService;

SettingsService::SettingsService()
{
   // Table options are defined by table scripts: they must only be persisted in the per-table override ini, never in the global application one
   m_appSettings.AddSaveExcludedGroup("TableOption"s);
}
