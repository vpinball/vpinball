#include "stdafx.h"

#include "Server.h"

#include "forms/FormBackglass.h"
#include "controls/B2SPictureBox.h"
#include "controls/B2SReelBox.h"
#include "controls/B2SLEDBox.h"
#include "dream7/Dream7Display.h"
#include "classes/B2SReelDisplay.h"
#include "classes/LEDDisplayDigitLocation.h"
#include "classes/CollectData.h"
#include "classes/AnimationInfo.h"
#include "plugin/PluginHost.h"

Server::Server()
{
   m_pB2SSettings = B2SSettings::GetInstance();
   m_pB2SData = B2SData::GetInstance();

   m_pFormBackglass = NULL;

   m_isVisibleStateSet = false;
   m_lastTopVisible = false;
   m_lastSecondVisible = false;

   m_lampThreshold = 0;
   m_giStringThreshold = 4;

   m_changedLampsCalled = false;
   m_changedSolenoidsCalled = false;
   m_changedGIStringsCalled = false;
   m_changedLEDsCalled = false;

   m_pCollectLampsData = new B2SCollectData(m_pB2SSettings->GetLampsSkipFrames());
   m_pCollectSolenoidsData = new B2SCollectData(m_pB2SSettings->GetSolenoidsSkipFrames());
   m_pCollectGIStringsData = new B2SCollectData(m_pB2SSettings->GetGIStringsSkipFrames());
   m_pCollectLEDsData = new B2SCollectData(m_pB2SSettings->GetLEDsSkipFrames());

   m_pTimer = new VP::Timer();
   m_pTimer->SetInterval(37);
   m_pTimer->SetElapsedListener(std::bind(&Server::TimerElapsed, this, std::placeholders::_1));

   srand(time(0));
}

Server::~Server()
{
   delete m_pTimer;

   m_pB2SData->FreeVPinMAME();

   if (m_pFormBackglass)
      delete m_pFormBackglass;

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->UnregisterAllPlugins();
}

void Server::TimerElapsed(VP::Timer* pTimer)
{
   // have a look for important pollings
   static int counter = 0;
   static bool callLamps = false;
   static bool callSolenoids = false;
   static bool callGIStrings = false;
   static bool callLEDs = false;
   static bool logged = false;

   if (counter <= 25) {
      counter++;
      callLamps = !m_changedLampsCalled && (m_pB2SData->IsUseRomLamps() || m_pB2SData->IsUseAnimationLamps());
      callSolenoids = !m_changedSolenoidsCalled && (m_pB2SData->IsUseRomSolenoids() || m_pB2SData->IsUseAnimationSolenoids());
      callGIStrings = !m_changedGIStringsCalled && (m_pB2SData->IsUseRomGIStrings() || m_pB2SData->IsUseAnimationGIStrings());
      callLEDs = !m_changedLEDsCalled && (m_pB2SData->IsUseLEDs() || m_pB2SData->IsUseLEDDisplays() || m_pB2SData->IsUseReels());
   }
   else {
      if (m_pB2SSettings->IsROMControlled()) {
         bool changed = false;
         if (callLamps) {
            if (!m_changedLampsCalled) {
               VARIANT ret;
               VariantInit(&ret);
               GetChangedLamps(&ret);
               VariantClear(&ret);
            }
            else {
               callLamps = false;
               changed = true;
            }
         }
         if (callSolenoids) {
            if (!m_changedSolenoidsCalled) {
               VARIANT ret;
               VariantInit(&ret);
               GetChangedSolenoids(&ret);
               VariantClear(&ret);
            }
            else {
               callSolenoids = false;
               changed = true;
            }
         }
         if (callGIStrings) {
            if (!m_changedGIStringsCalled) {
               VARIANT ret;
               VariantInit(&ret);
               GetChangedGIStrings(&ret);
               VariantClear(&ret);
            }
            else {
               callGIStrings = false;
               changed = true;
            }
         }
         if (callLEDs) {
            if (!m_changedLEDsCalled) {
               VARIANT varMask1;
               VariantInit(&varMask1);
               V_VT(&varMask1) = VT_I4;
               V_I4(&varMask1) = 0xFFFFFFFF;

               VARIANT varMask2;
               VariantInit(&varMask2);
               V_VT(&varMask2) = VT_I4;
               V_I4(&varMask2) = 0;

               VARIANT ret;
               VariantInit(&ret);
               GetChangedLEDs(varMask1, varMask1, varMask2, varMask2, &ret);
               VariantClear(&ret);
               VariantClear(&varMask1);
               VariantClear(&varMask2);
            }
            else {
               callLEDs = false;
               changed = true;
            }
         }

         if (!logged || changed) {
            PLOGI.printf("B2S polling status: lamps=%d, solenoids=%d, giStrings=%d, leds=%d", callLamps, callSolenoids, callGIStrings, callLEDs);

            if (!callLamps && !callSolenoids && !callGIStrings && !callLEDs)
               pTimer->Stop();

            logged = true;
         }
      }
   }
}

STDMETHODIMP Server::Dispose()
{
   return S_OK;
}

STDMETHODIMP Server::get_B2SServerVersion(BSTR *pRetVal)
{
   const WCHAR *const wzVersion = MakeWide(m_pB2SSettings->GetDirectB2SVersion());
   *pRetVal = SysAllocString(wzVersion);
   delete [] wzVersion;

   return S_OK;
}

STDMETHODIMP Server::get_B2SServerDirectory(BSTR *pRetVal)
{
   const WCHAR *const wzPath = MakeWide(g_pvp->m_szMyPath);
   *pRetVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP Server::get_GameName(BSTR *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_GameName(pRetVal);
}

STDMETHODIMP Server::put_GameName(BSTR pRetVal)
{
   m_pB2SData->GetVPinMAME()->put_GameName(pRetVal);
   m_pB2SSettings->SetGameName(MakeString(pRetVal));
   m_pB2SSettings->SetB2SName("");

   return S_OK;
}

STDMETHODIMP Server::get_ROMName(BSTR *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_ROMName(pRetVal);
}

STDMETHODIMP Server::get_B2SName(BSTR *pRetVal)
{
   const WCHAR *const wzB2SName = MakeWide(m_pB2SSettings->GetB2SName());
   *pRetVal = SysAllocString(wzB2SName);
   delete [] wzB2SName;

   return S_OK;
}

STDMETHODIMP Server::put_B2SName(BSTR pRetVal)
{
   string szB2SName = MakeString(pRetVal);
   szB2SName.erase(std::remove(szB2SName.begin(), szB2SName.end(), ' '), szB2SName.end());
   m_pB2SSettings->SetB2SName(szB2SName);
   m_pB2SSettings->SetGameName("");

   return S_OK;
}

STDMETHODIMP Server::get_TableName(BSTR *pRetVal)
{
   const WCHAR *const wzTableFileName = MakeWide(m_pB2SData->GetTableFileName());
   *pRetVal = SysAllocString(wzTableFileName);
   delete [] wzTableFileName;

   return S_OK;
}

STDMETHODIMP Server::put_TableName(BSTR pRetVal)
{
   m_pB2SData->SetTableFileName(MakeString(pRetVal));

   return S_OK;
}

STDMETHODIMP Server::put_WorkingDir(BSTR rhs)
{
   m_szPath = MakeString(rhs);

   return S_OK;
}

STDMETHODIMP Server::SetPath(BSTR path)
{
   m_szPath = MakeString(path);

   return S_OK;
}

STDMETHODIMP Server::get_Games(BSTR GameName, VARIANT *pRetVal)
{
   IGames* pGames;
   IGame* pGame;

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_Games(&pGames);

   if (hres == S_OK) {
      VARIANT var0;
      V_VT(&var0) = VT_BSTR;
      V_BSTR(&var0) = SysAllocString(GameName);

      hres = pGames->get_Item(&var0, &pGame);

      VariantClear(&var0);

      if (hres == S_OK) {
         V_VT(pRetVal) = VT_DISPATCH;
         V_DISPATCH(pRetVal) = pGame;
      }

      pGames->Release();
   }

   return hres;
}

STDMETHODIMP Server::get_Settings(VARIANT *pRetVal)
{
   IControllerSettings* pSettings;

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_Settings(&pSettings);

   if (hres == S_OK) {
      V_VT(pRetVal) = VT_DISPATCH;
      V_DISPATCH(pRetVal) = pSettings;
   }

   return hres;
}

STDMETHODIMP Server::get_Running(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_Running(pRetVal);
}

STDMETHODIMP Server::put_TimeFence(VARIANT timeInS)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &timeInS, 0, VT_R8);

   HRESULT hres = m_pB2SData->GetVPinMAME()->put_TimeFence(V_R8(&var0));

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_Pause(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_Pause(pRetVal);
}

STDMETHODIMP Server::put_Pause(VARIANT_BOOL pRetVal)
{
   HRESULT hres = m_pB2SData->GetVPinMAME()->put_Pause(pRetVal);

   if (m_pB2SSettings->ArePluginsOn()) {
      if (pRetVal == VARIANT_TRUE)
         m_pB2SSettings->GetPluginHost()->PinMamePause();
      else
         m_pB2SSettings->GetPluginHost()->PinMameContinue();
   }
   return hres;
}

STDMETHODIMP Server::get_Version(BSTR *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_Version(pRetVal);
}

STDMETHODIMP Server::Run(VARIANT handle)
{
   Startup();

   if (m_pB2SSettings->ArePluginsOn()) {
      m_pB2SSettings->GetPluginHost()->PluginInit(m_pB2SData->GetTableFileName(), 
         !m_pB2SSettings->GetB2SName().empty() ? m_pB2SSettings->GetB2SName() : m_pB2SSettings->GetGameName());
   }

   ShowBackglassForm();

   if (m_pB2SSettings->IsROMControlled()) {
      VARIANT var0;
      V_VT(&var0) = VT_EMPTY;
      VariantChangeType(&var0, &handle, 0, VT_I4);
      m_pB2SData->GetVPinMAME()->Run(V_I4(&var0));

      if (m_pB2SSettings->ArePluginsOn())
         m_pB2SSettings->GetPluginHost()->PinMameRun();

      VariantClear(&var0);

      // start end timer
      m_pTimer->Start();
   }

   return S_OK;
}

STDMETHODIMP Server::Stop()
{
   m_pTimer->Stop();
   HideBackglassForm();

   m_pB2SData->Stop();
   KillBackglassForm();   

   if (m_pB2SSettings->ArePluginsOn()) {
      m_pB2SSettings->GetPluginHost()->PinMameStop();
      m_pB2SSettings->GetPluginHost()->PluginFinish();
   }

   return S_OK;
}

STDMETHODIMP Server::get_LaunchBackglass(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_pB2SData->IsLaunchBackglass() ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Server::put_LaunchBackglass(VARIANT_BOOL pRetVal)
{
   m_pB2SData->SetLaunchBackglass(pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP Server::get_SplashInfoLine(BSTR *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_SplashInfoLine(pRetVal);
}

STDMETHODIMP Server::put_SplashInfoLine(BSTR pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_SplashInfoLine(pRetVal);
}

STDMETHODIMP Server::get_ShowFrame(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_ShowFrame(pRetVal);
}

STDMETHODIMP Server::put_ShowFrame(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_ShowFrame(pRetVal);
}

STDMETHODIMP Server::get_ShowTitle(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_ShowTitle(pRetVal);
}

STDMETHODIMP Server::put_ShowTitle(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_ShowTitle(pRetVal);
}

STDMETHODIMP Server::get_ShowDMDOnly(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_ShowDMDOnly(pRetVal);
}

STDMETHODIMP Server::put_ShowDMDOnly(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_ShowDMDOnly(pRetVal);
}

STDMETHODIMP Server::get_ShowPinDMD(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_ShowPinDMD(pRetVal);
}

STDMETHODIMP Server::put_ShowPinDMD(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_ShowPinDMD(pRetVal);
}

STDMETHODIMP Server::get_LockDisplay(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_LockDisplay(pRetVal);
}

STDMETHODIMP Server::put_LockDisplay(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_LockDisplay(pRetVal);
}

STDMETHODIMP Server::get_DoubleSize(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_DoubleSize(pRetVal);
}

STDMETHODIMP Server::put_DoubleSize(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_DoubleSize(pRetVal);
}

STDMETHODIMP Server::get_Hidden(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_Hidden(pRetVal);
}

STDMETHODIMP Server::put_Hidden(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_Hidden(pRetVal);
}

STDMETHODIMP Server::SetDisplayPosition(VARIANT x, VARIANT y, VARIANT handle)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &x, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &y, 0, VT_I4);

   VARIANT var2;
   V_VT(&var2) = VT_EMPTY;
   VariantChangeType(&var2, &handle, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->SetDisplayPosition(V_I4(&var0), V_I4(&var1), V_I4(&var2));

   VariantClear(&var0);
   VariantClear(&var1);
   VariantClear(&var2);

   return hres;
}

STDMETHODIMP Server::ShowOptsDialog(VARIANT handle)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &handle, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->ShowOptsDialog(V_I4(&var0));

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::ShowPathesDialog(VARIANT handle)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &handle, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->ShowPathesDialog(V_I4(&var0));

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::ShowAboutDialog(VARIANT handle)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &handle, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->ShowAboutDialog(V_I4(&var0));

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::CheckROMS(VARIANT showoptions, VARIANT handle)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP Server::get_PuPHide(VARIANT_BOOL *pRetVal)
{
   *pRetVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Server::put_PuPHide(VARIANT_BOOL pRetVal)
{
   if (pRetVal == VARIANT_TRUE) {
      if (m_pB2SSettings->ArePluginsOn())
         m_pB2SSettings->GetPluginHost()->DisablePup();
   }

   return S_OK;
}

STDMETHODIMP Server::get_HandleKeyboard(VARIANT_BOOL *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_HandleKeyboard(pRetVal);
}

STDMETHODIMP Server::put_HandleKeyboard(VARIANT_BOOL pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_HandleKeyboard(pRetVal);
}

STDMETHODIMP Server::get_HandleMechanics(short *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_HandleMechanics(reinterpret_cast<int*>(pRetVal));
}

STDMETHODIMP Server::put_HandleMechanics(short pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_HandleMechanics(pRetVal);
}

STDMETHODIMP Server::get_ChangedLamps(VARIANT *pRetVal)
{
   m_changedLampsCalled = true;
   return GetChangedLamps(pRetVal);
}

STDMETHODIMP Server::get_ChangedSolenoids(VARIANT *pRetVal)
{
   m_changedSolenoidsCalled = true;
   return GetChangedSolenoids(pRetVal);
}

STDMETHODIMP Server::get_ChangedGIStrings(VARIANT *pRetVal)
{
   m_changedGIStringsCalled = true;
   return GetChangedGIStrings(pRetVal);
}

STDMETHODIMP Server::get_ChangedLEDs(VARIANT mask2, VARIANT mask1, VARIANT mask3, VARIANT mask4, VARIANT *pRetVal)
{
   m_changedLEDsCalled = true;
   return GetChangedLEDs(mask2, mask1, mask3, mask4, pRetVal);
}

STDMETHODIMP Server::get_NewSoundCommands(VARIANT *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_NewSoundCommands(pRetVal);
}

STDMETHODIMP Server::get_Lamp(VARIANT number, VARIANT_BOOL *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_Lamp(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_Solenoid(VARIANT number, VARIANT_BOOL *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_Solenoid(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_GIString(VARIANT number, VARIANT_BOOL *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   int val;
   HRESULT hres = m_pB2SData->GetVPinMAME()->get_GIString(V_I4(&var0), &val);
   if (hres == S_OK)
      *pRetVal = val ? VARIANT_TRUE : VARIANT_FALSE;

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_Switch(VARIANT number, VARIANT_BOOL *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_Switch(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::put_Switch(VARIANT number, VARIANT_BOOL pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->put_Switch(V_I4(&var0), pRetVal);

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('W', V_I4(&var0), pRetVal == VARIANT_TRUE ? 1 : 0 );

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_Mech(VARIANT number, LONG *pRetVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP Server::put_Mech(VARIANT number, LONG pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->put_Mech(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_GetMech(VARIANT number, VARIANT *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   int val;
   HRESULT hres = m_pB2SData->GetVPinMAME()->get_GetMech(V_I4(&var0), &val);
   V_VT(pRetVal) = VT_I4;
   V_I4(pRetVal) = val;

   CheckGetMech(V_I4(&var0), val);

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('N', V_I4(&var0), val);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_Dip(VARIANT number, LONG *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_Dip(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::put_Dip(VARIANT number, LONG pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->put_Dip(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_SolMask(VARIANT number, LONG *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_SolMask(V_I4(&var0), pRetVal);

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::put_SolMask(VARIANT number, LONG pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &number, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->put_SolMask(V_I4(&var0), pRetVal);

   // There is a new setting for VPinMame.SolMask(2) to set the output mode:
   // 0 = default
   // 1 = modulated (PWM) solenoid (exist for some years already)
   // 2 = new PWM mode (all solenoids but also lamps, and value if physic meaning, not smoothed out binary state)
   // For this new mode, we now hardcode a value 64, if the lamp intensity exceed this value, it is binary 1
   if (V_I4(&var0) == 2)
      m_lampThreshold = (pRetVal == 2) ? 64 : 0;
   if (V_I4(&var0) == 2)
      m_giStringThreshold = (pRetVal == 2) ? 64 : 4;

   VariantClear(&var0);

   return hres;
}

STDMETHODIMP Server::get_RawDmdWidth(LONG *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_RawDmdWidth(pRetVal);
}

STDMETHODIMP Server::get_RawDmdHeight(LONG *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_RawDmdHeight(pRetVal);
}

STDMETHODIMP Server::get_RawDmdPixels(VARIANT *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_RawDmdPixels(pRetVal);
}

STDMETHODIMP Server::get_RawDmdColoredPixels(VARIANT *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_RawDmdColoredPixels(pRetVal);
}

STDMETHODIMP Server::get_ChangedNVRAM(VARIANT *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_ChangedNVRAM(pRetVal);
}

STDMETHODIMP Server::get_NVRAM(VARIANT *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_NVRAM(pRetVal);
}

STDMETHODIMP Server::get_SoundMode(LONG *pRetVal)
{
   return m_pB2SData->GetVPinMAME()->get_SoundMode(pRetVal);
}

STDMETHODIMP Server::put_SoundMode(LONG pRetVal)
{
   return m_pB2SData->GetVPinMAME()->put_SoundMode(pRetVal);
}

STDMETHODIMP Server::B2SSetData(VARIANT idORname, VARIANT value)
{
   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &value, 0, VT_I4);

   if (V_VT(&idORname) == VT_BSTR) {
      MyB2SSetData(MakeString(V_BSTR(&idORname)), V_I4(&var1));
   }
   else {
      VARIANT var0;
      V_VT(&var0) = VT_EMPTY;
      VariantChangeType(&var0, &idORname, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var0);
   }

   VariantClear(&var1);

   return S_OK;
}

STDMETHODIMP Server::B2SPulseData(VARIANT idORname)
{
   if (V_VT(&idORname) == VT_BSTR) {
      MyB2SSetData(MakeString(V_BSTR(&idORname)), 1);
      MyB2SSetData(MakeString(V_BSTR(&idORname)), 0);
   }
   else {
      VARIANT var0;
      V_VT(&var0) = VT_EMPTY;
      VariantChangeType(&var0, &idORname, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), 1);
      MyB2SSetData(V_I4(&var0), 0);

      VariantClear(&var0);
   }

   return S_OK;
}

STDMETHODIMP Server::B2SSetIllumination(VARIANT name, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &name, 0, VT_BSTR);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &value, 0, VT_I4);

   MyB2SSetData(MakeString(V_BSTR(&var0)), V_I4(&var1));

   VariantClear(&var0);
   VariantClear(&var1);

   return S_OK;
}

STDMETHODIMP Server::B2SSetLED(VARIANT digit, VARIANT valueORtext)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &digit, 0, VT_I4);

   if (V_VT(&valueORtext) == VT_BSTR) {
      MyB2SSetLED(V_I4(&var0), MakeString(V_BSTR(&valueORtext)));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &valueORtext, 0, VT_I4);

      MyB2SSetLED(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetLEDDisplay(VARIANT display, VARIANT text)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &display, 0, VT_I4);

   MyB2SSetLEDDisplay(V_I4(&var0), MakeString(V_BSTR(&text)));

   VariantClear(&var0);

   return S_OK;
}

// reel method(s)
STDMETHODIMP Server::B2SSetReel(VARIANT digit, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &digit, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &value, 0, VT_I4);

   MyB2SSetScore(V_I4(&var0), V_I4(&var1), true);

   VariantClear(&var0);
   VariantClear(&var1);

   return S_OK;
}

// score: 1-24
STDMETHODIMP Server::B2SSetScore(VARIANT display, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &display, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &value, 0, VT_I4);

   MyB2SSetScore(GetFirstDigitOfDisplay(V_I4(&var0)), V_I4(&var1));

   VariantClear(&var0);
   VariantClear(&var1);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer(VARIANT playerno, VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &playerno, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &score, 0, VT_I4);

   MyB2SSetScorePlayer(V_I4(&var0), V_I4(&var1));

   VariantClear(&var0);
   VariantClear(&var1);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer1(VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &score, 0, VT_I4);

   MyB2SSetScore(1, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer2(VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &score, 0, VT_I4);

   MyB2SSetScore(2, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer3(VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &score, 0, VT_I4);

   MyB2SSetScore(3, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer4(VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &score, 0, VT_I4);

   MyB2SSetScore(4, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer5(VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &score, 0, VT_I4);

   MyB2SSetScore(5, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScorePlayer6(VARIANT score)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &score, 0, VT_I4);

   MyB2SSetScore(6, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScoreDigit(VARIANT digit, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &digit, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &value, 0, VT_I4);

   MyB2SSetScore(V_I4(&var0), V_I4(&var1), false);

   VariantClear(&var0);
   VariantClear(&var1);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScoreRollover(VARIANT id, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &id, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &value, 0, VT_I4);

   MyB2SSetData(V_I4(&var0), V_I4(&var1));

   VariantClear(&var0);
   VariantClear(&var1);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScoreRolloverPlayer1(VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &value, 0, VT_I4);

   MyB2SSetData(25, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScoreRolloverPlayer2(VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &value, 0, VT_I4);

   MyB2SSetData(26, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScoreRolloverPlayer3(VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &value, 0, VT_I4);

   MyB2SSetData(27, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetScoreRolloverPlayer4(VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &value, 0, VT_I4);

   MyB2SSetData(28, V_I4(&var0));

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetCredits(VARIANT digitORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &digitORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetScore(29, V_I4(&var0), false);
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetScore(V_I4(&var0), V_I4(&var1), false);

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetPlayerUp(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(30, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetCanPlay(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(31, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetBallInPlay(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(32, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetTilt(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(33, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetMatch(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(34, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetGameOver(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(35, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SSetShootAgain(VARIANT idORvalue, VARIANT value)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &idORvalue, 0, VT_I4);

   if (V_VT(&value) == VT_EMPTY) {
      MyB2SSetData(36, V_I4(&var0));
   }
   else {
      VARIANT var1;
      V_VT(&var1) = VT_EMPTY;
      VariantChangeType(&var1, &value, 0, VT_I4);

      MyB2SSetData(V_I4(&var0), V_I4(&var1));

      VariantClear(&var1);
   }

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP Server::B2SStartAnimation(BSTR animationname, VARIANT_BOOL playreverse)
{
   MyB2SStartAnimation(MakeString(animationname), (playreverse == VARIANT_TRUE));

   return S_OK;
}

STDMETHODIMP Server::B2SStartAnimationReverse(BSTR animationname)
{
   MyB2SStartAnimation(MakeString(animationname), true);

   return S_OK;
}

STDMETHODIMP Server::B2SStopAnimation(BSTR animationname)
{
   MyB2SStopAnimation(MakeString(animationname));

   return S_OK;
}

STDMETHODIMP Server::B2SStopAllAnimations()
{
   MyB2SStopAllAnimations();

   return S_OK;
}

STDMETHODIMP Server::B2SIsAnimationRunning(BSTR animationname, VARIANT_BOOL *pRetVal)
{
   *pRetVal = MyB2SIsAnimationRunning(MakeString(animationname)) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Server::StartAnimation(BSTR animationname, VARIANT_BOOL playreverse)
{
   MyB2SStartAnimation(MakeString(animationname), (playreverse == VARIANT_TRUE));

   return S_OK;
}

STDMETHODIMP Server::StopAnimation(BSTR animationname)
{
   MyB2SStopAnimation(MakeString(animationname));

   return S_OK;
}

STDMETHODIMP Server::B2SStartRotation()
{
   MyB2SStartRotation();

   return S_OK;
}

STDMETHODIMP Server::B2SStopRotation()
{
   MyB2SStopRotation();

   return S_OK;
}

STDMETHODIMP Server::B2SShowScoreDisplays()
{
   MyB2SShowOrHideScoreDisplays(true);

   return S_OK;
}

STDMETHODIMP Server::B2SHideScoreDisplays()
{
    MyB2SShowOrHideScoreDisplays(false);

   return S_OK;
}

STDMETHODIMP Server::B2SStartSound(BSTR soundname)
{
   MyB2SPlaySound(MakeString(soundname));

   return S_OK;
}

STDMETHODIMP Server::B2SPlaySound(BSTR soundname)
{
   MyB2SPlaySound(MakeString(soundname));

   return S_OK;
}

STDMETHODIMP Server::B2SStopSound(BSTR soundname)
{
   MyB2SStopSound(MakeString(soundname));

   return S_OK;
}

STDMETHODIMP Server::B2SMapSound(VARIANT digit, BSTR soundname)
{
   return S_OK;
}

HRESULT Server::GetChangedLamps(VARIANT *pRetVal)
{
   HRESULT hres = m_pB2SData->GetVPinMAME()->get_ChangedLamps(pRetVal);

   SAFEARRAY* psa = pRetVal && V_VT(pRetVal) == (VT_ARRAY | VT_VARIANT) ? V_ARRAY(pRetVal) : NULL;

   if (m_pB2SData->IsLampsData())
      CheckLamps(psa);

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('L', psa);

   return hres;
}

HRESULT Server::GetChangedSolenoids(VARIANT *pRetVal)
{
   HRESULT hres = m_pB2SData->GetVPinMAME()->get_ChangedSolenoids(pRetVal);

   SAFEARRAY* psa = pRetVal && V_VT(pRetVal) == (VT_ARRAY | VT_VARIANT) ? V_ARRAY(pRetVal) : NULL;

   if (m_pB2SData->IsSolenoidsData())
      CheckSolenoids(psa);

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('S', psa);

   return hres;
}

HRESULT Server::GetChangedGIStrings(VARIANT *pRetVal)
{
   HRESULT hres = m_pB2SData->GetVPinMAME()->get_ChangedGIStrings(pRetVal);

   SAFEARRAY* psa = pRetVal && V_VT(pRetVal) == (VT_ARRAY | VT_VARIANT) ? V_ARRAY(pRetVal) : NULL;

   if (m_pB2SData->IsGIStringsData())
      CheckGIStrings(psa);

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('G', psa);

   return hres;
}

HRESULT Server::GetChangedLEDs(VARIANT mask2, VARIANT mask1, VARIANT mask3, VARIANT mask4, VARIANT *pRetVal)
{
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &mask2, 0, VT_I4);

   VARIANT var1;
   V_VT(&var1) = VT_EMPTY;
   VariantChangeType(&var1, &mask1, 0, VT_I4);

   VARIANT var2;
   V_VT(&var2) = VT_EMPTY;
   VariantChangeType(&var2, &mask3, 0, VT_I4);

   VARIANT var3;
   V_VT(&var3) = VT_EMPTY;
   VariantChangeType(&var3, &mask4, 0, VT_I4);

   HRESULT hres = m_pB2SData->GetVPinMAME()->get_ChangedLEDs(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), pRetVal);

   SAFEARRAY* psa = pRetVal && V_VT(pRetVal) == (VT_ARRAY | VT_VARIANT) ? V_ARRAY(pRetVal) : NULL;

   if (m_pB2SData->IsLEDsData())
      CheckLEDs(psa);

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('D', psa);

   VariantClear(&var0);
   VariantClear(&var1);
   VariantClear(&var2);
   VariantClear(&var3);

   return hres;
}

void Server::CheckGetMech(int number, int mech)
{
   if (number > 0) {
      int mechid = number;
      int mechvalue = mech;

      if (!m_pB2SSettings->GetBackglassFileVersion().empty() && m_pB2SSettings->GetBackglassFileVersion() <= "1.1")
         mechvalue -= 1;

      if (m_pB2SData->GetUsedRomMechIDs()->contains(mechid)) {
         if ((*m_pB2SData->GetRotatingPictureBox())[mechid] && m_pB2SData->GetRotatingImages()->contains(mechid) && (*m_pB2SData->GetRotatingImages())[mechid].size() > 0 && (*m_pB2SData->GetRotatingImages())[mechid].contains(mechvalue)) {
            (*m_pB2SData->GetRotatingPictureBox())[mechid]->SetBackgroundImage((*m_pB2SData->GetRotatingImages())[mechid][mechvalue]);
            (*m_pB2SData->GetRotatingPictureBox())[mechid]->SetVisible(true);
         }
      }
   }
}

void Server::CheckLamps(SAFEARRAY* psa)
{
   int lampId;
   bool lampState;

   if (psa) {
      LONG uCount = 0;
      LONG lBound;
      LONG uBound;

      if (SUCCEEDED(SafeArrayGetLBound(psa, 1, &lBound))) {
         if (SUCCEEDED(SafeArrayGetUBound(psa, 1, &uBound)))
            uCount = uBound - lBound + 1;
      }

      LONG ix[2];
      VARIANT varValue;

      for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
         ix[1] = 0;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         lampId = V_I4(&varValue);
         VariantClear(&varValue);

         ix[1] = 1;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         lampState = (V_I4(&varValue) > m_lampThreshold);
         VariantClear(&varValue);

         if (m_pB2SData->IsUseRomLamps() || m_pB2SData->IsUseAnimationLamps()) {
            // collect illumination data
            if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetTopRomID() == lampId) 
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_TopImage));
            else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetSecondRomID() == lampId)
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_SecondImage));
            if (m_pB2SData->GetUsedRomLampIDs()->contains(lampId))
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_Standard));

            // collect animation data
            if (m_pB2SData->GetUsedAnimationLampIDs()->contains(lampId) || m_pB2SData->GetUsedRandomAnimationLampIDs()->contains(lampId))
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_Animation));
         }
      }
   }

   // one collection loop is done
   m_pCollectLampsData->DataAdded();

   // maybe show the collected data
   if (m_pCollectLampsData->ShowData()) {
      for (const auto& [key, pCollectData] : *m_pCollectLampsData) {
         lampId = key;
         lampState = (pCollectData->GetState() > 0);
         int datatypes = pCollectData->GetTypes();

         // illumination stuff
         if ((datatypes & eCollectedDataType_TopImage) || (datatypes & eCollectedDataType_SecondImage)) {
            bool topvisible = m_lastTopVisible;
            bool secondvisible = m_lastSecondVisible;
            if (datatypes & eCollectedDataType_TopImage) {
               topvisible = lampState;
               if (m_pFormBackglass->IsTopRomInverted())
                  topvisible = !topvisible;
            }
            else if (datatypes & eCollectedDataType_SecondImage) {
               secondvisible = lampState;
               if (m_pFormBackglass->IsSecondRomInverted())
                  topvisible = !topvisible;
            }
            if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
               m_pB2SData->SetOffImageVisible(false);
               m_isVisibleStateSet = true;
               m_lastTopVisible = topvisible;
               m_lastSecondVisible = secondvisible;
               if (topvisible && secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
               else if (topvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
               else if (secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
               else {
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
                  m_pB2SData->SetOffImageVisible(true);
               }
            }
         }
         if (datatypes & eCollectedDataType_Standard) {
            for (const auto& pBase : (*m_pB2SData->GetUsedRomLampIDs())[lampId]) {
               B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
               if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
                  bool visible = lampState;
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  if (m_pB2SData->IsUseRotatingImage() && m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && pPicbox == (*m_pB2SData->GetRotatingPictureBox())[0]) {
                     if (visible)
                        m_pFormBackglass->StartRotation();
                     else
                        m_pFormBackglass->StopRotation();
                  }
                  else
                     pPicbox->SetVisible(visible);
               }
            }
         }

         // animation stuff
         if (datatypes & eCollectedDataType_Animation) {
            if (m_pB2SData->GetUsedAnimationLampIDs()->contains(lampId)) {
               for (const auto& animation : (*m_pB2SData->GetUsedAnimationLampIDs())[lampId]) {
                  bool start = lampState;
                  if (animation->IsInverted())
                     start = !start;
                  if (start)
                     m_pFormBackglass->StartAnimation(animation->GetAnimationName());
                  else
                     m_pFormBackglass->StopAnimation(animation->GetAnimationName());
               }
            }
            // random animation start
            if (m_pB2SData->GetUsedRandomAnimationLampIDs()->contains(lampId)) {
               bool start = lampState;
               bool isrunning = false;
               if (start) {
                  for (const auto& matchinganimation : (*m_pB2SData->GetUsedRandomAnimationLampIDs())[lampId]) {
                     if (m_pFormBackglass->IsAnimationRunning(matchinganimation->GetAnimationName())) {
                        isrunning = true;
                        break;
                     }
                  }
               }
               if (start) {
                  if (!isrunning) {
                     int random = RandomStarter((*m_pB2SData->GetUsedRandomAnimationLampIDs())[lampId].size());
                     auto& animation = (*m_pB2SData->GetUsedRandomAnimationLampIDs())[lampId][random];
                     m_lastRandomStartedAnimation = animation->GetAnimationName();
                     m_pFormBackglass->StartAnimation(m_lastRandomStartedAnimation);
                  }
               }
               else {
                  if (!m_lastRandomStartedAnimation.empty()) {
                     m_pFormBackglass->StopAnimation(m_lastRandomStartedAnimation);
                     m_lastRandomStartedAnimation = "";
                  }
               }
            }
         }
      }

      // reset all current data
      m_pCollectLampsData->ClearData(m_pB2SSettings->GetLampsSkipFrames());
   }
}

void Server::CheckSolenoids(SAFEARRAY* psa)
{
   int solenoidId;
   int solenoidState;

   if (psa) {
      LONG uCount = 0;
      LONG lBound;
      LONG uBound;

      if (SUCCEEDED(SafeArrayGetLBound(psa, 1, &lBound))) {
         if (SUCCEEDED(SafeArrayGetUBound(psa, 1, &uBound)))
            uCount = uBound - lBound + 1;
      }

      LONG ix[2];
      VARIANT varValue;

      for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
         ix[1] = 0;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         solenoidId = V_I4(&varValue);
         VariantClear(&varValue);

         ix[1] = 1;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         solenoidState = V_I4(&varValue);
         VariantClear(&varValue);

         if (m_pB2SData->IsUseRomSolenoids() || m_pB2SData->IsUseAnimationSolenoids()) {
            // collect illumination data
            if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_Solenoid && m_pFormBackglass->GetTopRomID() == solenoidId)
               m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_TopImage));
            else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Solenoid && m_pFormBackglass->GetSecondRomID() == solenoidId)
               m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_SecondImage));
            if (m_pB2SData->GetUsedRomSolenoidIDs()->contains(solenoidId))
               m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_Standard));

            // collect animation data
            if (m_pB2SData->GetUsedAnimationSolenoidIDs()->contains(solenoidId) || m_pB2SData->GetUsedRandomAnimationSolenoidIDs()->contains(solenoidId))
               m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_Animation));
         }
      }
   }

   // one collection loop is done
   m_pCollectSolenoidsData->DataAdded();

   // maybe show the collected data
   if (m_pCollectSolenoidsData->ShowData()) {
      for (const auto& [key, pCollectData] : *m_pCollectSolenoidsData) {
         solenoidId = key;
         solenoidState = pCollectData->GetState();
         int datatypes = pCollectData->GetTypes();

         // illumination stuff
         if ((datatypes & eCollectedDataType_TopImage) || (datatypes & eCollectedDataType_SecondImage)) {
            bool topvisible = m_lastTopVisible;
            bool secondvisible = m_lastSecondVisible;
            if (datatypes & eCollectedDataType_TopImage) {
               topvisible = (solenoidState != 0);
               if (m_pFormBackglass->IsTopRomInverted())
                  topvisible = !topvisible;
            }
            else if (datatypes & eCollectedDataType_SecondImage) {
               secondvisible = (solenoidState != 0);
               if (m_pFormBackglass->IsSecondRomInverted())
                  topvisible = !topvisible;
            }
            if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
               m_pB2SData->SetOffImageVisible(false);
               m_isVisibleStateSet = true;
               m_lastTopVisible = topvisible;
               m_lastSecondVisible = secondvisible;
               if (topvisible && secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
               else if (topvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
               else if (secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
               else {
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
                  m_pB2SData->SetOffImageVisible(true);
               }
            }
         }
         if (datatypes & eCollectedDataType_Standard) {
            for (const auto& pBase : (*m_pB2SData->GetUsedRomSolenoidIDs())[solenoidId]) {
               B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
               if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
                  bool visible = (solenoidState != 0);
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  if (m_pB2SData->IsUseRotatingImage() && m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && pPicbox == (*m_pB2SData->GetRotatingPictureBox())[0]) {
                     if (visible)
                        m_pFormBackglass->StartRotation();
                     else
                        m_pFormBackglass->StopRotation();
                  }
                  else
                     pPicbox->SetVisible(visible);
               }
            }
         }

         // animation stuff
         if (datatypes & eCollectedDataType_Animation) {
            if (m_pB2SData->GetUsedAnimationSolenoidIDs()->contains(solenoidId)) {
               for (const auto& animation : (*m_pB2SData->GetUsedAnimationSolenoidIDs())[solenoidId]) {
                  bool start = (solenoidState != 0);
                  if (animation->IsInverted())
                     start = !start;
                  if (start)
                     m_pFormBackglass->StartAnimation(animation->GetAnimationName());
                  else
                     m_pFormBackglass->StopAnimation(animation->GetAnimationName());
               }
            }
            // random animation start
            if (m_pB2SData->GetUsedRandomAnimationSolenoidIDs()->contains(solenoidId)) {
               bool start = (solenoidState != 0);
               bool isrunning = false;
               if (start) {
                  for (const auto& matchinganimation : (*m_pB2SData->GetUsedRandomAnimationSolenoidIDs())[solenoidId]) {
                     if (m_pFormBackglass->IsAnimationRunning(matchinganimation->GetAnimationName())) {
                        isrunning = true;
                        break;
                     }
                  }
               }
               if (start) {
                  if (!isrunning) {
                     int random = RandomStarter((*m_pB2SData->GetUsedRandomAnimationSolenoidIDs())[solenoidId].size());
                     auto& animation = (*m_pB2SData->GetUsedRandomAnimationSolenoidIDs())[solenoidId][random];
                     m_lastRandomStartedAnimation = animation->GetAnimationName();
                     m_pFormBackglass->StartAnimation(m_lastRandomStartedAnimation);
                  }
               }
               else {
                  if (!m_lastRandomStartedAnimation.empty()) {
                     m_pFormBackglass->StopAnimation(m_lastRandomStartedAnimation);
                     m_lastRandomStartedAnimation = "";
                  }
               }
            }
         }
      }

      // reset all current data
      m_pCollectSolenoidsData->ClearData(m_pB2SSettings->GetSolenoidsSkipFrames());
   }
}

void Server::CheckGIStrings(SAFEARRAY* psa)
{
   int giStringId;
   bool giStringBool;

   if (psa) {
      LONG uCount = 0;
      LONG lBound;
      LONG uBound;

      if (SUCCEEDED(SafeArrayGetLBound(psa, 1, &lBound))) {
         if (SUCCEEDED(SafeArrayGetUBound(psa, 1, &uBound)))
            uCount = uBound - lBound + 1;
      }

      LONG ix[2];
      VARIANT varValue;

      for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
         ix[1] = 0;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         giStringId = V_I4(&varValue) + 1;
         VariantClear(&varValue);

         ix[1] = 1;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         giStringBool = (V_I4(&varValue) > m_giStringThreshold);
         VariantClear(&varValue);

         if (m_pB2SData->IsUseRomGIStrings() || m_pB2SData->IsUseAnimationGIStrings()) {
            // collect illumination data
            if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_GIString && m_pFormBackglass->GetTopRomID() == giStringId)
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_TopImage));
            else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_GIString && m_pFormBackglass->GetSecondRomID() == giStringId)
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_SecondImage));
            if (m_pB2SData->GetUsedRomGIStringIDs()->contains(giStringId))
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_Standard));

            // collect animation data
            if (m_pB2SData->GetUsedAnimationGIStringIDs()->contains(giStringId) || m_pB2SData->GetUsedRandomAnimationGIStringIDs()->contains(giStringId))
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_Animation));
         }
      }
   }

   // one collection loop is done
   m_pCollectGIStringsData->DataAdded();

   // maybe show the collected data
   if (m_pCollectGIStringsData->ShowData()) {
      for (const auto& [key, pCollectData] : *m_pCollectGIStringsData) {
         giStringId = key;
         giStringBool = (pCollectData->GetState() > 0);
         int datatypes = pCollectData->GetTypes();

         // illumination stuff
         if ((datatypes & eCollectedDataType_TopImage) || (datatypes & eCollectedDataType_SecondImage)) {
            bool topvisible = m_lastTopVisible;
            bool secondvisible = m_lastSecondVisible;
            if (datatypes & eCollectedDataType_TopImage) {
               topvisible = giStringBool;
               if (m_pFormBackglass->IsTopRomInverted())
                  topvisible = !topvisible;
            }
            else if (datatypes & eCollectedDataType_SecondImage) {
               secondvisible = giStringBool;
               if (m_pFormBackglass->IsSecondRomInverted())
                  topvisible = !topvisible;
            }
            if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
               m_pB2SData->SetOffImageVisible(false);
               m_isVisibleStateSet = true;
               m_lastTopVisible = topvisible;
               m_lastSecondVisible = secondvisible;
               if (topvisible && secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
               else if (topvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
               else if (secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
               else {
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
                  m_pB2SData->SetOffImageVisible(true);
               }
            }
         }
         if (datatypes & eCollectedDataType_Standard) {
            for (const auto& pBase : (*m_pB2SData->GetUsedRomGIStringIDs())[giStringId]) {
               B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
               if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
                  bool visible = giStringBool;
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  if (m_pB2SData->IsUseRotatingImage() && m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && pPicbox == (*m_pB2SData->GetRotatingPictureBox())[0]) {
                     if (visible)
                        m_pFormBackglass->StartRotation();
                     else
                        m_pFormBackglass->StopRotation();
                  }
                  else
                     pPicbox->SetVisible(visible);
               }
            }
         }

         // animation stuff
         if (datatypes & eCollectedDataType_Animation) {
            if (m_pB2SData->GetUsedAnimationGIStringIDs()->contains(giStringId)) {
               for (const auto& animation : (*m_pB2SData->GetUsedAnimationGIStringIDs())[giStringId]) {
                  bool start = giStringBool;
                  if (animation->IsInverted())
                     start = !start;
                  if (start)
                     m_pFormBackglass->StartAnimation(animation->GetAnimationName());
                  else
                     m_pFormBackglass->StopAnimation(animation->GetAnimationName());
               }
            }
            // random animation start
            if (m_pB2SData->GetUsedRandomAnimationGIStringIDs()->contains(giStringId)) {
               bool start = giStringBool;
               bool isrunning = false;
               if (start) {
                  for (const auto& matchinganimation : (*m_pB2SData->GetUsedRandomAnimationGIStringIDs())[giStringId]) {
                     if (m_pFormBackglass->IsAnimationRunning(matchinganimation->GetAnimationName())) {
                        isrunning = true;
                        break;
                     }
                  }
               }
               if (start) {
                  if (!isrunning) {
                     int random = RandomStarter((*m_pB2SData->GetUsedRandomAnimationGIStringIDs())[giStringId].size());
                     auto& animation = (*m_pB2SData->GetUsedRandomAnimationGIStringIDs())[giStringId][random];
                     m_lastRandomStartedAnimation = animation->GetAnimationName();
                     m_pFormBackglass->StartAnimation(m_lastRandomStartedAnimation);
                  }
               }
               else {
                  if (!m_lastRandomStartedAnimation.empty()) {
                     m_pFormBackglass->StopAnimation(m_lastRandomStartedAnimation);
                     m_lastRandomStartedAnimation = "";
                  }
               }
            }
         }
      }

      // reset all current data
      m_pCollectGIStringsData->ClearData(m_pB2SSettings->GetGIStringsSkipFrames());
   }
}

void Server::CheckLEDs(SAFEARRAY* psa)
{
   int digit;
   int value;

   if (psa) {
      LONG uCount = 0;
      LONG lBound;
      LONG uBound;

      if (SUCCEEDED(SafeArrayGetLBound(psa, 1, &lBound))) {
         if (SUCCEEDED(SafeArrayGetUBound(psa, 1, &uBound)))
            uCount = uBound - lBound + 1;
      }

      LONG ix[2];
      VARIANT varValue;

      for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
         ix[1] = 0;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         digit = V_I4(&varValue);
         VariantClear(&varValue);

         ix[1] = 2;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         value = V_I4(&varValue);
         VariantClear(&varValue);

         // check whether leds are used
         if (m_pB2SData->IsUseLEDs() || m_pB2SData->IsUseLEDDisplays() || m_pB2SData->IsUseReels())
            m_pCollectLEDsData->Add(digit, new CollectData(value, 0));
      }
   }

   // one collection loop is done
   m_pCollectLEDsData->DataAdded();

   // maybe show the collected data
   if (m_pCollectLEDsData->ShowData()) {
      bool useLEDs = m_pB2SData->IsUseLEDs() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
      bool useLEDDisplays = m_pB2SData->IsUseLEDDisplays() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;
      bool useReels = m_pB2SData->IsUseReels();

      for (const auto& [key, pCollectData] : *m_pCollectLEDsData) {
         digit = key;
         value = pCollectData->GetState();

         if (useLEDs) {
            // rendered LEDs are used
            string ledname = "LEDBox" + std::to_string(digit + 1);
            if (m_pB2SData->GetLEDs()->contains(ledname))
               (*m_pB2SData->GetLEDs())[ledname]->SetValue(value);
         }

         if (useLEDDisplays) {
            // Dream 7 displays are used
            if (m_pB2SData->GetLEDDisplayDigits()->contains(digit)) {
               LEDDisplayDigitLocation* pLEDDisplayDigitLocation = (*m_pB2SData->GetLEDDisplayDigits())[digit];
               pLEDDisplayDigitLocation->GetLEDDisplay()->SetValue(pLEDDisplayDigitLocation->GetDigit(), value);
            }
         }

          if (useReels) {
            // reels are used
             string reelname = "ReelBox" + std::to_string(digit + 1);
            if (m_pB2SData->GetReels()->contains(reelname))
               (*m_pB2SData->GetReels())[reelname]->SetValue(value);
         }
      }

      // reset all current data
      m_pCollectLEDsData->ClearData(m_pB2SSettings->GetLEDsSkipFrames());
   }
}

void Server::MyB2SSetData(int id, int value)
{
   if (!m_pB2SData->IsValid())
      return;

   if (id == 99) {
      int i = 5;
   }

   // illumination stuff
   if ((m_pFormBackglass->GetTopRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetTopRomID() == id) || m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetSecondRomID() == id) {
      bool topvisible = m_lastTopVisible;
      bool secondvisible = m_lastSecondVisible;
      if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetTopRomID() == id) {
         topvisible = (value != 0);
         if (m_pFormBackglass->IsTopRomInverted())
            topvisible = !topvisible;
      }
      else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetSecondRomID() == id) {
         secondvisible = (value != 0);
         if (m_pFormBackglass->IsSecondRomInverted())
            topvisible = !topvisible;
      }
      if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
         m_pB2SData->SetOffImageVisible(false);
         m_isVisibleStateSet = true;
         m_lastTopVisible = topvisible;
         m_lastSecondVisible = secondvisible;
         if (topvisible && secondvisible)
            m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
         else if (topvisible)
            m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
         else if (secondvisible)
            m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
         else {
            m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
            m_pB2SData->SetOffImageVisible(true);
         }
      }
   }
   if (m_pB2SData->GetUsedRomLampIDs()->contains(id)) {
      for(const auto& pBase : (*m_pB2SData->GetUsedRomLampIDs())[id]) {
         B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
         if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
            if (pPicbox->GetRomIDValue() > 0)
               pPicbox->SetVisible(pPicbox->GetRomIDValue() == value);
            else {
               bool visible = (value != 0);
               if (pPicbox->IsRomInverted())
                  visible = !visible;
               auto rotatingPictureBox = m_pB2SData->GetRotatingPictureBox();
               if (m_pB2SData->IsUseRotatingImage() && rotatingPictureBox && (*rotatingPictureBox)[0] && pPicbox == (*rotatingPictureBox)[0]) {
                  if (visible)
                     m_pFormBackglass->StartRotation();
                  else
                     m_pFormBackglass->StopRotation();
               }
               else
                  pPicbox->SetVisible(visible);
            }
         }
      }
   }
   if (m_pB2SData->GetUsedRomReelLampIDs()->contains(id)) {
      for(const auto& pReelbox : (*m_pB2SData->GetUsedRomReelLampIDs())[id]) {
         if (!m_pB2SData->IsUseIlluminationLocks() || pReelbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pReelbox->GetGroupName())) {
            if (pReelbox->GetRomIDValue() > 0)
               pReelbox->SetIlluminated(pReelbox->GetRomIDValue() == value);
            else {
               bool illuminated = (value != 0);
               if (pReelbox->IsRomInverted())
                  illuminated = !illuminated;
               pReelbox->SetIlluminated(illuminated);
            }
         }
      }
   }

   // animation stuff
   if (m_pB2SData->GetUsedAnimationLampIDs()->contains(id)) {
      for (const auto& animation : (*m_pB2SData->GetUsedAnimationLampIDs())[id]) {
         bool start = (value != 0);
         if (animation->IsInverted())
            start = !start;
         if (start)
            m_pFormBackglass->StartAnimation(animation->GetAnimationName());
         else
            m_pFormBackglass->StopAnimation(animation->GetAnimationName());
      }
   }

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('E', id, value);
}

void Server::MyB2SSetData(const string& groupname, int value)
{
   if (!m_pB2SData->IsValid())
      return;

   // only do the lightning stuff if the group has a name
   if (!groupname.empty() && m_pB2SData->GetIlluminationGroups()->contains(groupname)) {
      // get all matching picture boxes
      for(const auto& pPicbox : (*m_pB2SData->GetIlluminationGroups())[groupname]) {
         if (pPicbox->GetPictureBoxType() == ePictureBoxType_StandardImage) {
            if (pPicbox->GetRomIDValue() > 0)
               pPicbox->SetVisible(pPicbox->GetRomIDValue() == value);
            else
               pPicbox->SetVisible(value != 0);
         }
      }
   }
}

void Server::MyB2SSetLED(int digit, int value)
{
   if (!m_pB2SData->IsValid())
      return;

   bool useLEDs = m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit)).c_str()) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
   bool useLEDDisplays = m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;

   if (useLEDs) {
      // rendered LEDs are used
      string ledname = "LEDBox" + std::to_string(digit);
      if (m_pB2SData->GetLEDs()->contains(ledname))
         (*m_pB2SData->GetLEDs())[ledname]->SetValue(value);
   }
   else if (useLEDDisplays) {
      // Dream 7 displays are used
      if (m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1)) {
         LEDDisplayDigitLocation* pLEDDisplayDigitLocation = (*m_pB2SData->GetLEDDisplayDigits())[digit - 1];
         pLEDDisplayDigitLocation->GetLEDDisplay()->SetValue(pLEDDisplayDigitLocation->GetDigit(), value);
      }
   }
}

void Server::MyB2SSetLED(int digit, const string& value)
{
   if (!m_pB2SData->IsValid())
      return;

   bool useLEDs = m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit)).c_str()) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
   bool useLEDDisplays = m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;

   if (useLEDs) {
      // rendered LEDs do not support string values
   }
   else if (useLEDDisplays) {
      // Dream 7 displays are used
      if (m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1)) {
         auto& pLEDDisplayDigit = (*m_pB2SData->GetLEDDisplayDigits())[digit - 1];
         pLEDDisplayDigit->GetLEDDisplay()->SetValue(pLEDDisplayDigit->GetDigit(), value);
      }
   }
}

void Server::MyB2SSetLEDDisplay(int display, const string& szText)
{
   if (!m_pB2SData->IsValid())
      return;

   int digit = GetFirstDigitOfDisplay(display);

   bool useLEDs = m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit)).c_str()) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
   bool useLEDDisplays = m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;

   if (useLEDs) {
      // TODO
   }
   else {
      if (m_pB2SData->GetLEDDisplayDigits()->contains(digit)) {
         auto& pLEDDisplayDigit = (*m_pB2SData->GetLEDDisplayDigits())[digit];
         pLEDDisplayDigit->GetLEDDisplay()->SetText(szText);
      }
   }
}

int Server::GetFirstDigitOfDisplay(int display)
{
    int ret = 0;
    for(const auto& [key, pReelbox] : *m_pB2SData->GetReels()) {
       if (pReelbox->GetDisplayID() == display) {
          ret = pReelbox->GetStartDigit();
          break;
       }
    }
    if (ret == 0) {
       for(const auto& [key, pLedbox] : *m_pB2SData->GetLEDs()) {
          if (pLedbox->GetDisplayID() == display) {
             ret = pLedbox->GetStartDigit();
             break;
          }
       }
    }
    return ret;
}

void Server::MyB2SSetScore(int digit, int value, bool animateReelChange, bool useLEDs, bool useLEDDisplays, bool useReels, int reeltype, eLEDType ledtype)
{
   if (!m_pB2SData->IsValid())
      return;

   if (digit > 0) {
      useLEDs = (m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit)).c_str()) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
      useLEDDisplays = (m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
      useReels = m_pB2SData->GetReels()->contains(string("ReelBox" + std::to_string(digit)).c_str());

      if (useLEDs) {
         // rendered LEDs are used
         string ledname = "LEDBox" + std::to_string(digit);
         (*m_pB2SData->GetLEDs())[ledname]->SetText(std::to_string(value));
      }
      else if (useLEDDisplays) {
         // Dream 7 displays are used
         LEDDisplayDigitLocation* pLEDDisplayDigitLocation = (*m_pB2SData->GetLEDDisplayDigits())[digit - 1];
         pLEDDisplayDigitLocation->GetLEDDisplay()->SetValue(pLEDDisplayDigitLocation->GetDigit(), std::to_string(value));
      }
      else if (useReels) {
         // reels are used
         string reelname = "ReelBox" + std::to_string(digit);
         (*m_pB2SData->GetReels())[reelname]->SetText(value, animateReelChange);
      }
   }

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('B', digit, value);
}

void Server::MyB2SSetScore(int digit, int score)
{
   if (!m_pB2SData->IsValid())
      return;

   if (digit > 0) {
      bool useLEDs = (m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit)).c_str()) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
      bool useLEDDisplays = (m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
      bool useReels = m_pB2SData->GetReels()->contains(string("ReelBox" + std::to_string(digit)).c_str());

      if (useLEDs) {
         // check the passed digit
         string led = "LEDBox" + std::to_string(digit);

         // get all necessary display data
         int startdigit = (*m_pB2SData->GetLEDs())[led]->GetStartDigit();
         int player = (*m_pB2SData->GetLEDs())[led]->GetID();
         int digits = (*m_pB2SData->GetLEDs())[led]->GetDigits();
         string scoreAsString = string(digits - std::to_string(score).length(), ' ') + std::to_string(score);

         // set digits
         for (int i = startdigit + digits - 1; i >= startdigit; i--)
            (*m_pB2SData->GetLEDs())["LEDBox" + std::to_string(i)]->SetText(scoreAsString.substr(i - startdigit, 1));
      }
      else if (useLEDDisplays) {
         LEDDisplayDigitLocation* pLEDDisplayDigitLocation = (*m_pB2SData->GetLEDDisplayDigits())[digit - 1];
         // get all necessary display data
         int digits = pLEDDisplayDigitLocation->GetLEDDisplay()->GetDigits();
         string scoreAsString = string(digits - std::to_string(score).length(), ' ') + std::to_string(score);

         // set digits
         for (int i = digits - 1; i >= 0; i--)
            pLEDDisplayDigitLocation->GetLEDDisplay()->SetValue(i, scoreAsString.substr(i, 1));
      }
      else if (useReels) {
         // get the necessary infos
         string reelname = "ReelBox" + std::to_string(digit);
         int id = (*m_pB2SData->GetReels())[reelname]->GetDisplayID();

         // set value
         if (m_pB2SData->GetReelDisplays()->contains(id))
            (*m_pB2SData->GetReelDisplays())[id]->SetScore_(score);
      }
   }

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('B', digit, score);
}

void Server::MyB2SSetScorePlayer(int playerno, int score)
{
   if (!m_pB2SData->IsValid())
      return;

   if (playerno > 0) {
      if (m_pB2SData->GetPlayers()->contains(playerno))
         (*m_pB2SData->GetPlayers())[playerno]->SetScore(score);
   }

   if (m_pB2SSettings->ArePluginsOn())
      m_pB2SSettings->GetPluginHost()->DataReceive('C', playerno, score);
}

void Server::MyB2SStartAnimation(const string& animationname, bool playreverse)
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->StartAnimation(animationname, playreverse);
}

void Server::MyB2SStopAnimation(const string& animationname)
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->StopAnimation(animationname);
}

void Server::MyB2SStopAllAnimations()
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->StopAllAnimations();
}

bool Server::MyB2SIsAnimationRunning(const string& animationname)
{
   if (!m_pB2SData->IsValid())
      return false;

   return m_pFormBackglass->IsAnimationRunning(animationname);
}

void Server::MyB2SStartRotation()
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->StartRotation();
}

void Server::MyB2SStopRotation()
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->StopRotation();
}

void Server::MyB2SShowOrHideScoreDisplays(bool visible)
{
   if (!m_pB2SData->IsValid())
      return;

   if (visible)
      m_pFormBackglass->ShowScoreDisplays();
   else
      m_pFormBackglass->HideScoreDisplays();
}

void Server::MyB2SPlaySound(const string& soundname)
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->PlaySound(soundname);
}

void Server::MyB2SStopSound(const string& soundname)
{
   if (!m_pB2SData->IsValid())
      return;

   m_pFormBackglass->StopSound(soundname);
}

void Server::Startup()
{
   m_pB2SData->SetTableFileName(g_pvp->GetActiveTable()->m_szFileName);
}

void Server::ShowBackglassForm()
{
   if (!m_pFormBackglass)
      m_pFormBackglass = new FormBackglass();

   m_pFormBackglass->Show();
   m_pFormBackglass->SetTopMost(true);
   m_pFormBackglass->BringToFront();
   m_pFormBackglass->SetTopMost(false);
   m_pB2SData->SetBackglassVisible(true);
}

void Server::HideBackglassForm()
{
   if (m_pFormBackglass)
      m_pFormBackglass->Hide();
}

void Server::KillBackglassForm()
{
}

int Server::RandomStarter(int top)
{
   static int lastone = -1;
   if (top <= 1)
      return 0;
   int ret;
   do {
      ret = rand() % top;
   } while (ret == lastone);
   lastone = ret;
   return ret;
}