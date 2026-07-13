// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ControllerPlugin.h"

#include <unordered_map>
#include <thread>


namespace PinMAME {

class Game;
class GameSettings;
class Settings;

class Controller final
{
public:
   Controller(const MsgPluginAPI* api, unsigned int endpointId, const PinmameConfig& config);
   ~Controller();

   PSC_IMPLEMENT_REFCOUNT()

   void SetOnDestroyHandler(void (*handler)(Controller*)) { m_onDestroyHandler = handler; }
   void SetOnGameStartHandler(void (*handler)(Controller*)) { m_onGameStartHandler = handler; }
   void SetOnGameEndHandler(void (*handler)(Controller*)) { m_onGameEndHandler = handler; }
   bool GetIsPlugin() const { return true; }

   string GetVersion() const;

   // TODO may also be accessed as a collection object
   Game* GetGames(const string& name) const;

   Settings* GetSettings();

   string GetGameName() const { return m_szGameName; }
   void SetGameName(const string& name);
   string GetROMName() const { if (m_pPinmameGame) return m_pPinmameGame->name; else return string(); }

   string GetSplashInfoLine() const { return m_splashInfoLine; }
   void SetSplashInfoLine(const string& text) { m_splashInfoLine = text; }

   bool GetHandleMechanics() const { return PinmameGetHandleMechanics(); }
   void SetHandleMechanics(const bool handle) { PinmameSetHandleMechanics(handle); }

   bool GetHidden() const { return m_hidden; }
   void SetHidden(const bool hidden) { m_hidden = hidden; }

   void Run(long hParentWnd = 0L, int nMinVersion = 100);
   bool GetRunning() const { return PinmameIsRunning(); }
   void SetPause(bool pause) { PinmamePause(pause); }
   bool GetPause() const { return PinmameIsPaused(); }
   void SetTimeFence(double fenceIns) { PinmameSetTimeFence(fenceIns); }
   void Stop();

   std::vector<uint8_t> GetNVRAM() const;
   const vector<PinmameNVRAMState>& GetChangedNVRAM();
   const vector<PinmameSoundCommand>& GetNewSoundCommands();

   // Inputs
   bool GetSwitch(int nSwitchNo) const;
   void SetSwitch(int nSwitchNo, bool state);
   int GetDip(int nNo) const;
   void SetDip(int nNo, int state);

   // Devices
   long GetSolMask(int nLow) const;
   void SetSolMask(int nLow, long newVal); // Define device emulation mode, or mask applied to GetChangedSolenoids
   int GetModOutputType(int output, int no) const; // FIXME deprecate in PinMAME/VPinMAME/LibPinMAME, this is wrong (it was added to wait while defining them inside PinMAME)
   void SetModOutputType(int output, int no, int newVal); // FIXME deprecate in PinMAME/VPinMAME/LibPinMAME, this is wrong (it was added to wait while defining them inside PinMAME)
   int GetSolenoid(int nSolenoid) const;
   int GetLamp(int nLamp) const;
   int GetGIString(int nString) const;
   int GetGetMech(int mechNo) const { return PinmameGetMech(mechNo); }
   void SetMech(int mechNo, int newVal);
   const vector<PinmameLampState>& GetChangedLamps();
   const vector<PinmameGIState>& GetChangedGIStrings();
   const vector<PinmameSolenoidState>& GetChangedSolenoids();

   // Segment displays
   const vector<PinmameLEDState>& GetChangedLEDs(int nHigh, int nLow, int nnHigh = 0, int nnLow = 0);

   // DMD displays
   int GetRawDmdWidth();
   int GetRawDmdHeight();
   std::vector<uint8_t> GetRawDmdPixels();
   std::vector<uint32_t> GetRawDmdColoredPixels();
   bool GetShowPinDMD() const { LOGE("ShowPinDMD is deprecated"s); return false; } // Deprecated as this must not be part of the table script but of the global setup
   void SetShowPinDMD(bool v) const { LOGE("ShowPinDMD is deprecated"s); } // Deprecated as this must not be part of the table script but of the global setup
   bool GetShowWinDMD() const { LOGE("ShowWinDMD is deprecated"s); return false; } // Deprecated (could be implemented as exposing or not the DMD, but there is no good use case for this)
   void SetShowWinDMD(bool v) const { LOGE("ShowWinDMD is deprecated"s); } // Deprecated (could be implemented as exposing or not the DMD, but there is no good use case for this)

   // Note: we force input handling to be always disabled as it would lead to setup problems and conflicts.
   // All interactions must be performed through the script or plugin API
   bool GetHandleKeyboard() const { LOGE("GetHandleKeyboard is deprecated"s); return false; }
   void SetHandleKeyboard(const bool handle) { LOGE("SetHandleKeyboard is deprecated"s); }

   // All these properties/methods are part of the VPinMAME IDL but doesn't seem to be used anywhere (or are deprecated)
   //STDMETHOD(get_DmdWidth)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(get_DmdHeight)(/*[out, retval]*/ int *pVal);
   //STDMETHOD(get_DmdPixel)(/*[in]*/ int x, /*[in]*/ int y, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_updateDmdPixels)(/*[in]*/ int **buf, /*[in]*/ int width, /*[in]*/ int height, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_ChangedLEDsState)(/*[in]*/ int nHigh, int nLow, int nnHigh, int nnLow, int **buf, /*[out, retval]*/ int *pVal);
   //STDMETHOD(get_Settings)(/*[out, retval]*/ IControllerSettings **pVal);
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
   bool GetLockDisplay() const { LOGE("LockDisplay is deprecated"s); return false; }
   void SetLockDisplay(bool v) const { LOGE("LockDisplay is deprecated"s); }
   bool GetDoubleSize() const { LOGE("DoubleSize is deprecated"s); return false; }
   void SetDoubleSize(bool v) const { LOGE("DoubleSize is deprecated"s); }
   bool GetShowFrame() const { LOGE("ShowFrame is deprecated"s); return false; }
   void SetShowFrame(bool v) const { LOGE("ShowFrame is deprecated"s); }
   bool GetShowDMDOnly() const { LOGE("ShowDMDOnly is deprecated"s); return false; }
   void SetShowDMDOnly(bool v) const { LOGE("ShowDMDOnly is deprecated"s); }
   bool GetShowTitle() const { LOGE("ShowTitle is deprecated"s); return false; }
   void SetShowTitle(bool v) const { LOGE("ShowTitle is deprecated"s); }
   int GetFastFrames() const { LOGE("FastFrames is deprecated"s); return 0; }
   void SetFastFrames(int v) const { LOGE("FastFrames is deprecated"s); }
   bool GetIgnoreRomCrc() const { LOGE("IgnoreRomCrc is deprecated"s); return false; }
   void SetIgnoreRomCrc(bool v) const { LOGE("IgnoreRomCrc is deprecated"s); }
   bool GetCabinetMode() const { LOGE("CabinetMode is deprecated"s); return false; }
   void SetCabinetMode(bool v) const { LOGE("CabinetMode is deprecated"s); }
   int GetSoundMode() const { LOGE("SoundMode is deprecated"s); return 0; }
   void SetSoundMode(int v) const { LOGE("SoundMode is deprecated"s); }
   void ShowOptsDialog(long hParentWnd = 0L) const { LOGE("ShowOptsDialog is deprecated"s); }
   //STDMETHOD(ShowPathesDialog)(/*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd);
   //STDMETHOD(SetDisplayPosition)(/*[in]*/ int x, /*[in]*/ int y, /*[in]*/ LONG_PTR hParentWindow);
   //STDMETHOD(CheckROMS)(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(ShowOptsDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   //STDMETHOD(get_ROMName)(/*[out, retval]*/ BSTR *pVal);

   const string& GetVpmPath() const { return m_vpmPath; }

   void SetCheat(bool cheat) { m_cheat = cheat; }

private:
   string m_vpmPath;
   string m_szGameName;
   mutable std::unordered_map<string, GameSettings*> m_gameSettings; // shared per game so settings survive repeated Games(name) accesses
   Settings* m_settings = nullptr;
   PinmameGame* m_pPinmameGame = nullptr;
   PinmameMechConfig* m_pPinmameMechConfig = nullptr;
   vector<PinmameLEDState> m_ledStates;
   vector<PinmameNVRAMState> m_nvramStates;
   vector<PinmameSoundCommand> m_soundCommands;
   string m_splashInfoLine; // Info line shown during startup
   bool m_hidden = true; // Show/Hide PinMAME window

   const MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;

   unsigned int m_getInputSrcMsgId, m_onInputChangedMsgId;
   mutable bool m_inputUpdatePending = true;
   mutable InputSrcId m_inputs { };
   mutable vector<int> m_switches;
   mutable vector<bool> m_switchStates;
   mutable vector<unsigned int> m_switchMap;
   mutable vector<int> m_dipSwitches;
   mutable vector<bool> m_dipSwitchStates;
   mutable vector<unsigned int> m_dipSwitchMap;
   static void OnInputSrcChanged(const unsigned int msgId, void* userData, void* msgData);
   void UpdateInputSrc() const;

   enum DeviceMode
   {
      DM_BINARY,
      DM_MODSOL,
      DM_PHYSOUT
   };
   uint64_t m_solMask = 0xFFFFFFFFFFFFFFFFULL; // Mask applied to (and only to) GetChangedSolenoids
   DeviceMode m_deviceMode = DM_BINARY;
   unsigned int m_getDeviceSrcMsgId, m_onDeviceChangedMsgId;
   vector<uint8_t> m_prevDeviceState;
   mutable bool m_deviceUpdatePending = true;
   mutable DevSrcId m_devices { };
   mutable vector<int> m_solenoids;
   mutable vector<unsigned int> m_solenoidMap;
   mutable vector<PinmameSolenoidState> m_solenoidStates;
   mutable vector<int> m_gis;
   mutable vector<unsigned int> m_giMap;
   mutable vector<PinmameGIState> m_giStates;
   mutable vector<int> m_lamps;
   mutable vector<unsigned int> m_lampMap;
   mutable vector<PinmameLampState> m_lampStates;
   static void OnDeviceSrcChanged(const unsigned int msgId, void* userData, void* msgData);
   void UpdateDeviceSrc() const;
   uint8_t GetGIValue(float value) const;
   uint8_t GetLampValue(uint8_t value) const { return m_deviceMode == DM_PHYSOUT ? value : (value != 0 ? 1 : 0); }
   uint8_t GetSolenoidValue(int solIndex, uint8_t value) const;

   unsigned int m_getDmdSrcMsgId, m_onDmdChangedMsgId;
   bool m_dmdUpdatePending = true;
   DisplaySrcId m_defaultDmd { };
   static void OnDmdSrcChanged(const unsigned int msgId, void* userData, void* msgData);
   void UpdateDmdSrc();

   void (*m_onDestroyHandler)(Controller*) = nullptr;
   void (*m_onGameStartHandler)(Controller*) = nullptr;
   void (*m_onGameEndHandler)(Controller*) = nullptr;

   bool m_cheat = false;

   const std::thread::id m_threadLock;
};

}
