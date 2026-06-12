#pragma once

#include "../common.h"
#include <map>
#include <string>
using std::string;

namespace B2SLegacy {

class B2SSettings final
{
public:
   B2SSettings(MsgPluginAPI* msgApi, unsigned int endpointId);
   ~B2SSettings();

   static const string& GetMinimumDirectB2SVersion() { static const string ver = "1.0"s; return ver; }
   const string& GetBackglassFileVersion() const { return m_szBackglassFileVersion; }
   void SetBackglassFileVersion(const string& szBackglassFileVersion) { m_szBackglassFileVersion = szBackglassFileVersion; }
   bool IsAllOut() const { return m_allOut; }
   void SetAllOut(const bool allOut) { m_allOut = allOut; }
   bool IsAllOff() const { return m_allOff; }
   void SetAllOff(const bool allOff) { m_allOff = allOff; }
   bool IsLampsOff() const { return m_lampsOff; }
   void SetLampsOff(const bool lampsOff) { m_lampsOff = lampsOff; }
   bool IsSolenoidsOff() const { return m_solenoidsOff; }
   void SetSolenoidsOff(const bool solenoidsOff) { m_solenoidsOff = solenoidsOff; }
   bool IsGIStringsOff() const { return m_giStringsOff; }
   void SetGIStringsOff(const bool giStringsOff) { m_giStringsOff = giStringsOff; }
   bool IsLEDsOff() const { return m_ledsOff; }
   void SetLEDsOff(const bool ledsOff) { m_ledsOff = ledsOff; }
   int GetLampsSkipFrames() const { return m_lampsSkipFrames; }
   void SetLampsSkipFrames(const int lampsSkipFrames) { m_lampsSkipFrames = lampsSkipFrames; }
   int GetSolenoidsSkipFrames() const { return m_solenoidsSkipFrames; }
   void SetSolenoidsSkipFrames(const int solenoidsSkipFrames) { m_solenoidsSkipFrames = solenoidsSkipFrames; }
   int GetGIStringsSkipFrames() const { return m_giStringsSkipFrames; }
   void SetGIStringsSkipFrames(const int giStringsSkipFrames) { m_giStringsSkipFrames = giStringsSkipFrames; }
   int GetLEDsSkipFrames() const { return m_ledsSkipFrames; }
   void SetLEDsSkipFrames(const int ledsSkipFrames) { m_ledsSkipFrames = ledsSkipFrames; }
   eLEDTypes GetUsedLEDType() const { return m_usedLEDType; }
   void SetUsedLEDType(const eLEDTypes usedLEDType) { m_usedLEDType = usedLEDType; }
   bool IsGlowBulbOn() const { return m_glowBulbOn; }
   void SetGlowBulbOn(const bool glowBulbOn) { m_glowBulbOn = glowBulbOn; }
   int GetGlowIndex() const { return m_glowIndex; }
   void SetGlowIndex(const int glowIndex) { m_glowIndex = glowIndex; }
   int GetDefaultGlow() const { return m_defaultGlow; }
   void SetDefaultGlow(const int defaultGlow) { m_defaultGlow = defaultGlow; }
   bool IsHideB2SDMD() const { return m_hideB2SDMD; }
   void SetHideB2SDMD(const bool hideB2SDMD) { m_hideB2SDMD = hideB2SDMD; }
   bool IsHideB2SBackglass() const { return m_hideB2SBackglass; }
   void SetHideB2SBackglass(const bool hideB2SBackglass) { m_hideB2SBackglass = hideB2SBackglass; }
   bool IsROMControlled() const { return !m_szGameName.empty(); }
   eDualMode GetCurrentDualMode() const { return m_currentDualMode; }
   void SetCurrentDualMode(const eDualMode currentDualMode) { m_currentDualMode = currentDualMode; }
   const string& GetGameName() const { return m_szGameName; }
   void SetGameName(const string& szGameName) { m_szGameName = szGameName; Load(false); }
   bool IsGameNameFound() const { return m_gameNameFound; }
   const string& GetB2SName() const { return m_szB2SName; }
   void SetB2SName(const string& szB2SName) { m_szB2SName = szB2SName; Load(false); }
   void Load(bool resetLogs = true);
   void ClearAll();
   bool IsHideGrill() const { return m_hideGrill; }
   bool IsHideDMD() const { return m_hideDMD; }
   bool IsFormToFront() const { return m_formToFront; }
   std::map<string, int>* GetAnimationSlowDowns() { return &m_animationSlowDowns; }
   int GetAllAnimationSlowDown() const { return m_allAnimationSlowDown; }
   void SetAllAnimationSlowDown(const int allAnimationSlowDown) { m_allAnimationSlowDown = allAnimationSlowDown; }

private:
   string m_szBackglassFileVersion;
   eDMDTypes m_dmdType = eDMDTypes_Standard;
   bool m_allOut = false;
   bool m_allOff = false;
   bool m_lampsOff = false;
   bool m_solenoidsOff = false;
   bool m_giStringsOff = false;
   bool m_ledsOff = false;
   int m_lampsSkipFrames = 0;
   int m_solenoidsSkipFrames = 0;
   int m_giStringsSkipFrames = 0;
   int m_ledsSkipFrames = 0;
   eLEDTypes m_usedLEDType = eLEDTypes_Undefined;
   bool m_glowBulbOn = false;
   int m_glowIndex = -1;
   int m_defaultGlow = -1;
   bool m_hideGrill = false;
   bool m_hideB2SDMD = false;
   bool m_hideB2SBackglass = false;
   bool m_hideDMD = true;
   eDualMode m_currentDualMode = (eDualMode)eDualMode_2_NotSet;
   string m_szGameName;
   bool m_gameNameFound = false;
   string m_szB2SName;
   int m_allAnimationSlowDown = 1;
   std::map<string, int> m_animationSlowDowns;
   bool m_formToFront = true;
   bool m_formToBack = false;
   bool m_formNoFocus = false;
   MsgPluginAPI* m_msgApi = nullptr;
   unsigned int m_endpointId = 0;
};

}
