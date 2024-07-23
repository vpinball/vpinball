#pragma once

#include "../b2s_i.h"
#include "../plugin/PluginHost.h"

#include <map>

#define B2S_SETTINGS_BACKGLASSX      15
#define B2S_SETTINGS_BACKGLASSY      30
#define B2S_SETTINGS_BACKGLASSWIDTH  290
#define B2S_SETTINGS_BACKGLASSHEIGHT 218
#define B2S_BACKGLASS_ZORDER         100

#define B2S_SETTINGS_DMDX            B2S_SETTINGS_BACKGLASSX
#define B2S_SETTINGS_DMDY            B2S_SETTINGS_BACKGLASSY + B2S_SETTINGS_BACKGLASSHEIGHT + 5
#define B2S_SETTINGS_DMDWIDTH        290
#define B2S_SETTINGS_DMDHEIGHT       75
#define B2S_DMD_ZORDER               120

enum B2SSettingsCheckedState {
    B2SSettingsCheckedState_Unchecked = 0,
    B2SSettingsCheckedState_Checked = 1,
    B2SSettingsCheckedState_Indeterminate = 2
};

class PluginHost;

class B2SSettings
{
public:
   static B2SSettings* GetInstance();

   string GetDirectB2SVersion() const { return "2.1.2"; }
   string GetMinimumDirectB2SVersion() const { return "1.0"; }
   string GetBackglassFileVersion() const { return m_szBackglassFileVersion; }
   void SetBackglassFileVersion(const string& szBackglassFileVersion) { m_szBackglassFileVersion = szBackglassFileVersion; }

   bool ArePluginsOn() const { return m_pluginsOn; }
   PluginHost* GetPluginHost() const { return m_pPluginHost; }
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
   bool IsROMControlled() { return !m_szGameName.empty(); }
   eDualMode GetCurrentDualMode() const { return m_currentDualMode; }
   void SetCurrentDualMode(const eDualMode currentDualMode) { m_currentDualMode = currentDualMode; }
   string GetGameName() const { return m_szGameName; }
   void SetGameName(const string& szGameName) { m_szGameName = szGameName; Load(false); }
   bool IsGameNameFound() const { return m_gameNameFound; }
   string GetB2SName() const { return m_szB2SName; }
   void SetB2SName(const string& szB2SName) { m_szB2SName = szB2SName; Load(false); }
   void Load(bool resetLogs = true);
   void ClearAll();
   B2SSettingsCheckedState GetHideGrill() const { return m_hideGrill; }
   B2SSettingsCheckedState GetHideDMD() const { return m_hideDMD; }
   bool IsFormToFront() const { return m_formToFront; }

   std::map<string, int>* GetAnimationSlowDowns() { return &m_animationSlowDowns; }
   int GetAllAnimationSlowDown() const { return m_allAnimationSlowDown; }
   void SetAllAnimationSlowDown(const int allAnimationSlowDown) { m_allAnimationSlowDown = allAnimationSlowDown; }

private:
   B2SSettings();
   ~B2SSettings();

   string m_szBackglassFileVersion;
   bool m_pluginsOn;
   PluginHost* m_pPluginHost;
   eDMDTypes m_dmdType;
   bool m_allOut;
   bool m_allOff;
   bool m_lampsOff;
   bool m_solenoidsOff;
   bool m_giStringsOff;
   bool m_ledsOff;
   int m_lampsSkipFrames;
   int m_solenoidsSkipFrames;
   int m_giStringsSkipFrames;
   int m_ledsSkipFrames;
   eLEDTypes m_usedLEDType;
   bool m_glowBulbOn;
   int m_glowIndex;
   int m_defaultGlow;
   B2SSettingsCheckedState m_hideGrill;
   bool m_hideB2SDMD;
   bool m_hideB2SBackglass;
   B2SSettingsCheckedState m_hideDMD;
   eDualMode m_currentDualMode;
   string m_szGameName;
   bool m_gameNameFound;
   string m_szB2SName;
   int m_allAnimationSlowDown;
   std::map<string, int> m_animationSlowDowns;
   bool m_formToFront;
   bool m_formToBack;
   bool m_formNoFocus;
   
   static B2SSettings* m_pInstance;
};
