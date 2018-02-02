#include "StdAfx.h"
#include "vpversion.h"
#include "resource.h"
#include "hash.h"
#include <algorithm>
#include <atlsafe.h>
#include "objloader.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <fstream>
#include <sstream>

using namespace rapidxml;

#define HASHLENGTH 16

const unsigned char TABLE_KEY[] = "Visual Pinball";
const unsigned char PARAPHRASE_KEY[] = { 0xB4, 0x0B, 0xBE, 0x37, 0xC3, 0x0C, 0x8E, 0xA1, 0x5A, 0x05, 0xDF, 0x1B, 0x2D, 0x02, 0xEF, 0x8D };

INT_PTR CALLBACK ProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define TIMER_ID_AUTOSAVE 12345
#define TIMER_ID_CLOSE_TABLE 12346


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void ScriptGlobalTable::Init(PinTable *pt)
{
   m_pt = pt;
}

STDMETHODIMP ScriptGlobalTable::BeginModal()
{
   if (g_pplayer)
      g_pplayer->m_ModalRefCount++;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndModal()
{
   if (g_pplayer)
   {
      if (g_pplayer->m_ModalRefCount > 0)
         g_pplayer->m_ModalRefCount--;
      g_pplayer->m_LastKnownGoodCounter++;
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::Nudge(float Angle, float Force)
{
   if (g_pplayer && (!g_pplayer->m_legacyNudge || g_pplayer->m_legacyNudgeTime == 0))
   {
      const float sn = sinf(ANGTORAD(Angle));
      const float cs = cosf(ANGTORAD(Angle));

      if(g_pplayer->m_legacyNudge)
      {
          g_pplayer->m_legacyNudgeBackX =  sn * Force;
          g_pplayer->m_legacyNudgeBackY = -cs * Force;
          g_pplayer->m_legacyNudgeTime = 100;
      }
      else
      {
          g_pplayer->m_tableVel.x +=  sn * Force;
          g_pplayer->m_tableVel.y += -cs * Force;
      }
   }

   return S_OK;
}


STDMETHODIMP ScriptGlobalTable::NudgeGetCalibration(VARIANT *XMax, VARIANT *YMax, VARIANT *XGain, VARIANT *YGain, VARIANT *DeadZone, VARIANT *TiltSensitivty)
{
	int tmp;

	if (SUCCEEDED(GetRegInt("Player", "PBWAccelGainX", &tmp)))
		CComVariant(tmp).Detach(XGain);
	if (SUCCEEDED(GetRegInt("Player", "PBWAccelGainY", &tmp)))
		CComVariant(tmp).Detach(YGain);
	if (SUCCEEDED(GetRegInt("Player", "PBWAccelMaxX", &tmp)))
		CComVariant(tmp).Detach(XMax);
	if (SUCCEEDED(GetRegInt("Player", "PBWAccelMaxY", &tmp)))
		CComVariant(tmp).Detach(YMax);
	if (SUCCEEDED(GetRegInt("player", "DeadZone", &tmp)))
		CComVariant(tmp).Detach(DeadZone);
	if (SUCCEEDED(GetRegInt("Player", "TiltSensitivity", &tmp)))
		CComVariant(tmp).Detach(TiltSensitivty);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::NudgeSetCalibration(int XMax, int YMax, int XGain, int YGain, int DeadZone, int TiltSensitivity)
{
	int newvalue;

	newvalue = XGain;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	SetRegValue("Player", "PBWAccelGainX", REG_DWORD, &newvalue, 4);

	newvalue = YGain;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	SetRegValue("Player", "PBWAccelGainY", REG_DWORD, &newvalue, 4);

	newvalue = DeadZone;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	if (newvalue > 100) { newvalue = 100; }
	SetRegValue("Player", "DeadZone", REG_DWORD, &newvalue, 4);

	newvalue = XMax;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	if (newvalue > 100) { newvalue = 100; }
	SetRegValue("Player", "PBWAccelMaxX", REG_DWORD, &newvalue, 4);

	newvalue = YMax;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	if (newvalue > 100) { newvalue = 100; }
	SetRegValue("Player", "PBWAccelMaxY", REG_DWORD, &newvalue, 4);

	if (TiltSensitivity > 0)
	{
		newvalue = TiltSensitivity;
		SetRegValue("Player", "TiltSensValue", REG_DWORD, &newvalue, 4);
		SetRegValue("Player", "TiltSensitivity", REG_DWORD, &newvalue, 4);
		newvalue = 1;
		SetRegValue("Player", "TiltSensCB", REG_DWORD, &newvalue, 4);
	}
	else
	{
		newvalue = 0;
		SetRegValue("Player", "TiltSensCB", REG_DWORD, &newvalue, 4);
		HKEY hkey;
		RegOpenKey(HKEY_CURRENT_USER, "Software\\Visual Pinball\\Player", &hkey);
		RegDeleteValue(hkey, "TiltSensitivity");
		RegCloseKey(hkey);
	}
	
	m_pt->ReadAccelerometerCalibration();

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::NudgeSensorStatus(VARIANT *XNudge, VARIANT *YNudge)
{
	CComVariant(m_pt->m_tblNudgeReadX).Detach(XNudge);
	m_pt->m_tblNudgeReadX = 0.0f;
	CComVariant(m_pt->m_tblNudgeReadY).Detach(YNudge);
	m_pt->m_tblNudgeReadY = 0.0f;

	return S_OK;
}
STDMETHODIMP ScriptGlobalTable::NudgeTiltStatus(VARIANT *XPlumb, VARIANT *YPlumb, VARIANT *Tilt)
{
	CComVariant(m_pt->m_tblNudgePlumbX).Detach(XPlumb);
	m_pt->m_tblNudgePlumbX = 0.0f;
	CComVariant(m_pt->m_tblNudgePlumbY).Detach(YPlumb);
	m_pt->m_tblNudgePlumbY = 0.0f;
	CComVariant(m_pt->m_tblNudgeReadTilt).Detach(Tilt);
	m_pt->m_tblNudgeReadTilt = 0.0f;

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlaySound(BSTR bstr, long LoopCount, float volume, float pan, float randompitch, long pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade)
{
   if (g_pplayer && g_pplayer->m_fPlaySound) m_pt->PlaySound(bstr, LoopCount, volume, pan, randompitch, pitch, usesame, restart, front_rear_fade);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::FireKnocker(int Count)
{
   if (g_pplayer) m_pt->FireKnocker(Count);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::QuitPlayer(int CloseType)
{
   if (g_pplayer) m_pt->QuitPlayer(CloseType);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::StopSound(BSTR Sound)
{
   if (g_pplayer && g_pplayer->m_fPlaySound)
      m_pt->StopSound(Sound);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlayMusic(BSTR str, float volume)
{
   if (g_pplayer && g_pplayer->m_fPlayMusic)
   {
      if (g_pplayer->m_pxap)
         EndMusic();

      char szT[512];
      char szPath[MAX_PATH + 512];

      WideCharToMultiByte(CP_ACP, 0, g_pvp->m_wzMyPath, -1, szPath, MAX_PATH, NULL, NULL);
      WideCharToMultiByte(CP_ACP, 0, str, -1, szT, 512, NULL, NULL);

      char szextension[MAX_PATH];
      ExtensionFromFilename(szT, szextension);

      //ppi->m_ppb;// = new PinBinary();

      lstrcat(szPath, "Music\\");

      //WideCharToMultiByte(CP_ACP, 0, str, -1, szT, 512, NULL, NULL);

      // We know that szT can't be more than 512 characters as this point, and that szPath can't be more than MAX_PATH
      lstrcat(szPath, szT);

      g_pplayer->m_pxap = new XAudPlayer();

      const float MusicVolumef = max(min((float)g_pplayer->m_MusicVolume*m_pt->m_TableMusicVolume*volume, 100.0f), 0.0f);
#ifdef NO_XAUDIO
      const float MusicVolume = MusicVolumef;
#else
      const int MusicVolume = (MusicVolumef == 0.0f) ? DSBVOLUME_MIN : (int)(logf(MusicVolumef)*(float)(1000.0 / log(10.0)) - 2000.0f); // 10 volume = -10Db
#endif

      if (!g_pplayer->m_pxap->Init(szPath, MusicVolume))
      {
         delete g_pplayer->m_pxap;
         g_pplayer->m_pxap = NULL;
      }
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndMusic()
{
   if (g_pplayer && g_pplayer->m_fPlayMusic)
   {
      if (g_pplayer->m_pxap)
      {
         g_pplayer->m_pxap->End();
         delete g_pplayer->m_pxap;
         g_pplayer->m_pxap = NULL;
      }
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_MusicVolume(float volume)
{
	if (g_pplayer && g_pplayer->m_fPlayMusic)
	{
		const float MusicVolumef = max(min((float)g_pplayer->m_MusicVolume*m_pt->m_TableMusicVolume*volume, 100.0f), 0.0f);
#ifdef NO_XAUDIO
		const float MusicVolume = MusicVolumef;
#else
		const int MusicVolume = (MusicVolumef == 0.0f) ? DSBVOLUME_MIN : (int)(logf(MusicVolumef)*(float)(1000.0 / log(10.0)) - 2000.0f); // 10 volume = -10Db
#endif
		g_pplayer->m_pxap->Volume(MusicVolume);
	}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Global");

   return S_OK;
}

IDispatch *ScriptGlobalTable::GetDispatch()
{
   return (IDispatch *)this;
}

STDMETHODIMP ScriptGlobalTable::get_LeftFlipperKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLeftFlipperKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightFlipperKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eRightFlipperKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftTiltKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLeftTiltKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightTiltKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eRightTiltKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_CenterTiltKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eCenterTiltKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlungerKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[ePlungerKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StartGameKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eStartGameKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eAddCreditKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey2(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eAddCreditKey2];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_MechanicalTilt(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eMechanicalTilt];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftMagnaSave(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLeftMagnaSave];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightMagnaSave(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eRightMagnaSave];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ExitGame(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eExitGame];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LockbarKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLockbarKey];

   return S_OK;
}

BOOL ScriptGlobalTable::GetTextFileFromDirectory(char *szfilename, char *dirname, BSTR *pContents)
{
   char *szPath;
   szPath = new char[MAX_PATH + lstrlen(szfilename)];
   BOOL fSuccess = fFalse;

   if (dirname != NULL)
   {
      lstrcpy(szPath, g_pvp->m_szMyPath);
      lstrcat(szPath, dirname);
   }
   else
   {
      // Current directory
      szPath[0] = '\0';
   }
   lstrcat(szPath, szfilename);

   int len;
   BYTE *szContents;

   if (RawReadFromFile(szPath, &len, (char **)&szContents))
   {
      BYTE *szDataStart = szContents;
      int encoding = CP_ACP;

      if (szContents[0] == 0xEF && szContents[1] == 0xBB && szContents[2] == 0xBF)
      {
         encoding = CP_UTF8;
         szDataStart += 3;
         len -= 3;
      }

      if (szContents[0] == 0xFF && szContents[1] == 0xFE)
      {
         // Unicode
         szDataStart += 2;
         len -= 2;
         *pContents = SysAllocString((WCHAR *)szDataStart);
      }
      else
      {
         WCHAR *wzContents = new WCHAR[len + 1];

         MultiByteToWideChar(encoding, 0, (char *)szDataStart, len, wzContents, len + 1);
         wzContents[len] = L'\0';

         *pContents = SysAllocString(wzContents);
         delete[] wzContents;
      }

      delete[] szContents;

      fSuccess = fTrue;
   }

   delete[] szPath;

   return fSuccess;
}

STDMETHODIMP ScriptGlobalTable::GetTextFile(BSTR FileName, BSTR *pContents)
{
   BOOL fSuccess;
   char szFileName[MAX_PATH];

   WideCharToMultiByte(CP_ACP, 0, FileName, -1, szFileName, MAX_PATH, NULL, NULL);

   // try to load the scripts from the current directory
   fSuccess = GetTextFileFromDirectory(szFileName, NULL, pContents);

   // if that fails, try the User, Scripts and Tables sub-directorys under where VP was loaded from
   if (!fSuccess)
      fSuccess = GetTextFileFromDirectory(szFileName, "User\\", pContents);

   if (!fSuccess)
      fSuccess = GetTextFileFromDirectory(szFileName, "Scripts\\", pContents);

   if (!fSuccess)
      fSuccess = GetTextFileFromDirectory(szFileName, "Tables\\", pContents);

   return (fSuccess) ? S_OK : E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::get_UserDirectory(BSTR *pVal)
{
   WCHAR wzPath[MAX_PATH];
   WideStrNCopy(g_pvp->m_wzMyPath, wzPath, MAX_PATH);
   WideStrCat(L"User\\", wzPath);
   *pVal = SysAllocString(wzPath);

   return S_OK;
}

#ifdef _WIN64
STDMETHODIMP ScriptGlobalTable::get_GetPlayerHWnd(size_t *pVal)
#else
STDMETHODIMP ScriptGlobalTable::get_GetPlayerHWnd(long *pVal)
#endif
{
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   else
   {
      *pVal = (size_t)g_pplayer->m_hwnd;
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::AddObject(BSTR Name, IDispatch *pdisp)
{
   if (!g_pplayer)
      return E_FAIL;

   g_pplayer->m_ptable->m_pcv->AddTemporaryItem(Name, pdisp);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value)
{
   IStorage* pstgRoot;
   IStorage* pstgTable;
   IStream* pstmValue;

   HRESULT hr = S_OK;

   WCHAR wzPath[MAX_PATH];
   WideStrNCopy(g_pvp->m_wzMyPath, wzPath, MAX_PATH);
   WideStrCat(L"User\\VPReg.stg", wzPath);

   if (FAILED(hr = StgOpenStorage(wzPath, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
   {
      // Registry file does not exist - create it
      if (FAILED(hr = StgCreateDocfile(wzPath, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
      {
         WCHAR wzMkPath[MAX_PATH];
         WideStrCopy(g_pvp->m_wzMyPath, wzMkPath);
         WideStrCat(L"User", wzMkPath);
         if (_wmkdir(wzMkPath) != 0)
            return hr;

         if (FAILED(hr = StgCreateDocfile(wzPath, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
            return hr;
      }
   }

   if (FAILED(hr = pstgRoot->OpenStorage(TableName, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgTable)))
   {
      // Table file does not exist
      if (FAILED(hr = pstgRoot->CreateStorage(TableName, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgTable)))
      {
         pstgRoot->Release();
         return hr;
      }
   }

   if (FAILED(hr = pstgTable->CreateStream(ValueName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmValue)))
   {
      pstgTable->Release();
      pstgRoot->Release();
      return hr;
   }

   BSTR bstr = BstrFromVariant(&Value, 0x409);

   DWORD writ;

   pstmValue->Write((WCHAR *)bstr, lstrlenW((WCHAR *)bstr) * (int)sizeof(WCHAR), &writ);

   SysFreeString(bstr);

   pstmValue->Release();

   pstgTable->Commit(STGC_DEFAULT);
   pstgTable->Release();

   pstgRoot->Commit(STGC_DEFAULT);
   pstgRoot->Release();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::LoadValue(BSTR TableName, BSTR ValueName, VARIANT *Value)
{
   IStorage* pstgRoot;
   IStorage* pstgTable;
   IStream* pstmValue;

   HRESULT hr = S_OK;

   WCHAR wzPath[MAX_PATH];
   WideStrNCopy(g_pvp->m_wzMyPath, wzPath, MAX_PATH);
   WideStrCat(L"User\\VPReg.stg", wzPath);

   if (FAILED(hr = StgOpenStorage(wzPath, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      return S_OK;
   }

   if (FAILED(hr = pstgRoot->OpenStorage(TableName, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgTable)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      pstgRoot->Release();
      return S_OK;
   }

   if (FAILED(hr = pstgTable->OpenStream(ValueName, 0, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmValue)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      pstgTable->Release();
      pstgRoot->Release();
      return S_OK;
   }

   STATSTG statstg;

   pstmValue->Stat(&statstg, STATFLAG_NONAME);

   WCHAR *wzT;

   int size = statstg.cbSize.LowPart / sizeof(WCHAR);

   wzT = new WCHAR[size + 1];

   DWORD read;

   hr = pstmValue->Read(wzT, size * (int)sizeof(WCHAR), &read);
   wzT[size] = L'\0';

   pstmValue->Release();

   pstgTable->Commit(STGC_DEFAULT);
   pstgTable->Release();

   pstgRoot->Commit(STGC_DEFAULT);
   pstgRoot->Release();

   SetVarBstr(Value, SysAllocString(wzT));

   delete[] wzT;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ActiveBall(IBall **pVal)
{
   if (!pVal || !g_pplayer || !g_pplayer->m_pactiveball)
      return E_POINTER;

   BallEx *pballex = g_pplayer->m_pactiveball->m_pballex;

   if (!pballex)
      return E_POINTER;

   pballex->QueryInterface(IID_IBall, (void **)pVal);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_GameTime(long *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = g_pplayer->m_time_msec;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_SystemTime(long *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = msec();

   return S_OK;
}

/*STDMETHODIMP ScriptGlobalTable::put_NightDay(int pVal)
{
if(g_pplayer)
g_pplayer->m_globalEmissionScale = dequantizeUnsignedPercent(newVal);
return S_OK;
}*/

STDMETHODIMP ScriptGlobalTable::get_NightDay(int *pVal)
{
   if (g_pplayer)
      *pVal = quantizeUnsignedPercent(g_pplayer->m_globalEmissionScale);
   return S_OK;
}

/*STDMETHODIMP ScriptGlobalTable::put_ShowDT(int pVal)
{
   if(g_pplayer)
      g_pplayer->m_ptable->m_BG_current_set = (!!newVal) ? 0 : 1;
   return S_OK;
}*/

STDMETHODIMP ScriptGlobalTable::get_ShowDT(VARIANT_BOOL *pVal)
{
   if (g_pplayer)
      *pVal = (VARIANT_BOOL)FTOVB(g_pplayer->m_ptable->m_BG_current_set == BG_DESKTOP || g_pplayer->m_ptable->m_BG_current_set == BG_FSS); // DT & FSS
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ShowFSS(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(g_pplayer->m_ptable->m_BG_enable_FSS);

   //*pVal = (VARIANT_BOOL)FTOVB(g_pplayer->m_ptable->m_BG_current_set == 2);

   return S_OK;
}

/*STDMETHODIMP PinTable::put_ShowFSS(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_BG_enable_FSS = !!newVal;

   if(m_BG_enable_FSS)
      m_BG_current_set = FULL_SINGLE_SCREEN;
   else
      GetRegInt("Player", "BGSet", (int*)&m_BG_current_set);

   STOPUNDO

   SetDirtyDraw();

   return S_OK;
}*/

STDMETHODIMP ScriptGlobalTable::MaterialColor(BSTR pVal, OLE_COLOR newVal)
{
	if (!g_pplayer)
		return E_POINTER;

	PinTable *pt = g_pplayer->m_ptable;

	char Name[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pVal, -1, Name, MAX_PATH, NULL, NULL);

	Material *tmp = pt->GetMaterial(Name);
	tmp->m_cBase = newVal;

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_WindowWidth(int *pVal)
{
   if (g_pplayer)
      *pVal = g_pplayer->m_width;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_WindowHeight(int *pVal)
{
   if (g_pplayer)
      *pVal = g_pplayer->m_height;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDWidth(int pVal)
{
   if (g_pplayer)
      g_pplayer->m_dmdx = pVal;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDHeight(int pVal)
{
   if (g_pplayer)
      g_pplayer->m_dmdy = pVal;
   return S_OK;
}

static inline float eq_col1(const DWORD AD, const DWORD BD)
{
    const float A[3] = { (float)(AD & 255) * (float)(1.0/255.),(float)(AD & 65280) * (float)(1.0 / 65280.0),(float)(AD & 16711680) * (float)(1.0 / 16711680.0) };
    const float B[3] = { (float)(BD & 255) * (float)(1.0/255.),(float)(BD & 65280) * (float)(1.0 / 65280.0),(float)(BD & 16711680) * (float)(1.0 / 16711680.0) };

    const float r = 0.5f * (A[0] + B[0]);
    const float d[3] = { A[0] - B[0], A[1] - B[1], A[2] - B[2] };
    const float tmp = (2.f + r) * d[0] * d[0] + 4.f * d[1] * d[1] + (3.f - r) * d[2] * d[2]; // dot(c*d, d)
    return 1.f - sqrtf(tmp) * (float)(1.0/3.);
}

#if 0
static inline float eq_col2(const DWORD e1, const DWORD e2) //!! test vs above, should be the same?
{
    const long rmean = ((long)(e1 & 255) + (long)(e2 & 255)) / 2;
    const long r = (long)(e1 & 255) - (long)(e2 & 255);
    const long g = (long)((e1 >> 8) & 255) - (long)((e2 >> 8) & 255);
    const long b = (long)((e1 >> 16) & 255) - (long)((e2 >> 16) & 255);
    return 1.f - sqrtf((float)((((512 + rmean)*r*r) >> 8) + 4 * g*g + (((767 - rmean)*b*b) >> 8)));
}
#endif

static inline float eq_brightness(const DWORD AD, const DWORD BD)
{
    return 1.f - (float)abs((int)(AD - BD)) * (float)(1.0 / 100.);
}

static const double SFX_CLR = 0.35;
static const float SFX_THR = (float)(1.0 - SFX_CLR);

// corner strength
static inline Vertex4D str(const Vertex4D &crn, const Vertex4D &ort) {
    return Vertex4D((crn.x > SFX_THR) ? max(2.f * crn.x - (ort.x + ort.w), 0.f) : 0.f,
                    (crn.y > SFX_THR) ? max(2.f * crn.y - (ort.y + ort.x), 0.f) : 0.f,
                    (crn.z > SFX_THR) ? max(2.f * crn.z - (ort.z + ort.y), 0.f) : 0.f,
                    (crn.w > SFX_THR) ? max(2.f * crn.w - (ort.w + ort.z), 0.f) : 0.f);
}

// corner dominance at junctions
static inline Vertex4D dom(const Vertex4D &strx, const Vertex4D &stry, const Vertex4D &strz, const Vertex4D &strw) {
    return Vertex4D(max(2.f * strx.z - (strx.y + strx.w), 0.f),
                    max(2.f * stry.w - (stry.z + stry.x), 0.f),
                    max(2.f * strz.x - (strz.w + strz.y), 0.f),
                    max(2.f * strw.y - (strw.x + strw.z), 0.f));
}

// necessary but not sufficient junction condition for orthogonal edges
static inline bool clear(const Vertex2D &crn, const Vertex4D &ort) {
    return (crn.x <= SFX_THR || crn.x <= ort.x || crn.x <= ort.w) &&
           (crn.y <= SFX_THR || crn.y <= ort.y || crn.y <= ort.x) &&
           (crn.x <= SFX_THR || crn.x <= ort.z || crn.x <= ort.y) &&
           (crn.y <= SFX_THR || crn.y <= ort.w || crn.y <= ort.z);
}

static inline bool4 ambi_dom(const Vertex4D &jDx)
{
    return bool4(
        jDx.x != 0.f && jDx.x + jDx.z > jDx.y + jDx.w,
        jDx.y != 0.f && jDx.y + jDx.w > jDx.z + jDx.x,
        jDx.z != 0.f && jDx.z + jDx.x > jDx.w + jDx.y,
        jDx.w != 0.f && jDx.w + jDx.y > jDx.x + jDx.z);
}

void upscale(DWORD * const data, const unsigned int xres, const unsigned int yres, const bool is_brightness_data)
{
    std::vector<Vertex4D> metric(xres*yres); //!! avoid constant reallocs?
    
    unsigned int o = 0;
    if (is_brightness_data)
    {
        for (unsigned int j = 0; j < yres; ++j)
        {
            const unsigned int jm1 = (j - 1)*xres;

            for (unsigned int i = 0; i < xres; ++i, ++o)
            {
                const unsigned int ip1 = i + 1;
                const DWORD E = data[j*xres + i];
                const DWORD E2 = (E & 0xFEFEFE) >> 1; // borders = half black/half border pixel
                const DWORD A = (j == 0) || (i == 0) ? E2 : data[jm1 + i - 1];
                const DWORD B = (j == 0) ? E2 : data[jm1 + i];
                const DWORD C = (j == 0) || (i == xres - 1) ? E2 : data[jm1 + ip1];
                const DWORD F = (i == xres - 1) ? E2 : data[j*xres + ip1];

                metric[o] = Vertex4D(eq_brightness(E, A), eq_brightness(E, B), eq_brightness(E, C), eq_brightness(E, F));
            }
        }
    }
    else
    {
        for (unsigned int j = 0; j < yres; ++j)
        {
            const unsigned int jm1 = (j - 1)*xres;

            for (unsigned int i = 0; i < xres; ++i, ++o)
            {
                const unsigned int ip1 = i + 1;
                const DWORD E = data[j*xres + i];
                const DWORD E2 = (E & 0xFEFEFEFE) >> 1; // borders = half black/half border pixel
                const DWORD A = (j == 0) || (i == 0) ? E2 : data[jm1 + i - 1];
                const DWORD B = (j == 0) ? E2 : data[jm1 + i];
                const DWORD C = (j == 0) || (i == xres - 1) ? E2 : data[jm1 + ip1];
                const DWORD F = (i == xres - 1) ? E2 : data[j*xres + ip1];

                metric[o] = Vertex4D(eq_col1(E, A), eq_col1(E, B), eq_col1(E, C), eq_col1(E, F));
            }
        }
    }

    std::vector<bool4> g_res(xres*yres); //!! avoid constant reallocs?
    std::vector<bool4> g_hori(xres*yres);
    std::vector<bool4> g_vert(xres*yres);
    std::vector<bool4> g_or(xres*yres);

    o = 0;
    for (unsigned int j = 0; j < yres; ++j)
    {
        const unsigned int jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const unsigned int jp1 = ((j == yres-1) ? yres-1 : j + 1)*xres;
        const unsigned int jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;

        for (unsigned int i = 0; i < xres; ++i, ++o)
        {
            const unsigned int im2 = (i <= 1) ? 0 : i - 2;
            const unsigned int im1 = (i == 0) ? 0 : i - 1;
            const unsigned int ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const unsigned int ip2 = (i >= xres - 2) ? xres - 1 : i + 2;

            const Vertex4D M = metric[jm1 + im2];
            const Vertex4D A = metric[jm1 + im1];
            const Vertex4D B = metric[jm1 + i];
            const Vertex4D C = metric[jm1 + ip1];
            const Vertex4D P = metric[jm1 + ip2];

            const Vertex4D N = metric[j*xres + im2];
            const Vertex4D D = metric[j*xres + im1];
            const Vertex4D E = metric[j*xres + i];
            const Vertex4D F = metric[j*xres + ip1];
            const Vertex4D Q = metric[j*xres + ip2];

            const Vertex4D O = metric[jp1 + im2];
            const Vertex4D G = metric[jp1 + im1];
            const Vertex4D H = metric[jp1 + i];
            const Vertex4D I = metric[jp1 + ip1];
            const Vertex4D R = metric[jp1 + ip2];

            const Vertex4D J = metric[jp2 + im1];
            const Vertex4D K = metric[jp2 + i];
            const Vertex4D L = metric[jp2 + ip1];

            const Vertex4D As = str(Vertex4D(M.z, B.x, D.z, D.x), Vertex4D(A.y, A.w, D.y, M.w));
            const Vertex4D Bs = str(Vertex4D(A.z, C.x, E.z, E.x), Vertex4D(B.y, B.w, E.y, A.w));
            const Vertex4D Cs = str(Vertex4D(B.z, P.x, F.z, F.x), Vertex4D(C.y, C.w, F.y, B.w));
            const Vertex4D Ds = str(Vertex4D(N.z, E.x, G.z, G.x), Vertex4D(D.y, D.w, G.y, N.w));
            const Vertex4D Es = str(Vertex4D(D.z, F.x, H.z, H.x), Vertex4D(E.y, E.w, H.y, D.w));
            const Vertex4D Fs = str(Vertex4D(E.z, Q.x, I.z, I.x), Vertex4D(F.y, F.w, I.y, E.w));
            const Vertex4D Gs = str(Vertex4D(O.z, H.x, J.z, J.x), Vertex4D(G.y, G.w, J.y, O.w));
            const Vertex4D Hs = str(Vertex4D(G.z, I.x, K.z, K.x), Vertex4D(H.y, H.w, K.y, G.w));
            const Vertex4D Is = str(Vertex4D(H.z, R.x, L.z, L.x), Vertex4D(I.y, I.w, L.y, H.w));

            // strength & dominance junctions
            const Vertex4D jSx = Vertex4D(As.z, Bs.w, Es.x, Ds.y), jDx = dom(As, Bs, Es, Ds);
            const Vertex4D jSy = Vertex4D(Bs.z, Cs.w, Fs.x, Es.y), jDy = dom(Bs, Cs, Fs, Es);
            const Vertex4D jSz = Vertex4D(Es.z, Fs.w, Is.x, Hs.y), jDz = dom(Es, Fs, Is, Hs);
            const Vertex4D jSw = Vertex4D(Ds.z, Es.w, Hs.x, Gs.y), jDw = dom(Ds, Es, Hs, Gs);

            // majority vote for ambiguous dominance junctions
            const bool4 jx = ambi_dom(jDx);
            const bool4 jy = ambi_dom(jDy);
            const bool4 jz = ambi_dom(jDz);
            const bool4 jw = ambi_dom(jDw);

            // inject strength without creating new contradictions
            bool4 res;
            res.x = jx.z || !(jx.y || jx.w) && (jSx.z != 0.f && (jx.x || jSx.x + jSx.z > jSx.y + jSx.w));
            res.y = jy.w || !(jy.z || jy.x) && (jSy.w != 0.f && (jy.y || jSy.y + jSy.w > jSy.x + jSy.z));
            res.z = jz.x || !(jz.w || jz.y) && (jSz.x != 0.f && (jz.z || jSz.x + jSz.z > jSz.y + jSz.w));
            res.w = jw.y || !(jw.x || jw.z) && (jSw.y != 0.f && (jw.w || jSw.y + jSw.w > jSw.x + jSw.z));

            // single pixel & end of line detection
            res.x = res.x && (jx.z || !(res.w && res.y));
            res.y = res.y && (jy.w || !(res.x && res.z));
            res.z = res.z && (jz.x || !(res.y && res.w));
            res.w = res.w && (jw.y || !(res.z && res.x));

            // output
            const bool4 clr(
                clear(Vertex2D(D.z, E.x), Vertex4D(A.w, E.y, D.w, D.y)),
                clear(Vertex2D(E.z, F.x), Vertex4D(B.w, F.y, E.w, E.y)),
                clear(Vertex2D(H.z, I.x), Vertex4D(E.w, I.y, H.w, H.y)),
                clear(Vertex2D(G.z, H.x), Vertex4D(D.w, H.y, G.w, G.y)));

            const Vertex4D low(max(E.y, SFX_THR), max(E.w, SFX_THR), max(H.y, SFX_THR), max(D.w, SFX_THR));

            const bool4 hori = bool4(low.x < max(D.w, A.w) && clr.x, low.x < max(E.w, B.w) && clr.y, low.z < max(E.w, H.w) && clr.z, low.z < max(D.w, G.w) && clr.w); // horizontal edges
            const bool4 vert = bool4(low.w < max(E.y, D.y) && clr.x, low.y < max(E.y, F.y) && clr.y, low.y < max(H.y, I.y) && clr.z, low.w < max(H.y, G.y) && clr.w); // vertical edges
            const bool4 or = bool4(A.w < D.y, B.w <= F.y, H.w < I.y, G.w <= G.y);							                              // orientation

            g_res[o] = res;
            g_hori[o] = hori;
            g_vert[o] = vert;
            g_or[o] = or;
        }
    }

    unsigned int* metric_tmp = (unsigned int*)&(metric[0].x);

    o = 0;
    for (unsigned int j = 0; j < yres; ++j)
    {
        const unsigned int jm3 = (j <= 2) ? 0 : (j - 3)*xres;
        const unsigned int jm2 = (j <= 1) ? 0 : (j - 2)*xres;
        const unsigned int jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const unsigned int jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;
        const unsigned int jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;
        const unsigned int jp3 = ((j >= yres - 3) ? yres - 1 : j + 3)*xres;

        for (unsigned int i = 0; i < xres; ++i, ++o)
        {
            const unsigned int im3 = (i <= 2) ? 0 : i - 3;
            const unsigned int im2 = (i <= 1) ? 0 : i - 2;
            const unsigned int im1 = (i == 0) ? 0 : i - 1;
            const unsigned int ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const unsigned int ip2 = (i >= xres - 2) ? xres - 1 : i + 2;
            const unsigned int ip3 = (i >= xres - 3) ? xres - 1 : i + 3;

            // read data
            const unsigned int E = j*xres + i;
            const unsigned int D = j*xres + im1, D0 = j*xres + im2, D1 = j*xres + im3;
            const unsigned int F = j*xres + ip1, F0 = j*xres + ip2, F1 = j*xres + ip3;
            const unsigned int B = jm1 + i, B0 = jm2 + i, B1 = jm3 + i;
            const unsigned int H = jp1 + i, H0 = jp2 + i, H1 = jp3 + i;

            // extract data
            const bool4 Ec = g_res[E], Eh = g_hori[E], Ev = g_vert[E], Eo = g_or[E];
            const bool4 Dc = g_res[D], Dh = g_hori[D], Do = g_or[D], D0c = g_res[D0], D0h = g_hori[D0], D1h = g_hori[D1];
            const bool4 Fc = g_res[F], Fh = g_hori[F], Fo = g_or[F], F0c = g_res[F0], F0h = g_hori[F0], F1h = g_hori[F1];
            const bool4 Bc = g_res[B], Bv = g_vert[B], Bo = g_or[B], B0c = g_res[B0], B0v = g_vert[B0], B1v = g_vert[B1];
            const bool4 Hc = g_res[H], Hv = g_vert[H], Ho = g_or[H], H0c = g_res[H0], H0v = g_vert[H0], H1v = g_vert[H1];

            // lvl2 mid (left, right / up, down)
            const bool2 lvl2x = bool2((Ec.x && Eh.y) && Dc.z, (Ec.y && Eh.x) && Fc.w);
            const bool2 lvl2y = bool2((Ec.y && Ev.z) && Bc.w, (Ec.z && Ev.y) && Hc.x);
            const bool2 lvl2z = bool2((Ec.w && Eh.z) && Dc.y, (Ec.z && Eh.w) && Fc.x);
            const bool2 lvl2w = bool2((Ec.x && Ev.w) && Bc.z, (Ec.w && Ev.x) && Hc.y);

            // lvl3 corners (hori, vert)
            const bool2 lvl3x = bool2(lvl2x.y && (Dh.y && Dh.x) && Fh.z, lvl2w.y && (Bv.w && Bv.x) && Hv.z);
            const bool2 lvl3y = bool2(lvl2x.x && (Fh.x && Fh.y) && Dh.w, lvl2y.y && (Bv.z && Bv.y) && Hv.w);
            const bool2 lvl3z = bool2(lvl2z.x && (Fh.w && Fh.z) && Dh.x, lvl2y.x && (Hv.y && Hv.z) && Bv.x);
            const bool2 lvl3w = bool2(lvl2z.y && (Dh.z && Dh.w) && Fh.y, lvl2w.x && (Hv.x && Hv.w) && Bv.y);

            // lvl4 corners (hori, vert)
            const bool2 lvl4x = bool2((Dc.x && Dh.y && Eh.x && Eh.y && Fh.x && Fh.y) && (D0c.z && D0h.w), (Bc.x && Bv.w && Ev.x && Ev.w && Hv.x && Hv.w) && (B0c.z && B0v.y));
            const bool2 lvl4y = bool2((Fc.y && Fh.x && Eh.y && Eh.x && Dh.y && Dh.x) && (F0c.w && F0h.z), (Bc.y && Bv.z && Ev.y && Ev.z && Hv.y && Hv.z) && (B0c.w && B0v.x));
            const bool2 lvl4z = bool2((Fc.z && Fh.w && Eh.z && Eh.w && Dh.z && Dh.w) && (F0c.x && F0h.y), (Hc.z && Hv.y && Ev.z && Ev.y && Bv.z && Bv.y) && (H0c.x && H0v.w));
            const bool2 lvl4w = bool2((Dc.w && Dh.z && Eh.w && Eh.z && Fh.w && Fh.z) && (D0c.y && D0h.x), (Hc.w && Hv.x && Ev.w && Ev.x && Bv.w && Bv.x) && (H0c.y && H0v.z));

            // lvl5 mid (left, right / up, down)
            const bool2 lvl5x = bool2(lvl4x.x && (F0h.x && F0h.y) && (D1h.z && D1h.w), lvl4y.x && (D0h.y && D0h.x) && (F1h.w && F1h.z));
            const bool2 lvl5y = bool2(lvl4y.y && (H0v.y && H0v.z) && (B1v.w && B1v.x), lvl4z.y && (B0v.z && B0v.y) && (H1v.x && H1v.w));
            const bool2 lvl5z = bool2(lvl4w.x && (F0h.w && F0h.z) && (D1h.y && D1h.x), lvl4z.x && (D0h.z && D0h.w) && (F1h.x && F1h.y));
            const bool2 lvl5w = bool2(lvl4x.y && (H0v.x && H0v.w) && (B1v.z && B1v.y), lvl4w.y && (B0v.w && B0v.x) && (H1v.y && H1v.z));

            // lvl6 corners (hori, vert)
            const bool2 lvl6x = bool2(lvl5x.y && (D1h.y && D1h.x), lvl5w.y && (B1v.w && B1v.x));
            const bool2 lvl6y = bool2(lvl5x.x && (F1h.x && F1h.y), lvl5y.y && (B1v.z && B1v.y));
            const bool2 lvl6z = bool2(lvl5z.x && (F1h.w && F1h.z), lvl5y.x && (H1v.y && H1v.z));
            const bool2 lvl6w = bool2(lvl5z.y && (D1h.z && D1h.w), lvl5w.x && (H1v.x && H1v.w));

            // subpixels - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
            const unsigned int crn_x = (Ec.x && Eo.x || lvl3x.x && Eo.y || lvl4x.x && Do.x || lvl6x.x && Fo.y) ? 5 : (Ec.x || lvl3x.y && !Eo.w || lvl4x.y && !Bo.x || lvl6x.y && !Ho.w) ? 1 : lvl3x.x ? 3 : lvl3x.y ? 7 : lvl4x.x ? 2 : lvl4x.y ? 6 : lvl6x.x ? 4 : lvl6x.y ? 8 : 0;
            const unsigned int crn_y = (Ec.y && Eo.y || lvl3y.x && Eo.x || lvl4y.x && Fo.y || lvl6y.x && Do.x) ? 5 : (Ec.y || lvl3y.y && !Eo.z || lvl4y.y && !Bo.y || lvl6y.y && !Ho.z) ? 3 : lvl3y.x ? 1 : lvl3y.y ? 7 : lvl4y.x ? 4 : lvl4y.y ? 6 : lvl6y.x ? 2 : lvl6y.y ? 8 : 0;
            const unsigned int crn_z = (Ec.z && Eo.z || lvl3z.x && Eo.w || lvl4z.x && Fo.z || lvl6z.x && Do.w) ? 7 : (Ec.z || lvl3z.y && !Eo.y || lvl4z.y && !Ho.z || lvl6z.y && !Bo.y) ? 3 : lvl3z.x ? 1 : lvl3z.y ? 5 : lvl4z.x ? 4 : lvl4z.y ? 8 : lvl6z.x ? 2 : lvl6z.y ? 6 : 0;
            const unsigned int crn_w = (Ec.w && Eo.w || lvl3w.x && Eo.z || lvl4w.x && Do.w || lvl6w.x && Fo.z) ? 7 : (Ec.w || lvl3w.y && !Eo.x || lvl4w.y && !Ho.w || lvl6w.y && !Bo.x) ? 1 : lvl3w.x ? 3 : lvl3w.y ? 5 : lvl4w.x ? 2 : lvl4w.y ? 8 : lvl6w.x ? 4 : lvl6w.y ? 6 : 0;

            const unsigned int mid_x = (lvl2x.x &&  Eo.x || lvl2x.y &&  Eo.y || lvl5x.x &&  Do.x || lvl5x.y &&  Fo.y) ? 5 : lvl2x.x ? 1 : lvl2x.y ? 3 : lvl5x.x ? 2 : lvl5x.y ? 4 : (Ec.x && Dc.z && Ec.y && Fc.w) ? (Eo.x ? Eo.y ? 5 : 3 : 1) : 0;
            const unsigned int mid_y = (lvl2y.x && !Eo.y || lvl2y.y && !Eo.z || lvl5y.x && !Bo.y || lvl5y.y && !Ho.z) ? 3 : lvl2y.x ? 5 : lvl2y.y ? 7 : lvl5y.x ? 6 : lvl5y.y ? 8 : (Ec.y && Bc.w && Ec.z && Hc.x) ? (!Eo.y ? !Eo.z ? 3 : 7 : 5) : 0;
            const unsigned int mid_z = (lvl2z.x &&  Eo.w || lvl2z.y &&  Eo.z || lvl5z.x &&  Do.w || lvl5z.y &&  Fo.z) ? 7 : lvl2z.x ? 1 : lvl2z.y ? 3 : lvl5z.x ? 2 : lvl5z.y ? 4 : (Ec.z && Fc.x && Ec.w && Dc.y) ? (Eo.z ? Eo.w ? 7 : 1 : 3) : 0;
            const unsigned int mid_w = (lvl2w.x && !Eo.x || lvl2w.y && !Eo.w || lvl5w.x && !Bo.x || lvl5w.y && !Ho.w) ? 1 : lvl2w.x ? 5 : lvl2w.y ? 7 : lvl5w.x ? 6 : lvl5w.y ? 8 : (Ec.w && Hc.y && Ec.x && Bc.z) ? (!Eo.w ? !Eo.x ? 1 : 5 : 7) : 0;

            // ouput
            metric_tmp[o] = crn_x | (crn_y << 4) | (crn_z << 8) | (crn_w << 12) | (mid_x << 16) | (mid_y << 20) | (mid_z << 24) | (mid_w << 28);
        }
    }

    memcpy(&g_or[0], data, xres*yres * sizeof(DWORD));

    o = 0;
    for (unsigned int j = 0; j < yres; ++j)
    {
#ifdef DMD_UPSCALE
        unsigned int offs = j*(xres*9);
        for (unsigned int i = 0; i < xres; ++i, ++o,offs += 3)
#else
        for (unsigned int i = 0; i < xres; ++i, ++o)
#endif
        {
            const unsigned int tmp = metric_tmp[o];
            const unsigned int crn_x = tmp & 15;
            const unsigned int crn_y = (tmp >> 4) & 15;
            const unsigned int crn_z = (tmp >> 8) & 15;
            const unsigned int crn_w = (tmp >> 12) & 15;

            const unsigned int mid_x = (tmp >> 16) & 15;
            const unsigned int mid_y = (tmp >> 20) & 15;
            const unsigned int mid_z = (tmp >> 24) & 15;
            const unsigned int mid_w = (tmp >> 28);

#ifndef DMD_UPSCALE
            unsigned int r = 0, g = 0, b = 0, a = 0;
#endif
            unsigned int o2 = 0;
            for (unsigned int j2 = 0; j2 < 3; ++j2)
                for (unsigned int i2 = 0; i2 < 3; ++i2,++o2)
                {
                    // determine subpixel
                    unsigned int sp=0;
                    switch (o2)
                    {
                    case 0: sp = crn_x; break;
                    case 1: sp = mid_x; break;
                    case 2: sp = crn_y; break;
                    case 3: sp = mid_w; break;
                    case 4: sp = 0; break;
                    case 5: sp = mid_y; break;
                    case 6: sp = crn_w; break;
                    case 7: sp = mid_z; break;
                    case 8: sp = crn_z; break;
                    }

                    // output coordinate - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
                    unsigned int res=0;
                    switch (sp)
                    {
                    case 0: res = 0; break;
                    case 1: res = ((i == 0) ? 0xdeadbeef : -1); break;
                    case 2: res = ((i <= 1) ? 0xdeadbeef : -2); break;
                    case 3: res = ((i == xres - 1) ? 0xdeadbeef : 1); break;
                    case 4: res = ((i >= xres - 2) ? 0xdeadbeef : 2); break;
                    case 5: res = ((j == 0) ? 0xdeadbeef : (unsigned int)(-(int)xres)); break;
                    case 6: res = ((j <= 1) ? 0xdeadbeef : -2 * xres); break;
                    case 7: res = ((j == yres - 1) ? 0xdeadbeef : xres); break;
                    case 8: res = ((j >= yres - 2) ? 0xdeadbeef : 2 * xres); break;
                    }

#ifdef DMD_UPSCALE
                    data[offs + j2*(xres*3) + i2] = (res == 0xdeadbeef) ? ((g_or[o].xyzw & 0xFEFEFEFE)>>1) : g_or[o+res].xyzw; // borders = half black/half border pixel
#else
                    const unsigned int tmp2 = (res == 0xdeadbeef) ? ((g_or[o].xyzw & 0xFEFEFEFE) >> 1) : g_or[o+res].xyzw;
                    r +=  tmp2      & 255;
                    g += (tmp2>>8)  & 255;
                    b += (tmp2>>16) & 255;
                    a +=  tmp2>>24;
#endif
                }

#ifndef DMD_UPSCALE
            r /= 9;
            g /= 9;
            b /= 9;
            a /= 9;
            data[o] = r | (g << 8) | (b << 16) | (a << 24);
#endif
        }
    }
}

STDMETHODIMP ScriptGlobalTable::put_DMDPixels(VARIANT pVal) //!! use 64bit instead of 8bit to reduce overhead??
{
   SAFEARRAY *psa = pVal.parray;

   if (psa && g_pplayer && g_pplayer->m_dmdx > 0 && g_pplayer->m_dmdy > 0)
   {
      const LONG size = g_pplayer->m_dmdx*g_pplayer->m_dmdy;
      if (!g_pplayer->m_texdmd
#ifdef DMD_UPSCALE
          || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size*(3*3)))
#else
          || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size))
#endif
      {
         if (g_pplayer->m_texdmd)
         {
            g_pplayer->m_pin3d.m_pd3dDevice->DMDShader->SetTexture("Texture0", (D3DTexture*)NULL);
            g_pplayer->m_pin3d.m_pd3dDevice->m_texMan.UnloadTexture(g_pplayer->m_texdmd);
            delete g_pplayer->m_texdmd;
         }
#ifdef DMD_UPSCALE
         g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmdx*3, g_pplayer->m_dmdy*3);
#else
         g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmdx, g_pplayer->m_dmdy);
#endif
      }

      DWORD* const data = (DWORD*)g_pplayer->m_texdmd->data(); //!! assumes tex data to be always 32bit

      VARIANT DMDState;
      DMDState.vt = VT_UI1;

      for (LONG ofs = 0; ofs < size; ++ofs)
      {
         SafeArrayGetElement(psa, &ofs, &DMDState);
         data[ofs] = DMDState.cVal; // store raw values (0..100), let shader do the rest
      }

      if (g_pplayer->m_scaleFX_DMD)
         upscale(data, g_pplayer->m_dmdx, g_pplayer->m_dmdy, true);

      g_pplayer->m_pin3d.m_pd3dDevice->m_texMan.SetDirty(g_pplayer->m_texdmd);
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDColoredPixels(VARIANT pVal) //!! use 64bit instead of 32bit to reduce overhead??
{
	SAFEARRAY *psa = pVal.parray;

	if (psa && g_pplayer && g_pplayer->m_dmdx > 0 && g_pplayer->m_dmdy > 0)
	{
		const LONG size = g_pplayer->m_dmdx*g_pplayer->m_dmdy;
		if (!g_pplayer->m_texdmd
#ifdef DMD_UPSCALE
            || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size*(3*3)))
#else
            || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size))
#endif
		{
			if (g_pplayer->m_texdmd)
			{
				g_pplayer->m_pin3d.m_pd3dDevice->DMDShader->SetTexture("Texture0", (D3DTexture*)NULL);
				g_pplayer->m_pin3d.m_pd3dDevice->m_texMan.UnloadTexture(g_pplayer->m_texdmd);
				delete g_pplayer->m_texdmd;
			}
#ifdef DMD_UPSCALE
			g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmdx*3, g_pplayer->m_dmdy*3);
#else
			g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmdx, g_pplayer->m_dmdy);
#endif
		}

		DWORD* const data = (DWORD*)g_pplayer->m_texdmd->data(); //!! assumes tex data to be always 32bit

		VARIANT DMDState;
		DMDState.vt = VT_UI4;

		for (LONG ofs = 0; ofs < size; ++ofs)
		{
			SafeArrayGetElement(psa, &ofs, &DMDState);
			data[ofs] = DMDState.uintVal | 0xFF000000u; // store RGB values and let shader do the rest (set alpha to let shader know that this is RGB and not just brightness)
		}

        if (g_pplayer->m_scaleFX_DMD)
            upscale(data, g_pplayer->m_dmdx, g_pplayer->m_dmdy, false);

		g_pplayer->m_pin3d.m_pd3dDevice->m_texMan.SetDirty(g_pplayer->m_texdmd);
	}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetBalls(LPSAFEARRAY *pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   CComSafeArray<VARIANT> balls((ULONG)g_pplayer->m_vball.size());

   for (unsigned i = 0; i < g_pplayer->m_vball.size(); ++i)
   {
      BallEx *pballex = g_pplayer->m_vball[i]->m_pballex;

      if (!pballex)
         return E_POINTER;

      CComVariant v = static_cast<IDispatch*>(pballex);
      v.Detach(&balls[(int)i]);
   }

   *pVal = balls.Detach();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetElements(LPSAFEARRAY *pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   PinTable *pt = g_pplayer->m_ptable;

   CComSafeArray<VARIANT> objs(pt->m_vedit.Size());

   for (int i = 0; i < pt->m_vedit.Size(); ++i)
   {
      IEditable *pie = pt->m_vedit.ElementAt(i);

      CComVariant v = pie->GetISelect()->GetDispatch();
      v.Detach(&objs[i]);
   }

   *pVal = objs.Detach();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetElementByName(BSTR name, IDispatch* *pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   PinTable *pt = g_pplayer->m_ptable;

   for (int i = 0; i < pt->m_vedit.Size(); ++i)
   {
      IEditable *pie = pt->m_vedit.ElementAt(i);

      if (wcscmp(name, pie->GetScriptable()->m_wzName) == 0)
      {
         IDispatch *id = pie->GetISelect()->GetDispatch();
         id->AddRef();
         *pVal = id;

         return S_OK;
      }
   }

   *pVal = NULL;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Version(int *pVal)
{
	*pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VPBuildVersion(int *pVal)
{
	*pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VersionMajor(int *pVal)
{
	*pVal = VP_VERSION_MAJOR;
	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VersionMinor(int *pVal)
{
	*pVal = VP_VERSION_MINOR;
	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VersionRevision(int *pVal)
{
	*pVal = VP_VERSION_REV;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////


PinTable::PinTable()
{
   for (int i = 0; i < 8; i++)
      m_activeLayers[i] = true;
   m_toggleAllLayers = false;
   m_savingActive = false;
   m_renderSolid = GetRegBoolWithDefault("Editor", "RenderSolid", true);

   ClearMultiSel();

   m_hbmOffScreen = NULL;

   m_undo.m_ptable = this;
   m_fGrid = true;
   m_fBackdrop = true;

   m_fRenderDecals = true;
   m_fRenderEMReels = true;

   m_fReflectionEnabled = false;

   m_fOverridePhysics = 0;
   m_defaultBulbIntensityScaleOnBall = 1.0f;

   SetDefaultPhysics(false);

   m_defaultScatter = 0;
   m_nudgeTime = 5.0f;

   m_plungerNormalize = 100;  //Mech-Plunger component adjustment or weak spring, aging
   m_plungerFilter = false;
   m_PhysicsMaxLoops = 0xFFFFFFFF;
   /*const HRESULT hr =*/ GetRegInt("Player", "PhysicsMaxLoops", (int*)&m_PhysicsMaxLoops);

   m_right = 0.0f;
   m_bottom = 0.0f;

   m_glassheight = 210;
   m_tableheight = 0;

   m_BG_current_set = BG_DESKTOP;
   /*const HRESULT hr =*/ GetRegInt("Player", "BGSet", (int*)&m_BG_current_set);
   m_currentBackglassMode = m_BG_current_set;

   m_BG_enable_FSS = false;
   //if (m_BG_enable_FSS)
   //   m_currentBackglassMode = FULL_SINGLE_SCREEN;

   for (int i = 0; i < NUM_BG_SETS; ++i)
   {
      m_BG_inclination[i] = 0;
      m_BG_FOV[i] = FLT_MAX;

      m_BG_rotation[i] = 0;
      m_BG_layback[i] = 0;

      m_BG_scalex[i] = 1.0f;
      m_BG_scaley[i] = 1.0f;

      m_BG_xlatex[i] = 0.0f;
      m_BG_xlatey[i] = 0.0f;

      m_BG_scalez[i] = 1.0f;
      m_BG_xlatez[i] = 0.0f;
   }

   CComObject<CodeViewer>::CreateInstance(&m_pcv);
   m_pcv->AddRef();
   m_pcv->Init((IScriptableHost*)this);
   m_pcv->Create();

   CComObject<ScriptGlobalTable>::CreateInstance(&m_psgt);
   m_psgt->AddRef();
   m_psgt->Init(this);

   m_sdsDirtyProp = eSaveClean;
   m_sdsDirtyScript = eSaveClean;
   m_sdsNonUndoableDirty = eSaveClean;
   m_sdsCurrentDirtyState = eSaveClean;

   // set up default protection security descripter
   ResetProtectionBlock();

   m_globalEmissionScale = 1.0f;

   m_szTableName = NULL;
   m_szAuthor = NULL;
   m_szVersion = NULL;
   m_szReleaseDate = NULL;
   m_szAuthorEMail = NULL;
   m_szWebSite = NULL;
   m_szBlurb = NULL;
   m_szDescription = NULL;
   m_szRules = NULL;

   m_pbTempScreenshot = NULL;

   m_tblMirrorEnabled = false;

   memset(m_szImage, 0, MAXTOKEN);
   memset(m_szEnvImage, 0, MAXTOKEN);
   m_hMaterialManager = NULL;

   m_numMaterials = 0;
   HRESULT hr;
   int tmp;


   F32 nudgesens = 0.50f;
   hr = GetRegInt("Player", "NudgeSensitivity", &tmp);
   if (hr == S_OK)
      nudgesens = (float)tmp*(float)(1.0 / 1000.0);
   nudge_set_sensitivity(nudgesens);

   m_globalDifficulty = 0.2f;			// easy by default
   hr = GetRegInt("Player", "GlobalDifficulty", &tmp);
   if (hr == S_OK)
      m_globalDifficulty = dequantizeUnsignedPercent(tmp);

   ReadAccelerometerCalibration();

   m_tblAutoStart = 0;
   hr = GetRegInt("Player", "Autostart", &tmp);
   if (hr == S_OK)
      m_tblAutoStart = tmp * 10;

   m_tblAutoStartRetry = 0;
   hr = GetRegInt("Player", "AutostartRetry", &tmp);
   if (hr == S_OK)
      m_tblAutoStartRetry = tmp * 10;

   m_tblAutoStartEnabled = false;
   hr = GetRegInt("Player", "asenable", &tmp);
   if (hr == S_OK)
      m_tblAutoStartEnabled = (tmp != 0);

   m_tblVolmod = 1.0f;
   hr = GetRegInt("Player", "Volmod", &tmp);
   if (hr == S_OK)
      m_tblVolmod = (float)tmp*(float)(1.0 / 1000.0);

   m_tblExitConfirm = 2000;
   hr = GetRegInt("Player", "Exitconfirm", &tmp);
   if (hr == S_OK)
      m_tblExitConfirm = tmp * 1000 / 60;

   SetRegValue("Version", "VPinball", REG_SZ, VP_VERSION_STRING_DIGITS, lstrlen(VP_VERSION_STRING_DIGITS));

   if (FAILED(GetRegInt("Player", "AlphaRampAccuracy", &m_globalDetailLevel)))
   {
      m_globalDetailLevel = 10;
   }
   m_userDetailLevel = 10;
   m_overwriteGlobalDetailLevel = false;

   m_overwriteGlobalDayNight = true;

   if (FAILED(GetRegStringAsFloat("Player", "Stereo3DZPD", &m_global3DZPD)))
   {
      m_global3DZPD = 0.5f;
   }
   m_3DZPD = 0.5f;
   if (FAILED(GetRegStringAsFloat("Player", "Stereo3DMaxSeparation", &m_global3DMaxSeparation)))
   {
      m_global3DMaxSeparation = 0.03f;
   }
   m_3DmaxSeparation = 0.03f;
   if (FAILED(GetRegStringAsFloat("Player", "Stereo3DOffset", &m_global3DOffset)))
   {
	   m_global3DOffset = 0.0f;
   }
   m_3DOffset = 0.0f;
   m_overwriteGlobalStereo3D = false;

   m_dbgChangedMaterials.clear();
   m_dbgChangedLights.clear();

   m_tblNudgeReadX = 0.0f;
   m_tblNudgeReadY = 0.0f;
   m_tblNudgeReadTilt = 0.0f;
   m_tblNudgePlumbX = 0.0f;
   m_tblNudgePlumbY = 0.0f;

#ifdef UNUSED_TILT
   if ( FAILED(GetRegInt("Player", "JoltAmount", &m_jolt_amount) )
      m_jolt_amount = 500;
   if ( FAILED(GetRegInt("Player", "TiltAmount", &m_tilt_amount) )
      m_tilt_amount = 950;
   if ( FAILED(GetRegInt("Player", "JoltTriggerTime", &m_jolt_trigger_time) )
      m_jolt_trigger_time = 1000;
   if ( FAILED(GetRegInt("Player", "TiltTriggerTime", &m_tilt_trigger_time) )
      m_tilt_trigger_time = 10000;
#endif
}

void PinTable::ReadAccelerometerCalibration()
{
	HRESULT hr;
	int tmp;

	int accel;
	hr = GetRegInt("Player", "PBWEnabled", &accel); // true if electronic accelerometer enabled
	if (hr == S_OK)
		m_tblAccelerometer = (accel != fFalse);
	else
		m_tblAccelerometer = true;

	hr = GetRegInt("Player", "PBWNormalMount", &accel); // true is normal mounting (left hand coordinates)
	if (hr == S_OK)
		m_tblAccelNormalMount = (accel != fFalse);
	else
		m_tblAccelNormalMount = true;

	m_tblAccelAngle = 0.0f;			// 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
	hr = GetRegInt("Player", "PBWRotationCB", &accel);
	if ((hr == S_OK) && accel)
	{
		hr = GetRegInt("Player", "PBWRotationValue", &tmp);
		if (hr == S_OK)
			m_tblAccelAngle = (float)tmp;
	}

	m_tblAccelAmpX = 1.5f;
	hr = GetRegInt("Player", "PBWAccelGainX", &tmp);
	if (hr == S_OK)
		m_tblAccelAmpX = dequantizeUnsignedPercentNoClamp(tmp);

	m_tblAccelAmpY = 1.5f;
	hr = GetRegInt("Player", "PBWAccelGainY", &tmp);
	if (hr == S_OK)
		m_tblAccelAmpY = dequantizeUnsignedPercentNoClamp(tmp);

	m_tblAccelMaxX = JOYRANGEMX;
	hr = GetRegInt("Player", "PBWAccelMaxX", &tmp);
	if (hr == S_OK)
		m_tblAccelMaxX = tmp*JOYRANGEMX / 100;

	m_tblAccelMaxY = JOYRANGEMX;
	hr = GetRegInt("Player", "PBWAccelMaxY", &tmp);
	if (hr == S_OK)
		m_tblAccelMaxY = tmp*JOYRANGEMX / 100;

	// bug!! If tilt sensitiivty is not set, it's supposed to disable analog tilting, see KeysConfigDialog.cpp
	F32 tiltsens = 0.40f;  
	hr = GetRegInt("Player", "TiltSensitivity", &tmp);
	if (hr == S_OK)
		tiltsens = (float)tmp*(float)(1.0 / 1000.0);
	plumb_set_sensitivity(tiltsens);

	if (g_pplayer)
		g_pplayer->m_pininput.LoadSettings();
}

PinTable::~PinTable()
{
   for (int i = 0; i < m_vedit.Size(); i++)
      m_vedit.ElementAt(i)->Release();

   ClearOldSounds();

   for (int i = 0; i < m_vsound.Size(); i++)
   {
      //m_vsound.ElementAt(i)->m_pDSBuffer->Release();
      delete m_vsound.ElementAt(i);
   }

   for (unsigned i = 0; i < m_vimage.size(); i++)
      delete m_vimage[i];

   for(int i = 0; i < m_materials.size(); ++i)
      delete m_materials.ElementAt(i);

   for (int i = 0; i < m_vfont.Size(); i++)
   {
      m_vfont.ElementAt(i)->UnRegister();
      delete m_vfont.ElementAt(i);
   }

   for (int i = 0; i < m_vcollection.Size(); i++)
      m_vcollection.ElementAt(i)->Release();

   for (int i = 0; i < m_vCustomInfoTag.Size(); i++)
   {
      delete m_vCustomInfoTag.ElementAt(i);
      delete m_vCustomInfoContent.ElementAt(i);
   }

   m_pcv->Release();

   m_psgt->Release();

   if (::IsWindow(m_hwnd))
   {
      //DestroyWindow(m_hwnd);
      ::SendMessage(g_pvp->m_hwndWork, WM_MDIDESTROY, (size_t)m_hwnd, 0);
   }

   if (m_hbmOffScreen)
      DeleteObject(m_hbmOffScreen);

   SAFE_VECTOR_DELETE(m_szTableName);
   SAFE_VECTOR_DELETE(m_szAuthor);
   SAFE_VECTOR_DELETE(m_szVersion);
   SAFE_VECTOR_DELETE(m_szReleaseDate);
   SAFE_VECTOR_DELETE(m_szAuthorEMail);
   SAFE_VECTOR_DELETE(m_szWebSite);
   SAFE_VECTOR_DELETE(m_szBlurb);
   SAFE_VECTOR_DELETE(m_szDescription);
   SAFE_VECTOR_DELETE(m_szRules);
}

BOOL PinTable::FVerifySaveToClose()
{
   if (m_vAsyncHandles.size() > 0)
   {
      /*const DWORD wait =*/ WaitForMultipleObjects((DWORD)m_vAsyncHandles.size(), m_vAsyncHandles.data(), TRUE, INFINITE);
      //MessageBox(NULL, "Async work items not done", NULL, 0);

      // Close the remaining handles here, since the window messages will never be processed
      for (unsigned i = 0; i < m_vAsyncHandles.size(); i++)
         CloseHandle(m_vAsyncHandles[i]);

      g_pvp->SetActionCur("");
   }

   return fTrue;
}

BOOL PinTable::CheckPermissions(unsigned long flag)
{
   return (((m_protectionData.flags & DISABLE_EVERYTHING) == DISABLE_EVERYTHING) ||
      ((m_protectionData.flags & flag) == flag));
}

BOOL PinTable::IsTableProtected()
{
   return (m_protectionData.flags != 0);
}

void PinTable::ResetProtectionBlock()
{
   // set up default protection security descripter
   ZeroMemory(&m_protectionData, sizeof(m_protectionData));
   m_protectionData.fileversion = PROT_DATA_VERSION;
   m_protectionData.size = sizeof(m_protectionData);
}

BOOL PinTable::SetupProtectionBlock(unsigned char *pPassword, unsigned long flags)
{
   int foo;
   HCRYPTPROV   hcp = NULL;
   HCRYPTKEY  	hkey = NULL;
   HCRYPTHASH 	hchkey = NULL;

   _protectionData *pProtectionData = &m_protectionData;

   // acquire a crypto context
   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
   foo = GetLastError();
   // create a hash
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, pPassword, lstrlen((char *)pPassword), 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
   foo = GetLastError();

   // copy our paraphrase key into the key field
   memcpy(pProtectionData->paraphrase, PARAPHRASE_KEY, sizeof(PARAPHRASE_KEY));

   // encypt this with the key generated with the password
   DWORD cryptlen;
   // get the size of the data to encrypt
   cryptlen = PROT_PASSWORD_LENGTH;

   // encrypt the paraphrase
   foo = CryptEncrypt(hkey,									// key to use
      0, 										// not hashing data at the same time
      TRUE, 									// last block (or only block)
      0, 										// no flags
      (BYTE *)pProtectionData->paraphrase,	// buffer to encrypt
      &cryptlen,								// size of data to encrypt
      sizeof(pProtectionData->paraphrase));	// maximum size of buffer (includes any padding)

   foo = GetLastError();		// purge any errors

   // set up the flags
   pProtectionData->flags = flags;

   // destroy our cryto
   foo = CryptDestroyHash(hchkey);
   foo = CryptDestroyKey(hkey);
   foo = CryptReleaseContext(hcp, 0);

   return fTrue;
}

BOOL PinTable::UnlockProtectionBlock(unsigned char *pPassword)
{
   char secret1[] = "Could not create";
   if ((memcmp(pPassword, &secret1, sizeof(secret1)) == 0))
   {
      ResetProtectionBlock();
      return fTrue;
   }

   int foo;
   HCRYPTPROV  	    hcp = NULL;
   HCRYPTKEY  		hkey = NULL;
   HCRYPTHASH 		hchkey = NULL;
   unsigned char	paraphrase[PROT_CIPHER_LENGTH];
   _protectionData *pProtectionData = &m_protectionData;

   // acquire a crypto context
   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
   foo = GetLastError();
   // create a hash
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, pPassword, lstrlen((char *)pPassword), 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
   foo = GetLastError();

   // copy the paraphase from the protection block to some local memory
   memcpy(paraphrase, pProtectionData->paraphrase, sizeof(paraphrase));

   // decypt this with the key generated with the password
   DWORD cryptlen;
   // get the size of the data to encrypt
   cryptlen = PROT_CIPHER_LENGTH;

   // encrypt the paraphrase
   foo = CryptDecrypt(hkey,									// key to use
      0, 										// not hashing data at the same time
      TRUE, 									// last block (or only block)
      0, 										// no flags
      (BYTE *)paraphrase,						// buffer to encrypt
      &cryptlen);								// size of data to decrypt

   // destroy our cryto
   foo = CryptDestroyHash(hchkey);
   foo = CryptDestroyKey(hkey);
   foo = CryptReleaseContext(hcp, 0);

   //	if the decrypted data matches the original paraphrase then unlock the table
   if ((cryptlen == PROT_PASSWORD_LENGTH) &&
      (memcmp(paraphrase, PARAPHRASE_KEY, sizeof(PARAPHRASE_KEY)) == 0))
   {
      ResetProtectionBlock();
      return fTrue;
   }
   return fFalse;
}

void PinTable::SwitchToLayer(int layerNumber)
{
   // scan through all layers if all elements are already stored to a layer
   // if not new elements will be stored in layer1
   for (int t = 0; t < m_vedit.Size(); t++)
   {
      IEditable *piedit = m_vedit.ElementAt(t);
      bool alreadyIn = false;
      for (int i = 0; i < 8; i++)
      {
         if (m_layer[i].IndexOf(piedit) != -1)
            alreadyIn = true;
      }
      if (!alreadyIn)
      {
         piedit->GetISelect()->layerIndex = 0;
         m_layer[0].AddElement(piedit);
      }
   }
   //toggle layer
   m_activeLayers[layerNumber] ^= true;

   // now set all elements to visible if their layer is active, otherwise hide them
   for (int i = 0; i < 8; i++)
   {
      if (m_activeLayers[i])
      {
         for (int t = 0; t < m_layer[i].Size(); t++)
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->m_isVisible = true;
         }
      }
      else
      {
         for (int t = 0; t < m_layer[i].Size(); t++)
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->m_isVisible = false;
         }
      }
   }

   SetDirtyDraw();
}

void PinTable::AssignToLayer(IEditable *obj, int layerNumber)
{
   if (!m_activeLayers[layerNumber])
      obj->m_isVisible = false;
   m_layer[obj->GetISelect()->layerIndex].RemoveElement(obj);
   obj->GetISelect()->layerIndex = layerNumber;
   m_layer[layerNumber].InsertElementAt(obj, 0);
   SetDirtyDraw();
}

void PinTable::MergeAllLayers()
{
   for (int t = 1; t < 8; t++)
   {
      for (int i = m_layer[t].Size() - 1; i >= 0; i--)
      {
         IEditable *piedit = m_layer[t].ElementAt(i);
         piedit->GetISelect()->layerIndex = 0;
         m_layer[0].AddElement(piedit);
      }
      m_layer[t].RemoveAllElements();
   }
   m_layer[0].Clone(&m_vedit);

   SetDirtyDraw();
}

void PinTable::BackupLayers()
{
   // scan through all layers if all elements are already stored to a layer
   // if not new elements will be stored in layer1
   for (int t = 0; t < m_vedit.Size(); t++)
   {
      IEditable *piedit = m_vedit.ElementAt(t);
      bool alreadyIn = false;
      for (int i = 0; i < 8; i++)
      {
         if (m_layer[i].IndexOf(piedit) != -1)
            alreadyIn = true;
      }
      if (!alreadyIn)
      {
         piedit->GetISelect()->layerIndex = 0;
         m_layer[0].AddElement(piedit);
      }
   }
   // make all elements visible again
   for (int t = 0; t < 8; t++)
   {
      //      for( int i=m_layer[t].Size()-1;i>=0;i-- )
      for (int i = 0; i < m_layer[t].Size(); i++)
      {
         IEditable *piedit = m_layer[t].ElementAt(i);
         piedit->m_isVisible = true;
      }
   }
}

void PinTable::RestoreLayers()
{
   for (int i = 0; i < 8; i++)
   {
      if (m_activeLayers[i])
      {
         for (int t = 0; t < m_layer[i].Size(); t++)
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->m_isVisible = true;
         }
      }
      else
      {
         for (int t = 0; t < m_layer[i].Size(); t++)
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->m_isVisible = false;
         }
      }
   }
}

void PinTable::DeleteFromLayer(IEditable *obj)
{
   for (int i = 0; i < 8; i++)
   {
      if (m_layer[i].IndexOf(obj) != -1)
      {
         m_layer[i].RemoveElement(obj);
         break;
      }
   }
}

#define NEWFROMRES 1

void PinTable::Init(VPinball *pvp, const bool useBlankTable)
{
   m_pvp = pvp;

#ifdef NEWFROMRES
   HRSRC hrsrc;
   // Get our new table resource, get it to be opened as a storage, and open it like a normal file
   if (useBlankTable)
      hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_BLANK_TABLE), "TABLE");
   else
      hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_EXAMPLE_TABLE), "TABLE");

   HGLOBAL hglobal = LoadResource(NULL, hrsrc);
   char *pchar = (char *)LockResource(hglobal);
   DWORD size = SizeofResource(NULL, hrsrc);
   HGLOBAL hcopiedmem = GlobalAlloc(GMEM_MOVEABLE, size);
   char *pcopied = (char *)GlobalLock(hcopiedmem);
   memcpy(pcopied, pchar, size);
   GlobalUnlock(hcopiedmem);

   ILockBytes *pilb;
   CreateILockBytesOnHGlobal(hcopiedmem, TRUE, &pilb); // "TRUE" parm gives ownership of hcopiedmem to Global Object

   IStorage *pis;
   StgOpenStorageOnILockBytes(pilb, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pis);
   pilb->Release();	// free pilb and hcopiedmem

   m_glassheight = 210;
   m_tableheight = 0;

   for (int i = 0; i < 16; i++)
      m_rgcolorcustom[i] = RGB(0, 0, 0);

   //pilb->Release();

   //LoadGameFromFilename("d:\\gdk\\data\\tables\\newsave\\basetable6.vpt");

   char szSuffix[32];

   LocalString ls(IDS_TABLE);
   lstrcpy(m_szTitle, ls.m_szbuffer/*"Table"*/);
   _itoa_s(g_pvp->m_NextTableID, szSuffix, sizeof(szSuffix), 10);
   lstrcat(m_szTitle, szSuffix);
   g_pvp->m_NextTableID++;
   m_szFileName[0] = '\0';

   LoadGameFromStorage(pis);

   m_angletiltMin = 6;
   m_angletiltMax = 726;

   //MAKE_WIDEPTR_FROMANSI(wszFileName, m_szFileName);
   //ApcProject->APC_PUT(DisplayName)(wszFileName);

   InitPostLoad(pvp);

   SetCaption(m_szTitle);

#else

   m_szFileName[0] = 0;
   m_szBlueprintFileName[0] = 0;

   //m_ptinfoCls = NULL;
   //m_ptinfoInt = NULL;

   m_left = 0;
   m_top = 0;
   m_right = EDITOR_BG_WIDTH;
   m_bottom = EDITOR_BG_WIDTH*2;

   m_scalex = 1.0f;
   m_scaley = 1.0f;
   m_zScale = 1.0f;

   m_xlatex = 0.0f;
   m_xlatey = 0.0f;
   m_xlatez = 0.0f;

   m_inclination = 43;
   m_layback = 0;
   m_FOV = 45;

   m_maxSeparation = 0.03f;
   m_ZPD = 0.5f;

   SetDefaultView();

   m_szImage[0] = 0;
   m_szImageBackdrop[0] = 0;
   m_ImageBackdropNightDay = false;
   m_szEnvImage[0] = 0;

   m_szImageColorGrade[0] = 0;

   m_colorplayfield = RGB(128,128,128);
   m_colorbackdrop = RGB(128,128,128);

   InitVBA();

   CreateTableWindow();

   SetMyScrollInfo();
#endif
}

void PinTable::SetDefaultView()
{
   FRect frect;
   GetViewRect(&frect);
   m_offset = frect.Center();
   m_zoom = 0.5f;
}

void PinTable::SetCaption(char *szCaption)
{
   ::SetWindowText(m_hwnd, szCaption);
   m_pcv->SetCaption(szCaption);
}


void PinTable::InitPostLoad(VPinball *pvp)
{
   m_pvp = pvp;
   pvp->m_ptableActive = (CComObject<PinTable> *)this;

   for(unsigned int i = 1; i < NUM_BG_SETS; ++i)
      if (m_BG_FOV[i] == FLT_MAX) // old table, copy FS and/or FSS settings over from old DT setting
      {
        m_BG_inclination[i] = m_BG_inclination[BG_DESKTOP];
        m_BG_FOV[i] = m_BG_FOV[BG_DESKTOP];

        m_BG_rotation[i] = m_BG_rotation[BG_DESKTOP];
        m_BG_layback[i] = m_BG_layback[BG_DESKTOP];

        m_BG_scalex[i] = m_BG_scalex[BG_DESKTOP];
        m_BG_scaley[i] = m_BG_scaley[BG_DESKTOP];

        m_BG_xlatex[i] = m_BG_xlatex[BG_DESKTOP];
        m_BG_xlatey[i] = m_BG_xlatey[BG_DESKTOP];

        m_BG_scalez[i] = m_BG_scalez[BG_DESKTOP];
        m_BG_xlatez[i] = m_BG_xlatez[BG_DESKTOP];

        if(m_BG_szImage[i][0] == 0 && i == BG_FSS) // copy image over for FSS mode
            strcpy_s(m_BG_szImage[i], m_BG_szImage[BG_DESKTOP]);
      }

   m_currentBackglassMode = m_BG_current_set;
   if (m_BG_enable_FSS)
      m_currentBackglassMode = BG_FSS;

   m_hbmOffScreen = NULL;
   m_fDirtyDraw = true;

   m_left = 0;
   m_top = 0;

   SetDefaultView();

   m_szBlueprintFileName[0] = 0;

   CreateGDIBackdrop();

   //InitVBA();
   m_pcv->AddItem(this, fFalse);
   m_pcv->AddItem(m_psgt, fTrue);
   m_pcv->AddItem(m_pcv->m_pdm, fFalse);

   CreateTableWindow();

   SetMyScrollInfo();
}


BOOL FWzEqual(const WCHAR *wz1, const WCHAR *wz2)
{
   while (*wz1 != 0 || *wz2 != 0)
   {
      if (*wz1++ != *wz2++)
      {
         return fFalse;
      }
   }
   return fTrue;
}


bool PinTable::IsNameUnique(WCHAR *wzName)
{
   return m_pcv->m_vcvd.GetSortedIndex(wzName) == -1;
}


void PinTable::GetUniqueName(ItemTypeEnum type, WCHAR *wzUniqueName)
{
   WCHAR wzRoot[256];
   ISelect::GetTypeNameForType(type, wzRoot);
   GetUniqueName(wzRoot, wzUniqueName);
}

void PinTable::GetUniqueName(WCHAR *wzRoot, WCHAR *wzUniqueName)
{
   int suffix = 1;
   bool fFound = false;
   WCHAR wzName[128];
   WCHAR wzSuffix[10];

   while (!fFound)
   {
      WideStrNCopy(wzRoot, wzName, 128);
      _itow_s(suffix, wzSuffix, sizeof(wzSuffix) / sizeof(WCHAR), 10);
      WideStrCat(wzSuffix, wzName);

      if (IsNameUnique(wzName))
      {
         fFound = true;
      }
      else
      {
         suffix += 1;
      }
   }

   WideStrCopy(wzName, wzUniqueName);
}

void PinTable::GetUniqueNamePasting(int type, WCHAR *wzUniqueName)
{
   //if the original name is not yet used, use that one (so there's nothing we have to do) 
   //otherwise add/increase the suffix untill we find a name that's not used yet
   if (!IsNameUnique(wzUniqueName))
   {
      //first remove the existing suffix
      while (iswdigit(wzUniqueName[wcslen(wzUniqueName) - 1]))
      {
         wzUniqueName[wcslen(wzUniqueName) - 1] = L'\0';
      }

      GetUniqueName(wzUniqueName, wzUniqueName);
   }
}

void PinTable::Render(Sur * const psur)
{
   RECT rc;
   ::GetClientRect(m_hwnd, &rc);
   psur->SetFillColor(g_pvp->m_backgroundColor);
   psur->SetBorderColor(-1, false, 0);

   FRect frect;
   GetViewRect(&frect);

   psur->Rectangle2(rc.left, rc.top, rc.right, rc.bottom);

   // can we view the table elements?? if not then draw a box for the table outline
   // got to give the punters at least something to know that the table has loaded
   if (CheckPermissions(DISABLE_TABLEVIEW))
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      psur->SetObject(NULL); 						// Don't hit test edgelines

      psur->SetLineColor(RGB(0, 0, 0), false, 0);		// black outline

      psur->Line(rlt.x, rlt.y, rrb.x, rlt.y);
      psur->Line(rrb.x, rlt.y, rrb.x, rrb.y);
      psur->Line(rlt.x, rrb.y, rrb.x, rrb.y);
      psur->Line(rlt.x, rlt.y, rlt.x, rrb.y);

      return;
   }

   if (m_fBackdrop)
   {
      Texture * const ppi = GetImage((!g_pvp->m_fBackglassView) ? m_szImage : m_BG_szImage[m_BG_current_set]);

      if (ppi)
      {
         ppi->EnsureHBitmap();
         if (ppi->m_hbmGDIVersion)
         {
            HDC hdcScreen = ::GetDC(NULL);
            HDC hdcNew = CreateCompatibleDC(hdcScreen);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, ppi->m_hbmGDIVersion);

            psur->Image(frect.left, frect.top, frect.right, frect.bottom, hdcNew, ppi->m_width, ppi->m_height);

            SelectObject(hdcNew, hbmOld);
            DeleteDC(hdcNew);
            ::ReleaseDC(NULL, hdcScreen);
         }
      }
   }

   if (g_pvp->m_fBackglassView)
   {
      Render3DProjection(psur);
   }


   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         if (ptr->m_isVisible)
            ptr->PreRender(psur);
      }
   }

   if (m_fGrid && g_pvp->m_gridSize > 0)
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      const float gridsize = (float)g_pvp->m_gridSize;

      const int beginx = (int)(rlt.x / gridsize);
      const float lenx = (rrb.x - rlt.x) / gridsize;//(((rc.right - rc.left)/m_zoom));
      const int beginy = (int)(rlt.y / gridsize);
      const float leny = (rrb.y - rlt.y) / gridsize;//(((rc.bottom - rc.top)/m_zoom));

      psur->SetObject(NULL); // Don't hit test gridlines

      psur->SetLineColor(RGB(190, 220, 240), false, 0);
      for (int i = 0; i < (lenx + 1); i++)
      {
         const float x = (float)(beginx + i)*gridsize;
         psur->Line(x, rlt.y, x, rrb.y);
      }

      for (int i = 0; i < (leny + 1); i++)
      {
         const float y = (float)(beginy + i)*gridsize;
         psur->Line(rlt.x, y, rrb.x, y);
      }
   }

   for (int i = 0; i < m_vedit.Size(); i++)
   {
      if (m_vedit.ElementAt(i)->m_fBackglass == g_pvp->m_fBackglassView)
      {
         if (m_vedit.ElementAt(i)->m_isVisible)
            m_vedit.ElementAt(i)->Render(psur);
      }
   }

   if (g_pvp->m_fBackglassView) // Outline of the view, for when the grid is off
   {
      psur->SetObject(NULL);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), false, 1);
      psur->Rectangle(0, 0, EDITOR_BG_WIDTH, EDITOR_BG_HEIGHT);
   }

   if (m_fDragging)
   {
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), true, 0);
      psur->Rectangle(m_rcDragRect.left, m_rcDragRect.top, m_rcDragRect.right, m_rcDragRect.bottom);
   }

   // display the layer string
   //    psur->SetObject(NULL);
   //    SetTextColor( psur->m_hdc,RGB(180,180,180));
   //    char text[64];
   //    char number[8];
   //    strcpy_s( text,"Layer_");
   //    _itoa_s(activeLayer+1, number, 10 );
   //    strcat_s( text, number);
   //    RECT textRect;
   //    SetRect( &textRect, rc.right-60,rc.top, rc.right, rc.top+30 );
   //    DrawText( psur->m_hdc, text, -1, &textRect, DT_LEFT);
   // 
   //    SetTextColor( psur->m_hdc,RGB(0,0,0));

   //   psur->DrawText( text,rc.left+10, rc.top, 90,20);
}

// draws the backdrop content
void PinTable::Render3DProjection(Sur * const psur)
{
   if (m_vedit.Size() == 0)
      return;

   const float rotation = ANGTORAD(m_BG_rotation[m_BG_current_set]);
   const float inclination = ANGTORAD(m_BG_inclination[m_BG_current_set]);
   const float FOV = (m_BG_FOV[m_BG_current_set] < 1.0f) ? 1.0f : m_BG_FOV[m_BG_current_set]; // Can't have a real zero FOV, but this will look almost the same

   std::vector<Vertex3Ds> vvertex3D;
   for (int i = 0; i < m_vedit.Size(); i++)
      m_vedit.ElementAt(i)->GetBoundingVertices(vvertex3D);

   // dummy coordinate system for backdrop view
   PinProjection pinproj;
   pinproj.m_rcviewport.left = 0;
   pinproj.m_rcviewport.top = 0;
   pinproj.m_rcviewport.right = EDITOR_BG_WIDTH;
   pinproj.m_rcviewport.bottom = EDITOR_BG_HEIGHT;

   //const float aspect = 4.0f/3.0f;
   int renderWidth, renderHeight;
   GetRegInt("Player", "Width", &renderWidth);
   GetRegInt("Player", "Height", &renderHeight);
   const float aspect = ((float)renderWidth) / ((float)renderHeight); //(float)(4.0/3.0);

   pinproj.FitCameraToVertices(vvertex3D, aspect, rotation, inclination, FOV, m_BG_xlatez[m_BG_current_set], m_BG_layback[m_BG_current_set]);
   pinproj.m_matView.RotateXMatrix((float)M_PI);  // convert Z=out to Z=in (D3D coordinate system)
   pinproj.m_matWorld.SetIdentity();
   D3DXMATRIX proj;
   D3DXMatrixPerspectiveFovLH(&proj, ANGTORAD(FOV), aspect, pinproj.m_rznear, pinproj.m_rzfar);
   memcpy(pinproj.m_matProj.m, proj.m, sizeof(float) * 4 * 4);

   //pinproj.SetFieldOfView(FOV, aspect, pinproj.m_rznear, pinproj.m_rzfar);

   pinproj.ScaleView(m_BG_scalex[m_BG_current_set], m_BG_scaley[m_BG_current_set], 1.0f);
   pinproj.TranslateView(m_BG_xlatex[m_BG_current_set] - pinproj.m_vertexcamera.x, m_BG_xlatey[m_BG_current_set] - pinproj.m_vertexcamera.y, -pinproj.m_vertexcamera.z);
   pinproj.RotateView(0, 0, rotation);
   pinproj.RotateView(inclination, 0, 0);
   pinproj.MultiplyView(ComputeLaybackTransform(m_BG_layback[m_BG_current_set]));

   pinproj.CacheTransform();

   psur->SetFillColor(RGB(200, 200, 200));
   psur->SetBorderColor(-1, false, 0);

   Vertex3Ds rgvIn[8];
   rgvIn[0].x = m_left;    rgvIn[0].y = m_top;    rgvIn[0].z = 50.0f;
   rgvIn[1].x = m_left;    rgvIn[1].y = m_top;    rgvIn[1].z = m_glassheight;
   rgvIn[2].x = m_right;   rgvIn[2].y = m_top;    rgvIn[2].z = m_glassheight;
   rgvIn[3].x = m_right;   rgvIn[3].y = m_top;    rgvIn[3].z = 50.0f;
   rgvIn[4].x = m_right;   rgvIn[4].y = m_bottom; rgvIn[4].z = 50.0f;
   rgvIn[5].x = m_right;   rgvIn[5].y = m_bottom; rgvIn[5].z = 0.0f;
   rgvIn[6].x = m_left;    rgvIn[6].y = m_bottom; rgvIn[6].z = 0.0f;
   rgvIn[7].x = m_left;    rgvIn[7].y = m_bottom; rgvIn[7].z = 50.0f;

   Vertex2D rgvOut[8];
   pinproj.TransformVertices(rgvIn, NULL, 8, rgvOut);
   psur->Polygon(rgvOut, 8);
}


BOOL PinTable::GetDecalsEnabled() const
{
   return m_fRenderDecals;
}


BOOL PinTable::GetEMReelsEnabled() const
{
   return m_fRenderEMReels;
}

// draws the main design screen
void PinTable::Paint(HDC hdc)
{
   //HBITMAP hbmOffScreen;

   RECT rc;
   ::GetClientRect(m_hwnd, &rc);

   if (m_fDirtyDraw)
   {
      if (m_hbmOffScreen)
      {
         DeleteObject(m_hbmOffScreen);
      }
      m_hbmOffScreen = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
   }

   HDC hdc2 = CreateCompatibleDC(hdc);

   HBITMAP hbmOld = (HBITMAP)SelectObject(hdc2, m_hbmOffScreen);

   if (m_fDirtyDraw)
   {
      Sur * const psur = new PaintSur(hdc2, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, GetSelectedItem());
      Render(psur);

      delete psur;
   }

   BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, hdc2, 0, 0, SRCCOPY);

   SelectObject(hdc2, hbmOld);

   DeleteDC(hdc2);

   m_fDirtyDraw = false;
   //DeleteObject(hbmOffScreen);
}

ISelect *PinTable::HitTest(const int x, const int y)
{
   HDC hdc = ::GetDC(m_hwnd);
   HWND listHwnd = NULL;

   RECT rc;
   ::GetClientRect(m_hwnd, &rc);

   HitSur * const phs = new HitSur(hdc, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, x, y, this);
   HitSur * const phs2 = new HitSur(hdc, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, x, y, this);

   m_allHitElements.RemoveAllElements();

   Render(phs);

   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         ptr->PreRender(phs2);
         ISelect* tmp = phs2->m_pselected;
         if (m_allHitElements.IndexOf(tmp) == -1 && tmp != NULL && tmp != this)
         {
            m_allHitElements.AddElement(tmp);
         }
      }
   }
   // it's possible that PreRender doesn't find all elements  (gates,plunger)
   // check here if everything was already stored in the list
   if (m_allHitElements.IndexOf(phs->m_pselected) == -1)
   {
      m_allHitElements.AddElement(phs->m_pselected);
   }
   delete phs2;

   Vector<ISelect> tmpBuffer;
   for (int i = m_allHitElements.Size() - 1; i >= 0; i--)
   {
      tmpBuffer.AddElement(m_allHitElements.ElementAt(i));
   }
   m_allHitElements.RemoveAllElements();
   for (int i = 0; i < tmpBuffer.Size(); i++)
   {
      m_allHitElements.AddElement(tmpBuffer.ElementAt(i));
   }
   tmpBuffer.RemoveAllElements();

   ISelect * const pisel = phs->m_pselected;
   delete phs;

   ::ReleaseDC(m_hwnd, hdc);

   return pisel;
}

void PinTable::SetDirtyDraw()
{
   m_fDirtyDraw = true;
   ::InvalidateRect(m_hwnd, NULL, fFalse);
}

void PinTable::Play(bool _cameraMode)
{
   if (g_pplayer)
      return; // Can't play twice

   mixer_get_volume();

   EndAutoSaveCounter();

   // get the load path from the table filename
   char szLoadDir[MAX_PATH];
   PathFromFilename(m_szFileName, szLoadDir);
   // make sure the load directory is the active directory
   SetCurrentDirectory(szLoadDir);
   BackupLayers();

   HWND hwndProgressDialog = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_PROGRESS), g_pvp->m_hwnd, ProgressProc);
   // TEXT
   ::ShowWindow(hwndProgressDialog, SW_SHOW);

   HWND hwndProgressBar = ::GetDlgItem(hwndProgressDialog, IDC_PROGRESS2);
   HWND hwndStatusName = ::GetDlgItem(hwndProgressDialog, IDC_STATUSNAME);

   ::SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);
   ::SetWindowText(hwndStatusName, "Backing Up Table State...");
   BackupForPlay();

   m_backupLayback = m_BG_layback[m_BG_current_set];
   m_backupRotation = m_BG_rotation[m_BG_current_set];
   m_backupInclination = m_BG_inclination[m_BG_current_set];
   m_backupOffset.x = m_BG_xlatex[m_BG_current_set];
   m_backupOffset.y = m_BG_xlatey[m_BG_current_set];
   m_backupOffset.z = m_BG_xlatez[m_BG_current_set];
   m_backupScale.x = m_BG_scalex[m_BG_current_set];
   m_backupScale.y = m_BG_scaley[m_BG_current_set];
   m_backupScale.z = m_BG_scalez[m_BG_current_set];
   m_backupFOV = m_BG_FOV[m_BG_current_set];
   m_backupLightHeight = m_lightHeight;
   m_backupLightRange = m_lightRange;
   m_backupEmisionScale = m_lightEmissionScale;
   m_backupEnvEmissionScale = m_envEmissionScale;

   g_fKeepUndoRecords = fFalse;

   m_pcv->m_fScriptError = false;
   m_pcv->Compile();

   if (!m_pcv->m_fScriptError)
   {
      // set up the texture hashtable for fast access
      m_textureMap.clear();
      for (unsigned i = 0; i < m_vimage.size(); i++)
      {
         m_textureMap[m_vimage[i]->m_szInternalName] = m_vimage[i];
      }
      m_materialMap.clear();
      for (int i = 0; i < m_materials.Size(); i++)
      {
         m_materialMap[m_materials.ElementAt(i)->m_szName] = m_materials.ElementAt(i);
      }

      g_pplayer = new Player(_cameraMode);
      HRESULT hr = g_pplayer->Init(this, hwndProgressBar, hwndStatusName);
      if (!m_pcv->m_fScriptError)
      {
         float fOverrideContactScatterAngle;

         if (m_fOverridePhysics)
         {
            char tmp[256];

            m_fOverrideGravityConstant = DEFAULT_TABLE_GRAVITY;
            sprintf_s(tmp, 256, "TablePhysicsGravityConstant%d", m_fOverridePhysics - 1);
            hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideGravityConstant);
            if (hr != S_OK)
               m_fOverrideGravityConstant = DEFAULT_TABLE_GRAVITY;
            m_fOverrideGravityConstant *= GRAVITYCONST;

            m_fOverrideContactFriction = DEFAULT_TABLE_CONTACTFRICTION;
            sprintf_s(tmp, 256, "TablePhysicsContactFriction%d", m_fOverridePhysics - 1);
            hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideContactFriction);
            if (hr != S_OK)
               m_fOverrideContactFriction = DEFAULT_TABLE_CONTACTFRICTION;

			m_fOverrideElasticity = DEFAULT_TABLE_ELASTICITY;
			sprintf_s(tmp, 256, "TablePhysicsElasticity%d", m_fOverridePhysics - 1);
			hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideElasticity);
			if (hr != S_OK)
				m_fOverrideElasticity = DEFAULT_TABLE_ELASTICITY;
			
			m_fOverrideElasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;
			sprintf_s(tmp, 256, "TablePhysicsElasticityFalloff%d", m_fOverridePhysics - 1);
			hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideElasticityFalloff);
			if (hr != S_OK)
				m_fOverrideElasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;
			
			m_fOverrideScatterAngle = DEFAULT_TABLE_PFSCATTERANGLE;
			sprintf_s(tmp, 256, "TablePhysicsScatterAngle%d", m_fOverridePhysics - 1);
			hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideScatterAngle);
			if (hr != S_OK)
				m_fOverrideScatterAngle = DEFAULT_TABLE_PFSCATTERANGLE;

			fOverrideContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;
            sprintf_s(tmp, 256, "TablePhysicsContactScatterAngle%d", m_fOverridePhysics - 1);
            hr = GetRegStringAsFloat("Player", tmp, &fOverrideContactScatterAngle);
            if (hr != S_OK)
               fOverrideContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;
         }

         c_hardScatter = ANGTORAD(m_fOverridePhysics ? fOverrideContactScatterAngle : m_defaultScatter);

         const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;

         g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);

         m_pcv->SetEnabled(fFalse); // Can't edit script while playing

         g_pvp->SetEnableToolbar();

         if (!m_pcv->m_fScriptError && (hr == S_OK))
         {
            ::ShowWindow(g_pvp->m_hwndWork, SW_HIDE);
         }
         else
         {
            ::SendMessage(g_pplayer->m_hwnd, WM_CLOSE, 0, 0);
         }
      }
   }
   else
   {
      RestoreBackup();
      // restore layers
      RestoreLayers();
      g_fKeepUndoRecords = fTrue;
      m_pcv->EndSession();
      //delete g_pplayer;
      //g_pplayer = NULL;
   }

   DestroyWindow(hwndProgressDialog);

   //EnableWindow(g_pvp->m_hwndWork, fFalse); // Go modal in our main app window
}


void PinTable::StopPlaying()
{
   // Unhook script connections
   //m_pcv->m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

   m_pcv->SetEnabled(fTrue);

   // Stop all sounds
   // In case we were playing any of the main buffers
   for (int i = 0; i < m_vsound.Size(); i++)
   {
      m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
   }
   // The usual case - copied sounds
   for (int i = 0; i < m_voldsound.Size(); i++)
   {
      PinSoundCopy *ppsc = m_voldsound.ElementAt(i);
      ppsc->m_pDSBuffer->Stop();
   }
   ClearOldSounds();

   m_pcv->EndSession();
   m_textureMap.clear();
   m_materialMap.clear();

   //	EnableWindow(g_pvp->m_hwndWork, fTrue); // Disable modal state after game ends

   // This was causing the application to crash 
   // if the simulation was run without a save first.
   // But I'm not sure how to fix it... - JEP

   float inclination = m_BG_inclination[m_BG_current_set];
   float fov = m_BG_FOV[m_BG_current_set];
   float layback = m_BG_layback[m_BG_current_set];
   float xlatex = m_BG_xlatex[m_BG_current_set];
   float xlatey = m_BG_xlatey[m_BG_current_set];
   float xlatez = m_BG_xlatez[m_BG_current_set];
   float xscale = m_BG_scalex[m_BG_current_set];
   float yscale = m_BG_scaley[m_BG_current_set];
   float zscale = m_BG_scalez[m_BG_current_set];
   float lightEmissionScale = m_lightEmissionScale;
   float lightRange = m_lightRange;
   float lightHeight = m_lightHeight;
   float envEmissionScale = m_envEmissionScale;

   RestoreBackup();

   m_BG_inclination[m_BG_current_set] = inclination;
   m_BG_FOV[m_BG_current_set] = fov;
   m_BG_layback[m_BG_current_set] = layback;
   m_BG_xlatex[m_BG_current_set] = xlatex;
   m_BG_xlatey[m_BG_current_set] = xlatey;
   m_BG_xlatez[m_BG_current_set] = xlatez;
   m_BG_scalex[m_BG_current_set] = xscale;
   m_BG_scaley[m_BG_current_set] = yscale;
   m_BG_scalez[m_BG_current_set] = zscale;
   m_lightRange = lightRange;
   m_lightHeight = lightHeight;
   m_lightEmissionScale = lightEmissionScale;
   m_envEmissionScale = envEmissionScale;

   g_fKeepUndoRecords = fTrue;

   ::ShowWindow(g_pvp->m_hwndWork, SW_SHOW);
   UpdateDbgMaterial();
   UpdateDbgLight();

   BeginAutoSaveCounter();
}


void PinTable::CreateTableWindow()
{
   WNDCLASSEX wcex;
   ZeroMemory(&wcex, sizeof(WNDCLASSEX));
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;//CS_NOCLOSE | CS_OWNDC;
   wcex.lpfnWndProc = TableWndProc;
   wcex.hInstance = g_hinst;
   wcex.lpszClassName = "PinTable";
   wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TABLE));
   wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = NULL;
   //wcex.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);

   RegisterClassEx(&wcex);

   m_hwnd = ::CreateWindowEx(WS_EX_MDICHILD /*| WS_EX_OVERLAPPEDWINDOW*/, "PinTable", m_szFileName, WS_HSCROLL | WS_VSCROLL | WS_MAXIMIZE | WS_VISIBLE | WS_CHILD | WS_OVERLAPPEDWINDOW/* | WS_MAXIMIZE*/,
      20, 20, 400, 400, m_pvp->m_hwndWork, NULL, g_hinst, 0);

   BeginAutoSaveCounter();

   ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (size_t)this);
}


HRESULT PinTable::InitVBA()
{
   HRESULT hr = S_OK;
   return hr;
}

void PinTable::CloseVBA()
{
}


HRESULT PinTable::TableSave()
{
   BOOL fSaveAs = (!m_szFileName[0]);
   return Save(fSaveAs);
}


HRESULT PinTable::SaveAs()
{
   return Save(fTrue);
}


HRESULT PinTable::ApcProject_Save()
{
   BOOL fSaveAs = (!m_szFileName[0]);
   return Save(fSaveAs);
}


void PinTable::BeginAutoSaveCounter()
{
   if (g_pvp->m_autosaveTime > 0)
      ::SetTimer(m_hwnd, TIMER_ID_AUTOSAVE, g_pvp->m_autosaveTime, NULL);
}


void PinTable::EndAutoSaveCounter()
{
   ::KillTimer(m_hwnd, TIMER_ID_AUTOSAVE);
}


void PinTable::AutoSave()
{
   if ((m_sdsCurrentDirtyState <= eSaveAutosaved) || CheckPermissions(DISABLE_TABLE_SAVE))
      return;

   ::KillTimer(m_hwnd, TIMER_ID_AUTOSAVE);

   {
      LocalString ls(IDS_AUTOSAVING);
      g_pvp->SetActionCur(ls.m_szbuffer);
      g_pvp->SetCursorCur(NULL, IDC_WAIT);
   }

   FastIStorage * const pstgroot = new FastIStorage();
   pstgroot->AddRef();

   const HRESULT hr = SaveToStorage(pstgroot);

   m_undo.SetCleanPoint((SaveDirtyState)min((int)m_sdsDirtyProp, (int)eSaveAutosaved));
   m_pcv->SetClean((SaveDirtyState)min((int)m_sdsDirtyScript, (int)eSaveAutosaved));
   SetNonUndoableDirty((SaveDirtyState)min((int)m_sdsNonUndoableDirty, (int)eSaveAutosaved));

   AutoSavePackage * const pasp = new AutoSavePackage();
   pasp->pstg = pstgroot;
   pasp->tableindex = g_pvp->m_vtable.IndexOf(this);
   pasp->HwndTable = m_hwnd;

   if (hr == S_OK)
   {
      HANDLE hEvent = g_pvp->PostWorkToWorkerThread(COMPLETE_AUTOSAVE, (LPARAM)pasp);
      m_vAsyncHandles.push_back(hEvent);

      g_pvp->SetActionCur("Completing AutoSave");
   }
   else
   {
      g_pvp->SetActionCur("");
   }

   g_pvp->SetCursorCur(NULL, IDC_ARROW);
}

HRESULT PinTable::Save(BOOL fSaveAs)
{
   IStorage* pstgRoot;

   // Get file name if needed
   if (fSaveAs)
   {
      //need to get a file name
      OPENFILENAME ofn;
      ZeroMemory(&ofn, sizeof(OPENFILENAME));
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = g_hinst;
      ofn.hwndOwner = g_pvp->m_hwnd;
      // TEXT
      ofn.lpstrFilter = "Visual Pinball Tables (*.vpx)\0*.vpx\0";
      ofn.lpstrFile = m_szFileName;
      ofn.nMaxFile = _MAX_PATH;
      ofn.lpstrDefExt = "vpx";
      ofn.Flags = OFN_OVERWRITEPROMPT;

      char szInitialDir[1024];
      HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
      char szFoo[MAX_PATH];
      if (hr == S_OK)
      {
         ofn.lpstrInitialDir = szInitialDir;
      }
      else
      {
         lstrcpy(szFoo, g_pvp->m_szMyPath);
         lstrcat(szFoo, "Tables");
         ofn.lpstrInitialDir = szFoo;
      }

      int ret = GetSaveFileName(&ofn);

      // user cancelled
      if (ret == 0)
         return S_FALSE;

      strcpy_s(szInitialDir, sizeof(szInitialDir), m_szFileName);
      szInitialDir[ofn.nFileOffset] = 0;
      hr = SetRegValue("RecentDir", "LoadDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));

      {
         MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);

         STGOPTIONS stg;
         stg.usVersion = 1;
         stg.reserved = 0;
         stg.ulSectorSize = 4096;

         if (FAILED(hr = StgCreateStorageEx(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
            STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgRoot)))
         {
            LocalString ls(IDS_SAVEERROR);
            ::MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
            return hr;
         }
      }

      TitleFromFilename(m_szFileName, m_szTitle);
      SetCaption(m_szTitle);
   }
   else
   {
      char *ptr = strstr(m_szFileName, ".vpt");
      if (ptr != NULL)
      {
         strcpy_s(ptr, 5, ".vpx");
      }
      MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);

      STGOPTIONS stg;
      stg.usVersion = 1;
      stg.reserved = 0;
      stg.ulSectorSize = 4096;

      HRESULT hr;
      if (FAILED(hr = StgCreateStorageEx(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
         STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgRoot)))
      {
         LocalString ls(IDS_SAVEERROR);
         ::MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
         return hr;
      }
   }

   {
      LocalString ls(IDS_SAVING);
      g_pvp->SetActionCur(ls.m_szbuffer);
      g_pvp->SetCursorCur(NULL, IDC_WAIT);
   }

   // merge all elements for saving
   BackupLayers();

   HRESULT hr = SaveToStorage(pstgRoot);

   if (!FAILED(hr))
   {
      pstgRoot->Commit(STGC_DEFAULT);
      pstgRoot->Release();

      g_pvp->SetActionCur("");
      g_pvp->SetCursorCur(NULL, IDC_ARROW);

      m_undo.SetCleanPoint(eSaveClean);
      m_pcv->SetClean(eSaveClean);
      SetNonUndoableDirty(eSaveClean);
   }

   // restore layers
   RestoreLayers();
   return S_OK;
}

HRESULT PinTable::SaveToStorage(IStorage *pstgRoot)
{
   IStorage *pstgData, *pstgInfo;
   IStream *pstmGame, *pstmItem;

   m_savingActive = true;
   RECT rc;
   ::SendMessage(g_pvp->m_hwndStatusBar, SB_GETRECT, 2, (size_t)&rc);

   HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR)NULL,
      WS_CHILD | WS_VISIBLE, rc.left,
      rc.top,
      rc.right - rc.left, rc.bottom - rc.top,
      g_pvp->m_hwndStatusBar, (HMENU)0, g_hinst, NULL);

   ::SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

   //////////////// Begin Encryption
   HCRYPTPROV hcp;
   HCRYPTHASH hch;
   HCRYPTKEY  hkey;
   HCRYPTHASH hchkey;

   int foo;

   hcp = NULL;
   hch = NULL;
   hkey = NULL;
   hchkey = NULL;

   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

   //foo = CryptGenKey(hcp, CALG_RC2, CRYPT_EXPORTABLE, &hkey);

   foo = GetLastError();

   foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

   foo = GetLastError();

   BYTE hashval[256];
   DWORD hashlen = 256;

   foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);

   foo = GetLastError();

   // create a key hash (we have to use a second hash as deriving a key from the
   // integrity hash actually modifies it and thus it calculates the wrong hash)
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE | 0x00280000, &hkey);
   foo = GetLastError();

   ////////////// End Encryption

   int ctotalitems = (int)(m_vedit.Size() + m_vsound.Size() + m_vimage.size() + m_vfont.Size() + m_vcollection.Size());
   int csaveditems = 0;

   ::SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

   //first save our own data
   HRESULT hr;
   if (SUCCEEDED(hr = pstgRoot->CreateStorage(L"GameStg", STGM_DIRECT/*STGM_TRANSACTED*/ | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgData)))
   {
      if (SUCCEEDED(hr = pstgData->CreateStream(L"GameData", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmGame)))
      {
         if (SUCCEEDED(hr = pstgData->CreateStream(L"Version", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
         {
            ULONG writ;
            int version = CURRENT_FILE_FORMAT_VERSION;
            CryptHashData(hch, (BYTE *)&version, sizeof(version), 0);
            pstmItem->Write(&version, sizeof(version), &writ);
            pstmItem->Release();
            pstmItem = NULL;
         }

         if (SUCCEEDED(hr = pstgRoot->CreateStorage(L"TableInfo", STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgInfo)))
         {
            SaveInfo(pstgInfo, hch);

            if (SUCCEEDED(hr = pstgData->CreateStream(L"CustomInfoTags", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
            {
               SaveCustomInfo(pstgInfo, pstmItem, hch);
               pstmItem->Release();
               pstmItem = NULL;
            }

            pstgInfo->Release();
         }

         if (SUCCEEDED(hr = SaveData(pstmGame, hch, hkey)))
         {
            char szSuffix[32], szStmName[64];

            for (int i = 0; i < m_vedit.Size(); i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "GameItem");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  ULONG writ;
                  IEditable *piedit = m_vedit.ElementAt(i);
                  ItemTypeEnum type = piedit->GetItemType();
                  pstmItem->Write(&type, sizeof(int), &writ);
                  hr = piedit->SaveData(pstmItem, NULL, NULL);
                  pstmItem->Release();
                  pstmItem = NULL;
                  //if(FAILED(hr)) goto Error;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i = 0; i < m_vsound.Size(); i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Sound");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  SaveSoundToStream(m_vsound.ElementAt(i), pstmItem);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (unsigned i = 0; i < m_vimage.size(); i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Image");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vimage[i]->SaveToStream(pstmItem, this);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i = 0; i < m_vfont.Size(); i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Font");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vfont.ElementAt(i)->SaveToStream(pstmItem);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i = 0; i < m_vcollection.Size(); i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Collection");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vcollection.ElementAt(i)->SaveData(pstmItem, hch, hkey);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }
         }
         pstmGame->Release();
      }

      // Authentication block
      foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

      hashlen = 256;
      foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

      if (SUCCEEDED(hr = pstgData->CreateStream(L"MAC", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
      {
         ULONG writ;
         //int version = CURRENT_FILE_FORMAT_VERSION;
         pstmItem->Write(hashval, hashlen, &writ);
         pstmItem->Release();
         pstmItem = NULL;
         //if(FAILED(hr)) goto Error;

         //CryptExportKey(hkey, NULL, PUBLICKEYBLOB, 0, BYTE *pbData, DWORD *pdwDataLen);
      }

      foo = CryptDestroyHash(hch);

      foo = CryptDestroyHash(hchkey);

      foo = CryptDestroyKey(hkey);

      foo = CryptReleaseContext(hcp, 0);
      //////// End Authentication block

      if (SUCCEEDED(hr))
         pstgData->Commit(STGC_DEFAULT);
      else
      {
         pstgData->Revert();
         pstgRoot->Revert();
         LocalString ls(IDS_SAVEERROR);
         ::MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
      }
      pstgData->Release();
   }

   //Error:

   DestroyWindow(hwndProgressBar);
   m_savingActive = false;
   return hr;
}

HRESULT PinTable::SaveSoundToStream(PinSound *pps, IStream *pstm)
{
   ULONG writ = 0;
   int len = lstrlen(pps->m_szName);

   HRESULT hr;
   if (FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(pps->m_szName, len, &writ)))
      return hr;

   len = lstrlen(pps->m_szPath);

   if (FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(pps->m_szPath, len, &writ)))
      return hr;

   len = lstrlen(pps->m_szInternalName);

   if (FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(pps->m_szInternalName, len, &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(&pps->m_wfx, sizeof(pps->m_wfx), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(&pps->m_cdata, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(pps->m_pdata, pps->m_cdata, &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(&pps->m_iOutputTarget, sizeof(bool), &writ)))
      return hr;

   // Begin NEW_SOUND_VERSION data

   if (FAILED(hr = pstm->Write(&pps->m_iVolume, sizeof(int), &writ)))
	   return hr;
   if (FAILED(hr = pstm->Write(&pps->m_iBalance, sizeof(int), &writ)))
	   return hr;
   if (FAILED(hr = pstm->Write(&pps->m_iFade, sizeof(int), &writ)))
	   return hr;
   if (FAILED(hr = pstm->Write(&pps->m_iVolume, sizeof(int), &writ)))
	   return hr;

   return S_OK;
}


HRESULT PinTable::LoadSoundFromStream(IStream *pstm, const int LoadFileVersion)
{
   int len;
   ULONG read = 0;
   HRESULT hr = S_OK;

   if (FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
      return hr;

   PinSound * const pps = new PinSound();
   if(FAILED(hr = pstm->Read(pps->m_szName, len, &read)))
   {
       delete pps;
       return hr;
   }
   pps->m_szName[len] = 0;

   if (FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
   {
       delete pps;
       return hr;
   }

   if (FAILED(hr = pstm->Read(pps->m_szPath, len, &read)))
   {
       delete pps;
       return hr;
   }

   pps->m_szPath[len] = 0;

   if (FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
   {
       delete pps;
       return hr;
   }

   if (FAILED(hr = pstm->Read(pps->m_szInternalName, len, &read)))
   {
       delete pps;
       return hr;
   }

   pps->m_szInternalName[len] = 0;

   if (FAILED(hr = pstm->Read(&pps->m_wfx, sizeof(pps->m_wfx), &read)))
   {
       delete pps;
       return hr;
   }

   if (FAILED(hr = pstm->Read(&pps->m_cdata, sizeof(int), &read)))
   {
       delete pps;
       return hr;
   }

   pps->m_pdata = new char[pps->m_cdata];

   //LZWReader lzwreader(pstm, (int *)pps->m_pdata, pps->m_cdata, 1, pps->m_cdata);

   //lzwreader.Decoder();

   if (FAILED(hr = pstm->Read(pps->m_pdata, pps->m_cdata, &read)))
   {
      delete pps;
      return hr;
   }

   if (LoadFileVersion >= NEW_SOUND_FORMAT_VERSION)
   {
	   if (FAILED(hr = pstm->Read(&pps->m_iOutputTarget, sizeof(char), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_iVolume, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_iBalance, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_iFade, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_iVolume, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
   }
   else
   {
	   bool bToBackglassOutput;

	   if (FAILED(hr = pstm->Read(&bToBackglassOutput, sizeof(bool), &read)))
	   {
		   delete pps;
		   return hr;
	   }

	   pps->m_iOutputTarget = bToBackglassOutput ? SNDOUT_BACKGLASS : SNDOUT_TABLE;	
   }

   if (FAILED(hr = pps->GetPinDirectSound()->CreateDirectFromNative(pps)))
   {
      delete pps;
      return hr;
   }

   m_vsound.AddElement(pps);
   return S_OK;
}


HRESULT PinTable::WriteInfoValue(IStorage* pstg, WCHAR *wzName, char *szValue, HCRYPTHASH hcrypthash)
{
   HRESULT hr;
   IStream *pstm;

   if (szValue && SUCCEEDED(hr = pstg->CreateStream(wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
   {
      ULONG writ;
      BiffWriter bw(pstm, hcrypthash, NULL);

      const int len = lstrlen(szValue);
      WCHAR *wzT = new WCHAR[len + 1];
      MultiByteToWideChar(CP_ACP, 0, szValue, -1, wzT, len + 1);

      bw.WriteBytes(wzT, len*(int)sizeof(WCHAR), &writ);
      delete[] wzT;
      pstm->Release();
      pstm = NULL;
   }

   return S_OK;
}


HRESULT PinTable::SaveInfo(IStorage* pstg, HCRYPTHASH hcrypthash)
{
   WriteInfoValue(pstg, L"TableName", m_szTableName, hcrypthash);
   WriteInfoValue(pstg, L"AuthorName", m_szAuthor, hcrypthash);
   WriteInfoValue(pstg, L"TableVersion", m_szVersion, hcrypthash);
   WriteInfoValue(pstg, L"ReleaseDate", m_szReleaseDate, hcrypthash);
   WriteInfoValue(pstg, L"AuthorEmail", m_szAuthorEMail, hcrypthash);
   WriteInfoValue(pstg, L"AuthorWebSite", m_szWebSite, hcrypthash);
   WriteInfoValue(pstg, L"TableBlurb", m_szBlurb, hcrypthash);
   WriteInfoValue(pstg, L"TableDescription", m_szDescription, hcrypthash);
   WriteInfoValue(pstg, L"TableRules", m_szRules, hcrypthash);

   Texture *pin = GetImage(m_szScreenShot);
   if (pin != NULL && pin->m_ppb != NULL)
   {
      IStream *pstm;
      HRESULT hr;
      ULONG writ;

      if (SUCCEEDED(hr = pstg->CreateStream(L"Screenshot", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
      {
         BiffWriter bw(pstm, hcrypthash, NULL);
         bw.WriteBytes(pin->m_ppb->m_pdata, pin->m_ppb->m_cdata, &writ);
         pstm->Release();
         pstm = NULL;
      }
   }

   pstg->Commit(STGC_DEFAULT);

   return S_OK;
}


HRESULT PinTable::SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash)
{
   BiffWriter bw(pstmTags, hcrypthash, NULL);

   for (int i = 0; i < m_vCustomInfoTag.Size(); i++)
   {
      bw.WriteString(FID(CUST), m_vCustomInfoTag.ElementAt(i));
   }

   bw.WriteTag(FID(ENDB));

   for (int i = 0; i < m_vCustomInfoTag.Size(); i++)
   {
      char *szName = m_vCustomInfoTag.ElementAt(i);
      int len = lstrlen(szName);
      WCHAR *wzName = new WCHAR[len + 1];
      MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, len + 1);

      WriteInfoValue(pstg, wzName, m_vCustomInfoContent.ElementAt(i), hcrypthash);

      delete[] wzName;
   }

   pstg->Commit(STGC_DEFAULT);

   return S_OK;
}


HRESULT PinTable::ReadInfoValue(IStorage* pstg, WCHAR *wzName, char **pszValue, HCRYPTHASH hcrypthash)
{
   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(wzName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);

      const int len = ss.cbSize.LowPart / (DWORD)sizeof(WCHAR);
      WCHAR *wzT = new WCHAR[len + 1];
      *pszValue = new char[len + 1];

      ULONG read;
      BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
      br.ReadBytes(wzT, ss.cbSize.LowPart, &read);
      wzT[len] = L'\0';

      WideCharToMultiByte(CP_ACP, 0, wzT, -1, *pszValue, len + 1, NULL, NULL);

      //delete br;
      //pstm->Read(*pszValue, ss.cbSize.LowPart, &read);

      delete[] wzT;
      pstm->Release();
   }

   return S_OK;
}


HRESULT PinTable::LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version)
{
   ReadInfoValue(pstg, L"TableName", &m_szTableName, hcrypthash);
   ReadInfoValue(pstg, L"AuthorName", &m_szAuthor, hcrypthash);
   ReadInfoValue(pstg, L"TableVersion", &m_szVersion, hcrypthash);
   ReadInfoValue(pstg, L"ReleaseDate", &m_szReleaseDate, hcrypthash);
   ReadInfoValue(pstg, L"AuthorEmail", &m_szAuthorEMail, hcrypthash);
   ReadInfoValue(pstg, L"AuthorWebSite", &m_szWebSite, hcrypthash);
   ReadInfoValue(pstg, L"TableBlurb", &m_szBlurb, hcrypthash);
   ReadInfoValue(pstg, L"TableDescription", &m_szDescription, hcrypthash);
   ReadInfoValue(pstg, L"TableRules", &m_szRules, hcrypthash);

   // Check pointer.
   if (m_szVersion != NULL)
   {
      // Write the version to the registry.  This will be read later by the front end.
      SetRegValue("Version", m_szTableName, REG_SZ, m_szVersion, lstrlen(m_szVersion));
   }

   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(L"Screenshot", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);
      //char *pdata = new char[ss.cbSize.LowPart];
      m_pbTempScreenshot = new PinBinary();

      m_pbTempScreenshot->m_cdata = ss.cbSize.LowPart;
      m_pbTempScreenshot->m_pdata = new char[m_pbTempScreenshot->m_cdata];

      //m_pbTempScreenshot->LoadFromStream(pstm, version);

      ULONG read;
      BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
      br.ReadBytes(m_pbTempScreenshot->m_pdata, m_pbTempScreenshot->m_cdata, &read);

      //delete pdata;

      pstm->Release();
   }

   return S_OK;
}

HRESULT PinTable::LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version)
{
   BiffReader br(pstmTags, this, NULL, version, hcrypthash, NULL);
   br.Load();

   for (int i = 0; i < m_vCustomInfoTag.Size(); i++)
   {
      char *szName = m_vCustomInfoTag.ElementAt(i);
      char *szValue;
      const int len = lstrlen(szName);
      WCHAR *wzName = new WCHAR[len + 1];
      MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, len + 1);

      ReadInfoValue(pstg, wzName, &szValue, hcrypthash);
      m_vCustomInfoContent.AddElement(szValue);

      delete[] wzName;
   }

   return S_OK;
}

HRESULT PinTable::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteFloat(FID(LEFT), m_left);
   bw.WriteFloat(FID(TOPX), m_top);
   bw.WriteFloat(FID(RGHT), m_right);
   bw.WriteFloat(FID(BOTM), m_bottom);

   bw.WriteFloat(FID(ROTA), m_BG_rotation[0]);
   bw.WriteFloat(FID(INCL), m_BG_inclination[0]);
   bw.WriteFloat(FID(LAYB), m_BG_layback[0]);
   bw.WriteFloat(FID(FOVX), m_BG_FOV[0]);
   bw.WriteFloat(FID(XLTX), m_BG_xlatex[0]);
   bw.WriteFloat(FID(XLTY), m_BG_xlatey[0]);
   bw.WriteFloat(FID(XLTZ), m_BG_xlatez[0]);
   bw.WriteFloat(FID(SCLX), m_BG_scalex[0]);
   bw.WriteFloat(FID(SCLY), m_BG_scaley[0]);
   bw.WriteFloat(FID(SCLZ), m_BG_scalez[0]);

   bw.WriteBool(FID(EFSS), m_BG_enable_FSS);

   bw.WriteFloat(FID(ROTF), m_BG_rotation[1]);
   bw.WriteFloat(FID(INCF), m_BG_inclination[1]);
   bw.WriteFloat(FID(LAYF), m_BG_layback[1]);
   bw.WriteFloat(FID(FOVF), m_BG_FOV[1]);
   bw.WriteFloat(FID(XLFX), m_BG_xlatex[1]);
   bw.WriteFloat(FID(XLFY), m_BG_xlatey[1]);
   bw.WriteFloat(FID(XLFZ), m_BG_xlatez[1]);
   bw.WriteFloat(FID(SCFX), m_BG_scalex[1]);
   bw.WriteFloat(FID(SCFY), m_BG_scaley[1]);
   bw.WriteFloat(FID(SCFZ), m_BG_scalez[1]);

   bw.WriteFloat(FID(ROFS), m_BG_rotation[2]);
   bw.WriteFloat(FID(INFS), m_BG_inclination[2]);
   bw.WriteFloat(FID(LAFS), m_BG_layback[2]);
   bw.WriteFloat(FID(FOFS), m_BG_FOV[2]);
   bw.WriteFloat(FID(XLXS), m_BG_xlatex[2]);
   bw.WriteFloat(FID(XLYS), m_BG_xlatey[2]);
   bw.WriteFloat(FID(XLZS), m_BG_xlatez[2]);
   bw.WriteFloat(FID(SCXS), m_BG_scalex[2]);
   bw.WriteFloat(FID(SCYS), m_BG_scaley[2]);
   bw.WriteFloat(FID(SCZS), m_BG_scalez[2]);

   bw.WriteInt(FID(ORRP), m_fOverridePhysics);
   bw.WriteFloat(FID(GAVT), m_Gravity);
   bw.WriteFloat(FID(FRCT), m_friction);
   bw.WriteFloat(FID(ELAS), m_elasticity);
   bw.WriteFloat(FID(ELFA), m_elasticityFalloff);
   bw.WriteFloat(FID(PFSC), m_scatter);
   bw.WriteFloat(FID(SCAT), m_defaultScatter);
   bw.WriteFloat(FID(NDGT), m_nudgeTime);
   bw.WriteInt(FID(MPGC), m_plungerNormalize);
   bw.WriteBool(FID(MPDF), m_plungerFilter);
   bw.WriteInt(FID(PHML), m_PhysicsMaxLoops);

   //bw.WriteFloat(FID(IMTCOL), m_transcolor);

   bw.WriteBool(FID(REEL), m_fRenderEMReels);
   bw.WriteBool(FID(DECL), m_fRenderDecals);

   bw.WriteFloat(FID(OFFX), m_offset.x);
   bw.WriteFloat(FID(OFFY), m_offset.y);

   bw.WriteFloat(FID(ZOOM), m_zoom);

   bw.WriteFloat(FID(SLPX), m_angletiltMax);
   bw.WriteFloat(FID(SLOP), m_angletiltMin);

   bw.WriteFloat(FID(MAXSEP), m_3DmaxSeparation);
   bw.WriteFloat(FID(ZPD), m_3DZPD);
   bw.WriteFloat(FID(STO), m_3DOffset);
   bw.WriteBool(FID(OGST), m_overwriteGlobalStereo3D);

   bw.WriteString(FID(IMAG), m_szImage);
   bw.WriteString(FID(BIMG), m_BG_szImage[0]);
   bw.WriteString(FID(BIMF), m_BG_szImage[1]);
   bw.WriteString(FID(BIMS), m_BG_szImage[2]);
   bw.WriteBool(FID(BIMN), m_ImageBackdropNightDay);
   bw.WriteString(FID(IMCG), m_szImageColorGrade);
   bw.WriteString(FID(BLIM), m_szBallImage);
   bw.WriteString(FID(BLIF), m_szBallImageFront);
   bw.WriteString(FID(EIMG), m_szEnvImage);

   bw.WriteString(FID(SSHT), m_szScreenShot);

   bw.WriteBool(FID(FBCK), m_fBackdrop);

   bw.WriteFloat(FID(GLAS), m_glassheight);
   bw.WriteFloat(FID(TBLH), m_tableheight);

   bw.WriteString(FID(PLMA), m_szPlayfieldMaterial);
   bw.WriteInt(FID(BCLR), m_colorbackdrop);

   bw.WriteFloat(FID(TDFT), m_globalDifficulty);

   bw.WriteInt(FID(LZAM), m_lightAmbient);
   bw.WriteInt(FID(LZDI), m_Light[0].emission);
   bw.WriteFloat(FID(LZHI), m_lightHeight);
   bw.WriteFloat(FID(LZRA), m_lightRange);
   bw.WriteFloat(FID(LIES), m_lightEmissionScale);
   bw.WriteFloat(FID(ENES), m_envEmissionScale);
   bw.WriteFloat(FID(GLES), m_globalEmissionScale);
   bw.WriteFloat(FID(AOSC), m_AOScale);

   bw.WriteFloat(FID(SVOL), m_TableSoundVolume);
   bw.WriteFloat(FID(MVOL), m_TableMusicVolume);

   bw.WriteInt(FID(AVSY), m_TableAdaptiveVSync);

   bw.WriteInt(FID(BREF), m_useReflectionForBalls);
   bw.WriteInt(FID(PLST), quantizeUnsigned<8>(m_playfieldReflectionStrength));
   bw.WriteInt(FID(BTRA), m_useTrailForBalls);
   bw.WriteBool(FID(BDMO), m_BallDecalMode);
   bw.WriteFloat(FID(BPRS), m_ballPlayfieldReflectionStrength);
   bw.WriteFloat(FID(DBIS), m_defaultBulbIntensityScaleOnBall);
   bw.WriteInt(FID(BTST), quantizeUnsigned<8>(m_ballTrailStrength));
   bw.WriteInt(FID(ARAC), m_userDetailLevel);
   bw.WriteBool(FID(OGAC), m_overwriteGlobalDetailLevel);
   bw.WriteBool(FID(OGDN), m_overwriteGlobalDayNight);
   bw.WriteBool(FID(GDAC), m_fGrid);
   bw.WriteBool(FID(REOP), m_fReflectElementsOnPlayfield);

   bw.WriteInt(FID(UAAL), m_useAA);
   bw.WriteInt(FID(UFXA), m_useFXAA);
   bw.WriteInt(FID(UAOC), m_useAO);
   bw.WriteFloat(FID(BLST), m_bloom_strength);

   bw.WriteInt(FID(MASI), m_materials.Size());
   if (m_materials.Size() > 0)
   {
      SaveMaterial * const mats = (SaveMaterial*)malloc(sizeof(SaveMaterial)*m_materials.Size());
      for (int i = 0; i < m_materials.Size(); i++)
      {
         const Material* const m = m_materials.ElementAt(i);
         mats[i].cBase = m->m_cBase;
         mats[i].cGlossy = m->m_cGlossy;
         mats[i].cClearcoat = m->m_cClearcoat;
         mats[i].fWrapLighting = m->m_fWrapLighting;
         mats[i].fRoughness = m->m_fRoughness;
         mats[i].fGlossyImageLerp = 255 - quantizeUnsigned<8>(clamp(m->m_fGlossyImageLerp, 0.f, 1.f)); // '255 -' to be compatible with previous table versions
         mats[i].fThickness = quantizeUnsigned<8>(clamp(m->m_fThickness, 0.05f, 1.f)); // clamp with 0.05f to be compatible with previous table versions
         mats[i].fEdge = m->m_fEdge;
         mats[i].fOpacity = m->m_fOpacity;
         mats[i].bIsMetal = m->m_bIsMetal;
         mats[i].bOpacityActive_fEdgeAlpha = m->m_bOpacityActive ? 1 : 0;
         mats[i].bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(clamp(m->m_fEdgeAlpha, 0.f, 1.f)) << 1;
         strcpy_s(mats[i].szName, m->m_szName);
      }
      bw.WriteStruct(FID(MATE), mats, (int)sizeof(SaveMaterial)*m_materials.Size());
      SavePhysicsMaterial * const phymats = (SavePhysicsMaterial*)malloc(sizeof(SavePhysicsMaterial)*m_materials.Size());
      for ( int i = 0; i < m_materials.Size(); i++ )
      {
          const Material* const m = m_materials.ElementAt( i );
          strcpy_s( phymats[i].szName, m->m_szName );
          phymats[i].fElasticity = m->m_fElasticity;
          phymats[i].fElasticityFallOff = m->m_fElasticityFalloff;
          phymats[i].fFriction = m->m_fFriction;
          phymats[i].fScatterAngle = m->m_fScatterAngle;
      }
      bw.WriteStruct( FID( PHMA ), phymats, (int)sizeof( SavePhysicsMaterial )*m_materials.Size() );
      free(mats);
      free(phymats);
   }
   // HACK!!!! - Don't save special values when copying for undo.  For instance, don't reset the code.
   // Someday save these values into there own stream, used only when saving to file.

   if (hcrypthash != 0)
   {
      bw.WriteInt(FID(SEDT), m_vedit.Size());
      bw.WriteInt(FID(SSND), m_vsound.Size());
      bw.WriteInt(FID(SIMG), (int)m_vimage.size());
      bw.WriteInt(FID(SFNT), m_vfont.Size());
      bw.WriteInt(FID(SCOL), m_vcollection.Size());

      bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

      bw.WriteStruct(FID(CCUS), m_rgcolorcustom, sizeof(COLORREF) * 16);

      bw.WriteStruct(FID(SECB), &m_protectionData, sizeof(_protectionData));

      // save the script source code
      bw.WriteTag(FID(CODE));
      // if the script is protected then we pass in the proper cyptokey into the code savestream
      m_pcv->SaveToStream(pstm, hcrypthash, CheckPermissions(DISABLE_SCRIPT_EDITING) ? hcryptkey : NULL);
   }

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT PinTable::LoadGameFromFilename(char *szFileName)
{
   IStorage* pstgRoot;
   HRESULT hr = S_OK;

   if (szFileName == NULL)
   {
      ShowError("Empty File Name String!");
      return S_FALSE;
   }

   strcpy_s(m_szFileName, sizeof(m_szFileName), szFileName);
   {
      MAKE_WIDEPTR_FROMANSI(wszCodeFile, szFileName);
      if (FAILED(hr = StgOpenStorage(wszCodeFile, NULL, STGM_TRANSACTED | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
      {
         // TEXT
         char msg[256];
         sprintf_s(msg, "Error loading %s", m_szFileName);
         ::MessageBox(g_pvp->m_hwnd, msg, "Load Error", 0);
         return hr;
      }
   }

   return LoadGameFromStorage(pstgRoot);
}

HRESULT PinTable::LoadGameFromStorage(IStorage *pstgRoot)
{
   IStorage *pstgData, *pstgInfo;
   IStream *pstmGame, *pstmItem, *pstmVersion;
   HRESULT hr = S_OK;

   int ctotalitems;
   int cloadeditems;

   RECT rc;
   ::SendMessage(g_pvp->m_hwndStatusBar, SB_GETRECT, 2, (size_t)&rc);

   HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR)NULL,
      WS_CHILD | WS_VISIBLE, rc.left,
      rc.top,
      rc.right - rc.left, rc.bottom - rc.top,
      g_pvp->m_hwndStatusBar, (HMENU)0, g_hinst, NULL);

   ::SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

   LocalString ls(IDS_LOADING);
   g_pvp->SetActionCur(ls.m_szbuffer);
   g_pvp->SetCursorCur(NULL, IDC_WAIT);

   HCRYPTPROV hcp = NULL;
   HCRYPTHASH hch = NULL;
   HCRYPTHASH hchkey = NULL;
   HCRYPTKEY  hkey = NULL;

   ///////// Begin MAC
   int foo;

   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

   foo = GetLastError();

   foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

   foo = GetLastError();

   BYTE hashval[256];
   DWORD hashlen = 256;

   foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);

   foo = GetLastError();

   // create a key hash (we have to use a second hash as deriving a key from the
   // integrity hash actually modifies it and thus it calculates the wrong hash)
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   // We need to figure out the file verison before we can create the key

   ////////////// End MAC

   int loadfileversion = CURRENT_FILE_FORMAT_VERSION;

   //load our stuff first
   if (SUCCEEDED(hr = pstgRoot->OpenStorage(L"GameStg", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgData)))
   {
      if (SUCCEEDED(hr = pstgData->OpenStream(L"GameData", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmGame)))
      {
         int csubobj = 0;
         int csounds = 0;
         int ctextures = 0;
         int cfonts = 0;
         int ccollection = 0;

         if (SUCCEEDED(hr = pstgData->OpenStream(L"Version", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
         {
            ULONG read;
            hr = pstmVersion->Read(&loadfileversion, sizeof(int), &read);
            CryptHashData(hch, (BYTE *)&loadfileversion, sizeof(int), 0);
            pstmVersion->Release();
            if (loadfileversion > CURRENT_FILE_FORMAT_VERSION)
            {
               char errorMsg[MAX_PATH] = { 0 };
               sprintf_s(errorMsg, "This table was saved with version %i.%02i and is newer than the supported version %i.%02i! You might get problems loading/playing it!", loadfileversion / 100, loadfileversion % 100, CURRENT_FILE_FORMAT_VERSION / 100, CURRENT_FILE_FORMAT_VERSION%100);
               ShowError(errorMsg);
/*
               pstgRoot->Release();
               pstmGame->Release();
               pstgData->Release();
               DestroyWindow(hwndProgressBar);
               g_pvp->SetCursorCur(NULL, IDC_ARROW);
               return -1;
*/
            }

            // Create a block cipher session key based on the hash of the password.
            CryptDeriveKey(hcp, CALG_RC2, hchkey, (loadfileversion == 600) ? CRYPT_EXPORTABLE : (CRYPT_EXPORTABLE | 0x00280000), &hkey);
         }

         if (SUCCEEDED(hr = pstgRoot->OpenStorage(L"TableInfo", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgInfo)))
         {
            LoadInfo(pstgInfo, hch, loadfileversion);
            if (SUCCEEDED(hr = pstgData->OpenStream(L"CustomInfoTags", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
            {
               LoadCustomInfo(pstgInfo, pstmItem, hch, loadfileversion);
               pstmItem->Release();
               pstmItem = NULL;
            }
            pstgInfo->Release();
         }

         if (SUCCEEDED(hr = LoadData(pstmGame, csubobj, csounds, ctextures, cfonts, ccollection, loadfileversion, hch, hkey)))
         {

            ctotalitems = csubobj + csounds + ctextures + cfonts;
            cloadeditems = 0;
            ::SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

            for (int i = 0; i < csubobj; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "GameItem");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  ULONG read;
                  ItemTypeEnum type;
                  pstmItem->Read(&type, sizeof(int), &read);

                  IEditable *piedit = EditableRegistry::Create(type);

                  //AddSpriteProjItem();
                  int id = 0; // VBA id for this item
                  hr = piedit->InitLoad(pstmItem, this, &id, loadfileversion, (loadfileversion < 1000) ? hch : NULL, (loadfileversion < 1000) ? hkey : NULL); // 1000 (VP10 beta) removed the encryption
                  piedit->InitVBA(fFalse, id, NULL);
                  pstmItem->Release();
                  pstmItem = NULL;
                  if (FAILED(hr)) break;

                  m_vedit.AddElement(piedit);

                  //hr = piedit->InitPostLoad();
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for (int i = 0; i < csounds; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Sound");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  LoadSoundFromStream(pstmItem,loadfileversion);
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for (int i = 0; i < ctextures; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Image");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  LoadImageFromStream(pstmItem, loadfileversion);
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for (int i = 0; i < cfonts; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Font");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  PinFont *ppf;
                  ppf = new PinFont();
                  ppf->LoadFromStream(pstmItem, loadfileversion);
                  m_vfont.AddElement(ppf);
                  ppf->Register();
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for (int i = 0; i < ccollection; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Collection");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  CComObject<Collection> *pcol;
                  CComObject<Collection>::CreateInstance(&pcol);
                  pcol->AddRef();
                  pcol->LoadData(pstmItem, this, loadfileversion, hch, hkey);
                  m_vcollection.AddElement(pcol);
                  m_pcv->AddItem((IScriptable *)pcol, fFalse);
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for (int i = 0; i < m_vedit.Size(); i++)
            {
               IEditable * const piedit = m_vedit.ElementAt(i);
               piedit->InitPostLoad();
            }
         }
         pstmGame->Release();

         // Authentication block

         if (loadfileversion > 40)
         {
            if (SUCCEEDED(hr = pstgData->OpenStream(L"MAC", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
            {
               BYTE hashvalOld[256];
               //DWORD hashlenOld = 256;
               ULONG read;
               hr = pstmVersion->Read(&hashvalOld, HASHLENGTH, &read);

               foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

               hashlen = 256;
               foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

               foo = CryptDestroyHash(hch);

               foo = CryptDestroyHash(hchkey);

               foo = CryptDestroyKey(hkey);

               foo = CryptReleaseContext(hcp, 0);

               for (int i = 0; i < HASHLENGTH; i++)
               {
                  if (hashval[i] != hashvalOld[i])
                  {
                     hr = E_ACCESSDENIED;
                  }
               }
            }
            else
            {
               // Error
               hr = E_ACCESSDENIED;
            }
         }

         if (loadfileversion < 1030) // the m_fGlossyImageLerp part was included first with 10.3, so set all previously saved materials to the old default
             for (int i = 0; i < m_materials.size(); ++i)
                 m_materials.ElementAt(i)->m_fGlossyImageLerp = 1.f;

         if (loadfileversion < 1040) // the m_fThickness part was included first with 10.4, so set all previously saved materials to the old default
             for (int i = 0; i < m_materials.size(); ++i)
                 m_materials.ElementAt(i)->m_fThickness = 0.05f;

         //////// End Authentication block
      }
      pstgData->Release();
   }

   if (m_pbTempScreenshot) // For some reason, no image picked up the screenshot.  Not good; but we'll dump it to make sure it gets cleaned up
   {
      delete m_pbTempScreenshot;
      m_pbTempScreenshot = NULL;
   }

   DestroyWindow(hwndProgressBar);
   //DestroyWindow(hwndProgressDialog);

   pstgRoot->Release();

   g_pvp->SetActionCur("");

   for (int t = 0; t < 8; t++) m_layer[t].Empty();

   // copy all elements into their layers
   for (int i = 0; i < 8; i++)
   {
      for (int t = 0; t < m_vedit.Size(); t++)
      {
         IEditable *piedit = m_vedit.ElementAt(t);
         if (piedit->GetISelect()->layerIndex == i)
         {
            m_layer[i].AddElement(piedit);
         }
      }
   }

   return hr;
}

void PinTable::SetLoadDefaults()
{
   for (unsigned int i = 0; i < NUM_BG_SETS; ++i)
      m_BG_szImage[i][0] = 0;
   m_szImageColorGrade[0] = 0;
   m_szBallImage[0] = 0;
   m_szBallImageFront[0] = 0;
   m_ImageBackdropNightDay = false;
   m_szEnvImage[0] = 0;

   m_szScreenShot[0] = 0;

   m_colorbackdrop = RGB(0x62, 0x6E, 0x8E);

   m_lightAmbient = RGB((int)(0.1 * 255), (int)(0.1 * 255), (int)(0.1 * 255));
   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      m_Light[i].emission = RGB((int)(0.4 * 255), (int)(0.4 * 255), (int)(0.4 * 255));
      m_Light[i].pos = Vertex3Ds(0.f, 0.f, 400.0f);
   }

   m_lightHeight = 1000.0f;
   m_lightRange = 3000.0f;
   m_lightEmissionScale = 1000000.0f;
   m_globalEmissionScale = 1.0f;
   m_envEmissionScale = 10.0f;
   m_AOScale = 1.75f;
   m_angletiltMax = 726.0f;
   m_angletiltMin = 4.5f;

   m_useReflectionForBalls = -1;
   m_playfieldReflectionStrength = 0.2f;
   m_fReflectElementsOnPlayfield = false;

   m_useTrailForBalls = -1;
   m_ballTrailStrength = 0.4f;
   m_ballPlayfieldReflectionStrength = 1.f;

   m_useAA = -1;
   m_useFXAA = -1;
   m_useAO = -1;

   m_bloom_strength = 1.0f;

   m_TableSoundVolume = 1.0f;
   m_TableMusicVolume = 1.0f;

   m_BallDecalMode = false;

   m_TableAdaptiveVSync = -1;
}

HRESULT PinTable::LoadData(IStream* pstm, int& csubobj, int& csounds, int& ctextures, int& cfonts, int& ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetLoadDefaults();

   int rgi[6] = { 0, 0, 0, 0, 0, 0 };

   BiffReader br(pstm, this, rgi, version, hcrypthash, hcryptkey);

   br.Load();

   csubobj = rgi[1];
   csounds = rgi[2];
   ctextures = rgi[3];
   cfonts = rgi[4];
   ccollection = rgi[5];

   return S_OK;
}

BOOL PinTable::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[0]);
   }
   else if (id == FID(LEFT))
   {
      pbr->GetFloat(&m_left);
   }
   else if (id == FID(TOPX))
   {
      pbr->GetFloat(&m_top);
   }
   else if (id == FID(RGHT))
   {
      pbr->GetFloat(&m_right);
   }
   else if (id == FID(BOTM))
   {
      pbr->GetFloat(&m_bottom);
   }
   else if (id == FID(ROTA))
   {
      pbr->GetFloat(&m_BG_rotation[BG_DESKTOP]);
   }
   else if (id == FID(LAYB))
   {
      pbr->GetFloat(&m_BG_layback[BG_DESKTOP]);
   }
   else if (id == FID(INCL))
   {
      pbr->GetFloat(&m_BG_inclination[BG_DESKTOP]);
   }
   else if (id == FID(FOVX))
   {
      pbr->GetFloat(&m_BG_FOV[BG_DESKTOP]);
   }
   else if (id == FID(SCLX))
   {
      pbr->GetFloat(&m_BG_scalex[BG_DESKTOP]);
   }
   else if (id == FID(SCLY))
   {
      pbr->GetFloat(&m_BG_scaley[BG_DESKTOP]);
   }
   else if (id == FID(SCLZ))
   {
      pbr->GetFloat(&m_BG_scalez[BG_DESKTOP]);
   }
   else if (id == FID(XLTX))
   {
      pbr->GetFloat(&m_BG_xlatex[BG_DESKTOP]);
   }
   else if (id == FID(XLTY))
   {
      pbr->GetFloat(&m_BG_xlatey[BG_DESKTOP]);
   }
   else if (id == FID(XLTZ))
   {
      pbr->GetFloat(&m_BG_xlatez[BG_DESKTOP]);
   }
   else if (id == FID(ROTF))
   {
      pbr->GetFloat(&m_BG_rotation[BG_FULLSCREEN]);
   }
   else if (id == FID(LAYF))
   {
      pbr->GetFloat(&m_BG_layback[BG_FULLSCREEN]);
   }
   else if (id == FID(INCF))
   {
      pbr->GetFloat(&m_BG_inclination[BG_FULLSCREEN]);
   }
   else if (id == FID(FOVF))
   {
      pbr->GetFloat(&m_BG_FOV[BG_FULLSCREEN]);
   }
   else if (id == FID(SCFX))
   {
      pbr->GetFloat(&m_BG_scalex[BG_FULLSCREEN]);
   }
   else if (id == FID(SCFY))
   {
      pbr->GetFloat(&m_BG_scaley[BG_FULLSCREEN]);
   }
   else if (id == FID(SCFZ))
   {
      pbr->GetFloat(&m_BG_scalez[BG_FULLSCREEN]);
   }
   else if (id == FID(XLFX))
   {
      pbr->GetFloat(&m_BG_xlatex[BG_FULLSCREEN]);
   }
   else if (id == FID(XLFY))
   {
      pbr->GetFloat(&m_BG_xlatey[BG_FULLSCREEN]);
   }
   else if (id == FID(XLFZ))
   {
      pbr->GetFloat(&m_BG_xlatez[BG_FULLSCREEN]);
   }
   else if (id == FID(ROFS))
   {
      pbr->GetFloat(&m_BG_rotation[BG_FSS]);
   }
   else if (id == FID(LAFS))
   {
      pbr->GetFloat(&m_BG_layback[BG_FSS]);
   }
   else if (id == FID(INFS))
   {
      pbr->GetFloat(&m_BG_inclination[BG_FSS]);
   }
   else if (id == FID(FOFS))
   {
      pbr->GetFloat(&m_BG_FOV[BG_FSS]);
   }
   else if (id == FID(SCXS))
   {
      pbr->GetFloat(&m_BG_scalex[BG_FSS]);
   }
   else if (id == FID(SCYS))
   {
      pbr->GetFloat(&m_BG_scaley[BG_FSS]);
   }
   else if (id == FID(SCZS))
   {
      pbr->GetFloat(&m_BG_scalez[BG_FSS]);
   }
   else if (id == FID(XLXS))
   {
      pbr->GetFloat(&m_BG_xlatex[BG_FSS]);
   }
   else if (id == FID(XLYS))
   {
      pbr->GetFloat(&m_BG_xlatey[BG_FSS]);
   }
   else if (id == FID(XLZS))
   {
      pbr->GetFloat(&m_BG_xlatez[BG_FSS]);
   }
   else if (id == FID(EFSS))
   {
      pbr->GetBool(&m_BG_enable_FSS);
      if(m_BG_enable_FSS)
         m_BG_current_set = BG_FSS; //!! FSS
   }
#if 0
   else if (id == FID(VERS))
   {
      pbr->GetString(szVersion);
   }
#endif
   else if (id == FID(ORRP))
   {
      pbr->GetInt(&m_fOverridePhysics);
   }
   else if (id == FID(GAVT))
   {
      pbr->GetFloat(&m_Gravity);
   }
   else if (id == FID(FRCT))
   {
      pbr->GetFloat(&m_friction);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_elasticity);
   }
   else if (id == FID(ELFA))
   {
      pbr->GetFloat(&m_elasticityFalloff);
   }
   else if (id == FID(PFSC))
   {
      pbr->GetFloat(&m_scatter);
   }
   else if (id == FID(SCAT))
   {
      pbr->GetFloat(&m_defaultScatter);
   }
   else if (id == FID(NDGT))
   {
      pbr->GetFloat(&m_nudgeTime);
   }
   else if (id == FID(MPGC))
   {
      pbr->GetInt(&m_plungerNormalize);
      /*const HRESULT hr =*/ GetRegInt("Player", "PlungerNormalize", &m_plungerNormalize);
   }
   else if (id == FID(MPDF))
   {
      int tmp;
      pbr->GetBool(&tmp);
      /*const HRESULT hr =*/ GetRegInt("Player", "PlungerFilter", &tmp);
      m_plungerFilter = (tmp != 0);
   }
   else if (id == FID(PHML))
   {
      pbr->GetInt(&m_PhysicsMaxLoops);
      if (m_PhysicsMaxLoops == 0xFFFFFFFF)
         /*const HRESULT hr =*/ GetRegInt("Player", "PhysicsMaxLoops", (int*)&m_PhysicsMaxLoops);
   }
   else if (id == FID(DECL))
   {
      pbr->GetBool(&m_fRenderDecals);
   }
   else if (id == FID(REEL))
   {
      pbr->GetBool(&m_fRenderEMReels);
   }
   else if (id == FID(OFFX))
   {
      pbr->GetFloat(&m_offset.x);
   }
   else if (id == FID(OFFY))
   {
      pbr->GetFloat(&m_offset.y);
   }
   else if (id == FID(ZOOM))
   {
      pbr->GetFloat(&m_zoom);
   }
   else if (id == FID(MAXSEP))
   {
      pbr->GetFloat(&m_3DmaxSeparation);
   }
   else if (id == FID(ZPD))
   {
      pbr->GetFloat(&m_3DZPD);
   }
   else if (id == FID(STO))
   {
	   pbr->GetFloat(&m_3DOffset);
   }
   else if (id == FID(OGST))
   {
      pbr->GetBool(&m_overwriteGlobalStereo3D);
   }
   else if (id == FID(SLPX))
   {
      pbr->GetFloat(&m_angletiltMax);
   }
   else if (id == FID(SLOP))
   {
      pbr->GetFloat(&m_angletiltMin);
      if (m_angletiltMax == 726.0f) m_angletiltMax = m_angletiltMin; //!! ??
   }
   else if (id == FID(GLAS))
   {
      pbr->GetFloat(&m_glassheight);
   }
   else if (id == FID(TBLH))
   {
      pbr->GetFloat(&m_tableheight);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_szImage);
   }
   else if (id == FID(BLIM))
   {
      pbr->GetString(m_szBallImage);
   }
   else if (id == FID(BLIF))
   {
      pbr->GetString(m_szBallImageFront);
   }
   else if (id == FID(SSHT))
   {
      pbr->GetString(m_szScreenShot);
   }
   else if (id == FID(FBCK))
   {
      pbr->GetBool(&m_fBackdrop);
   }
   else if (id == FID(SEDT))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[1]);
   }
   else if (id == FID(SSND))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[2]);
   }
   else if (id == FID(SIMG))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[3]);
   }
   else if (id == FID(SFNT))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[4]);
   }
   else if (id == FID(SCOL))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[5]);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(BIMG))
   {
      pbr->GetString(m_BG_szImage[0]);
   }
   else if (id == FID(BIMF))
   {
      pbr->GetString(m_BG_szImage[1]);
   }
   else if (id == FID(BIMS))
   {
      pbr->GetString(m_BG_szImage[2]);
   }
   else if (id == FID(BIMN))
   {
      pbr->GetBool(&m_ImageBackdropNightDay);
   }
   else if (id == FID(IMCG))
   {
      pbr->GetString(m_szImageColorGrade);
   }
   else if (id == FID(EIMG))
   {
      pbr->GetString(m_szEnvImage);
   }
   else if (id == FID(PLMA))
   {
      pbr->GetString(m_szPlayfieldMaterial);
   }
   else if (id == FID(LZAM))
   {
      pbr->GetInt(&m_lightAmbient);
   }
   else if (id == FID(LZDI))
   {
      pbr->GetInt(&m_Light[0].emission);
   }
   else if (id == FID(LZHI))
   {
      pbr->GetFloat(&m_lightHeight);
   }
   else if (id == FID(LZRA))
   {
      pbr->GetFloat(&m_lightRange);
   }
   else if (id == FID(LIES))
   {
      pbr->GetFloat(&m_lightEmissionScale);
   }
   else if (id == FID(ENES))
   {
      pbr->GetFloat(&m_envEmissionScale);
   }
   else if (id == FID(GLES))
   {
      pbr->GetFloat(&m_globalEmissionScale);
   }
   else if (id == FID(AOSC))
   {
      pbr->GetFloat(&m_AOScale);
   }
   else if (id == FID(BREF))
   {
      pbr->GetInt(&m_useReflectionForBalls);
   }
   else if (id == FID(PLST))
   {
      int tmp;
      pbr->GetInt(&tmp);
      m_playfieldReflectionStrength = dequantizeUnsigned<8>(tmp);
   }
   else if (id == FID(BTRA))
   {
      pbr->GetInt(&m_useTrailForBalls);
   }
   else if (id == FID(BTST))
   {
      int tmp;
      pbr->GetInt(&tmp);
      m_ballTrailStrength = dequantizeUnsigned<8>(tmp);
   }
   else if (id == FID(BPRS))
   {
      pbr->GetFloat(&m_ballPlayfieldReflectionStrength);
   }
   else if (id == FID(DBIS))
   {
      pbr->GetFloat(&m_defaultBulbIntensityScaleOnBall);
   }
   else if (id == FID(UAAL))
   {
      pbr->GetInt(&m_useAA);
   }
   else if (id == FID(UAOC))
   {
      pbr->GetInt(&m_useAO);
   }
   else if (id == FID(UFXA))
   {
      pbr->GetInt(&m_useFXAA);
   }
   else if (id == FID(BLST))
   {
      pbr->GetFloat(&m_bloom_strength);
   }
   else if (id == FID(BCLR))
   {
      pbr->GetInt(&m_colorbackdrop);
   }
   else if (id == FID(SECB))
   {
      pbr->GetStruct(&m_protectionData, sizeof(_protectionData));
      if (pbr->m_version < 700 && m_protectionData.flags != 0)
      {
         m_protectionData.flags |= DISABLE_DEBUGGER;
      }
   }
   else if (id == FID(CODE))
   {
      // if the script is protected then we pass in the proper cyptokey into the code loadstream
      m_pcv->LoadFromStream(pbr->m_pistream, pbr->m_hcrypthash, CheckPermissions(DISABLE_SCRIPT_EDITING) ? pbr->m_hcryptkey : NULL);
   }
   else if (id == FID(CCUS))
   {
      pbr->GetStruct(m_rgcolorcustom, sizeof(COLORREF) * 16);
   }
   else if (id == FID(TDFT))
   {
      pbr->GetFloat(&m_globalDifficulty);
      int tmp;
      HRESULT hr = GetRegInt("Player", "GlobalDifficulty", &tmp);
      if (hr == S_OK) m_globalDifficulty = dequantizeUnsignedPercent(tmp);
   }
   else if (id == FID(CUST))
   {
      char szT[1024];  //maximum length of tagnames right now
      pbr->GetString(szT);
      char *szName;
      szName = new char[lstrlen(szT) + 1];
      lstrcpy(szName, szT);
      m_vCustomInfoTag.AddElement(szName);
   }
   else if (id == FID(SVOL))
   {
      pbr->GetFloat(&m_TableSoundVolume);
   }
   else if (id == FID(BDMO))
   {
      pbr->GetBool(&m_BallDecalMode);
   }
   else if (id == FID(MVOL))
   {
      pbr->GetFloat(&m_TableMusicVolume);
   }
   else if (id == FID(AVSY))
   {
      pbr->GetInt(&m_TableAdaptiveVSync);
   }
   else if (id == FID(OGAC))
   {
      pbr->GetBool(&m_overwriteGlobalDetailLevel);
   }
   else if (id == FID(OGDN))
   {
	   pbr->GetBool(&m_overwriteGlobalDayNight);
   }
   else if (id == FID(GDAC))
   {
      pbr->GetBool(&m_fGrid);
   }
   else if (id == FID(REOP))
   {
      pbr->GetBool(&m_fReflectElementsOnPlayfield);
   }
   else if (id == FID(ARAC))
   {
      pbr->GetInt(&m_userDetailLevel);
   }
   else if (id == FID(MASI))
   {
      pbr->GetInt(&m_numMaterials);
   }
   else if (id == FID(MATE))
   {
      SaveMaterial * const mats = (SaveMaterial*)malloc(sizeof(SaveMaterial)*m_numMaterials);
      pbr->GetStruct(mats, (int)sizeof(SaveMaterial)*m_numMaterials);

      for(int i = 0; i < m_materials.size(); ++i)
          delete m_materials.ElementAt(i);
      m_materials.Reset();

      for (int i = 0; i < m_numMaterials; i++)
      {
         Material * const pmat = new Material();
         pmat->m_cBase = mats[i].cBase;
         pmat->m_cGlossy = mats[i].cGlossy;
         pmat->m_cClearcoat = mats[i].cClearcoat;
         pmat->m_fWrapLighting = mats[i].fWrapLighting;
         pmat->m_fRoughness = mats[i].fRoughness;
         pmat->m_fGlossyImageLerp = 1.0f - dequantizeUnsigned<8>(mats[i].fGlossyImageLerp); //!! '1.0f -' to be compatible with previous table versions
         pmat->m_fThickness = (mats[i].fThickness == 0) ? 0.05f : dequantizeUnsigned<8>(mats[i].fThickness); //!! 0 -> 0.05f to be compatible with previous table versions
         pmat->m_fEdge = mats[i].fEdge;
         pmat->m_fOpacity = mats[i].fOpacity;
         pmat->m_bIsMetal = mats[i].bIsMetal;
         pmat->m_bOpacityActive = !!(mats[i].bOpacityActive_fEdgeAlpha & 1);
         pmat->m_fEdgeAlpha = dequantizeUnsigned<7>(mats[i].bOpacityActive_fEdgeAlpha >> 1);
         strcpy_s(pmat->m_szName, mats[i].szName);
         m_materials.AddElement(pmat);
      }
      free(mats);
   }
   else if(id==FID(PHMA))
   {
       SavePhysicsMaterial * const mats = (SavePhysicsMaterial*)malloc( sizeof( SavePhysicsMaterial )*m_numMaterials );
       pbr->GetStruct( mats, (int)sizeof( SavePhysicsMaterial )*m_numMaterials );

       for ( int i = 0; i < m_numMaterials; i++ )
       {
           bool found=true;
           Material * pmat = GetMaterial(mats[i].szName);
           if( pmat==NULL )
           {
               pmat = new Material();
               found=false;
           }
           pmat->m_fElasticity = mats[i].fElasticity;
           pmat->m_fElasticityFalloff = mats[i].fElasticityFallOff;
           pmat->m_fFriction = mats[i].fFriction;
           pmat->m_fScatterAngle = mats[i].fScatterAngle;
           if( !found )
              m_materials.AddElement( pmat );
       }
       free( mats );
   }

   return fTrue;
}


bool PinTable::ExportSound(HWND hwndListView, PinSound *pps, char *szfilename)
{
   MMIOINFO mmio;
   MMCKINFO pck;
   ZeroMemory(&mmio, sizeof(mmio));
   ZeroMemory(&pck, sizeof(pck));

   HMMIO hmmio = mmioOpen(szfilename, &mmio, MMIO_ALLOCBUF | MMIO_CREATE | MMIO_EXCLUSIVE | MMIO_READWRITE);

   if (hmmio != NULL)
   {
      // quick and dirty ... in a big hurry

      pck.ckid = mmioStringToFOURCC("RIFF", MMIO_TOUPPER);
      pck.cksize = pps->m_cdata + 36;
      pck.fccType = mmioStringToFOURCC("WAVE", MMIO_TOUPPER);

      MMRESULT result = mmioCreateChunk(hmmio, &pck, MMIO_CREATERIFF); //RIFF header
      mmioWrite(hmmio, "fmt ", 4);			//fmt

      // Create the format chunk.
      pck.cksize = sizeof(WAVEFORMATEX);
      result = mmioCreateChunk(hmmio, &pck, 4);//0
      // Write the wave format data.
      int i = 16;
      mmioWrite(hmmio, (char *)&i, 4);
      mmioWrite(hmmio, (char*)&pps->m_wfx, (LONG)sizeof(pps->m_wfx) - 2); //END OF CORRECTION

      mmioWrite(hmmio, "data", 4);						//data chunk
      i = pps->m_cdata; mmioWrite(hmmio, (char *)&i, 4);	// data size bytes

      const LONG wcch = mmioWrite(hmmio, pps->m_pdata, pps->m_cdata);
      result = mmioClose(hmmio, 0);

      if (wcch != pps->m_cdata) 
          ::MessageBox(m_hwnd, "Sound file incomplete!", "Visual Pinball", MB_ICONERROR);
      else return true;

   }
   else 
       ::MessageBox(m_hwnd, "Can not Open/Create Sound file!", "Visual Pinball", MB_ICONERROR);

   return false;
}

void PinTable::ReImportSound(HWND hwndListView, PinSound *pps, char *filename, BOOL fPlay)
{
   PinSound * const ppsNew = g_pvp->m_pds.LoadWaveFile(filename);

   if (ppsNew == NULL)
   {
      return;
   }

   PinSound psT = *pps;
   *pps = *ppsNew;
   *ppsNew = psT;

   lstrcpy(pps->m_szName, ppsNew->m_szName);
   lstrcpy(pps->m_szInternalName, ppsNew->m_szInternalName);

   // make sure sound data doesn't get deleted twice
   psT.m_pdata = NULL;
   psT.m_pDSBuffer = NULL;
   psT.m_pDS3DBuffer = NULL;
   delete ppsNew;

   if (fPlay)
   {
      pps->m_pDSBuffer->Play(0, 0, 0);
   }
}


void PinTable::ImportSound(HWND hwndListView, char *szfilename, BOOL fPlay)
{
   PinSound * const pps = g_pvp->m_pds.LoadWaveFile(szfilename);

   if (pps == NULL)
   {
      return;
   }

   if (fPlay)
   {
      pps->m_pDSBuffer->Play(0, 0, 0);
   }

   m_vsound.AddElement(pps);

   const int index = AddListSound(hwndListView, pps);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
}

void PinTable::ListSounds(HWND hwndListView)
{
	ListView_DeleteAllItems(hwndListView);
	for (int i = 0; i < m_vsound.Size(); i++)
	{
		AddListSound(hwndListView, m_vsound.ElementAt(i));
	}
}


int PinTable::AddListSound(HWND hwndListView, PinSound *pps)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = pps->m_szName;
   lvitem.lParam = (size_t)pps;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, pps->m_szPath);

   switch (pps->m_iOutputTarget)
   {
   case SNDOUT_BACKGLASS:
	   ListView_SetItemText(hwndListView, index, 2, "Backglass");
	   break;
   default:
	   ListView_SetItemText(hwndListView, index, 2, "Table");
	   break;
   }
   char textBuf[40];
   sprintf_s(textBuf, "%.03f", dequantizeSignedPercent(pps->m_iBalance));
   ListView_SetItemText(hwndListView, index, 3, textBuf);
   sprintf_s(textBuf, "%.03f", dequantizeSignedPercent(pps->m_iFade));
   ListView_SetItemText(hwndListView, index, 4, textBuf);
   sprintf_s(textBuf, "%.03f", dequantizeSignedPercent(pps->m_iVolume));
   ListView_SetItemText(hwndListView, index, 5, textBuf);

   return index;
}

void PinTable::RemoveSound(PinSound *pps)
{
   m_vsound.RemoveElement(pps);
   delete pps;
}

void PinTable::ImportFont(HWND hwndListView, char *filename)
{
   PinFont * const ppb = new PinFont();

   ppb->ReadFromFile(filename);

   if (ppb->m_pdata != NULL)
   {
      m_vfont.AddElement(ppb);

      const int index = AddListBinary(hwndListView, ppb);

      ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

      ppb->Register();
   }
}


void PinTable::RemoveFont(PinFont *ppf)
{
   m_vfont.RemoveElement(ppf);
   ppf->UnRegister();
   delete ppf;
}


void PinTable::ListFonts(HWND hwndListView)
{
   for (int i = 0; i < m_vfont.Size(); i++)
   {
      AddListBinary(hwndListView, m_vfont.ElementAt(i));
   }
}

int PinTable::AddListBinary(HWND hwndListView, PinBinary *ppb)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = ppb->m_szName;
   lvitem.lParam = (size_t)ppb;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, ppb->m_szPath);

   return index;
}

void PinTable::NewCollection(HWND hwndListView, BOOL fFromSelection)
{
   WCHAR wzT[128];

   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();

   LocalStringW prefix(IDS_COLLECTION);
   GetUniqueName(prefix.str, wzT);

   WideStrNCopy(wzT, pcol->m_wzName, MAXNAMEBUFFER);

   if (fFromSelection && !MultiSelIsEmpty())
   {
      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         ISelect *pisel = m_vmultisel.ElementAt(i);
         IEditable *piedit = pisel->GetIEditable();
         if (piedit)
         {
            if (piedit->GetISelect() == pisel) // Do this check so we don't put walls in a collection when we only have the control point selected
            {
               if (piedit->GetScriptable()) // check for scriptable because can't add decals to a collection - they have no name
               {
                  piedit->m_vCollection.AddElement(pcol);
                  piedit->m_viCollection.AddElement((void *)pcol->m_visel.Size());
                  pcol->m_visel.AddElement(m_vmultisel.ElementAt(i));
               }
            }
         }
      }
   }

   const int index = AddListCollection(hwndListView, pcol);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

   m_vcollection.AddElement(pcol);
   m_pcv->AddItem((IScriptable *)pcol, fFalse);
}

int PinTable::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
{
   char szT[MAX_PATH];

   WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = szT;
   lvitem.lParam = (size_t)pcol;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   char buf[16] = { 0 };
   sprintf_s(buf, "%i", pcol->m_visel.Size());
   ListView_SetItemText(hwndListView, index, 1, buf);
   return index;
}

void PinTable::ListCollections(HWND hwndListView)
{
   //ListView_DeleteAllItems(hwndListView);

   for (int i = 0; i < m_vcollection.Size(); i++)
   {
      CComObject<Collection> * const pcol = m_vcollection.ElementAt(i);

      AddListCollection(hwndListView, pcol);
   }
}

void PinTable::RemoveCollection(CComObject<Collection> *pcol)
{
   m_pcv->RemoveItem((IScriptable *)pcol);
   m_vcollection.RemoveElement(pcol);
   pcol->Release();
}

void PinTable::MoveCollectionUp(CComObject<Collection> *pcol)
{
   const int idx = m_vcollection.IndexOf(pcol);
   m_vcollection.RemoveElementAt(idx);
   if (idx - 1 < 0)
      m_vcollection.AddElement(pcol);
   else
      m_vcollection.InsertElementAt(pcol, idx - 1);
}

int PinTable::GetDetailLevel()
{
   return m_overwriteGlobalDetailLevel ? m_userDetailLevel : m_globalDetailLevel;
}

float PinTable::GetZPD()
{
   return m_overwriteGlobalStereo3D ? m_3DZPD : m_global3DZPD;
}

float PinTable::GetMaxSeparation()
{
   return m_overwriteGlobalStereo3D ? m_3DmaxSeparation : m_global3DMaxSeparation;
}

float PinTable::Get3DOffset()
{
	return m_overwriteGlobalStereo3D ? m_3DOffset : m_global3DOffset;
}

FRect3D PinTable::GetBoundingBox()
{
   FRect3D bbox;
   bbox.left = m_left;
   bbox.right = m_right;
   bbox.top = m_top;
   bbox.bottom = m_bottom;
   bbox.zlow = m_tableheight;
   bbox.zhigh = m_glassheight;
   return bbox;
}

void PinTable::MoveCollectionDown(CComObject<Collection> *pcol)
{
   int idx = m_vcollection.IndexOf(pcol);
   m_vcollection.RemoveElementAt(idx);
   if (idx + 1 >= m_vcollection.Size())
      m_vcollection.InsertElementAt(pcol, 0);
   else
      m_vcollection.InsertElementAt(pcol, idx + 1);
}

void PinTable::SetCollectionName(Collection *pcol, char *szName, HWND hwndList, int index)
{
   WCHAR wzT[1024];
   MultiByteToWideChar(CP_ACP, 0, szName, -1, wzT, 1024);
   if (m_pcv->ReplaceName((IScriptable *)pcol, wzT) == S_OK)
   {
      if (hwndList)
      {
         ListView_SetItemText(hwndList, index, 0, szName);
      }
      WideStrNCopy(wzT, pcol->m_wzName, MAXNAMEBUFFER);
   }
}

void PinTable::SetZoom(float zoom)
{
   m_zoom = zoom;
   SetMyScrollInfo();
}

void PinTable::GetViewRect(FRect *pfrect)
{
   if (!g_pvp->m_fBackglassView)
   {
      pfrect->left = m_left;
      pfrect->top = m_top;
      pfrect->right = m_right;
      pfrect->bottom = m_bottom;
   }
   else
   {
      pfrect->left = 0;
      pfrect->top = 0;
      pfrect->right = EDITOR_BG_WIDTH;
      pfrect->bottom = EDITOR_BG_HEIGHT;
   }
}

void PinTable::SetMyScrollInfo()
{
   FRect frect;
   GetViewRect(&frect);

   RECT rc;
   ::GetClientRect(m_hwnd, &rc);

   HitSur * const phs = new HitSur(NULL, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

   Vertex2D rgv[2];
   rgv[0] = phs->ScreenToSurface(rc.left, rc.top);
   rgv[1] = phs->ScreenToSurface(rc.right, rc.bottom);

   delete phs;

   SCROLLINFO si;
   ZeroMemory(&si, sizeof(SCROLLINFO));
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
   si.nMin = (int)min(frect.left, rgv[0].x);
   si.nMax = (int)max(frect.right, rgv[1].x);
   si.nPage = (int)(rgv[1].x - rgv[0].x);
   si.nPos = (int)(rgv[0].x);

   ::SetScrollInfo(m_hwnd, SB_HORZ, &si, fTrue);

   si.nMin = (int)min(frect.top, rgv[0].y);
   si.nMax = (int)max(frect.bottom, rgv[1].y);
   si.nPage = (int)(rgv[1].y - rgv[0].y);
   si.nPos = (int)(rgv[0].y);

   ::SetScrollInfo(m_hwnd, SB_VERT, &si, fTrue);
}

void PinTable::FireKeyEvent(int dispid, int keycode)
{
   CComVariant rgvar[1] = { CComVariant(keycode) };

   DISPPARAMS dispparams = {
      rgvar,
      NULL,
      1,
      0
   };

#ifdef LOG
   if (g_pplayer)
   {
      if (dispid == DISPID_GameEvents_KeyDown)
      {
         fprintf(g_pplayer->m_flog, "Key Down %d\n", keycode);
      }
      else
      {
         fprintf(g_pplayer->m_flog, "Key Up %d\n", keycode);
      }
   }
#endif

   FireDispID(dispid, &dispparams);
}

void PinTable::DoLButtonDown(int x, int y, bool zoomIn)
{
   const int ksshift = GetKeyState(VK_SHIFT);
   const int ksctrl = GetKeyState(VK_CONTROL);

   // set the focus of the window so all keyboard and mouse inputs are processed.
   // (this fixes the problem of selecting a element on the properties dialog, clicking on a table
   // object and not being able to use the cursor keys/wheely mouse
   ::SetFocus(g_pvp->m_hwndWork);

   if ((g_pvp->m_ToolCur == ID_TABLE_MAGNIFY) || (ksctrl & 0x80000000))
   {
      if (m_zoom < MAX_ZOOM)
      {
         m_offset = TransformPoint(x, y);

         if (zoomIn)
            SetZoom(m_zoom * 1.5f);
         else
            SetZoom(m_zoom * 0.5f);

         SetDirtyDraw();
      }
   }

   // if disabling table view then don't allow the table to be selected (thus bringing up table properties)
   else if (!CheckPermissions(DISABLE_TABLEVIEW))
      // Normal click
   {
      ISelect * const pisel = HitTest(x, y);

      const bool fAdd = ((ksshift & 0x80000000) != 0);

      if (pisel == (ISelect *)this && fAdd)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         OnLButtonDown(x, y); // Start the band select
         return;
      }

      AddMultiSel(pisel, fAdd);

      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         ISelect *pisel2 = m_vmultisel.ElementAt(i);
         if ( pisel2 )
            pisel2->OnLButtonDown(x, y);
      }
   }
}

void PinTable::DoLButtonUp(int x, int y)
{
   //m_pselcur->OnLButtonUp(x,y);

   if (!m_fDragging) // Not doing band select
   {
      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         ISelect *pisel = m_vmultisel.ElementAt(i);
         if ( pisel )
            pisel->OnLButtonUp(x, y);
      }
   }
   else
   {
      OnLButtonUp(x, y);
   }
}

void PinTable::DoRButtonDown(int x, int y)
{
   DoLButtonUp(x, y); //corrects issue with left mouse button being in 'stuck down' position on a control point or object - BDS

   const int ks = GetKeyState(VK_CONTROL);

   if ((g_pvp->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000))
   {
      if (m_zoom > MIN_ZOOM)
      {
         m_offset = TransformPoint(x, y);
         SetZoom(m_zoom * 0.5f);
         SetDirtyDraw();
      }
   }
   else
   {
      // keep the selection if clicking over a selected object, even if
      // the selected object is hidden behind other objects
      ISelect *hit = HitTest(x, y);
      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         if (m_allHitElements.IndexOf(m_vmultisel.ElementAt(i)) != -1)
         {
            // found a selected item - keep the current selection set
            // by re-selecting this item (which will also promote it
            // to the head of the selection list)
            hit = m_vmultisel.ElementAt(i);
            break;
         }
      }

      // update the selection
      AddMultiSel(hit, false);
   }
}

void PinTable::FillCollectionContextMenu(HMENU hmenu, HMENU colSubMenu, ISelect *psel)
{
    LocalString ls16(IDS_TO_COLLECTION);
    AppendMenu(hmenu, MF_POPUP | MF_STRING, (size_t)colSubMenu, ls16.m_szbuffer);

    int maxItems = m_vcollection.Size() - 1;
    if(maxItems > 32) maxItems = 32;

    // run through all collections and list up to 32 of them in the context menu
    // the actual processing is done in ISelect::DoCommand() 
    for(int i = maxItems; i >= 0; i--)
    {
        CComBSTR bstr;
        m_vcollection.ElementAt(i)->get_Name(&bstr);
        char szT[64]; // Names can only be 32 characters (plus terminator)
        WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);

        AppendMenu(colSubMenu, MF_POPUP, 0x40000 + i, szT);
        CheckMenuItem(colSubMenu, 0x40000 + i, MF_UNCHECKED);
    }
    if(m_vmultisel.Size() == 1)
    {
        for(int i = maxItems; i >= 0; i--)
        {
            for(int t = 0; t < m_vcollection.ElementAt(i)->m_visel.Size(); t++)
            {
                if(psel == m_vcollection.ElementAt(i)->m_visel.ElementAt(t))
                {
                    CheckMenuItem(colSubMenu, 0x40000 + i, MF_CHECKED);
                }
            }
        }
    }
    else
    {
        vector<int> allIndices;

        for(int t = 0; t < m_vmultisel.Size(); t++)
        {
            ISelect *iSel = m_vmultisel.ElementAt(t);

            for(int i = maxItems; i >= 0 ; i--)
            {
                for(int t = 0; t < m_vcollection.ElementAt(i)->m_visel.Size(); t++)
                {
                    if((iSel == m_vcollection.ElementAt(i)->m_visel.ElementAt(t)))
                    {
                        allIndices.push_back(i);
                    }
                }
            }
        }
        if(allIndices.size() % m_vmultisel.Size() == 0)
        {
            for(size_t i = 0; i < allIndices.size();i++)
                CheckMenuItem(colSubMenu, 0x40000 + allIndices[i], MF_CHECKED);
        }
        else
        {
            for(size_t i = 0; i < allIndices.size(); i++)
                CheckMenuItem(colSubMenu, 0x40000 + allIndices[i], MF_CHECKED);
        }
    }
}

void PinTable::DoContextMenu(int x, int y, int menuid, ISelect *psel)
{
   POINT pt;
   pt.x = x;
   pt.y = y;
   ::ClientToScreen(m_hwnd, &pt);

   HMENU hmenumain;
   HMENU hmenu;
   HMENU subMenu;
   HMENU colSubMenu;
   if (menuid != -1)
   {
      hmenumain = LoadMenu(g_hinst, MAKEINTRESOURCE(menuid));

      hmenu = GetSubMenu(hmenumain, 0);
   }
   else
   {
      hmenu = CreatePopupMenu();
      subMenu = CreatePopupMenu();
      colSubMenu = CreatePopupMenu();
   }

   psel->EditMenu(hmenu);

   if (menuid != IDR_POINTMENU && menuid != IDR_TABLEMENU && menuid != IDR_POINTMENU_SMOOTH) //psel->GetIEditable() != NULL)
   {
      if (GetMenuItemCount(hmenu) > 0)
      {
         AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");
         subMenu = CreatePopupMenu();
         colSubMenu = CreatePopupMenu();
      }
      // TEXT
      LocalString ls17(IDS_COPY_ELEMENT);
      AppendMenu(hmenu, MF_STRING, IDC_COPY, ls17.m_szbuffer);
      LocalString ls18(IDS_PASTE_ELEMENT);
      AppendMenu(hmenu, MF_STRING, IDC_PASTE, ls18.m_szbuffer);
      LocalString ls19(IDS_PASTE_AT_ELEMENT);
      AppendMenu(hmenu, MF_STRING, IDC_PASTEAT, ls19.m_szbuffer);

      AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");

      LocalString ls14(IDS_DRAWING_ORDER_HIT);
      AppendMenu(hmenu, MF_STRING, ID_EDIT_DRAWINGORDER_HIT, ls14.m_szbuffer);
      LocalString ls15(IDS_DRAWING_ORDER_SELECT);
      AppendMenu(hmenu, MF_STRING, ID_EDIT_DRAWINGORDER_SELECT, ls15.m_szbuffer);

      LocalString ls1(IDS_DRAWINFRONT);
      LocalString ls2(IDS_DRAWINBACK);
      AppendMenu(hmenu, MF_STRING, ID_DRAWINFRONT, ls1.m_szbuffer);
      AppendMenu(hmenu, MF_STRING, ID_DRAWINBACK, ls2.m_szbuffer);

      LocalString ls3(IDS_SETASDEFAULT);
      AppendMenu(hmenu, MF_STRING, ID_SETASDEFAULT, ls3.m_szbuffer);

      LocalString ls4(IDS_ASSIGNTO);
      AppendMenu(hmenu, MF_POPUP | MF_STRING, (size_t)subMenu, ls4.m_szbuffer);
      LocalString ls6(IDS_LAYER1);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER1, ls6.m_szbuffer);
      LocalString ls7(IDS_LAYER2);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER2, ls7.m_szbuffer);
      LocalString ls8(IDS_LAYER3);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER3, ls8.m_szbuffer);
      LocalString ls9(IDS_LAYER4);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER4, ls9.m_szbuffer);
      LocalString ls10(IDS_LAYER5);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER5, ls10.m_szbuffer);
      LocalString ls11(IDS_LAYER6);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER6, ls11.m_szbuffer);
      LocalString ls12(IDS_LAYER7);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER7, ls12.m_szbuffer);
      LocalString ls13(IDS_LAYER8);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER8, ls13.m_szbuffer);

      if (psel->layerIndex == 0)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER1, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER1, MF_UNCHECKED);
      if (psel->layerIndex == 1)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER2, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER2, MF_UNCHECKED);
      if (psel->layerIndex == 2)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER3, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER3, MF_UNCHECKED);
      if (psel->layerIndex == 3)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER4, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER4, MF_UNCHECKED);
      if (psel->layerIndex == 4)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER5, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER5, MF_UNCHECKED);
      if (psel->layerIndex == 5)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER6, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER6, MF_UNCHECKED);
      if (psel->layerIndex == 6)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER7, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER7, MF_UNCHECKED);
      if (psel->layerIndex == 7)
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER8, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER8, MF_UNCHECKED);

      FillCollectionContextMenu(hmenu, colSubMenu, psel);

      LocalString ls5(IDS_LOCK);
      AppendMenu(hmenu, MF_STRING, ID_LOCK, ls5.m_szbuffer);

      AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");
      AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");
      /*now list all elements that are stacked at the mouse pointer*/
      for (int i = 0; i < m_allHitElements.Size(); i++)
      {
         if (!m_allHitElements.ElementAt(i)->GetIEditable()->m_isVisible)
         {
            continue;
         }

         ISelect *ptr = m_allHitElements.ElementAt(i);
         if (ptr)
         {
            IEditable *pedit = m_allHitElements.ElementAt(i)->GetIEditable();
            if (pedit)
            {
               char *szTemp;
               szTemp = GetElementName(pedit);

               if (szTemp)
               {
                  // what a hack!
                  // the element index of the allHitElements vector is encoded inside the ID of the context menu item
                  // I didn't find an easy way to identify the selected menu item of a context menu
                  // so the ID_SELECT_ELEMENT is the global ID for selecting an element from the list and the rest is
                  // added for finding the element out of the list
                  // the selection is done in ISelect::DoCommand()
                  unsigned long id = 0x80000000 + (i << 16) + ID_SELECT_ELEMENT;
                  AppendMenu(hmenu, MF_STRING, id, szTemp);
               }
            }
         }
      }
      bool fLocked = psel->m_fLocked;
      // HACK
      if (psel == this) // multi-select case
      {
         fLocked = FMutilSelLocked();
      }

      CheckMenuItem(hmenu, ID_LOCK, MF_BYCOMMAND | (fLocked ? MF_CHECKED : MF_UNCHECKED));
   }

   const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD, pt.x, pt.y, m_hwnd, NULL);

   if (icmd != 0)
   {
      psel->DoCommand(icmd, x, y);
   }

   DestroyMenu(hmenu);

   if (menuid != -1)
   {
      DestroyMenu(hmenumain);
   }
}

char elementName[256];
char *PinTable::GetElementName(IEditable *pedit)
{
   WCHAR *elemName = NULL;
   IScriptable *pscript = NULL;
   if (pedit)
   {
      pscript = pedit->GetScriptable();
      if (pedit->GetItemType() == eItemDecal)
      {
         return "Decal";
      }
      if (pscript)
      {
         elemName = pscript->m_wzName;
      }
   }
   if (elemName)
   {
      WideCharToMultiByte(CP_ACP, 0, elemName, -1, elementName, 256, NULL, NULL);
      return elementName;
   }
   return NULL;
}

IEditable *PinTable::GetElementByName(const char *name)
{
   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *pedit = m_vedit.ElementAt(i);
      if (strcmp(name, GetElementName(pedit)) == 0)
         return pedit;
   }
   return NULL;
}

bool PinTable::FMutilSelLocked()
{
   bool fLocked = false;

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      if (m_vmultisel.ElementAt(i)->m_fLocked)
      {
         fLocked = true;
         break;
      }
   }

   return fLocked;
}

void PinTable::DoCommand(int icmd, int x, int y)
{
   if (((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020))
   {
      UpdateCollection(icmd & 0x000000FF);
      return;
   }

   if ((icmd & 0x0000FFFF) == ID_SELECT_ELEMENT)
   {
      int i = (icmd & 0x00FF0000) >> 16;
      ISelect * const pisel = m_allHitElements.ElementAt(i);
      pisel->DoCommand(icmd, x, y);
      return;
   }

   switch (icmd)
   {
   case ID_DRAWINFRONT:
   case ID_DRAWINBACK:
   {
      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         ISelect *psel;
         psel = m_vmultisel.ElementAt(i);
         _ASSERTE(psel != this); // Would make an infinite loop
         psel->DoCommand(icmd, x, y);
      }
   }
   break;

   case ID_EDIT_DRAWINGORDER_HIT:
      g_pvp->ShowDrawingOrderDialog(false);
      break;

   case ID_EDIT_DRAWINGORDER_SELECT:
      g_pvp->ShowDrawingOrderDialog(true);
      break;

   case ID_LOCK:
   {
      LockElements();
      break;
   }

   case ID_WALLMENU_FLIP:
   {
      Vertex2D vCenter;

      GetCenter(&vCenter);

      FlipY(&vCenter);
      break;
   }

   case ID_WALLMENU_MIRROR:
   {
      Vertex2D vCenter;

      GetCenter(&vCenter);

      FlipX(&vCenter);
      break;
   }
   case IDC_COPY:
   {
       if (CheckPermissions(DISABLE_CUTCOPYPASTE))
           g_pvp->ShowPermissionError();
       else
           Copy(x,y);
       break;
   }
   case IDC_PASTE:
   {
       Paste(fFalse, x, y);
       break;
   }
   case IDC_PASTEAT:
   {
      Paste(fTrue, x, y);
      break;
   }

   case ID_WALLMENU_ROTATE:
   {
      DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_ROTATE),
         g_pvp->m_hwnd, RotateProc, (size_t)(ISelect *)this);
   }
   break;

   case ID_WALLMENU_SCALE:
   {
      DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SCALE),
         g_pvp->m_hwnd, ScaleProc, (size_t)(ISelect *)this);
   }
   break;

   case ID_WALLMENU_TRANSLATE:
   {
      DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_TRANSLATE),
         g_pvp->m_hwnd, TranslateProc, (size_t)(ISelect *)this);
   }
   break;
   case ID_ASSIGNTO_LAYER1:
   {
      AssignMultiToLayer(0, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER2:
   {
      AssignMultiToLayer(1, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER3:
   {
      AssignMultiToLayer(2, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER4:
   {
      AssignMultiToLayer(3, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER5:
   {
      AssignMultiToLayer(4, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER6:
   {
      AssignMultiToLayer(5, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER7:
   {
      AssignMultiToLayer(6, x, y);
      break;
   }
   case ID_ASSIGNTO_LAYER8:
   {
      AssignMultiToLayer(7, x, y);
      break;
   }
   }
}
void PinTable::AssignMultiToLayer(int layerNumber, int x, int y)
{
   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      ISelect *psel;
      psel = m_vmultisel.ElementAt(i);
      _ASSERTE(psel != this); // Would make an infinite loop
      switch (layerNumber)
      {
      case 0:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER1, x, y);
         break;
      }
      case 1:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER2, x, y);
         break;
      }
      case 2:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER3, x, y);
         break;
      }
      case 3:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER4, x, y);
         break;
      }
      case 4:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER5, x, y);
         break;
      }
      case 5:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER6, x, y);
         break;
      }
      case 6:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER7, x, y);
         break;
      }
      case 7:
      {
         psel->DoCommand(ID_ASSIGNTO_LAYER8, x, y);
         break;
      }
      }
   }
}

void PinTable::UpdateCollection(int index)
{
   if (index < m_vcollection.Size() && index < 32)
   {
      if (m_vmultisel.Size() > 0)
      {
         bool removeOnly = false;
         /* if the selection is part of the selected collection remove only remove these elements*/
         for (int t = 0; t < m_vmultisel.Size(); t++)
         {
            ISelect *ptr = m_vmultisel.ElementAt(t);
            for (int k = 0; k < m_vcollection.ElementAt(index)->m_visel.Size(); k++)
            {
               if (ptr == m_vcollection.ElementAt(index)->m_visel.ElementAt(k))
               {
                  m_vcollection.ElementAt(index)->m_visel.RemoveElement(ptr);
                  removeOnly = true;
                  break;
               }
            }
         }

         if(removeOnly)
             return;

         /*selected elements are not part of the the selected collection and can be added*/
         for(int t = 0; t < m_vmultisel.Size(); t++)
         {
            ISelect *ptr = m_vmultisel.ElementAt(t);
            m_vcollection.ElementAt(index)->m_visel.AddElement(ptr);
        }
      }
   }
}

bool PinTable::GetCollectionIndex(ISelect *element, int &collectionIndex, int &elementIndex)
{
   for (int i = 0; i < m_vcollection.size(); i++)
   {
      for (int t = 0; t < m_vcollection.ElementAt(i)->m_visel.size(); t++)
      {
         if (element == m_vcollection.ElementAt(i)->m_visel.ElementAt(t))
         {
            collectionIndex = i;
            elementIndex = t;
            return true;
         }
      }
   }
   return false;
}

void PinTable::LockElements()
{
   BeginUndo();
   const bool fLock = !FMutilSelLocked();
   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      ISelect * const psel = m_vmultisel.ElementAt(i);
      if (psel)
      {
         IEditable * const pedit = psel->GetIEditable();
         if (pedit)
         {
            pedit->MarkForUndo();
            psel->m_fLocked = fLock;
         }
      }
   }
   EndUndo();
   SetDirtyDraw();
}

LRESULT PinTable::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CComObject<PinTable> *pt;
//    HWND hwnd = GetHwnd();
    HWND hwnd=0;
    switch(uMsg)
    {
        case WM_CLOSE:
        {
            ::KillTimer(hwnd, TIMER_ID_AUTOSAVE);
            ::SetTimer(hwnd, TIMER_ID_CLOSE_TABLE, 100, NULL);	//wait 250 milliseconds

            return 0;	// destroy the WM_CLOSE message
        }
        case WM_TIMER:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            switch(wParam)
            {
                case TIMER_ID_AUTOSAVE:
                {
                    pt->AutoSave();
                    break;
                }

                case TIMER_ID_CLOSE_TABLE:
                {
                    ::KillTimer(hwnd, TIMER_ID_CLOSE_TABLE);
                    pt->m_pvp->CloseTable(pt);
                    //DestroyWindow(hwnd);
                    return 0;
                    break;
                }
            }
        }
        break;

        case WM_SETCURSOR:
        {
            if(LOWORD(lParam) == HTCLIENT)
            {
                char *cursorid;
                HINSTANCE hinst = g_hinst;

                if(g_pvp->m_ToolCur == ID_TABLE_MAGNIFY)
                {
                    cursorid = MAKEINTRESOURCE(IDC_MAGNIFY);
                }
                else if(g_pvp->m_ToolCur == ID_INSERT_TARGET)
                {
                    // special case for targets, which are particular walls
                    cursorid = MAKEINTRESOURCE(IDC_TARGET);
                }
                else
                {
                    ItemTypeEnum type = EditableRegistry::TypeFromToolID(g_pvp->m_ToolCur);
                    if(type != eItemInvalid)
                        cursorid = MAKEINTRESOURCE(EditableRegistry::GetCursorID(type));
                    else
                    {
                        hinst = NULL;
                        cursorid = IDC_ARROW;
                    }
                }
                HCURSOR hcursor = LoadCursor(hinst, cursorid);
                SetCursor(hcursor);
                return TRUE;
            }
        }
        break;

        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->Paint(hdc);
            ::EndPaint(hwnd, &ps);
            break;
        }

        case WM_MOUSEACTIVATE:
        case WM_ACTIVATE:
        if(LOWORD(wParam) != WA_INACTIVE)
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->m_pvp->m_ptableActive = pt;

            // re-evaluate the toolbar depending on table permissions
            g_pvp->SetEnableToolbar();
        }
        break;

        case WM_LBUTTONDOWN: 
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            if((g_pvp->m_ToolCur == IDC_SELECT) || (g_pvp->m_ToolCur == ID_TABLE_MAGNIFY))
            {
                pt->DoLButtonDown(x, y);
            }
            else
            {
                pt->UseTool(x, y, g_pvp->m_ToolCur);
            }
            break;
        }

        case WM_LBUTTONDBLCLK: 
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            pt->DoLDoubleClick(x, y);
            break;
        }

        case WM_LBUTTONUP:
        {
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->DoLButtonUp(x, y);
        }
        break;

        case WM_MOUSEMOVE:
        {
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const BOOL middleMouseButtonPressed = ((GetKeyState(VK_MBUTTON) & 0x100) != 0);  //((GetKeyState(VK_MENU) & 0x80000000) != 0);
            if(middleMouseButtonPressed)
            {
                // panning feature starts here...if the user holds the middle mouse button and moves the mouse 
                // everything is moved in the direction of the mouse was moved
                int dx = abs(pt->m_oldMousePosX - x);
                int dy = abs(pt->m_oldMousePosY - y);
                if(pt->m_oldMousePosX > x)  pt->m_offset.x += dx;
                if(pt->m_oldMousePosX < x)  pt->m_offset.x -= dx;
                if(pt->m_oldMousePosY > y)  pt->m_offset.y += dy;
                if(pt->m_oldMousePosY < y)  pt->m_offset.y -= dy;
                pt->SetDirtyDraw();
                pt->SetMyScrollInfo();
                pt->m_oldMousePosX = x;
                pt->m_oldMousePosY = y;
                break;
            }
            pt->DoMouseMove(x, y);
            pt->m_oldMousePosX = x;
            pt->m_oldMousePosY = y;
        }
        break;

        case WM_RBUTTONDOWN:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);

            pt->DoRButtonDown(x, y);
            break;
        }
        case WM_CONTEXTMENU:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            long x = (long)(lParam & 0xffff);
            long y = (long)((lParam >> 16) & 0xffff);
            POINT p;
            if(GetCursorPos(&p) && ::ScreenToClient(hwnd, &p))
            {
                x = p.x;
                y = p.y;
            }
            pt->DoRButtonUp(x, y);
            break;
        }
        case WM_KEYDOWN:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->OnKeyDown((int)wParam);
            break;
        }

        case WM_HSCROLL:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            SCROLLINFO si;
            ZeroMemory(&si, sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            ::GetScrollInfo(hwnd, SB_HORZ, &si);
            switch(LOWORD(wParam))
            {
                case SB_LINELEFT:
                pt->m_offset.x -= si.nPage / 10;
                break;
                case SB_LINERIGHT:
                pt->m_offset.x += si.nPage / 10;
                break;
                case SB_PAGELEFT:
                pt->m_offset.x -= si.nPage / 2;
                break;
                case SB_PAGERIGHT:
                pt->m_offset.x += si.nPage / 2;
                break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(pt->m_offset.x - si.nPos);
                    pt->m_offset.x = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            pt->SetDirtyDraw();
            pt->SetMyScrollInfo();
            return 0;
        }
        break;

        case WM_VSCROLL:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            SCROLLINFO si;
            ZeroMemory(&si, sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            ::GetScrollInfo(hwnd, SB_VERT, &si);
            switch(LOWORD(wParam))
            {
                case SB_LINEUP:
                pt->m_offset.y -= si.nPage / 10;
                break;
                case SB_LINEDOWN:
                pt->m_offset.y += si.nPage / 10;
                break;
                case SB_PAGEUP:
                pt->m_offset.y -= si.nPage / 2;
                break;
                case SB_PAGEDOWN:
                pt->m_offset.y += si.nPage / 2;
                break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(pt->m_offset.y - si.nPos);
                    pt->m_offset.y = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            pt->SetDirtyDraw();
            pt->SetMyScrollInfo();
            return 0;
        }
        break;

        case WM_MOUSEWHEEL:
        {
            //zoom in/out by pressing CTRL+mouse wheel
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            const int ksctrl = GetKeyState(VK_CONTROL);
            if((ksctrl & 0x80000000))
            {
                POINT curpt;
                curpt.x = GET_X_LPARAM(lParam);
                curpt.y = GET_Y_LPARAM(lParam);
                ::ScreenToClient(hwnd, &curpt);
                const short x = (short)curpt.x;
                const short y = (short)curpt.y;
                if((g_pvp->m_ToolCur == IDC_SELECT) || (g_pvp->m_ToolCur == ID_TABLE_MAGNIFY))
                {
                    pt->DoLButtonDown(x, y, zDelta != -120);
                }
            }
            else
            {
                pt->m_offset.y -= zDelta / pt->m_zoom;	// change to orientation to match windows default
                pt->SetDirtyDraw();
                pt->SetMyScrollInfo();
            }
            return 0;
        }
        break;

        case WM_SIZE:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(pt) // Window might have just been created
            {
                pt->SetMyScrollInfo();
                pt->m_fDirtyDraw = true;
                // this window command is called whenever the MDI window changes over
                // re-evaluate the toolbar depending on table permissions
                g_pvp->SetEnableToolbar();
            }
            break;
        }

        case WM_COMMAND:
        break;

        case DONE_AUTOSAVE:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(lParam == S_OK)
            {
                g_pvp->SetActionCur("");
            }
            else
            {
                g_pvp->SetActionCur("Autosave Failed");
            }
            pt->BeginAutoSaveCounter();
            HANDLE hEvent = (HANDLE)wParam;
            RemoveFromVector(pt->m_vAsyncHandles, hEvent);
            CloseHandle(hEvent);
        }
        break;
    }
//    return WndProcDefault(uMsg, wParam, lParam);
    return 0;
}

void PinTable::FlipY(Vertex2D * const pvCenter)
{
   BeginUndo();

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      m_vmultisel.ElementAt(i)->FlipY(pvCenter);
   }

   EndUndo();
}

void PinTable::FlipX(Vertex2D * const pvCenter)
{
   BeginUndo();

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      m_vmultisel.ElementAt(i)->FlipX(pvCenter);
   }

   EndUndo();
}

void PinTable::Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter)
{
   BeginUndo();

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      m_vmultisel.ElementAt(i)->Rotate(ang, pvCenter, useElementCenter);
   }

   EndUndo();
}

void PinTable::Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter)
{
   BeginUndo();

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      m_vmultisel.ElementAt(i)->Scale(scalex, scaley, pvCenter, useElementsCenter);
   }

   EndUndo();
}

void PinTable::Translate(Vertex2D *pvOffset)
{
   BeginUndo();

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      m_vmultisel.ElementAt(i)->Translate(pvOffset);
   }

   EndUndo();
}

void PinTable::GetCenter(Vertex2D * const pv) const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      ISelect * const psel = m_vmultisel.ElementAt(i);
      Vertex2D vCenter;
      psel->GetCenter(&vCenter);

      minx = min(minx, vCenter.x);
      maxx = max(maxx, vCenter.x);
      miny = min(miny, vCenter.y);
      maxy = max(maxy, vCenter.y);
      //tx += m_vdpoint.ElementAt(i)->m_v.x;
      //ty += m_vdpoint.ElementAt(i)->m_v.y;
   }

   pv->x = (maxx + minx)*0.5f;
   pv->y = (maxy + miny)*0.5f;
}

void PinTable::PutCenter(const Vertex2D * const pv)
{
}

void PinTable::DoRButtonUp(int x, int y)
{
   GetSelectedItem()->OnRButtonUp(x, y);

   const int ks = GetKeyState(VK_CONTROL);

   // Only bring up context menu if we weren't in magnify mode
   if (!((g_pvp->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000)))
   {
      if (m_vmultisel.Size() > 1)
      {
         DoContextMenu(x, y, IDR_MULTIMENU, this);
      }
      else if (!MultiSelIsEmpty())
      {
         DoContextMenu(x, y, GetSelectedItem()->m_menuid, GetSelectedItem());
      }
      else
      {
         DoContextMenu(x, y, IDR_TABLEMENU, this);
      }
   }
}

void PinTable::DoMouseMove(int x, int y)
{
   Vertex2D v = TransformPoint(x, y);

   g_pvp->SetPosCur(v.x, v.y);

   if (!m_fDragging) // Not doing band select
   {
      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         m_vmultisel.ElementAt(i)->OnMouseMove(x, y);
      }
   }
   else
   {
      OnMouseMove(x, y);
   }
}

void PinTable::DoLDoubleClick(int x, int y)
{
   //g_pvp->m_sb.SetVisible(fTrue);
   //::SendMessage(g_pvp->m_hwnd, WM_SIZE, 0, 0);
}

void PinTable::ExportBlueprint()
{
   BOOL fSaveAs = fTrue;
   bool solid = false;

   if (fSaveAs)
   {
      //need to get a file name
      OPENFILENAME ofn;
      ZeroMemory(&ofn, sizeof(OPENFILENAME));
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = g_hinst;
      ofn.hwndOwner = g_pvp->m_hwnd;
      // TEXT
      ofn.lpstrFilter = "Bitmap (*.bmp)\0*.bmp\0";
      ofn.lpstrFile = m_szBlueprintFileName;
      ofn.nMaxFile = _MAX_PATH;
      ofn.lpstrDefExt = "bmp";
      ofn.Flags = OFN_OVERWRITEPROMPT;

      int ret = GetSaveFileName(&ofn);

      // user cancelled
      if (ret == 0)
         return;// S_FALSE;
   }
   const int result = ::MessageBox(g_pvp->m_hwnd, "Do you want a solid blueprint?", "Export As Solid?", MB_YESNO);
   if (result == IDYES)
      solid = true;

   HANDLE hfile = CreateFile(m_szBlueprintFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

   float tableheight, tablewidth;
   if (g_pvp->m_fBackglassView)
   {
      tablewidth = (float)EDITOR_BG_WIDTH;
      tableheight = (float)EDITOR_BG_HEIGHT;
   }
   else
   {
      tablewidth = m_right - m_left;
      tableheight = m_bottom - m_top;
   }

   int bmwidth, bmheight;
   if (tableheight > tablewidth)
   {
      bmheight = 4096;
      bmwidth = (int)((tablewidth / tableheight) * bmheight + 0.5f);
   }
   else
   {
      bmwidth = 4096;
      bmheight = (int)((tableheight / tablewidth) * bmwidth + 0.5f);
   }

   int totallinebytes = bmwidth * 3;
   totallinebytes = (((totallinebytes - 1) / 4) + 1) * 4; // make multiple of four
   const int bmlinebuffer = totallinebytes - (bmwidth * 3);

   BITMAPFILEHEADER bmfh;
   ZeroMemory(&bmfh, sizeof(bmfh));
   bmfh.bfType = 'M' << 8 | 'B';
   bmfh.bfSize = sizeof(bmfh) + sizeof(BITMAPINFOHEADER) + totallinebytes*bmheight;
   bmfh.bfOffBits = (DWORD)sizeof(bmfh) + (DWORD)sizeof(BITMAPINFOHEADER);

   DWORD foo;
   WriteFile(hfile, &bmfh, sizeof(bmfh), &foo, NULL);

   BITMAPINFO bmi;
   ZeroMemory(&bmi, sizeof(bmi));
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = bmwidth;
   bmi.bmiHeader.biHeight = bmheight;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 24;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = totallinebytes*bmheight;

   WriteFile(hfile, &bmi, sizeof(BITMAPINFOHEADER), &foo, NULL);

   HDC hdcScreen = ::GetDC(NULL);
   HDC hdc2 = CreateCompatibleDC(hdcScreen);

   char *pbits;
   HBITMAP hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

   /*const HBITMAP hbmOld =*/ (HBITMAP)SelectObject(hdc2, hdib);

   PaintSur * const psur = new PaintSur(hdc2, (float)bmwidth / tablewidth, tablewidth*0.5f, tableheight*0.5f, bmwidth, bmheight, NULL);

   SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
   PatBlt(hdc2, 0, 0, bmwidth, bmheight, PATCOPY);

   if (g_pvp->m_fBackglassView)
   {
      Render3DProjection(psur);
   }

   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->m_isVisible && ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         ptr->RenderBlueprint(psur, solid);
      }
   }

   //Render(psur);

   delete psur;

   for (int i = 0; i < bmheight; i++)
   {
      WriteFile(hfile, (pbits + ((i*bmwidth) * 3)), bmwidth * 3, &foo, NULL);
   }

   // For some reason to make our bitmap compatible with all programs,
   // We need to write out dummy bytes as if our totalwidthbytes had been
   // a multiple of 4.
   for (int i = 0; i < bmheight; i++)
   {
      for (int l = 0; l < bmlinebuffer; l++)
      {
         WriteFile(hfile, pbits, 1, &foo, NULL);
      }
   }

   DeleteDC(hdc2);
   ::ReleaseDC(NULL, hdcScreen);

   DeleteObject(hdib);

   CloseHandle(hfile);
   ::MessageBox(NULL, "Export finished!", "Info", MB_OK | MB_ICONEXCLAMATION);
}

void PinTable::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);

   Vertex3D_NoTex2 rgv[7];
   rgv[0].x = m_left;     rgv[0].y = m_top;      rgv[0].z = m_tableheight;
   rgv[1].x = m_right;    rgv[1].y = m_top;      rgv[1].z = m_tableheight;
   rgv[2].x = m_right;    rgv[2].y = m_bottom;   rgv[2].z = m_tableheight;
   rgv[3].x = m_left;     rgv[3].y = m_bottom;   rgv[3].z = m_tableheight;

   // These next 4 vertices are used just to set the extents
   rgv[4].x = m_left;     rgv[4].y = m_top;      rgv[4].z = m_tableheight + 50.0f;
   rgv[5].x = m_left;     rgv[5].y = m_bottom;   rgv[5].z = m_tableheight + 50.0f;
   rgv[6].x = m_right;    rgv[6].y = m_bottom;   rgv[6].z = m_tableheight + 50.0f;
   //rgv[7].x=g_pplayer->m_ptable->m_right;    rgv[7].y=g_pplayer->m_ptable->m_top;      rgv[7].z=50.0f;

   for (int i = 0; i < 4; ++i)
   {
      rgv[i].nx = 0;
      rgv[i].ny = 0;
      rgv[i].nz = 1.0f;

      rgv[i].tv = (i & 2) ? 1.0f : 0.f;
      rgv[i].tu = (i == 1 || i == 2) ? 1.0f : 0.f;
   }

   const WORD playfieldPolyIndices[10] = { 0, 1, 3, 0, 3, 2, 2, 3, 5, 6 };

   Vertex3D_NoTex2 *buffer = new Vertex3D_NoTex2[4 + 7];
   unsigned int offs = 0;
   for (unsigned int y = 0; y <= 1; ++y)
      for (unsigned int x = 0; x <= 1; ++x, ++offs)
      {
         buffer[offs].x = (x & 1) ? rgv[1].x : rgv[0].x;
         buffer[offs].y = (y & 1) ? rgv[2].y : rgv[0].y;
         buffer[offs].z = rgv[0].z;

         buffer[offs].tu = (x & 1) ? rgv[1].tu : rgv[0].tu;
         buffer[offs].tv = (y & 1) ? rgv[2].tv : rgv[0].tv;

         buffer[offs].nx = rgv[0].nx;
         buffer[offs].ny = rgv[0].ny;
         buffer[offs].nz = rgv[0].nz;
      }

   SetNormal(rgv, playfieldPolyIndices + 6, 4);

   WaveFrontObj_WriteObjectName(f, name);
   WaveFrontObj_WriteVertexInfo(f, buffer, 4);
   const Material * const mat = GetMaterial(m_szPlayfieldMaterial);
   WaveFrontObj_WriteMaterial(m_szPlayfieldMaterial, NULL, mat);
   WaveFrontObj_UseTexture(f, m_szPlayfieldMaterial);
   WaveFrontObj_WriteFaceInfoList(f, playfieldPolyIndices, 6);
   WaveFrontObj_UpdateFaceOffset(4);
   delete[] buffer;
}

void PinTable::ExportTableMesh()
{
   OPENFILENAME ofn;
   memset(m_szObjFileName, 0, MAX_PATH);
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_hinst;
   ofn.hwndOwner = g_pvp->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Wavefront obj(*.obj)\0*.obj\0";
   ofn.lpstrFile = m_szObjFileName;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_OVERWRITEPROMPT;

   int ret = GetSaveFileName(&ofn);

   // user canceled
   if (ret == 0)
      return;// S_FALSE;

   FILE *f = WaveFrontObj_ExportStart(m_szObjFileName);
   if (f == NULL)
   {
      ShowError("Unable to create obj file!");
      return;
   }
   ExportMesh(f);
   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->m_isVisible && ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         ptr->ExportMesh(f);
      }
   }
   WaveFrontObj_ExportEnd(f);
   ::MessageBox(NULL, "Export finished!", "Info", MB_OK | MB_ICONEXCLAMATION);

}

void PinTable::ImportBackdropPOV(const char *filename)
{
    char szFileName[1024];
    bool oldFormatLoaded = false;
    szFileName[0] = '\0';

	if (filename == NULL)
	{
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = g_hinst;
		ofn.hwndOwner = g_pvp->m_hwnd;
		// TEXT
		ofn.lpstrFilter = "POV file (*.pov)\0*.pov\0Old POV file(*.xml)\0*.xml\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = _MAX_PATH;
		ofn.lpstrDefExt = "pov";
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

		const int ret = GetOpenFileName(&ofn);
		if (ret == 0)
			return;
	}
	else
	{
		strcpy_s(szFileName, filename);
	}

    xml_document<> xmlDoc;

    try
    {
        std::stringstream buffer;
        std::ifstream myFile(szFileName);
        buffer << myFile.rdbuf();
        myFile.close();

        std::string content(buffer.str());
        xmlDoc.parse<0>(&content[0]);

        xml_node<> *root = xmlDoc.first_node("POV");
        xml_node<> *desktop = root->first_node("desktop");
        sscanf_s(desktop->first_node("inclination")->value(), "%f", &m_BG_inclination[BG_DESKTOP]);
        sscanf_s(desktop->first_node("fov")->value(), "%f", &m_BG_FOV[BG_DESKTOP]);
        sscanf_s(desktop->first_node("layback")->value(), "%f", &m_BG_layback[BG_DESKTOP]);
        sscanf_s(desktop->first_node("rotation")->value(), "%f", &m_BG_rotation[BG_DESKTOP]);
        sscanf_s(desktop->first_node("xscale")->value(), "%f", &m_BG_scalex[BG_DESKTOP]);
        sscanf_s(desktop->first_node("yscale")->value(), "%f", &m_BG_scaley[BG_DESKTOP]);
        sscanf_s(desktop->first_node("zscale")->value(), "%f", &m_BG_scalez[BG_DESKTOP]);
        sscanf_s(desktop->first_node("xoffset")->value(), "%f", &m_BG_xlatex[BG_DESKTOP]);
        sscanf_s(desktop->first_node("yoffset")->value(), "%f", &m_BG_xlatey[BG_DESKTOP]);
        sscanf_s(desktop->first_node("zoffset")->value(), "%f", &m_BG_xlatez[BG_DESKTOP]);

        xml_node<> *fullscreen = root->first_node("fullscreen");
        sscanf_s(fullscreen->first_node("inclination")->value(), "%f", &m_BG_inclination[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("fov")->value(), "%f", &m_BG_FOV[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("layback")->value(), "%f", &m_BG_layback[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("rotation")->value(), "%f", &m_BG_rotation[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("xscale")->value(), "%f", &m_BG_scalex[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("yscale")->value(), "%f", &m_BG_scaley[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("zscale")->value(), "%f", &m_BG_scalez[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("xoffset")->value(), "%f", &m_BG_xlatex[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("yoffset")->value(), "%f", &m_BG_xlatey[BG_FULLSCREEN]);
        sscanf_s(fullscreen->first_node("zoffset")->value(), "%f", &m_BG_xlatez[BG_FULLSCREEN]);
        oldFormatLoaded = true;

        xml_node<> *fullsinglescreen = root->first_node("fullsinglescreen");
        sscanf_s(fullsinglescreen->first_node("inclination")->value(), "%f", &m_BG_inclination[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("fov")->value(), "%f", &m_BG_FOV[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("layback")->value(), "%f", &m_BG_layback[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("rotation")->value(), "%f", &m_BG_rotation[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("xscale")->value(), "%f", &m_BG_scalex[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("yscale")->value(), "%f", &m_BG_scaley[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("zscale")->value(), "%f", &m_BG_scalez[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("xoffset")->value(), "%f", &m_BG_xlatex[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("yoffset")->value(), "%f", &m_BG_xlatey[BG_FSS]);
        sscanf_s(fullsinglescreen->first_node("zoffset")->value(), "%f", &m_BG_xlatez[BG_FSS]);
    }
    catch (...)
    {
       if (!oldFormatLoaded)
         ShowError("Error parsing POV XML file");
    }

    xmlDoc.clear();
}

void PinTable::ExportBackdropPOV()
{
	OPENFILENAME ofn;
	memset(m_szObjFileName, 0, MAX_PATH);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hinst;
	ofn.hwndOwner = g_pvp->m_hwnd;
	// TEXT
	ofn.lpstrFilter = "POV file(*.pov)\0*.pov\0";
	ofn.lpstrFile = m_szObjFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrDefExt = "pov";
	ofn.Flags = OFN_OVERWRITEPROMPT;

	int ret = GetSaveFileName(&ofn);

	// user canceled
	if (ret == 0)
		return;// S_FALSE;

    char strBuf[MAX_PATH];
    xml_document<> xmlDoc;
    try
    {
        xml_node<>*dcl = xmlDoc.allocate_node(node_declaration);
        dcl->append_attribute(xmlDoc.allocate_attribute("version", "1.0"));
        dcl->append_attribute(xmlDoc.allocate_attribute("encoding", "utf-8"));
        xmlDoc.append_node(dcl);

        //root node
        xml_node<>*root = xmlDoc.allocate_node(node_element, "POV");

        {
        xml_node<>*desktop = xmlDoc.allocate_node(node_element, "desktop");
        sprintf_s(strBuf, "%f", m_BG_inclination[BG_DESKTOP]);
        xml_node<>*dtIncl = xmlDoc.allocate_node(node_element, "inclination", (new string(strBuf))->c_str());
        desktop->append_node(dtIncl);
        sprintf_s(strBuf, "%f", m_BG_FOV[BG_DESKTOP]);
        xml_node<>*dtFov = xmlDoc.allocate_node(node_element, "fov", (new string(strBuf))->c_str());
        desktop->append_node(dtFov);
        sprintf_s(strBuf, "%f", m_BG_layback[BG_DESKTOP]);
        xml_node<>*dtLayback = xmlDoc.allocate_node(node_element, "layback", (new string(strBuf))->c_str());
        desktop->append_node(dtLayback);
        sprintf_s(strBuf, "%f", m_BG_rotation[BG_DESKTOP]);
        xml_node<>*dtRotation = xmlDoc.allocate_node(node_element, "rotation", (new string(strBuf))->c_str());
        desktop->append_node(dtRotation);
        sprintf_s(strBuf, "%f", m_BG_scalex[BG_DESKTOP]);
        xml_node<>*dtScalex = xmlDoc.allocate_node(node_element, "xscale", (new string(strBuf))->c_str());
        desktop->append_node(dtScalex);
        sprintf_s(strBuf, "%f", m_BG_scaley[BG_DESKTOP]);
        xml_node<>*dtScaley = xmlDoc.allocate_node(node_element, "yscale", (new string(strBuf))->c_str());
        desktop->append_node(dtScaley);
        sprintf_s(strBuf, "%f", m_BG_scalez[BG_DESKTOP]);
        xml_node<>*dtScalez = xmlDoc.allocate_node(node_element, "zscale", (new string(strBuf))->c_str());
        desktop->append_node(dtScalez);
        sprintf_s(strBuf, "%f", m_BG_xlatex[BG_DESKTOP]);
        xml_node<>*dtOffsetx = xmlDoc.allocate_node(node_element, "xoffset", (new string(strBuf))->c_str());
        desktop->append_node(dtOffsetx);
        sprintf_s(strBuf, "%f", m_BG_xlatey[BG_DESKTOP]);
        xml_node<>*dtOffsety = xmlDoc.allocate_node(node_element, "yoffset", (new string(strBuf))->c_str());
        desktop->append_node(dtOffsety);
        sprintf_s(strBuf, "%f", m_BG_xlatez[BG_DESKTOP]);
        xml_node<>*dtOffsetz = xmlDoc.allocate_node(node_element, "zoffset", (new string(strBuf))->c_str());
        desktop->append_node(dtOffsetz);

        root->append_node(desktop);
        }
        {
        xml_node<>*fullscreen = xmlDoc.allocate_node(node_element, "fullscreen");
        sprintf_s(strBuf, "%f", m_BG_inclination[BG_FULLSCREEN]);
        xml_node<>*fsIncl = xmlDoc.allocate_node(node_element, "inclination", (new string(strBuf))->c_str());
        fullscreen->append_node(fsIncl);
        sprintf_s(strBuf, "%f", m_BG_FOV[BG_FULLSCREEN]);
        xml_node<>*fsFov = xmlDoc.allocate_node(node_element, "fov", (new string(strBuf))->c_str());
        fullscreen->append_node(fsFov);
        sprintf_s(strBuf, "%f", m_BG_layback[BG_FULLSCREEN]);
        xml_node<>*fsLayback = xmlDoc.allocate_node(node_element, "layback", (new string(strBuf))->c_str());
        fullscreen->append_node(fsLayback);
        sprintf_s(strBuf, "%f", m_BG_rotation[BG_FULLSCREEN]);
        xml_node<>*fsRotation = xmlDoc.allocate_node(node_element, "rotation", (new string(strBuf))->c_str());
        fullscreen->append_node(fsRotation);
        sprintf_s(strBuf, "%f", m_BG_scalex[BG_FULLSCREEN]);
        xml_node<>*fsScalex = xmlDoc.allocate_node(node_element, "xscale", (new string(strBuf))->c_str());
        fullscreen->append_node(fsScalex);
        sprintf_s(strBuf, "%f", m_BG_scaley[BG_FULLSCREEN]);
        xml_node<>*fsScaley = xmlDoc.allocate_node(node_element, "yscale", (new string(strBuf))->c_str());
        fullscreen->append_node(fsScaley);
        sprintf_s(strBuf, "%f", m_BG_scalez[BG_FULLSCREEN]);
        xml_node<>*fsScalez = xmlDoc.allocate_node(node_element, "zscale", (new string(strBuf))->c_str());
        fullscreen->append_node(fsScalez);
        sprintf_s(strBuf, "%f", m_BG_xlatex[BG_FULLSCREEN]);
        xml_node<>*fsOffsetx = xmlDoc.allocate_node(node_element, "xoffset", (new string(strBuf))->c_str());
        fullscreen->append_node(fsOffsetx);
        sprintf_s(strBuf, "%f", m_BG_xlatey[BG_FULLSCREEN]);
        xml_node<>*fsOffsety = xmlDoc.allocate_node(node_element, "yoffset", (new string(strBuf))->c_str());
        fullscreen->append_node(fsOffsety);
        sprintf_s(strBuf, "%f", m_BG_xlatez[BG_FULLSCREEN]);
        xml_node<>*fsOffsetz = xmlDoc.allocate_node(node_element, "zoffset", (new string(strBuf))->c_str());
        fullscreen->append_node(fsOffsetz);

        root->append_node(fullscreen);
        }
        {
        xml_node<>*fullsinglescreen = xmlDoc.allocate_node(node_element, "fullsinglescreen");
        sprintf_s(strBuf, "%f", m_BG_inclination[BG_FSS]);
        xml_node<>*fssIncl = xmlDoc.allocate_node(node_element, "inclination", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssIncl);
        sprintf_s(strBuf, "%f", m_BG_FOV[BG_FSS]);
        xml_node<>*fssFov = xmlDoc.allocate_node(node_element, "fov", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssFov);
        sprintf_s(strBuf, "%f", m_BG_layback[BG_FSS]);
        xml_node<>*fssLayback = xmlDoc.allocate_node(node_element, "layback", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssLayback);
        sprintf_s(strBuf, "%f", m_BG_rotation[BG_FSS]);
        xml_node<>*fssRotation = xmlDoc.allocate_node(node_element, "rotation", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssRotation);
        sprintf_s(strBuf, "%f", m_BG_scalex[BG_FSS]);
        xml_node<>*fssScalex = xmlDoc.allocate_node(node_element, "xscale", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssScalex);
        sprintf_s(strBuf, "%f", m_BG_scaley[BG_FSS]);
        xml_node<>*fssScaley = xmlDoc.allocate_node(node_element, "yscale", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssScaley);
        sprintf_s(strBuf, "%f", m_BG_scalez[BG_FSS]);
        xml_node<>*fssScalez = xmlDoc.allocate_node(node_element, "zscale", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssScalez);
        sprintf_s(strBuf, "%f", m_BG_xlatex[BG_FSS]);
        xml_node<>*fssOffsetx = xmlDoc.allocate_node(node_element, "xoffset", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssOffsetx);
        sprintf_s(strBuf, "%f", m_BG_xlatey[BG_FSS]);
        xml_node<>*fssOffsety = xmlDoc.allocate_node(node_element, "yoffset", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssOffsety);
        sprintf_s(strBuf, "%f", m_BG_xlatez[BG_FSS]);
        xml_node<>*fssOffsetz = xmlDoc.allocate_node(node_element, "zoffset", (new string(strBuf))->c_str());
        fullsinglescreen->append_node(fssOffsetz);

        root->append_node(fullsinglescreen);
        }
        xmlDoc.append_node(root);
        std::ofstream myfile(m_szObjFileName);
        myfile << xmlDoc;
        myfile.close();
    }
    catch (...)
    {
        ShowError("Error exporting POV settings!");
    }
    xmlDoc.clear();
}

void PinTable::SelectItem(IScriptable *piscript)
{
   ISelect * const pisel = piscript->GetISelect();
   if (pisel)
   {
      AddMultiSel(pisel, false);
   }
}

void PinTable::DoCodeViewCommand(int command)
{
   switch (command)
   {
   case ID_SAVE:
      // added by chris as part of table protection
      if (!CheckPermissions(DISABLE_TABLE_SAVE))
      {
         TableSave();
      }
      break;
   case ID_TABLE_CAMERAMODE:
   {
      Play(true);
      break;
   }
   case ID_TABLE_PLAY:
      Play(false);
      break;
   }
}

void PinTable::SetDirtyScript(SaveDirtyState sds)
{
   m_sdsDirtyScript = sds;
   CheckDirty();
}

void PinTable::SetDirty(SaveDirtyState sds)
{
   m_sdsDirtyProp = sds;
   CheckDirty();
}

void PinTable::SetNonUndoableDirty(SaveDirtyState sds)
{
   m_sdsNonUndoableDirty = sds;
   CheckDirty();
}

void PinTable::CheckDirty()
{
   SaveDirtyState sdsNewDirtyState = (SaveDirtyState)max(max((int)m_sdsDirtyProp, (int)m_sdsDirtyScript), (int)m_sdsNonUndoableDirty);

   if (sdsNewDirtyState != m_sdsCurrentDirtyState)
   {
      if (sdsNewDirtyState > eSaveClean)
      {
         char szWindowName[MAX_LINE_LENGTH + 1];
         lstrcpy(szWindowName, m_szTitle);
         lstrcat(szWindowName, "*");

         SetCaption(szWindowName);
      }
      else
      {
         SetCaption(m_szTitle);
      }
   }

   m_sdsCurrentDirtyState = sdsNewDirtyState;
}

BOOL PinTable::FDirty()
{
   return (m_sdsCurrentDirtyState > eSaveClean);
}

void PinTable::BeginUndo()
{
   m_undo.BeginUndo();
}

void PinTable::EndUndo()
{
   m_undo.EndUndo();
}

void PinTable::Undo()
{
   m_undo.Undo();

   SetDirtyDraw();
   if (m_searchSelectDlg.IsWindow())
   {
      m_searchSelectDlg.Update();
   }
   g_pvp->m_sb.RefreshProperties();
}

void PinTable::Uncreate(IEditable *pie)
{
   if (pie->GetISelect()->m_selectstate != eNotSelected)
   {
      AddMultiSel(pie->GetISelect(), true); // Remove the item from the multi-select list
   }

   pie->GetISelect()->Uncreate();
   pie->Release();
}

void PinTable::Undelete(IEditable *pie)
{
   m_vedit.AddElement(pie);
   pie->Undelete();
   SetDirtyDraw();
}

void PinTable::BackupForPlay()
{
   m_undo.BeginUndo();

   m_undo.MarkForUndo((IEditable *)this);
   for (int i = 0; i < m_vedit.Size(); i++)
   {
      m_undo.MarkForUndo(m_vedit.ElementAt(i));
   }

   m_undo.EndUndo();
}

void PinTable::RestoreBackup()
{
   m_undo.Undo();
}

void PinTable::Copy(int x, int y)
{
   ULONG writ = 0;

   if (MultiSelIsEmpty()) // Can't copy table
   {
      return;
   }

   if(m_vmultisel.Size() == 1)
   {
       // special check if the user selected a Control Point and wants to copy the coordinates
       ISelect *pItem = HitTest(x, y);
       if(pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint *pPoint = (DragPoint*)pItem;
           pPoint->Copy();
           return;
       }
   }

   Vector<IStream> vstm;
   //m_vstmclipboard
   for(int i = 0; i < m_vmultisel.Size(); i++)
   {
       HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, 1);

       IStream *pstm;
       CreateStreamOnHGlobal(hglobal, TRUE, &pstm);

       IEditable *pe = m_vmultisel.ElementAt(i)->GetIEditable();

       ////////!! BUG!  With multi-select, if you have multiple dragpoints on
       //////// a surface selected, the surface will get copied multiple times
       const int type = pe->GetItemType();
       pstm->Write(&type, sizeof(int), &writ);

       pe->SaveData(pstm, NULL, NULL);

       vstm.AddElement(pstm);
   }

   g_pvp->SetClipboard(&vstm);
}

void PinTable::Paste(BOOL fAtLocation, int x, int y)
{
   BOOL fError = fFalse;
   int cpasted = 0;

   if (CheckPermissions(DISABLE_CUTCOPYPASTE))
   {
      g_pvp->ShowPermissionError();
      return;
   }

   if(m_vmultisel.Size() == 1)
   {
       // User wants to paste the copied coordinates of a Control Point
       ISelect *pItem = HitTest(x, y);
       if(pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint *pPoint = (DragPoint*)pItem;
           pPoint->Paste();
           SetDirtyDraw();
           return;
       }
   }

   const unsigned viewflag = (g_pvp->m_fBackglassView ? VIEW_BACKGLASS : VIEW_PLAYFIELD);

   IStream* pstm;

   // Do a backwards loop, so that the primary selection we had when
   // copying will again be the primary selection, since it will be
   // selected last.  Purely cosmetic.
   for (int i = (g_pvp->m_vstmclipboard.Size() - 1); i >= 0; i--)
      //for (i=0;i<g_pvp->m_vstmclipboard.Size();i++)
   {
      pstm = g_pvp->m_vstmclipboard.ElementAt(i);

      // Go back to beginning of stream to load
      LARGE_INTEGER foo;
      foo.QuadPart = 0;
      pstm->Seek(foo, STREAM_SEEK_SET, NULL);

      ULONG writ = 0;
      ItemTypeEnum type;
      /*const HRESULT hr =*/ pstm->Read(&type, sizeof(int), &writ);

      if (!(EditableRegistry::GetAllowedViews(type) & viewflag))
      {
         fError = fTrue;
      }
      else
      {
         IEditable *peditNew = EditableRegistry::Create(type);

         int id;
         peditNew->InitLoad(pstm, this, &id, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);

         if (type != eItemDecal)
         {
            GetUniqueNamePasting(type, peditNew->GetScriptable()->m_wzName);
            peditNew->InitVBA(fTrue, 0, peditNew->GetScriptable()->m_wzName);
         }

         m_vedit.AddElement(peditNew);
         // copy the new element to the same layer as the source element
         m_layer[peditNew->GetISelect()->layerIndex].AddElement(peditNew);
         peditNew->InitPostLoad();
         peditNew->m_fBackglass = g_pvp->m_fBackglassView;

         AddMultiSel(peditNew->GetISelect(), (i == g_pvp->m_vstmclipboard.Size() - 1) ? false : true);
         cpasted++;
      }
   }

   // Center view on newly created objects, if they are off the screen
   if (cpasted > 0)
   {
      Vertex2D vcenter;
      GetCenter(&vcenter);
   }

   if ((cpasted > 0) && fAtLocation)
   {
      Vertex2D vcenter;
      GetCenter(&vcenter);

      Vertex2D vPos = TransformPoint(x, y);
      Vertex2D vOffset = vPos - vcenter;
      Translate(&vOffset);
   }

   if (fError)
   {
      LocalString ls(IDS_NOPASTEINVIEW);
      ::MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", 0);
   }
}

void PinTable::PreRender(Sur * const psur)
{
}

HRESULT PinTable::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   int csubobj, csounds, ctextures, cfonts, ccollection;

   LoadData(pstm, csubobj, csounds, ctextures, cfonts, ccollection, version, hcrypthash, hcryptkey);

   return S_OK;
}

HRESULT PinTable::InitPostLoad()
{
   return S_OK;
}

HRESULT PinTable::InitVBA(BOOL fNew, int id, WCHAR *wzName)
{
   return S_OK;
}

ISelect *PinTable::GetISelect()
{
   return (ISelect *)this;
}

void PinTable::SetDefaults(bool fromMouseClick)
{
}

void PinTable::SetDefaultPhysics(bool fromMouseClick)
{
   m_Gravity = 0.97f*GRAVITYCONST;

   m_friction = DEFAULT_TABLE_CONTACTFRICTION;
   m_elasticity = DEFAULT_TABLE_ELASTICITY;
   m_elasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;
   m_scatter = DEFAULT_TABLE_PFSCATTERANGLE;
}

IScriptable *PinTable::GetScriptable()
{
   return (IScriptable *)this;
}

void PinTable::ClearMultiSel(ISelect* newSel)
{
   for (int i = 0; i < m_vmultisel.Size(); i++)
      m_vmultisel.ElementAt(i)->m_selectstate = eNotSelected;

   //remove the clone of the multi selection in the smart browser class
   //to sync the clone and the actual multi-selection 
   //it will be updated again on AddMultiSel() call
   g_pvp->DeletePropSel();
   m_vmultisel.RemoveAllElements();

   if (newSel == NULL)
      newSel = this;
   m_vmultisel.AddElement(newSel);
   newSel->m_selectstate = eSelected;
}

bool PinTable::MultiSelIsEmpty()
{
   // empty selection means only the table itself is selected
   return (m_vmultisel.Size() == 1 && m_vmultisel.ElementAt(0) == this);
}

// fUpdate tells us whether to go ahead and change the UI
// based on the new selection, or whether more stuff is coming
// down the pipe (speeds up drag-selection)
void PinTable::AddMultiSel(ISelect *psel, bool fAdd, bool fUpdate, bool fContextClick)
{
   int index = m_vmultisel.IndexOf(psel);
   ISelect *piSelect = NULL;
   //_ASSERTE(m_vmultisel.ElementAt(0)->m_selectstate == eSelected);

   if (index == -1) // If we aren't selected yet, do that
   {
      _ASSERTE(psel->m_selectstate == eNotSelected);
      // If we non-shift click on an element outside the multi-select group, delete the old group
      // If the table is currently selected, deselect it - the table can not be part of a multi-select
      if (!fAdd || MultiSelIsEmpty())
      {
         ClearMultiSel(psel);
         if (!fAdd && !fContextClick)
         {
            int colIndex = -1;
            int elemIndex = -1;
            if (GetCollectionIndex(psel, colIndex, elemIndex))
            {
               CComObject<Collection> *col = m_vcollection.ElementAt(colIndex);
               if (col->m_fGroupElements)
               {
                  for (int i = 0; i < col->m_visel.size(); i++)
                  {
                     col->m_visel[i].m_selectstate = eMultiSelected;
                     // current element is already in m_vmultisel. (ClearMultiSel(psel) added it
                     if (col->m_visel.ElementAt(i) != psel)
                        m_vmultisel.AddElement(&col->m_visel[i]);
                  }
               }
            }
         }
      }
      else
      {
         // Make this new selection the primary one for the group
         piSelect = m_vmultisel.ElementAt(0);
         if ( piSelect!=NULL )
            piSelect->m_selectstate = eMultiSelected;
         m_vmultisel.InsertElementAt(psel, 0);
      }

      psel->m_selectstate = eSelected;

      if (fUpdate)
         SetDirtyDraw();
   }
   else if (fAdd) // Take the element off the list
   {
      _ASSERTE(psel->m_selectstate != eNotSelected);
      m_vmultisel.RemoveElementAt(index);
      psel->m_selectstate = eNotSelected;
      if (m_vmultisel.Size() == 0)
      {
         // Have to have something selected
         m_vmultisel.AddElement((ISelect *)this);
      }
      // The main element might have changed
      piSelect = m_vmultisel.ElementAt(0);
      if (piSelect != NULL)
         piSelect->m_selectstate = eSelected;

      if (fUpdate)
         SetDirtyDraw();
   }
   else if (m_vmultisel.ElementAt(0) != psel) // Object already in list - no change to selection, only to primary
   {
      int colIndex = -1;
      int elemIndex = -1;
      if (!GetCollectionIndex(psel, colIndex, elemIndex))
      {
         _ASSERTE(psel->m_selectstate != eNotSelected);

         // Make this new selection the primary one for the group
         piSelect = m_vmultisel.ElementAt(0);
         if (piSelect != NULL)
            piSelect->m_selectstate = eMultiSelected;
         m_vmultisel.RemoveElementAt(index);
         m_vmultisel.InsertElementAt(psel, 0);

         psel->m_selectstate = eSelected;
      }
      else
         ClearMultiSel(psel);

      if (fUpdate)
         SetDirtyDraw();
   }

   if (fUpdate)
   {
      g_pvp->SetPropSel(&m_vmultisel);
   }

   piSelect = m_vmultisel.ElementAt(0);
   if (piSelect && piSelect->GetIEditable() && piSelect->GetIEditable()->GetScriptable())
   {
      string info = string("Layer ") + to_string((long long)piSelect->layerIndex+1);
      if (piSelect->GetItemType() == eItemPrimitive)
      {
         Primitive *prim = (Primitive*)piSelect;
         if (prim->m_mesh.m_animationFrames.size() > 0)
            info = info + " (animated " + to_string((unsigned long long)prim->m_mesh.m_animationFrames.size() - 1) + " frames)";
      }
      g_pvp->SetStatusBarElementInfo(info.c_str());
      m_pcv->SelectItem(piSelect->GetIEditable()->GetScriptable());
   }
}

void PinTable::OnDelete()
{
   Vector<ISelect> m_vseldelete;

   for (int i = 0; i < m_vmultisel.Size(); i++)
   {
      // Can't delete these items yet - ClearMultiSel() will try to mark them as unselected
      m_vseldelete.AddElement(m_vmultisel.ElementAt(i));
   }

   ClearMultiSel();

   bool inCollection = false;
   for (int t = 0; t < m_vseldelete.Size() && !inCollection; t++)
   {
      ISelect *ptr = m_vseldelete.ElementAt(t);
      for (int i = 0; i < m_vcollection.size() && !inCollection; i++)
      {
         for (int k = 0; k < m_vcollection.ElementAt(i)->m_visel.Size(); k++)
         {
            if (ptr == m_vcollection.ElementAt(i)->m_visel.ElementAt(k))
            {
               inCollection = true;
               break;
            }
         }
      }
   }
   if (inCollection)
   {
      LocalString ls(IDS_DELETE_ELEMENTS);
      const int ans = MessageBox(m_hwnd, ls.m_szbuffer/*"Selected elements are part of one or more collections.\nDo you really want to delete them?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
      if (ans != IDYES)
      {
         return;
      }
   }

   for (int i = 0; i < m_vseldelete.Size(); i++)
   {
      DeleteFromLayer(m_vseldelete.ElementAt(i)->GetIEditable());
      m_vseldelete.ElementAt(i)->Delete();
   }
   // update properties to show the properties of the table
   g_pvp->SetPropSel(&m_vmultisel);
   if (m_searchSelectDlg.IsWindow())
      m_searchSelectDlg.Update();
   SetDirtyDraw();
}

void PinTable::OnKeyDown(int key)
{
   const int fShift = GetKeyState(VK_SHIFT) & 0x8000;
   //const int fCtrl = GetKeyState(VK_CONTROL) & 0x8000;
   //const int fAlt = GetKeyState(VK_MENU) & 0x8000;

   switch (key)
   {
   case VK_DELETE:
   {
      OnDelete();
   }
   break;

   case VK_LEFT:
   case VK_RIGHT:
   case VK_UP:
   case VK_DOWN:
   {
      BeginUndo();
      const int distance = fShift ? 10 : 1;
      for (int i = 0; i < m_vmultisel.Size(); i++)
      {
         ISelect *const pisel = m_vmultisel.ElementAt(i);
         if (!pisel->GetIEditable()->GetISelect()->m_fLocked) // control points get lock info from parent - UNDONE - make this code snippet be in one place
         {
            switch (key)
            {
            case VK_LEFT:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(-distance / m_zoom, 0);
               break;

            case VK_RIGHT:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(distance / m_zoom, 0);
               break;

            case VK_UP:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(0, -distance / m_zoom);
               break;

            case VK_DOWN:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(0, distance / m_zoom);
               break;
            }
         }
         g_pvp->m_sb.RefreshProperties();    // update position fields
      }
      EndUndo();
   }
   break;
   }
}

void PinTable::UseTool(int x, int y, int tool)
{
   Vertex2D v = TransformPoint(x, y);
   IEditable *pie = NULL;

    ItemTypeEnum type = EditableRegistry::TypeFromToolID(tool);
    pie = EditableRegistry::CreateAndInit(type, this, v.x, v.y);

   if (pie)
   {
      pie->m_fBackglass = g_pvp->m_fBackglassView;
      m_vedit.AddElement(pie);
      AddMultiSel(pie->GetISelect(), false);
      if (m_searchSelectDlg.IsWindow())
         m_searchSelectDlg.Update();
      BeginUndo();
      m_undo.MarkForCreate(pie);
      EndUndo();
   }

   g_pvp->ParseCommand(IDC_SELECT, g_pvp->m_hwnd, 0);
}

Vertex2D PinTable::TransformPoint(int x, int y) const
{
   Vertex2D result;
   RECT rc;
   ::GetClientRect(m_hwnd, &rc);

   HitSur * const phs = new HitSur(NULL, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

   result = phs->ScreenToSurface(x, y);

   delete phs;
   return result;
}

void PinTable::OnLButtonDown(int x, int y)
{
   Vertex2D v = TransformPoint(x, y);

   m_rcDragRect.left = v.x;
   m_rcDragRect.right = v.x;
   m_rcDragRect.top = v.y;
   m_rcDragRect.bottom = v.y;

   m_fDragging = true;

   ::SetCapture(GetPTable()->m_hwnd);

   SetDirtyDraw();
}

void PinTable::OnLButtonUp(int x, int y)
{
   if (m_fDragging)
   {
      m_fDragging = false;
      ReleaseCapture();
      if ((m_rcDragRect.left != m_rcDragRect.right) || (m_rcDragRect.top != m_rcDragRect.bottom))
      {
         Vector<ISelect> vsel;

         HDC hdc = ::GetDC(m_hwnd);

         RECT rc;
         ::GetClientRect(m_hwnd, &rc);

         HitRectSur * const phrs = new HitRectSur(hdc, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, &m_rcDragRect, &vsel);

         // Just want one rendering pass (no PreRender) so we don't select things twice
         Render(phrs);

         const int ksshift = GetKeyState(VK_SHIFT);
         const bool fAdd = ((ksshift & 0x80000000) != 0);
         if (!fAdd)
            ClearMultiSel();

         int minlevel = INT_MAX;

         for (int i = 0; i < vsel.Size(); i++)
         {
            minlevel = min(minlevel, vsel.ElementAt(i)->GetSelectLevel());
         }

         if (vsel.Size() > 0)
         {
            int lastItemForUpdate = -1;
            //first check which item is the last item to add to the multi selection
            for (int i = 0; i < vsel.Size(); i++)
            {
               if (vsel.ElementAt(i)->GetSelectLevel() == minlevel)
               {
                  lastItemForUpdate = i;
               }
            }

            for (int i = 0; i < vsel.Size(); i++)
            {
               if (vsel.ElementAt(i)->GetSelectLevel() == minlevel)
               {
                  AddMultiSel(vsel.ElementAt(i), true, (i == lastItemForUpdate)); //last item updates the (multi-)selection in the editor
               }
            }
         }

         delete phrs;

         ::ReleaseDC(m_hwnd, hdc);
      }
   }

   SetDirtyDraw();
}

void PinTable::OnMouseMove(int x, int y)
{
   Vertex2D v = TransformPoint(x, y);

   m_rcDragRect.right = v.x;
   m_rcDragRect.bottom = v.y;

   if (m_fDragging)
   {
      SetDirtyDraw();
   }
}

HRESULT PinTable::GetTypeName(BSTR *pVal)
{
   int stringid = (!g_pvp->m_fBackglassView) ? IDS_TABLE : IDS_TB_BACKGLASS;

   LocalStringW ls(stringid);
   *pVal = SysAllocString(ls.str);

   return S_OK;
}

void PinTable::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   if (!g_pvp->m_fBackglassView)
   {
      PropertyPane *pproppane;

      pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_USER, IDS_TABLE_USER);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_VISUALS, IDS_PLAYFIELD);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_BALL, IDS_DEFAULTBALL);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_PHYSICS, IDS_DIMENSIONSSLOPE);
      pvproppane->AddElement(pproppane);

	  pproppane = new PropertyPane(IDD_PROPTABLE_PHYSICS2, IDS_PHYSICS);
	  pvproppane->AddElement(pproppane);
	  
	  pproppane = new PropertyPane(IDD_PROPTABLE_LIGHTSOURCES, IDS_LIGHTSOURCES);
      pvproppane->AddElement(pproppane);
   }
   else
   {
      PropertyPane *pproppane;

      pproppane = new PropertyPane(IDD_PROPBACKGLASS_VISUALS, IDS_VISUALS2);
      pvproppane->AddElement(pproppane);
   }
}

LRESULT CALLBACK TableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CComObject<PinTable> *pt;

    switch(uMsg)
    {
        case WM_CLOSE:
        {
            ::KillTimer(hwnd, TIMER_ID_AUTOSAVE);
            ::SetTimer(hwnd, TIMER_ID_CLOSE_TABLE, 100, NULL);	//wait 250 milliseconds

            return 0;	// destroy the WM_CLOSE message
        }
        case WM_TIMER:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            switch(wParam)
            {
                case TIMER_ID_AUTOSAVE:
                {
                    pt->AutoSave();
                    break;
                }

                case TIMER_ID_CLOSE_TABLE:
                {
                    ::KillTimer(hwnd, TIMER_ID_CLOSE_TABLE);
                    pt->m_pvp->CloseTable(pt);
                    //DestroyWindow(hwnd);
                    return 0;
                    break;
                }
            }
        }
        break;

        case WM_SETCURSOR:
        {
            if(LOWORD(lParam) == HTCLIENT)
            {
                char *cursorid;
                HINSTANCE hinst = g_hinst;

                if(g_pvp->m_ToolCur == ID_TABLE_MAGNIFY)
                {
                    cursorid = MAKEINTRESOURCE(IDC_MAGNIFY);
                }
                else if(g_pvp->m_ToolCur == ID_INSERT_TARGET)
                {
                    // special case for targets, which are particular walls
                    cursorid = MAKEINTRESOURCE(IDC_TARGET);
                }
                else
                {
                    ItemTypeEnum type = EditableRegistry::TypeFromToolID(g_pvp->m_ToolCur);
                    if(type != eItemInvalid)
                        cursorid = MAKEINTRESOURCE(EditableRegistry::GetCursorID(type));
                    else
                    {
                        hinst = NULL;
                        cursorid = IDC_ARROW;
                    }
                }
                HCURSOR hcursor = LoadCursor(hinst, cursorid);
                SetCursor(hcursor);
                return TRUE;
            }
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->Paint(hdc);
            ::EndPaint(hwnd, &ps);
            break;
        }

        case WM_MOUSEACTIVATE:
        case WM_ACTIVATE:
        if(LOWORD(wParam) != WA_INACTIVE)
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->m_pvp->m_ptableActive = pt;

            // re-evaluate the toolbar depending on table permissions
            g_pvp->SetEnableToolbar();
        }
        break;

        case WM_LBUTTONDOWN:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            if((g_pvp->m_ToolCur == IDC_SELECT) || (g_pvp->m_ToolCur == ID_TABLE_MAGNIFY))
            {
                pt->DoLButtonDown(x, y);
            }
            else
            {
                pt->UseTool(x, y, g_pvp->m_ToolCur);
            }
            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            pt->DoLDoubleClick(x, y);
            break;
        }

        case WM_LBUTTONUP:
        {
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->DoLButtonUp(x, y);
        }
        break;

        case WM_MOUSEMOVE:
        {
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const BOOL middleMouseButtonPressed = ((GetKeyState(VK_MBUTTON) & 0x100) != 0);  //((GetKeyState(VK_MENU) & 0x80000000) != 0);
            if(middleMouseButtonPressed)
            {
                // panning feature starts here...if the user holds the middle mouse button and moves the mouse 
                // everything is moved in the direction of the mouse was moved
                int dx = abs(pt->m_oldMousePosX - x);
                int dy = abs(pt->m_oldMousePosY - y);
                if(pt->m_oldMousePosX > x)  pt->m_offset.x += dx;
                if(pt->m_oldMousePosX < x)  pt->m_offset.x -= dx;
                if(pt->m_oldMousePosY > y)  pt->m_offset.y += dy;
                if(pt->m_oldMousePosY < y)  pt->m_offset.y -= dy;
                pt->SetDirtyDraw();
                pt->SetMyScrollInfo();
                pt->m_oldMousePosX = x;
                pt->m_oldMousePosY = y;
                break;
            }
            pt->DoMouseMove(x, y);
            pt->m_oldMousePosX = x;
            pt->m_oldMousePosY = y;
        }
        break;

        case WM_RBUTTONDOWN:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short x = (short)(lParam & 0xffff);
            const short y = (short)((lParam >> 16) & 0xffff);

            pt->DoRButtonDown(x, y);
            break;
        }
        case WM_CONTEXTMENU:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            long x = (long)(lParam & 0xffff);
            long y = (long)((lParam >> 16) & 0xffff);
            POINT p;
            if(GetCursorPos(&p) && ::ScreenToClient(hwnd, &p))
            {
                x = p.x;
                y = p.y;
            }
            pt->DoRButtonUp(x, y);
            break;
        }
        case WM_KEYDOWN:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            pt->OnKeyDown((int)wParam);
            break;
        }

        case WM_HSCROLL:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            SCROLLINFO si;
            ZeroMemory(&si, sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            ::GetScrollInfo(hwnd, SB_HORZ, &si);
            switch(LOWORD(wParam))
            {
                case SB_LINELEFT:
                pt->m_offset.x -= si.nPage / 10;
                break;
                case SB_LINERIGHT:
                pt->m_offset.x += si.nPage / 10;
                break;
                case SB_PAGELEFT:
                pt->m_offset.x -= si.nPage / 2;
                break;
                case SB_PAGERIGHT:
                pt->m_offset.x += si.nPage / 2;
                break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(pt->m_offset.x - si.nPos);
                    pt->m_offset.x = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            pt->SetDirtyDraw();
            pt->SetMyScrollInfo();
            return 0;
        }
        break;

        case WM_VSCROLL:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            SCROLLINFO si;
            ZeroMemory(&si, sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            ::GetScrollInfo(hwnd, SB_VERT, &si);
            switch(LOWORD(wParam))
            {
                case SB_LINEUP:
                pt->m_offset.y -= si.nPage / 10;
                break;
                case SB_LINEDOWN:
                pt->m_offset.y += si.nPage / 10;
                break;
                case SB_PAGEUP:
                pt->m_offset.y -= si.nPage / 2;
                break;
                case SB_PAGEDOWN:
                pt->m_offset.y += si.nPage / 2;
                break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(pt->m_offset.y - si.nPos);
                    pt->m_offset.y = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            pt->SetDirtyDraw();
            pt->SetMyScrollInfo();
            return 0;
        }
        break;

        case WM_MOUSEWHEEL:
        {
            //zoom in/out by pressing CTRL+mouse wheel
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            const short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            const int ksctrl = GetKeyState(VK_CONTROL);
            if((ksctrl & 0x80000000))
            {
                POINT curpt;
                curpt.x = GET_X_LPARAM(lParam);
                curpt.y = GET_Y_LPARAM(lParam);
                ::ScreenToClient(hwnd, &curpt);
                const short x = (short)curpt.x;
                const short y = (short)curpt.y;
                if((g_pvp->m_ToolCur == IDC_SELECT) || (g_pvp->m_ToolCur == ID_TABLE_MAGNIFY))
                {
                    pt->DoLButtonDown(x, y, zDelta != -120);
                }
            }
            else
            {
                pt->m_offset.y -= zDelta / pt->m_zoom;	// change to orientation to match windows default
                pt->SetDirtyDraw();
                pt->SetMyScrollInfo();
            }
            return 0;
        }
        break;

        case WM_SIZE:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(pt) // Window might have just been created
            {
                pt->SetMyScrollInfo();
                pt->m_fDirtyDraw = true;
                // this window command is called whenever the MDI window changes over
                // re-evaluate the toolbar depending on table permissions
                g_pvp->SetEnableToolbar();
            }
            break;
        }

        case WM_COMMAND:
        break;

        case DONE_AUTOSAVE:
        {
            pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if(lParam == S_OK)
            {
                g_pvp->SetActionCur("");
            }
            else
            {
                g_pvp->SetActionCur("Autosave Failed");
            }
            pt->BeginAutoSaveCounter();
            HANDLE hEvent = (HANDLE)wParam;
            RemoveFromVector(pt->m_vAsyncHandles, hEvent);
            CloseHandle(hEvent);
        }
        break;
    }
    return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
}

STDMETHODIMP PinTable::get_FileName(BSTR *pVal)
{
   WCHAR *wz = MakeWide(m_szTitle);
   *pVal = SysAllocString(wz);
   delete[] wz;

   return S_OK;
}

STDMETHODIMP PinTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString((WCHAR *)m_wzName);

   return S_OK;
}

STDMETHODIMP PinTable::put_Name(BSTR newVal)
{
   //GetIApcProjectItem()->put_Name(newVal);

   STARTUNDO

      const int l = lstrlenW(newVal);
   if ((l > 32) || (l < 1))
   {
      return E_FAIL;
   }

   if (m_pcv->ReplaceName((IScriptable *)this, newVal) == S_OK)
   {
      WideStrNCopy(newVal, (WCHAR *)m_wzName, MAXNAMEBUFFER);
      //lstrcpyW((WCHAR *)m_wzName, newVal);
   }

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_MaxSeparation(float *pVal)
{
   if (m_overwriteGlobalStereo3D)
      *pVal = m_3DmaxSeparation;
   else
      *pVal = m_global3DMaxSeparation;

   return S_OK;
}

STDMETHODIMP PinTable::put_MaxSeparation(float newVal)
{
   STARTUNDO

   if (m_overwriteGlobalStereo3D)
      m_3DmaxSeparation = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ZPD(float *pVal)
{
   if (m_overwriteGlobalStereo3D)
      *pVal = m_3DZPD;
   else
      *pVal = m_global3DZPD;

   return S_OK;
}

STDMETHODIMP PinTable::put_ZPD(float newVal)
{
   STARTUNDO

   if (m_overwriteGlobalStereo3D)
      m_3DZPD = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Offset(float *pVal)
{
	if (m_overwriteGlobalStereo3D)
		*pVal = m_3DOffset;
	else
		*pVal = m_global3DOffset;

	return S_OK;
}

STDMETHODIMP PinTable::put_Offset(float newVal)
{
	STARTUNDO

	if (m_overwriteGlobalStereo3D)
		m_3DOffset = newVal;

	STOPUNDO

	return S_OK;
}

void PinTable::ClearOldSounds()
{
   for (int i = 0; i < m_voldsound.Size(); i++)
   {
      //LPDIRECTSOUNDBUFFER pdsbOld = (LPDIRECTSOUNDBUFFER)(m_voldsound.ElementAt(i));
      PinSoundCopy * const ppsc = m_voldsound.ElementAt(i);
      DWORD status;
      ppsc->m_pDSBuffer->GetStatus(&status);
      if (!(status & DSBSTATUS_PLAYING)) //sound is done, we can throw it away now
      {
         ppsc->m_pDSBuffer->Release();
         m_voldsound.RemoveElementAt(i);
         delete ppsc;
         i--; // elements will shift, check this one again
      }
   }
}

HRESULT PinTable::StopSound(BSTR Sound)
{
   MAKE_ANSIPTR_FROMWIDE(szName, Sound);
   CharLowerBuff(szName, lstrlen(szName));

   // In case we were playing any of the main buffers
   for (int i = 0; i < m_vsound.Size(); i++)
   {
      if (!lstrcmp(m_vsound.ElementAt(i)->m_szInternalName, szName))
      {
         m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
         break;
      }
   }

   for (int i = 0; i < m_voldsound.Size(); i++)
   {
      PinSoundCopy * const ppsc = m_voldsound.ElementAt(i);
      if (!lstrcmp(ppsc->m_ppsOriginal->m_szInternalName, szName))
      {
         ppsc->m_pDSBuffer->Stop();
         break;
      }
   }

   return S_OK;
}

void PinTable::StopAllSounds()
{
	// In case we were playing any of the main buffers
	for (int i = 0; i < m_vsound.Size(); i++)
	{
		m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
	}

	for (int i = 0; i < m_voldsound.Size(); i++)
	{
		PinSoundCopy * const ppsc = m_voldsound.ElementAt(i);
		ppsc->m_pDSBuffer->Stop();
	}
}


STDMETHODIMP PinTable::PlaySound(BSTR bstr, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade)
{
   MAKE_ANSIPTR_FROMWIDE(szName, bstr);
   CharLowerBuff(szName, lstrlen(szName));

   if (!lstrcmp("knock", szName) || !lstrcmp("knocker", szName))
   {
      hid_knock();
   }

   int i;
   for (i = 0; i < m_vsound.Size(); i++)
   {
      if (!lstrcmp(m_vsound.ElementAt(i)->m_szInternalName, szName))
      {
         break;
      }
   }

   if (i == m_vsound.Size()) // did not find it
   {
      return S_OK;
   }

   ClearOldSounds();
   PinSound * const pps = m_vsound.ElementAt(i);

   volume += dequantizeSignedPercent(pps->m_iVolume);
   pan += dequantizeSignedPercent(pps->m_iBalance);
   front_rear_fade += dequantizeSignedPercent(pps->m_iFade);
   
   const int flags = (loopcount == -1) ? DSBPLAY_LOOPING : 0;
   // 10 volume = -10Db

   const LPDIRECTSOUNDBUFFER pdsb = pps->m_pDSBuffer;
   //PinDirectSound *pDS = pps->m_pPinDirectSound;
   PinSoundCopy * ppsc = NULL;
   bool foundsame = false;
   if (usesame)
   {
      for (int i2 = 0; i2 < m_voldsound.Size(); i2++)
      {
         if (m_voldsound.ElementAt(i2)->m_ppsOriginal->m_pDSBuffer == pdsb)
         {
            ppsc = m_voldsound.ElementAt(i2);
            foundsame = true;
            break;
         }
      }
   }

   if (ppsc == NULL)
   {
      ppsc = new PinSoundCopy(pps);
   }

   if (m_tblMirrorEnabled)
      pan = -pan;

   if (ppsc->m_pDSBuffer)
   {
	  ppsc->Play(volume * m_TableSoundVolume* ((float)g_pplayer->m_SoundVolume), randompitch, pitch, pan, front_rear_fade, flags, !!restart);
      if (!foundsame)
      {
         m_voldsound.AddElement(ppsc);
      }
   }
   else // Couldn't or didn't want to create a copy - just play the original
   {
      delete ppsc;

	  pps->Play(volume * m_TableSoundVolume * ((float)g_pplayer->m_SoundVolume), randompitch, pitch, pan, front_rear_fade, flags, !!restart);
   }

   return S_OK;
}


Texture *PinTable::GetImage(char * const szName) const
{
   if (szName == NULL || szName[0] == '\0')
      return NULL;

   CharLowerBuff(szName, lstrlen(szName));

   // during playback, we use the hashtable for lookup
   if (!m_textureMap.empty())
   {
      std::tr1::unordered_map<const char*, Texture*, StringHashFunctor, StringComparator>::const_iterator
         it = m_textureMap.find(szName);
      if (it != m_textureMap.end())
         return it->second;
      else
         return NULL;
   }

   for (unsigned i = 0; i < m_vimage.size(); i++)
   {
      if (!lstrcmp(m_vimage[i]->m_szInternalName, szName))
      {
         return m_vimage[i];
      }
   }

   return NULL;
}

void PinTable::CreateGDIBackdrop()
{
}

void PinTable::ReImportImage(HWND hwndListView, Texture *ppi, char *filename)
{
   char szextension[MAX_PATH];
   ExtensionFromFilename(filename, szextension);

   BOOL fBinary;
   if (!lstrcmpi(szextension, "bmp"))
   {
      fBinary = fFalse;
   }
   else // other format
   {
      fBinary = fTrue;
   }

   PinBinary *ppb = 0;
   if (fBinary)
   {
      ppb = new PinBinary();
      ppb->ReadFromFile(filename);
   }

   BaseTexture *tex = BaseTexture::CreateFromFile(filename);

   if (tex == NULL)
   {
      if (ppb) delete ppb;
      return;
   }

   ppi->FreeStuff();

   if (fBinary)
   {
      ppi->m_ppb = ppb;
   }

   //SAFE_RELEASE(ppi->m_pdsBuffer);

   ppi->m_width = tex->width();
   ppi->m_height = tex->height();
   ppi->m_pdsBuffer = tex;

   strncpy_s(ppi->m_szPath, filename, MAX_PATH);
}


bool PinTable::ExportImage(HWND hwndListView, Texture *ppi, char *szfilename)
{
   if (ppi->m_ppb != NULL)
      return ppi->m_ppb->WriteToFile(szfilename);
   else if (ppi->m_pdsBuffer != NULL)
   {
      HANDLE hFile = CreateFile(szfilename, GENERIC_WRITE, FILE_SHARE_READ,
         NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

      if (hFile == INVALID_HANDLE_VALUE)
      {
         ShowError("The graphic file could not be written.");
         return false;
      }
      const int surfwidth = ppi->m_width;					// texture width 
      const int surfheight = ppi->m_height;					// and height		

      int bmplnsize = (surfwidth * 4 + 3) & -4;		// line size ... 4 bytes per pixel + pad to 4 byte boundary		

      //<<<< began bmp file header and info <<<<<<<<<<<<<<<

      BITMAPFILEHEADER bmpf;		// file header
      bmpf.bfType = 'MB';
      bmpf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + surfheight*bmplnsize;
      bmpf.bfReserved1 = 0;
      bmpf.bfReserved2 = 0;
      bmpf.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

      DWORD write;
      // write BMP file header	
      WriteFile(hFile, &bmpf, sizeof(BITMAPFILEHEADER), &write, NULL);

      BITMAPINFOHEADER bmpi;		// info header		
      bmpi.biSize = sizeof(BITMAPINFOHEADER);	//only a few fields are used 
      bmpi.biWidth = surfwidth;
      bmpi.biHeight = surfheight;
      bmpi.biPlanes = 1;
      bmpi.biBitCount = 32;
      bmpi.biCompression = 0;
      bmpi.biSizeImage = surfheight*bmplnsize;
      bmpi.biXPelsPerMeter = 0;
      bmpi.biYPelsPerMeter = 0;
      bmpi.biClrUsed = 0;
      bmpi.biClrImportant = 0;

      //write BMP Info Header
      WriteFile(hFile, &bmpi, sizeof(BITMAPINFOHEADER), &write, NULL);

      unsigned char* sinfo = new unsigned char[bmplnsize + 4]; //linebuffer and safty pad
      if (!sinfo)
      {
         CloseHandle(hFile);
         return false;
      }

      unsigned char* info;
      for (info = sinfo + surfwidth * 3; info < sinfo + bmplnsize; *info++ = 0); //fill padding with 0			

      const int pitch = ppi->m_pdsBuffer->pitch();
      const BYTE *spch = ppi->m_pdsBuffer->data() + (surfheight * pitch);	// just past the end of the Texture part of DD surface

      for (int i = 0; i < surfheight; i++)
      {
         info = sinfo; //reset to start	
         const BYTE *pch = (spch -= pitch);  // start on previous previous line

         for (int l = 0; l < surfwidth; l++)
         {
            *(unsigned int*)info = *(unsigned int*)pch;
            info += 4;
            pch += 4;
         }

         WriteFile(hFile, sinfo, bmplnsize, &write, NULL);
         GetLastError();
      }

      delete[] sinfo;
      CloseHandle(hFile);
      return true;
   }
   return false;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>>

void PinTable::ImportImage(HWND hwndListView, char *filename)
{
   Texture * const ppi = new Texture();

   ReImportImage(hwndListView, ppi, filename);

   if (ppi->m_pdsBuffer == NULL)
   {
      delete ppi;
      return;
   }

   // The first time we import a file, parse the name of the texture from the filename

   int begin, end;
   const int len = lstrlen(filename);

   for (begin = len; begin >= 0; begin--)
   {
      if (filename[begin] == '\\')
      {
         begin++;
         break;
      }
   }

   for (end = len; end >= 0; end--)
   {
      if (filename[end] == '.')
      {
         break;
      }
   }

   if (end == 0)
   {
      end = len - 1;
   }

   strncpy_s(ppi->m_szName, &filename[begin], MAXTOKEN);

   ppi->m_szName[end - begin] = 0;

   strncpy_s(ppi->m_szInternalName, ppi->m_szName, MAXTOKEN);

   CharLowerBuff(ppi->m_szInternalName, lstrlen(ppi->m_szInternalName));

   m_vimage.push_back(ppi);

   const int index = AddListImage(hwndListView, ppi);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
}

void PinTable::ListImages(HWND hwndListView)
{
   for (unsigned i = 0; i < m_vimage.size(); i++)
   {
      AddListImage(hwndListView, m_vimage[i]);
   }
}

int PinTable::AddListImage(HWND hwndListView, Texture *ppi)
{
   char sizeString[MAXTOKEN] = { 0 };
   char * const usedStringYes="X";
   char * const usedStringNo=" ";

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = ppi->m_szName;
   lvitem.lParam = (size_t)ppi;
   
   _snprintf_s(sizeString, MAXTOKEN, "%ix%i", ppi->m_realWidth, ppi->m_realHeight);
   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, ppi->m_szPath);
   ListView_SetItemText(hwndListView, index, 2, sizeString);
   ListView_SetItemText(hwndListView, index, 3, usedStringNo);
   
   _snprintf_s(sizeString, MAXTOKEN, "%i", ppi->m_pdsBuffer->m_data.size());

   ListView_SetItemText(hwndListView, index, 4, sizeString);
   if((_stricmp(m_szImage, ppi->m_szName) == 0)
       || (_stricmp( m_szBallImage, ppi->m_szName ) == 0) 
       || (_stricmp( m_szBallImageFront, ppi->m_szName)==0 )
       || (_stricmp( m_szEnvImage, ppi->m_szName ) == 0)
       || (_stricmp( m_BG_szImage[BG_DESKTOP], ppi->m_szName )==0)
       || (_stricmp( m_BG_szImage[BG_FSS], ppi->m_szName ) == 0)
       || (_stricmp( m_BG_szImage[BG_FULLSCREEN], ppi->m_szName ) == 0)
       || (_stricmp( m_szImageColorGrade, ppi->m_szName ) == 0))
   {
       ListView_SetItemText( hwndListView, index, 3, usedStringYes );
   }
   else
   {
       for(int i=0; i < m_vedit.Size(); i++)
       {
           bool inUse=false;
           IEditable *pEdit=m_vedit.ElementAt( i );
           if(pEdit == NULL)
               continue;

           switch(pEdit->GetItemType())
           {
               case eItemDispReel:
               {
                   DispReel *pReel = (DispReel*)pEdit;
                   if(_stricmp( pReel->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemPrimitive:
               {
                   Primitive *pPrim = (Primitive*)pEdit;
                   if((_stricmp( pPrim->m_d.m_szImage, ppi->m_szName ) == 0) || (_stricmp( pPrim->m_d.m_szNormalMap, ppi->m_szName ) == 0))
                       inUse=true;
                   break;
               }
               case eItemRamp:
               {
                   Ramp *pRamp = (Ramp*)pEdit;
                   if(_stricmp( pRamp->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemSurface:
               {
                   Surface *pSurf = (Surface*)pEdit;
                   if((_stricmp( pSurf->m_d.m_szImage, ppi->m_szName ) == 0) || (_stricmp( pSurf->m_d.m_szSideImage, ppi->m_szName ) == 0))
                       inUse=true;
                   break;
               }
               case eItemDecal:
               {
                   Decal *pDecal = (Decal*)pEdit;
                   if(_stricmp( pDecal->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemFlasher:
               {
                   Flasher *pFlash = (Flasher*)pEdit;
                   if((_stricmp( pFlash->m_d.m_szImageA, ppi->m_szName ) == 0) || (_stricmp( pFlash->m_d.m_szImageB, ppi->m_szName ) == 0))
                       inUse=true;
                   break;
               }
               case eItemFlipper:
               {
                   Flipper *pFlip = (Flipper*)pEdit;
                   if(_stricmp( pFlip->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemHitTarget:
               {
                   HitTarget *pHit = (HitTarget*)pEdit;
                   if(_stricmp( pHit->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemLight:
               {
                   Light *pLight = (Light*)pEdit;
                   if(_stricmp( pLight->m_d.m_szOffImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemPlunger:
               {
                   Plunger *pPlung = (Plunger*)pEdit;
                   if(_stricmp( pPlung->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemRubber:
               {
                   Rubber *pRub = (Rubber*)pEdit;
                   if(_stricmp( pRub->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               case eItemSpinner:
               {
                   Spinner *pSpin = (Spinner*)pEdit;
                   if(_stricmp( pSpin->m_d.m_szImage, ppi->m_szName ) == 0)
                       inUse=true;
                   break;
               }
               default:
               {
                   break;
               }
           }

           if(inUse)
           {
               ListView_SetItemText( hwndListView, index, 3, usedStringYes );
               break;
           }
       }//for
   }//else
   return index;
}

void PinTable::RemoveImage(Texture *ppi)
{
   RemoveFromVector(m_vimage, ppi);
   delete ppi;
}

void PinTable::ListMaterials(HWND hwndListView)
{
   for (int i = 0; i < m_materials.Size(); i++)
   {
      AddListMaterial(hwndListView, m_materials.ElementAt(i));
   }
}

bool PinTable::IsMaterialNameUnique(char *name)
{
   for (int i = 0; i < m_materials.Size(); i++)
   {
      if (!lstrcmpi(m_materials.ElementAt(i)->m_szName, name))
         return false;
   }
   return true;
}


Material* PinTable::GetMaterial(char * const szName) const
{
   if (szName == NULL || szName[0] == '\0')
      return &g_pvp->dummyMaterial;

   // during playback, we use the hashtable for lookup
   if (!m_materialMap.empty())
   {
      std::tr1::unordered_map<const char*, Material*, StringHashFunctor, StringComparator>::const_iterator
         it = m_materialMap.find(szName);
      if (it != m_materialMap.end())
         return it->second;
      else
         return &g_pvp->dummyMaterial;
   }

   for (int i = 0; i < m_materials.Size(); i++)
   {
      if (!lstrcmp(m_materials.ElementAt(i)->m_szName, szName))
      {
         return m_materials.ElementAt(i);
      }
   }

   return &g_pvp->dummyMaterial;
}

void PinTable::AddMaterial(Material *pmat)
{
   int suffix = 1;
   if (pmat->m_szName[0] == 0 || !strcmp(pmat->m_szName, "dummyMaterial"))
   {
      strcpy_s(pmat->m_szName, "Material");
   }

   if (!IsMaterialNameUnique(pmat->m_szName) || !strcmp(pmat->m_szName, "Material"))
   {
      char textBuf[32];
      do
      {
         sprintf_s(textBuf, "%s%i", pmat->m_szName, suffix);
         suffix++;
      } while (!IsMaterialNameUnique(textBuf));
      lstrcpy(pmat->m_szName, textBuf);
   }

   m_materials.AddElement(pmat);
}

void PinTable::AddDbgMaterial(Material *pmat)
{
   bool alreadyIn = false;
   unsigned int i;

   for (i = 0; i < m_dbgChangedMaterials.size(); i++)
   {
      if (strcmp(pmat->m_szName, m_dbgChangedMaterials[i]->m_szName) == 0)
      {
         alreadyIn = true;
         break;
      }
   }
      
   if (alreadyIn)
   {
      m_dbgChangedMaterials[i]->m_bIsMetal = pmat->m_bIsMetal;
      m_dbgChangedMaterials[i]->m_bOpacityActive = pmat->m_bOpacityActive;
      m_dbgChangedMaterials[i]->m_cBase = pmat->m_cBase;
      m_dbgChangedMaterials[i]->m_cClearcoat = pmat->m_cClearcoat;
      m_dbgChangedMaterials[i]->m_cGlossy = pmat->m_cGlossy;
      m_dbgChangedMaterials[i]->m_fEdge = pmat->m_fEdge;
      m_dbgChangedMaterials[i]->m_fEdgeAlpha = pmat->m_fEdgeAlpha;
      m_dbgChangedMaterials[i]->m_fOpacity = pmat->m_fOpacity;
      m_dbgChangedMaterials[i]->m_fRoughness = pmat->m_fRoughness;
      m_dbgChangedMaterials[i]->m_fGlossyImageLerp = pmat->m_fGlossyImageLerp;
      m_dbgChangedMaterials[i]->m_fThickness = pmat->m_fThickness;
      m_dbgChangedMaterials[i]->m_fWrapLighting = pmat->m_fWrapLighting;
   }
   else
   {
      Material * const newMat = new Material();
      newMat->m_bIsMetal = pmat->m_bIsMetal;
      newMat->m_bOpacityActive = pmat->m_bOpacityActive;
      newMat->m_cBase = pmat->m_cBase;
      newMat->m_cClearcoat = pmat->m_cClearcoat;
      newMat->m_cGlossy = pmat->m_cGlossy;
      newMat->m_fEdge = pmat->m_fEdge;
      newMat->m_fEdgeAlpha = pmat->m_fEdgeAlpha;
      newMat->m_fOpacity = pmat->m_fOpacity;
      newMat->m_fRoughness = pmat->m_fRoughness;
      newMat->m_fGlossyImageLerp = pmat->m_fGlossyImageLerp;
      newMat->m_fThickness = pmat->m_fThickness;
      newMat->m_fWrapLighting = pmat->m_fWrapLighting;
      strcpy_s(newMat->m_szName, pmat->m_szName);
      m_dbgChangedMaterials.push_back(newMat);
   }
}

void PinTable::UpdateDbgMaterial(void)
{
   bool somethingChanged = false;
   for (unsigned int i = 0; i < m_dbgChangedMaterials.size(); i++)
   {
      const Material * const pmat = m_dbgChangedMaterials[i];
      for (int t = 0; t < m_materials.Size(); t++)
      {
         if (strcmp(pmat->m_szName, m_materials.ElementAt(t)->m_szName) == 0)
         {
            Material * const mat = m_materials.ElementAt(t);
            mat->m_bIsMetal = pmat->m_bIsMetal;
            mat->m_bOpacityActive = pmat->m_bOpacityActive;
            mat->m_cBase = pmat->m_cBase;
            mat->m_cClearcoat = pmat->m_cClearcoat;
            mat->m_cGlossy = pmat->m_cGlossy;
            mat->m_fEdge = pmat->m_fEdge;
            mat->m_fEdgeAlpha = pmat->m_fEdgeAlpha;
            mat->m_fOpacity = pmat->m_fOpacity;
            mat->m_fRoughness = pmat->m_fRoughness;
            mat->m_fGlossyImageLerp = pmat->m_fGlossyImageLerp;
            mat->m_fThickness = pmat->m_fThickness;
            mat->m_fWrapLighting = pmat->m_fWrapLighting;
            somethingChanged = true;
            break;
         }
      }
   }
   m_dbgChangedMaterials.clear();
   if (somethingChanged)
   {
      SetNonUndoableDirty(eSaveDirty);
   }
}

int PinTable::AddListMaterial(HWND hwndListView, Material *pmat)
{
   LVITEM lvitem;
   char * const usedStringYes = "X";
   char * const usedStringNo = " ";
   
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = pmat->m_szName;
   lvitem.lParam = (size_t)pmat;

   const int index = ListView_InsertItem(hwndListView, &lvitem);
   ListView_SetItemText(hwndListView, index, 1, usedStringNo);
   if ((_stricmp(m_szPlayfieldMaterial, pmat->m_szName) == 0))
   {
      ListView_SetItemText(hwndListView, index, 1, usedStringYes);
   }
   else
   {
      for (int i = 0; i < m_vedit.Size(); i++)
      {
         bool inUse = false;
         IEditable *pEdit = m_vedit.ElementAt(i);
         if (pEdit == NULL)
            continue;

         switch (pEdit->GetItemType())
         {
         case eItemPrimitive:
         {
            Primitive *pPrim = (Primitive*)pEdit;
            if ((_stricmp(pPrim->m_d.m_szMaterial, pmat->m_szName) == 0) || (_stricmp(pPrim->m_d.m_szPhysicsMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemRamp:
         {
            Ramp *pRamp = (Ramp*)pEdit;
            if ((_stricmp(pRamp->m_d.m_szMaterial, pmat->m_szName) == 0) || (_stricmp(pRamp->m_d.m_szPhysicsMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemSurface:
         {
            Surface *pSurf = (Surface*)pEdit;
            if ((_stricmp(pSurf->m_d.m_szPhysicsMaterial, pmat->m_szName) == 0) || (_stricmp(pSurf->m_d.m_szSideMaterial, pmat->m_szName) == 0) || (_stricmp(pSurf->m_d.m_szTopMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemDecal:
         {
            Decal *pDecal = (Decal*)pEdit;
            if ((_stricmp(pDecal->m_d.m_szMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemFlipper:
         {
            Flipper *pFlip = (Flipper*)pEdit;
            if ((_stricmp(pFlip->m_d.m_szRubberMaterial, pmat->m_szName) == 0) || (_stricmp(pFlip->m_d.m_szMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemHitTarget:
         {
            HitTarget *pHit = (HitTarget*)pEdit;
            if ((_stricmp(pHit->m_d.m_szMaterial, pmat->m_szName) == 0) || (_stricmp(pHit->m_d.m_szPhysicsMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemPlunger:
         {
            Plunger *pPlung = (Plunger*)pEdit;
            if (_stricmp(pPlung->m_d.m_szMaterial, pmat->m_szName) == 0)
               inUse = true;
            break;
         }
         case eItemSpinner:
         {
            Spinner *pSpin = (Spinner*)pEdit;
            if (_stricmp(pSpin->m_d.m_szMaterial, pmat->m_szName) == 0)
               inUse = true;
            break;
         }
         case eItemRubber:
         {
            Rubber *pRub = (Rubber*)pEdit;
            if ((_stricmp(pRub->m_d.m_szMaterial, pmat->m_szName) == 0) || (_stricmp(pRub->m_d.m_szPhysicsMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemBumper:
         {
            Bumper *pBump = (Bumper*)pEdit;
            if ((_stricmp(pBump->m_d.m_szCapMaterial, pmat->m_szName) == 0) || (_stricmp(pBump->m_d.m_szBaseMaterial, pmat->m_szName) == 0) ||
                (_stricmp(pBump->m_d.m_szSkirtMaterial, pmat->m_szName) == 0) || (_stricmp(pBump->m_d.m_szRingMaterial, pmat->m_szName) == 0))
               inUse = true;
            break;
         }
         case eItemKicker:
         {
            Kicker *pKick = (Kicker*)pEdit;
            if (_stricmp(pKick->m_d.m_szMaterial, pmat->m_szName) == 0)
               inUse = true;
            break;
         }
         case eItemTrigger:
         {
            Trigger *pTrig = (Trigger*)pEdit;
            if (_stricmp(pTrig->m_d.m_szMaterial, pmat->m_szName) == 0)
               inUse = true;
            break;
         }
         default:
            break;
         }

         if (inUse)
         {
            ListView_SetItemText(hwndListView, index, 1, usedStringYes);
            break;
         }
      }//for
   }
   return index;
}

void PinTable::RemoveMaterial(Material *pmat)
{
   m_materials.RemoveElement(pmat);
   delete pmat;
}

void PinTable::AddDbgLight( Light *plight )
{
    bool alreadyIn = false;
    unsigned int i;
    char *lightName = GetElementName( plight );

    for(i = 0; i < m_dbgChangedMaterials.size(); i++)
    {
        if(strcmp( lightName, m_dbgChangedLights[i]->name) == 0)
        {
            alreadyIn = true;
            break;
        }
    }
    if(alreadyIn)
    {
        m_dbgChangedLights[i]->color1 = plight->m_d.m_color;
        m_dbgChangedLights[i]->color2 = plight->m_d.m_color2;
        plight->get_BulbModulateVsAdd(&m_dbgChangedLights[i]->bulbModulateVsAdd);
        plight->get_FadeSpeedDown(&m_dbgChangedLights[i]->fadeSpeedDown);
        plight->get_FadeSpeedUp(&m_dbgChangedLights[i]->fadeSpeedUp);
        plight->get_State(&m_dbgChangedLights[i]->lightstate);
        plight->get_Falloff(&m_dbgChangedLights[i]->falloff);
        plight->get_FalloffPower(&m_dbgChangedLights[i]->falloffPower);
        plight->get_Intensity(&m_dbgChangedLights[i]->intensity);
        plight->get_TransmissionScale(&m_dbgChangedLights[i]->transmissionScale);
    }
    else
    {
        DebugLightData *data = new DebugLightData;
        data->color1 = plight->m_d.m_color;
        data->color2 = plight->m_d.m_color2;
        plight->get_BulbModulateVsAdd( &data->bulbModulateVsAdd );
        plight->get_FadeSpeedDown( &data->fadeSpeedDown );
        plight->get_FadeSpeedUp( &data->fadeSpeedUp );
        plight->get_State( &data->lightstate );
        plight->get_Falloff( &data->falloff );
        plight->get_FalloffPower( &data->falloffPower );
        plight->get_Intensity( &data->intensity );
        plight->get_TransmissionScale( &data->transmissionScale );
        strcpy_s( data->name, lightName );
        m_dbgChangedLights.push_back(data);
    }
}

void PinTable::UpdateDbgLight( void )
{
    bool somethingChanged = false;
    for(unsigned int i = 0; i < m_dbgChangedLights.size(); i++)
    {
        DebugLightData *data = m_dbgChangedLights[i];
        for(int t = 0; t < m_vedit.Size(); t++)
        {
            if(m_vedit.ElementAt( t )->GetItemType() == eItemLight)
            {
                Light *plight = (Light*)m_vedit.ElementAt( t );
                if(strcmp( data->name, GetElementName( plight ) ) == 0)
                {
                    plight->m_d.m_color = data->color1;
                    plight->m_d.m_color2 = data->color2;
                    plight->m_d.m_fadeSpeedDown = data->fadeSpeedDown;
                    plight->m_d.m_fadeSpeedUp = data->fadeSpeedUp;
                    plight->m_d.m_falloff = data->falloff;
                    plight->m_d.m_falloff_power = data->falloffPower;
                    plight->m_d.m_intensity = data->intensity;
                    plight->m_d.m_modulate_vs_add = data->bulbModulateVsAdd;
                    plight->m_d.m_transmissionScale = data->transmissionScale;
                    plight->m_d.m_state = data->lightstate;
                    somethingChanged = true;
                    break;
                }
            }
        }
    }
    m_dbgChangedLights.clear();
    if(somethingChanged)
    {
        SetNonUndoableDirty( eSaveDirty );
    }
}


int PinTable::GetImageLink(Texture *ppi)
{
   if (!lstrcmp(ppi->m_szInternalName, m_szScreenShot))
      return 1;
   else
      return 0;
}

PinBinary *PinTable::GetImageLinkBinary(int id)
{
   switch (id)
   {
   case 1: //Screenshot
      // Transfer ownership of the screenshot pinbary blob to the image
      PinBinary * const pbT = m_pbTempScreenshot;
      m_pbTempScreenshot = NULL;
      return pbT;
      break;
   }

   return NULL;
}

void PinTable::ListCustomInfo(HWND hwndListView)
{
   for (int i = 0; i < m_vCustomInfoTag.Size(); i++)
      AddListItem(hwndListView, m_vCustomInfoTag.ElementAt(i), m_vCustomInfoContent.ElementAt(i), NULL);
}

int PinTable::AddListItem(HWND hwndListView, char *szName, char *szValue1, LPARAM lparam)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = szName;
   lvitem.lParam = lparam;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, szValue1);

   return index;
}


HRESULT PinTable::LoadImageFromStream(IStream *pstm, int version)
{
   if (version < 100) // Tech Beta 3 and below
   {
      ShowError("Tables from Tech Beta 3 and below are not supported in this version.");
      return E_FAIL;
   }
   else
   {
      Texture * const ppi = new Texture();

      if (ppi->LoadFromStream(pstm, version, this) == S_OK)
      {
         m_vimage.push_back(ppi);
      }
      else
      {
         delete ppi;
      }
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_szImage,szImage);

   if (!g_pplayer)
   {
      CreateGDIBackdrop();
      SetDirtyDraw();
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut)
{
   return GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, NULL);
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
{
   return GetPredefinedValue(dispID, dwCookie, pVarOut, NULL);
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit)
{
   if (pcaStringsOut == NULL || pcaCookiesOut == NULL)
      return E_POINTER;

   int cvar;
   WCHAR **rgstr;
   DWORD *rgdw;

   switch (dispID)
   {
   case DISPID_Image:
   case DISPID_Image2:
   case DISPID_Image3:
   case DISPID_Image4:
   case DISPID_Image5:
   case DISPID_Image6:
   case DISPID_Image7:
   case DISPID_Image8:
   {
      cvar = (int)m_vimage.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      LocalString ls(IDS_NONE);
      MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (int ivar = 0; ivar < cvar; ivar++)
      {
         char *szSrc = m_vimage[ivar]->m_szName;
         DWORD cwch = lstrlen(szSrc) + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == NULL)
            ShowError("DISPID_Image alloc failed");

         MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = ivar;
      }
      cvar++;
   }
   break;
   case IDC_MATERIAL_COMBO:
   case IDC_MATERIAL_COMBO2:
   case IDC_MATERIAL_COMBO3:
   case IDC_MATERIAL_COMBO4:
   {
      cvar = m_materials.Size();
      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      LocalString ls(IDS_NONE);
      MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (int ivar = 0; ivar < cvar; ivar++)
      {
         char *szSrc = m_materials.ElementAt(ivar)->m_szName;
         DWORD cwch = lstrlen(szSrc) + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == NULL)
            ShowError("IDC_MATERIAL_COMBO alloc failed");

         MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = ivar;
      }
      cvar++;
      break;
   }
   case DISPID_Sound:
   {
      cvar = m_vsound.Size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      MultiByteToWideChar(CP_ACP, 0, "<None>", -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (int ivar = 0; ivar < cvar; ivar++)
      {
         char *szSrc = m_vsound.ElementAt(ivar)->m_szName;
         DWORD cwch = lstrlen(szSrc) + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == NULL)
         {
            ShowError("DISPID_Sound alloc failed");
         }

         MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = ivar;
      }
      cvar++;
   }
   break;

   case DISPID_Collection:
   {
      cvar = m_vcollection.Size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      MultiByteToWideChar(CP_ACP, 0, "<None>", -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (int ivar = 0; ivar < cvar; ivar++)
      {
         DWORD cwch = sizeof(m_vcollection.ElementAt(ivar)->m_wzName) + sizeof(DWORD); //!! +DWORD?
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch);
         if (wzDst == NULL)
            ShowError("DISPID_Collection alloc failed (1)");
         else
            memcpy(wzDst, m_vcollection.ElementAt(ivar)->m_wzName, cwch);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = ivar;
      }
      cvar++;
   }
   break;

   case DISPID_Surface:
   {
      cvar = 0;

      const bool fRamps = true;
      const bool fFlashers = true;

      for (int ivar = 0; ivar < m_vedit.Size(); ivar++)
         if (m_vedit.ElementAt(ivar)->GetItemType() == eItemSurface ||
            (fRamps && m_vedit.ElementAt(ivar)->GetItemType() == eItemRamp) ||
            // **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (fFlashers && m_vedit.ElementAt(ivar)->GetItemType() == eItemFlasher))
            cvar++;

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      cvar = 0;

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      //MultiByteToWideChar(CP_ACP, 0, "None", -1, wzDst, 5);
      // TEXT
      WideStrCopy(L"<None>", wzDst);
      rgstr[cvar] = wzDst;
      rgdw[cvar] = ~0u;
      cvar++;

      for (int ivar = 0; ivar < m_vedit.Size(); ivar++)
      {
         if (m_vedit.ElementAt(ivar)->GetItemType() == eItemSurface ||
            (fRamps && m_vedit.ElementAt(ivar)->GetItemType() == eItemRamp) ||
            // **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (fFlashers && m_vedit.ElementAt(ivar)->GetItemType() == eItemFlasher))
         {
            CComBSTR bstr;
            m_vedit.ElementAt(ivar)->GetScriptable()->get_Name(&bstr);

            DWORD cwch = lstrlenW(bstr) + 1;
            //wzDst = ::SysAllocString(bstr);

            wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
            if (wzDst == NULL)
               ShowError("DISPID_Surface alloc failed (1)");

            WideStrCopy(bstr, wzDst);

            //MultiByteToWideChar(CP_ACP, 0, "Hello", -1, wzDst, cwch);

            //MsoWzCopy(szSrc,szDst);
            rgstr[cvar] = wzDst;
            rgdw[cvar] = ivar;
            cvar++;
         }
      }
   }
   break;
   case IDC_EFFECT_COMBO:
   {
      cvar = 5;

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar)* sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar)* sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(5 * sizeof(WCHAR));
      MultiByteToWideChar(CP_ACP, 0, "None", -1, wzDst, 5);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;
      wzDst = (WCHAR *)CoTaskMemAlloc(9 * sizeof(WCHAR));
      MultiByteToWideChar(CP_ACP, 0, "Additive", -1, wzDst, 9);
      rgstr[1] = wzDst;
      rgdw[1] = 1;
      wzDst = (WCHAR *)CoTaskMemAlloc(9 * sizeof(WCHAR));
      MultiByteToWideChar(CP_ACP, 0, "Multiply", -1, wzDst, 9);
      rgstr[2] = wzDst;
      rgdw[2] = 2;
      wzDst = (WCHAR *)CoTaskMemAlloc(8 * sizeof(WCHAR));
      MultiByteToWideChar(CP_ACP, 0, "Overlay", -1, wzDst, 8);
      rgstr[3] = wzDst;
      rgdw[3] = 3;
      wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      MultiByteToWideChar(CP_ACP, 0, "Screen", -1, wzDst, 7);
      rgstr[4] = wzDst;
      rgdw[4] = 4;

      break;
   }
   default:
      return E_NOTIMPL;
   }

   pcaStringsOut->cElems = cvar;
   pcaStringsOut->pElems = rgstr;

   pcaCookiesOut->cElems = cvar;
   pcaCookiesOut->pElems = rgdw;

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit)
{
   WCHAR *wzDst;

   switch (dispID)
   {
   default:
      return E_NOTIMPL;

   case DISPID_Image:
   case DISPID_Image2:
   case DISPID_Image3:
   case DISPID_Image4:
   case DISPID_Image5:
   case DISPID_Image6:
   case DISPID_Image7:
   case DISPID_Image8:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)CoTaskMemAlloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         char *szSrc = m_vimage[dwCookie]->m_szName;
         DWORD cwch = lstrlen(szSrc) + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

         MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
      }
   }
   break;
   case IDC_MATERIAL_COMBO:
   case IDC_MATERIAL_COMBO2:
   case IDC_MATERIAL_COMBO3:
   case IDC_MATERIAL_COMBO4:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)CoTaskMemAlloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         char *szSrc = m_materials.ElementAt(dwCookie)->m_szName;
         DWORD cwch = lstrlen(szSrc) + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

         MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
      }
      break;
   }
   case DISPID_Sound:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)CoTaskMemAlloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         char *szSrc = m_vsound.ElementAt(dwCookie)->m_szName;
         DWORD cwch = lstrlen(szSrc) + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

         MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
      }
   }
   break;
   case DISPID_Collection:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)CoTaskMemAlloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         size_t cwch = sizeof(m_vcollection.ElementAt(dwCookie)->m_wzName) + sizeof(DWORD); //!! +DWORD?
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch);
         if (wzDst == NULL)
            ShowError("DISPID_Collection alloc failed (2)");
         else
            memcpy(wzDst, m_vcollection.ElementAt(dwCookie)->m_wzName, cwch - sizeof(DWORD)); //!! see above
      }
   }
   break;
   case IDC_EFFECT_COMBO:
   {
      int idx = dwCookie;
      char *filterNames[5] = { "None", "Additive", "Multiply", "Overlay", "Screen" };
      if (dwCookie == -1)
      {
         idx = 0;
      }
      DWORD cwch = lstrlen(filterNames[idx]) + 1;
      wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

      MultiByteToWideChar(CP_ACP, 0, filterNames[idx], -1, wzDst, cwch);
      break;
   }
   case DISPID_Surface:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)CoTaskMemAlloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         CComBSTR bstr;
         m_vedit.ElementAt(dwCookie)->GetScriptable()->get_Name(&bstr);

         DWORD cwch = lstrlenW(bstr) + 1;
         //wzDst = ::SysAllocString(bstr);

         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == NULL)
         {
            ShowError("DISPID_Surface alloc failed (2)");
         }
         else
            WideStrNCopy(bstr, wzDst, cwch*(DWORD)sizeof(WCHAR));
      }
   }
   break;
   }

   CComVariant var(wzDst);

   CoTaskMemFree(wzDst);

   return var.Detach(pVarOut);
}

float PinTable::GetSurfaceHeight(char *szName, float x, float y)
{
   if (szName == NULL || szName[0] == 0)
      return m_tableheight;

   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *item = m_vedit.ElementAt(i);
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
         item->GetScriptable()->get_Name(&bstr);
         if (!WzSzStrCmp(bstr, szName))
         {
            IEditable * const piedit = item;
            switch (piedit->GetItemType())
            {
            case eItemSurface:
               return ((Surface *)piedit)->m_d.m_heighttop + m_tableheight;

            case eItemRamp:
               return ((Ramp *)piedit)->GetSurfaceHeight(x, y);
            }
         }
      }
   }
   return m_tableheight;
}

Material* PinTable::GetSurfaceMaterial(char *szName)
{
   if (szName == NULL || szName[0] == 0)
      return GetMaterial(m_szPlayfieldMaterial);

   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *item = m_vedit.ElementAt(i);
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
         item->GetScriptable()->get_Name(&bstr);
         if (!WzSzStrCmp(bstr, szName))
         {
            IEditable * const piedit = item;
            switch (piedit->GetItemType())
            {
            case eItemSurface:
               return GetMaterial(((Surface *)piedit)->m_d.m_szTopMaterial);

            case eItemRamp:
               return GetMaterial(((Ramp *)piedit)->m_d.m_szMaterial);
            }
         }
      }
   }
   return GetMaterial(m_szPlayfieldMaterial);
}

Texture *PinTable::GetSurfaceImage(char *szName)
{
   if (szName == NULL || szName[0] == 0)
      return GetImage(m_szImage);

   for (int i = 0; i < m_vedit.Size(); i++)
   {
      IEditable *item = m_vedit.ElementAt(i);
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
         item->GetScriptable()->get_Name(&bstr);
         if (!WzSzStrCmp(bstr, szName))
         {
            IEditable * const piedit = item;
            switch (piedit->GetItemType())
            {
            case eItemSurface:
               return GetImage(((Surface *)piedit)->m_d.m_szImage);

            case eItemRamp:
               return GetImage(((Ramp *)piedit)->m_d.m_szImage);
            }
         }
      }
   }
   return GetImage(m_szImage);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP PinTable::get_DisplayGrid(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fGrid);

   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayGrid(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_fGrid = !!newVal;

   SetDirtyDraw();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DisplayBackdrop(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fBackdrop);

   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayBackdrop(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_fBackdrop = !!newVal;

   SetDirtyDraw();

   STOPUNDO

   return S_OK;
}

INT_PTR CALLBACK ProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      RECT rcProgress;
      RECT rcMain;
      GetWindowRect(g_pvp->m_hwnd, &rcMain);
      GetWindowRect(hwndDlg, &rcProgress);

      SetWindowPos(hwndDlg, NULL,
         (rcMain.right + rcMain.left) / 2 - (rcProgress.right - rcProgress.left) / 2,
         (rcMain.bottom + rcMain.top) / 2 - (rcProgress.bottom - rcProgress.top) / 2,
         0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);
      return TRUE;
   }
   break;
   }

   return FALSE;
}

STDMETHODIMP PinTable::get_GlassHeight(float *pVal)
{
   *pVal = m_glassheight;

   return S_OK;
}

STDMETHODIMP PinTable::put_GlassHeight(float newVal)
{
   STARTUNDO

      m_glassheight = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_TableHeight(float *pVal)
{
   *pVal = m_tableheight;

   return S_OK;
}

STDMETHODIMP PinTable::put_TableHeight(float newVal)
{
   STARTUNDO

      m_tableheight = newVal;

   STOPUNDO

      return S_OK;
}
STDMETHODIMP PinTable::get_Width(float *pVal)
{
   *pVal = m_right;
   g_pvp->SetStatusBarUnitInfo("");

   return S_OK;
}

STDMETHODIMP PinTable::put_Width(float newVal)
{
   STARTUNDO

      m_right = newVal;

   SetDirtyDraw();

   SetMyScrollInfo();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Height(float *pVal)
{
   *pVal = m_bottom;

   return S_OK;
}

STDMETHODIMP PinTable::put_Height(float newVal)
{
   STARTUNDO

      m_bottom = newVal;

   SetDirtyDraw();

   SetMyScrollInfo();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_PlayfieldMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szPlayfieldMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldMaterial(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szPlayfieldMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_LightAmbient(OLE_COLOR *pVal)
{
   *pVal = m_lightAmbient;

   return S_OK;
}

STDMETHODIMP PinTable::put_LightAmbient(OLE_COLOR newVal)
{
   STARTUNDO

      m_lightAmbient = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Light0Emission(OLE_COLOR *pVal)
{
   *pVal = m_Light[0].emission;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0Emission(OLE_COLOR newVal)
{
   STARTUNDO

      m_Light[0].emission = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_LightHeight(float *pVal)
{
   *pVal = m_lightHeight;

   return S_OK;
}

STDMETHODIMP PinTable::put_LightHeight(float newVal)
{
   STARTUNDO

      m_lightHeight = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_LightRange(float *pVal)
{
   *pVal = m_lightRange;

   return S_OK;
}

STDMETHODIMP PinTable::put_LightRange(float newVal)
{
   STARTUNDO

   m_lightRange = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_LightEmissionScale(float *pVal)
{
   *pVal = m_lightEmissionScale;

   return S_OK;
}

STDMETHODIMP PinTable::put_LightEmissionScale(float newVal)
{
   STARTUNDO

   m_lightEmissionScale = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_NightDay(int *pVal)
{
   *pVal = quantizeUnsignedPercent(m_globalEmissionScale);

   return S_OK;
}

STDMETHODIMP PinTable::put_NightDay(int newVal)
{
   STARTUNDO

   m_globalEmissionScale = dequantizeUnsignedPercent(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_AOScale(float *pVal)
{
   *pVal = m_AOScale;

   return S_OK;
}

STDMETHODIMP PinTable::put_AOScale(float newVal)
{
   STARTUNDO

   m_AOScale = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnvironmentEmissionScale(float *pVal)
{
   *pVal = m_envEmissionScale;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnvironmentEmissionScale(float newVal)
{
   STARTUNDO

   m_envEmissionScale = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallReflection(UserDefaultOnOff *pVal)
{
   *pVal = (UserDefaultOnOff)m_useReflectionForBalls;

   return S_OK;
}

STDMETHODIMP PinTable::put_BallReflection(UserDefaultOnOff newVal)
{
   STARTUNDO

   m_useReflectionForBalls = (int)newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_PlayfieldReflectionStrength(int *pVal)
{
   *pVal = quantizeUnsignedPercent(m_playfieldReflectionStrength);

   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldReflectionStrength(int newVal)
{
   STARTUNDO

   m_playfieldReflectionStrength = dequantizeUnsignedPercent(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallTrail(UserDefaultOnOff *pVal)
{
   *pVal = (UserDefaultOnOff)m_useTrailForBalls;

   return S_OK;
}

STDMETHODIMP PinTable::put_BallTrail(UserDefaultOnOff newVal)
{
   STARTUNDO

   m_useTrailForBalls = (int)newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TrailStrength(int *pVal)
{
   *pVal = quantizeUnsignedPercent(m_ballTrailStrength);

   return S_OK;
}

STDMETHODIMP PinTable::put_TrailStrength(int newVal)
{
   STARTUNDO

   m_ballTrailStrength = dequantizeUnsignedPercent(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallPlayfieldReflectionScale(float *pVal)
{
	*pVal = m_ballPlayfieldReflectionStrength;

	return S_OK;
}

STDMETHODIMP PinTable::put_BallPlayfieldReflectionScale(float newVal)
{
	STARTUNDO

	m_ballPlayfieldReflectionStrength = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_DefaultBulbIntensityScale(float *pVal)
{
   *pVal = m_defaultBulbIntensityScaleOnBall;

   return S_OK;
}

STDMETHODIMP PinTable::put_DefaultBulbIntensityScale(float newVal)
{
   STARTUNDO

      m_defaultBulbIntensityScaleOnBall = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_BloomStrength(float *pVal)
{
   *pVal = m_bloom_strength;

   return S_OK;
}

STDMETHODIMP PinTable::put_BloomStrength(float newVal)
{
   STARTUNDO

   m_bloom_strength = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableSoundVolume(int *pVal)
{
   *pVal = quantizeUnsignedPercent(m_TableSoundVolume);

   return S_OK;
}

STDMETHODIMP PinTable::put_TableSoundVolume(int newVal)
{
   STARTUNDO

   m_TableSoundVolume = dequantizeUnsignedPercent(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DetailLevel(int *pVal)
{
   if (m_overwriteGlobalDetailLevel)
      *pVal = m_userDetailLevel;
   else
      *pVal = m_globalDetailLevel;

   return S_OK;
}

STDMETHODIMP PinTable::put_DetailLevel(int newVal)
{
   STARTUNDO

   if (m_overwriteGlobalDetailLevel)
      m_userDetailLevel = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalAlphaAcc(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_overwriteGlobalDetailLevel);

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalAlphaAcc(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_overwriteGlobalDetailLevel = !!newVal;
   if (!m_overwriteGlobalDetailLevel)
   {
      m_userDetailLevel = m_globalDetailLevel;
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalDayNight(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_overwriteGlobalDayNight);

	return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDayNight(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_overwriteGlobalDayNight = !!newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinTable::get_GlobalStereo3D(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_overwriteGlobalStereo3D);

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalStereo3D(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_overwriteGlobalStereo3D = !!newVal;
   if (!m_overwriteGlobalStereo3D)
   {
      m_3DmaxSeparation = m_global3DMaxSeparation;
      m_3DZPD = m_global3DZPD;
	  m_3DOffset = m_global3DOffset;
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallDecalMode(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_BallDecalMode);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallDecalMode(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_BallDecalMode = !!newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableMusicVolume(int *pVal)
{
   *pVal = quantizeUnsignedPercent(m_TableMusicVolume);

   return S_OK;
}

STDMETHODIMP PinTable::put_TableMusicVolume(int newVal)
{
   STARTUNDO

   m_TableMusicVolume = dequantizeUnsignedPercent(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableAdaptiveVSync(int *pVal)
{
   *pVal = m_TableAdaptiveVSync;

   return S_OK;
}

STDMETHODIMP PinTable::put_TableAdaptiveVSync(int newVal)
{
   STARTUNDO

      m_TableAdaptiveVSync = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_BackdropColor(OLE_COLOR *pVal)
{
   *pVal = m_colorbackdrop;

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropColor(OLE_COLOR newVal)
{
   STARTUNDO

      m_colorbackdrop = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImageApplyNightDay(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_ImageBackdropNightDay);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImageApplyNightDay(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_ImageBackdropNightDay = !!newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ShowFSS(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_BG_enable_FSS);

   //*pVal = (VARIANT_BOOL)FTOVB(m_BG_current_set == 2);

   return S_OK;
}

STDMETHODIMP PinTable::put_ShowFSS(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_BG_enable_FSS = !!newVal;

   if(m_BG_enable_FSS)
      m_BG_current_set = FULL_SINGLE_SCREEN;
   else
      GetRegInt("Player", "BGSet", (int*)&m_BG_current_set);

   STOPUNDO

   SetDirtyDraw();

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_DT(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_BG_szImage[0], -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_DT(BSTR newVal) //!! HDR??
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_BG_szImage[0], 32, NULL, NULL);

   if (!g_pplayer)
   {
      CreateGDIBackdrop();
      SetDirtyDraw();
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_FS(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_BG_szImage[1], -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_FS(BSTR newVal) //!! HDR??
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_BG_szImage[1], 32, NULL, NULL);

   if (!g_pplayer)
   {
      CreateGDIBackdrop();
      SetDirtyDraw();
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_FSS(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_BG_szImage[2], -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_FSS(BSTR newVal) //!! HDR??
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_BG_szImage[2], 32, NULL, NULL);

   if (!g_pplayer)
   {
      CreateGDIBackdrop();
      SetDirtyDraw();
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ColorGradeImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szImageColorGrade, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_ColorGradeImage(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = GetImage(szImage);
   if(tex && (tex->m_width != 256 || tex->m_height != 16))
   {
       ShowError("Wrong image size, needs to be 256x16 resolution");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_szImageColorGrade,szImage);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Gravity(float *pVal)
{
   *pVal = m_Gravity*(float)(1.0 / GRAVITYCONST);

   return S_OK;
}

STDMETHODIMP PinTable::put_Gravity(float newVal)
{
   if (newVal < 0) newVal = 0;

   if (g_pplayer)
   {
      m_Gravity = newVal*GRAVITYCONST;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
         m_Gravity = newVal*GRAVITYCONST;
      STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_Friction(float *pVal)
{
   *pVal = m_friction;

   return S_OK;
}

STDMETHODIMP PinTable::put_Friction(float newVal)
{
   STARTUNDO

   m_friction = clamp(newVal, 0.0f, 1.0f);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Elasticity(float *pVal)
{
   *pVal = m_elasticity;

   return S_OK;
}

STDMETHODIMP PinTable::put_Elasticity(float newVal)
{
   STARTUNDO

      m_elasticity = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP PinTable::put_ElasticityFalloff(float newVal)
{
   STARTUNDO

      m_elasticityFalloff = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Scatter(float *pVal)
{
   *pVal = m_scatter;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scatter(float newVal)
{
   STARTUNDO

      m_scatter = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_DefaultScatter(float *pVal)
{
   *pVal = m_defaultScatter;

   return S_OK;
}

STDMETHODIMP PinTable::put_DefaultScatter(float newVal)
{
   STARTUNDO

      m_defaultScatter = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_NudgeTime(float *pVal)
{
   *pVal = m_nudgeTime;

   return S_OK;
}

STDMETHODIMP PinTable::put_NudgeTime(float newVal)
{
   STARTUNDO
      m_nudgeTime = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_PlungerNormalize(int *pVal)
{
   *pVal = m_plungerNormalize;

   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerNormalize(int newVal)
{
   STARTUNDO

      m_plungerNormalize = newVal;
   GetRegInt("Player", "PlungerNormalize", &m_plungerNormalize);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_PlungerFilter(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_plungerFilter);
   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerFilter(VARIANT_BOOL newVal)
{
   STARTUNDO

      BOOL tmp = VBTOF(newVal);
   GetRegInt("Player", "PlungerFilter", &tmp);
   m_plungerFilter = (tmp != 0);

   STOPUNDO
      return S_OK;
}

STDMETHODIMP PinTable::get_PhysicsLoopTime(int *pVal)
{
   *pVal = m_PhysicsMaxLoops;

   return S_OK;
}

STDMETHODIMP PinTable::put_PhysicsLoopTime(int newVal)
{
   STARTUNDO

      m_PhysicsMaxLoops = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_BackglassMode(BackglassIndex *pVal)
{
   *pVal = (BackglassIndex)(m_currentBackglassMode+DESKTOP);
   return S_OK;
}

STDMETHODIMP PinTable::put_BackglassMode(BackglassIndex pVal)
{
   m_currentBackglassMode = (int)(pVal-DESKTOP);
   g_pvp->m_sb.RefreshProperties();

   return S_OK;
}

STDMETHODIMP PinTable::get_FieldOfView(float *pVal)
{
   *pVal = m_BG_FOV[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_FieldOfView(float newVal)
{
   STARTUNDO

      m_BG_FOV[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Inclination(float *pVal)
{
   *pVal = m_BG_inclination[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Inclination(float newVal)
{
   STARTUNDO

      m_BG_inclination[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Layback(float *pVal)
{
   *pVal = m_BG_layback[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Layback(float newVal)
{
   STARTUNDO

      m_BG_layback[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Rotation(float *pVal)
{
   *pVal = m_BG_rotation[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Rotation(float newVal)
{
   STARTUNDO

      m_BG_rotation[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Scalex(float *pVal)
{
   *pVal = m_BG_scalex[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Scalex(float newVal)
{
   STARTUNDO

      m_BG_scalex[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Scaley(float *pVal)
{
   *pVal = m_BG_scaley[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Scaley(float newVal)
{
   STARTUNDO

      m_BG_scaley[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Scalez(float *pVal)
{
   *pVal = m_BG_scalez[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Scalez(float newVal)
{
   STARTUNDO

      m_BG_scalez[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Xlatex(float *pVal)
{
   *pVal = m_BG_xlatex[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatex(float newVal)
{
   STARTUNDO

      m_BG_xlatex[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Xlatey(float *pVal)
{
   *pVal = m_BG_xlatey[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatey(float newVal)
{
   STARTUNDO

      m_BG_xlatey[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Xlatez(float *pVal)
{
   *pVal = m_BG_xlatez[m_currentBackglassMode];

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatez(float newVal)
{
   STARTUNDO

      m_BG_xlatez[m_currentBackglassMode] = newVal;

   STOPUNDO

      return S_OK;
}


STDMETHODIMP PinTable::get_SlopeMax(float *pVal)
{
   if (m_angletiltMax == 726.0f) m_angletiltMax = m_angletiltMin;
   *pVal = m_angletiltMax;

   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMax(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMax = newVal;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
         m_angletiltMax = newVal;
      STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_SlopeMin(float *pVal)
{
   *pVal = m_angletiltMin;

   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMin(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMin = newVal;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
         m_angletiltMin = newVal;
      STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_BallImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szBallImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallImage(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImage, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnvironmentImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szEnvImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnvironmentImage(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = GetImage(szImage);
   if(tex && (tex->m_width != tex->m_height*2))
   {
       ShowError("Wrong image size, needs to be 2x width in comparison to height");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_szEnvImage,szImage);

   STOPUNDO

   return S_OK;

   return S_OK;
}

STDMETHODIMP PinTable::get_YieldTime(long *pVal)
{
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   else
      *pVal = g_pplayer->m_sleeptime;

   return S_OK;
}

STDMETHODIMP PinTable::put_YieldTime(long newVal)
{
   if (!g_pplayer)
      return E_FAIL;
   else
   {
      g_pplayer->m_sleeptime = newVal;
      if (g_pplayer->m_sleeptime > 1000)
         g_pplayer->m_sleeptime = 1000;
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableAntialiasing(UserDefaultOnOff *pVal)
{
   *pVal = (UserDefaultOnOff)m_useAA;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableAntialiasing(UserDefaultOnOff newVal)
{
   STARTUNDO
   m_useAA = (int)newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableAO(UserDefaultOnOff *pVal)
{
   *pVal = (UserDefaultOnOff)m_useAO;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableAO(UserDefaultOnOff newVal)
{
   STARTUNDO
   m_useAO = (int)newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableFXAA(FXAASettings *pVal)
{
   *pVal = (FXAASettings)m_useFXAA;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableFXAA(FXAASettings newVal)
{
   STARTUNDO
   m_useFXAA = (int)newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_OverridePhysics(PhysicsSet *pVal)
{
   *pVal = (PhysicsSet)m_fOverridePhysics;

   return S_OK;
}

STDMETHODIMP PinTable::put_OverridePhysics(PhysicsSet newVal)
{
   STARTUNDO
      m_fOverridePhysics = (int)newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::ImportPhysics()
{
   char szFileName[1024];
   char szInitialDir[1024];
   szFileName[0] = '\0';

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_hinst;
   ofn.hwndOwner = g_pvp->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrDefExt = "vpp";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   const HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
   char szFoo[MAX_PATH];
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
      lstrcpy(szFoo, "c:\\");
      ofn.lpstrInitialDir = szFoo;
   }

   const int ret = GetOpenFileName(&ofn);
   if (ret == 0)
      return S_OK;

   xml_document<> xmlDoc;
   float FlipperPhysicsMass, FlipperPhysicsStrength, FlipperPhysicsElasticity, FlipperPhysicsScatter, FlipperPhysicsTorqueDamping, FlipperPhysicsTorqueDampingAngle, FlipperPhysicsReturnStrength, FlipperPhysicsElasticityFalloff, FlipperPhysicsFriction, FlipperPhysicsCoilRampUp;
   try
   {
      std::stringstream buffer;
      std::ifstream myFile(ofn.lpstrFile);
      buffer << myFile.rdbuf();
      myFile.close();

      std::string content(buffer.str());
      xmlDoc.parse<0>(&content[0]);
      xml_node<> *root = xmlDoc.first_node("physics");
      xml_node<> *physTab = root->first_node("table");
      xml_node<> *physFlip = root->first_node("flipper");

      char str[16];
      float val;
      strcpy_s(str, physTab->first_node("gravityConstant")->value());
      sscanf_s(str, "%f", &val);
      put_Gravity(val);

      strcpy_s(str, physTab->first_node("contactFriction")->value());
      sscanf_s(str, "%f", &val);
      put_Friction(val);

      strcpy_s(str, physTab->first_node("elasticity")->value());
      sscanf_s(str, "%f", &val);
      put_Elasticity(val);

      strcpy_s(str, physTab->first_node("elasticityFalloff")->value());
      sscanf_s(str, "%f", &val);
      put_ElasticityFalloff(val);

      strcpy_s(str, physTab->first_node("playfieldScatter")->value());
      sscanf_s(str, "%f", &val);
      put_Scatter(val);

      strcpy_s(str, physTab->first_node("defaultElementScatter")->value());
      sscanf_s(str, "%f", &val);
      put_DefaultScatter(val);

      strcpy_s(str, physFlip->first_node("speed")->value());
      sscanf_s(str, "%f", &FlipperPhysicsMass);
      
      strcpy_s(str, physFlip->first_node("strength")->value());
      sscanf_s(str, "%f", &FlipperPhysicsStrength);

      strcpy_s(str, physFlip->first_node("elasticity")->value());
      sscanf_s(str, "%f", &FlipperPhysicsElasticity);

      strcpy_s(str, physFlip->first_node("scatter")->value());
      sscanf_s(str, "%f", &FlipperPhysicsScatter);

      strcpy_s(str, physFlip->first_node("eosTorque")->value());
      sscanf_s(str, "%f", &FlipperPhysicsTorqueDamping);

      strcpy_s(str, physFlip->first_node("eosTorqueAngle")->value());
      sscanf_s(str, "%f", &FlipperPhysicsTorqueDampingAngle);

      strcpy_s(str, physFlip->first_node("returnStrength")->value());
      sscanf_s(str, "%f", &FlipperPhysicsReturnStrength);

      strcpy_s(str, physFlip->first_node("elasticityFalloff")->value());
      sscanf_s(str, "%f", &FlipperPhysicsElasticityFalloff);

      strcpy_s(str, physFlip->first_node("friction")->value());
      sscanf_s(str, "%f", &FlipperPhysicsFriction);

      strcpy_s(str, physFlip->first_node("coilRampUp")->value());
      sscanf_s(str, "%f", &FlipperPhysicsCoilRampUp);
   }
   catch (...)
   {
      ShowError("Error parsing physics settings file");
   }
   xmlDoc.clear();

   for (int i = 0; i < m_vedit.Size(); i++)
      if (m_vedit.ElementAt(i)->GetItemType() == eItemFlipper)
      {
         Flipper * const flipper = (Flipper *)m_vedit.ElementAt(i);
         flipper->put_Mass(FlipperPhysicsMass); // was speed previously
         flipper->put_Strength(FlipperPhysicsStrength);
         flipper->put_Elasticity(FlipperPhysicsElasticity);
         flipper->put_Return(FlipperPhysicsReturnStrength);
         flipper->put_ElasticityFalloff(FlipperPhysicsElasticityFalloff);
         flipper->put_Friction(FlipperPhysicsFriction);
         flipper->put_RampUp(FlipperPhysicsCoilRampUp);
         flipper->put_Scatter(FlipperPhysicsScatter);
         flipper->put_EOSTorque(FlipperPhysicsTorqueDamping);
         flipper->put_EOSTorqueAngle(FlipperPhysicsTorqueDampingAngle);
      }

   return S_OK;
}

STDMETHODIMP PinTable::ExportPhysics()
{
   bool foundflipper = false;
   int i;
   for (i = 0; i < m_vedit.Size(); i++)
   {
      if (m_vedit.ElementAt(i)->GetItemType() == eItemFlipper)
      {
         foundflipper = true;
         break;
      }
   }

   if (!foundflipper)
   {
      ShowError("No Flipper found to copy settings from");
      return S_OK;
   }

   Flipper * const flipper = (Flipper *)m_vedit.ElementAt(i);

   char szFileName[1024];
   char szInitialDir[1024];
   szFileName[0] = '\0';

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_hinst;
   ofn.hwndOwner = g_pvp->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrDefExt = "vpp";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   const HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
   char szFoo[MAX_PATH];
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
      lstrcpy(szFoo, "c:\\");
      ofn.lpstrInitialDir = szFoo;
   }

   const int ret = GetSaveFileName(&ofn);
   if (ret == 0)
      return S_OK;

   xml_document<> xmlDoc;
   xml_node<>*dcl = xmlDoc.allocate_node(node_declaration);
   dcl->append_attribute(xmlDoc.allocate_attribute("version", "1.0"));
   dcl->append_attribute(xmlDoc.allocate_attribute("encoding", "utf-8"));
   xmlDoc.append_node(dcl);

   //root node
   xml_node<>*root = xmlDoc.allocate_node(node_element, "physics");
   xml_node<>*physFlip = xmlDoc.allocate_node(node_element, "flipper");
   xml_node<>*physTab = xmlDoc.allocate_node(node_element, "table");

   float val;
   char fspeed[16], fstrength[16], felasticity[16], fscatter[16], ftorquedamping[16], ftorquedampingangle[16], freturn[16], felasticityFalloff[16], fFriction[16], fRampup[16];

   flipper->get_Mass(&val); // was speed
   sprintf_s(fspeed, "%f", val);
   xml_node<>*flipSpeed = xmlDoc.allocate_node(node_element, "speed", fspeed);
   physFlip->append_node(flipSpeed);

   flipper->get_Strength(&val);
   sprintf_s(fstrength, "%f", val);
   xml_node<>*flipPhysStrength = xmlDoc.allocate_node(node_element, "strength", fstrength);
   physFlip->append_node(flipPhysStrength);

   flipper->get_Elasticity(&val);
   sprintf_s(felasticity, "%f", val);
   xml_node<>*flipElasticity = xmlDoc.allocate_node(node_element, "elasticity", felasticity);
   physFlip->append_node(flipElasticity);

   flipper->get_Scatter(&val); // was scatter angle
   sprintf_s(fscatter, "%f", val);
   xml_node<>*flipScatter = xmlDoc.allocate_node(node_element, "scatter", fscatter);
   physFlip->append_node(flipScatter);

   flipper->get_EOSTorque(&val);
   sprintf_s(ftorquedamping, "%f", val);
   xml_node<>*flipTorqueDamping = xmlDoc.allocate_node(node_element, "eosTorque", ftorquedamping);
   physFlip->append_node(flipTorqueDamping);

   flipper->get_EOSTorqueAngle(&val);
   sprintf_s(ftorquedampingangle, "%f", val);
   xml_node<>*flipTorqueDampingAngle = xmlDoc.allocate_node(node_element, "eosTorqueAngle", ftorquedampingangle);
   physFlip->append_node(flipTorqueDampingAngle);

   flipper->get_Return(&val);
   sprintf_s(freturn, "%f", val);
   xml_node<>*flipReturnStrength = xmlDoc.allocate_node(node_element, "returnStrength", freturn);
   physFlip->append_node(flipReturnStrength);

   flipper->get_ElasticityFalloff(&val);
   sprintf_s(felasticityFalloff, "%f", val);
   xml_node<>*flipElasticityFalloff = xmlDoc.allocate_node(node_element, "elasticityFalloff", felasticityFalloff);
   physFlip->append_node(flipElasticityFalloff);

   flipper->get_Friction(&val);
   sprintf_s(fFriction, "%f", val);
   xml_node<>*flipfriction = xmlDoc.allocate_node(node_element, "friction", fFriction);
   physFlip->append_node(flipfriction);

   flipper->get_RampUp(&val);
   sprintf_s(fRampup, "%f", val);
   xml_node<>*flipCoilRampUp = xmlDoc.allocate_node(node_element, "coilRampUp", fRampup);
   physFlip->append_node(flipCoilRampUp);

   char tgravity[16], tFriction[16], tDefaultScatter[16], telasticity[16], telasticityFallOff[16], tcontactScatter[16];

   get_Gravity(&val);
   sprintf_s(tgravity, "%f", val);
   xml_node<>*tabGravityConst = xmlDoc.allocate_node(node_element, "gravityConstant", tgravity);
   physTab->append_node(tabGravityConst);

   get_Friction(&val);
   sprintf_s(tFriction, "%f", val);
   xml_node<>*tabContactFriction = xmlDoc.allocate_node(node_element, "contactFriction", tFriction);
   physTab->append_node(tabContactFriction);

   get_Elasticity(&val);
   sprintf_s(telasticity, "%f", val);
   xml_node<>*tabElasticity = xmlDoc.allocate_node(node_element, "elasticity", telasticity);
   physTab->append_node(tabElasticity);

   get_ElasticityFalloff(&val);
   sprintf_s(telasticityFallOff, "%f", val);
   xml_node<>*tabElasticityFalloff = xmlDoc.allocate_node(node_element, "elasticityFalloff", telasticityFallOff);
   physTab->append_node(tabElasticityFalloff);

   get_DefaultScatter(&val);
   sprintf_s(tDefaultScatter, "%f", val);
   xml_node<>*tabScatterAngle = xmlDoc.allocate_node(node_element, "defaultElementScatter", tDefaultScatter);
   physTab->append_node(tabScatterAngle);

   get_Scatter(&val);
   sprintf_s(tcontactScatter, "%f", val);
   xml_node<>*tabContactScatterAngle = xmlDoc.allocate_node(node_element, "playfieldScatter", tcontactScatter);
   physTab->append_node(tabContactScatterAngle);

   xml_node<>*settingName = xmlDoc.allocate_node(node_element, "name", m_szTitle);
   root->append_node(settingName);
   root->append_node(physTab);
   root->append_node(physFlip);
   xmlDoc.append_node(root);

   std::ofstream myfile(ofn.lpstrFile);
   myfile << xmlDoc;
   myfile.close();
   return S_OK;
}

STDMETHODIMP PinTable::get_EnableDecals(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fRenderDecals);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableDecals(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_fRenderDecals = !!newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_ShowDT(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_BG_current_set == BG_DESKTOP || m_BG_current_set == BG_FSS); // DT & FSS

   return S_OK;
}

STDMETHODIMP PinTable::put_ShowDT(VARIANT_BOOL newVal)
{
   //STARTUNDO // not saved/just a simple toggle, so do not trigger undo

   m_BG_current_set = (!!newVal) ? (m_BG_enable_FSS ? BG_FSS : BG_DESKTOP) : BG_FULLSCREEN;

   //STOPUNDO

   SetDirtyDraw();

   return S_OK;
}

STDMETHODIMP PinTable::get_ReflectElementsOnPlayfield(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fReflectElementsOnPlayfield);

   return S_OK;
}

STDMETHODIMP PinTable::put_ReflectElementsOnPlayfield(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_fReflectElementsOnPlayfield = (!!newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_EnableEMReels(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fRenderEMReels);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableEMReels(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_fRenderEMReels = !!newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalDifficulty(float *pVal)
{
	*pVal = m_globalDifficulty*100.f; //VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDifficulty(float newVal)
{
   if (!g_pplayer)
   {  //VP Editor
      int tmp;
      const HRESULT hr = GetRegInt("Player", "GlobalDifficulty", &tmp);
      if (hr == S_OK)
		  m_globalDifficulty = (float)tmp*(float)(1.0 / 100.0);
      else
      {
         if (newVal < 0) newVal = 0;
            else if (newVal > 100.0f) newVal = 100.0f;
         STARTUNDO
		 m_globalDifficulty = newVal*(float)(1.0 / 100.0);
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_Accelerometer(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_tblAccelerometer);

   return S_OK;
}

STDMETHODIMP PinTable::put_Accelerometer(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_tblAccelerometer = !!newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_AccelNormalMount(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_tblAccelNormalMount);

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelNormalMount(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_tblAccelNormalMount = !!newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAngle(float *pVal)
{
   *pVal = m_tblAccelAngle;

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAngle(float newVal)
{
   STARTUNDO
   m_tblAccelAngle = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DeadZone(int *pVal)
{
   int deadz;
   const HRESULT hr = GetRegInt("Player", "DeadZone", &deadz);
   if (hr != S_OK)
      deadz = 0;

   *pVal = deadz;

   return S_OK;
}

STDMETHODIMP PinTable::put_DeadZone(int newVal)
{
   if (newVal > 100) newVal = 100;
   if (newVal < 0) newVal = 0;

   SetRegValue("Player", "DeadZone", REG_DWORD, &newVal, 4);

   return S_OK;
}

#ifdef UNUSED_TILT
STDMETHODIMP PinTable::get_JoltAmount(int *pVal)
{
   *pVal = m_jolt_amount;

   return S_OK;
}

STDMETHODIMP PinTable::put_JoltAmount(int newVal)
{
   STARTUNDO
      m_jolt_amount = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_TiltAmount(int *pVal)
{
   *pVal = m_tilt_amount; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_TiltAmount(int newVal)
{
   STARTUNDO
      m_tilt_amount = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_JoltTriggerTime(int *pVal)
{
   *pVal = m_jolt_trigger_time;

   return S_OK;
}

STDMETHODIMP PinTable::put_JoltTriggerTime(int newVal)
{
   STARTUNDO
      m_jolt_trigger_time = newVal;
   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_TiltTriggerTime(int *pVal)
{
   *pVal = m_tilt_trigger_time;

   return S_OK;
}

STDMETHODIMP PinTable::put_TiltTriggerTime(int newVal)
{
   STARTUNDO
   m_tilt_trigger_time = newVal;
   STOPUNDO

   return S_OK;
}
#endif

STDMETHODIMP PinTable::get_BallFrontDecal(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szBallImageFront, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallFrontDecal(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_szBallImageFront,szImage);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::FireKnocker(int Count)
{
   if (g_pplayer)
      hid_knock(Count);

   return S_OK;
}

STDMETHODIMP PinTable::QuitPlayer(int CloseType)
{
   if (g_pplayer)
   {
      g_pplayer->m_fCloseType = CloseType;
      ExitApp();
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Version(int *pVal)
{
   *pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
   return S_OK;
}

STDMETHODIMP PinTable::get_VPBuildVersion(int *pVal)
{
	*pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
	return S_OK;
}

STDMETHODIMP PinTable::get_VersionMajor(int *pVal)
{
	*pVal = VP_VERSION_MAJOR;
	return S_OK;
}

STDMETHODIMP PinTable::get_VersionMinor(int *pVal)
{
	*pVal = VP_VERSION_MINOR;
	return S_OK;
}

STDMETHODIMP PinTable::get_VersionRevision(int *pVal)
{
	*pVal = VP_VERSION_REV;
	return S_OK;
}

void PinTable::InvokeBallBallCollisionCallback(Ball *b1, Ball *b2, float hitVelocity)
{
   if (g_pplayer)
   {
      CComPtr<IDispatch> disp;
      m_pcv->m_pScript->GetScriptDispatch(NULL, &disp);

      static wchar_t FnName[] = L"OnBallBallCollision";
      LPOLESTR fnNames = FnName;

      DISPID dispid;
      HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &dispid);

      if (SUCCEEDED(hr))  // did we find the collision callback function?
      {
         // note: arguments are passed in reverse order
         CComVariant rgvar[3] = {
            CComVariant(hitVelocity),
            CComVariant(static_cast<IDispatch*>(b2->m_pballex)),
            CComVariant(static_cast<IDispatch*>(b1->m_pballex))
         };
         DISPPARAMS dispparams = { rgvar, NULL, 3, 0 };

         disp->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparams, NULL, NULL, NULL);
      }
   }
}
