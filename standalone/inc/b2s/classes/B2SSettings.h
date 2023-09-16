#pragma once

#include "../b2s_i.h"

class B2SData;

class B2SSettings
{
public:
   static B2SSettings* GetInstance();

   string GetDirectB2SVersion() const { return "2.0.3"; }
   string GetMinimumDirectB2SVersion() const { return "1.0"; }
   string GetBackglassFileVersion() const { return m_szBackglassFileVersion; }
   void SetBackglassFileVersion(const string& szBackglassFileVersion) { m_szBackglassFileVersion = szBackglassFileVersion; }

   bool IsAllOut() const { return m_allOut; }
   void SetAllOut(const bool allOut) { m_allOut = allOut; }
   bool IsAllOff() const { return m_allOff; }
   void SetAllOff(const bool allOff);
   bool IsLampsOff() const { return m_lampsOff; }
   void SetLampsOff(const bool lampsOff);
   bool IsSolenoidsOff() const { return m_solenoidsOff; }
   void SetSolenoidsOff(const bool solenoidsOff);
   bool IsGIStringsOff() const { return m_giStringsOff; }
   void SetGIStringsOff(const bool giStringsOff);
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
   bool IsROMControlled() { return !m_szGameName.empty(); }
   eDualMode_2 GetCurrentDualMode() const { return m_currentDualMode; }
   void SetCurrentDualMode(const eDualMode_2 currentDualMode) { m_currentDualMode = currentDualMode; }
   string GetGameName() const { return m_szGameName; }
   void SetGameName(const string& szGameName) { m_szGameName = szGameName; Load(false); }
   bool IsGameNameFound() const { return m_gameNameFound; }
   string GetB2SName() const { return m_szB2SName; }
   void SetB2SName(const string& szB2SName) { m_szB2SName = szB2SName; Load(false); }
   void Load(bool resetLogs = true);
   void ClearAll();

private:
   B2SSettings();
   ~B2SSettings();

   string m_szBackglassFileVersion;
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
   bool m_hideB2SDMD;
   eDualMode_2 m_currentDualMode;
   string m_szGameName;
   bool m_gameNameFound;
   string m_szB2SName;

   static B2SSettings* m_pInstance;
};
