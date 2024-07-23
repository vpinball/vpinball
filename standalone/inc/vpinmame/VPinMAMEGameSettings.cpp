#include "stdafx.h"
#include "VPinMAMEGameSettings.h"
#include "VPinMAMEGame.h"
#include "mINI/ini.h"

VPinMAMEGameSettings::VPinMAMEGameSettings(VPinMAMEController* pController, IGame* pGame)
{
   m_pController = pController;

   m_pGame = pGame;
   m_pGame->AddRef();
}

VPinMAMEGameSettings::~VPinMAMEGameSettings()
{
   if (m_pGame)
      m_pGame->Release();
}

STDMETHODIMP VPinMAMEGameSettings::ShowSettingsDlg(LONG_PTR hParentWnd)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMEGameSettings::Clear()
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMEGameSettings::get_Value(BSTR sName, VARIANT *pVal)
{
   mINI::INIStructure ini;
   mINI::INIFile file(m_pController->GetIniPath() + ((VPinMAMEGame*)m_pGame)->GetName() + ".ini");
   file.read(ini);

   string szName = MakeString(sName);
   if (ini.has("settings") && ini["settings"].has(szName)) {
      const WCHAR * const wz = MakeWide(ini["settings"][szName]);
      SetVarBstr(pVal, SysAllocString(wz));
      delete[] wz;
   }
   else
      SetVarBstr(pVal, SysAllocString(L""));

   string szValue = MakeString(V_BSTR(pVal));

   PLOGD.printf("name=%s, value=%s", szName.c_str(), szValue.c_str());

   return S_OK;
}

STDMETHODIMP VPinMAMEGameSettings::put_Value(BSTR sName, VARIANT newVal)
{
   mINI::INIStructure ini;
   mINI::INIFile file(m_pController->GetIniPath() + ((VPinMAMEGame*)m_pGame)->GetName() + ".ini");
   file.read(ini);

   string szName = MakeString(sName);
   BSTR bstr = BstrFromVariant(&newVal, 0x409);
   string szValue = MakeString(bstr);
   ini["settings"][szName] = MakeString(bstr);
   SysFreeString(bstr);

   file.write(ini);

   PLOGD.printf("name=%s, value=%s", szName.c_str(), szValue.c_str());

   return S_OK;
}

STDMETHODIMP VPinMAMEGameSettings::SetDisplayPosition(VARIANT newValX, VARIANT newValY, LONG_PTR hWnd)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}
