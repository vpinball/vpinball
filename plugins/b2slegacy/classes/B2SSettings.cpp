#include "../common.h"
#include "B2SSettings.h"
#include "B2SData.h"
#include <charconv>
#include <cstring>

namespace B2SLegacy {

MSGPI_BOOL_VAL_SETTING(hideGrillProp, "B2SHideGrill", "B2SHideGrill", "", true, false); // VB uses CheckedState_Indeterminate
MSGPI_BOOL_VAL_SETTING(hideB2SProp, "B2SHideB2SDMD", "B2SHideB2SDMD", "", true, false);
MSGPI_BOOL_VAL_SETTING(hideB2SBackglassProp, "B2SHideB2SBackglass", "B2SHideB2SBackglass", "", true, false);
MSGPI_BOOL_VAL_SETTING(hideDMDProp, "B2SHideDMD", "B2SHideDMD", "", true, true); // VB uses CheckedState_Indeterminate
MSGPI_INT_VAL_SETTING(dualModeProp, "B2SDualMode", "B2SDualMode", "", true, eDualMode_2_Authentic, eDualMode_2_Fantasy, eDualMode_2_Authentic);

B2SSettings::B2SSettings(MsgPluginAPI* msgApi, unsigned int endpointId)
   : m_msgApi(msgApi)
   , m_endpointId(endpointId)
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
   m_msgApi->RegisterSetting(m_endpointId, &hideGrillProp);
   m_msgApi->RegisterSetting(m_endpointId, &hideB2SProp);
   m_msgApi->RegisterSetting(m_endpointId, &hideB2SBackglassProp);
   m_msgApi->RegisterSetting(m_endpointId, &hideDMDProp);
   m_msgApi->RegisterSetting(m_endpointId, &dualModeProp);
   m_hideGrill = hideGrillProp_Val;
   m_hideB2SDMD = hideB2SProp_Val;
   m_hideB2SBackglass = hideB2SBackglassProp_Val;
   m_hideDMD = hideDMDProp_Val;
   m_currentDualMode = (eDualMode)dualModeProp_Val;
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
   m_hideGrill = false; // VB uses CheckedState_Indeterminate
   m_hideB2SDMD = false;
   m_hideB2SBackglass = false;
   m_hideDMD = true; // VB uses CheckedState_Indeterminate
   m_animationSlowDowns.clear();
   m_allAnimationSlowDown = 1;
   m_currentDualMode = (eDualMode)eDualMode_2_NotSet;
   m_formToFront = true;
   m_formToBack = false;
   m_formNoFocus = false;
}

}
