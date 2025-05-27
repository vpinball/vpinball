#pragma once

#include "common.h"
#include "libpinmame.h"

namespace B2S {
   
class B2SServer final
{
public:
   B2SServer();
   ~B2SServer();

   PSC_IMPLEMENT_REFCOUNT()

   void Dispose();
   string GetB2SServerVersion() const;
   double GetB2SBuildVersion() const;
   string GetB2SServerDirectory() const;

   string GetGameName() const;
   void SetGameName(const std::string& gameName);
   string GetROMName() const;
   string GetB2SName() const;
   void SetB2SName(const std::string& b2sName);
   string GetTableName() const;
   void SetTableName(const std::string& tableName);
   void SetWorkingDir(const std::string& workingDir);
   void SetPath(const std::string& path);

   bool GetRunning() const;
   void SetTimeFence(double timeFence);
   bool GetPause() const;
   void SetPause(bool pause);
   string GetVersion() const;
   string GetVPMBuildVersion() const;
   void Run(int32_t handle = 0);
   void Stop();
   bool GetLaunchBackglass() const;
   void SetLaunchBackglass(bool launchBackglass);
   string GetSplashInfoLine() const;
   void SetSplashInfoLine(string splashInfoLine);
   bool GetShowFrame() const;
   void SetShowFrame(bool showFrame);
   bool GetShowTitle() const;
   void SetShowTitle(bool showTitle);
   bool GetShowDMDOnly() const;
   void SetShowDMDOnly(bool showDMDOnly);
   bool GetShowPinDMD() const;
   void SetShowPinDMD(bool showPinDMD);
   bool GetLockDisplay() const;
   void SetLockDisplay(bool lockDisplay);
   bool GetDoubleSize() const;
   void SetDoubleSize(bool doubleSize);
   bool GetHidden() const;
   void SetHidden(bool hidden);
   bool GetPuPHide() const;
   void SetPuPHide(bool puPHide);
   bool GetHandleKeyboard() const;
   void SetHandleKeyboard(bool handleKeyboard);
   bool GetHandleMechanics() const;
   void SetHandleMechanics(bool handleMechanics);
   const vector<PinmameLampState>& GetChangedLamps();
   const vector<PinmameSolenoidState>& GetChangedSolenoids();
   const vector<PinmameGIState>& GetChangedGIStrings();
   const vector<PinmameLEDState>& GetChangedLEDs(int nHigh, int nLow, int nnHigh = 0, int nnLow = 0);
   const vector<PinmameSoundCommand>& GetNewSoundCommands();
   bool GetLamp(int nLamp) const;
   bool GetSolenoid(int nSolenoid) const;
   int GetGIString(int nString) const;
   bool GetSwitch(int nSwitchNo) const;
   void SetSwitch(int nSwitchNo, bool state);
   int GetGetMech(int mechNo) const;
   void SetMech(int mechNo, int state);
   int GetDip(int nNo) const;
   void SetDip(int nNo, int state);
   long GetSolMask(int nLow) const;
   void SetSolMask(int nLow, long newVal);
   int GetRawDmdWidth();
   int GetRawDmdHeight();
   std::vector<uint8_t> GetRawDmdPixels();
   std::vector<uint32_t> GetRawDmdColoredPixels();
   const vector<PinmameNVRAMState>& GetChangedNVRAM();
   int GetSoundMode() const;
   void SetSoundMode(int v);

   void B2SSetData(int id, int value);
   void B2SSetData(const std::string& name, int value);
   void B2SPulseData(int id);
   void B2SPulseData(const std::string& name);

   bool GetB2SIsAnimationRunning(const std::string& animationName) const;
   void B2SStartAnimation(const std::string& animationName);

private:
};

}