#pragma once

#include "libpinmame.h"
#include "altsound.h"
#include "DMDUtil/DMDUtil.h"

#include "vpinmame_i.h"
#include "../common/DMDWindow.h"

#include <map>

class VPinMAMEGames;

typedef struct {
   PinmameDisplayLayout layout;
   UINT8 r;
   UINT8 g;
   UINT8 b;
   DMDUtil::DMD* pDMD;
} VPinMAMEDisplay;

class VPinMAMEController : public IController
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IController) {
         *ppv = reinterpret_cast<VPinMAMEController*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<VPinMAMEController*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<VPinMAMEController*>(this);
         AddRef();
         return S_OK;
      } else {
         *ppv = NULL;
         return E_NOINTERFACE;
      }
   }

   STDMETHOD_(ULONG, AddRef)() {
      m_dwRef++;
      return m_dwRef;
   }

   STDMETHOD_(ULONG, Release)() {
      ULONG dwRef = --m_dwRef;

      if (dwRef == 0)
         delete this;

      return dwRef;
   }

   STDMETHOD(GetTypeInfoCount)(UINT *pCountTypeInfo) { *pCountTypeInfo = 0; return S_OK;  }
   STDMETHOD(GetTypeInfo)(UINT iInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   static constexpr UINT8 LEVELS_WPC[] = { 0x14, 0x21, 0x43, 0x64 };
   static constexpr UINT8 LEVELS_SAM[] = { 0x00, 0x14, 0x19, 0x1E, 0x23, 0x28, 0x2D, 0x32, 0x37, 0x3C, 0x41, 0x46, 0x4B, 0x50, 0x5A, 0x64 };
   static constexpr UINT8 LEVELS_GTS3[] = { 0x00, 0x1E, 0x23, 0x28, 0x2D, 0x32, 0x37, 0x3C, 0x41, 0x46, 0x4B, 0x50, 0x55, 0x5A, 0x5F, 0x64 };

   VPinMAMEController();
   ~VPinMAMEController();

   STDMETHOD(get_ChangedLEDs)(/*[in]*/ int nHigh, int nLow, int nnHigh, int nnLow, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_ChangedLEDsState)(/*[in]*/ int nHigh, int nLow, int nnHigh, int nnLow, int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_Settings)(/*[out, retval]*/ IControllerSettings **pVal);
   STDMETHOD(get_Games)(/*[out, retval]*/ IGames **pVal);
   STDMETHOD(get_Version)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_SolMask)(/*[in]*/ int nLow, /*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_SolMask)(/*[in]*/ int nLow, /*[in]*/ LONG newVal);
   STDMETHOD(get_ModOutputType)(/*[in]*/int output,/*[in]*/ int no, /*[out, retval]*/ int* pVal);
   STDMETHOD(put_ModOutputType)(/*[in]*/int output, /*[in]*/int no, /*[in]*/ int newVal);
   STDMETHOD(put_TimeFence)(/*[in]*/ double fenceIns);
   STDMETHOD(put_Mech)(/*[in]*/ int param, /*[in]*/ int newVal);
   STDMETHOD(get_LockDisplay)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_LockDisplay)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_GetMech)(/*[in]*/ int mechNo, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_GIStrings)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Dip)(/*[in]*/ int nNo, /*[out, retval]*/ int *pVal);
   STDMETHOD(put_Dip)(/*[in]*/ int nNo, /*[in]*/ int newVal);
   STDMETHOD(get_Solenoids)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_SplashInfoLine)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SplashInfoLine)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_ChangedGIStrings)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_GIString)(int nString, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_HandleMechanics)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_HandleMechanics)(/*[in]*/ int newVal);
   STDMETHOD(get_Running)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Machines)(/*[in]*/ BSTR sMachine, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Pause)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Pause)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_HandleKeyboard)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HandleKeyboard)(/*[int]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_GameName)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_GameName)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_ChangedSolenoids)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Switches)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_Switches)(/*[out, retval]*/ VARIANT newVal);
   STDMETHOD(get_ChangedLamps)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Lamps)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_WPCNumbering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Switch)(/*[in]*/ int nSwitchNo, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Switch)(/*[in]*/ int nSwitchNo, /*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Solenoid)(/*[in]*/ int nSolenoid, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Lamp)(/*[in]*/ int nLamp, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(ShowAboutDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   STDMETHOD(Stop)();
   STDMETHOD(Run)(/*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd = 0, /*[in,defaultvalue(100)]*/ int nMinVersion = 0);
   STDMETHOD(get_Hidden)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Hidden)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_MechSamples)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_MechSamples)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Game)(/*[out, retval]*/ IGame **pVal);
   STDMETHOD(GetWindowRect)(/*[in,defaultvalue(0)]*/ LONG_PTR hWnd, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(GetClientRect)(/*[in,defaultvalue(0)]*/ LONG_PTR hWnd, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_NVRAM)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_ChangedNVRAM)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_RawDmdWidth)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_RawDmdHeight)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_RawDmdPixels)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_RawDmdColoredPixels)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_DmdWidth)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_DmdHeight)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_DmdPixel)(/*[in]*/ int x, /*[in]*/ int y, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_updateDmdPixels)(/*[in]*/ int **buf, /*[in]*/ int width, /*[in]*/ int height, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_ChangedLampsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_LampsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_ChangedSolenoidsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_SolenoidsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_ChangedGIsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_MasterVolume)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_MasterVolume)(/*[in]*/ int newVal);
   STDMETHOD(get_EnumAudioDevices)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_AudioDevicesCount)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_AudioDeviceDescription)(/*[in]*/ int num, /*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_AudioDeviceModule)(/*[in]*/ int num, /*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_CurrentAudioDevice)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_CurrentAudioDevice)(/*[in]*/ int num);
   STDMETHOD(get_ShowPinDMD)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowPinDMD)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowWinDMD)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowWinDMD)(/*[in]*/ VARIANT_BOOL newVal);

   /* deprecated methods/properties */
   STDMETHOD(get_NewSoundCommands)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(ShowPathesDialog)(/*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd);
   STDMETHOD(SetDisplayPosition)(/*[in]*/ int x, /*[in]*/ int y, /*[in]*/ LONG_PTR hParentWindow);
   STDMETHOD(get_DoubleSize)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DoubleSize)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowFrame)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowFrame)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowDMDOnly)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowDMDOnly)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(CheckROMS)(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_ShowTitle)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowTitle)(/*[in]*/ VARIANT_BOOL newpVal);
   STDMETHOD(ShowOptsDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   STDMETHOD(get_FastFrames)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_FastFrames)(/*[in]*/ int newVal);
   STDMETHOD(get_IgnoreRomCrc)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IgnoreRomCrc)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_CabinetMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_CabinetMode)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_SoundMode)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_SoundMode)(/*[in]*/ int newVal);
   STDMETHOD(get_ROMName)(/*[out, retval]*/ BSTR *pVal);

   static void PINMAMECALLBACK GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData);
   static void PINMAMECALLBACK OnDisplayAvailable(int index, int displayCount, PinmameDisplayLayout* p_displayLayout, const void* pUserData);
   static void PINMAMECALLBACK OnDisplayUpdated(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout, const void* pUserData);
   static int PINMAMECALLBACK OnAudioAvailable(PinmameAudioInfo* p_audioInfo, const void* pUserData);
   static int PINMAMECALLBACK OnAudioUpdated(void* p_buffer, int samples, const void* pUserData);
   static void PINMAMECALLBACK OnLogMessage(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, const void* pUserData);
   static void PINMAMECALLBACK OnSoundCommand(int boardNo, int cmd, const void* pUserData);
   string GetIniPath() { return m_szIniPath; }

private:
   string m_szPath;
   string m_szIniPath;
   string m_szSplashInfoLine;
   PinmameGame* m_pPinmameGame;
   PinmameMechConfig* m_pPinmameMechConfig;
   PinmameSolenoidState* m_pSolenoidBuffer;
   PinmameLampState* m_pLampBuffer;
   PinmameGIState* m_pGIBuffer;
   PinmameLEDState* m_pLEDBuffer;
   PinmameSoundCommand* m_pSoundCommandBuffer;
   PinmameNVRAMState* m_pNVRAMBuffer;
   VPinMAMEGames* m_pGames;
   vector<VPinMAMEDisplay*> m_displays;
   OLE_COLOR m_dmdColor;
   int m_enableSound;
   AudioPlayer* m_pAudioPlayer;
   int m_audioChannels;
   VPinMAMEDisplay* m_pActiveDisplay;
   VP::DMDWindow* m_pDMDWindow;
   bool m_hidden;
   DMDUtil::RGB24DMD* m_pRGB24DMD;
   DMDUtil::LevelDMD* m_pLevelDMD;
   std::thread* m_pThread;
   bool m_running;

   ULONG m_dwRef = 0;
};
