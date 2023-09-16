#pragma once

#include "b2s_i.h"

#include "../vpinmame/VPinMAMEController.h"
#include "forms/FormBackglass.h"
#include "classes/B2SCollectData.h"

class Server : public IDispatch
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID__Server) {
         *ppv = reinterpret_cast<Server*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<Server*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<Server*>(this);
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
      m_dwRef--;

      if (m_dwRef == 0)
         delete this;

      return m_dwRef;
   }

   STDMETHOD(GetTypeInfoCount)(UINT *pCountTypeInfo) { *pCountTypeInfo = 0; return S_OK;  }
   STDMETHOD(GetTypeInfo)(UINT iInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   Server();
   ~Server();

   STDMETHOD(Dispose)();
   STDMETHOD(get_B2SServerVersion)(BSTR *pRetVal);
   STDMETHOD(get_B2SServerDirectory)(BSTR *pRetVal);
   STDMETHOD(get_GameName)(BSTR *pRetVal);
   STDMETHOD(put_GameName)(BSTR pRetVal);
   STDMETHOD(get_ROMName)(BSTR *pRetVal);
   STDMETHOD(get_B2SName)(BSTR *pRetVal);
   STDMETHOD(put_B2SName)(BSTR pRetVal);
   STDMETHOD(get_TableName)(BSTR *pRetVal);
   STDMETHOD(put_TableName)(BSTR pRetVal);
   STDMETHOD(put_WorkingDir)(BSTR rhs);
   STDMETHOD(SetPath)(BSTR path);
   STDMETHOD(get_Games)(BSTR GameName, VARIANT *pRetVal);
   STDMETHOD(get_Settings)(VARIANT *pRetVal);
   STDMETHOD(get_Running)(VARIANT_BOOL *pRetVal);
   STDMETHOD(get_Pause)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Pause)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_Version)(BSTR *pRetVal);
   STDMETHOD(Run)(VARIANT handle);
   STDMETHOD(Stop)();
   STDMETHOD(get_LaunchBackglass)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_LaunchBackglass)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_SplashInfoLine)(BSTR *pRetVal);
   STDMETHOD(put_SplashInfoLine)(BSTR pRetVal);
   STDMETHOD(get_ShowFrame)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_ShowFrame)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ShowTitle)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_ShowTitle)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ShowDMDOnly)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_ShowDMDOnly)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ShowPinDMD)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_ShowPinDMD)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_LockDisplay)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_LockDisplay)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_DoubleSize)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_DoubleSize)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_Hidden)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Hidden)(VARIANT_BOOL pRetVal);
   STDMETHOD(SetDisplayPosition)(VARIANT x, VARIANT y, VARIANT handle);
   STDMETHOD(ShowOptsDialog)(VARIANT handle);
   STDMETHOD(ShowPathesDialog)(VARIANT handle);
   STDMETHOD(ShowAboutDialog)(VARIANT handle);
   STDMETHOD(CheckROMS)(VARIANT showoptions, VARIANT handle);
   STDMETHOD(get_PuPHide)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_PuPHide)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_HandleKeyboard)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_HandleKeyboard)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_HandleMechanics)(short *pRetVal);
   STDMETHOD(put_HandleMechanics)(short pRetVal);
   STDMETHOD(get_ChangedLamps)(VARIANT *pRetVal);
   STDMETHOD(get_ChangedSolenoids)(VARIANT *pRetVal);
   STDMETHOD(get_ChangedGIStrings)(VARIANT *pRetVal);
   STDMETHOD(get_ChangedLEDs)(VARIANT mask2, VARIANT mask1, VARIANT mask3, VARIANT mask4, VARIANT *pRetVal);
   STDMETHOD(get_NewSoundCommands)(VARIANT *pRetVal);
   STDMETHOD(get_Lamp)(VARIANT number, VARIANT_BOOL *pRetVal);
   STDMETHOD(get_Solenoid)(VARIANT number, VARIANT_BOOL *pRetVal);
   STDMETHOD(get_GIString)(VARIANT number, VARIANT_BOOL *pRetVal);
   STDMETHOD(get_Switch)(VARIANT number, VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Switch)(VARIANT number, VARIANT_BOOL pRetVal);
   STDMETHOD(get_Mech)(VARIANT number, LONG *pRetVal);
   STDMETHOD(put_Mech)(VARIANT number, LONG pRetVal);
   STDMETHOD(get_GetMech)(VARIANT number, VARIANT *pRetVal);
   STDMETHOD(get_Dip)(VARIANT number, LONG *pRetVal);
   STDMETHOD(put_Dip)(VARIANT number, LONG pRetVal);
   STDMETHOD(get_SolMask)(VARIANT number, LONG *pRetVal);
   STDMETHOD(put_SolMask)(VARIANT number, LONG pRetVal);
   STDMETHOD(get_RawDmdWidth)(LONG *pRetVal);
   STDMETHOD(get_RawDmdHeight)(LONG *pRetVal);
   STDMETHOD(get_RawDmdPixels)(VARIANT *pRetVal);
   STDMETHOD(get_RawDmdColoredPixels)(VARIANT *pRetVal);
   STDMETHOD(get_ChangedNVRAM)(VARIANT *pRetVal);
   STDMETHOD(get_NVRAM)(VARIANT *pRetVal);
   STDMETHOD(get_SoundMode)(LONG *pRetVal);
   STDMETHOD(put_SoundMode)(LONG pRetVal);
   STDMETHOD(B2SSetData)(VARIANT idORname, VARIANT value);
   STDMETHOD(B2SPulseData)(VARIANT idORname);
   STDMETHOD(B2SSetIllumination)(VARIANT name, VARIANT value);
   STDMETHOD(B2SSetLED)(VARIANT digit, VARIANT valueORtext);
   STDMETHOD(B2SSetLEDDisplay)(VARIANT display, VARIANT text);
   STDMETHOD(B2SSetReel)(VARIANT digit, VARIANT value);
   STDMETHOD(B2SSetScore)(VARIANT display, VARIANT value);
   STDMETHOD(B2SSetScorePlayer)(VARIANT playerno, VARIANT score);
   STDMETHOD(B2SSetScorePlayer1)(VARIANT score);
   STDMETHOD(B2SSetScorePlayer2)(VARIANT score);
   STDMETHOD(B2SSetScorePlayer3)(VARIANT score);
   STDMETHOD(B2SSetScorePlayer4)(VARIANT score);
   STDMETHOD(B2SSetScorePlayer5)(VARIANT score);
   STDMETHOD(B2SSetScorePlayer6)(VARIANT score);
   STDMETHOD(B2SSetScoreDigit)(VARIANT digit, VARIANT value);
   STDMETHOD(B2SSetScoreRollover)(VARIANT id, VARIANT value);
   STDMETHOD(B2SSetScoreRolloverPlayer1)(VARIANT value);
   STDMETHOD(B2SSetScoreRolloverPlayer2)(VARIANT value);
   STDMETHOD(B2SSetScoreRolloverPlayer3)(VARIANT value);
   STDMETHOD(B2SSetScoreRolloverPlayer4)(VARIANT value);
   STDMETHOD(B2SSetCredits)(VARIANT digitORvalue, VARIANT value);
   STDMETHOD(B2SSetPlayerUp)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SSetCanPlay)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SSetBallInPlay)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SSetTilt)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SSetMatch)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SSetGameOver)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SSetShootAgain)(VARIANT idORvalue, VARIANT value);
   STDMETHOD(B2SStartAnimation)(BSTR animationname, VARIANT_BOOL playreverse);
   STDMETHOD(B2SStartAnimationReverse)(BSTR animationname);
   STDMETHOD(B2SStopAnimation)(BSTR animationname);
   STDMETHOD(B2SStopAllAnimations)();
   STDMETHOD(B2SIsAnimationRunning)(BSTR animationname, VARIANT_BOOL *pRetVal);
   STDMETHOD(StartAnimation)(BSTR animationname, VARIANT_BOOL playreverse);
   STDMETHOD(StopAnimation)(BSTR animationname);
   STDMETHOD(B2SStartRotation)();
   STDMETHOD(B2SStopRotation)();
   STDMETHOD(B2SShowScoreDisplays)();
   STDMETHOD(B2SHideScoreDisplays)();
   STDMETHOD(B2SStartSound)(BSTR soundname);
   STDMETHOD(B2SPlaySound)(BSTR soundname);
   STDMETHOD(B2SStopSound)(BSTR soundname);
   STDMETHOD(B2SMapSound)(VARIANT digit, BSTR soundname);
 
   VPinMAMEController* GetVPinMAME() { if (!m_pVPinMAMEController) { m_pVPinMAMEController = new VPinMAMEController(); } return m_pVPinMAMEController; }
 
private:
   void Startup();
   void ShowBackglassForm();
   void HideBackglassForm();
   void KillBackglassForm();

   void MyB2SSetData(int id, int value);
   void MyB2SSetData(string groupname, int value);

   void MyB2SSetLED(int digit, int value);
   void MyB2SSetLED(int digit, string value);

   void MyB2SSetScore(int digit, int value, bool animateReelChange, bool useLEDs = false, bool useLEDDisplays = false, bool useReels = false, int reeltype = 0, eLEDType ledtype = eLEDType_Undefined);
   void MyB2SSetScore(int digit, int score);
   void MyB2SSetScorePlayer(int playerno, int score);

   B2SSettings* m_pB2SSettings;
   B2SData* m_pB2SData;

   FormBackglass* m_pFormBackglass;
   VPinMAMEController* m_pVPinMAMEController;

   bool m_isVisibleStateSet;
   bool m_lastTopVisible;
   bool m_lastSecondVisible;

   bool m_changedLampsCalled;
   bool m_changedSolenoidsCalled;
   bool m_changedGIStringsCalled;
   bool m_changedLEDsCalled;

   B2SCollectData* m_pCollectLampsData;
   B2SCollectData* m_pCollectSolenoidsData;
   B2SCollectData* m_pCollectGIStringsData;
   B2SCollectData* m_pCollectLEDsData;

   int GetFirstDigitOfDisplay(int display);

   void CheckLamps(SAFEARRAY* psa);
   void CheckSolenoids(SAFEARRAY* psa);

   string m_szPath;
   
   ULONG m_dwRef = 0;
};