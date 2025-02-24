#pragma once

#include "common.h"

class Game;

class Controller
{
public:
   Controller(PinmameConfig& config);
   ~Controller();

   PSC_IMPLEMENT_REFCOUNT()

   void SetOnDestroyHandler(void (*handler)(Controller*)) { m_onDestroyHandler = handler; }
   void SetOnGameStartHandler(void (*handler)(Controller*)) { m_onGameStartHandler = handler; }
   void SetOnGameEndHandler(void (*handler)(Controller*)) { m_onGameEndHandler = handler; }

   string GetVersion() const;

   // TODO may also be accessed as a collection object
   Game* GetGames(const string& name) const;

   string GetGameName() const { if (m_pPinmameGame) return m_pPinmameGame->name; else return string(); }
   void SetGameName(const string& name);

   string GetSplashInfoLine() const { return m_splashInfoLine; }
   void SetSplashInfoLine(const string& text) { m_splashInfoLine = text; }

   bool GetHandleKeyboard() const { return PinmameGetHandleKeyboard(); }
   void SetHandleKeyboard(const bool handle) {PinmameSetHandleKeyboard(handle); }

   bool GetHandleMechanics() const { return PinmameGetHandleMechanics(); }
   void SetHandleMechanics(const bool handle) { PinmameSetHandleMechanics(handle); }

   bool GetHidden() const { return m_hidden; }
   void SetHidden(const bool hidden) { m_hidden = hidden; }

   int GetModOutputType(int output, int no) const { return output != static_cast<PINMAME_MOD_OUTPUT_TYPE>(PINMAME_MOD_OUTPUT_TYPE_SOLENOID) ? 0 : PinmameGetModOutputType(output, no); }
   void SetModOutputType(int output, int no, int newVal) { if (output == static_cast<PINMAME_MOD_OUTPUT_TYPE>(PINMAME_MOD_OUTPUT_TYPE_SOLENOID)) PinmameSetModOutputType(output, no, static_cast<PINMAME_MOD_OUTPUT_TYPE>(newVal)); }

   long GetSolMask(int nLow) const { return PinmameGetSolenoidMask(nLow); }
   void SetSolMask(int nLow, long newVal) { if ((0 <= nLow && nLow <= 2) || (1000 <= nLow && nLow < 2999)) PinmameSetSolenoidMask(nLow, newVal); }

   void Run(long hParentWnd = 0L, int nMinVersion = 100);
   bool GetRunning() const { return PinmameIsRunning(); }
   void SetPause(bool pause) { PinmamePause(pause); }
   bool GetPause() const { return PinmameIsPaused(); }
   void SetTimeFence(double fenceIns) { PinmameSetTimeFence(fenceIns); }
   void Stop();

   // Emulated machine state access
   int GetGetMech(int mechNo) const { return PinmameGetMech(mechNo); }
   void SetMech(int mechNo, int newVal);
   bool GetSwitch(int nSwitchNo) const { return PinmameGetSwitch(nSwitchNo); }
   void SetSwitch(int nSwitchNo, bool state) { PinmameSetSwitch(nSwitchNo, state ? 1 : 0); }
   int GetDip(int nNo) const { return PinmameGetDIP(nNo); }
   void SetDip(int nNo, int state) { PinmameSetDIP(nNo, state); }
   bool GetSolenoid(int nSolenoid) const { return PinmameGetSolenoid(nSolenoid); }
   bool GetLamp(int nLamp) const { return PinmameGetLamp(nLamp); }
   int GetGIString(int nString) const { return PinmameGetGI(nString); }
   std::vector<uint8_t> GetNVRAM() const;
   int GetRawDmdWidth() const;
   int GetRawDmdHeight() const;
   std::vector<uint8_t> GetRawDmdPixels() const;
   std::vector<uint32_t> GetRawDmdColoredPixels() const;
   const vector<PinmameNVRAMState>& GetChangedNVRAM();
   const vector<PinmameSoundCommand>& GetNewSoundCommands();
   const vector<PinmameLampState>& GetChangedLamps();
   const vector<PinmameLEDState>& GetChangedLEDs(int nHigh, int nLow, int nnHigh = 0, int nnLow = 0);
   const vector<PinmameGIState>& GetChangedGIStrings();
   const vector<PinmameSolenoidState>& GetChangedSolenoids();

   // TODO should we bridge this ? but to what as External dmddevice.dll is handled through the plugin bus ?
   bool GetShowPinDMD() const { LOGE("ShowPinDMD is not implemented"); return false; }
   void SetShowPinDMD(bool v) const { LOGE("ShowPinDMD is not implemented"); }

   // TODO should we bridge this ? but to what as Windows DMD is handled through the plugin bus ?
   bool GetShowWinDMD() const { LOGE("ShowWinDMD is not implemented"); return false; }
   void SetShowWinDMD(bool v) const { LOGE("ShowWinDMD is not implemented"); }

   // All these properties/methods are part of the VPinMame IDL but doesn't seem to be used anywhere (or are deprecated)
   //STDMETHOD(get_DmdWidth)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(get_DmdHeight)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(get_DmdPixel)(/*[in]*/ int x, /*[in]*/ int y, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_updateDmdPixels)(/*[in]*/ int **buf, /*[in]*/ int width, /*[in]*/ int height, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_ChangedLEDsState)(/*[in]*/ int nHigh, int nLow, int nnHigh, int nnLow, int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_Settings)(/*[out, retval]*/ IControllerSettings **pVal);
   //STDMETHOD(get_LockDisplay)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(put_LockDisplay)(/*[in]*/ VARIANT_BOOL newVal);
   //STDMETHOD(get_GIStrings)(/*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(get_Solenoids)(/*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(get_ChangedGIStrings)(/*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(get_HandleMechanics)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(put_HandleMechanics)(/*[in]*/ int newVal);
   //STDMETHOD(get_Machines)(/*[in]*/ BSTR sMachine, /*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(get_Switches)(/*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(put_Switches)(/*[out, retval]*/ VARIANT newVal);
   //STDMETHOD(get_Lamps)(/*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(get_WPCNumbering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(ShowAboutDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   //STDMETHOD(get_MechSamples)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(put_MechSamples)(/*[in]*/ VARIANT_BOOL newVal);
   //STDMETHOD(get_Game)(/*[out, retval]*/ IGame **pVal);
   //STDMETHOD(GetWindowRect)(/*[in,defaultvalue(0)]*/ LONG_PTR hWnd, /*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(GetClientRect)(/*[in,defaultvalue(0)]*/ LONG_PTR hWnd, /*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(get_ChangedLampsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_LampsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_ChangedSolenoidsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_SolenoidsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_ChangedGIsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_MasterVolume)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(put_MasterVolume)(/*[in]*/ int newVal);
   //STDMETHOD(get_EnumAudioDevices)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(get_AudioDevicesCount)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(get_AudioDeviceDescription)(/*[in]*/ int num, /*[out, retval]*/ BSTR *pVal);
   //STDMETHOD(get_AudioDeviceModule)(/*[in]*/ int num, /*[out, retval]*/ BSTR *pVal);
   //STDMETHOD(get_CurrentAudioDevice)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(put_CurrentAudioDevice)(/*[in]*/ int num);
   bool GetDoubleSize() const { LOGE("DoubleSize is deprecated"); return false; }
   void SetDoubleSize(bool v) const { LOGE("DoubleSize is deprecated"); }
   bool GetShowFrame() const { LOGE("ShowFrame is deprecated"); return false; }
   void SetShowFrame(bool v) const { LOGE("ShowFrame is deprecated"); }
   bool GetShowDMDOnly() const { LOGE("ShowDMDOnly is deprecated"); return false; }
   void SetShowDMDOnly(bool v) const { LOGE("ShowDMDOnly is deprecated"); }
   bool GetShowTitle() const { LOGE("ShowTitle is deprecated"); return false; }
   void SetShowTitle(bool v) const { LOGE("ShowTitle is deprecated"); }
   int GetFastFrames() const { LOGE("FastFrames is deprecated"); return 0; }
   void SetFastFrames(int v) const { LOGE("FastFrames is deprecated"); }
   bool GetIgnoreRomCrc() const { LOGE("IgnoreRomCrc is deprecated"); return false; }
   void SetIgnoreRomCrc(bool v) const { LOGE("IgnoreRomCrc is deprecated"); }
   bool GetCabinetMode() const { LOGE("CabinetMode is deprecated"); return false; }
   void SetCabinetMode(bool v) const { LOGE("CabinetMode is deprecated"); }
   int GetSoundMode() const { LOGE("SoundMode is deprecated"); return 0; }
   void SetSoundMode(int v) const { LOGE("SoundMode is deprecated"); }
   //STDMETHOD(ShowPathesDialog)(/*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd);
   //STDMETHOD(SetDisplayPosition)(/*[in]*/ int x, /*[in]*/ int y, /*[in]*/ LONG_PTR hParentWindow);
   //STDMETHOD(CheckROMS)(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(ShowOptsDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   //STDMETHOD(get_ROMName)(/*[out, retval]*/ BSTR *pVal);

   const pinmame_tMachineOutputState* GetStateBlock(int updateMask) const
   {
      PinmameGetStateBlock(updateMask, &m_stateBlock);
      return m_stateBlock;
   }

   const string& GetVpmPath() const { return m_vpmPath; }

private:
   string m_vpmPath;
   PinmameGame* m_pPinmameGame = nullptr; // Game selected by setting GameName property
   PinmameMechConfig* m_pPinmameMechConfig = nullptr;
   vector<PinmameLampState> m_lampStates;
   vector<PinmameLEDState> m_ledStates;
   vector<PinmameNVRAMState> m_nvramStates;
   vector<PinmameSoundCommand> m_soundCommands;
   vector<PinmameGIState> m_giStates;
   vector<PinmameSolenoidState> m_solenoidStates;
   string m_splashInfoLine; // Info line shown during startup
   bool m_hidden = true; // Show/Hide PinMame window

   mutable pinmame_tMachineOutputState* m_stateBlock = nullptr;

   void (*m_onDestroyHandler)(Controller*) = nullptr;
   void (*m_onGameStartHandler)(Controller*) = nullptr;
   void (*m_onGameEndHandler)(Controller*) = nullptr;
};
