#include "../common.h"
#include "B2SSettings.h"
#include "B2SData.h"
#include <charconv>
#include <cstring>

namespace B2SLegacy {

B2SSettings::B2SSettings(MsgPluginAPI* msgApi)
   : m_msgApi(msgApi)
{
   m_gameNameFound = false;

   ClearAll();
}

B2SSettings::~B2SSettings()
{
}

void B2SSettings::Load(bool resetLogs)
{
   ClearAll();

   m_hideGrill = (B2SSettingsCheckedState)GetSettingInt("B2SHideGrill", (int)B2SSettingsCheckedState_Indeterminate);
   m_hideB2SDMD = GetSettingBool("B2SHideB2SDMD", false);
   m_hideB2SBackglass = GetSettingBool("B2SHideB2SBackglass", false);
   m_hideDMD = (B2SSettingsCheckedState)GetSettingInt("B2SHideDMD", (int)B2SSettingsCheckedState_Indeterminate);
   m_currentDualMode = (eDualMode)GetSettingInt("B2SDualMode", (int)eDualMode_2_NotSet);
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

int B2SSettings::GetSettingInt(const char* key, int def) const
{
   char buf[256];
   m_msgApi->GetSetting("B2SLegacy", key, buf, sizeof(buf));
   if (!buf[0]) return def;

   int result;
   auto [ptr, ec] = std::from_chars(buf, buf + strlen(buf), result);
   return (ec == std::errc{}) ? result : def;
}

bool B2SSettings::GetSettingBool(const char* key, bool def) const
{
   char buf[256];
   m_msgApi->GetSetting("B2SLegacy", key, buf, sizeof(buf));
   if (!buf[0]) return def;

   int result;
   auto [ptr, ec] = std::from_chars(buf, buf + strlen(buf), result);
   return (ec == std::errc{}) ? (result != 0) : def;
}

}
