#include "stdafx.h"

#include "B2SSettings.h"
#include "B2SData.h"

B2SSettings::B2SSettings()
{
   m_gameNameFound = false;

   m_pluginsOn = false;
   m_pPluginHost = PluginHost::GetInstance();

   ClearAll();

    m_pluginsOn = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SPlugins"s, false);
}

B2SSettings::~B2SSettings()
{
   if (ArePluginsOn())
      GetPluginHost()->UnregisterAllPlugins();
}

void B2SSettings::Load(bool resetLogs)
{
   ClearAll();

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   m_hideGrill = (B2SSettingsCheckedState)pSettings->LoadValueWithDefault(Settings::Standalone, "B2SHideGrill"s, (int)B2SSettingsCheckedState_Indeterminate);
   m_hideB2SDMD = pSettings->LoadValueWithDefault(Settings::Standalone, "B2SHideB2SDMD"s, false);
   m_hideB2SBackglass = pSettings->LoadValueWithDefault(Settings::Standalone, "B2SHideB2SBackglass"s, false);
   m_hideDMD = (B2SSettingsCheckedState)pSettings->LoadValueWithDefault(Settings::Standalone, "B2SHideDMD"s, (int)B2SSettingsCheckedState_Indeterminate);
   m_currentDualMode = (eDualMode)pSettings->LoadValueWithDefault(Settings::Standalone, "B2SDualMode"s, (int)eDualMode_2_NotSet);
}

void B2SSettings::ClearAll()
{
   // do not add GameName or B2SName here;
   m_dmdType = eDMDTypes_Standard;
   m_allOut = false;
   m_allOff = false;
   m_lampsOff = false;
   m_solenoidsOff = false;
   m_giStringsOff = false;
   m_ledsOff = false;
   m_lampsSkipFrames = 0;
   m_solenoidsSkipFrames = 0;
   m_giStringsSkipFrames = 0;
   m_ledsSkipFrames = 0;
   m_usedLEDType = eLEDTypes_Undefined;
   m_glowBulbOn = false;
   m_glowIndex = -1;
   m_defaultGlow = -1;
   m_hideGrill = B2SSettingsCheckedState_Indeterminate;
   m_hideB2SDMD = false;
   m_hideB2SBackglass = false;
   m_hideDMD = B2SSettingsCheckedState_Indeterminate;
   m_animationSlowDowns.clear();
   m_allAnimationSlowDown = 1;
   m_currentDualMode = (eDualMode)eDualMode_2_NotSet;
   m_formToFront = true;
   m_formToBack = false;
   m_formNoFocus = false;
}