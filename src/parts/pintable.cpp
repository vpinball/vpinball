#include "stdafx.h"
#include "vpversion.h"
#include "resource.h"
#include "hash.h"
#include <algorithm>
#include <atlsafe.h>
#include "objloader.h"
#include "tinyxml2/tinyxml2.h"
#include <fstream>
#include <sstream>
#include "renderer/Shader.h"
#include "captureExt.h"
#include "freeimage.h"
#include "ThreadPool.h"
#include "scalefx.h"

#include "serial.h"
static serial Serial;

#define HASHLENGTH 16

constexpr unsigned char TABLE_KEY[] = "Visual Pinball";
//constexpr unsigned char PARAPHRASE_KEY[] = { 0xB4, 0x0B, 0xBE, 0x37, 0xC3, 0x0C, 0x8E, 0xA1, 0x5A, 0x05, 0xDF, 0x1B, 0x2D, 0x02, 0xEF, 0x8D };

#pragma region ScriptGlobalTable

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void ScriptGlobalTable::Init(VPinball *vpinball, PinTable *pt)
{
   m_pt = pt;
   m_vpinball = vpinball;
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
      const float a  = ANGTORAD(Angle);
      const float sn = sinf(a) * Force;
      const float cs = cosf(a) * Force;

      if (g_pplayer->m_legacyNudge)
      {
          g_pplayer->m_legacyNudgeBack.x =  sn * g_pplayer->m_legacyNudgeStrength;
          g_pplayer->m_legacyNudgeBack.y = -cs * g_pplayer->m_legacyNudgeStrength;
          g_pplayer->m_legacyNudgeTime = 100;
      }
      else
      {
          g_pplayer->m_tableVel.x +=  sn;
          g_pplayer->m_tableVel.y += -cs;
      }
   }

   return S_OK;
}


STDMETHODIMP ScriptGlobalTable::NudgeGetCalibration(VARIANT *XMax, VARIANT *YMax, VARIANT *XGain, VARIANT *YGain, VARIANT *DeadZone, VARIANT *TiltSensitivty)
{
	int tmp;

	if (g_pvp->m_settings.LoadValue(Settings::Player, "PBWAccelGainX"s, tmp))
		CComVariant(tmp).Detach(XGain);
	if (g_pvp->m_settings.LoadValue(Settings::Player, "PBWAccelGainY"s, tmp))
		CComVariant(tmp).Detach(YGain);
	if (g_pvp->m_settings.LoadValue(Settings::Player, "PBWAccelMaxX"s, tmp))
		CComVariant(tmp).Detach(XMax);
	if (g_pvp->m_settings.LoadValue(Settings::Player, "PBWAccelMaxY"s, tmp))
		CComVariant(tmp).Detach(YMax);
	if (g_pvp->m_settings.LoadValue(Settings::Player, "DeadZone"s, tmp))
		CComVariant(tmp).Detach(DeadZone);
	if (g_pvp->m_settings.LoadValue(Settings::Player, "TiltSensitivity"s, tmp))
		CComVariant(tmp).Detach(TiltSensitivty);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::NudgeSetCalibration(int XMax, int YMax, int XGain, int YGain, int DeadZone, int TiltSensitivity)
{
	int newvalue;

	newvalue = XGain;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainX"s, newvalue);

	newvalue = YGain;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainY"s, newvalue);

	newvalue = DeadZone;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	if (newvalue > 100) { newvalue = 100; }
	g_pvp->m_settings.SaveValue(Settings::Player, "DeadZone"s, newvalue);

	newvalue = XMax;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	if (newvalue > 100) { newvalue = 100; }
	g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxX"s, newvalue);

	newvalue = YMax;
	if ((SSIZE_T)newvalue < 0) { newvalue = 0; }
	if (newvalue > 100) { newvalue = 100; }
	g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxY"s, newvalue);

	g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensCB"s, TiltSensitivity > 0);
	if (TiltSensitivity > 0)
	{
		newvalue = TiltSensitivity;
		g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensValue"s, newvalue);
		g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensitivity"s, newvalue);
	}
	else
		g_pvp->m_settings.DeleteValue(Settings::Player, "TiltSensitivity"s);

	m_pt->ReadAccelerometerCalibration();

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::NudgeSensorStatus(VARIANT *XNudge, VARIANT *YNudge)
{
	CComVariant(m_pt->m_tblNudgeRead.x).Detach(XNudge);
	CComVariant(m_pt->m_tblNudgeRead.y).Detach(YNudge);
	m_pt->m_tblNudgeRead = Vertex2D(0.f,0.f);

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::NudgeTiltStatus(VARIANT *XPlumb, VARIANT *YPlumb, VARIANT *Tilt)
{
	CComVariant(m_pt->m_tblNudgePlumb.x).Detach(XPlumb);
	CComVariant(m_pt->m_tblNudgePlumb.y).Detach(YPlumb);
	m_pt->m_tblNudgePlumb = Vertex2D(0.f,0.f);
	CComVariant(m_pt->m_tblNudgeReadTilt).Detach(Tilt);
	m_pt->m_tblNudgeReadTilt = 0.0f;

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlaySound(BSTR bstr, long LoopCount, float volume, float pan, float randompitch, long pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade)
{
   if (g_pplayer && g_pplayer->m_PlaySound) m_pt->PlaySound(bstr, LoopCount, volume, pan, randompitch, pitch, usesame, restart, front_rear_fade);

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
   if (g_pplayer && g_pplayer->m_PlaySound)
      m_pt->StopSound(Sound);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlayMusic(BSTR str, float volume)
{
   if (g_pplayer && g_pplayer->m_PlayMusic)
   {
      EndMusic();

      g_pplayer->m_audio = new AudioPlayer();
      const float MusicVolume = max(min((float)g_pplayer->m_MusicVolume*m_pt->m_TableMusicVolume*volume, 100.0f), 0.0f) * (float)(1.0/100.0);

      char szT[512];
      WideCharToMultiByteNull(CP_ACP, 0, str, -1, szT, 512, nullptr, nullptr);

      if (!g_pplayer->m_audio->MusicInit(szT, MusicVolume))
      {
         delete g_pplayer->m_audio;
         g_pplayer->m_audio = nullptr;
      }
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndMusic()
{
   delete g_pplayer->m_audio;
   g_pplayer->m_audio = nullptr;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_MusicVolume(float volume)
{
	if (g_pplayer && g_pplayer->m_PlayMusic)
	{
		const float MusicVolume = max(min((float)g_pplayer->m_MusicVolume*m_pt->m_TableMusicVolume*volume, 100.0f), 0.0f) * (float)(1.0/100.0);
		g_pplayer->m_audio->MusicVolume(MusicVolume);
	}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Global");

   return S_OK;
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

STDMETHODIMP ScriptGlobalTable::get_StagedLeftFlipperKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eStagedLeftFlipperKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StagedRightFlipperKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eStagedRightFlipperKey];

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

STDMETHODIMP ScriptGlobalTable::get_JoyCustomKey(long index, long *pVal)
{
   switch (index)
   {
   case 1: *pVal = g_pplayer->m_pininput.m_joycustom1key; break;
   case 2: *pVal = g_pplayer->m_pininput.m_joycustom2key; break;
   case 3: *pVal = g_pplayer->m_pininput.m_joycustom3key; break;
   case 4: *pVal = g_pplayer->m_pininput.m_joycustom4key; break;
   default: return E_FAIL;
   }

   return S_OK;
}

bool ScriptGlobalTable::GetTextFileFromDirectory(const string& szfilename, const string& dirname, BSTR *pContents)
{
   string szPath;
   if (!dirname.empty())
      szPath = m_vpinball->m_szMyPath + dirname;
   // else: use current directory
   szPath += szfilename;

   int len;
   BYTE *szContents;
   if (RawReadFromFile(szPath.c_str(), &len, (char **)&szContents))
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
         WCHAR * const wzContents = new WCHAR[len + 1];

         MultiByteToWideCharNull(encoding, 0, (char *)szDataStart, len, wzContents, len + 1);

         *pContents = SysAllocString(wzContents);
         delete[] wzContents;
      }

      delete[] szContents;

      return true;
   }

   return false;
}

STDMETHODIMP ScriptGlobalTable::GetCustomParam(long index, BSTR *param)
{
    if (index <= 0 || index > MAX_CUSTOM_PARAM_INDEX)
        return E_FAIL;

    *param = SysAllocString(m_vpinball->m_customParameters[index-1]);
    return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Setting(BSTR Section, BSTR SettingName, BSTR *param)
{
   string value;
   const int sectionLen = (int)lstrlenW(Section);
   char *const sectionSz = new char[sectionLen + 1];
   WideCharToMultiByteNull(CP_ACP, 0, Section, -1, sectionSz, sectionLen + 1, nullptr, nullptr);
   const int settingLen = (int)lstrlenW(SettingName);
   char *const settingSz = new char[settingLen + 1];
   WideCharToMultiByteNull(CP_ACP, 0, SettingName, -1, settingSz, settingLen + 1, nullptr, nullptr);
   if (g_pvp->m_settings.LoadValue(Settings::GetSection(sectionSz), settingSz, value))
   {
      const int len = (int)value.length() + 1;
      WCHAR *const wzT = new WCHAR[len];
      MultiByteToWideCharNull(CP_ACP, 0, value.c_str(), -1, wzT, len);
      *param = SysAllocString(wzT);
      delete [] wzT;
      delete [] sectionSz;
      delete [] settingSz;
      return S_OK;
   }
   delete [] sectionSz;
   delete [] settingSz;
   return E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::GetTextFile(BSTR FileName, BSTR *pContents)
{
   char szFileName[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, FileName, -1, szFileName, MAX_PATH, nullptr, nullptr);

   for(size_t i = 0; i < std::size(defaultFileNameSearch); ++i)
      if(GetTextFileFromDirectory(defaultFileNameSearch[i] + szFileName, defaultPathSearch[i], pContents))
         return S_OK;

   return S_FALSE;
}

STDMETHODIMP ScriptGlobalTable::get_UserDirectory(BSTR *pVal)
{
   string szPath = m_vpinball->m_szMyPath + "user" + PATH_SEPARATOR_CHAR;
   if (!DirExists(szPath))
   {
      szPath = m_vpinball->m_currentTablePath + "user" + PATH_SEPARATOR_CHAR;
      if (!DirExists(szPath))
      {
         szPath = PATH_USER;
         if (!DirExists(szPath))
            return S_FALSE;
      }
   }
   const WCHAR *const wzPath = MakeWide(szPath);
   *pVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_TablesDirectory(BSTR *pVal)
{
   string szPath = m_vpinball->m_szMyPath + "tables" + PATH_SEPARATOR_CHAR;
   if (!DirExists(szPath))
   {
      szPath = m_vpinball->m_currentTablePath + "tables" + PATH_SEPARATOR_CHAR;
      if (!DirExists(szPath))
      {
         szPath = PATH_TABLES;
         if (!DirExists(szPath))
            return S_FALSE;
      }
   }
   const WCHAR *const wzPath = MakeWide(szPath);
   *pVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_MusicDirectory(VARIANT pSubDir, BSTR *pVal)
{
   // Optional sub directory parameter must be either missing or a string
   if (V_VT(&pSubDir) != VT_ERROR && V_VT(&pSubDir) != VT_EMPTY && V_VT(&pSubDir) != VT_BSTR)
      return S_FALSE;

   const string endPath = V_VT(&pSubDir) == VT_BSTR ? (MakeString(V_BSTR(&pSubDir)) + PATH_SEPARATOR_CHAR) : string();
   string szPath = m_vpinball->m_szMyPath + "music"s + PATH_SEPARATOR_CHAR + endPath;
   if (!DirExists(szPath))
   {
      szPath = m_vpinball->m_currentTablePath + "music"s + PATH_SEPARATOR_CHAR + endPath;
      if (!DirExists(szPath))
      {
         szPath = PATH_MUSIC + endPath;
         if (!DirExists(szPath))
            return S_FALSE;
      }
   }
   const WCHAR *const wzPath = MakeWide(szPath);
   *pVal = SysAllocString(wzPath);
   delete[] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ScriptsDirectory(BSTR *pVal)
{
   string szPath = m_vpinball->m_szMyPath + "scripts" + PATH_SEPARATOR_CHAR;
   if (!DirExists(szPath))
   {
      szPath = m_vpinball->m_currentTablePath + "scripts" + PATH_SEPARATOR_CHAR;
      if (!DirExists(szPath))
      {
         szPath = PATH_SCRIPTS;
         if (!DirExists(szPath))
            return S_FALSE;
      }
   }
   const WCHAR *const wzPath = MakeWide(szPath);
   *pVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlatformOS(BSTR *pVal)
{
   const WCHAR *const wzPath = MakeWide(GET_PLATFORM_OS);
   *pVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlatformCPU(BSTR *pVal)
{
   const WCHAR *const wzPath = MakeWide(GET_PLATFORM_CPU);
   *pVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlatformBits(BSTR *pVal)
{
   const WCHAR *const wzPath = MakeWide(GET_PLATFORM_BITS);
   *pVal = SysAllocString(wzPath);
   delete [] wzPath;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_ShowCursor(VARIANT_BOOL enable)
{
   /*if(VBTOb(enable)) // not needed, otherwise hides mouse cursor in windowed mode
   {
      while (ShowCursor(FALSE) >= 0) ;
      while (ShowCursor(TRUE) < 0) ;
   }
   else
   {
      while (ShowCursor(TRUE) < 0) ;
      while (ShowCursor(FALSE) >= 0) ;
   }*/

   ShowCursor(VBTOb(enable) ? TRUE : FALSE);

   return S_OK;
}

#ifdef _WIN64
STDMETHODIMP ScriptGlobalTable::get_GetPlayerHWnd(SIZE_T *pVal)
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
      *pVal = (size_t)g_pplayer->GetHwnd();
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::AddObject(BSTR Name, IDispatch *pdisp)
{
   if (!g_pplayer)
      return E_FAIL;

   m_pt->m_pcv->AddTemporaryItem(Name, pdisp);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value)
{
   HRESULT hr;

   const wstring wzPath = m_vpinball->m_wzMyPath + L"user" + PATH_SEPARATOR_WCHAR + L"VPReg.stg";

   IStorage *pstgRoot;
   if (FAILED(hr = StgOpenStorage(wzPath.c_str(), nullptr, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgRoot)))
   {
      // Registry file does not exist - create it
      if (FAILED(hr = StgCreateDocfile(wzPath.c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
      {
         const wstring wzMkPath = m_vpinball->m_wzMyPath + L"user";
         if (_wmkdir(wzMkPath.c_str()) != 0)
            return hr;

         if (FAILED(hr = StgCreateDocfile(wzPath.c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
            return hr;
      }
   }

   IStorage *pstgTable;
   if (FAILED(hr = pstgRoot->OpenStorage(TableName, nullptr, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgTable)))
   {
      // Table file does not exist
      if (FAILED(hr = pstgRoot->CreateStorage(TableName, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgTable)))
      {
         pstgRoot->Release();
         return hr;
      }
   }

   IStream *pstmValue;
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
   HRESULT hr;

   const wstring wzPath = m_vpinball->m_wzMyPath + L"user" + PATH_SEPARATOR_WCHAR + L"VPReg.stg";

   IStorage *pstgRoot;
   if (FAILED(hr = StgOpenStorage(wzPath.c_str(), nullptr, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgRoot)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      return S_OK;
   }

   IStorage* pstgTable;
   if (FAILED(hr = pstgRoot->OpenStorage(TableName, nullptr, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgTable)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      pstgRoot->Release();
      return S_OK;
   }

   IStream* pstmValue;
   if (FAILED(hr = pstgTable->OpenStream(ValueName, 0, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmValue)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      pstgTable->Release();
      pstgRoot->Release();
      return S_OK;
   }

   STATSTG statstg;
   pstmValue->Stat(&statstg, STATFLAG_NONAME);

   const int size = statstg.cbSize.LowPart / sizeof(WCHAR);

   WCHAR *wzT = new WCHAR[size + 1];

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

STDMETHODIMP ScriptGlobalTable::get_FrameIndex(long *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = g_pplayer->m_overall_frames;

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
   if (g_pplayer)
      g_pplayer->m_globalEmissionScale = dequantizeUnsignedPercent(newVal);
   return S_OK;
}*/

STDMETHODIMP ScriptGlobalTable::get_NightDay(int *pVal)
{
   if (g_pplayer)
      *pVal = quantizeUnsignedPercent(g_pplayer->m_globalEmissionScale);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ShowDT(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pt->m_BG_current_set == BG_DESKTOP || m_pt->m_BG_current_set == BG_FSS); // DT & FSS
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ShowFSS(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pt->m_BG_current_set == BG_FSS);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::UpdateMaterial(BSTR pVal, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity,
   OLE_COLOR base, OLE_COLOR glossy, OLE_COLOR clearcoat, VARIANT_BOOL isMetal, VARIANT_BOOL opacityActive,
   float elasticity, float elasticityFalloff, float friction, float scatterAngle)
{
   if (!g_pplayer)
      return E_POINTER;

   char Name[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, pVal, -1, Name, MAX_PATH, nullptr, nullptr);

   Material * const pMat = m_pt->GetMaterial(Name);
   if (pMat != &m_vpinball->m_dummyMaterial)
   {
      pMat->m_fWrapLighting = wrapLighting;
      pMat->m_fRoughness = roughness;
      pMat->m_fGlossyImageLerp = glossyImageLerp;
      pMat->m_fThickness = thickness;
      pMat->m_fEdge = edge;
      pMat->m_fEdgeAlpha = edgeAlpha;
      pMat->m_fOpacity = opacity;
      pMat->m_cBase = base;
      pMat->m_cGlossy = glossy;
      pMat->m_cClearcoat = clearcoat;
      pMat->m_type = VBTOb(isMetal) ? Material::MaterialType::METAL : Material::MaterialType::BASIC;
      pMat->m_bOpacityActive = VBTOb(opacityActive);
      pMat->m_fElasticity = elasticity;
      pMat->m_fElasticityFalloff = elasticityFalloff;
      pMat->m_fFriction = friction;
      pMat->m_fScatterAngle = scatterAngle;

      return S_OK;
   }
   else
      return E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::GetMaterial(BSTR pVal, VARIANT *wrapLighting, VARIANT *roughness, VARIANT *glossyImageLerp, VARIANT *thickness, VARIANT *edge, VARIANT *edgeAlpha, VARIANT *opacity,
   VARIANT *base, VARIANT *glossy, VARIANT *clearcoat, VARIANT *isMetal, VARIANT *opacityActive,
   VARIANT *elasticity, VARIANT *elasticityFalloff, VARIANT *friction, VARIANT *scatterAngle)
{
   if (!g_pplayer)
      return E_POINTER;

   char Name[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, pVal, -1, Name, MAX_PATH, nullptr, nullptr);

   const Material * const pMat = m_pt->GetMaterial(Name);
   if (pMat != &m_vpinball->m_dummyMaterial)
   {
      CComVariant(pMat->m_fWrapLighting).Detach(wrapLighting);
      CComVariant(pMat->m_fRoughness).Detach(roughness);
      CComVariant(pMat->m_fGlossyImageLerp).Detach(glossyImageLerp);
      CComVariant(pMat->m_fThickness).Detach(thickness);
      CComVariant(pMat->m_fEdge).Detach(edge);
      CComVariant(pMat->m_fEdgeAlpha).Detach(edgeAlpha);
      CComVariant(pMat->m_fOpacity).Detach(opacity);
      CComVariant(pMat->m_cBase).Detach(base);
      CComVariant(pMat->m_cGlossy).Detach(glossy);
      CComVariant(pMat->m_cClearcoat).Detach(clearcoat);
      CComVariant(pMat->m_type == Material::MaterialType::METAL).Detach(isMetal);
      CComVariant(pMat->m_bOpacityActive).Detach(opacityActive);
      CComVariant(pMat->m_fElasticity).Detach(elasticity);
      CComVariant(pMat->m_fElasticityFalloff).Detach(elasticityFalloff);
      CComVariant(pMat->m_fFriction).Detach(friction);
      CComVariant(pMat->m_fScatterAngle).Detach(scatterAngle);

      return S_OK;
   }
   else
      return E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::UpdateMaterialPhysics(BSTR pVal, float elasticity, float elasticityFalloff, float friction, float scatterAngle)
{
   if (!g_pplayer)
      return E_POINTER;

   char Name[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, pVal, -1, Name, MAX_PATH, nullptr, nullptr);

   Material * const pMat = m_pt->GetMaterial(Name);
   if (pMat != &m_vpinball->m_dummyMaterial)
   {
      pMat->m_fElasticity = elasticity;
      pMat->m_fElasticityFalloff = elasticityFalloff;
      pMat->m_fFriction = friction;
      pMat->m_fScatterAngle = scatterAngle;

      return S_OK;
   }
   else
      return E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::GetMaterialPhysics(BSTR pVal, VARIANT *elasticity, VARIANT *elasticityFalloff, VARIANT *friction, VARIANT *scatterAngle)
{
   if (!g_pplayer)
      return E_POINTER;

   char Name[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, pVal, -1, Name, MAX_PATH, nullptr, nullptr);

   const Material * const pMat = m_pt->GetMaterial(Name);
   if (pMat != &m_vpinball->m_dummyMaterial)
   {
      CComVariant(pMat->m_fElasticity).Detach(elasticity);
      CComVariant(pMat->m_fElasticityFalloff).Detach(elasticityFalloff);
      CComVariant(pMat->m_fFriction).Detach(friction);
      CComVariant(pMat->m_fScatterAngle).Detach(scatterAngle);

      return S_OK;
   }
   else
      return E_FAIL;
}

// only sets the base color
STDMETHODIMP ScriptGlobalTable::MaterialColor(BSTR pVal, OLE_COLOR newVal)
{
   if (!g_pplayer)
      return E_POINTER;

   char Name[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, pVal, -1, Name, MAX_PATH, nullptr, nullptr);

   Material * const pMat = m_pt->GetMaterial(Name);
   if (pMat != &m_vpinball->m_dummyMaterial)
      pMat->m_cBase = newVal;
   else
      return E_FAIL;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::LoadTexture(BSTR imageName, BSTR fileName)
{
   if (!g_pplayer)
      return E_FAIL;
   char szImageName[MAX_PATH], szFileName[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, imageName, -1, szImageName, MAX_PATH, nullptr, nullptr);
   WideCharToMultiByteNull(CP_ACP, 0, fileName, -1, szFileName, MAX_PATH, nullptr, nullptr);
   // Do not allow to load an image with the same name as one of the edited table as they would conflict
   if (g_pplayer->m_pEditorTable->GetImage(szImageName))
      return E_FAIL;
   Texture *image = m_pt->ImportImage(szFileName, szImageName);
   return image->m_pdsBuffer == nullptr ? E_FAIL : S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_WindowWidth(int *pVal)
{
   if (g_pplayer)
      *pVal = g_pplayer->m_wnd_width;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_WindowHeight(int *pVal)
{
   if (g_pplayer)
      *pVal = g_pplayer->m_wnd_height;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDWidth(int pVal)
{
   if (g_pplayer)
      g_pplayer->m_dmd.x = pVal;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDHeight(int pVal)
{
   if (g_pplayer)
      g_pplayer->m_dmd.y = pVal;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDPixels(VARIANT pVal) // assumes VT_UI1 as input //!! use 64bit instead of 8bit to reduce overhead??
{
   if (HasDMDCapture()) // If DMD capture is enabled check if external DMD exists
      return S_OK;

   SAFEARRAY *psa = V_ARRAY(&pVal);

   if (psa && g_pplayer && g_pplayer->m_dmd.x > 0 && g_pplayer->m_dmd.y > 0)
   {
      const int size = g_pplayer->m_dmd.x*g_pplayer->m_dmd.y;
      if (!g_pplayer->m_texdmd
#ifdef DMD_UPSCALE
          || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size*(3*3)))
#else
          || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size))
#endif
      {
         if (g_pplayer->m_texdmd)
         {
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetTextureNull(SHADER_tex_dmd);
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(g_pplayer->m_texdmd);
            delete g_pplayer->m_texdmd;
         }
#ifdef DMD_UPSCALE
         g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmd.x*3, g_pplayer->m_dmd.y*3, BaseTexture::RGBA);
#else
         g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmd.x, g_pplayer->m_dmd.y, BaseTexture::RGBA);
#endif
      }

      DWORD* const data = (DWORD*)g_pplayer->m_texdmd->data(); //!! assumes tex data to be always 32bit

      VARIANT *p;
      SafeArrayAccessData(psa,(void**)&p);
      for (int ofs = 0; ofs < size; ++ofs)
         data[ofs] = V_UI4(&p[ofs]); // store raw values (0..100), let shader do the rest
      SafeArrayUnaccessData(psa);

      if (g_pplayer->m_scaleFX_DMD)
         upscale(data, g_pplayer->m_dmd, true);

      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(g_pplayer->m_texdmd);
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDColoredPixels(VARIANT pVal) //!! assumes VT_UI4 as input //!! use 64bit instead of 32bit to reduce overhead??
{
   if (HasDMDCapture()) // If DMD capture is enabled check if external DMD exists
      return S_OK;

	SAFEARRAY *psa = V_ARRAY(&pVal);

	if (psa && g_pplayer && g_pplayer->m_dmd.x > 0 && g_pplayer->m_dmd.y > 0)
	{
		const int size = g_pplayer->m_dmd.x*g_pplayer->m_dmd.y;
		if (!g_pplayer->m_texdmd
#ifdef DMD_UPSCALE
            || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size*(3*3)))
#else
            || (g_pplayer->m_texdmd->width()*g_pplayer->m_texdmd->height() != size))
#endif
      {
         if (g_pplayer->m_texdmd)
         {
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetTextureNull(SHADER_tex_dmd);
            g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(g_pplayer->m_texdmd);
            delete g_pplayer->m_texdmd;
         }
#ifdef DMD_UPSCALE
         g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmd.x*3, g_pplayer->m_dmd.y*3, BaseTexture::RGBA);
#else
         g_pplayer->m_texdmd = new BaseTexture(g_pplayer->m_dmd.x, g_pplayer->m_dmd.y, BaseTexture::RGBA);
#endif
      }

		DWORD* const data = (DWORD*)g_pplayer->m_texdmd->data(); //!! assumes tex data to be always 32bit

		VARIANT *p;
		SafeArrayAccessData(psa, (void **)&p);
		for (int ofs = 0; ofs < size; ++ofs)
			data[ofs] = V_UI4(&p[ofs]) | 0xFF000000u; // store RGB values and let shader do the rest (set alpha to let shader know that this is RGB and not just brightness)
		SafeArrayUnaccessData(psa);

		if (g_pplayer->m_scaleFX_DMD)
			upscale(data, g_pplayer->m_dmd, false);

		g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(g_pplayer->m_texdmd);
	}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_DisableStaticPrerendering(VARIANT_BOOL *pVal)
{
   if (g_pplayer == nullptr)
      return E_FAIL;
   *pVal = FTOVB(!g_pplayer->IsUsingStaticPrepass());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DisableStaticPrerendering(VARIANT_BOOL newVal)
{
   if (g_pplayer == nullptr)
      return E_FAIL;
   g_pplayer->DisableStaticPrePass(VBTOb(newVal));
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetBalls(LPSAFEARRAY *pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   CComSafeArray<VARIANT> balls((ULONG)g_pplayer->m_vball.size());

   for (size_t i = 0; i < g_pplayer->m_vball.size(); ++i)
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

   CComSafeArray<VARIANT> objs((ULONG)m_pt->m_vedit.size());

   for (size_t i = 0; i < m_pt->m_vedit.size(); ++i)
   {
      IEditable * const pie = m_pt->m_vedit[i];

      CComVariant v = pie->GetISelect()->GetDispatch();
      v.Detach(&objs[(LONG)i]);
   }

   *pVal = objs.Detach();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetElementByName(BSTR name, IDispatch* *pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   for (size_t i = 0; i < m_pt->m_vedit.size(); ++i)
   {
      IEditable * const pie = m_pt->m_vedit[i];

      if (wcscmp(name, pie->GetScriptable()->m_wzName) == 0)
      {
         IDispatch * const id = pie->GetISelect()->GetDispatch();
         id->AddRef();
         *pVal = id;

         return S_OK;
      }
   }

   *pVal = nullptr;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ActiveTable(ITable **pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   m_pt->QueryInterface(IID_ITable, (void**)pVal);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Version(int *pVal)
{
	*pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VPBuildVersion(double *pVal)
{
	*pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV + GIT_REVISION / 10000.0;
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

// Serial/RS232 stuff

STDMETHODIMP ScriptGlobalTable::OpenSerial(BSTR device)
{
   char szDevice[MAX_PATH];
   WideCharToMultiByteNull(CP_ACP, 0, device, -1, szDevice, MAX_PATH, nullptr, nullptr);

   return Serial.open(szDevice) ? S_OK : E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::CloseSerial()
{
   Serial.close();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::FlushSerial()
{
   Serial.flush();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::SetupSerial(int baud, int bits, int parity, int stopbit, VARIANT_BOOL rts, VARIANT_BOOL dtr)
{
   Serial.setup(serial::get_baud(baud),serial::get_bits(bits),parity == 0 ? SERIAL_PARITY_NONE : (parity == 1 ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD),serial::get_stopbit(stopbit));
   Serial.set_rts(VBTOb(rts));
   Serial.set_dtr(VBTOb(dtr));

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::ReadSerial(int size, VARIANT *pVal)
{
   SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, size);

   VARIANT *pData;
   SafeArrayAccessData(psa, (void **)&pData);
   vector<char> data(size,0);
   Serial.read(data);
   for (int i = 0; i < size; ++i)
   {
      pData[i].vt = VT_UI1;
      pData[i].cVal = data[i];
   }
   SafeArrayUnaccessData(psa);

   pVal->vt = VT_ARRAY | VT_VARIANT;
   pVal->parray = psa;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::WriteSerial(VARIANT pVal)
{
   SAFEARRAY *psa = pVal.parray;
   SAFEARRAYBOUND *psafearraybound = &((psa->rgsabound)[0]);
   const LONG size = (LONG)psafearraybound->cElements;

   vector<char> data(size);

   VARIANT state;
   state.vt = VT_UI1;

   for (LONG ofs = 0; ofs < size; ++ofs)
   {
      SafeArrayGetElement(psa, &ofs, &state);
      data[ofs] = state.cVal;
   }

   Serial.write(data);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetSerialDevices(VARIANT *pVal)
{
   static vector<string> availablePorts;
   serial::list_ports(availablePorts);

   SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, (ULONG)availablePorts.size());
   CComVariant varDevice;
   for (LONG i = 0; i < (LONG)availablePorts.size(); ++i)
   {
      varDevice = availablePorts[i].c_str();
      SafeArrayPutElement(psa, &i, &varDevice);
   }

   pVal->vt = VT_ARRAY | VT_VARIANT;
   pVal->parray = psa;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RenderingMode(int *pVal)
{
   if (g_pplayer->m_stereo3D == STEREO_VR)
      *pVal = 2; // VR
   else if ((g_pplayer->m_stereo3D != STEREO_OFF) && g_pplayer->m_stereo3Denabled)
      *pVal = 1; // Stereo 3D (3DTV or anaglyph)
   else
      *pVal = 0; // 2D
   return S_OK;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region PinTable

PinTable::PinTable()
   : m_settings(&(g_pvp->m_settings))
{
   m_savingActive = false;
   m_renderSolid = m_settings.LoadValueWithDefault(Settings::Editor, "RenderSolid"s, true);
   ClearMultiSel();

   m_hbmOffScreen = nullptr;
   m_undo.m_ptable = this;
   m_grid = true;
   m_backdrop = true;
   m_moving = false;

   m_renderDecals = true;
   m_renderEMReels = true;

   m_overridePhysics = 0;
   m_overridePhysicsFlipper = false;

   m_defaultBulbIntensityScaleOnBall = 1.0f;

   SetDefaultPhysics(false);

   m_defaultScatter = 0.f;
   m_nudgeTime = 5.0f;

   m_plungerNormalize = 100;  //Mech-Plunger component adjustment or weak spring, aging
   m_plungerFilter = false;
   m_PhysicsMaxLoops = m_settings.LoadValueWithDefault(Settings::Player, "PhysicsMaxLoops"s, (int)0xFFFFFFFFu);

   m_right = 0.0f;
   m_bottom = 0.0f;

   m_glassTopHeight = m_glassBottomHeight = 210;

   m_BG_enable_FSS = false;
   UpdateCurrentBGSet();
   m_currentBackglassMode = m_BG_current_set;

   CComObject<CodeViewer>::CreateInstance(&m_pcv);
   m_pcv->AddRef();
   m_pcv->Init((IScriptableHost*)this);
   m_pcv->Create(nullptr);

   CComObject<ScriptGlobalTable>::CreateInstance(&m_psgt);
   m_psgt->AddRef();
   m_psgt->Init(m_vpinball, this);

   m_sdsDirtyProp = eSaveClean;
   m_sdsDirtyScript = eSaveClean;
   m_sdsNonUndoableDirty = eSaveClean;
   m_sdsCurrentDirtyState = eSaveClean;

   ZeroMemory(&m_protectionData, sizeof(m_protectionData)); //!!

   m_globalEmissionScale = 1.0f;

   m_numTimesSaved = 0;

   m_pbTempScreenshot = nullptr;

   m_tblMirrorEnabled = false;

   m_hMaterialManager = nullptr;

   m_numMaterials = 0;

   nudge_set_sensitivity((float)m_settings.LoadValueWithDefault(Settings::Player, "NudgeSensitivity"s, 500) * (float)(1.0/1000.0));

   m_difficulty = 0.2f; // easy by default
   m_globalDifficulty = m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, m_difficulty);

   ReadAccelerometerCalibration();

   m_tblAutoStart = m_settings.LoadValueWithDefault(Settings::Player, "Autostart"s, 0) * 10;
   m_tblAutoStartRetry = m_settings.LoadValueWithDefault(Settings::Player, "AutostartRetry"s, 0) * 10;
   m_tblAutoStartEnabled = m_settings.LoadValueWithDefault(Settings::Player, "asenable"s, false);
   m_tblVolmod = (float)m_settings.LoadValueWithDefault(Settings::Player, "Volmod"s, 1000) * (float)(1.0/1000.0);
   m_tblExitConfirm = m_settings.LoadValueWithDefault(Settings::Player, "Exitconfirm"s, 120) * 1000 / 60; // this is supposed to be seconds, but is seconds*60  :/

   m_global3DZPD = m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DZPD"s, 0.5f);
   m_3DZPD = 0.5f;
   m_global3DMaxSeparation = m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DMaxSeparation"s, 0.03f);
   m_3DmaxSeparation = 0.03f;
   m_global3DOffset = m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DOffset"s, 0.f);
   m_3DOffset = 0.0f;
   m_overwriteGlobalStereo3D = false;

   m_tblNudgeRead = Vertex2D(0.f,0.f);
   m_tblNudgeReadTilt = 0.0f;
   m_tblNudgePlumb = Vertex2D(0.f,0.f);

#ifdef UNUSED_TILT
   m_jolt_amount = m_settings.LoadValueWithDefault(Settings::Player, "JoltAmount"s, 500);
   m_tilt_amount = m_settings.LoadValueWithDefault(Settings::Player, "TiltAmount"s, 950);
   m_jolt_trigger_time = m_settings.LoadValueWithDefault(Settings::Player, "JoltTriggerTime"s, 1000);
   m_tilt_trigger_time = m_settings.LoadValueWithDefault(Settings::Player, "TiltTriggerTime"s, 10000);
#endif
}

void PinTable::ReadAccelerometerCalibration()
{
   m_tblAccelerometer = m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true); // true if electronic accelerometer enabled
   m_tblAccelNormalMount = m_settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, true); // true is normal mounting (left hand coordinates)

   m_tblAccelAngle = 0.0f; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   const bool accel = m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false);
   if (accel)
      m_tblAccelAngle = (float)m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 0);

   m_tblAccelAmp.x = dequantizeUnsignedPercentNoClamp(m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150));
   m_tblAccelAmp.y = dequantizeUnsignedPercentNoClamp(m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150));
   m_tblAccelMax.x = m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100) * JOYRANGEMX / 100;
   m_tblAccelMax.y = m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100) * JOYRANGEMX / 100;

   //!! bug!! If tilt sensitivity is not set, it's supposed to disable analog tilting, see KeysConfigDialog.cpp
   plumb_set_sensitivity((float)m_settings.LoadValueWithDefault(Settings::Player, "TiltSensitivity"s, 400) * (float)(1.0 / 1000.0));

   if (g_pplayer)
      g_pplayer->m_pininput.LoadSettings(m_settings);
}

PinTable::~PinTable()
{
   for (size_t i = 0; i < m_vedit.size(); i++)
      m_vedit[i]->Release();

   m_vpinball->m_ps.ClearStoppedCopiedWavs();

   if (!m_isLiveInstance)
   { // Sounds, Fonts and images are owned by the editor's table, live table instances just use shallow copy, so don't release them
      for (size_t i = 0; i < m_vsound.size(); i++)
         delete m_vsound[i];

      for (size_t i = 0; i < m_vimage.size(); i++)
         delete m_vimage[i];
  
      for (size_t i = 0; i < m_vfont.size(); i++)
      {
         m_vfont[i]->UnRegister();
         delete m_vfont[i];
      }
   }

   for (size_t i = 0; i < m_vliveimage.size(); i++)
      delete m_vliveimage[i];

   for (size_t i = 0; i < m_materials.size(); ++i)
      delete m_materials[i];

   for (size_t i = 0; i < m_vrenderprobe.size(); ++i)
      delete m_vrenderprobe[i];

   for (int i = 0; i < m_vcollection.size(); i++)
      m_vcollection.ElementAt(i)->Release();

   m_pcv->CleanUpScriptEngine();
   m_pcv->Release();
   m_pcv = nullptr;

   m_psgt->Release();
   m_psgt = nullptr;

   if (m_hbmOffScreen)
      DeleteObject(m_hbmOffScreen);
}

void PinTable::FVerifySaveToClose()
{
   if (!m_vAsyncHandles.empty())
   {
      /*const DWORD wait =*/ WaitForMultipleObjects((DWORD)m_vAsyncHandles.size(), m_vAsyncHandles.data(), TRUE, INFINITE);
      //m_vpinball->MessageBox("Async work items not done", nullptr, 0);

      // Close the remaining handles here, since the window messages will never be processed
      for (size_t i = 0; i < m_vAsyncHandles.size(); i++)
         CloseHandle(m_vAsyncHandles[i]);

      m_vpinball->SetActionCur(string());
   }
}

void PinTable::DeleteFromLayer(IEditable *obj)
{
   for (int i = 0; i < MAX_LAYERS; i++)
   {
      if (FindIndexOf(m_layer[i], obj) != -1)
      {
         RemoveFromVectorSingle(m_layer[i], obj);
         break;
      }
   }
}

void PinTable::UpdatePropertyImageList()
{ 
    // just update the combo boxes in the property dialog
    g_pvp->GetPropertiesDocker()->GetContainProperties()->GetPropertyDialog()->UpdateTabs(m_vmultisel);
}

void PinTable::UpdatePropertyMaterialList()
{
    // just update the combo boxes in the property dialog
    g_pvp->GetPropertiesDocker()->GetContainProperties()->GetPropertyDialog()->UpdateTabs(m_vmultisel);
}

void PinTable::ClearForOverwrite()
{
   for (size_t i = 0; i < m_materials.size(); ++i)
      delete m_materials[i];
   m_materials.clear();

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
      delete m_vrenderprobe[i];
   m_vrenderprobe.clear();
}

void PinTable::InitBuiltinTable(const size_t tableId)
{
   string path;
   // Get our new table resource, get it to be opened as a storage, and open it like a normal file
   switch (tableId)
   {
   case ID_NEW_EXAMPLETABLE: path = "exampleTable.vpx"s; break;
   case ID_NEW_STRIPPEDTABLE: path = "strippedTable.vpx"s; break;
   case ID_NEW_LIGHTSEQTABLE: path = "lightSeqTable.vpx"s; break;
   case ID_NEW_BLANKTABLE:
   default: path = "blankTable.vpx"s;
   }
   m_glassTopHeight = m_glassBottomHeight = 210;
   for (int i = 0; i < 16; i++)
      m_rgcolorcustom[i] = RGB(0, 0, 0);
   LoadGameFromFilename(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + path);
   const LocalString ls(IDS_TABLE);
   m_szTitle = ls.m_szbuffer/*"Table"*/ + std::to_string(m_vpinball->m_NextTableID);
   m_vpinball->m_NextTableID++;
   m_settings.SetIniPath(string());
   m_szFileName.clear();
}

void PinTable::SetDefaultView()
{
   FRect frect;
   GetViewRect(&frect);
   m_offset = frect.Center();
   m_zoom = 0.5f;
}

void PinTable::SetCaption(const string& szCaption)
{
   if (m_mdiTable != nullptr && m_mdiTable->IsWindow())
      m_mdiTable->SetWindowText(szCaption.c_str());
   m_pcv->SetCaption(szCaption);
}

void PinTable::SetMouseCapture()
{
    SetCapture();
}

int PinTable::ShowMessageBox(const char *text) const
{
    return m_mdiTable->MessageBox(text, "Visual Pinball", MB_YESNO);
}

POINT PinTable::GetScreenPoint() const
{
    CPoint pt = GetCursorPos();
    ScreenToClient(pt);
    return pt;
}

#define CLEAN_MATERIAL(pEditMaterial) \
{robin_hood::unordered_map<string, Material*, StringHashFunctor, StringComparator>::const_iterator \
   it = m_materialMap.find(pEditMaterial); \
if (it == m_materialMap.end()) \
   pEditMaterial.clear();}

#define CLEAN_IMAGE(pEditImage) \
{robin_hood::unordered_map<string, Texture*, StringHashFunctor, StringComparator>::const_iterator \
   it = m_textureMap.find(pEditImage); \
if (it == m_textureMap.end()) \
   pEditImage.clear();}

#define CLEAN_SURFACE(pEditSurface) \
{if (!pEditSurface.empty()) \
{ \
bool found = false; \
for (size_t ie = 0; ie < m_vedit.size(); ie++) \
{ \
    IEditable* const item = m_vedit[ie]; \
    if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp) \
    { \
        CComBSTR bstr; \
        item->GetScriptable()->get_Name(&bstr); \
        if (!WzSzStrCmp(bstr, pEditSurface.c_str())) \
        { \
            found = true; \
            break; \
        } \
    } \
} \
if(!found) \
    pEditSurface.clear(); \
}}


void PinTable::InitTablePostLoad()
{
   PLOGI << "InitTablePostLoad"; // For profiling

   for (unsigned int i = 1; i < NUM_BG_SETS; ++i)
      if (mViewSetups[i].mFOV == FLT_MAX) // old table, copy FS and/or FSS settings over from old DT setting
      {
         mViewSetups[i] = mViewSetups[BG_DESKTOP];
         if (m_BG_image[i].empty() && i == BG_FSS) // copy image over for FSS mode
            m_BG_image[i] = m_BG_image[BG_DESKTOP];
      }

   m_globalDifficulty = m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, m_difficulty);

   m_currentBackglassMode = m_BG_current_set;
   if (m_BG_enable_FSS)
      m_currentBackglassMode = BG_FSS;

   m_hbmOffScreen = nullptr;
   m_dirtyDraw = true;

   m_left = 0.f;
   m_top = 0.f;

   SetDefaultView();

   m_pcv->AddItem(this, false);
   m_pcv->AddItem(m_psgt, true);
   m_pcv->AddItem(m_pcv->m_pdm, false);

   RemoveInvalidReferences();
}

// cleanup old bugs, i.e. currently buggy/non-existing material, image & surface names
// (also does the same for the <None> entries of droplists)
void PinTable::RemoveInvalidReferences()
{
   // set up the texture & material hashtables for faster access
   m_textureMap.clear();
   for (size_t i = 0; i < m_vimage.size(); i++)
       m_textureMap[m_vimage[i]->m_szName] = m_vimage[i];
   m_materialMap.clear();
   for (size_t i = 0; i < m_materials.size(); i++)
       m_materialMap[m_materials[i]->m_szName] = m_materials[i];

   for (size_t i = 0; i < m_vedit.size(); i++)
   {
        IEditable* const pEdit = m_vedit[i];
        if (pEdit == nullptr)
            continue;

        switch (pEdit->GetItemType())
        {
        case eItemPrimitive:
        {
            CLEAN_MATERIAL(((Primitive*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Primitive*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Primitive*)pEdit)->m_d.m_szImage);
            CLEAN_IMAGE(((Primitive*)pEdit)->m_d.m_szNormalMap);
            break;
        }
        case eItemRamp:
        {
            CLEAN_MATERIAL(((Ramp*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Ramp*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Ramp*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemSurface:
        {
            //CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szSideMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szTopMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szSlingShotMaterial);
            CLEAN_IMAGE(((Surface*)pEdit)->m_d.m_szImage);
            CLEAN_IMAGE(((Surface*)pEdit)->m_d.m_szSideImage);
            break;
        }
        case eItemDecal:
        {
            CLEAN_MATERIAL(((Decal*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Decal*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Decal*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Decal*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemFlipper:
        {
            CLEAN_MATERIAL(((Flipper*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Flipper*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_MATERIAL(((Flipper*)pEdit)->m_d.m_szRubberMaterial);
            CLEAN_IMAGE(((Flipper*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Flipper*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemHitTarget:
        {
            CLEAN_MATERIAL(((HitTarget*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((HitTarget*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((HitTarget*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemPlunger:
        {
            CLEAN_MATERIAL(((Plunger*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Plunger*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Plunger*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Plunger*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemSpinner:
        {
            CLEAN_MATERIAL(((Spinner*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Spinner*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Spinner*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Spinner*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemRubber:
        {
            CLEAN_MATERIAL(((Rubber*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Rubber*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Rubber*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemBumper:
        {
            //CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szCapMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szBaseMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szSkirtMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szRingMaterial);
            CLEAN_SURFACE(((Bumper*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemKicker:
        {
            CLEAN_MATERIAL(((Kicker*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Kicker*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_SURFACE(((Kicker*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemTrigger:
        {
            CLEAN_MATERIAL(((Trigger*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Trigger*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_SURFACE(((Trigger*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemDispReel:
        {
            CLEAN_IMAGE(((DispReel*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemFlasher:
        {
            CLEAN_IMAGE(((Flasher*)pEdit)->m_d.m_szImageA);
            CLEAN_IMAGE(((Flasher*)pEdit)->m_d.m_szImageB);
            break;
        }
        case eItemLight:
        {
            CLEAN_IMAGE(((Light*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Light*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemGate:
        {
            CLEAN_SURFACE(((Gate*)pEdit)->m_d.m_szSurface);
            break;
        }
        default:
        {
            break;
        }
        }
   }

   m_textureMap.clear();
   m_materialMap.clear();
}

bool PinTable::IsNameUnique(const WCHAR * const wzName) const
{
   return m_pcv->m_vcvd.GetSortedIndex(wzName) == -1;
}

void PinTable::GetUniqueName(const ItemTypeEnum type, WCHAR * const wzUniqueName, const DWORD wzUniqueName_maxlength) const
{
   WCHAR wzRoot[256] = { 0 };
   GetTypeNameForType(type, wzRoot);
   GetUniqueName(wzRoot, wzUniqueName, wzUniqueName_maxlength);
}

void PinTable::GetUniqueName(const WCHAR *const wzRoot, WCHAR * const wzUniqueName, const DWORD wzUniqueName_maxlength) const
{
   int suffix = 1;
   bool found = false;
   WCHAR * const wzName = new WCHAR[wzUniqueName_maxlength];

   while (!found)
   {
      WideStrNCopy(wzRoot, wzName, wzUniqueName_maxlength-3);
      WCHAR wzSuffix[4] = { 0 };
      _itow_s(suffix, wzSuffix, sizeof(wzSuffix)/sizeof(wzSuffix[0]), 10);
      if(suffix < 10)
         WideStrCat(L"0", wzName, wzUniqueName_maxlength);
      if(suffix < 100)
         WideStrCat(L"0", wzName, wzUniqueName_maxlength);
      WideStrCat(wzSuffix, wzName, wzUniqueName_maxlength);

      if (IsNameUnique(wzName) || suffix == 999)
         found = true;
      else
         suffix++;
   }

   WideStrNCopy(wzName, wzUniqueName, wzUniqueName_maxlength);
   delete[] wzName;
}

void PinTable::GetUniqueNamePasting(const int type, WCHAR * const wzUniqueName, const DWORD wzUniqueName_maxlength)
{
   //if the original name is not yet used, use that one (so there's nothing we have to do) 
   //otherwise add/increase the suffix until we find a name that's not used yet
   if (!IsNameUnique(wzUniqueName))
   {
      //first remove the existing suffix
      while (iswdigit(wzUniqueName[wcslen(wzUniqueName) - 1]))
      {
         wzUniqueName[wcslen(wzUniqueName) - 1] = L'\0';
      }

      GetUniqueName(wzUniqueName, wzUniqueName, wzUniqueName_maxlength);
   }
}

void PinTable::UIRenderPass2(Sur * const psur)
{
   const CRect rc = GetClientRect();
   psur->SetFillColor(m_vpinball->m_backgroundColor);
   psur->SetBorderColor(-1, false, 0);

   FRect frect;
   GetViewRect(&frect);

   psur->Rectangle2(rc.left, rc.top, rc.right, rc.bottom);

   if (m_backdrop)
   {
      Texture * const ppi = GetImage((!m_vpinball->m_backglassView) ? m_image : m_BG_image[m_BG_current_set]);

      if (ppi)
      {
         ppi->CreateGDIVersion();
         if (ppi->m_hbmGDIVersion)
         {
            CDC dc;
            dc.CreateCompatibleDC(nullptr);
            const CBitmap hbmOld = dc.SelectObject(ppi->m_hbmGDIVersion);

            psur->Image(frect.left, frect.top, frect.right, frect.bottom, dc.GetHDC(), ppi->m_width, ppi->m_height);

            dc.SelectObject(hbmOld);
         }
      }
   }

   if (m_vpinball->m_backglassView)
   {
      Render3DProjection(psur);
   }

   for(const auto &ptr : m_vedit)
   {
      if (ptr->m_backglass == m_vpinball->m_backglassView && ptr->GetISelect()->m_isVisible)
        ptr->UIRenderPass1(psur);
   }

   if (m_grid && m_vpinball->m_gridSize > 0)
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      const float gridsize = (float)m_vpinball->m_gridSize;

      const int beginx = (int)(rlt.x / gridsize);
      const int lenx = (int)((rrb.x - rlt.x) / gridsize);//(((rc.right - rc.left)/m_zoom));
      const int beginy = (int)(rlt.y / gridsize);
      const int leny = (int)((rrb.y - rlt.y) / gridsize);//(((rc.bottom - rc.top)/m_zoom));

      psur->SetObject(nullptr); // Don't hit test gridlines

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

   for(const auto &ptr : m_vedit)
   {
      if (ptr->m_backglass == m_vpinball->m_backglassView && ptr->GetISelect()->m_isVisible)
        ptr->UIRenderPass2(psur);
   }

   if (m_vpinball->m_backglassView) // Outline of the view, for when the grid is off
   {
      psur->SetObject(nullptr);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), false, 1);
      psur->Rectangle(0, 0, EDITOR_BG_WIDTH, EDITOR_BG_HEIGHT);
   }

   if (m_dragging)
   {
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), true, 0);
      psur->Rectangle(m_rcDragRect.left, m_rcDragRect.top, m_rcDragRect.right, m_rcDragRect.bottom);
   }

   // display the layer string
   //    psur->SetObject(nullptr);
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
   if (m_vedit.empty())
      return;

   // dummy coordinate system for backdrop view
   ModelViewProj mvp;
   if (mViewSetups[m_currentBackglassMode].mMode == VLM_WINDOW)
      mViewSetups[m_currentBackglassMode].SetWindowModeFromSettings(this);
   mViewSetups[m_currentBackglassMode].ComputeMVP(this, (float)EDITOR_BG_WIDTH / (float)EDITOR_BG_HEIGHT, false, mvp);

   Vertex3Ds rgvIn[8];
   rgvIn[0].x = m_left;  rgvIn[0].y = m_top;    rgvIn[0].z = 50.0f;
   rgvIn[1].x = m_left;  rgvIn[1].y = m_top;    rgvIn[1].z = m_glassTopHeight;
   rgvIn[2].x = m_right; rgvIn[2].y = m_top;    rgvIn[2].z = m_glassTopHeight;
   rgvIn[3].x = m_right; rgvIn[3].y = m_top;    rgvIn[3].z = 50.0f;
   rgvIn[4].x = m_right; rgvIn[4].y = m_bottom; rgvIn[4].z = 50.0f;
   rgvIn[5].x = m_right; rgvIn[5].y = m_bottom; rgvIn[5].z = m_glassBottomHeight;
   rgvIn[6].x = m_left;  rgvIn[6].y = m_bottom; rgvIn[6].z = m_glassBottomHeight;
   rgvIn[7].x = m_left;  rgvIn[7].y = m_bottom; rgvIn[7].z = 50.0f;

   Vertex2D rgvOut[8];
   RECT viewport;
   viewport.left = 0;
   viewport.top = 0;
   viewport.right = EDITOR_BG_WIDTH;
   viewport.bottom = EDITOR_BG_HEIGHT;
   mvp.GetModelViewProj(0).TransformVertices(rgvIn, nullptr, 8, rgvOut, viewport);

   psur->SetFillColor(RGB(200, 200, 200));
   psur->SetBorderColor(-1, false, 0);
   psur->Polygon(rgvOut, 8);
}


bool PinTable::GetDecalsEnabled() const
{
   return m_renderDecals;
}


bool PinTable::GetEMReelsEnabled() const
{
   return m_renderEMReels;
}

// draws the main design screen
void PinTable::Paint(HDC hdc)
{
   const CRect rc = GetClientRect();

   if (m_dirtyDraw)
   {
      if (m_hbmOffScreen)
      {
         DeleteObject(m_hbmOffScreen);
      }
      m_hbmOffScreen = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
   }

   CDC dc;
   dc.CreateCompatibleDC(hdc);

   const CBitmap hbmOld = dc.SelectObject(m_hbmOffScreen);

   if (m_dirtyDraw)
   {
      Sur * const psur = new PaintSur(dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, GetSelectedItem());
      UIRenderPass2(psur);

      delete psur;
   }

   BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, dc.GetHDC(), 0, 0, SRCCOPY);

   dc.SelectObject(hbmOld);

   m_dirtyDraw = false;
}

ISelect *PinTable::HitTest(const int x, const int y)
{
   const CDC dc;

   const CRect rc = GetClientRect();

   HitSur phs (dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, x, y, this);
   HitSur phs2(dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, x, y, this);

   m_allHitElements.clear();

   UIRenderPass2(&phs);

   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const ptr = m_vedit[i];
      if (ptr->m_backglass == m_vpinball->m_backglassView)
      {
         ptr->UIRenderPass1(&phs2);
         ISelect* const tmp = phs2.m_pselected;
         if (FindIndexOf(m_allHitElements, tmp) == -1 && tmp != nullptr && tmp != this)
         {
            m_allHitElements.push_back(tmp);
         }
      }
   }
   // it's possible that UIRenderPass1 doesn't find all elements (gates,plunger)
   // check here if everything was already stored in the list
   if (FindIndexOf(m_allHitElements, phs.m_pselected) == -1)
   {
      m_allHitElements.push_back(phs.m_pselected);
   }

   std::reverse(m_allHitElements.begin(), m_allHitElements.end());

   return phs.m_pselected;
}

void PinTable::SetDirtyDraw()
{
   if(g_pplayer)
       return;

   m_dirtyDraw = true;
   InvalidateRect(false);
}

void PinTable::HandleLoadFailure()
{
   m_progressDialog.Destroy();
   g_pvp->m_table_played_via_SelectTableOnStart = false;
}

void PinTable::Play(const int playMode)
{
   // Creates Player instance with a shallow duplicate of this table
   if (g_pplayer)
      return; // Can't play twice

   PLOGI << "Starting Play mode [table: " << m_szTableName << ", play mode: " << playMode << ']';
   m_vpinball->ShowSubDialog(m_progressDialog, !g_pvp->m_open_minimized);

   m_progressDialog.SetProgress(1);
   m_progressDialog.SetName("Creating Live Table..."s);

   PinTable *src = this;
   CComObject<PinTable> *live_table;
   CComObject<PinTable>::CreateInstance(&live_table);
   live_table->AddRef();
   live_table->m_isLiveInstance = true;

   CComObject<PinTable> *dst = live_table;
   const size_t cchar = SendMessage(src->m_pcv->m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   char *const szText = new char[cchar + 1];
   SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);
   SendMessage(dst->m_pcv->m_hwndScintilla, SCI_SETTEXT, 0, (size_t)szText);
   delete[] szText;

   dst->m_settings = src->m_settings;

   dst->m_szTitle = src->m_szTitle;
   dst->m_szFileName = src->m_szFileName;
   dst->m_szTableName = src->m_szTableName;
   dst->m_left = src->m_left;
   dst->m_top = src->m_top;
   dst->m_right = src->m_right;
   dst->m_bottom = src->m_bottom;
   dst->m_overridePhysics = src->m_overridePhysics;
   dst->m_overridePhysicsFlipper = src->m_overridePhysicsFlipper;
   dst->m_Gravity = src->m_Gravity;
   dst->m_friction = src->m_friction;
   dst->m_elasticity = src->m_elasticity;
   dst->m_elasticityFalloff = src->m_elasticityFalloff;
   dst->m_scatter = src->m_scatter;
   dst->m_defaultScatter = src->m_defaultScatter;
   dst->m_nudgeTime = src->m_nudgeTime;
   dst->m_plungerNormalize = src->m_plungerNormalize;
   dst->m_plungerFilter = src->m_plungerFilter;
   dst->m_PhysicsMaxLoops = src->m_PhysicsMaxLoops;
   dst->m_renderEMReels = src->m_renderEMReels;
   dst->m_renderDecals = src->m_renderDecals;
   dst->m_offset = src->m_offset;
   dst->m_zoom = src->m_zoom;
   dst->m_angletiltMax = src->m_angletiltMax;
   dst->m_angletiltMin = src->m_angletiltMin;
   dst->m_3DZPD = src->m_3DZPD;
   dst->m_3DOffset = src->m_3DOffset;
   dst->m_overwriteGlobalStereo3D = src->m_overwriteGlobalStereo3D;
   dst->m_image = src->m_image;
   dst->m_ImageBackdropNightDay = src->m_ImageBackdropNightDay;
   dst->m_imageColorGrade = src->m_imageColorGrade;
   dst->m_ballImage = src->m_ballImage;
   dst->m_ballSphericalMapping = src->m_ballSphericalMapping;
   dst->m_ballImageDecal = src->m_ballImageDecal;
   dst->m_envImage = src->m_envImage;
   dst->m_notesText = src->m_notesText;
   dst->m_szScreenShot = src->m_szScreenShot;
   dst->m_backdrop = src->m_backdrop;
   dst->m_glassBottomHeight = src->m_glassBottomHeight;
   dst->m_glassTopHeight = src->m_glassTopHeight;
   dst->m_playfieldMaterial = src->m_playfieldMaterial;
   dst->m_colorbackdrop = src->m_colorbackdrop;
   dst->m_difficulty = src->m_difficulty;
   dst->m_globalDifficulty = m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, dst->m_difficulty);
   dst->m_lightAmbient = src->m_lightAmbient;
   dst->m_lightHeight = src->m_lightHeight;
   dst->m_lightRange = src->m_lightRange;
   dst->m_lightEmissionScale = src->m_lightEmissionScale;
   dst->m_envEmissionScale = src->m_envEmissionScale;
   dst->m_globalEmissionScale = src->m_globalEmissionScale;
   dst->m_AOScale = src->m_AOScale;
   dst->m_SSRScale = src->m_SSRScale;
   dst->m_TableSoundVolume = src->m_TableSoundVolume;
   dst->m_TableMusicVolume = src->m_TableMusicVolume;
   dst->m_playfieldReflectionStrength = src->m_playfieldReflectionStrength;
   dst->m_BallDecalMode = src->m_BallDecalMode;
   dst->m_ballPlayfieldReflectionStrength = src->m_ballPlayfieldReflectionStrength;
   dst->m_defaultBulbIntensityScaleOnBall = src->m_defaultBulbIntensityScaleOnBall;
   dst->m_grid = src->m_grid;
   dst->m_enableAO = src->m_enableAO;
   dst->m_enableSSR = src->m_enableSSR;
   dst->m_toneMapper = src->m_toneMapper;
   dst->m_bloom_strength = src->m_bloom_strength;
   memcpy(dst->m_wzName, src->m_wzName, MAXNAMEBUFFER * sizeof(src->m_wzName[0]));

   dst->m_Light[0].emission = src->m_Light[0].emission;

   dst->m_BG_enable_FSS = src->m_BG_enable_FSS;
   dst->m_BG_override = src->m_BG_override;
   dst->m_BG_current_set = src->m_BG_current_set;
   dst->UpdateCurrentBGSet();
   dst->m_currentBackglassMode = src->m_currentBackglassMode;
   dst->m_3DmaxSeparation = src->m_3DmaxSeparation;
   for (int i = 0; i < 3; i++)
   {
      dst->mViewSetups[i] = src->mViewSetups[i];
      dst->m_BG_image[i] = src->m_BG_image[i];
      dst->mViewSetups[i].ApplyTableOverrideSettings(m_settings, (ViewSetupID) i);
   }
   for (size_t i = 0; i < src->m_materials.size(); i++)
   {
      Material *mat = new Material(src->m_materials[i]);
      dst->m_materials.push_back(mat);
      dst->m_startupToLive[src->m_materials[i]] = mat;
      dst->m_liveToStartup[mat] = src->m_materials[i];
   }

   // Don't perform deep copy for these parts that can't be modified by the script
   for (size_t i = 0; i < src->m_vimage.size(); i++)
      dst->m_vimage.push_back(src->m_vimage[i]);
   for (size_t i = 0; i < src->m_vsound.size(); i++)
      dst->m_vsound.push_back(src->m_vsound[i]);
   for (size_t i = 0; i < src->m_vfont.size(); i++)
      dst->m_vfont.push_back(src->m_vfont[i]);

   PLOGI << "Duplicating parts for live instance"; // For profiling
   for (size_t i = 0; i < src->m_vedit.size(); i++)
   {
      IEditable *edit_dst = nullptr;
      switch (src->m_vedit[i]->GetItemType())
      {
      case eItemBumper:    edit_dst = ((Bumper *)   src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemDecal:     edit_dst = ((Decal *)    src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemDispReel:  edit_dst = ((DispReel *) src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemFlasher:   edit_dst = ((Flasher *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemFlipper:   edit_dst = ((Flipper *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemGate:      edit_dst = ((Gate *)     src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemHitTarget: edit_dst = ((HitTarget *)src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemKicker:    edit_dst = ((Kicker *)   src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemLight:     edit_dst = ((Light *)    src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemLightSeq:  edit_dst = ((LightSeq *) src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemPlunger:   edit_dst = ((Plunger *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemPrimitive: edit_dst = ((Primitive *)src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemRamp:      edit_dst = ((Ramp *)     src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemRubber:    edit_dst = ((Rubber *)   src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemSpinner:   edit_dst = ((Spinner *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemSurface:   edit_dst = ((Surface *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemTextbox:   edit_dst = ((Textbox *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemTimer:     edit_dst = ((Timer *)    src->m_vedit[i])->CopyForPlay(live_table); break;
      case eItemTrigger:   edit_dst = ((Trigger *)  src->m_vedit[i])->CopyForPlay(live_table); break;
      default: assert(false); // Unexpected table part
      }
      live_table->m_vedit.push_back(edit_dst);
      dst->m_startupToLive[src->m_vedit[i]] = edit_dst;
      dst->m_liveToStartup[edit_dst] = src->m_vedit[i];
   }

   PLOGI << "Duplicating collections"; // For profiling
   for (int i = 0; i < m_vcollection.size(); i++)
   {
      CComObject<Collection> *pcol;
      CComObject<Collection>::CreateInstance(&pcol);
      pcol->AddRef();
      memcpy(pcol->m_wzName, m_vcollection[i].m_wzName, MAXNAMEBUFFER * sizeof(pcol->m_wzName[0]));
      pcol->m_fireEvents = m_vcollection[i].m_fireEvents;
      pcol->m_stopSingleEvents = m_vcollection[i].m_stopSingleEvents;
      pcol->m_groupElements = m_vcollection[i].m_groupElements;
      for (int j = 0; j < m_vcollection[i].m_visel.size(); ++j)
      {
         if (dst->m_startupToLive.find(m_vcollection[i].m_visel[j].GetIEditable()) != dst->m_startupToLive.end())
         {
            auto edit_item = (IEditable*) dst->m_startupToLive[m_vcollection[i].m_visel[j].GetIEditable()];
            edit_item->m_vCollection.push_back(pcol);
            edit_item->m_viCollection.push_back(pcol->m_visel.size());
            pcol->m_visel.push_back(edit_item->GetISelect());
         }
      }
      live_table->m_vcollection.push_back(pcol);
      live_table->m_pcv->AddItem(pcol, false);
   }

   live_table->m_pcv->AddItem(live_table, false);
   live_table->m_pcv->AddItem(live_table->m_psgt, true);
   live_table->m_pcv->AddItem(live_table->m_pcv->m_pdm, false);

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
   {
      RenderProbe *rp = new RenderProbe();
      rp->SetRoughness(m_vrenderprobe[i]->GetRoughness());
      rp->SetType(m_vrenderprobe[i]->GetType());
      rp->SetName(m_vrenderprobe[i]->GetName());
      rp->SetReflectionMode(m_vrenderprobe[i]->GetReflectionMode());
      vec4 plane;
      m_vrenderprobe[i]->GetReflectionPlane(plane);
      rp->SetReflectionPlane(plane);
      rp->SetReflectionNoLightmaps(m_vrenderprobe[i]->GetReflectionNoLightmaps());
      live_table->m_vrenderprobe.push_back(rp);
   }
      
   mixer_get_volume();

   EndAutoSaveCounter();

   // get the load path from the table filename
   const string szLoadDir = PathFromFilename(m_szFileName);
   // make sure the load directory is the active directory
   SetCurrentDirectory(szLoadDir.c_str());

   PLOGI << "Compiling script"; // For profiling

   live_table->m_pcv->m_scriptError = false;
   live_table->m_pcv->Compile(false);

   if (!live_table->m_pcv->m_scriptError)
   {
      // set up the texture & material hashtables for faster access
      live_table->m_textureMap.clear();
      for (size_t i = 0; i < live_table->m_vimage.size(); i++)
         live_table->m_textureMap[live_table->m_vimage[i]->m_szName] = live_table->m_vimage[i];
      live_table->m_materialMap.clear();
      for (size_t i = 0; i < live_table->m_materials.size(); i++)
         live_table->m_materialMap[live_table->m_materials[i]->m_szName] = live_table->m_materials[i];
      live_table->m_lightMap.clear();
      for (size_t i = 0; i < live_table->m_vedit.size(); i++)
      {
         IEditable *const pe = live_table->m_vedit[i];
         if (pe->GetItemType() == ItemTypeEnum::eItemLight)
            live_table->m_lightMap[pe->GetName()] = (Light *)pe;
      }
      live_table->m_renderprobeMap.clear();
      for (size_t i = 0; i < live_table->m_vrenderprobe.size(); i++)
         live_table->m_renderprobeMap[live_table->m_vrenderprobe[i]->GetName()] = live_table->m_vrenderprobe[i];

      // parse the (optional) override-physics-sets that can be set globally
      float fOverrideContactScatterAngle;
      live_table->m_fOverrideGravityConstant = GRAVITYCONST * m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsGravityConstant" + std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_GRAVITY);
      live_table->m_fOverrideContactFriction = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsContactFriction"+std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_CONTACTFRICTION);
      live_table->m_fOverrideElasticity = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsElasticity" + std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_ELASTICITY);
      live_table->m_fOverrideElasticityFalloff = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsElasticityFalloff"+std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_ELASTICITY_FALLOFF);
      live_table->m_fOverrideScatterAngle = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsScatterAngle" + std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_PFSCATTERANGLE);
      live_table->m_fOverrideMinSlope = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsMinSlope" + std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_MIN_SLOPE);
      live_table->m_fOverrideMaxSlope = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsMaxSlope" + std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_MAX_SLOPE);
      fOverrideContactScatterAngle = m_settings.LoadValueWithDefault(Settings::Player, "TablePhysicsContactScatterAngle"+std::to_string(live_table->m_overridePhysics - 1), DEFAULT_TABLE_SCATTERANGLE);
      c_hardScatter = ANGTORAD(live_table->m_overridePhysics ? fOverrideContactScatterAngle : live_table->m_defaultScatter);

      // create Player and init that one

      PLOGI << "Creating player"; // For profiling
      g_pplayer = new Player(this, live_table, playMode);
      g_pplayer->CreateWnd();
      const float minSlope = (live_table->m_overridePhysics ? live_table->m_fOverrideMinSlope : live_table->m_angletiltMin);
      const float maxSlope = (live_table->m_overridePhysics ? live_table->m_fOverrideMaxSlope : live_table->m_angletiltMax);
      const float slope = minSlope + (maxSlope - minSlope) * live_table->m_globalDifficulty;
      g_pplayer->SetGravity(slope, live_table->m_overridePhysics ? live_table->m_fOverrideGravityConstant : live_table->m_Gravity);

      m_vpinball->ToggleToolbar();
   }
   else
   {
      live_table->Release();
      HandleLoadFailure();
   }
}

// Called on the live table when playing ends (normally or when startup failed)
void PinTable::StopPlaying()
{
   assert(g_pplayer == nullptr || g_pplayer->m_ptable == this);
   assert(m_isLiveInstance);

   if (m_pcv)
      m_pcv->CleanUpScriptEngine();

   // Stop all sounds
   // In case we were playing any of the main buffers
   for (size_t i = 0; i < m_vsound.size(); i++)
      m_vsound[i]->Stop();
   // The usual case - copied sounds
   m_vpinball->m_ps.StopAndClearCopiedWavs();

   m_textureMap.clear();
   m_materialMap.clear();
   m_lightMap.clear();
   m_renderprobeMap.clear();

   PLOGI << "Ending Play mode [table: " << m_szTableName << ']';
}

HRESULT PinTable::InitVBA()
{
   return S_OK;
}

void PinTable::CloseVBA()
{
}

HRESULT PinTable::TableSave()
{
   return Save(m_szFileName.empty());
}

HRESULT PinTable::SaveAs()
{
   return Save(true);
}

HRESULT PinTable::ApcProject_Save()
{
   return Save(m_szFileName.empty());
}

void PinTable::BeginAutoSaveCounter()
{
   if (m_vpinball->m_autosaveTime > 0)
       m_vpinball->SetTimer(VPinball::TIMER_ID_AUTOSAVE, m_vpinball->m_autosaveTime, nullptr);
}

void PinTable::EndAutoSaveCounter()
{
   m_vpinball->KillTimer(VPinball::TIMER_ID_AUTOSAVE);
}

void PinTable::AutoSave()
{
   if (m_sdsCurrentDirtyState <= eSaveAutosaved)
      return;

   m_vpinball->KillTimer(VPinball::TIMER_ID_AUTOSAVE);

   {
      const LocalString ls(IDS_AUTOSAVING);
      m_vpinball->SetActionCur(ls.m_szbuffer);
      m_vpinball->SetCursorCur(nullptr, IDC_WAIT);
   }

   FastIStorage * const pstgroot = new FastIStorage();
   pstgroot->AddRef();

   const HRESULT hr = SaveToStorage(pstgroot);

   m_undo.SetCleanPoint((SaveDirtyState)min((int)m_sdsDirtyProp, (int)eSaveAutosaved));
   m_pcv->SetClean((SaveDirtyState)min((int)m_sdsDirtyScript, (int)eSaveAutosaved));
   SetNonUndoableDirty((SaveDirtyState)min((int)m_sdsNonUndoableDirty, (int)eSaveAutosaved));

   AutoSavePackage * const pasp = new AutoSavePackage();
   pasp->pstg = pstgroot;
   pasp->tableindex = FindIndexOf(m_vpinball->m_vtable, (CComObject<PinTable> *)this);
   pasp->hwndtable = GetHwnd();

   if (hr == S_OK)
   {
      const HANDLE hEvent = m_vpinball->PostWorkToWorkerThread(COMPLETE_AUTOSAVE, (LPARAM)pasp);
      m_vAsyncHandles.push_back(hEvent);

      m_vpinball->SetActionCur("Completing AutoSave");
   }
   else
   {
      m_vpinball->SetActionCur(string());
   }

   m_vpinball->SetCursorCur(nullptr, IDC_ARROW);
}

HRESULT PinTable::Save(const bool saveAs)
{
   IStorage* pstgRoot;

   // Get file name if needed
   if (saveAs)
   {
      //need to get a file name
      OPENFILENAME ofn = {};
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = m_vpinball->theInstance;
      ofn.hwndOwner = m_vpinball->GetHwnd();
      // TEXT
      ofn.lpstrFilter = "Visual Pinball Tables (*.vpx)\0*.vpx\0";

      char fileName[MAXSTRING];
      strncpy_s(fileName, m_szFileName.c_str(), sizeof(fileName)-1);
      char* const ptr = StrStrI(fileName, ".vpt");
      if (ptr != nullptr)
          strcpy_s(ptr, 5, ".vpx");
      ofn.lpstrFile = fileName;
      ofn.nMaxFile = sizeof(fileName);
      ofn.lpstrDefExt = "vpx";
      ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

      {
      string szInitialDir;
      // First, use dir of current table
      const size_t index = m_szFileName.find_last_of(PATH_SEPARATOR_CHAR);
      if (index != string::npos)
         szInitialDir = m_szFileName.substr(0, index);
      // Or try with the standard last-used dir
      else
      {
         if (!m_settings.LoadValue(Settings::RecentDir, "LoadDir"s, szInitialDir))
            szInitialDir = m_vpinball->m_szMyPath + "tables" + PATH_SEPARATOR_CHAR;
      }
      ofn.lpstrInitialDir = szInitialDir.c_str();

      const int ret = GetSaveFileName(&ofn);
      // user cancelled
      if (ret == 0)
         return S_FALSE;
      }

      m_szFileName = fileName;

      char szInitialDir[MAXSTRING];
      strncpy_s(szInitialDir, m_szFileName.c_str(), sizeof(szInitialDir)-1);
      szInitialDir[ofn.nFileOffset] = '\0'; // truncate after folder
      g_pvp->m_settings.SaveValue(Settings::RecentDir, "LoadDir"s, szInitialDir);

      {
         MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName.c_str());

         STGOPTIONS stg;
         stg.usVersion = 1;
         stg.reserved = 0;
         stg.ulSectorSize = 4096;

         HRESULT hr;
         if (FAILED(hr = StgCreateStorageEx(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
            STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgRoot)))
         {
            const LocalString ls(IDS_SAVEERROR);
            m_mdiTable->MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
            return hr;
         }
      }

      m_szTitle = TitleFromFilename(m_szFileName);
      SetCaption(m_szTitle);
   }
   else
   {
      char * const ptr = StrStrI(m_szFileName.c_str(), ".vpt");
      if (ptr != nullptr)
         strcpy_s(ptr, 5, ".vpx");
      MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName.c_str());

      STGOPTIONS stg;
      stg.usVersion = 1;
      stg.reserved = 0;
      stg.ulSectorSize = 4096;

      HRESULT hr;
      if (FAILED(hr = StgCreateStorageEx(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
         STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgRoot)))
      {
         const LocalString ls(IDS_SAVEERROR);
         m_mdiTable->MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
         return hr;
      }
   }

   {
      const LocalString ls(IDS_SAVING);
      m_vpinball->SetActionCur(ls.m_szbuffer);
      m_vpinball->SetCursorCur(nullptr, IDC_WAIT);
   }

   RemoveInvalidReferences();

   const HRESULT hr = SaveToStorage(pstgRoot);

   if (SUCCEEDED(hr))
   {
      pstgRoot->Commit(STGC_DEFAULT);
      pstgRoot->Release();

      m_vpinball->SetActionCur(string());
      m_vpinball->SetCursorCur(nullptr, IDC_ARROW);

      m_undo.SetCleanPoint(eSaveClean);
      m_pcv->SetClean(eSaveClean);
      SetNonUndoableDirty(eSaveClean);
   }

   // Save user custom settings file (if any) along the table file
   const string szINIFilename = GetSettingsFileName();
   if (!szINIFilename.empty())
   {
      // Force saving as we may have upgraded the table version (from pre 10.8 to 10.8) or changed the file path
      m_settings.SetModified(true);
      m_settings.SaveToFile(szINIFilename);
   }

   return S_OK;
}

HRESULT PinTable::SaveToStorage(IStorage *pstgRoot)
{
   m_savingActive = true;
   RECT rc;
   ::SendMessage(m_vpinball->m_hwndStatusBar, SB_GETRECT, 2, (size_t)&rc);

   HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR)nullptr,
      WS_CHILD | WS_VISIBLE, rc.left,
      rc.top,
      rc.right - rc.left, rc.bottom - rc.top,
      m_vpinball->m_hwndStatusBar, (HMENU)0, m_vpinball->theInstance, nullptr);

   ::SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

   //////////////// Begin Encryption
   HCRYPTPROV hcp = NULL;
   HCRYPTHASH hch = NULL;
   HCRYPTKEY  hkey = NULL;
   HCRYPTHASH hchkey = NULL;

   int foo;

   foo = CryptAcquireContext(&hcp, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

   //foo = CryptGenKey(hcp, CALG_RC2, CRYPT_EXPORTABLE, &hkey);

   foo = GetLastError();

   foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

   foo = GetLastError();

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

   const int ctotalitems = (int)(m_vedit.size() + m_vsound.size() + m_vimage.size() + m_vfont.size() + m_vcollection.size());
   int csaveditems = 0;

   ::SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

   //first save our own data
   IStorage* pstgData;
   HRESULT hr;
   if (SUCCEEDED(hr = pstgRoot->CreateStorage(L"GameStg", STGM_DIRECT/*STGM_TRANSACTED*/ | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgData)))
   {
      IStream *pstmGame;
      if (SUCCEEDED(hr = pstgData->CreateStream(L"GameData", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmGame)))
      {
         IStream *pstmItem;
         if (SUCCEEDED(hr = pstgData->CreateStream(L"Version", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
         {
            int version = CURRENT_FILE_FORMAT_VERSION;
            CryptHashData(hch, (BYTE *)&version, sizeof(version), 0);
            ULONG writ;
            pstmItem->Write(&version, sizeof(version), &writ);
            pstmItem->Release();
            pstmItem = nullptr;
         }

         IStorage *pstgInfo;
         if (SUCCEEDED(hr = pstgRoot->CreateStorage(L"TableInfo", STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgInfo)))
         {
            SaveInfo(pstgInfo, hch);

            if (SUCCEEDED(hr = pstgData->CreateStream(L"CustomInfoTags", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
            {
               SaveCustomInfo(pstgInfo, pstmItem, hch);
               pstmItem->Release();
               pstmItem = nullptr;
            }

            pstgInfo->Release();
         }

         if (SUCCEEDED(hr = SaveData(pstmGame, hch, false)))
         {
            for (size_t i = 0; i < m_vedit.size(); i++)
            {
               const string szStmName = "GameItem" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  ULONG writ;
                  IEditable *const piedit = m_vedit[i];
                  const ItemTypeEnum type = piedit->GetItemType();
                  pstmItem->Write(&type, sizeof(int), &writ);
                  hr = piedit->SaveData(pstmItem, NULL, false);
                  pstmItem->Release();
                  pstmItem = nullptr;
                  //if (FAILED(hr)) goto Error;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (size_t i = 0; i < m_vsound.size(); i++)
            {
               const string szStmName = "Sound" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  SaveSoundToStream(m_vsound[i], pstmItem);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (size_t i = 0; i < m_vimage.size(); i++)
            {
               const string szStmName = "Image" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vimage[i]->SaveToStream(pstmItem, this);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (size_t i = 0; i < m_vfont.size(); i++)
            {
               const string szStmName = "Font" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vfont[i]->SaveToStream(pstmItem);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i = 0; i < m_vcollection.size(); i++)
            {
               const string szStmName = "Collection" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               if (SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vcollection[i].SaveData(pstmItem, hch, false);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

         }
         pstmGame->Release();
      }

      BYTE hashval[256];
      DWORD hashlen = 256;

      // Authentication block
      foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

      hashlen = 256;
      foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

      IStream* pstmItem;
      if (SUCCEEDED(hr = pstgData->CreateStream(L"MAC", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
      {
         ULONG writ;
         //int version = CURRENT_FILE_FORMAT_VERSION;
         pstmItem->Write(hashval, hashlen, &writ);
         pstmItem->Release();
         pstmItem = nullptr;
         //if (FAILED(hr)) goto Error;

         //CryptExportKey(hkey, nullptr, PUBLICKEYBLOB, 0, BYTE *pbData, DWORD *pdwDataLen);
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
         const LocalString ls(IDS_SAVEERROR);
         m_mdiTable->MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
      }
      pstgData->Release();
   }

   //Error:

   DestroyWindow(hwndProgressBar);
   m_savingActive = false;

   return hr;
}

HRESULT PinTable::SaveSoundToStream(const PinSound * const pps, IStream *pstm)
{
   ULONG writ = 0;
   int len = (int)pps->m_szName.length();

   HRESULT hr;
   if (FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(pps->m_szName.c_str(), len, &writ)))
      return hr;

   len = (int)pps->m_szPath.length();

   if (FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = pstm->Write(pps->m_szPath.c_str(), len, &writ)))
      return hr;

   // removed: previously did write the same name again, but just in lower case
   //  this rudimentary version here needs to stay as otherwise problems when loading, as one field less
   len = 1;
   if (FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;
   constexpr char tmp = '\0'; // now just writes a short dummy/empty string
   if (FAILED(hr = pstm->Write(&tmp, len, &writ)))
      return hr;
   //

   if (pps->IsWav2()) // only use old code if playing wav's
   if (FAILED(hr = pstm->Write(&pps->m_wfx, sizeof(pps->m_wfx), &writ)))
      return hr;

#ifdef ONLY_USE_BASS
   if (FAILED(hr = pstm->Write(pps->IsWav2() ? &pps->m_cdata_org : &pps->m_cdata, sizeof(int), &writ)))
#else
   if (FAILED(hr = pstm->Write(&pps->m_cdata, sizeof(int), &writ)))
#endif
      return hr;

#ifdef ONLY_USE_BASS
   if (FAILED(hr = pstm->Write(pps->IsWav2() ? pps->m_pdata_org : pps->m_pdata, pps->m_cdata, &writ)))
#else
   if (FAILED(hr = pstm->Write(pps->m_pdata, pps->m_cdata, &writ)))
#endif
      return hr;

   SoundOutTypes outputTarget = pps->GetOutputTarget();
   if (FAILED(hr = pstm->Write(&outputTarget, sizeof(bool), &writ)))
      return hr;

   // Begin NEW_SOUND_VERSION data

   if (FAILED(hr = pstm->Write(&pps->m_volume, sizeof(int), &writ)))
      return hr;
   if (FAILED(hr = pstm->Write(&pps->m_balance, sizeof(int), &writ)))
      return hr;
   if (FAILED(hr = pstm->Write(&pps->m_fade, sizeof(int), &writ)))
      return hr;
   if (FAILED(hr = pstm->Write(&pps->m_volume, sizeof(int), &writ)))
      return hr;

   return S_OK;
}


HRESULT PinTable::LoadSoundFromStream(IStream *pstm, const int LoadFileVersion)
{
   int len;
   ULONG read;
   HRESULT hr;

   if (FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
      return hr;

   PinSound * const pps = new PinSound();
   char* tmp = new char[len+1];
   if (FAILED(hr = pstm->Read(tmp, len, &read)))
   {
       delete pps;
       return hr;
   }
   tmp[len] = '\0';
   pps->m_szName = tmp;
   delete[] tmp;

   if (FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
   {
       delete pps;
       return hr;
   }

   tmp = new char[len+1];
   if (FAILED(hr = pstm->Read(tmp, len, &read)))
   {
       delete pps;
       return hr;
   }
   tmp[len] = '\0';
   pps->m_szPath = tmp;
   delete[] tmp;

   // deprecated lower case name, but not used anymore nowadays, so 10.8+ stores only 1,'\0'
   if (FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
   {
       delete pps;
       return hr;
   }

   tmp = new char[len];
   if (FAILED(hr = pstm->Read(tmp, len, &read)))
   {
       delete[] tmp;
       delete pps;
       return hr;
   }
   delete[] tmp;
   //

   if (pps->IsWav2()) // only use old code if playing wav's
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

#ifdef ONLY_USE_BASS
   // reconvert to make it look like an original wave file to BASS
   DWORD waveFileSize;
   char *waveFilePointer;
   if (pps->IsWav2())
   {
	   struct WAVEHEADER
	   {
		   DWORD   dwRiff;    // "RIFF"
		   DWORD   dwSize;    // Size
		   DWORD   dwWave;    // "WAVE"
		   DWORD   dwFmt;     // "fmt "
		   DWORD   dwFmtSize; // Wave Format Size
	   };
	   //  Static RIFF header
	   static constexpr BYTE WaveHeader[] =
	   {
		   'R','I','F','F',0x00,0x00,0x00,0x00,'W','A','V','E','f','m','t',' ',0x00,0x00,0x00,0x00
	   };
	   // Static wave DATA tag
	   static constexpr BYTE WaveData[] = { 'd','a','t','a' };

	   waveFileSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize + sizeof(WaveData) + sizeof(DWORD) + pps->m_cdata;
	   pps->m_pdata = new char[waveFileSize];
	   waveFilePointer = pps->m_pdata;
	   WAVEHEADER * const waveHeader = reinterpret_cast<WAVEHEADER *>(pps->m_pdata);

	   // Wave header
	   memcpy(waveFilePointer, WaveHeader, sizeof(WaveHeader));
	   waveFilePointer += sizeof(WaveHeader);

	   // Update sizes in wave header
	   waveHeader->dwSize = waveFileSize - sizeof(DWORD) * 2;
	   waveHeader->dwFmtSize = sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize;

	   // WAVEFORMATEX
	   memcpy(waveFilePointer, &pps->m_wfx, sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize);
	   waveFilePointer += sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize;

	   // Data header
	   memcpy(waveFilePointer, WaveData, sizeof(WaveData));
	   waveFilePointer += sizeof(WaveData);
	   *(reinterpret_cast<DWORD *>(waveFilePointer)) = pps->m_cdata;
	   waveFilePointer += sizeof(DWORD);
   }
   else
#endif
	   pps->m_pdata = new char[pps->m_cdata];

   //LZWReader lzwreader(pstm, (int *)pps->m_pdata, pps->m_cdata, 1, pps->m_cdata); // TODO could compress wav data
   //lzwreader.Decoder();

#ifdef ONLY_USE_BASS
   if (FAILED(hr = pstm->Read(pps->IsWav2() ? waveFilePointer : pps->m_pdata, pps->m_cdata, &read)))
#else
   if (FAILED(hr = pstm->Read(pps->m_pdata, pps->m_cdata, &read)))
#endif
   {
      delete pps;
      return hr;
   }
#ifdef ONLY_USE_BASS
   if (pps->IsWav2())
   {
      pps->m_pdata_org = waveFilePointer;
      pps->m_cdata_org = pps->m_cdata;
      pps->m_cdata = waveFileSize;
   }
#endif

   if (LoadFileVersion >= NEW_SOUND_FORMAT_VERSION)
   {
      SoundOutTypes outputTarget;
	   if (FAILED(hr = pstm->Read(&outputTarget, sizeof(char), &read)))
	   {
		   delete pps;
		   return hr;
	   }
      pps->SetOutputTarget(outputTarget);
	   if (FAILED(hr = pstm->Read(&pps->m_volume, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_balance, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_fade, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
	   if (FAILED(hr = pstm->Read(&pps->m_volume, sizeof(int), &read)))
	   {
		   delete pps;
		   return hr;
	   }
   }
   else
   {
	   bool toBackglassOutput = false; // false: for pre-VPX tables
	   if (FAILED(hr = pstm->Read(&toBackglassOutput, sizeof(bool), &read)))
	   {
		   delete pps;
		   return hr;
	   }

	   pps->SetOutputTarget((StrStrI(pps->m_szName.c_str(), "bgout_") != nullptr)
                        || (lstrcmpi(pps->m_szPath.c_str(), "* Backglass Output *") == 0) // legacy behavior, where the BG selection was encoded into the strings directly
	                     || toBackglassOutput ? SNDOUT_BACKGLASS : SNDOUT_TABLE);
   }

   if (FAILED(hr = pps->ReInitialize()))
   {
      delete pps;
      return hr;
   }

   // search for duplicate names, do not load dupes
   for(size_t i = 0; i < m_vsound.size(); ++i)
      if (m_vsound[i]->m_szName == pps->m_szName && m_vsound[i]->m_szPath == pps->m_szPath)
      {
         delete pps;
         return S_FAIL;
      }

   m_vsound.push_back(pps);
   return S_OK;
}


HRESULT PinTable::WriteInfoValue(IStorage* pstg, const WCHAR * const wzName, const string& szValue, HCRYPTHASH hcrypthash)
{
   HRESULT hr = S_OK;
   IStream *pstm;

   if (!szValue.empty() && SUCCEEDED(hr = pstg->CreateStream(wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
   {
      ULONG writ;
      BiffWriter bw(pstm, hcrypthash);

      const int len = (int)szValue.length();
      WCHAR * const wzT = new WCHAR[len + 1];
      MultiByteToWideCharNull(CP_ACP, 0, szValue.c_str(), -1, wzT, len + 1);

      bw.WriteBytes(wzT, len*(int)sizeof(WCHAR), &writ);
      delete[] wzT;
      pstm->Release();
      pstm = nullptr;
   }

   return hr;
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
   time_t hour_machine;
   time(&hour_machine);
   tm local_hour;
   localtime_s(&local_hour, &hour_machine);
   char buffer[256];
   asctime_s(buffer, &local_hour);
   buffer[strnlen_s(buffer,sizeof(buffer))-1] = '\0'; // remove line break
   WriteInfoValue(pstg, L"TableSaveDate", buffer, NULL);
   _itoa_s(++m_numTimesSaved, buffer, 10);
   WriteInfoValue(pstg, L"TableSaveRev", buffer, NULL);

   Texture * const pin = GetImage(m_szScreenShot);
   if (pin != nullptr && pin->m_ppb != nullptr)
   {
      IStream *pstm;
      HRESULT hr;

      if (SUCCEEDED(hr = pstg->CreateStream(L"Screenshot", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
      {
         BiffWriter bw(pstm, hcrypthash);
         ULONG writ;
         bw.WriteBytes(pin->m_ppb->m_pdata, pin->m_ppb->m_cdata, &writ);
         pstm->Release();
         pstm = nullptr;
      }
   }

   pstg->Commit(STGC_DEFAULT);

   return S_OK;
}


HRESULT PinTable::SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash)
{
   BiffWriter bw(pstmTags, hcrypthash);

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
      bw.WriteString(FID(CUST), m_vCustomInfoTag[i]);

   bw.WriteTag(FID(ENDB));

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
   {
      const int len = (int)m_vCustomInfoTag[i].length();
      WCHAR * const wzName = new WCHAR[len + 1];
      MultiByteToWideCharNull(CP_ACP, 0, m_vCustomInfoTag[i].c_str(), -1, wzName, len + 1);

      WriteInfoValue(pstg, wzName, m_vCustomInfoContent[i], hcrypthash);

      delete[] wzName;
   }

   pstg->Commit(STGC_DEFAULT);

   return S_OK;
}


HRESULT PinTable::ReadInfoValue(IStorage* pstg, const WCHAR * const wzName, string &output, HCRYPTHASH hcrypthash)
{
   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(wzName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);

      const int len = ss.cbSize.LowPart / (DWORD)sizeof(WCHAR);
      WCHAR * const wzT = new WCHAR[len + 1];
      memset(wzT, 0, sizeof(WCHAR) * (len + 1));
      char* pszValue = new char[len + 1];

      ULONG read;
      BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
      br.ReadBytes(wzT, ss.cbSize.LowPart, &read);
      wzT[len] = L'\0';

      WideCharToMultiByteNull(CP_ACP, 0, wzT, -1, pszValue, len + 1, nullptr, nullptr);

      //delete br;
      //pstm->Read(pszValue, ss.cbSize.LowPart, &read);

      output = pszValue;
      delete[] pszValue;

      delete[] wzT;
      pstm->Release();
   }

   return hr;
}


HRESULT PinTable::LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version)
{
   ReadInfoValue(pstg, L"TableName", m_szTableName, hcrypthash);
   ReadInfoValue(pstg, L"AuthorName", m_szAuthor, hcrypthash);
   ReadInfoValue(pstg, L"TableVersion", m_szVersion, hcrypthash);
   ReadInfoValue(pstg, L"ReleaseDate", m_szReleaseDate, hcrypthash);
   ReadInfoValue(pstg, L"AuthorEmail", m_szAuthorEMail, hcrypthash);
   ReadInfoValue(pstg, L"AuthorWebSite", m_szWebSite, hcrypthash);
   ReadInfoValue(pstg, L"TableBlurb", m_szBlurb, hcrypthash);
   ReadInfoValue(pstg, L"TableDescription", m_szDescription, hcrypthash);
   ReadInfoValue(pstg, L"TableRules", m_szRules, hcrypthash);
   ReadInfoValue(pstg, L"TableSaveDate", m_szDateSaved, NULL);

   string numTimesSaved;
   ReadInfoValue(pstg, L"TableSaveRev", numTimesSaved, NULL);
   m_numTimesSaved = !numTimesSaved.empty() ? atoi(numTimesSaved.c_str()) : 0;

   // Write the version to the registry.  This will be read later by the front end.
   g_pvp->m_settings.SaveValue(Settings::Version, m_szTableName, m_szVersion);

   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(L"Screenshot", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);
      m_pbTempScreenshot = new PinBinary();

      m_pbTempScreenshot->m_cdata = ss.cbSize.LowPart;
      m_pbTempScreenshot->m_pdata = new char[m_pbTempScreenshot->m_cdata];

      //m_pbTempScreenshot->LoadFromStream(pstm, version);

      ULONG read;
      BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
      br.ReadBytes(m_pbTempScreenshot->m_pdata, m_pbTempScreenshot->m_cdata, &read);

      pstm->Release();
   }

   return hr;
}

HRESULT PinTable::LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version)
{
   BiffReader br(pstmTags, this, NULL, version, hcrypthash, NULL);
   const HRESULT hr = br.Load();

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
   {
      const int len = (int)m_vCustomInfoTag[i].length();
      WCHAR * const wzName = new WCHAR[len + 1];
      MultiByteToWideCharNull(CP_ACP, 0, m_vCustomInfoTag[i].c_str(), -1, wzName, len + 1);

      string customInfo;
      ReadInfoValue(pstg, wzName, customInfo, hcrypthash);
      m_vCustomInfoContent.push_back(customInfo);

      delete[] wzName;
   }

   return hr;
}

HRESULT PinTable::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteFloat(FID(LEFT), m_left);
   bw.WriteFloat(FID(TOPX), m_top);
   bw.WriteFloat(FID(RGHT), m_right);
   bw.WriteFloat(FID(BOTM), m_bottom);

   bw.WriteBool(FID(EFSS), m_BG_enable_FSS);
   static constexpr int vsFields[NUM_BG_SETS][19] = { 
      { FID(VSM0), FID(ROTA), FID(INCL), FID(LAYB), FID(FOVX), FID(XLTX), FID(XLTY), FID(XLTZ), FID(SCLX), FID(SCLY), FID(SCLZ), FID(HOF0), FID(VOF0), FID(WTX0), FID(WTY0), FID(WTZ0), FID(WBX0), FID(WBY0), FID(WBZ0) },
      { FID(VSM1), FID(ROTF), FID(INCF), FID(LAYF), FID(FOVF), FID(XLFX), FID(XLFY), FID(XLFZ), FID(SCFX), FID(SCFY), FID(SCFZ), FID(HOF1), FID(VOF1), FID(WTX1), FID(WTY1), FID(WTZ1), FID(WBX1), FID(WBY1), FID(WBZ1) },
      { FID(VSM2), FID(ROFS), FID(INFS), FID(LAFS), FID(FOFS), FID(XLXS), FID(XLYS), FID(XLZS), FID(SCXS), FID(SCYS), FID(SCZS), FID(HOF2), FID(VOF2), FID(WTX2), FID(WTY2), FID(WTZ2), FID(WBX2), FID(WBY2), FID(WBZ2) },
   };
   for (int i = 0; i < 3; i++)
   {
      bw.WriteInt(vsFields[i][0], mViewSetups[i].mMode);
      bw.WriteFloat(vsFields[i][1], mViewSetups[i].mViewportRotation);
      bw.WriteFloat(vsFields[i][2], mViewSetups[i].mLookAt);
      bw.WriteFloat(vsFields[i][3], mViewSetups[i].mLayback);
      bw.WriteFloat(vsFields[i][4], mViewSetups[i].mFOV);
      bw.WriteFloat(vsFields[i][5], mViewSetups[i].mViewX);
      bw.WriteFloat(vsFields[i][6], mViewSetups[i].mViewY);
      bw.WriteFloat(vsFields[i][7], mViewSetups[i].mViewZ);
      bw.WriteFloat(vsFields[i][8], mViewSetups[i].mSceneScaleX);
      bw.WriteFloat(vsFields[i][9], mViewSetups[i].mSceneScaleY);
      bw.WriteFloat(vsFields[i][10], mViewSetups[i].mSceneScaleZ);
      bw.WriteFloat(vsFields[i][11], mViewSetups[i].mViewHOfs);
      bw.WriteFloat(vsFields[i][12], mViewSetups[i].mViewVOfs);
      bw.WriteFloat(vsFields[i][15], mViewSetups[i].mWindowTopZOfs);
      bw.WriteFloat(vsFields[i][18], mViewSetups[i].mWindowBottomZOfs);
   }

   bw.WriteInt(FID(ORRP), m_overridePhysics);
   bw.WriteBool(FID(ORPF), m_overridePhysicsFlipper);
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

   bw.WriteBool(FID(REEL), m_renderEMReels);
   bw.WriteBool(FID(DECL), m_renderDecals);

   bw.WriteFloat(FID(OFFX), m_offset.x);
   bw.WriteFloat(FID(OFFY), m_offset.y);

   bw.WriteFloat(FID(ZOOM), m_zoom);

   bw.WriteFloat(FID(SLPX), m_angletiltMax);
   bw.WriteFloat(FID(SLOP), m_angletiltMin);

   bw.WriteFloat(FID(MAXSEP), m_3DmaxSeparation);
   bw.WriteFloat(FID(ZPD), m_3DZPD);
   bw.WriteFloat(FID(STO), m_3DOffset);
   bw.WriteBool(FID(OGST), m_overwriteGlobalStereo3D);

   bw.WriteString(FID(IMAG), m_image);
   bw.WriteString(FID(BIMG), m_BG_image[0]);
   bw.WriteString(FID(BIMF), m_BG_image[1]);
   bw.WriteString(FID(BIMS), m_BG_image[2]);
   bw.WriteBool(FID(BIMN), m_ImageBackdropNightDay);
   bw.WriteString(FID(IMCG), m_imageColorGrade);
   bw.WriteString(FID(BLIM), m_ballImage);
   bw.WriteBool(FID(BLSM), m_ballSphericalMapping);
   bw.WriteString(FID(BLIF), m_ballImageDecal);
   bw.WriteString(FID(EIMG), m_envImage);
   bw.WriteString(FID(NOTX), m_notesText);

   bw.WriteString(FID(SSHT), m_szScreenShot);

   bw.WriteBool(FID(FBCK), m_backdrop);

   bw.WriteFloat(FID(GLAS), m_glassTopHeight);
   bw.WriteFloat(FID(GLAB), m_glassBottomHeight);

   bw.WriteString(FID(PLMA), m_playfieldMaterial);
   bw.WriteInt(FID(BCLR), m_colorbackdrop);

   bw.WriteFloat(FID(TDFT), m_difficulty);

   bw.WriteInt(FID(LZAM), m_lightAmbient);
   bw.WriteInt(FID(LZDI), m_Light[0].emission);
   bw.WriteFloat(FID(LZHI), m_lightHeight);
   bw.WriteFloat(FID(LZRA), m_lightRange);
   bw.WriteFloat(FID(LIES), m_lightEmissionScale);
   bw.WriteFloat(FID(ENES), m_envEmissionScale);
   bw.WriteFloat(FID(GLES), m_globalEmissionScale);
   bw.WriteFloat(FID(AOSC), m_AOScale);
   bw.WriteFloat(FID(SSSC), m_SSRScale);

   bw.WriteFloat(FID(SVOL), m_TableSoundVolume);
   bw.WriteFloat(FID(MVOL), m_TableMusicVolume);

   bw.WriteInt(FID(PLST), quantizeUnsigned<8>(m_playfieldReflectionStrength));
   bw.WriteBool(FID(BDMO), m_BallDecalMode);
   bw.WriteFloat(FID(BPRS), m_ballPlayfieldReflectionStrength);
   bw.WriteFloat(FID(DBIS), m_defaultBulbIntensityScaleOnBall);
   bw.WriteBool(FID(GDAC), m_grid);

   bw.WriteInt(FID(UAOC), m_enableAO);
   bw.WriteInt(FID(USSR), m_enableSSR);
   bw.WriteInt(FID(TMAP), m_toneMapper);
   bw.WriteFloat(FID(BLST), m_bloom_strength);

   // Legacy material saving for backward compatibility
   bw.WriteInt(FID(MASI), (int)m_materials.size());
   if (!m_materials.empty())
   {
      vector<SaveMaterial> mats(m_materials.size());
      for (size_t i = 0; i < m_materials.size(); i++)
      {
         const Material* const m = m_materials[i];
         mats[i].cBase = m->m_cBase;
         mats[i].cGlossy = m->m_cGlossy;
         mats[i].cClearcoat = m->m_cClearcoat;
         mats[i].fWrapLighting = m->m_fWrapLighting;
         mats[i].fRoughness = m->m_fRoughness;
         mats[i].fGlossyImageLerp = 255 - quantizeUnsigned<8>(clamp(m->m_fGlossyImageLerp, 0.f, 1.f)); // '255 -' to be compatible with previous table versions
         mats[i].fThickness = quantizeUnsigned<8>(clamp(m->m_fThickness, 0.05f, 1.f)); // clamp with 0.05f to be compatible with previous table versions
         mats[i].fEdge = m->m_fEdge;
         mats[i].fOpacity = m->m_fOpacity;
         mats[i].bIsMetal = m->m_type == Material::MaterialType::METAL;
         mats[i].bOpacityActive_fEdgeAlpha = m->m_bOpacityActive ? 1 : 0;
         mats[i].bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(clamp(m->m_fEdgeAlpha, 0.f, 1.f)) << 1;
         strncpy_s(mats[i].szName, m->m_szName.c_str(), sizeof(mats[i].szName)-1);
         for (size_t c = strnlen_s(mats[i].szName, sizeof(mats[i].szName)); c < sizeof(mats[i].szName); ++c) // to avoid garbage after 0
             mats[i].szName[c] = 0;
      }
      bw.WriteStruct(FID(MATE), mats.data(), (int)(sizeof(SaveMaterial)*m_materials.size()));

      vector<SavePhysicsMaterial> phymats(m_materials.size());
      for (size_t i = 0; i < m_materials.size(); i++)
      {
          const Material* const m = m_materials[i];
          strncpy_s(phymats[i].szName, m->m_szName.c_str(), sizeof(phymats[i].szName)-1);
          for (size_t c = strnlen_s(phymats[i].szName, sizeof(phymats[i].szName)); c < sizeof(phymats[i].szName); ++c) // to avoid garbage after 0
              phymats[i].szName[c] = 0;
          phymats[i].fElasticity = m->m_fElasticity;
          phymats[i].fElasticityFallOff = m->m_fElasticityFalloff;
          phymats[i].fFriction = m->m_fFriction;
          phymats[i].fScatterAngle = m->m_fScatterAngle;
      }
      bw.WriteStruct(FID(PHMA), phymats.data(), (int)(sizeof(SavePhysicsMaterial)*m_materials.size()));
   }
   // 10.8+ material saving (this format supports new properties, can be extended in future versions, and does not perform quantizations)
   for (size_t i = 0; i < m_materials.size(); i++)
   {
      const size_t record_size = m_materials[i]->GetSaveSize();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      CComPtr<IStream> spStream;
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
      m_materials[i]->SaveData(spStream, NULL, false);
      LPVOID pData = ::GlobalLock(hMem);
      bw.WriteStruct(FID(MATR), pData, record_size);
      ::GlobalUnlock(hMem);
   }

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
   {
      // Save each render probe as a data blob inside the main gamedata.
      // This allows backward compatibility since the block will be blindly discarded on older versions, still hashing it.
      const int record_size = m_vrenderprobe[i]->GetSaveSize();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      CComPtr<IStream> spStream;
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
      m_vrenderprobe[i]->SaveData(spStream, NULL, false);
      LPVOID pData = ::GlobalLock(hMem);
      bw.WriteStruct(FID(RPRB), pData, record_size);
      ::GlobalUnlock(hMem);
   }

   // HACK!!!! - Don't save special values when copying for undo.  For instance, don't reset the code.
   // Someday save these values into there own stream, used only when saving to file.

   if (hcrypthash != 0)
   {
      bw.WriteInt(FID(SEDT), (int)m_vedit.size());
      bw.WriteInt(FID(SSND), (int)m_vsound.size());
      bw.WriteInt(FID(SIMG), (int)m_vimage.size());
      bw.WriteInt(FID(SFNT), (int)m_vfont.size());
      bw.WriteInt(FID(SCOL), m_vcollection.size());

      bw.WriteWideString(FID(NAME), m_wzName);

      bw.WriteStruct(FID(CCUS), m_rgcolorcustom, sizeof(COLORREF) * 16);

      // save the script source code
      bw.WriteTag(FID(CODE));
      // if the script is protected then we pass in the proper cyptokey into the code savestream
      m_pcv->SaveToStream(pstm, hcrypthash);
   }

   bw.WriteInt(FID(TLCK), m_locked);
   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT PinTable::LoadGameFromFilename(const string& szFileName)
{
   if (szFileName.empty())
   {
      ShowError("Empty File Name String!");
      return S_FALSE;
   }

   PLOGI << "LoadGameFromFilename " + szFileName; // For profiling

   m_szFileName = szFileName;

   // Load user custom settings before actually loading the table for settings applying during load
   string szINIFilename = GetSettingsFileName();
   if (!szINIFilename.empty())
      m_settings.LoadFromFile(szINIFilename, false);

   MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName.c_str());
   HRESULT hr;
   IStorage* pstgRoot;
   if (FAILED(hr = StgOpenStorage(wszCodeFile, nullptr, STGM_TRANSACTED | STGM_READ, nullptr, 0, &pstgRoot)))
   {
      char msg[MAXSTRING+32];
      sprintf_s(msg, sizeof(msg), "Error 0x%X loading \"%s\"", hr, m_szFileName.c_str());
      m_vpinball->MessageBox(msg, "Load Error", 0);
      return hr;
   }

   RECT rc;
   ::SendMessage(m_vpinball->m_hwndStatusBar, SB_GETRECT, 2, (size_t)&rc);

   HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR)nullptr,
      WS_CHILD | WS_VISIBLE, rc.left,
      rc.top,
      rc.right - rc.left, rc.bottom - rc.top,
      m_vpinball->m_hwndStatusBar, (HMENU)0, m_vpinball->theInstance, nullptr);

   ::SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

   const LocalString ls(IDS_LOADING);
   m_vpinball->SetActionCur(ls.m_szbuffer);
   m_vpinball->SetCursorCur(nullptr, IDC_WAIT);

   HCRYPTPROV hcp = NULL;
   HCRYPTHASH hch = NULL;
   HCRYPTHASH hchkey = NULL;
   HCRYPTKEY  hkey = NULL;

   ///////// Begin MAC
   int foo;

   foo = CryptAcquireContext(&hcp, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

   foo = GetLastError();

   foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

   foo = GetLastError();

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
   IStorage* pstgData;
   if (SUCCEEDED(hr = pstgRoot->OpenStorage(L"GameStg", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgData)))
   {
      IStream *pstmGame;
      if (SUCCEEDED(hr = pstgData->OpenStream(L"GameData", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmGame)))
      {
         IStream* pstmVersion;
         if (SUCCEEDED(hr = pstgData->OpenStream(L"Version", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
         {
            ULONG read;
            hr = pstmVersion->Read(&loadfileversion, sizeof(int), &read);
            CryptHashData(hch, (BYTE *)&loadfileversion, sizeof(int), 0);
            pstmVersion->Release();
            if (loadfileversion > CURRENT_FILE_FORMAT_VERSION)
            {
               char errorMsg[MAX_PATH] = { 0 };
               sprintf_s(errorMsg, sizeof(errorMsg), "This table was saved with file version %i.%02i and is newer than the supported file version %i.%02i!\nYou might get problems loading/playing it, so please update to the latest VPX at https://github.com/vpinball/vpinball/releases!", loadfileversion / 100, loadfileversion % 100, CURRENT_FILE_FORMAT_VERSION / 100, CURRENT_FILE_FORMAT_VERSION % 100);
               ShowError(errorMsg);
               /*
                              pstgRoot->Release();
                              pstmGame->Release();
                              pstgData->Release();
                              DestroyWindow(hwndProgressBar);
                              m_vpinball->SetCursorCur(nullptr, IDC_ARROW);
                              return -1;
               */
            }

            // Create a block cipher session key based on the hash of the password.
            CryptDeriveKey(hcp, CALG_RC2, hchkey, (loadfileversion == 600) ? CRYPT_EXPORTABLE : (CRYPT_EXPORTABLE | 0x00280000), &hkey);
         }

         IStorage* pstgInfo;
         if (SUCCEEDED(hr = pstgRoot->OpenStorage(L"TableInfo", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgInfo)))
         {
            LoadInfo(pstgInfo, hch, loadfileversion);
            IStream* pstmItem;
            if (SUCCEEDED(hr = pstgData->OpenStream(L"CustomInfoTags", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
            {
               hr = LoadCustomInfo(pstgInfo, pstmItem, hch, loadfileversion);
               pstmItem->Release();
               pstmItem = nullptr;
            }
            pstgInfo->Release();
         }

         int csubobj = 0;
         int csounds = 0;
         int ctextures = 0;
         int cfonts = 0;
         int ccollection = 0;

         if (SUCCEEDED(hr = LoadData(pstmGame, csubobj, csounds, ctextures, cfonts, ccollection, loadfileversion, hch, (loadfileversion < NO_ENCRYPTION_FORMAT_VERSION) ? hkey : NULL)))
         {
            PLOGI << "LoadData loaded"; // For profiling

            const int ctotalitems = csubobj + csounds + ctextures + cfonts;
            int cloadeditems = 0;
            ::SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

            for (int i = 0; i < csubobj; i++)
            {
               const string szStmName = "GameItem" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  ULONG read;
                  ItemTypeEnum type;
                  pstmItem->Read(&type, sizeof(int), &read);

                  IEditable * const piedit = EditableRegistry::Create(type);

                  int id = 0; // VBA id for this item
                  hr = piedit->InitLoad(pstmItem, this, &id, loadfileversion, (loadfileversion < 1000) ? hch : NULL, (loadfileversion < 1000) ? hkey : NULL); // 1000 (VP10 beta) removed the encryption
                  piedit->InitVBA(fFalse, id, nullptr);
                  pstmItem->Release();
                  pstmItem = nullptr;
                  if (FAILED(hr)) break;

                  m_vedit.push_back(piedit);

                  //hr = piedit->InitPostLoad();
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            PLOGI << "GameItem loaded"; // For profiling

            for (int i = 0; i < csounds; i++)
            {
               const string szStmName = "Sound" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  LoadSoundFromStream(pstmItem, loadfileversion);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            PLOGI << "Sound loaded"; // For profiling

            assert(m_vimage.empty());
            m_vimage.resize(ctextures); // due to multithreaded loading do pre-allocation
            {
               ThreadPool pool(g_pvp->m_logicalNumberOfProcessors); //!! Note that this dramatically increases the amount of temporary memory needed, especially if Max Texture Dimension is set (as then all the additional conversion/rescale mem is also needed 'in parallel')

               for (int i = 0; i < ctextures; i++)
               {
                  pool.enqueue([i, loadfileversion, pstgData, this] {
                     const string szStmName = "Image" + std::to_string(i);
                     MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

                     IStream* pstmItem;
                     HRESULT hr;
                     if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
                     {
                        hr = LoadImageFromStream(pstmItem, i, loadfileversion, false);
                        if (FAILED(hr))
                           return hr;
                        pstmItem->Release();
                        pstmItem = nullptr;
                     }
                     return hr;
                  });
                  cloadeditems++;
               }
               pool.wait_until_nothing_in_flight();
            }
            // due to multithreaded loading and pre-allocation, check if some images could not be loaded, and perform a retry since more memory is available now
            string failed_load_img;
            for (size_t i = 0; i < m_vimage.size(); ++i)
                if (!m_vimage[i] || m_vimage[i]->m_pdsBuffer == nullptr)
                {
                    const string szStmName = "Image" + std::to_string(i);
                    MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

                    IStream* pstmItem;
                    if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
                    {
                        hr = LoadImageFromStream(pstmItem, i, loadfileversion, true);
                        if (SUCCEEDED(hr))
                        {
                            pstmItem->Release();
                            pstmItem = nullptr;
                        }
                    }

                    if (!m_vimage[i] || m_vimage[i]->m_pdsBuffer == nullptr)
                        failed_load_img += '\n' + (m_vimage[i] ? m_vimage[i]->m_szName : szStmName);
                    else if ((m_vimage[i]->m_realWidth > m_vimage[i]->m_width) || (m_vimage[i]->m_realHeight > m_vimage[i]->m_height)) { //!! do not warn on resize, as original image file/binary blob is always loaded into mem! (otherwise table load failure is triggered) {
                        PLOGW << "Image '" << m_vimage[i]->m_szName << "' was downsized from " << m_vimage[i]->m_realWidth << 'x' << m_vimage[i]->m_realHeight << " to " << m_vimage[i]->m_width << 'x' << m_vimage[i]->m_height << " due to low memory ";
                    }
                }

            if (!failed_load_img.empty())
            {
#ifdef _WIN64
               m_vpinball->MessageBox(("WARNING ! WARNING ! WARNING ! WARNING !\n\nNot all images were loaded for an unknown reason.\n\nDO NOT SAVE THIS FILE OR YOU MAY LOOSE DATA!\n\nAffected Files:\n" + failed_load_img).c_str(), "Load Error", 0);
#else
               m_vpinball->MessageBox(("WARNING ! WARNING ! WARNING ! WARNING !\n\nNot all images were loaded, likely due to low memory.\nPlease use the 64-bit version of the application.\n\nDO NOT SAVE THIS FILE OR YOU MAY LOOSE DATA!\n\nAffected Files:\n" + failed_load_img).c_str(), "Load Error", 0);
#endif
            }

            // check if some images could not be loaded and erase them
            for (size_t i = 0; i < m_vimage.size(); ++i)
                if (!m_vimage[i] || m_vimage[i]->m_pdsBuffer == nullptr)
                {
                    m_vimage.erase(m_vimage.begin()+i);
                    --i;
                }

            // search for duplicate names, delete dupes
            if (!m_vimage.empty())
               for (size_t i = 0; i < m_vimage.size() - 1; ++i)
                  for (size_t i2 = i+1; i2 < m_vimage.size(); ++i2)
                     if (m_vimage[i]->m_szName == m_vimage[i2]->m_szName && m_vimage[i]->m_szPath == m_vimage[i2]->m_szPath)
                     {
                        m_vimage.erase(m_vimage.begin()+i2);
                        --i2;
                     }

            PLOGI << "Image loaded"; // Profiling

            ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);

            for (int i = 0; i < cfonts; i++)
            {
               const string szStmName = "Font" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  PinFont * const ppf = new PinFont();
                  ppf->LoadFromStream(pstmItem, loadfileversion);
                  m_vfont.push_back(ppf);
                  ppf->Register();
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            PLOGI << "Font loaded"; // For profiling

            for (int i = 0; i < ccollection; i++)
            {
               const string szStmName = "Collection" + std::to_string(i);
               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName.c_str());

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wszStmName, nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  CComObject<Collection> *pcol;
                  CComObject<Collection>::CreateInstance(&pcol);
                  pcol->AddRef();
                  pcol->LoadData(pstmItem, this, loadfileversion, hch, (loadfileversion < NO_ENCRYPTION_FORMAT_VERSION) ? hkey : NULL);
                  m_vcollection.push_back(pcol);
                  m_pcv->AddItem((IScriptable *)pcol, false);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
               cloadeditems++;
               ::SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            PLOGI << "Collection loaded"; // For profiling

            for (size_t i = 0; i < m_vedit.size(); i++)
            {
               IEditable * const piedit = m_vedit[i];
               piedit->InitPostLoad();
            }

            PLOGI << "IEditable PostLoad performed"; // For profiling
         }
         pstmGame->Release();

         // Authentication block

         if (loadfileversion > 40)
         {
            if (SUCCEEDED(hr = pstgData->OpenStream(L"MAC", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
            {
               BYTE hashvalOld[256];
               //DWORD hashlenOld = 256;
               ULONG read;
               hr = pstmVersion->Read(&hashvalOld, HASHLENGTH, &read);

               BYTE hashval[256];
               DWORD hashlen = 256;
               foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

               hashlen = 256;
               foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

               foo = CryptDestroyHash(hch);

               foo = CryptDestroyHash(hchkey);

               foo = CryptDestroyKey(hkey);

               foo = CryptReleaseContext(hcp, 0);
               pstmVersion->Release();

               for (int i = 0; i < HASHLENGTH; i++)
                  if (hashval[i] != hashvalOld[i])
                  {
                     hr = APPX_E_BLOCK_HASH_INVALID;
                     break;
                  }
            }
            else
            {
               // Error
               hr = APPX_E_CORRUPT_CONTENT;
            }
         }

         if (loadfileversion < 1030) // the m_fGlossyImageLerp part was included first with 10.3, so set all previously saved materials to the old default
            for (size_t i = 0; i < m_materials.size(); ++i)
               m_materials[i]->m_fGlossyImageLerp = 1.f;

         if (loadfileversion < 1040) // the m_fThickness part was included first with 10.4, so set all previously saved materials to the old default
            for (size_t i = 0; i < m_materials.size(); ++i)
               m_materials[i]->m_fThickness = 0.05f;

         if (loadfileversion < 1072) // playfield meshes were always forced as collidable until 10.7.1
            for (size_t i = 0; i < m_vedit.size(); ++i)
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)m_vedit[i])->IsPlayfield()))
               {
                  Primitive* const prim = (Primitive *)m_vedit[i];
                  prim->put_IsToy(FTOVB(false));
                  prim->put_Collidable(FTOVB(true));
               }

         // reflections were hardcoded without render probe before 10.8.0
         RenderProbe *pf_reflection_probe = GetRenderProbe(PLAYFIELD_REFLECTION_RENDERPROBE_NAME);
         if (pf_reflection_probe == nullptr)
         {
            pf_reflection_probe = new RenderProbe();
            pf_reflection_probe->SetName(PLAYFIELD_REFLECTION_RENDERPROBE_NAME);
            pf_reflection_probe->SetReflectionMode(RenderProbe::ReflectionMode::REFL_DYNAMIC);
            m_vrenderprobe.push_back(pf_reflection_probe);
         }
         vec4 plane = vec4(0.f, 0.f, 1.f, 0.f);
         pf_reflection_probe->SetType(RenderProbe::PLANE_REFLECTION);
         pf_reflection_probe->SetReflectionPlane(plane);
         pf_reflection_probe->SetReflectionNoLightmaps(true);

         if (loadfileversion < 1080)
         {
            // Glass was horizontal before 10.8
            m_glassBottomHeight = m_glassTopHeight;

            for (size_t i = 0; i < m_vedit.size(); ++i)
            {
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)m_vedit[i])->m_d.m_disableLightingBelow != 1.0f))
               {
                  Primitive *const prim = (Primitive *)m_vedit[i];
                  // Before 10.8 alpha channel of texture was discarded if material transparency was 1, in turn leading to disabling lighting from below.
                  Material* mat = GetMaterial(prim->m_d.m_szMaterial);
                  if (mat && (!mat->m_bOpacityActive || mat->m_fOpacity == 1.0f))
                     prim->m_d.m_disableLightingBelow = 1.0f;
               }
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)m_vedit[i])->IsPlayfield()))
               {
                  Primitive* const prim = (Primitive *)m_vedit[i];
                  // playfield meshes were always processed as static until 10.8.0 (more precisely, directly rendered before everything else even in camera mode, then skipped when rendering all parts)
                  prim->m_d.m_staticRendering = true;
                  // since playfield were always rendered before bulb light buffer until 10.8, they would never have transmitted light
                  prim->m_d.m_disableLightingBelow = 1.0f;
                  // playfield meshes were always forced as visible until 10.8.0
                  prim->put_Visible(FTOVB(true));
                  // playfield meshes were always drawn before other transparent parts until 10.8.0
                  prim->m_d.m_depthBias = 100000.0f;
                  // playfield meshes did not handle backfaces until 10.8.0
                  prim->m_d.m_backfacesEnabled = false;
               }
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemLight)
               {
                  Light* const light = (Light *)m_vedit[i];
                  // Before 10.8, lights would never be reflected
                  light->m_d.m_reflectionEnabled = false;
                  // Before 10.8, lights did not have a z coordinate for the light emission point: classic lights where renderer at surface+0.1, bulb light at surface+halo height+0.1
                  // This needs to be preserved to avoid changing the light falloff curve, so we set up with the same definition (the 0.1 offset on z axis being applied when rendering to avoid z fighting)
                  light->m_d.m_height = light->m_d.m_BulbLight ? light->m_d.m_bulbHaloHeight : 0.0f;
                  if (!light->m_d.m_BulbLight)
                  {
                     // Before 10.8, classic light could not have a bulb mesh so force it off
                     light->m_d.m_showBulbMesh = false;
                     // Before 10.8, classic light could not have ball reflection so force it off
                     light->m_d.m_showReflectionOnBall = false;
                  }
                  // Before 10.8, bulb mesh visibility was combined with lightmap visibility (i.e. a hidden light could be reflecting but not have a bulb mesh). Note that light visible property was only accessible through script
                  if (!light->m_d.m_visible)
                     light->m_d.m_showBulbMesh = false;
               }
            }
         }

         // Do not consider properties converted to settings as changes to avoid creating an ini for each opened old table (they will be imported again as they are part of the VPX file)
         m_settings.SetModified(false);

         //////// End Authentication block
      }
      pstgData->Release();
   }

   if (m_pbTempScreenshot) // For some reason, no image picked up the screenshot.  Not good; but we'll dump it to make sure it gets cleaned up
   {
      delete m_pbTempScreenshot;
      m_pbTempScreenshot = nullptr;
   }

   DestroyWindow(hwndProgressBar);
   //DestroyWindow(hwndProgressDialog);

   pstgRoot->Release();

   m_vpinball->SetActionCur(string());

   m_vpinball->GetLayersListDialog()->ClearList();
   // copy all elements into their layers
   for (int i = 0; i < MAX_LAYERS; i++)
   {
      m_layer[i].clear();

      for (size_t t = 0; t < m_vedit.size(); t++)
      {
         IEditable * const piedit = m_vedit[t];
         ISelect * const psel = piedit->GetISelect();
         if (psel->m_oldLayerIndex == i)
         {
             m_layer[i].push_back(piedit);
             if (psel->m_layerName.empty())
                 psel->m_layerName = "Layer_" + std::to_string(i+1);
             m_vpinball->GetLayersListDialog()->AddLayer(psel->m_layerName, piedit);
         }
      }
   }

   return hr;
}

void PinTable::SetLoadDefaults()
{
   for (unsigned int i = 0; i < NUM_BG_SETS; ++i)
      m_BG_image[i].clear();
   m_imageColorGrade.clear();
   m_ballImage.clear();
   m_ballSphericalMapping = true;
   m_ballImageDecal.clear();
   m_ImageBackdropNightDay = false;
   m_envImage.clear();

   m_szScreenShot.clear();

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
   m_SSRScale = 0.5f;

   m_angletiltMax = 6.0f;
   m_angletiltMin = 4.5f;

   m_playfieldReflectionStrength = 0.2f;

   m_ballPlayfieldReflectionStrength = 1.f;

   m_enableAO = true;
   m_enableSSR = true;
   m_toneMapper = TM_REINHARD;

   m_bloom_strength = 1.0f;

   m_TableSoundVolume = 1.0f;
   m_TableMusicVolume = 1.0f;

   m_BallDecalMode = false;

   m_overridePhysicsFlipper = false;
}

HRESULT PinTable::LoadData(IStream* pstm, int& csubobj, int& csounds, int& ctextures, int& cfonts, int& ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetLoadDefaults();

   int rgi[6] = { 0, 0, 0, 0, 0, 0 };

   BiffReader br(pstm, this, rgi, version, hcrypthash, hcryptkey);
   const HRESULT hr = br.Load();

   csubobj = rgi[1];
   csounds = rgi[2];
   ctextures = rgi[3];
   cfonts = rgi[4];
   ccollection = rgi[5];

   return hr;
}

bool PinTable::LoadToken(const int id, BiffReader * const pbr)
{
   const string szINIFilename = GetSettingsFileName();
   const bool hasIni = !szINIFilename.empty() && FileExists(szINIFilename);
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(LEFT): pbr->GetFloat(m_left); break;
   case FID(TOPX): pbr->GetFloat(m_top); break;
   case FID(RGHT): pbr->GetFloat(m_right); break;
   case FID(BOTM): pbr->GetFloat(m_bottom); break;
   case FID(VSM0): pbr->GetInt(&mViewSetups[BG_DESKTOP].mMode); break;
   case FID(ROTA): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewportRotation); break;
   case FID(LAYB): pbr->GetFloat(mViewSetups[BG_DESKTOP].mLayback); break;
   case FID(INCL): pbr->GetFloat(mViewSetups[BG_DESKTOP].mLookAt); break;
   case FID(FOVX): pbr->GetFloat(mViewSetups[BG_DESKTOP].mFOV); break;
   case FID(SCLX): pbr->GetFloat(mViewSetups[BG_DESKTOP].mSceneScaleX); break;
   case FID(SCLY): pbr->GetFloat(mViewSetups[BG_DESKTOP].mSceneScaleY); break;
   case FID(SCLZ): pbr->GetFloat(mViewSetups[BG_DESKTOP].mSceneScaleZ); break;
   case FID(XLTX): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewX); break;
   case FID(XLTY): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewY); break;
   case FID(XLTZ): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewZ); break;
   case FID(HOF0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewHOfs); break;
   case FID(VOF0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewVOfs); break;
   case FID(WTZ0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mWindowTopZOfs); break;
   case FID(WBZ0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mWindowBottomZOfs); break;
   case FID(VSM1): pbr->GetInt(&mViewSetups[BG_FULLSCREEN].mMode); break;
   case FID(ROTF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewportRotation); break;
   case FID(LAYF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mLayback); break;
   case FID(INCF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mLookAt); break;
   case FID(FOVF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mFOV); break;
   case FID(SCFX): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mSceneScaleX); break;
   case FID(SCFY): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mSceneScaleY); break;
   case FID(SCFZ): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mSceneScaleZ); break;
   case FID(XLFX): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewX); break;
   case FID(XLFY): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewY); break;
   case FID(XLFZ): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewZ); break;
   case FID(HOF1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewHOfs); break;
   case FID(VOF1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewVOfs); break;
   case FID(WTZ1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mWindowTopZOfs); break;
   case FID(WBZ1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mWindowBottomZOfs); break;
   case FID(VSM2): pbr->GetInt(&mViewSetups[BG_FSS].mMode); break;
   case FID(ROFS): pbr->GetFloat(mViewSetups[BG_FSS].mViewportRotation); break;
   case FID(LAFS): pbr->GetFloat(mViewSetups[BG_FSS].mLayback); break;
   case FID(INFS): pbr->GetFloat(mViewSetups[BG_FSS].mLookAt); break;
   case FID(FOFS): pbr->GetFloat(mViewSetups[BG_FSS].mFOV); break;
   case FID(SCXS): pbr->GetFloat(mViewSetups[BG_FSS].mSceneScaleX); break;
   case FID(SCYS): pbr->GetFloat(mViewSetups[BG_FSS].mSceneScaleY); break;
   case FID(SCZS): pbr->GetFloat(mViewSetups[BG_FSS].mSceneScaleZ); break;
   case FID(XLXS): pbr->GetFloat(mViewSetups[BG_FSS].mViewX); break;
   case FID(XLYS): pbr->GetFloat(mViewSetups[BG_FSS].mViewY); break;
   case FID(XLZS): pbr->GetFloat(mViewSetups[BG_FSS].mViewZ); break;
   case FID(HOF2): pbr->GetFloat(mViewSetups[BG_FSS].mViewHOfs); break;
   case FID(VOF2): pbr->GetFloat(mViewSetups[BG_FSS].mViewVOfs); break;
   case FID(WTZ2): pbr->GetFloat(mViewSetups[BG_FSS].mWindowTopZOfs); break;
   case FID(WBZ2): pbr->GetFloat(mViewSetups[BG_FSS].mWindowBottomZOfs); break;
   case FID(EFSS): { pbr->GetBool(m_BG_enable_FSS); UpdateCurrentBGSet(); break; }
   //case FID(VERS): pbr->GetString(szVersion); break;
   case FID(ORRP): pbr->GetInt(m_overridePhysics); break;
   case FID(ORPF): pbr->GetBool(m_overridePhysicsFlipper); break;
   case FID(GAVT): pbr->GetFloat(m_Gravity); break;
   case FID(FRCT): pbr->GetFloat(m_friction); break;
   case FID(ELAS): pbr->GetFloat(m_elasticity); break;
   case FID(ELFA): pbr->GetFloat(m_elasticityFalloff); break;
   case FID(PFSC): pbr->GetFloat(m_scatter); break;
   case FID(SCAT): pbr->GetFloat(m_defaultScatter); break;
   case FID(NDGT): pbr->GetFloat(m_nudgeTime); break;
   case FID(MPGC):
   {
      int tmp;
      pbr->GetInt(tmp);
      m_plungerNormalize = m_settings.LoadValueWithDefault(Settings::Player, "PlungerNormalize"s, tmp);
      break;
   }
   case FID(MPDF):
   {
      bool tmp;
      pbr->GetBool(tmp);
      m_plungerFilter = m_settings.LoadValueWithDefault(Settings::Player, "PlungerFilter"s, tmp);
      break;
   }
   case FID(PHML):
   {
      pbr->GetInt(m_PhysicsMaxLoops);
      if (m_PhysicsMaxLoops == 0xFFFFFFFF)
         m_settings.LoadValue(Settings::Player, "PhysicsMaxLoops"s, m_PhysicsMaxLoops);
      break;
   }
   case FID(DECL): pbr->GetBool(m_renderDecals); break;
   case FID(REEL): pbr->GetBool(m_renderEMReels); break;
   case FID(OFFX): pbr->GetFloat(m_offset.x); break;
   case FID(OFFY): pbr->GetFloat(m_offset.y); break;
   case FID(ZOOM): pbr->GetFloat(m_zoom); break;
   case FID(MAXSEP): pbr->GetFloat(m_3DmaxSeparation); break;
   case FID(ZPD): pbr->GetFloat(m_3DZPD); break;
   case FID(STO): pbr->GetFloat(m_3DOffset); break;
   case FID(OGST): pbr->GetBool(m_overwriteGlobalStereo3D); break;
   case FID(SLPX): pbr->GetFloat(m_angletiltMax); break;
   case FID(SLOP): pbr->GetFloat(m_angletiltMin); break;
   case FID(GLAS): pbr->GetFloat(m_glassTopHeight); break;
   case FID(GLAB): pbr->GetFloat(m_glassBottomHeight); break;
   case FID(IMAG): pbr->GetString(m_image); break;
   case FID(BLIM): pbr->GetString(m_ballImage); break;
   case FID(BLSM): pbr->GetBool(m_ballSphericalMapping); break;
   case FID(BLIF): pbr->GetString(m_ballImageDecal); break;
   case FID(SSHT): pbr->GetString(m_szScreenShot); break;
   case FID(FBCK): pbr->GetBool(m_backdrop); break;
   case FID(SEDT): pbr->GetInt(&((int *)pbr->m_pdata)[1]); break;
   case FID(SSND): pbr->GetInt(&((int *)pbr->m_pdata)[2]); break;
   case FID(SIMG): pbr->GetInt(&((int *)pbr->m_pdata)[3]); break;
   case FID(SFNT): pbr->GetInt(&((int *)pbr->m_pdata)[4]); break;
   case FID(SCOL): pbr->GetInt(&((int *)pbr->m_pdata)[5]); break;
   case FID(NAME): pbr->GetWideString(m_wzName, sizeof(m_wzName) / sizeof(m_wzName[0])); break;
   case FID(BIMG): pbr->GetString(m_BG_image[0]); break;
   case FID(BIMF): pbr->GetString(m_BG_image[1]); break;
   case FID(BIMS): pbr->GetString(m_BG_image[2]); break;
   case FID(BIMN): pbr->GetBool(m_ImageBackdropNightDay); break;
   case FID(IMCG): pbr->GetString(m_imageColorGrade); break;
   case FID(EIMG): pbr->GetString(m_envImage); break;
   case FID(PLMA): pbr->GetString(m_playfieldMaterial); break;
   case FID(NOTX): {string txt;  pbr->GetString(txt); m_notesText = CString(txt.c_str()); break; }
   case FID(LZAM): pbr->GetInt(m_lightAmbient); break;
   case FID(LZDI): pbr->GetInt(m_Light[0].emission); break;
   case FID(LZHI): pbr->GetFloat(m_lightHeight); break;
   case FID(LZRA): pbr->GetFloat(m_lightRange); break;
   case FID(LIES): pbr->GetFloat(m_lightEmissionScale); break;
   case FID(ENES): pbr->GetFloat(m_envEmissionScale); break;
   case FID(GLES): pbr->GetFloat(m_globalEmissionScale); break;
   case FID(AOSC): pbr->GetFloat(m_AOScale); break;
   case FID(SSSC): pbr->GetFloat(m_SSRScale); break;
   // Removed in 10.8 since we now directly define reflection in render probe. Table author can disable default playfield reflection by setting PF reflection strength to 0. Player uses app/table settings to tweak
   //case FID(BREF): pbr->GetInt(m_useReflectionForBalls); break;
   case FID(PLST):
   {
      int tmp;
      pbr->GetInt(tmp);
      m_playfieldReflectionStrength = dequantizeUnsigned<8>(tmp);
      break;
   }
   case FID(BTRA):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int useTrailForBalls;
         pbr->GetInt(useTrailForBalls);
         if (useTrailForBalls != -1)
             m_settings.SaveValue(Settings::Player, "BallTrail"s, useTrailForBalls == 1);
      }
      break;
   case FID(BTST):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int ballTrailStrength;
         pbr->GetInt(ballTrailStrength);
         m_settings.SaveValue(Settings::Player, "BallTrailStrength"s, dequantizeUnsigned<8>(ballTrailStrength));
      }
      break;
   case FID(BPRS): pbr->GetFloat(m_ballPlayfieldReflectionStrength); break;
   case FID(DBIS): pbr->GetFloat(m_defaultBulbIntensityScaleOnBall); break;
   case FID(UAAL):
      if (hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int useAA;
         pbr->GetInt(useAA);
         if (useAA != -1)
             m_settings.SaveValue(Settings::Player, "AAFactor"s, useAA == 0 ? 1.f : 2.f);
      }
      break;
   case FID(UAOC):
      {
         // Before 10.8, this setting could be set to -1, meaning override table definition using video options instead
         int useAO;
         pbr->GetInt(useAO);
         m_enableAO = useAO != 0;
      }
      break;
   case FID(USSR):
      {
         // Before 10.8, this setting could be set to -1, meaning override table definition using video options instead
         int useSSR;
         pbr->GetInt(useSSR);
         m_enableSSR = useSSR != 0;
      }
      break;
   case FID(TMAP): pbr->GetInt(&m_toneMapper); break;
   case FID(UFXA):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int fxaa;
         pbr->GetInt(fxaa);
         if (fxaa != -1)
            m_settings.SaveValue(Settings::Player, "FXAA"s, fxaa);
      }
      break;
   case FID(BLST): pbr->GetFloat(m_bloom_strength); break;
   case FID(BCLR): pbr->GetInt(m_colorbackdrop); break;
   case FID(SECB): pbr->GetStruct(&m_protectionData, sizeof(ProtectionData)); break;
   case FID(CODE):
   {
      // if the script is protected then we pass in the proper cryptokey into the code loadstream
      const bool script_protected = (((m_protectionData.flags & DISABLE_EVERYTHING) == DISABLE_EVERYTHING) ||
          ((m_protectionData.flags & DISABLE_SCRIPT_EDITING) == DISABLE_SCRIPT_EDITING));

      m_pcv->LoadFromStream(pbr->m_pistream, pbr->m_hcrypthash, script_protected ? pbr->m_hcryptkey : NULL);
      break;
   }
   case FID(CCUS): pbr->GetStruct(m_rgcolorcustom, sizeof(COLORREF) * 16); break;
   case FID(TDFT): pbr->GetFloat(m_difficulty); break;
   case FID(CUST):
   {
      string tmp;
      pbr->GetString(tmp);
      m_vCustomInfoTag.push_back(tmp);
      break;
   }
   case FID(SVOL): pbr->GetFloat(m_TableSoundVolume); break;
   case FID(BDMO): pbr->GetBool(m_BallDecalMode); break;
   case FID(MVOL): pbr->GetFloat(m_TableMusicVolume); break;
   case FID(AVSY):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int tableAdaptiveVSync;
         pbr->GetInt(tableAdaptiveVSync);
         if (tableAdaptiveVSync != -1)
         {
            switch (tableAdaptiveVSync)
            {
            case 0:
                m_settings.SaveValue(Settings::Player, "MaxFramerate"s, 0);
                m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_NONE);
                break;
            case 1:
                m_settings.SaveValue(Settings::Player, "MaxFramerate"s, 0);
                m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_VSYNC);
                break;
            case 2:
                m_settings.SaveValue(Settings::Player, "MaxFramerate"s, 0);
                m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_ADAPTIVE_VSYNC);
                break;
            default:
                m_settings.SaveValue(Settings::Player, "MaxFramerate"s, tableAdaptiveVSync);
                m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_ADAPTIVE_VSYNC);
                break;
            }
         }
      }
      break;
   case FID(OGAC):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         bool overwriteGlobalDetailLevel;
         pbr->GetBool(overwriteGlobalDetailLevel);
         if (!overwriteGlobalDetailLevel)
            m_settings.DeleteValue(Settings::Player, "AlphaRampAccuracy"s);
      }
      break;
   case FID(OGDN):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         // Global Day/Night was fairly convoluted:
         // - table would define the value
         // - user could select in video options to override this value by an automatic value
         // - table could then define to reject this user settings
         // - user could define in commandline to finally override the value
         // Now the logic is the same as all other settings:
         // - table defines the default value, then user defines if he wants to override this value (through app/table settings or commandline)
         bool overwriteGlobalDayNight;
         pbr->GetBool(overwriteGlobalDayNight);
         if (overwriteGlobalDayNight)
            m_settings.SaveValue(Settings::Player, "OverrideTableEmissionScale"s, false);
      }
      break;
   case FID(GDAC): pbr->GetBool(m_grid); break;
   // Removed in 10.8 since we now directly define reflection in render probe. Table author can disable default playfield reflection by setting PF reflection strength to 0. Player uses app/table settings to tweak
   // case FID(REOP): pbr->GetBool(m_reflectElementsOnPlayfield); break;
   case FID(ARAC):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int userDetailLevel; // The detail level was always saved **before** the override flag so we always load to settings, eventually deleting afterward
         pbr->GetInt(userDetailLevel);
         m_settings.SaveValue(Settings::Player, "AlphaRampAccuracy"s, userDetailLevel);
      }
      break;
   case FID(MASI): pbr->GetInt(m_numMaterials); break;
   case FID(MATE):
   {
      vector<SaveMaterial> mats(m_numMaterials);
      pbr->GetStruct(mats.data(), (int)sizeof(SaveMaterial)*m_numMaterials);
      if (pbr->m_version < 1080
         || m_materials.empty()) // Also loads materials for 10.8+ tables if these were saved before the new material format was added. // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
      {
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
            pmat->m_type = mats[i].bIsMetal ? Material::MaterialType::METAL : Material::MaterialType::BASIC;
            pmat->m_bOpacityActive = !!(mats[i].bOpacityActive_fEdgeAlpha & 1);
            pmat->m_fEdgeAlpha = dequantizeUnsigned<7>(mats[i].bOpacityActive_fEdgeAlpha >> 1);
            pmat->m_szName = mats[i].szName;
            m_materials.push_back(pmat);
         }
      }
      break;
   }
   case FID(PHMA):
   {
       vector<SavePhysicsMaterial> mats(m_numMaterials);
       pbr->GetStruct(mats.data(), (int)sizeof(SavePhysicsMaterial)*m_numMaterials);
       if (pbr->m_version < 1080
          || m_materials.size() == m_numMaterials) // Also loads materials for 10.8+ tables if these were saved before the new material format was added. // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
       {
          for (int i = 0; i < m_numMaterials; i++)
          {
              bool found = true;
              Material * pmat = GetMaterial(mats[i].szName);
              if (pmat == &m_vpinball->m_dummyMaterial)
              {
                  assert(!"SaveMaterial not found");
                  pmat = new Material();
                  pmat->m_szName = mats[i].szName;
                  found = false;
              }
              pmat->m_fElasticity = mats[i].fElasticity;
              pmat->m_fElasticityFalloff = mats[i].fElasticityFallOff;
              pmat->m_fFriction = mats[i].fFriction;
              pmat->m_fScatterAngle = mats[i].fScatterAngle;
              if (!found)
                 m_materials.push_back(pmat);
          }
       }
       break;
   }
   case FID(MATR):
   {
      // Replace legacy materials with the new ones. // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
      if (pbr->m_version >= 1080 && m_materials.size() == m_numMaterials)
      {
         for (int i = 0; i < m_numMaterials; i++)
            delete m_materials[i];
         m_materials.clear();
      }
      const int record_size = pbr->GetBytesInRecordRemaining();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      LPVOID pData = ::GlobalLock(hMem);
      pbr->GetStruct(pData, record_size);
      ::GlobalUnlock(hMem);
      Material *rpb = new Material();
      CComPtr<IStream> spStream;
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
      if (rpb->LoadData(spStream, this, pbr->m_version, NULL, NULL) != S_OK)
      {
         assert(!"Invalid binary image file");
         return false;
      }
      m_materials.push_back(rpb);
      ::GlobalFree(hMem);
      break;
   }
   case FID(RPRB):
   {
      const int record_size = pbr->GetBytesInRecordRemaining();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      LPVOID pData = ::GlobalLock(hMem);
      pbr->GetStruct(pData, record_size);
      ::GlobalUnlock(hMem);
      RenderProbe *rpb = new RenderProbe();
      CComPtr<IStream> spStream;
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
      if (rpb->LoadData(spStream, this, pbr->m_version, NULL, NULL) != S_OK)
      {
         assert(!"Invalid binary image file");
         delete rpb;
         return false;
      }
      m_vrenderprobe.push_back(rpb);
      ::GlobalFree(hMem);
      break;
   }
   case FID(TLCK): pbr->GetInt(m_locked); break;
   }
   return true;
}

bool PinTable::ExportSound(PinSound * const pps, const char * const szfilename)
{
   if(!pps->IsWav2())
   {
      FILE* f;
      if ((fopen_s(&f, szfilename, "wb") == 0) && f)
      {
         fwrite(pps->m_pdata, 1, pps->m_cdata, f);
         fclose(f);
         return true;
      }

      m_mdiTable->MessageBox("Can not Open/Create Sound file!", "Visual Pinball", MB_ICONERROR);
      return false;
   }

   // standard/old .wav export pipeline:

   MMIOINFO mmio = {};

   const HMMIO hmmio = mmioOpen((LPSTR)szfilename, &mmio, MMIO_ALLOCBUF | MMIO_CREATE | MMIO_EXCLUSIVE | MMIO_READWRITE);

   if (hmmio != nullptr)
   {
      MMCKINFO pck = {};

      pck.ckid = mmioStringToFOURCC("RIFF", MMIO_TOUPPER);
      pck.cksize = pps->m_cdata + 36;
      pck.fccType = mmioStringToFOURCC("WAVE", MMIO_TOUPPER);

      MMRESULT result = mmioCreateChunk(hmmio, &pck, MMIO_CREATERIFF); //RIFF header
      mmioWrite(hmmio, "fmt ", 4);                                     //fmt

      // Create the format chunk.
      pck.cksize = sizeof(WAVEFORMATEX);
      result = mmioCreateChunk(hmmio, &pck, 4);//0
      // Write the wave format data.
      int i = 16;
      mmioWrite(hmmio, (char *)&i, 4);
      mmioWrite(hmmio, (char*)&pps->m_wfx, (LONG)sizeof(pps->m_wfx) - 2); //END OF CORRECTION

      mmioWrite(hmmio, "data", 4);                       //data chunk
      i = pps->m_cdata; mmioWrite(hmmio, (char *)&i, 4); // data size bytes

      const LONG wcch = mmioWrite(hmmio, pps->m_pdata, pps->m_cdata);
      result = mmioClose(hmmio, 0);

      if (wcch != pps->m_cdata) 
         m_mdiTable->MessageBox("Sound file incomplete!", "Visual Pinball", MB_ICONERROR);
      else
         return true;
   }
   else
      m_mdiTable->MessageBox("Can not Open/Create Sound file!", "Visual Pinball", MB_ICONERROR);

   return false;
}

void PinTable::ReImportSound(const HWND hwndListView, PinSound * const pps, const string& filename)
{
   PinSound * const ppsNew = m_vpinball->m_ps.LoadFile(filename);

   if (ppsNew == nullptr)
      return;

   //!! meh to all of this: manually copy old sound manager params to temp vars

   const int balance = pps->m_balance;
   const int fade = pps->m_fade;
   const int volume = pps->m_volume;
   const SoundOutTypes outputTarget = pps->GetOutputTarget();
   const string szName = pps->m_szName;

   //!! meh to all of this: kill old raw sound data and DSound/BASS stuff, then copy new one over

   pps->UnInitialize();
   delete[] pps->m_pdata;

   *pps = *ppsNew;

   //!! meh to all of this: set to 0, so this is not free'd in the dtor, as used in pps from now on

   ppsNew->m_pdata = nullptr;
   ppsNew->m_pDS3DBuffer = nullptr;
   ppsNew->m_pDSBuffer = nullptr;
   ppsNew->m_BASSstream = 0;

   delete ppsNew;

   // recopy old settings over to new sound file
   pps->m_balance = balance;
   pps->m_fade = fade;
   pps->m_volume = volume;
   pps->SetOutputTarget(outputTarget);
   pps->m_szName = szName;
}


void PinTable::ImportSound(const HWND hwndListView, const string& szfilename)
{
   PinSound * const pps = m_vpinball->m_ps.LoadFile(szfilename);

   if (pps == nullptr)
      return;

   m_vsound.push_back(pps);

   const int index = AddListSound(hwndListView, pps);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
}

void PinTable::ListSounds(HWND hwndListView)
{
	ListView_DeleteAllItems(hwndListView);
	for (size_t i = 0; i < m_vsound.size(); i++)
		AddListSound(hwndListView, m_vsound[i]);
}


int PinTable::AddListSound(HWND hwndListView, PinSound * const pps)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)pps->m_szName.c_str();
   lvitem.lParam = (size_t)pps;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, (LPSTR)pps->m_szPath.c_str());

   switch (pps->GetOutputTarget())
   {
   case SNDOUT_BACKGLASS:
	   ListView_SetItemText(hwndListView, index, 2, (LPSTR)"Backglass");
	   break;
   default:
	   assert(false);
   case SNDOUT_TABLE:
	   ListView_SetItemText(hwndListView, index, 2, (LPSTR)"Table");
	   break;
   }
   char textBuf[40];
   sprintf_s(textBuf, sizeof(textBuf), "%.03f", dequantizeSignedPercent(pps->m_balance));
   ListView_SetItemText(hwndListView, index, 3, textBuf);
   sprintf_s(textBuf, sizeof(textBuf), "%.03f", dequantizeSignedPercent(pps->m_fade));
   ListView_SetItemText(hwndListView, index, 4, textBuf);
   sprintf_s(textBuf, sizeof(textBuf), "%.03f", dequantizeSignedPercent(pps->m_volume));
   ListView_SetItemText(hwndListView, index, 5, textBuf);

   return index;
}

void PinTable::RemoveSound(PinSound * const pps)
{
   RemoveFromVectorSingle(m_vsound, pps);

   delete pps;
}

void PinTable::ImportFont(HWND hwndListView, const string& filename)
{
   PinFont * const ppb = new PinFont();

   ppb->ReadFromFile(filename);

   if (ppb->m_pdata != nullptr)
   {
      m_vfont.push_back(ppb);

      const int index = AddListBinary(hwndListView, ppb);

      ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

      ppb->Register();
   }
}

void PinTable::RemoveFont(PinFont * const ppf)
{
   RemoveFromVectorSingle(m_vfont, ppf);

   ppf->UnRegister();
   delete ppf;
}

void PinTable::ListFonts(HWND hwndListView)
{
   for (size_t i = 0; i < m_vfont.size(); i++)
      AddListBinary(hwndListView, m_vfont[i]);
}

int PinTable::AddListBinary(HWND hwndListView, PinBinary *ppb)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)ppb->m_szName.c_str();
   lvitem.lParam = (size_t)ppb;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, (LPSTR)ppb->m_szPath.c_str());

   return index;
}

void PinTable::NewCollection(const HWND hwndListView, const bool fromSelection)
{
   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();

   const LocalStringW prefix(IDS_COLLECTION);
   WCHAR wzT[128];
   GetUniqueName(prefix.m_szbuffer, wzT, 128);

   WideStrNCopy(wzT, pcol->m_wzName, MAXNAMEBUFFER);

   if (fromSelection && !MultiSelIsEmpty())
   {
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect * const pisel = m_vmultisel.ElementAt(i);
         IEditable * const piedit = pisel->GetIEditable();
         if (piedit)
         {
            if (piedit->GetISelect() == pisel) // Do this check so we don't put walls in a collection when we only have the control point selected
            {
               if (piedit->GetScriptable()) // check for scriptable because can't add decals to a collection - they have no name
               {
                  piedit->m_vCollection.push_back(pcol);
                  piedit->m_viCollection.push_back(pcol->m_visel.size());
                  pcol->m_visel.push_back(m_vmultisel.ElementAt(i));
               }
            }
         }
      }
   }

   const int index = AddListCollection(hwndListView, pcol);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

   m_vcollection.push_back(pcol);
   m_pcv->AddItem((IScriptable *)pcol, false);
}

int PinTable::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
{
   char szT[sizeof(pcol->m_wzName)/sizeof(pcol->m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, pcol->m_wzName, -1, szT, sizeof(szT), nullptr, nullptr);

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = szT;
   lvitem.lParam = (size_t)pcol;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   char buf[16] = { 0 };
   sprintf_s(buf, sizeof(buf), "%i", pcol->m_visel.size());
   ListView_SetItemText(hwndListView, index, 1, buf);
   return index;
}

void PinTable::ListCollections(HWND hwndListView)
{
   //ListView_DeleteAllItems(hwndListView);

   for (int i = 0; i < m_vcollection.size(); i++)
   {
      CComObject<Collection> * const pcol = m_vcollection.ElementAt(i);

      AddListCollection(hwndListView, pcol);
   }
}

void PinTable::RemoveCollection(CComObject<Collection> *pcol)
{
   m_pcv->RemoveItem((IScriptable *)pcol);
   m_vcollection.find_erase(pcol);
   pcol->Release();
}

void PinTable::MoveCollectionUp(CComObject<Collection> *pcol)
{
   const int idx = m_vcollection.find(pcol);
   assert(idx >= 0);
   m_vcollection.erase(idx);
   if (idx - 1 < 0)
      m_vcollection.push_back(pcol);
   else
      m_vcollection.insert(pcol, idx - 1);
}

float PinTable::GetZPD() const
{
   return m_overwriteGlobalStereo3D ? m_3DZPD : m_global3DZPD;
}

void PinTable::SetZPD(const float value)
{
   if (m_overwriteGlobalStereo3D)
      m_3DZPD = value;
}

float PinTable::GetMaxSeparation() const
{
   return m_overwriteGlobalStereo3D ? m_3DmaxSeparation : m_global3DMaxSeparation;
}

void PinTable::SetMaxSeparation(const float value)
{
   if (m_overwriteGlobalStereo3D)
      m_3DmaxSeparation = value;
}

float PinTable::Get3DOffset() const
{
   return m_overwriteGlobalStereo3D ? m_3DOffset : m_global3DOffset;
}

FRect3D PinTable::GetBoundingBox() const
{
   FRect3D bbox;
   bbox.left = m_left;
   bbox.right = m_right;
   bbox.top = m_top;
   bbox.bottom = m_bottom;
   bbox.zlow = 0.f;
   bbox.zhigh = m_glassTopHeight;
   return bbox;
}

void PinTable::ComputeNearFarPlane(const Matrix3D &matWorldView, const float scale, float &zNear, float &zFar) const
{
   zNear = FLT_MAX;
   zFar = -FLT_MAX;
   // Adjust near/far plane for each projected bounding boxes
   vector<Vertex3Ds> vvertex3D;
   for (IEditable *editable : m_vedit)
   {
      editable->GetBoundingVertices(vvertex3D, false);
      for (const Vertex3Ds &v : vvertex3D)
      {
         Vertex3Ds p = v;
         matWorldView.TransformVec3(p);
         if (p.z > 0.0f)
         {
            // Clip points behind the viewer (VR room have a lot of these)
            zNear = min(zNear, p.z);
            zFar = max(zFar, p.z);
         }
      }
      vvertex3D.clear();
   }
   // Add a bit of margin
   zNear *= 0.9f;
   zFar *= 1.1f;
   // Clip to sensible value to fix tables with parts far far away breaking depth buffer precision
   zNear = max(zNear, scale * CMTOVPU(5.f)); // Avoid wasting depth buffer precision for parts too near to be useful
   zFar = clamp(zFar, zNear + 1.f, scale * CMTOVPU(100000.f)); // 1 km (yes some VR room do really need this...)
   // Could not reproduce, so I disabled it for the sake of avoiding to pass inc to the method which is not really meaningful here (we would have to compute it from the matWorldView)
   //!! magic threshold, otherwise kicker holes are missing for inclination ~0
   //if (fabsf(inc) < 0.0075f)
   //   zFar += 10.f;
   //PLOGD << "Near/Far plane: " << zNear << " to " << zFar;
}

void PinTable::MoveCollectionDown(CComObject<Collection> *pcol)
{
   const int idx = m_vcollection.find(pcol);
   assert(idx >= 0);
   m_vcollection.erase(idx);
   if (idx + 1 >= m_vcollection.size())
      m_vcollection.insert(pcol, 0);
   else
      m_vcollection.insert(pcol, idx + 1);
}

void PinTable::SetCollectionName(Collection *pcol, const char *szName, HWND hwndList, int index)
{
   WCHAR wzT[MAXSTRING];
   MultiByteToWideCharNull(CP_ACP, 0, szName, -1, wzT, MAXSTRING);
   if (m_pcv->ReplaceName((IScriptable *)pcol, wzT) == S_OK)
   {
      if (hwndList)
         ListView_SetItemText(hwndList, index, 0, (char*)szName);
      WideStrNCopy(wzT, pcol->m_wzName, MAXNAMEBUFFER);
   }
}

void PinTable::SetZoom(float zoom)
{
   m_zoom = zoom;
   SetMyScrollInfo();
}

void PinTable::GetViewRect(FRect * const pfrect) const
{
   if (!m_vpinball->m_backglassView)
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

   const CRect rc = GetClientRect();

   const HitSur phs(nullptr, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

   Vertex2D rgv[2];
   rgv[0] = phs.ScreenToSurface(rc.left, rc.top);
   rgv[1] = phs.ScreenToSurface(rc.right, rc.bottom);

   SCROLLINFO si = {};
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
   si.nMin = (int)min(frect.left, rgv[0].x);
   si.nMax = (int)max(frect.right, rgv[1].x);
   si.nPage = (int)(rgv[1].x - rgv[0].x);
   si.nPos = (int)(rgv[0].x);

   SetScrollInfo(SB_HORZ, si, fTrue);

   si.nMin = (int)min(frect.top, rgv[0].y);
   si.nMax = (int)max(frect.bottom, rgv[1].y);
   si.nPage = (int)(rgv[1].y - rgv[0].y);
   si.nPos = (int)(rgv[0].y);

   SetScrollInfo(SB_VERT, si, fTrue);
}

void PinTable::FireKeyEvent(int dispid, int keycode)
{
   CComVariant rgvar[1] = { CComVariant(keycode) };

   DISPPARAMS dispparams = {
      rgvar,
      nullptr,
      1,
      0
   };

#ifdef LOG
   if (g_pplayer)
   {
      if (dispid == DISPID_GameEvents_KeyDown)
         PLOGD.printf("Key Down %d", keycode);
      else
         PLOGD.printf("Key Up %d", keycode);
   }
#endif

   FireDispID(dispid, &dispparams);
}

void PinTable::DoLeftButtonDown(int x, int y, bool zoomIn)
{
   const int ksshift = GetKeyState(VK_SHIFT);
   const int ksctrl = GetKeyState(VK_CONTROL);

   // set the focus of the window so all keyboard and mouse inputs are processed.
   // (this fixes the problem of selecting a element on the properties dialog, clicking on a table
   // object and not being able to use the cursor keys/wheely mouse)
   m_vpinball->SetFocus();

   if ((m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY) || (ksctrl & 0x80000000))
   {
      if (m_zoom < MAX_ZOOM)
      {
         m_offset = TransformPoint(x, y);

         SetZoom(m_zoom * (zoomIn ? 1.5f : 0.5f));

         SetDirtyDraw();
      }
   }
   else
   {
      ISelect * const pisel = HitTest(x, y);

      const bool add = ((ksshift & 0x80000000) != 0);

      if (pisel == (ISelect *)this && add)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         OnLButtonDown(x, y); // Start the band select
         return;
      }

      AddMultiSel(pisel, add, true, false);

      m_moving = true;
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect *const pisel2 = m_vmultisel.ElementAt(i);
         if (pisel2)
            pisel2->OnLButtonDown(x, y);
      }
   }
}

void PinTable::OnLeftButtonUp(int x, int y)
{
   if (!m_dragging) // Not doing band select
   {
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect * const pisel = m_vmultisel.ElementAt(i);
         if (pisel)
            pisel->OnLButtonUp(x, y);
      }
      if (m_moving)
      {
          m_moving = false;
          m_vpinball->SetPropSel(m_vmultisel);
      }
   }
   else
   {
      OnLButtonUp(x, y);
   }
}

void PinTable::OnRightButtonDown(int x, int y)
{
   OnLeftButtonUp(x, y); //corrects issue with left mouse button being in 'stuck down' position on a control point or object - BDS

   const int ks = GetKeyState(VK_CONTROL);

   if ((m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000))
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
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         if (FindIndexOf(m_allHitElements, m_vmultisel.ElementAt(i)) != -1)
         {
            // found a selected item - keep the current selection set
            // by re-selecting this item (which will also promote it
            // to the head of the selection list)
            hit = m_vmultisel.ElementAt(i);
            break;
         }
      }

      // update the selection
      AddMultiSel(hit, false, true, false);
   }
}

void PinTable::FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, ISelect *psel)
{
    const LocalString ls16(IDS_TO_COLLECTION);
    mainMenu.AppendMenu(MF_POPUP | MF_STRING, (size_t)colSubMenu.GetHandle(), ls16.m_szbuffer);

    const int maxItems = m_vcollection.size() - 1;

    // run through all collections and list them in the context menu
    // the actual processing is done in ISelect::DoCommand() 
    for (int i = maxItems; i >= 0; i--)
    {
        CComBSTR bstr;
        m_vcollection[i].get_Name(&bstr);
        char szT[MAXNAMEBUFFER*2]; // Names can only be 32 characters (plus terminator)
        WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, szT, MAXNAMEBUFFER*2, nullptr, nullptr);

        UINT flags = MF_POPUP | MF_UNCHECKED;
        if ((maxItems-i) % 32 == 0) // add new column each 32 entries
           flags |= MF_MENUBREAK;
        colSubMenu.AppendMenu(flags, 0x40000 + i, szT);
    }
    if (m_vmultisel.size() == 1)
    {
        for (int i = maxItems; i >= 0; i--)
            for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
                if (psel == m_vcollection[i].m_visel.ElementAt(t))
                    colSubMenu.CheckMenuItem(0x40000 + i, MF_CHECKED);
    }
    else
    {
        vector<int> allIndices;

        for (int t = 0; t < m_vmultisel.size(); t++)
        {
            const ISelect * const iSel = m_vmultisel.ElementAt(t);

            for (int i = maxItems; i >= 0; i--)
                for (int t2 = 0; t2 < m_vcollection[i].m_visel.size(); t2++)
                    if ((iSel == m_vcollection[i].m_visel.ElementAt(t2)))
                        allIndices.push_back(i);
        }
        for (size_t i = 0; i < allIndices.size(); i++)
            colSubMenu.CheckMenuItem(0x40000 + allIndices[i], MF_CHECKED);
    }
}

void PinTable::FillLayerContextMenu(CMenu &mainMenu, CMenu &layerSubMenu, ISelect *psel) 
{
   const LocalString ls16(IDS_ASSIGN_TO_LAYER2);
   mainMenu.AppendMenu(MF_POPUP | MF_STRING, (size_t)layerSubMenu.GetHandle(), ls16.m_szbuffer);
   const vector<string> layerNames = g_pvp->GetLayersListDialog()->GetAllLayerNames();
   int i = 0;
   for (const auto &name : layerNames)
   {
      layerSubMenu.AppendMenu(MF_STRING, ID_ASSIGN_TO_LAYER1 + i, name.c_str());
      i++;
      if (i == NUM_ASSIGN_LAYERS)
        break;
   }
}

void PinTable::DoContextMenu(int x, int y, const int menuid, ISelect *psel)
{
   POINT pt;
   pt.x = x;
   pt.y = y;
   m_mdiTable->ClientToScreen(pt);

   CMenu mainMenu;
   CMenu newMenu;

   mainMenu.LoadMenu(menuid);
   if (menuid != -1)
       newMenu = mainMenu.GetSubMenu(0);
   else
       newMenu.CreatePopupMenu();

   psel->EditMenu(newMenu);

   if (menuid != IDR_POINTMENU && menuid != IDR_TABLEMENU && menuid != IDR_POINTMENU_SMOOTH)
   {
       
      if (newMenu.GetMenuItemCount() > 0)
          newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      
      CMenu assignLayerMenu;
      CMenu colSubMenu;
      CMenu layerSubMenu;

      assignLayerMenu.CreatePopupMenu();
      colSubMenu.CreatePopupMenu();
      layerSubMenu.CreatePopupMenu();

      // TEXT
      const LocalString ls17(IDS_COPY_ELEMENT);
      newMenu.AppendMenu(MF_STRING, IDC_COPY, ls17.m_szbuffer);
      const LocalString ls18(IDS_PASTE_ELEMENT);
      newMenu.AppendMenu(MF_STRING, IDC_PASTE, ls18.m_szbuffer);
      const LocalString ls19(IDS_PASTE_AT_ELEMENT);
      newMenu.AppendMenu(MF_STRING, IDC_PASTEAT, ls19.m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");

      const LocalString ls14(IDS_DRAWING_ORDER_HIT);
      newMenu.AppendMenu(MF_STRING, ID_EDIT_DRAWINGORDER_HIT, ls14.m_szbuffer);
      const LocalString ls15(IDS_DRAWING_ORDER_SELECT);
      newMenu.AppendMenu(MF_STRING, ID_EDIT_DRAWINGORDER_SELECT, ls15.m_szbuffer);

      const LocalString ls1(IDS_DRAWINFRONT);
      const LocalString ls2(IDS_DRAWINBACK);
      newMenu.AppendMenu(MF_STRING, ID_DRAWINFRONT, ls1.m_szbuffer);
      newMenu.AppendMenu(MF_STRING, ID_DRAWINBACK, ls2.m_szbuffer);

      const LocalString ls3(IDS_SETASDEFAULT);
      newMenu.AppendMenu(MF_STRING, ID_SETASDEFAULT, ls3.m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");

      FillLayerContextMenu(newMenu, layerSubMenu, psel);
      const LocalString lsLayer(IDS_ASSIGN_TO_CURRENT_LAYER);
      newMenu.AppendMenu(MF_STRING, ID_ASSIGN_TO_CURRENT_LAYER, lsLayer.m_szbuffer);
      FillCollectionContextMenu(newMenu, colSubMenu, psel);

      const LocalString ls5(IDS_LOCK);
      newMenu.AppendMenu(MF_STRING, ID_LOCK, ls5.m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      // now list all elements that are stacked at the mouse pointer
      for (size_t i = 0; i < m_allHitElements.size(); i++)
      {
         if (!m_allHitElements[i]->GetIEditable()->GetISelect()->m_isVisible)
            continue;

         ISelect * const ptr = m_allHitElements[i];
         if (ptr)
         {
            IEditable * const pedit = m_allHitElements[i]->GetIEditable();
            if (pedit)
            {
               const char * const szTemp = GetElementName(pedit);

               if (szTemp)
               {
                  //!! what a hack!
                  // the element index of the allHitElements vector is encoded inside the ID of the context menu item
                  // I didn't find an easy way to identify the selected menu item of a context menu
                  // so the ID_SELECT_ELEMENT is the global ID for selecting an element from the list and the rest is
                  // added for finding the element out of the list
                  // the selection is done in ISelect::DoCommand()
                  const unsigned long id = 0x80000000 + ((unsigned long)i << 16) + ID_SELECT_ELEMENT;
                  newMenu.AppendMenu(MF_STRING, id, szTemp);
               }
            }
         }
      }
      bool locked = psel->m_locked;
      //!! HACK
      if (psel == this) // multi-select case
      {
         locked = FMutilSelLocked();
      }
      newMenu.CheckMenuItem(ID_LOCK, MF_BYCOMMAND | (locked ? MF_CHECKED : MF_UNCHECKED));
   }

   const int icmd = newMenu.TrackPopupMenuEx(TPM_RETURNCMD, pt.x, pt.y, m_mdiTable->GetHwnd(), nullptr);

   if (icmd != 0)
      psel->DoCommand(icmd, x, y);

   newMenu.DestroyMenu();

   if (menuid != -1)
       mainMenu.DestroyMenu();
}

const char *PinTable::GetElementName(IEditable *pedit) const
{
   WCHAR *elemName = nullptr;
   if (pedit)
   {
      if (pedit->GetItemType() == eItemDecal)
         return "Decal";

      IScriptable * const pscript = pedit->GetScriptable();
      if (pscript)
         elemName = pscript->m_wzName;
   }
   if (elemName)
   {
      static char elementName[256];
      WideCharToMultiByteNull(CP_ACP, 0, elemName, -1, elementName, 256, nullptr, nullptr);
      return elementName;
   }
   return nullptr;
}

IEditable *PinTable::GetElementByName(const char * const name)
{
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const pedit = m_vedit[i];
      if (strcmp(name, GetElementName(pedit)) == 0)
         return pedit;
   }
   return nullptr;
}

bool PinTable::FMutilSelLocked()
{
   for (int i = 0; i < m_vmultisel.size(); i++)
      if (m_vmultisel[i].m_locked)
         return true;

   return false;
}

void PinTable::DoCommand(int icmd, int x, int y)
{
   if (((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020))
   {
      UpdateCollection(icmd & 0x000000FF);
      return;
   }

   if ((icmd >= ID_ASSIGN_TO_LAYER1) && (icmd <= ID_ASSIGN_TO_LAYER1+NUM_ASSIGN_LAYERS-1))
   {
      /*add to layer*/
      m_vpinball->GetLayersListDialog()->AssignToLayerByIndex(icmd - ID_ASSIGN_TO_LAYER1);
   }

   if ((icmd & 0x0000FFFF) == ID_SELECT_ELEMENT)
   {
      const int i = (icmd & 0x00FF0000) >> 16;
      ISelect * const pisel = m_allHitElements[i];
      pisel->DoCommand(icmd, x, y);
      return;
   }

   switch (icmd)
   {
       case ID_DRAWINFRONT:
       case ID_DRAWINBACK:
       {
           for (int i = 0; i < m_vmultisel.size(); i++)
           {
               ISelect *const psel = m_vmultisel.ElementAt(i);
               _ASSERTE(psel != this); // Would make an infinite loop
               psel->DoCommand(icmd, x, y);
           }
           break;
       }
       case ID_ASSIGN_TO_CURRENT_LAYER: m_vpinball->GetLayersListDialog()->OnAssignButton(); break;
       case ID_EDIT_DRAWINGORDER_HIT: m_vpinball->ShowDrawingOrderDialog(false); break;
       case ID_EDIT_DRAWINGORDER_SELECT: m_vpinball->ShowDrawingOrderDialog(true); break;
       case ID_LOCK: LockElements(); break;
       case ID_WALLMENU_FLIP: FlipY(GetCenter()); break;
       case ID_WALLMENU_MIRROR: FlipX(GetCenter()); break;
       case IDC_COPY: Copy(x, y); break;
       case IDC_PASTE: Paste(false, x, y); break;
       case IDC_PASTEAT: Paste(true, x, y); break;
       case ID_WALLMENU_ROTATE: DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_ROTATE), m_vpinball->GetHwnd(), RotateProc, (size_t)(ISelect *)this); break;
       case ID_WALLMENU_SCALE: DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_SCALE), m_vpinball->GetHwnd(), ScaleProc, (size_t)(ISelect *)this); break;
       case ID_WALLMENU_TRANSLATE: DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_TRANSLATE), m_vpinball->GetHwnd(), TranslateProc, (size_t)(ISelect *)this); break;
   }
}

void PinTable::UpdateCollection(const int index)
{
   if (index < m_vcollection.size())
   {
      if (!m_vmultisel.empty())
      {
         bool removeOnly = false;
         /* if the selection is part of the selected collection remove only these elements*/
         for (int t = 0; t < m_vmultisel.size(); t++)
         {
            ISelect * const ptr = m_vmultisel.ElementAt(t);
            for (int k = 0; k < m_vcollection[index].m_visel.size(); k++)
            {
               if (ptr == m_vcollection[index].m_visel.ElementAt(k))
               {
                  m_vcollection[index].m_visel.find_erase(ptr);
                  removeOnly = true;
                  break;
               }
            }
         }

         if (removeOnly)
            return;

         /*selected elements are not part of the selected collection and can be added*/
         for (int t = 0; t < m_vmultisel.size(); t++)
         {
            ISelect * const ptr = m_vmultisel.ElementAt(t);
            m_vcollection.ElementAt(index)->m_visel.push_back(ptr);
        }
      }
   }
}

bool PinTable::GetCollectionIndex(const ISelect * const element, int &collectionIndex, int &elementIndex)
{
   for (int i = 0; i < m_vcollection.size(); i++)
   {
      for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
      {
         if (element == m_vcollection[i].m_visel.ElementAt(t))
         {
            collectionIndex = i;
            elementIndex = t;
            return true;
         }
      }
   }
   return false;
}

WCHAR *PinTable::GetCollectionNameByElement(const ISelect * const element)
{
    for (int i = 0; i < m_vcollection.size(); i++)
        for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
            if (element == m_vcollection[i].m_visel.ElementAt(t))
                return m_vcollection[i].m_wzName;

    return nullptr;
}

void PinTable::LockElements()
{
   BeginUndo();
   const bool lock = !FMutilSelLocked();
   for (int i = 0; i < m_vmultisel.size(); i++)
   {
      ISelect * const psel = m_vmultisel.ElementAt(i);
      if (psel)
      {
         IEditable * const pedit = psel->GetIEditable();
         if (pedit)
         {
            pedit->MarkForUndo();
            psel->m_locked = lock;
         }
      }
   }
   EndUndo();
   SetDirtyDraw();
}

LRESULT PinTable::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SETCURSOR:
            SetMouseCursor();
            return FinalWindowProc(uMsg, wParam, lParam);
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            const HDC hdc = BeginPaint(ps);
            Paint(hdc);
            EndPaint(ps);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_SIZE: 
            OnSize();
            return FinalWindowProc(uMsg, wParam, lParam);
        case WM_LBUTTONDOWN:
        {
            const short x = (short)GET_X_LPARAM(lParam);
            const short y = (short)GET_Y_LPARAM(lParam);
            OnLeftButtonDown(x, y);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_LBUTTONDBLCLK:
        {
            const short x = (short)GET_X_LPARAM(lParam);
            const short y = (short)GET_Y_LPARAM(lParam);
            OnLeftDoubleClick(x, y);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_LBUTTONUP:
        {
            const short x = (short)GET_X_LPARAM(lParam);
            const short y = (short)GET_Y_LPARAM(lParam);
            OnLeftButtonUp(x, y);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_MOUSEMOVE:
        {
            const short x = (short)GET_X_LPARAM(lParam);
            const short y = (short)GET_Y_LPARAM(lParam);
            OnMouseMove(x, y);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_RBUTTONDOWN:
        {
            const short x = (short)GET_X_LPARAM(lParam);
            const short y = (short)GET_Y_LPARAM(lParam);
            OnRightButtonDown(x, y);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_CONTEXTMENU:
        {
            LONG x = GET_X_LPARAM(lParam);
            LONG y = GET_Y_LPARAM(lParam);
            POINT p;
            if (GetCursorPos(&p) && ScreenToClient(p))
            {
                x = p.x;
                y = p.y;
            }
            OnRightButtonUp(x, y);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_KEYDOWN:
        {
            OnKeyDown((int)wParam);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_HSCROLL:
        {
            SCROLLINFO si = {};
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(SB_HORZ, si);
            switch (LOWORD(wParam))
            {
                case SB_LINELEFT:
                    m_offset.x -= si.nPage / 10;
                    break;
                case SB_LINERIGHT:
                    m_offset.x += si.nPage / 10;
                    break;
                case SB_PAGELEFT:
                    m_offset.x -= si.nPage / 2;
                    break;
                case SB_PAGERIGHT:
                    m_offset.x += si.nPage / 2;
                    break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(m_offset.x - (float)si.nPos);
                    m_offset.x = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            SetDirtyDraw();
            SetMyScrollInfo();
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_VSCROLL:
        {
            SCROLLINFO si = {};
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(SB_VERT, si);
            switch (LOWORD(wParam))
            {
                case SB_LINEUP:
                    m_offset.y -= si.nPage / 10;
                    break;
                case SB_LINEDOWN:
                    m_offset.y += si.nPage / 10;
                    break;
                case SB_PAGEUP:
                    m_offset.y -= si.nPage / 2;
                    break;
                case SB_PAGEDOWN:
                    m_offset.y += si.nPage / 2;
                    break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(m_offset.y - (float)si.nPos);
                    m_offset.y = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            SetDirtyDraw();
            SetMyScrollInfo();
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case WM_MOUSEWHEEL:
        {
            //zoom in/out by pressing CTRL+mouse wheel
            const short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            OnMouseWheel(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), zDelta);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        case DONE_AUTOSAVE:
        {
            if (lParam == S_OK)
            {
                m_vpinball->SetActionCur("");
            }
            else
            {
                m_vpinball->SetActionCur("Autosave Failed");
            }
            BeginAutoSaveCounter();
            const HANDLE hEvent = (HANDLE)wParam;
            RemoveFromVectorSingle(m_vAsyncHandles, hEvent);
            CloseHandle(hEvent);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        default:
            break;
    }
    return WndProcDefault(uMsg, wParam, lParam);
}

void PinTable::FlipY(const Vertex2D& pvCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].FlipY(pvCenter);
   EndUndo();
}

void PinTable::FlipX(const Vertex2D& pvCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].FlipX(pvCenter);
   EndUndo();
}

void PinTable::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].Rotate(ang, pvCenter, useElementCenter);
   EndUndo();
}

void PinTable::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].Scale(scalex, scaley, pvCenter, useElementCenter);
   EndUndo();
}

void PinTable::Translate(const Vertex2D &pvOffset)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].Translate(pvOffset);
   EndUndo();
}

Vertex2D PinTable::GetCenter() const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (int i = 0; i < m_vmultisel.size(); i++)
   {
      const ISelect * const psel = m_vmultisel.ElementAt(i);
      const Vertex2D vCenter = psel->GetCenter();

      minx = min(minx, vCenter.x);
      maxx = max(maxx, vCenter.x);
      miny = min(miny, vCenter.y);
      maxy = max(maxy, vCenter.y);
      //tx += m_vdpoint[i]->m_v.x;
      //ty += m_vdpoint[i]->m_v.y;
   }

   return Vertex2D((maxx + minx)*0.5f, (maxy + miny)*0.5f);
}

void PinTable::PutCenter(const Vertex2D& pv)
{
}

void PinTable::OnRightButtonUp(int x, int y)
{
   GetSelectedItem()->OnRButtonUp(x, y);

   const int ks = GetKeyState(VK_CONTROL);

   // Only bring up context menu if we weren't in magnify mode
   if (!((m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000)))
   {
      if (m_vmultisel.size() > 1)
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
   const Vertex2D v = TransformPoint(x, y);

   m_vpinball->SetPosCur(v.x, v.y);

   if (!m_dragging) // Not doing band select
   {
       for (int i = 0; i < m_vmultisel.size(); i++)
         m_vmultisel[i].OnMouseMove(x, y);
   }
   else
      OnMouseMove(x, y);
}

void PinTable::OnLeftDoubleClick(int x, int y)
{
   //m_vpinball->m_sb.SetVisible(true);
   //::SendMessage(m_vpinball->m_hwnd, WM_SIZE, 0, 0);
}

void PinTable::ExportBlueprint()
{
   //bool saveAs = true;
   //if (saveAs)
   //{
      //need to get a file name
      OPENFILENAME ofn = {};
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = m_vpinball->theInstance;
      ofn.hwndOwner = m_vpinball->GetHwnd();
      ofn.lpstrFilter = "PNG (.png)\0*.png;\0Bitmap (.bmp)\0*.bmp;\0TGA (.tga)\0*.tga;\0TIFF (.tiff/.tif)\0*.tiff;*.tif;\0WEBP (.webp)\0*.webp;\0";
      char szBlueprintFileName[MAXSTRING];
      strncpy_s(szBlueprintFileName, m_szFileName.c_str(), sizeof(szBlueprintFileName)-1);
      const size_t idx = m_szFileName.find_last_of('.');
      if (idx != string::npos && idx < MAXSTRING)
          szBlueprintFileName[idx] = '\0';
      ofn.lpstrFile = szBlueprintFileName;
      ofn.nMaxFile = sizeof(szBlueprintFileName);
      ofn.lpstrDefExt = "png";
      ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

      const int ret = GetSaveFileName(&ofn);

      // user cancelled
      if (ret == 0)
         return;// S_FALSE;
   //}

   const int result = m_vpinball->MessageBox("Do you want a solid blueprint?", "Export As Solid?", MB_YESNO);
   const bool solid = (result == IDYES);

   float tableheight, tablewidth;
   if (m_vpinball->m_backglassView)
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
      bmwidth = (int)((tablewidth / tableheight) * (float)bmheight + 0.5f);
   }
   else
   {
      bmwidth = 4096;
      bmheight = (int)((tableheight / tablewidth) * (float)bmwidth + 0.5f);
   }

   int totallinebytes = bmwidth * 3;
   totallinebytes = (((totallinebytes - 1) / 4) + 1) * 4; // make multiple of four
#if 0
   HANDLE hfile = CreateFile(szBlueprintFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
   const int bmlinebuffer = totallinebytes - (bmwidth * 3);

   BITMAPFILEHEADER bmfh = {};
   bmfh.bfType = 'M' << 8 | 'B';
   bmfh.bfSize = sizeof(bmfh) + sizeof(BITMAPINFOHEADER) + totallinebytes*bmheight;
   bmfh.bfOffBits = (DWORD)sizeof(bmfh) + (DWORD)sizeof(BITMAPINFOHEADER);

   DWORD foo;
   WriteFile(hfile, &bmfh, sizeof(bmfh), &foo, nullptr);
#endif
   BITMAPINFO bmi = {};
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = bmwidth;
   bmi.bmiHeader.biHeight = bmheight;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 24;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = totallinebytes*bmheight;
#if 0
   WriteFile(hfile, &bmi, sizeof(BITMAPINFOHEADER), &foo, nullptr);
#endif

   CDC dc;
   dc.CreateCompatibleDC(nullptr);
   char *pbits;
   dc.CreateDIBSection(dc.GetHDC(), &bmi, DIB_RGB_COLORS, (void **)&pbits, nullptr, 0);

   {
   PaintSur psur(dc.GetHDC(), (float)bmwidth / tablewidth, tablewidth*0.5f, tableheight*0.5f, bmwidth, bmheight, nullptr);

   dc.SelectObject(reinterpret_cast<HBRUSH>(dc.GetStockObject(WHITE_BRUSH)));
   dc.PatBlt(0, 0, bmwidth, bmheight, PATCOPY);

   if (m_vpinball->m_backglassView)
      Render3DProjection(&psur);

   for(const auto &ptr : m_vedit)
   {
      if (ptr->GetISelect()->m_isVisible && ptr->m_backglass == m_vpinball->m_backglassView)
         ptr->RenderBlueprint(&psur, solid);
   }
   }

#if 0
   for (int i = 0; i < bmheight; i++)
      WriteFile(hfile, (pbits + ((i*bmwidth) * 3)), bmwidth * 3, &foo, nullptr);

   // For some reason to make our bitmap compatible with all programs,
   // We need to write out dummy bytes as if our totalwidthbytes had been
   // a multiple of 4.
   for (int i = 0; i < bmheight; i++)
      for (int l = 0; l < bmlinebuffer; l++)
         WriteFile(hfile, pbits, 1, &foo, nullptr);

   CloseHandle(hfile);
#else
   FIBITMAP * dib = FreeImage_Allocate(bmwidth, bmheight, 24);
   BYTE * const psrc = FreeImage_GetBits(dib);
   memcpy(psrc, pbits, (size_t)bmwidth*bmheight * 3);
   if (!FreeImage_Save(FreeImage_GetFIFFromFilename(szBlueprintFileName), dib, szBlueprintFileName, PNG_Z_BEST_COMPRESSION | BMP_SAVE_RLE))
       m_vpinball->MessageBox("Export failed!", "Blueprint Export", MB_OK | MB_ICONEXCLAMATION);
   else
#endif
       m_vpinball->MessageBox("Export finished!", "Blueprint Export", MB_OK);
#if 1
   FreeImage_Unload(dib);
#endif
}

void PinTable::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);

   Vertex3D_NoTex2 rgv[7];
   rgv[0].x = m_left;     rgv[0].y = m_top;      rgv[0].z = 0.f;
   rgv[1].x = m_right;    rgv[1].y = m_top;      rgv[1].z = 0.f;
   rgv[2].x = m_right;    rgv[2].y = m_bottom;   rgv[2].z = 0.f;
   rgv[3].x = m_left;     rgv[3].y = m_bottom;   rgv[3].z = 0.f;

   // These next 4 vertices are used just to set the extents
   rgv[4].x = m_left;     rgv[4].y = m_top;      rgv[4].z = 50.0f;
   rgv[5].x = m_left;     rgv[5].y = m_bottom;   rgv[5].z = 50.0f;
   rgv[6].x = m_right;    rgv[6].y = m_bottom;   rgv[6].z = 50.0f;
   //rgv[7].x = m_right;    rgv[7].y = m_top;      rgv[7].z = 50.0f;

   for (int i = 0; i < 4; ++i)
   {
      rgv[i].nx = 0;
      rgv[i].ny = 0;
      rgv[i].nz = 1.0f;

      rgv[i].tv = (i & 2) ? 1.0f : 0.f;
      rgv[i].tu = (i == 1 || i == 2) ? 1.0f : 0.f;
   }

   constexpr WORD playfieldPolyIndices[10] = { 0, 1, 3, 0, 3, 2, 2, 3, 5, 6 };

   Vertex3D_NoTex2 buffer[4 + 7];
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

   loader.WriteObjectName(name);
   loader.WriteVertexInfo(buffer, 4);
   const Material * const mat = GetMaterial(m_playfieldMaterial);
   loader.WriteMaterial(m_playfieldMaterial, string(), mat);
   loader.UseTexture(m_playfieldMaterial);
   loader.WriteFaceInfoList(playfieldPolyIndices, 6);
   loader.UpdateFaceOffset(4);
}

void PinTable::ExportTableMesh()
{
   char szObjFileName[MAXSTRING];
   strncpy_s(szObjFileName, m_szFileName.c_str(), sizeof(szObjFileName)-1);
   const size_t idx = m_szFileName.find_last_of('.');
   if (idx != string::npos && idx < MAXSTRING)
       szObjFileName[idx] = '\0';
   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = m_vpinball->theInstance;
   ofn.hwndOwner = m_vpinball->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Wavefront obj(*.obj)\0*.obj\0";
   ofn.lpstrFile = szObjFileName;
   ofn.nMaxFile = sizeof(szObjFileName);
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

   const int ret = GetSaveFileName(&ofn);

   // user canceled
   if (ret == 0)
      return;// S_FALSE;
   const string filename = string(szObjFileName);

   ObjLoader loader;
   loader.ExportStart(filename);
   ExportMesh(loader);
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const ptr = m_vedit[i];
      if (ptr->GetISelect()->m_isVisible && ptr->m_backglass == m_vpinball->m_backglassView)
         ptr->ExportMesh(loader);
   }
   loader.ExportEnd();
   m_vpinball->MessageBox("Export finished!", "Info", MB_OK | MB_ICONEXCLAMATION);
}

// Import Point of View file. This can be either:
// - a UI interaction from table author, loading to table **properties** after file selection,
// - without UI interaction, triggered to load user settings preference to table **settings**.
void PinTable::ImportBackdropPOV(const string &filename)
{
   string file = filename;
   const bool toUserSettings = !filename.empty();
   const bool wasModified = m_settings.IsModified();
   if (!toUserSettings)
   {
      if (IsLocked())
         return;
      string initialDir = m_settings.LoadValueWithDefault(Settings::RecentDir, "POVDir"s, PATH_TABLES);
      vector<string> fileNames;
      if (!m_vpinball->OpenFileDialog(initialDir, fileNames, 
         "User settings file (*.ini)\0*.ini\0Old POV file (*.pov)\0*.pov\0Legacy POV file(*.xml)\0*.xml\0",
         "ini", 0, toUserSettings ? "Import POV to user settings" : "Import POV to table properties"))
         return;
      file = fileNames[0];
      const size_t index = file.find_last_of(PATH_SEPARATOR_CHAR);
      if (index != string::npos)
         g_pvp->m_settings.SaveValue(Settings::RecentDir, "POVDir"s, file.substr(0, index));
   }

   string ext = ExtensionFromFilename(file);
   StrToLower(ext);

   static const string vsPrefix[3] = { "ViewDT"s, "ViewCab"s, "ViewFSS"s };
   static const char *vsFields[15] = { "Mode", "ScaleX", "ScaleY", "ScaleZ", "PlayerX", "PlayerY", "PlayerZ", "LookAt", "Rotation", "FOV", "Layback", "HOfs", "VOfs", "WindowTop", "WindowBot" };
   if (ext == "ini")
   {
      Settings settings;
      settings.LoadFromFile(file, false);
      for (int id = 0; id < 3; id++)
      {
         const string &keyPrefix = vsPrefix[id];
         if (toUserSettings)
         {
            for (int j = 0; j < 15; j++)
               if (settings.HasValue(Settings::TableOverride, keyPrefix + vsFields[j]))
                  m_settings.SaveValue(Settings::TableOverride, keyPrefix + vsFields[j], settings.LoadValueWithDefault(Settings::TableOverride, keyPrefix + vsFields[j], 0.f));
         }
         else
            mViewSetups[id].ApplyTableOverrideSettings(settings, (ViewSetupID)id);
      }
   }
   else if (ext == "pov" || ext == "xml")
   {
      tinyxml2::XMLDocument xmlDoc;
      try
      {
         std::stringstream buffer;
         std::ifstream myFile(file);
         buffer << myFile.rdbuf();
         myFile.close();
         auto xml = buffer.str();
         if (xmlDoc.Parse(xml.c_str()))
         {
            ShowError("Error parsing POV XML file");
            return;
         }
         auto root = xmlDoc.FirstChildElement("POV");
         if (root == nullptr)
         {
            ShowError("Error parsing POV XML file: root 'POV' element is missing");
            xmlDoc.Clear();
            return;
         }
         #define POV_FIELD(name, settingField, field) \
         { \
            auto node = section->FirstChildElement(name); \
            if (node != nullptr) \
            { \
               float value; sscanf_s(node->GetText(), "%f", &value); \
               if (toUserSettings) \
                  m_settings.SaveValue(Settings::TableOverride, keyPrefix + settingField, value); \
               else \
                  field = value; \
            } \
         }
         static const string sections[] = { "desktop"s, "fullscreen"s, "fullsinglescreen"s };
         for (int i = 0; i < 3; i++)
         {
            auto section = root->FirstChildElement(sections[i].c_str());
            if (section)
            {
               const string &keyPrefix = vsPrefix[i];
               POV_FIELD("inclination", "LookAt", mViewSetups[i].mLookAt);
               POV_FIELD("fov", "FOV", mViewSetups[i].mFOV);
               POV_FIELD("layback", "Layback", mViewSetups[i].mLayback);
               POV_FIELD("lookat", "LookAt", mViewSetups[i].mLookAt);
               POV_FIELD("rotation", "Rotation", mViewSetups[i].mViewportRotation);
               POV_FIELD("xscale", "ScaleX", mViewSetups[i].mSceneScaleX);
               POV_FIELD("yscale", "ScaleY", mViewSetups[i].mSceneScaleY);
               POV_FIELD("zscale", "ScaleZ", mViewSetups[i].mSceneScaleZ);
               POV_FIELD("xoffset", "PlayerX", mViewSetups[i].mViewX);
               POV_FIELD("yoffset", "PlayerY", mViewSetups[i].mViewY);
               POV_FIELD("zoffset", "PlayerZ", mViewSetups[i].mViewZ);
               POV_FIELD("ViewHOfs", "HOfs", mViewSetups[i].mViewHOfs);
               POV_FIELD("ViewVOfs", "VOfs", mViewSetups[i].mViewVOfs);
               POV_FIELD("WindowTopZOfs", "WindowTop", mViewSetups[i].mWindowTopZOfs);
               POV_FIELD("WindowBottomZOfs", "WindowBot", mViewSetups[i].mWindowBottomZOfs);
               {
                  auto node = section->FirstChildElement("LayoutMode");
                  if (node != nullptr)
                  {
                     int value;
                     sscanf_s(node->GetText(), "%i", &value);
                     if (toUserSettings)
                        m_settings.SaveValue(Settings::TableOverride, keyPrefix + "Mode", value);
                     else
                        mViewSetups[i].mMode = (ViewLayoutMode) value;
                  }
               }
            }
         }
         #undef POV_FIELD
         if (toUserSettings)
         {
            auto section = root->FirstChildElement("customsettings");
            if (section)
            {
               // POV_FIELD("SSAA", "%i", m_useAA);
               {
                  auto node = section->FirstChildElement("postprocAA");
                  if (node != nullptr)
                  {
                     int useAA;
                     sscanf_s(node->GetText(), "%i", &useAA);
                     if (useAA > -1)
                        m_settings.SaveValue(Settings::Player, "AAFactor"s, useAA == 0 ? 1.f : 2.f);
                  }
               }
               // POV_FIELD("postprocAA", "%i", m_useFXAA);
               {
                  auto node = section->FirstChildElement("postprocAA");
                  if (node != nullptr)
                  {
                     int useFXAA;
                     sscanf_s(node->GetText(), "%i", &useFXAA);
                     if (useFXAA > -1)
                        m_settings.SaveValue(Settings::Player, "FXAA"s, useFXAA == 1 ? Standard_FXAA : Disabled);
                  }
               }
               //POV_FIELD("ingameAO", "%i", m_useAO);
               auto node = section->FirstChildElement("ingameAO");
               if (node)
               {
                  int ingameAO;
                  sscanf_s(node->GetText(), "%i", &ingameAO);
                  if (ingameAO != -1)
                     m_settings.SaveValue(Settings::Player, "DisableAO"s, ingameAO == 0);
               }
               //POV_FIELD("ScSpReflect", "%i", m_useSSR);
               node = section->FirstChildElement("ScSpReflect");
               if (node)
               {
                  int useSSR;
                  sscanf_s(node->GetText(), "%i", &useSSR);
                  if (useSSR != -1)
                     m_settings.SaveValue(Settings::Player, "SSRefl"s, useSSR != 0);
               }
               //POV_FIELD("FPSLimiter", "%i", m_TableAdaptiveVSync);
               node = section->FirstChildElement("FPSLimiter");
               if (node)
               {
                  int tableAdaptiveVSync;
                  sscanf_s(node->GetText(), "%i", &tableAdaptiveVSync);
                  if (tableAdaptiveVSync != -1)
                  {
                     switch (tableAdaptiveVSync)
                     {
                     case 0:
                        m_settings.SaveValue(Settings::Player, "MaxFramerate"s, 0);
                        m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_NONE);
                        break;
                     case 1:
                        m_settings.SaveValue(Settings::Player, "MaxFramerate"s, 0);
                        m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_VSYNC);
                        break;
                     case 2:
                        m_settings.SaveValue(Settings::Player, "MaxFramerate"s, 0);
                        m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_ADAPTIVE_VSYNC);
                        break;
                     default:
                        m_settings.SaveValue(Settings::Player, "MaxFramerate"s, tableAdaptiveVSync);
                        m_settings.SaveValue(Settings::Player, "SyncMode"s, VideoSyncMode::VSM_ADAPTIVE_VSYNC);
                        break;
                     }
                  }
               }
               //POV_FIELD("BallTrail", "%i", m_useTrailForBalls);
               node = section->FirstChildElement("BallTrail");
               if (node)
               {
                  int useTrailForBalls;
                  sscanf_s(node->GetText(), "%i", &useTrailForBalls);
                  if (useTrailForBalls != -1)
                     m_settings.SaveValue(Settings::Player, "BallTrail"s, useTrailForBalls == 1);
               }
               //POV_FIELD("BallTrailStrength", "%f", m_ballTrailStrength);
               node = section->FirstChildElement("BallTrailStrength");
               if (node)
               {
                  float strength;
                  sscanf_s(node->GetText(), "%f", &strength);
                  m_settings.SaveValue(Settings::Player, "BallTrailStrength"s, strength);
               }
               //int overwriteGlobalDetailLevel = (int)m_overwriteGlobalDetailLevel;
               //POV_FIELD("OverwriteDetailsLevel", "%i", overwriteGlobalDetailLevel);
               node = section->FirstChildElement("OverwriteDetailsLevel");
               if (node)
               {
                  int value;
                  sscanf_s(node->GetText(), "%i", &value);
                  if (value == 1)
                  {
                     //POV_FIELD("DetailsLevel", "%i", m_userDetailLevel);
                     node = section->FirstChildElement("DetailsLevel");
                     sscanf_s(node->GetText(), "%i", &value);
                     m_settings.SaveValue(Settings::Player, "AlphaRampAccuracy"s, value);
                  }
               }
               node = section->FirstChildElement("OverwriteNightDay");
               if (node)
               {
                  int value;
                  sscanf_s(node->GetText(), "%i", &value);
                  //m_overwriteGlobalDayNight = (value == 1);
                  if (value == 1)
                  {
                     m_settings.SaveValue(Settings::Player, "OverrideTableEmissionScale"s, true);
                     node = section->FirstChildElement("NightDayLevel");
                     if (node)
                     {
                        float valuef;
                        sscanf_s(node->GetText(), "%f", &valuef);
                        m_settings.SaveValue(Settings::Player, "EmissionScale"s, valuef / 100.f);
                     }
                  }
               }
               node = section->FirstChildElement("GameplayDifficulty");
               if (node)
               {
                  float value;
                  sscanf_s(node->GetText(), "%f", &value);
                  m_settings.SaveValue(Settings::TableOverride, "Difficulty"s, value / 100.f);
               }
               node = section->FirstChildElement("SoundVolume");
               if (node)
               {
                  int value;
                  sscanf_s(node->GetText(), "%i", &value);
                  m_settings.SaveValue(Settings::Player, "SoundVolume"s, value);
               }
               node = section->FirstChildElement("MusicVolume");
               if (node)
               {
                  int value;
                  sscanf_s(node->GetText(), "%i", &value);
                  m_settings.SaveValue(Settings::Player, "MusicVolume"s, value);
               }
               // FIXME these are the last 3 settings which were not ported to the setting API
               // - for physics set, since they can be applied at the part level, for each flipper
               // - for ball reflection, since I don't think that matters and there is no obvious way
               //POV_FIELD("BallReflection", "%i", m_useReflectionForBalls); // removed in 10.8
               /* node = section->FirstChildElement("PhysicsSet");
               if (node)
                  sscanf_s(node->GetText(), "%i", &m_overridePhysics);
               node = section->FirstChildElement("IncludeFlipperPhysics");
               if (node)
               {
                  int value;
                  sscanf_s(node->GetText(), "%i", &value);
                  m_overridePhysicsFlipper = (value == 1);
               } */
            }
         }
      }
      catch (...)
      {
         ShowError("Error parsing POV XML file");
      }
      xmlDoc.Clear();
   }

   // If loaded without UI interaction, do not mark settings as modified
   if (!filename.empty())
      m_settings.SetModified(wasModified);

   // update properties UI
   if (!toUserSettings)
      SetNonUndoableDirty(eSaveDirty);
   m_vpinball->SetPropSel(m_vmultisel);
}

// Select file and export the point of view definition
void PinTable::ExportBackdropPOV()
{
   string iniFileName;
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = m_vpinball->theInstance;
	ofn.hwndOwner = m_vpinball->GetHwnd();
	// TEXT
	ofn.lpstrFilter = "INI file(*.ini)\0*.ini\0";
	char szFileName[MAXSTRING];
	strncpy_s(szFileName, m_szFileName.c_str(), sizeof(szFileName)-1);
	const size_t idx = m_szFileName.find_last_of('.');
	if(idx != string::npos && idx < MAXSTRING)
		szFileName[idx] = '\0';
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrDefExt = "ini";
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	const int ret = GetSaveFileName(&ofn);
	// user canceled
	if (ret == 0)
		return;// S_FALSE;
	iniFileName = szFileName;

   // Save view setups (only overriden properties if we are given a reference view setup set)
   Settings settings;
   for (int i = 0; i < 3; i++)
      mViewSetups[i].SaveToTableOverrideSettings(settings, (ViewSetupID)i);
   settings.SaveToFile(iniFileName);

   if (settings.IsModified())
   {
      settings.SaveToFile(iniFileName);
      if (g_pplayer)
         g_pplayer->m_liveUI->PushNotification("POV exported to "s.append(iniFileName), 5000);
   }
   else if (g_pplayer)
   {
      g_pplayer->m_liveUI->PushNotification("POV was not exported to "s + iniFileName + " (nothing to save)", 5000);
   }

   PLOGI << "View setup exported to '" << iniFileName << '\'';
}

void PinTable::SelectItem(IScriptable *piscript)
{
   ISelect * const pisel = piscript->GetISelect();
   if (pisel)
      AddMultiSel(pisel, false, true, false);
}

void PinTable::DoCodeViewCommand(int command)
{
   switch (command)
   {
   case ID_SAVE: TableSave(); break;
   case ID_TABLE_PLAY: Play(0); break;
   case ID_TABLE_CAMERAMODE: Play(1); break;
   case ID_TABLE_LIVEEDIT: Play(2); break;
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
   const SaveDirtyState sdsNewDirtyState = (SaveDirtyState)max(max((int)m_sdsDirtyProp, (int)m_sdsDirtyScript), (int)m_sdsNonUndoableDirty);

   if (sdsNewDirtyState != m_sdsCurrentDirtyState)
   {
      if (sdsNewDirtyState > eSaveClean)
         SetCaption(m_szTitle + '*');
      else
         SetCaption(m_szTitle);
   }

   m_sdsCurrentDirtyState = sdsNewDirtyState;
}

bool PinTable::FDirty() const
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
   SetMyScrollInfo();

   if (m_searchSelectDlg.IsWindow())
      m_searchSelectDlg.Update();
}

void PinTable::Uncreate(IEditable *pie)
{
   if (pie->GetISelect()->m_selectstate != eNotSelected)
      AddMultiSel(pie->GetISelect(), true, true, false); // Remove the item from the multi-select list

   pie->GetISelect()->Uncreate();
   pie->Release();
}

void PinTable::Undelete(IEditable *pie)
{
   m_vedit.push_back(pie);
   pie->Undelete();
   SetDirtyDraw();
}

void PinTable::Copy(int x, int y)
{
   if (MultiSelIsEmpty()) // Can't copy table
      return;

   if (m_vmultisel.size() == 1)
   {
       // special check if the user selected a Control Point and wants to copy the coordinates
       ISelect *const pItem = HitTest(x, y);
       if (pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint *pPoint = (DragPoint*)pItem;
           pPoint->Copy();
           return;
       }
   }

   vector<IStream*> vstm;
   //m_vstmclipboard
   for (int i = 0; i < m_vmultisel.size(); i++)
   {
       const HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, 1);

       IStream *pstm;
       CreateStreamOnHGlobal(hglobal, TRUE, &pstm);

       IEditable * const pe = m_vmultisel[i].GetIEditable();

       ////////!! BUG!  With multi-select, if you have multiple dragpoints on
       //////// a surface selected, the surface will get copied multiple times
       const int type = pe->GetItemType();
       ULONG writ = 0;
       pstm->Write(&type, sizeof(int), &writ);

       pe->SaveData(pstm, NULL, false);

       vstm.push_back(pstm);
   }

   m_vpinball->SetClipboard(&vstm);
}

void PinTable::Paste(const bool atLocation, const int x, const int y)
{
   bool error = false;
   int cpasted = 0;

   if (m_vmultisel.size() == 1)
   {
       // User wants to paste the copied coordinates of a Control Point
       ISelect * const pItem = HitTest(x, y);
       if (pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint * const pPoint = (DragPoint*)pItem;
           pPoint->Paste();
           SetDirtyDraw();
           return;
       }
   }

   const unsigned viewflag = (m_vpinball->m_backglassView ? VIEW_BACKGLASS : VIEW_PLAYFIELD);

   // Do a backwards loop, so that the primary selection we had when
   // copying will again be the primary selection, since it will be
   // selected last.  Purely cosmetic.
   for (SSIZE_T i = m_vpinball->m_vstmclipboard.size() - 1; i >= 0; i--)
   //for (size_t i=0; i<m_vpinball->m_vstmclipboard.size(); i++)
   {
      IStream* const pstm = m_vpinball->m_vstmclipboard[i];

      // Go back to beginning of stream to load
      LARGE_INTEGER foo;
      foo.QuadPart = 0;
      pstm->Seek(foo, STREAM_SEEK_SET, nullptr);

      ULONG writ = 0;
      ItemTypeEnum type;
      /*const HRESULT hr =*/ pstm->Read(&type, sizeof(int), &writ);

      if (!(EditableRegistry::GetAllowedViews(type) & viewflag))
      {
         error = true;
      }
      else
      {
         IEditable *peditNew = EditableRegistry::Create(type);

         int id;
         peditNew->InitLoad(pstm, this, &id, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);

         if (type != eItemDecal)
         {
            GetUniqueNamePasting(type, peditNew->GetScriptable()->m_wzName, sizeof(peditNew->GetScriptable()->m_wzName)/sizeof(peditNew->GetScriptable()->m_wzName[0]));
            peditNew->InitVBA(fTrue, 0, peditNew->GetScriptable()->m_wzName);
         }

         peditNew->InitPostLoad();
         peditNew->m_backglass = m_vpinball->m_backglassView;

         peditNew->GetISelect()->m_layerName = m_vpinball->GetLayersListDialog()->GetCurrentSelectedLayerName();
         m_vpinball->GetLayersListDialog()->AddLayer(m_vpinball->GetLayersListDialog()->GetCurrentSelectedLayerName(), peditNew);

         m_vedit.push_back(peditNew);
         
         AddMultiSel(peditNew->GetISelect(), (i != m_vpinball->m_vstmclipboard.size() - 1), true, false);
         cpasted++;
      }
   }

   // Center view on newly created objects, if they are off the screen
   if ((cpasted > 0) && atLocation)
      Translate(TransformPoint(x, y) - GetCenter());

   if (error)
   {
      const LocalString ls(IDS_NOPASTEINVIEW);
      m_mdiTable->MessageBox(ls.m_szbuffer, "Visual Pinball", 0);
   }
}

void PinTable::UIRenderPass1(Sur * const psur)
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

HRESULT PinTable::InitVBA(BOOL fNew, int id, WCHAR * const wzName)
{
   return S_OK;
}

void PinTable::SetDefaults(const bool fromMouseClick)
{
}

void PinTable::SetDefaultPhysics(const bool fromMouseClick)
{
   m_Gravity = 0.97f*GRAVITYCONST;

   m_friction = DEFAULT_TABLE_CONTACTFRICTION;
   m_elasticity = DEFAULT_TABLE_ELASTICITY;
   m_elasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;
   m_scatter = DEFAULT_TABLE_PFSCATTERANGLE;
}

void PinTable::ClearMultiSel(ISelect* newSel)
{
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].m_selectstate = eNotSelected;

   //remove the clone of the multi selection in the smart browser class
   //to sync the clone and the actual multi-selection 
   //it will be updated again on AddMultiSel() call
   m_vmultisel.clear();

   if (newSel == nullptr)
      newSel = this;
   m_vmultisel.push_back(newSel);
   newSel->m_selectstate = eSelected;
}

bool PinTable::MultiSelIsEmpty()
{
   // empty selection means only the table itself is selected
   return (m_vmultisel.size() == 1 && m_vmultisel.ElementAt(0) == this);
}

// 'update' tells us whether to go ahead and change the UI
// based on the new selection, or whether more stuff is coming
// down the pipe (speeds up drag-selection)
void PinTable::AddMultiSel(ISelect *psel, const bool add, const bool update, const bool contextClick)
{
   const int index = m_vmultisel.find(psel);
   ISelect *piSelect = nullptr;
   //_ASSERTE(m_vmultisel[0].m_selectstate == eSelected);

   if (IsLocked())
      return;

   if (index == -1) // If we aren't selected yet, do that
   {
      _ASSERTE(psel->m_selectstate == eNotSelected);
      // If we non-shift click on an element outside the multi-select group, delete the old group
      // If the table is currently selected, deselect it - the table can not be part of a multi-select
      if (!add || MultiSelIsEmpty())
      {
         ClearMultiSel(psel);
         if (!add && !contextClick)
         {
            int colIndex = -1;
            int elemIndex = -1;
            if (GetCollectionIndex(psel, colIndex, elemIndex))
            {
               CComObject<Collection> *col = m_vcollection.ElementAt(colIndex);
               if (col->m_groupElements)
               {
                  for (int i = 0; i < col->m_visel.size(); i++)
                  {
                     col->m_visel[i].m_selectstate = eMultiSelected;
                     // current element is already in m_vmultisel. (ClearMultiSel(psel) added it)
                     if (col->m_visel.ElementAt(i) != psel)
                        m_vmultisel.push_back(&col->m_visel[i]);
                  }
               }
            }
         }
      }
      else
      {
         // Make this new selection the primary one for the group
         piSelect = m_vmultisel.ElementAt(0);
         if (piSelect != nullptr)
            piSelect->m_selectstate = eMultiSelected;
         m_vmultisel.insert(psel, 0);
      }

      psel->m_selectstate = eSelected;

      if (update)
         SetDirtyDraw();
   }
   else if (add) // Take the element off the list
   {
      _ASSERTE(psel->m_selectstate != eNotSelected);
      m_vmultisel.erase(index);
      psel->m_selectstate = eNotSelected;
      if (m_vmultisel.empty())
      {
         // Have to have something selected
         m_vmultisel.push_back((ISelect *)this);
      }
      // The main element might have changed
      piSelect = m_vmultisel.ElementAt(0);
      if (piSelect != nullptr)
         piSelect->m_selectstate = eSelected;

      if (update)
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
         if (piSelect != nullptr)
            piSelect->m_selectstate = eMultiSelected;
         m_vmultisel.erase(index);
         m_vmultisel.insert(psel, 0);

         psel->m_selectstate = eSelected;
      }
      else
         ClearMultiSel(psel);

      if (update)
         SetDirtyDraw();
   }

   if (update)
   {
       m_vpinball->SetPropSel(m_vmultisel);
       m_vmultisel[0].UpdateStatusBarInfo();
   }

    piSelect = m_vmultisel.ElementAt(0);
    if (piSelect && piSelect->GetIEditable() && piSelect->GetIEditable()->GetScriptable())
    {
        string info = "Layer: " + piSelect->m_layerName;
        if (piSelect->GetItemType() == eItemPrimitive)
        {
            const Primitive *const prim = (Primitive *)piSelect;
            if (!prim->m_mesh.m_animationFrames.empty())
                info += " (animated " + std::to_string((unsigned long long)prim->m_mesh.m_animationFrames.size() - 1) + " frames)";
        }
        m_vpinball->SetStatusBarElementInfo(info);
        m_pcv->SelectItem(piSelect->GetIEditable()->GetScriptable());
        // Update the layer dialog
        m_vpinball->GetLayersListDialog()->UpdateLayerInfo();

    }
}

void PinTable::RefreshProperties()
{
    m_vpinball->SetPropSel(m_vmultisel);
}

void PinTable::OnDelete()
{
   vector<ISelect*> m_vseldelete;
   m_vseldelete.reserve(m_vmultisel.size());

   for (int i = 0; i < m_vmultisel.size(); i++)
   {
      // Can't delete these items yet - ClearMultiSel() will try to mark them as unselected
      m_vseldelete.push_back(m_vmultisel.ElementAt(i));
   }

   ClearMultiSel();

   bool inCollection = false;
   for (size_t t = 0; t < m_vseldelete.size() && !inCollection; t++)
   {
      const ISelect * const ptr = m_vseldelete[t];
      for (int i = 0; i < m_vcollection.size() && !inCollection; i++)
      {
         for (int k = 0; k < m_vcollection[i].m_visel.size(); k++)
         {
            // Identify Editable in collection, as well as sub part of collection's editable (like light center for example)
            if (ptr == m_vcollection[i].m_visel.ElementAt(k) || ptr->GetIEditable() == m_vcollection[i].m_visel.ElementAt(k)->GetIEditable())
            {
               inCollection = true;
               break;
            }
         }
      }
   }
   if (inCollection)
   {
      const LocalString ls(IDS_DELETE_ELEMENTS);
      const int ans = m_mdiTable->MessageBox(ls.m_szbuffer/*"Selected elements are part of one or more collections.\nDo you really want to delete them?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
      if (ans != IDYES)
         return;
   }

   for (size_t i = 0; i < m_vseldelete.size(); i++)
   {
      ISelect* pisel = m_vseldelete[i];
      if (pisel != nullptr)
      {
         if(pisel->GetItemType()!=eItemDragPoint)
            m_vpinball->GetLayersListDialog()->DeleteElement(m_vseldelete[i]->GetIEditable());
         m_vseldelete[i]->Delete();
      }
   }
   // update properties to show the properties of the table
   m_vpinball->SetPropSel(m_vmultisel);
   if (m_searchSelectDlg.IsWindow())
      m_searchSelectDlg.Update();

   SetDirtyDraw();
}

void PinTable::OnKeyDown(int key)
{
   const int shift = GetKeyState(VK_SHIFT) & 0x8000;
   //const int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
   //const int alt = GetKeyState(VK_MENU) & 0x8000;

   switch (key)
   {
   case VK_DELETE: OnDelete(); break;

   case VK_LEFT:
   case VK_RIGHT:
   case VK_UP:
   case VK_DOWN:
   {
      BeginUndo();
      const float distance = shift ? 10.f : 1.f;
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect *const pisel = m_vmultisel.ElementAt(i);
         if (!pisel->GetIEditable()->GetISelect()->m_locked) // control points get lock info from parent - UNDONE - make this code snippet be in one place
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
      }
      EndUndo();
      SetDirtyDraw();
   }
   break;
   }
}

void PinTable::UseTool(int x, int y, int tool)
{
   const Vertex2D v = TransformPoint(x, y);

   const ItemTypeEnum type = EditableRegistry::TypeFromToolID(tool);
   IEditable * const pie = EditableRegistry::CreateAndInit(type, this, v.x, v.y);

   if (pie)
   {
      pie->m_backglass = m_vpinball->m_backglassView;
      m_vedit.push_back(pie);
      pie->GetISelect()->m_layerName = m_vpinball->GetLayersListDialog()->GetCurrentSelectedLayerName();

      if (m_searchSelectDlg.IsWindow())
         m_searchSelectDlg.Update();
      m_vpinball->GetLayersListDialog()->AddLayer(m_vpinball->GetLayersListDialog()->GetCurrentSelectedLayerName(), pie);
      BeginUndo();
      m_undo.MarkForCreate(pie);
      EndUndo();
      AddMultiSel(pie->GetISelect(), false, true, false);
   }

   m_vpinball->ParseCommand(IDC_SELECT, false);
}

Vertex2D PinTable::TransformPoint(int x, int y) const
{
   const CRect rc = GetClientRect();

   const HitSur phs(nullptr, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

   const Vertex2D result = phs.ScreenToSurface(x, y);

   return result;
}

void PinTable::OnLButtonDown(int x, int y)
{
   const Vertex2D v = TransformPoint(x, y);

   m_rcDragRect.left = v.x;
   m_rcDragRect.right = v.x;
   m_rcDragRect.top = v.y;
   m_rcDragRect.bottom = v.y;

   m_dragging = true;

   SetCapture();

   SetDirtyDraw();
}

void PinTable::OnLButtonUp(int x, int y)
{
   if (m_dragging)
   {
      m_dragging = false;
      ReleaseCapture();
      if ((m_rcDragRect.left != m_rcDragRect.right) || (m_rcDragRect.top != m_rcDragRect.bottom))
      {
         vector<ISelect*> vsel;

         const CDC &dc = m_mdiTable->GetDC();

         const CRect rc = m_mdiTable->GetClientRect();

         HitRectSur * const phrs = new HitRectSur(dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, &m_rcDragRect, &vsel);

         // Just want one rendering pass (no UIRenderPass1) so we don't select things twice
         UIRenderPass2(phrs);

         const int ksshift = GetKeyState(VK_SHIFT);
         const bool add = ((ksshift & 0x80000000) != 0);
         if (!add)
            ClearMultiSel();

         int minlevel = INT_MAX;

         for(const auto &ptr : vsel)
            minlevel = min(minlevel, ptr->GetSelectLevel());

         if (!vsel.empty())
         {
            size_t lastItemForUpdate = -1;
            // first check which item is the last item to add to the multi selection
            for (size_t i = 0; i < vsel.size(); i++)
               if (vsel[i]->GetSelectLevel() == minlevel)
                  lastItemForUpdate = i;

            for (size_t i = 0; i < vsel.size(); i++)
               if (vsel[i]->GetSelectLevel() == minlevel)
                  AddMultiSel(vsel[i], true, (i == lastItemForUpdate), false); //last item updates the (multi-)selection in the editor
         }

         delete phrs;
      }
   }
   SetDirtyDraw();
}

void PinTable::OnMouseMove(int x, int y)
{
   const Vertex2D v = TransformPoint(x, y);

   m_rcDragRect.right = v.x;
   m_rcDragRect.bottom = v.y;

   if (m_dragging)
      SetDirtyDraw();
}

HRESULT PinTable::GetTypeName(BSTR *pVal)
{
   const int stringid = (!m_vpinball->m_backglassView) ? IDS_TABLE : IDS_TB_BACKGLASS;

   const LocalStringW ls(stringid);
   *pVal = SysAllocString(ls.m_szbuffer);

   return S_OK;
}

STDMETHODIMP PinTable::get_FileName(BSTR *pVal)
{
   const WCHAR * const wz = MakeWide(m_szTitle);
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

   const int l = lstrlenW(newVal);
   if ((l > MAXNAMEBUFFER) || (l < 1))
   {
      return E_FAIL;
   }

   STARTUNDO
   if(m_pcv->ReplaceName((IScriptable *)this, newVal) == S_OK)
   {
      WideStrNCopy(newVal, (WCHAR *)m_wzName, MAXNAMEBUFFER);
      //lstrcpyW((WCHAR *)m_wzName, newVal);
   }
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_MaxSeparation(float *pVal)
{

    *pVal = GetMaxSeparation();

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
    *pVal = GetZPD();
   return S_OK;
}

STDMETHODIMP PinTable::put_ZPD(float newVal)
{
   STARTUNDO
   SetZPD(newVal);
   STOPUNDO

   return S_OK;
}

void PinTable::Set3DOffset(const float value)
{
    if (m_overwriteGlobalStereo3D)
        m_3DOffset = value;
}

STDMETHODIMP PinTable::get_Offset(float *pVal)
{
   *pVal = Get3DOffset();
   return S_OK;
}

STDMETHODIMP PinTable::put_Offset(float newVal)
{
   STARTUNDO
   Set3DOffset(newVal);
   STOPUNDO

   return S_OK;
}

HRESULT PinTable::StopSound(BSTR Sound)
{
   char szName[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, Sound, -1, szName, MAXSTRING, nullptr, nullptr);

   // In case we were playing any of the main buffers
   for (size_t i = 0; i < m_vsound.size(); i++)
      if (!lstrcmpi(m_vsound[i]->m_szName.c_str(), szName))
      {
         m_vsound[i]->Stop();
         break;
      }

   m_vpinball->m_ps.StopCopiedWav(szName);

   return S_OK;
}

void PinTable::StopAllSounds()
{
   // In case we were playing any of the main buffers
   for (size_t i = 0; i < m_vsound.size(); i++)
      m_vsound[i]->Stop();

   m_vpinball->m_ps.StopCopiedWavs();
}


STDMETHODIMP PinTable::PlaySound(BSTR bstr, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade)
{
   char szName[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, szName, MAXSTRING, nullptr, nullptr);

   if (!lstrcmpi("knock", szName) || !lstrcmpi("knocker", szName))
      hid_knock();

   size_t i;
   for (i = 0; i < m_vsound.size(); i++)
      if (!lstrcmpi(m_vsound[i]->m_szName.c_str(), szName))
         break;

   if (i == m_vsound.size()) // did not find it
   {
      if (szName[0] && m_pcv && g_pplayer && g_pplayer->m_hwndDebugOutput)
      {
         const string logmsg = "Request to play \""s + szName + "\", but sound not found.";
         m_pcv->AddToDebugOutput(logmsg.c_str());
      }
      return S_OK;
   }

   PinSound * const pps = m_vsound[i];

   volume += dequantizeSignedPercent(pps->m_volume);
   pan += dequantizeSignedPercent(pps->m_balance);
   front_rear_fade += dequantizeSignedPercent(pps->m_fade);
   
   if (m_tblMirrorEnabled)
      pan = -pan;

   m_vpinball->m_ps.Play(pps, volume * m_TableSoundVolume * (float)g_pplayer->m_SoundVolume, randompitch, pitch, pan, front_rear_fade, loopcount, VBTOb(usesame), VBTOb(restart));

   return S_OK;
}

RenderProbe *PinTable::GetRenderProbe(const string &szName) const
{
   if (szName.empty())
      return nullptr;

   // during playback, we use the hashtable for lookup
   if (!m_renderprobeMap.empty())
   {
      const robin_hood::unordered_map<string, RenderProbe *, StringHashFunctor, StringComparator>::const_iterator it = m_renderprobeMap.find(szName);
      if (it != m_renderprobeMap.end())
         return it->second;
      else
         return nullptr;
   }

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
      if (!lstrcmpi(m_vrenderprobe[i]->GetName().c_str(), szName.c_str()))
         return m_vrenderprobe[i];

   return nullptr;
}

Light *PinTable::GetLight(const string &szName) const
{
   if (szName.empty())
      return nullptr;

   // during playback, we use the hashtable for lookup
   if (!m_lightMap.empty())
   {
      const robin_hood::unordered_map<string, Light *, StringHashFunctor, StringComparator>::const_iterator it = m_lightMap.find(szName);
      if (it != m_lightMap.end())
         return it->second;
      else
         return nullptr;
   }

   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable *const pe = m_vedit[i];
      if (pe->GetItemType() == ItemTypeEnum::eItemLight && !lstrcmpi(pe->GetName(), szName.c_str()))
         return (Light *)pe;
   }

   return nullptr;
}

Texture* PinTable::GetImage(const string &szName) const
{
   if (szName.empty())
      return nullptr;

   // during playback, we use the hashtable for lookup
   if (!m_textureMap.empty())
   {
      const robin_hood::unordered_map<string, Texture*, StringHashFunctor, StringComparator>::const_iterator
         it = m_textureMap.find(szName);
      if (it != m_textureMap.end())
         return it->second;
      else
         return nullptr;
   }

   for (size_t i = 0; i < m_vimage.size(); i++)
      if (!lstrcmpi(m_vimage[i]->m_szName.c_str(), szName.c_str()))
         return m_vimage[i];

   return nullptr;
}

bool PinTable::ExportImage(const Texture * const ppi, const char * const szfilename)
{
   if (ppi->m_ppb != nullptr)
      return ppi->m_ppb->WriteToFile(szfilename);
   else if (ppi->m_pdsBuffer != nullptr)
   {
#if 0
      HANDLE hFile = CreateFile(szfilename, GENERIC_WRITE, FILE_SHARE_READ,
         nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

      if (hFile == INVALID_HANDLE_VALUE)
         return false;

      const unsigned int surfwidth  = ppi->m_width;            // texture width
      const unsigned int surfheight = ppi->m_height;           // and height

      const unsigned int bmplnsize = (surfwidth * 4 + 3) & -4; // line size ... 4 bytes per pixel + pad to 4 byte boundary		

      //<<<< began bmp file header and info <<<<<<<<<<<<<<<

      BITMAPFILEHEADER bmpf;		// file header
      bmpf.bfType = 'MB';
      bmpf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + surfheight*bmplnsize;
      bmpf.bfReserved1 = 0;
      bmpf.bfReserved2 = 0;
      bmpf.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

      DWORD write;
      // write BMP file header	
      WriteFile(hFile, &bmpf, sizeof(BITMAPFILEHEADER), &write, nullptr);

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
      WriteFile(hFile, &bmpi, sizeof(BITMAPINFOHEADER), &write, nullptr);

      unsigned char* const sinfo = new unsigned char[bmplnsize + 4]; //linebuffer and safety pad
      if (!sinfo)
      {
         CloseHandle(hFile);
         return false;
      }

      unsigned char* info;
      for (info = sinfo + surfwidth * 3; info < sinfo + bmplnsize; *info++ = 0); //fill padding with 0			

      const unsigned int pitch = ppi->m_pdsBuffer->pitch();
      const BYTE *spch = ppi->m_pdsBuffer->data() + (surfheight * pitch); // just past the end of the Texture part of DD surface

      for (unsigned int i = 0; i < surfheight; i++)
      {
         const BYTE * const pch = (spch -= pitch); // start on previous previous line
         memcpy(sinfo, pch, surfwidth*4);

         WriteFile(hFile, sinfo, bmplnsize, &write, nullptr);
         GetLastError();
      }

      delete[] sinfo;
      CloseHandle(hFile);
#else
      if (ppi->m_pdsBuffer->m_format == BaseTexture::RGB_FP16 || ppi->m_pdsBuffer->m_format == BaseTexture::RGB_FP32)
      {
          assert(!"float format export");
          return false; // Unsupported but this should not happen since all HDR images are imported and have a m_ppb field
      }

      FIBITMAP *dib = FreeImage_Allocate(ppi->m_width, ppi->m_height, ppi->m_pdsBuffer->has_alpha() ? 32 : 24);
      BYTE *const psrc = FreeImage_GetBits(dib);

      const unsigned int pitch = ppi->m_pdsBuffer->pitch();
      const unsigned int pitch_dst = FreeImage_GetPitch(dib);
      const BYTE *spch = ppi->m_pdsBuffer->data() + (ppi->m_height * pitch); // just past the end of the Texture part of DD surface
      const unsigned int ch = ppi->m_pdsBuffer->has_alpha() ? 4 : 3;

      for (unsigned int i = 0; i < ppi->m_height; i++)
      {
         const BYTE *__restrict src = (spch -= pitch); // start on previous previous line
         BYTE * __restrict dst = psrc + i * pitch_dst;
         for (unsigned int x = 0; x < ppi->m_width; x++,src+=ch,dst+=ch) // copy and swap red & blue
         {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            if (ppi->m_pdsBuffer->has_alpha())
               dst[3] = src[3];
         }
      }

      const FREE_IMAGE_FORMAT fiff = FreeImage_GetFIFFromFilename(szfilename);
      int flags;
      switch (fiff)
      {
      case FIF_BMP: flags = BMP_SAVE_RLE; break;
      case FIF_JPEG: flags = JPEG_QUALITYGOOD | JPEG_OPTIMIZE; break;
      case FIF_PNG: flags = PNG_Z_BEST_COMPRESSION; break;
      case FIF_TARGA: flags = TARGA_SAVE_RLE; break;
      case FIF_WEBP: flags = WEBP_DEFAULT; break; // WEBP_LOSSLESS
      default: flags = 0; break;
      }
      if (!FreeImage_Save(fiff, dib, szfilename, flags))
          m_vpinball->MessageBox("Export failed!", "BMP Export", MB_OK | MB_ICONEXCLAMATION);
      //else
      //   m_vpinball->MessageBox("Export finished!", "BMP Export", MB_OK);
      FreeImage_Unload(dib);
#endif
      return true;
   }
   return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>>

Texture *PinTable::ImportImage(const string &filename, const string &imagename)
{
   bool isUpdate = true;
   Texture *ppi = nullptr;
   if (!imagename.empty())
      ppi = GetImage(imagename);
   if (ppi == nullptr)
   {
      ppi = new Texture();
      isUpdate = false;
   }
   ppi->LoadFromFile(filename, imagename.empty());
   if (ppi->m_pdsBuffer == nullptr)
   {
      if (!isUpdate)
         delete ppi;
      return nullptr;
   }
   if (!isUpdate)
   {
      m_textureMap[imagename.empty() ? ppi->m_szName : imagename] = ppi;
      m_vimage.push_back(ppi);
      if (m_isLiveInstance)
         m_vliveimage.push_back(ppi);
   }
   return ppi;
}

void PinTable::ListImages(HWND hwndListView)
{
   for (size_t i = 0; i < m_vimage.size(); i++)
      AddListImage(hwndListView, m_vimage[i]);
}

int PinTable::AddListImage(HWND hwndListView, Texture * const ppi)
{
   char sizeString[MAXTOKEN];
   constexpr char usedStringYes[] = "X";
   constexpr char usedStringNo[] = " ";

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)ppi->m_szName.c_str();
   lvitem.lParam = (size_t)ppi;

   if (ppi->m_realWidth == ppi->m_width && ppi->m_realHeight == ppi->m_height)
      _snprintf_s(sizeString, MAXTOKEN - 1, "%ix%i", ppi->m_realWidth, ppi->m_realHeight);
   else if (ppi->m_realWidth > ppi->m_width || ppi->m_realHeight > ppi->m_height)
      _snprintf_s(sizeString, MAXTOKEN - 1, "%ix%i downsized to %ix%i", ppi->m_realWidth, ppi->m_realHeight, ppi->m_width, ppi->m_height);
   else
      _snprintf_s(sizeString, MAXTOKEN - 1, "%ix%i upscaled to %ix%i", ppi->m_realWidth, ppi->m_realHeight, ppi->m_width, ppi->m_height);
   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, (LPSTR)ppi->m_szPath.c_str());
   ListView_SetItemText(hwndListView, index, 2, sizeString);
   ListView_SetItemText(hwndListView, index, 3, (LPSTR)usedStringNo);

   char *const sizeConv = StrFormatByteSize64(ppi->m_pdsBuffer->height() * ppi->m_pdsBuffer->pitch(), sizeString, MAXTOKEN);
   ListView_SetItemText(hwndListView, index, 4, sizeConv);

   if (ppi->m_pdsBuffer == nullptr)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"-");
   }
   else if (ppi->m_pdsBuffer->m_format == BaseTexture::SRGB)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"sRGB");
   }
   else if (ppi->m_pdsBuffer->m_format == BaseTexture::SRGBA)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"sRGBA");
   }
   else if (ppi->m_pdsBuffer->m_format == BaseTexture::RGB)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"RGB");
   }
   else if (ppi->m_pdsBuffer->m_format == BaseTexture::RGBA)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"RGBA");
   }
   else if (ppi->m_pdsBuffer->m_format == BaseTexture::RGB_FP16)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"RGB 16F");
   }
   else if (ppi->m_pdsBuffer->m_format == BaseTexture::RGB_FP32)
   {
      ListView_SetItemText(hwndListView, index, 5, (LPSTR)"RGB 32F");
   }
   else
      assert(!"unknown format");

   if ((lstrcmpi(m_image.c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_ballImage.c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_ballImageDecal.c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_envImage.c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_BG_image[BG_DESKTOP].c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_BG_image[BG_FSS].c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_BG_image[BG_FULLSCREEN].c_str(), ppi->m_szName.c_str()) == 0)
    || (lstrcmpi(m_imageColorGrade.c_str(), ppi->m_szName.c_str()) == 0))
   {
       ListView_SetItemText(hwndListView, index, 3, (LPSTR)usedStringYes);
   }
   else
   {
       for (size_t i = 0; i < m_vedit.size(); i++)
       {
           bool inUse = false;
           IEditable * const pEdit = m_vedit[i];
           if (pEdit == nullptr)
               continue;

           switch (pEdit->GetItemType())
           {
               case eItemDispReel:
               {
                   const DispReel * const pReel = (DispReel*)pEdit;
                   if(lstrcmpi(pReel->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemPrimitive:
               {
                   const Primitive * const pPrim = (Primitive*)pEdit;
                   if ((lstrcmpi(pPrim->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0) || (lstrcmpi(pPrim->m_d.m_szNormalMap.c_str(), ppi->m_szName.c_str()) == 0))
                       inUse = true;
                   break;
               }
               case eItemRamp:
               {
                   const Ramp * const pRamp = (Ramp*)pEdit;
                   if (lstrcmpi(pRamp->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemSurface:
               {
                   const Surface * const pSurf = (Surface*)pEdit;
                   if ((lstrcmpi(pSurf->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0) || (lstrcmpi(pSurf->m_d.m_szSideImage.c_str(), ppi->m_szName.c_str()) == 0))
                       inUse = true;
                   break;
               }
               case eItemDecal:
               {
                   const Decal * const pDecal = (Decal*)pEdit;
                   if (lstrcmpi(pDecal->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemFlasher:
               {
                   const Flasher * const pFlash = (Flasher*)pEdit;
                   if ((lstrcmpi(pFlash->m_d.m_szImageA.c_str(), ppi->m_szName.c_str()) == 0) || (lstrcmpi(pFlash->m_d.m_szImageB.c_str(), ppi->m_szName.c_str()) == 0))
                       inUse = true;
                   break;
               }
               case eItemFlipper:
               {
                   const Flipper * const pFlip = (Flipper*)pEdit;
                   if (lstrcmpi(pFlip->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemHitTarget:
               {
                   const HitTarget * const pHit = (HitTarget*)pEdit;
                   if (lstrcmpi(pHit->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemLight:
               {
                   const Light * const pLight = (Light*)pEdit;
                   if (lstrcmpi(pLight->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemPlunger:
               {
                   const Plunger * const pPlung = (Plunger*)pEdit;
                   if (lstrcmpi(pPlung->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemRubber:
               {
                   const Rubber * const pRub = (Rubber*)pEdit;
                   if (lstrcmpi(pRub->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               case eItemSpinner:
               {
                   const Spinner * const pSpin = (Spinner*)pEdit;
                   if (lstrcmpi(pSpin->m_d.m_szImage.c_str(), ppi->m_szName.c_str()) == 0)
                       inUse = true;
                   break;
               }
               default:
               {
                   break;
               }
           }

           if (inUse)
           {
               ListView_SetItemText(hwndListView, index, 3, (LPSTR)usedStringYes);
               break;
           }
       }//for
   }//else
   return index;
}

void PinTable::RemoveImage(Texture * const ppi)
{
   RemoveFromVectorSingle(m_vimage, ppi);

   delete ppi;
}

void PinTable::ListMaterials(HWND hwndListView)
{
   for (size_t i = 0; i < m_materials.size(); i++)
      AddListMaterial(hwndListView, m_materials[i]);
}

bool PinTable::IsMaterialNameUnique(const string &name) const
{
   for (size_t i = 0; i < m_materials.size(); i++)
      if(m_materials[i]->m_szName==name)
         return false;

   return true;
}


Material* PinTable::GetMaterial(const string &szName) const
{
   if (szName.empty())
      return &m_vpinball->m_dummyMaterial;

   // during playback, we use the hashtable for lookup
   if (!m_materialMap.empty())
   {
      const robin_hood::unordered_map<string, Material*, StringHashFunctor, StringComparator>::const_iterator
         it = m_materialMap.find(szName);
      if (it != m_materialMap.end())
         return it->second;
      else
         return &m_vpinball->m_dummyMaterial;
   }

   for (size_t i = 0; i < m_materials.size(); i++)
      if(m_materials[i]->m_szName==szName)
         return m_materials[i];

   return &m_vpinball->m_dummyMaterial;
}

void PinTable::AddMaterial(Material * const pmat)
{
   if (pmat->m_szName.empty() || pmat->m_szName == "dummyMaterial")
      pmat->m_szName = "Material";

   if (!IsMaterialNameUnique(pmat->m_szName) || pmat->m_szName == "Material")
   {
      int suffix = 1;
      char textBuf[MAXNAMEBUFFER];
      do
      {
         sprintf_s(textBuf, sizeof(textBuf), "%s%i", pmat->m_szName.c_str(), suffix);
         suffix++;
      } while (!IsMaterialNameUnique(textBuf));
      pmat->m_szName = textBuf;
   }

   m_materials.push_back(pmat);
}

int PinTable::AddListMaterial(HWND hwndListView, Material * const pmat)
{
   constexpr char usedStringYes[] = "X";
   constexpr char usedStringNo[] = " ";

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)pmat->m_szName.c_str();
   lvitem.lParam = (size_t)pmat;

   const int index = ListView_InsertItem(hwndListView, &lvitem);
   ListView_SetItemText(hwndListView, index, 1, (LPSTR)usedStringNo);
   if(pmat->m_szName == m_playfieldMaterial)
   {
      ListView_SetItemText(hwndListView, index, 1, (LPSTR)usedStringYes);
   }
   else
   {
      for (size_t i = 0; i < m_vedit.size(); i++)
      {
         bool inUse = false;
         IEditable * const pEdit = m_vedit[i];
         if (pEdit == nullptr)
            continue;

         switch (pEdit->GetItemType())
         {
         case eItemPrimitive:
         {
            const Primitive * const pPrim = (Primitive*)pEdit;
            if ((lstrcmpi(pPrim->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pPrim->m_d.m_szPhysicsMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemRamp:
         {
            const Ramp * const pRamp = (Ramp*)pEdit;
            if ((lstrcmpi(pRamp->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pRamp->m_d.m_szPhysicsMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemSurface:
         {
            const Surface * const pSurf = (Surface*)pEdit;
            if ((lstrcmpi(pSurf->m_d.m_szPhysicsMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pSurf->m_d.m_szSideMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pSurf->m_d.m_szTopMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pSurf->m_d.m_szSlingShotMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemDecal:
         {
            const Decal * const pDecal = (Decal*)pEdit;
            if ((lstrcmpi(pDecal->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemFlipper:
         {
            const Flipper * const pFlip = (Flipper*)pEdit;
            if ((lstrcmpi(pFlip->m_d.m_szRubberMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pFlip->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemHitTarget:
         {
            const HitTarget * const pHit = (HitTarget*)pEdit;
            if ((lstrcmpi(pHit->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pHit->m_d.m_szPhysicsMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemPlunger:
         {
            const Plunger * const pPlung = (Plunger*)pEdit;
            if (lstrcmpi(pPlung->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0)
               inUse = true;
            break;
         }
         case eItemSpinner:
         {
            const Spinner * const pSpin = (Spinner*)pEdit;
            if (lstrcmpi(pSpin->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0)
               inUse = true;
            break;
         }
         case eItemRubber:
         {
            const Rubber * const pRub = (Rubber*)pEdit;
            if ((lstrcmpi(pRub->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pRub->m_d.m_szPhysicsMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemBumper:
         {
            const Bumper * const pBump = (Bumper*)pEdit;
            if ((lstrcmpi(pBump->m_d.m_szCapMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pBump->m_d.m_szBaseMaterial.c_str(), pmat->m_szName.c_str()) == 0) ||
                (lstrcmpi(pBump->m_d.m_szSkirtMaterial.c_str(), pmat->m_szName.c_str()) == 0) || (lstrcmpi(pBump->m_d.m_szRingMaterial.c_str(), pmat->m_szName.c_str()) == 0))
               inUse = true;
            break;
         }
         case eItemKicker:
         {
            const Kicker * const pKick = (Kicker*)pEdit;
            if (lstrcmpi(pKick->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0)
               inUse = true;
            break;
         }
         case eItemTrigger:
         {
            const Trigger * const pTrig = (Trigger*)pEdit;
            if (lstrcmpi(pTrig->m_d.m_szMaterial.c_str(), pmat->m_szName.c_str()) == 0)
               inUse = true;
            break;
         }
         default:
            break;
         }

         if (inUse)
         {
            ListView_SetItemText(hwndListView, index, 1, (LPSTR)usedStringYes);
            break;
         }
      }//for
   }
   return index;
}

void PinTable::RemoveMaterial(Material * const pmat)
{
   RemoveFromVectorSingle(m_materials, pmat);

   delete pmat;
}

bool PinTable::GetImageLink(const Texture * const ppi) const
{
   return (!lstrcmpi(ppi->m_szName.c_str(), m_szScreenShot.c_str()));
}

PinBinary *PinTable::GetImageLinkBinary(const int id)
{
   switch (id)
   {
   case 1: //Screenshot
      // Transfer ownership of the screenshot pinbary blob to the image
      PinBinary * const pbT = m_pbTempScreenshot;
      m_pbTempScreenshot = nullptr;
      return pbT;
      break;
   }

   return nullptr;
}

class InfoDialog : public CDialog
{
public:
   InfoDialog(const string &message)
      : CDialog(IDD_INFOTEXT)
      , m_message(message)
   {
   }

   virtual BOOL OnInitDialog() override
   {
      SetDlgItemText(IDC_INFOTEXT_EDIT, m_message.c_str());
	   return TRUE;
   }

   string m_message;
};


void PinTable::AuditTable() const
{
   // Perform a simple table audit (disable lighting vs static, script reference of static parts, png vs webp, hdr vs exr,...)
   std::stringstream ss;

   // Ultra basic parser to get a (somewhat) valid list of referenced parts
   const size_t cchar = SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   char * const szText = new char[cchar + 1];
   SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);
   const char *wordStart = nullptr;
   const char *wordPos = szText;
   string inClass;
   bool nextIsFunc = false, nextIsEnd = false, nextIsClass = false, isInString = false, isInComment = false;
   vector<string> functions, identifiers;
   int line = 0;
   while (wordPos[0] != '\0')
   {
      if (isInComment)
      {
         // skip
      }
      else if (wordPos[0] == '"')
         isInString = !isInString;
      else if (!isInString)
      {
         // Detect comments
         if (wordPos[0] == '\'')
            isInComment = true;
         // Split identifiers (eventually class/function identifier)
         else if (wordPos[0] == ' ' || wordPos[0] == '\r' || wordPos[0] == '\t' || wordPos[0] == '\n' || wordPos[0] == '.' 
               || wordPos[0] == ':' || wordPos[0] == '('  || wordPos[0] == ')'  || wordPos[0] == '['  || wordPos[0] == ']')
         {
            if (wordStart)
            {
               string word(wordStart, (int)(wordPos - wordStart));
               StrToLower(word);
               if (word == "end" || word == "exit")
               {
                  nextIsFunc = false;
                  nextIsEnd = true;
               }
               else if (word == "class")
               {
                  if (nextIsEnd)
                     inClass.clear();
                  nextIsClass = !nextIsEnd;
                  nextIsEnd = false;
               }
               else if (word == "function" || word == "sub")
               {
                  nextIsFunc = !nextIsEnd;
                  nextIsEnd = false;
               }
               else
               {
                  if (word[0] >= 'a' && word[0] <= 'z')
                  {
                     if (nextIsClass)
                        inClass = word;
                     else if (nextIsFunc)
                     {
                        //ss << "- " << word << ", line=" << (line + 1) << ", class=" << inClass << "\r\n";
                        if (FindIndexOf(functions, inClass + '.' + word) != -1)
                           ss << ". Duplicate declaration of '" << word << "' in script at line " << line << "\r\n";
                        else
                           functions.push_back(inClass + '.' + word);
                     }
                     else if (FindIndexOf(identifiers, word) == -1)
                        identifiers.push_back(word);
                  }
                  nextIsFunc = false;
                  nextIsEnd = false;
                  nextIsClass = false;
               }
            }
            wordStart = nullptr;
         }
         else if (wordStart == nullptr)
            wordStart = wordPos;
      }
      // Next line
      if (wordPos[0] == '\n')
      {
         isInComment = false;
         line++;
      }

      wordPos++;
   }
   delete[] szText;

   if (FindIndexOf(identifiers, "execute"s) != -1)
      ss << ". Warning: Scripts seems to use the 'Execute' command. This command triggers computer security checks and will likely causes stutters during play.\r\n";

   if (m_glassBottomHeight > m_glassTopHeight)
      ss << ". Warning: Glass height seems invalid: bottom is higher than top\r\n";

   if (m_glassBottomHeight < INCHESTOVPU(2) || m_glassTopHeight < INCHESTOVPU(2))
      ss << ". Warning: Glass height seems invalid: glass is below 2\"\r\n";

   if (m_ballSphericalMapping)
      ss << ". Warning: Ball uses legacy 'spherical mapping', it will be rendered like a 2D object and therefore will look bad in VR, stereo or headtracking\r\n";

   // Search for inconsistencies in the table parts
   bool hasPulseTimer = false, hasPinMameTimer = false;
   for (auto part : m_vedit)
   {
      auto type = part->GetItemType();
      Primitive *prim = type == eItemPrimitive ? (Primitive *)part : nullptr;
      Surface *surf = type == eItemSurface ? (Surface *)part : nullptr;

      // Referencing a static object from script (ok if it is for reading properties, not for writing)
      if (type == eItemPrimitive && prim->m_d.m_staticRendering && FindIndexOf(identifiers, string(prim->GetName())) != -1) 
         ss << ". Warning: Primitive '" << prim->GetName() << "' seems to be referenced from the script while it is marked as static (most properties of a static object may not be modified at runtime).\r\n";

      if (type == eItemTimer) {
         string name = ((Timer *)part)->GetName();
         StrToLower(name);
         hasPulseTimer |= name == "pulsetimer";
         hasPinMameTimer |= name == "pinmametimer";
      }

      // Warning on very fast timers (lower than 5ms)
      TimerDataRoot *tdr = nullptr;
      switch (type)
      {
      // case eItemPrimitive: tdr = &prim->m_d.m_tdr; break; // Note: primitive have timers but they are not exposed to the UI
      case eItemSurface: tdr = &surf->m_d.m_tdr; break;
      case eItemTimer: tdr = &((Timer *)part)->m_d.m_tdr; break;
      case eItemLight: tdr = &((Light *)part)->m_d.m_tdr; break;
      case eItemRamp: tdr = &((Ramp *)part)->m_d.m_tdr; break;
      case eItemPlunger: tdr = &((Plunger *)part)->m_d.m_tdr; break;
      case eItemSpinner: tdr = &((Spinner *)part)->m_d.m_tdr; break;
      case eItemTrigger: tdr = &((Trigger *)part)->m_d.m_tdr; break;
      case eItemKicker: tdr = &((Kicker *)part)->m_d.m_tdr; break;
      case eItemRubber: tdr = &((Rubber *)part)->m_d.m_tdr; break;
      case eItemFlasher: tdr = &((Flasher *)part)->m_d.m_tdr; break;
      case eItemLightSeq: tdr = &((LightSeq *)part)->m_d.m_tdr; break;
      case eItemHitTarget: tdr = &((HitTarget *)part)->m_d.m_tdr; break;
      case eItemBumper: tdr = &((Bumper *)part)->m_d.m_tdr; break;
      case eItemFlipper: tdr = &((Flipper *)part)->m_d.m_tdr; break;
      case eItemGate: tdr = &((Gate *)part)->m_d.m_tdr; break;
      }
      if (tdr && tdr->m_TimerEnabled && tdr->m_TimerInterval != -1 && tdr->m_TimerInterval != -2 && tdr->m_TimerInterval < 17)
         ss << ". Warning: Part '" << part->GetName() << "' uses a timer with a very short period of " << tdr->m_TimerInterval << "ms, below a 60FPS framerate. This will likely cause stutters and the table will not support 'frame pacing'.\r\n";

      if (type == eItemPrimitive && prim->m_d.m_visible
         && prim->m_d.m_disableLightingBelow != 1.f && !prim->m_d.m_staticRendering 
         && (!GetMaterial(prim->m_d.m_szMaterial)->m_bOpacityActive || GetMaterial(prim->m_d.m_szMaterial)->m_fOpacity == 1.f)
         && (GetImage(prim->m_d.m_szImage) == nullptr || GetImage(prim->m_d.m_szImage)->m_pdsBuffer->IsOpaque()))
         ss << ". Warning: Primitive '" << prim->GetName() << "' uses translucency (lighting from below) while it is fully opaque. Translucency will be discarded.\r\n";

      // Disabled as this is now enforced in the rendering
      // Enabling translucency (light from below) won't work with static parts: otherwise the rendering will be different in VR/Headtracked vs desktop modes. It also needs a non opaque alpha.
      //if (type == eItemPrimitive && prim->m_d.m_disableLightingBelow != 1.f && prim->m_d.m_staticRendering) 
      //   ss << ". Warning: Primitive '" << prim->GetName() << "' has translucency enabled but is also marked as static. Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
      //if (type == eItemSurface && surf->m_d.m_disableLightingBelow != 1.f && surf->StaticRendering()) 
      //   ss << ". Warning: Wall '" << surf->GetName() << "' has translucency enabled but will be staticly rendered (not droppable with opaque materials). Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
   }

   if ((FindIndexOf(identifiers, "loadvpm"s) != -1) || (FindIndexOf(identifiers, "loadvpmalt"s) != -1))
   {
      if (!hasPinMameTimer)
         ss << ". Warning: VPM controller is used but table is missing a Timer object named 'PinMAMETimer'.\r\n";
      if (FindIndexOf(identifiers, "vpminit"s) == -1)
         ss << ". Warning: VPM controller is used but vpmInit is not called. pause/resume/exit will likely exhibit bugs and physic outputs won't be supported.\r\n";
   }

   if (!hasPulseTimer && (FindIndexOf(identifiers, "vpmTimer"s) != -1))
      ss << " Warning: script uses 'vpmTimer' but table is missing a Timer object named 'PulseTimer'. vpmTimer will not work as expected.\r\n";

   for (auto image : m_vimage)
   {
      if (image->m_ppb == nullptr)
         ss << " Warning: Image '" << image->m_szName << "' uses legacy encoding causing waste of memory / file size. It should be converted to WEBP file format.\r\n";
   }

   if (ss.str().empty())
      ss << "No issue identified.\r\n";

   // Also output a log of the table file content to allow easier size optimization
   unsigned long totalSize = 0, totalGpuSize = 0;
   for (auto sound : m_vsound)
   {
      //ss << "  . Sound: '" << sound->m_szName << "', size: " << (sound->m_cdata / 1024) << "KiB\r\n";
      totalSize += sound->m_cdata;
   }
   ss << ". Total sound size: " << (totalSize / (1024 * 1024)) << "MiB\r\n";

   totalSize = 0;
   for (auto image : m_vimage)
   {
      unsigned int imageSize = image->m_ppb != nullptr ? image->m_ppb->m_cdata : image->m_pdsBuffer->height() * image->m_pdsBuffer->pitch();
      unsigned int gpuSize = image->m_pdsBuffer->height() * image->m_pdsBuffer->pitch();
      //ss << "  . Image: '" << image->m_szName << "', size: " << (imageSize / 1024) << "KiB, GPU mem size: " << (gpuSize / 1024) << "KiB\r\n";
      totalSize += imageSize;
      totalGpuSize += gpuSize;
   }
   ss << ". Total image size: " << (totalSize / (1024 * 1024)) << "MiB in VPX file, at least " << (totalGpuSize / (1024 * 1024)) << "MiB in GPU memory when played\r\n";

   int nPrimTris = 0, primMemSize = 0;
   for (auto part : m_vedit)
      if (part->GetItemType() == eItemPrimitive && ((Primitive *)part)->m_d.m_use3DMesh /* && ((Primitive *)part)->m_d.m_visible */ )
      {
         primMemSize += (((Primitive *)part)->m_mesh.NumIndices() > 65536 ? 4 : 2) * (int) ((Primitive *)part)->m_mesh.NumIndices();
         primMemSize += (int) ((Primitive *)part)->m_mesh.NumVertices() * sizeof(Vertex3D_NoTex2);
         nPrimTris += (int) ((Primitive *)part)->m_mesh.NumIndices() / 3;
      }
   ss << ". Total number of faces used in primitives: " << nPrimTris << ", needing " << (primMemSize / (1024 * 1024)) << "MiB in GPU memory when played\r\n";

   string msg = "Table audit:\r\n" + ss.str();
   PLOGI << msg;
   InfoDialog info(msg);
   info.DoModal();
}

void PinTable::ListCustomInfo(HWND hwndListView)
{
   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
      AddListItem(hwndListView, m_vCustomInfoTag[i], m_vCustomInfoContent[i], NULL);
}

int PinTable::AddListItem(HWND hwndListView, const string& szName, const string& szValue1, LPARAM lparam)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (char*)szName.c_str();
   lvitem.lParam = lparam;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, (char*)szValue1.c_str());

   return index;
}

HRESULT PinTable::LoadImageFromStream(IStream *pstm, size_t idx, int version, bool resize_on_low_mem)
{
   if (version < 100) // Tech Beta 3 and below
   {
      ShowError("Tables from Tech Beta 3 and below are not supported in this version.");
      return E_FAIL;
   }
   else
   {
      Texture * const ppi = new Texture();
      ppi->m_maxTexDim = m_settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0); // default: Don't resize textures
      if (ppi->LoadFromStream(pstm, version, this, resize_on_low_mem) == S_OK)
         m_vimage[idx] = ppi;
      else
         delete ppi;
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_image.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO
   m_image = szImage;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut)
{
   return GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, nullptr);
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
{
   return GetPredefinedValue(dispID, dwCookie, pVarOut, nullptr);
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit)
{
   if (pcaStringsOut == nullptr || pcaCookiesOut == nullptr)
      return E_POINTER;

   size_t cvar;
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
      cvar = m_vimage.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      const LocalString ls(IDS_NONE);
      MultiByteToWideCharNull(CP_ACP, 0, ls.m_szbuffer, -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const DWORD cwch = (DWORD)m_vimage[ivar]->m_szName.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("DISPID_Image alloc failed");

         MultiByteToWideCharNull(CP_ACP, 0, m_vimage[ivar]->m_szName.c_str(), -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = (DWORD)ivar;
      }
      cvar++;
   }
   break;
   case IDC_MATERIAL_COMBO:
   case IDC_MATERIAL_COMBO2:
   case IDC_MATERIAL_COMBO3:
   case IDC_MATERIAL_COMBO4:
   {
      cvar = m_materials.size();
      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      const LocalString ls(IDS_NONE);
      MultiByteToWideCharNull(CP_ACP, 0, ls.m_szbuffer, -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const DWORD cwch = (DWORD)m_materials[ivar]->m_szName.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("IDC_MATERIAL_COMBO alloc failed");

         MultiByteToWideCharNull(CP_ACP, 0, m_materials[ivar]->m_szName.c_str(), -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = (DWORD)ivar;
      }
      cvar++;
      break;
   }
   case DISPID_Sound:
   {
      cvar = m_vsound.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR * wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      MultiByteToWideCharNull(CP_ACP, 0, "<None>", -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const DWORD cwch = (DWORD)m_vsound[ivar]->m_szName.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("DISPID_Sound alloc failed");

         MultiByteToWideCharNull(CP_ACP, 0, m_vsound[ivar]->m_szName.c_str(), -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = (DWORD)ivar;
      }
      cvar++;
   }
   break;

   case DISPID_Collection:
   {
      cvar = m_vcollection.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      MultiByteToWideCharNull(CP_ACP, 0, "<None>", -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         constexpr DWORD cwch = sizeof(m_vcollection[(int)ivar].m_wzName) + sizeof(DWORD); //!! +DWORD?
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch);
         if (wzDst == nullptr)
            ShowError("DISPID_Collection alloc failed (1)");
         else
            memcpy(wzDst, m_vcollection[(int)ivar].m_wzName, cwch);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = (DWORD)ivar;
      }
      cvar++;
   }
   break;

   case DISPID_Surface:
   {
      cvar = 0;

      constexpr bool ramps = true;
      constexpr bool flashers = true;

      for (size_t ivar = 0; ivar < m_vedit.size(); ivar++)
         if (m_vedit[ivar]->GetItemType() == eItemSurface ||
            (ramps && m_vedit[ivar]->GetItemType() == eItemRamp) ||
            //!! **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (flashers && m_vedit[ivar]->GetItemType() == eItemFlasher))
            cvar++;

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (DWORD *)CoTaskMemAlloc((cvar + 1) * sizeof(DWORD));

      cvar = 0;

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      if (wzDst == nullptr)
         ShowError("DISPID_Surface alloc failed (0)");
      // TEXT
      WideStrNCopy(L"<None>", wzDst, 7);
      rgstr[cvar] = wzDst;
      rgdw[cvar] = ~0u;
      cvar++;

      for (size_t ivar = 0; ivar < m_vedit.size(); ivar++)
      {
         if (m_vedit[ivar]->GetItemType() == eItemSurface ||
            (ramps && m_vedit[ivar]->GetItemType() == eItemRamp) ||
            //!! **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (flashers && m_vedit[ivar]->GetItemType() == eItemFlasher))
         {
            CComBSTR bstr;
            m_vedit[ivar]->GetScriptable()->get_Name(&bstr);

            const DWORD cwch = lstrlenW(bstr) + 1;
            //wzDst = ::SysAllocString(bstr);

            wzDst = (WCHAR *)CoTaskMemAlloc(cwch * sizeof(WCHAR));
            if (wzDst == nullptr)
               ShowError("DISPID_Surface alloc failed (1)");

            WideStrNCopy(bstr, wzDst, cwch);
            rgstr[cvar] = wzDst;
            rgdw[cvar] = (DWORD)ivar;
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

      WCHAR * wzDst = (WCHAR *)CoTaskMemAlloc(5 * sizeof(WCHAR));
      MultiByteToWideCharNull(CP_ACP, 0, "None", -1, wzDst, 5);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;
      wzDst = (WCHAR *)CoTaskMemAlloc(9 * sizeof(WCHAR));
      MultiByteToWideCharNull(CP_ACP, 0, "Additive", -1, wzDst, 9);
      rgstr[1] = wzDst;
      rgdw[1] = 1;
      wzDst = (WCHAR *)CoTaskMemAlloc(9 * sizeof(WCHAR));
      MultiByteToWideCharNull(CP_ACP, 0, "Multiply", -1, wzDst, 9);
      rgstr[2] = wzDst;
      rgdw[2] = 2;
      wzDst = (WCHAR *)CoTaskMemAlloc(8 * sizeof(WCHAR));
      MultiByteToWideCharNull(CP_ACP, 0, "Overlay", -1, wzDst, 8);
      rgstr[3] = wzDst;
      rgdw[3] = 3;
      wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      MultiByteToWideCharNull(CP_ACP, 0, "Screen", -1, wzDst, 7);
      rgstr[4] = wzDst;
      rgdw[4] = 4;

      break;
   }
   default:
      return E_NOTIMPL;
   }

   pcaStringsOut->cElems = (int)cvar;
   pcaStringsOut->pElems = rgstr;

   pcaCookiesOut->cElems = (int)cvar;
   pcaCookiesOut->pElems = rgdw;

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit)
{
   WCHAR *wzDst;

   switch (dispID)
   {
   default: return E_NOTIMPL;

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
         const DWORD cwch = (DWORD)m_vimage[dwCookie]->m_szName.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

         MultiByteToWideCharNull(CP_ACP, 0, m_vimage[dwCookie]->m_szName.c_str(), -1, wzDst, cwch);
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
         const DWORD cwch = (DWORD)m_materials[dwCookie]->m_szName.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

         MultiByteToWideCharNull(CP_ACP, 0, m_materials[dwCookie]->m_szName.c_str(), -1, wzDst, cwch);
      }
      break;
   }
   case DISPID_Sound:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)CoTaskMemAlloc(1 * sizeof(WCHAR));
         if (wzDst == nullptr)
             ShowError("DISPID_Sound alloc failed");
         wzDst[0] = L'\0';
      }
      else
      {
         const DWORD cwch = (DWORD)m_vsound[dwCookie]->m_szName.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
             ShowError("DISPID_Sound alloc failed");
         MultiByteToWideCharNull(CP_ACP, 0, m_vsound[dwCookie]->m_szName.c_str(), -1, wzDst, cwch);
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
         constexpr size_t cwch = sizeof(m_vcollection[dwCookie].m_wzName) + sizeof(DWORD); //!! +DWORD?
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch);
         if (wzDst == nullptr)
            ShowError("DISPID_Collection alloc failed (2)");
         else
            memcpy(wzDst, m_vcollection[dwCookie].m_wzName, cwch - sizeof(DWORD)); //!! see above
      }
   }
   break;
   case IDC_EFFECT_COMBO:
   {
      const int idx = (dwCookie == -1) ? 0 : dwCookie;
      static const char * const filterNames[5] = { "None", "Additive", "Multiply", "Overlay", "Screen" };
      const DWORD cwch = lstrlen(filterNames[idx]) + 1;
      wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));

      MultiByteToWideCharNull(CP_ACP, 0, filterNames[idx], -1, wzDst, cwch);
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
         m_vedit[dwCookie]->GetScriptable()->get_Name(&bstr);

         const DWORD cwch = lstrlenW(bstr) + 1;
         //wzDst = ::SysAllocString(bstr);

         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
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

float PinTable::GetSurfaceHeight(const string& name, float x, float y) const
{
   if (!name.empty())
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const item = m_vedit[i];
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
         item->GetScriptable()->get_Name(&bstr);
         if (!WzSzStrCmp(bstr, name.c_str()))
         {
            if (item->GetItemType() == eItemSurface)
               return ((Surface *)item)->m_d.m_heighttop;
            else //if (item->GetItemType() == eItemRamp)
               return ((Ramp *)item)->GetSurfaceHeight(x, y);
         }
      }
   }

   return 0.f;
}

Material* PinTable::GetSurfaceMaterial(const string& name) const
{
   if (!name.empty())
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const item = m_vedit[i];
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
         item->GetScriptable()->get_Name(&bstr);
         if (!WzSzStrCmp(bstr, name.c_str()))
         {
            if (item->GetItemType() == eItemSurface)
               return GetMaterial(((Surface *)item)->m_d.m_szTopMaterial);
            else //if (item->GetItemType() == eItemRamp)
               return GetMaterial(((Ramp *)item)->m_d.m_szMaterial);
         }
      }
   }

   return GetMaterial(m_playfieldMaterial);
}

Texture* PinTable::GetSurfaceImage(const string& name) const
{
   if (!name.empty())
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const item = m_vedit[i];
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
         item->GetScriptable()->get_Name(&bstr);
         if (!WzSzStrCmp(bstr, name.c_str()))
         {
            if (item->GetItemType() == eItemSurface)
               return GetImage(((Surface *)item)->m_d.m_szImage);
            else //if (item->GetItemType() == eItemRamp)
               return GetImage(((Ramp *)item)->m_d.m_szImage);
         }
      }
   }

   return GetImage(m_image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP PinTable::get_DisplayGrid(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_grid);

   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayGrid(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_grid = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DisplayBackdrop(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_backdrop);

   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayBackdrop(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_backdrop = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlassHeight(float *pVal)
{
   *pVal = m_glassTopHeight;

   return S_OK;
}

STDMETHODIMP PinTable::put_GlassHeight(float newVal)
{
   STARTUNDO
   m_glassTopHeight = newVal;
   STOPUNDO

   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::get_TableHeight(float *pVal)
{
   *pVal = 0.f;

   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::put_TableHeight(float newVal)
{
   /* STARTUNDO
   m_tableheight = newVal;
   STOPUNDO*/

   return S_OK;
}

float PinTable::GetTableWidth() const
{
    return m_right - m_left;
}

void PinTable::SetTableWidth(const float value)
{
    m_right = value;
}

STDMETHODIMP PinTable::get_Width(float *pVal)
{
   *pVal = GetTableWidth();

   return S_OK;
}

STDMETHODIMP PinTable::put_Width(float newVal)
{
   STARTUNDO
   SetTableWidth(newVal);
   STOPUNDO

   SetMyScrollInfo();

   return S_OK;
}

float PinTable::GetHeight() const
{
    return m_bottom - m_top;
}

void PinTable::SetHeight(const float value)
{
    m_bottom = value;
}

STDMETHODIMP PinTable::get_Height(float *pVal)
{
   *pVal = GetHeight();

   return S_OK;
}

STDMETHODIMP PinTable::put_Height(float newVal)
{
   STARTUNDO
   SetHeight(newVal);
   STOPUNDO

   SetMyScrollInfo();

   return S_OK;
}

STDMETHODIMP PinTable::get_PlayfieldMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_playfieldMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   STARTUNDO
   m_playfieldMaterial = buf;
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

int PinTable::GetGlobalEmissionScale() const
{
    return quantizeUnsignedPercent(m_globalEmissionScale);
}

void PinTable::SetGlobalEmissionScale(const int value)
{
    m_globalEmissionScale = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_NightDay(int *pVal)
{
   *pVal = GetGlobalEmissionScale();

   return S_OK;
}

STDMETHODIMP PinTable::put_NightDay(int newVal)
{
   STARTUNDO
   SetGlobalEmissionScale(newVal);
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

STDMETHODIMP PinTable::get_SSRScale(float *pVal)
{
   *pVal = m_SSRScale;

   return S_OK;
}

STDMETHODIMP PinTable::put_SSRScale(float newVal)
{
   STARTUNDO
   m_SSRScale = newVal;
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
   // FIXME Deprecated
   *pVal = UserDefaultOnOff::On; //(UserDefaultOnOff) m_useReflectionForBalls;

   return S_OK;
}

STDMETHODIMP PinTable::put_BallReflection(UserDefaultOnOff newVal)
{
   // FIXME Deprecated
   //STARTUNDO
   //m_useReflectionForBalls = (int)newVal;
   //STOPUNDO

   return S_OK;
}

int PinTable::GetPlayfieldReflectionStrength() const
{
    return quantizeUnsignedPercent(m_playfieldReflectionStrength);
}

void PinTable::SetPlayfieldReflectionStrength(const int value)
{
    m_playfieldReflectionStrength = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_PlayfieldReflectionStrength(int *pVal)
{
   *pVal = GetPlayfieldReflectionStrength();

   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldReflectionStrength(int newVal)
{
   STARTUNDO
   SetPlayfieldReflectionStrength(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallTrail(UserDefaultOnOff *pVal)
{
   if (!m_settings.HasValue(Settings::Player, "BallTrail"s))
      *pVal = UserDefaultOnOff::Default;
   else
      *pVal = m_settings.LoadValueWithDefault(Settings::Player, "BallTrail"s, true) ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   return S_OK;
}

STDMETHODIMP PinTable::put_BallTrail(UserDefaultOnOff newVal)
{
   if (newVal == -1)
      m_settings.DeleteValue(Settings::Player, "BallTrail"s);
   else
      m_settings.SaveValue(Settings::Player, "BallTrail"s, newVal == 1, true);
   return S_OK;
}

STDMETHODIMP PinTable::get_TrailStrength(int *pVal)
{
   *pVal = (int) (100.f * m_settings.LoadValueWithDefault(Settings::Player, "BallTrailStrength"s, 0.5f));
   return S_OK;
}

STDMETHODIMP PinTable::put_TrailStrength(int newVal)
{
   m_settings.SaveValue(Settings::Player, "BallTrailStrength"s, (float)newVal / 100.f);
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

int PinTable::GetTableSoundVolume() const
{
    return quantizeUnsignedPercent(m_TableSoundVolume);
}

void PinTable::SetTableSoundVolume(const int value)
{
    m_TableSoundVolume = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_TableSoundVolume(int *pVal)
{
   *pVal = GetTableSoundVolume();

   return S_OK;
}

STDMETHODIMP PinTable::put_TableSoundVolume(int newVal)
{
   STARTUNDO
   SetTableSoundVolume(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DetailLevel(int *pVal)
{
   *pVal = m_settings.LoadValueWithDefault(Settings::Player, "AlphaRampAccuracy"s, 10);
   return S_OK;
}

STDMETHODIMP PinTable::put_DetailLevel(int newVal)
{
   m_settings.SaveValue(Settings::Player, "AlphaRampAccuracy"s, newVal, true);
   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalAlphaAcc(VARIANT_BOOL *pVal)
{
   // FIXME NOOP remove
   //*pVal = FTOVB(m_overwriteGlobalDetailLevel);
   *pVal = FTOVB(true);
   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalAlphaAcc(VARIANT_BOOL newVal)
{
   // FIXME NOOP remove
   /*STARTUNDO
   m_overwriteGlobalDetailLevel = VBTOb(newVal);
   if (!m_overwriteGlobalDetailLevel)
      m_userDetailLevel = m_globalDetailLevel;
   STOPUNDO*/
   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalDayNight(VARIANT_BOOL *pVal)
{
   // FIXME deprecated
   //*pVal = FTOVB(m_overwriteGlobalDayNight);
   *pVal = FTOVB(m_settings.HasValue(Settings::Player, "OverrideTableEmissionScale"s));

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDayNight(VARIANT_BOOL newVal)
{
   // FIXME deprecated
   //STARTUNDO
   //m_overwriteGlobalDayNight = VBTOb(newVal);
   //STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalStereo3D(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_overwriteGlobalStereo3D);

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalStereo3D(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_overwriteGlobalStereo3D = VBTOb(newVal);
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
   *pVal = FTOVB(m_BallDecalMode);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallDecalMode(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_BallDecalMode = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

int PinTable::GetTableMusicVolume() const
{
    return quantizeUnsignedPercent(m_TableMusicVolume);
}

void PinTable::SetTableMusicVolume(const int value)
{
    m_TableMusicVolume = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_TableMusicVolume(int *pVal)
{
   *pVal = GetTableMusicVolume();

   return S_OK;
}

STDMETHODIMP PinTable::put_TableMusicVolume(int newVal)
{
   STARTUNDO
   SetTableMusicVolume(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableAdaptiveVSync(int *pVal)
{
   // FIXME deprecated remove
   //*pVal = m_TableAdaptiveVSync;
   *pVal = -1;
   return S_OK;
}

STDMETHODIMP PinTable::put_TableAdaptiveVSync(int newVal)
{
   // FIXME deprecated remove
   //STARTUNDO
   //m_TableAdaptiveVSync = newVal;
   //STOPUNDO
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
   *pVal = FTOVB(m_ImageBackdropNightDay);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImageApplyNightDay(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_ImageBackdropNightDay = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

bool PinTable::IsFSSEnabled() const
{
   return m_BG_enable_FSS;
}

void PinTable::EnableFSS(const bool enable)
{
   m_BG_enable_FSS = enable;
   UpdateCurrentBGSet();
}

void PinTable::UpdateCurrentBGSet()
{
   if (m_BG_override != BG_INVALID)
      m_BG_current_set = m_BG_override;
   else
   {
      const int setup = m_settings.LoadValueWithDefault(Settings::Player, "BGSet"s, 0);
      switch (setup)
      {
      case 0: m_BG_current_set = m_BG_enable_FSS ? BG_FSS : BG_DESKTOP; break; // Desktop mode (FSS if table supports it, usual dekstop otherwise)
      case 1: m_BG_current_set = BG_FULLSCREEN; break; // Cabinet mode
      case 2: m_BG_current_set = BG_DESKTOP; break; // Desktop mode with FSS disabled (forced desktop)
      }
   }
}

STDMETHODIMP PinTable::get_ShowFSS(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(IsFSSEnabled());

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_DT(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_BG_image[0].c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_DT(BSTR newVal) //!! HDR??
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   STARTUNDO
   m_BG_image[0] = buf;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_FS(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_BG_image[1].c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_FS(BSTR newVal) //!! HDR??
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   STARTUNDO
   m_BG_image[1] = buf;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_FSS(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_BG_image[2].c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_FSS(BSTR newVal) //!! HDR??
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   STARTUNDO
   m_BG_image[2] = buf;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ColorGradeImage(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_imageColorGrade.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_ColorGradeImage(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = GetImage(szImage);
   if (tex && (tex->m_width != 256 || tex->m_height != 16))
   {
       ShowError("Wrong image size, needs to be 256x16 resolution");
       return E_FAIL;
   }

   STARTUNDO
   m_imageColorGrade = szImage;
   STOPUNDO

   return S_OK;
}

float PinTable::GetGravity() const
{
   return m_Gravity * (float)(1.0 / GRAVITYCONST);
}

void PinTable::SetGravity(const float value)
{
    m_Gravity = value * GRAVITYCONST;
}

STDMETHODIMP PinTable::get_Gravity(float *pVal)
{
   *pVal = GetGravity();

   return S_OK;
}

STDMETHODIMP PinTable::put_Gravity(float newVal)
{
   if (newVal < 0.f) newVal = 0.f;

   if (g_pplayer)
   {
      SetGravity(newVal);
      const float minSlope = (m_overridePhysics ? m_fOverrideMinSlope : m_angletiltMin);
      const float maxSlope = (m_overridePhysics ? m_fOverrideMaxSlope : m_angletiltMax);
      const float slope = minSlope + (maxSlope - minSlope) * m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_overridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
      SetGravity(newVal);
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Friction(float *pVal)
{
   *pVal = m_friction;

   return S_OK;
}

void PinTable::SetFriction(const float value)
{
    m_friction = clamp(value, 0.0f, 1.0f);
}

STDMETHODIMP PinTable::put_Friction(float newVal)
{
   STARTUNDO
   SetFriction(newVal);
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

void PinTable::SetPlungerNormalize(const int value)
{
   m_plungerNormalize = m_settings.LoadValueWithDefault(Settings::Player, "PlungerNormalize"s, value);
}

STDMETHODIMP PinTable::put_PlungerNormalize(int newVal)
{
   STARTUNDO
   SetPlungerNormalize(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_PlungerFilter(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_plungerFilter);

   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerFilter(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_plungerFilter = m_settings.LoadValueWithDefault(Settings::Player, "PlungerFilter"s, VBTOb(newVal));
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
   m_currentBackglassMode = (ViewSetupID)(pVal - DESKTOP);

   return S_OK;
}

STDMETHODIMP PinTable::get_FieldOfView(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mFOV;

   return S_OK;
}

STDMETHODIMP PinTable::put_FieldOfView(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mFOV = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Inclination(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mLookAt;

   return S_OK;
}

STDMETHODIMP PinTable::put_Inclination(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mLookAt = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Layback(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mLayback;

   return S_OK;
}

STDMETHODIMP PinTable::put_Layback(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mLayback = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Rotation(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewportRotation;

   return S_OK;
}

STDMETHODIMP PinTable::put_Rotation(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewportRotation = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalex(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mSceneScaleX;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scalex(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mSceneScaleX = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scaley(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mSceneScaleY;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scaley(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mSceneScaleY = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalez(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mSceneScaleZ;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scalez(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mSceneScaleZ = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatex(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewX;

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatex(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewX = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatey(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewY;

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatey(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewY = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatez(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewZ;

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatez(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewZ = newVal;
   STOPUNDO

   return S_OK;
}


STDMETHODIMP PinTable::get_SlopeMax(float *pVal)
{
   *pVal = m_angletiltMax;

   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMax(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMax = newVal;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin) * m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_overridePhysics ? m_fOverrideGravityConstant : m_Gravity);
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
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin) * m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_overridePhysics ? m_fOverrideGravityConstant : m_Gravity);
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
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_ballImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallImage(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   STARTUNDO
   m_ballImage = buf;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnvironmentImage(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_envImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnvironmentImage(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = GetImage(szImage);
   if (tex && (tex->m_width != tex->m_height*2))
   {
       ShowError("Wrong image size, needs to be 2x width in comparison to height");
       return E_FAIL;
   }

   STARTUNDO
   m_envImage = szImage;
   STOPUNDO

   return S_OK;
}

// deprecated
STDMETHODIMP PinTable::get_YieldTime(long *pVal)
{
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   else
      *pVal = 0;

   return S_OK;
}

// deprecated
STDMETHODIMP PinTable::put_YieldTime(long newVal)
{
   if (!g_pplayer)
      return E_FAIL;

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableAntialiasing(UserDefaultOnOff *pVal)
{
   // See put_EnableFXAA, not sure why we keep this
   if (m_settings.HasValue(Settings::Player, "AAFactor"s))
      *pVal = m_settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, 1.f) > 1.f ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   else
      *pVal = UserDefaultOnOff::Default;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableAntialiasing(UserDefaultOnOff newVal)
{
   // See put_EnableFXAA, not sure why we keep this
   STARTUNDO
   if (newVal == UserDefaultOnOff::Default)
      m_settings.DeleteValue(Settings::Player, "AAFactor"s);
   else
      m_settings.SaveValue(Settings::Player, "AAFactor"s, newVal == UserDefaultOnOff::On ? 2.f : 1.f);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableSSR(UserDefaultOnOff *pVal)
{
   *pVal = m_enableSSR ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   return S_OK;
}

STDMETHODIMP PinTable::put_EnableSSR(UserDefaultOnOff newVal)
{
   if (newVal == UserDefaultOnOff::Default)
      return S_FAIL;
   STARTUNDO
   m_enableSSR = (int)newVal;
   STOPUNDO
   return S_OK;
}

STDMETHODIMP PinTable::get_EnableAO(UserDefaultOnOff *pVal)
{
   *pVal = m_enableAO ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   return S_OK;
}

STDMETHODIMP PinTable::put_EnableAO(UserDefaultOnOff newVal)
{
   if (newVal == UserDefaultOnOff::Default)
      return S_FAIL;
   STARTUNDO
   m_enableAO = (int)newVal;
   STOPUNDO
   return S_OK;
}

STDMETHODIMP PinTable::get_EnableFXAA(FXAASettings *pVal)
{
   // See put_EnableFXAA, not sure why we keep this
   if (m_settings.HasValue(Settings::Player, "FXAA"s))
      *pVal = (FXAASettings)m_settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Standard_FXAA);
   else
      *pVal = FXAASettings::Defaults;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableFXAA(FXAASettings newVal)
{
   // TODO I don't really get why we would expose the FXAA to script, undo is wrong too, it also may create an unwanted ini (did the caller wanted to change the app or the table setting ? is this used by someone ?)
   STARTUNDO
   if (newVal == FXAASettings::Defaults)
      m_settings.DeleteValue(Settings::Player, "FXAA"s);
   else
      m_settings.SaveValue(Settings::Player, "FXAA"s, (int)newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_OverridePhysics(PhysicsSet *pVal)
{
   *pVal = (PhysicsSet)m_overridePhysics;

   return S_OK;
}

STDMETHODIMP PinTable::put_OverridePhysics(PhysicsSet newVal)
{
   STARTUNDO
   m_overridePhysics = (int)newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_OverridePhysicsFlippers(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_overridePhysicsFlipper);

   return S_OK;
}

STDMETHODIMP PinTable::put_OverridePhysicsFlippers(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_overridePhysicsFlipper = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::ImportPhysics()
{
   string szInitialDir;
   if (!m_settings.LoadValue(Settings::RecentDir, "PhysicsDir"s, szInitialDir))
      szInitialDir = PATH_TABLES;

   vector<string> szFileName;
   if (!m_vpinball->OpenFileDialog(szInitialDir, szFileName, "Visual Pinball Physics (*.vpp)\0*.vpp\0", "vpp", 0))
      return S_OK;

   const size_t index = szFileName[0].find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
      g_pvp->m_settings.SaveValue(Settings::RecentDir, "PhysicsDir"s, szFileName[0].substr(0, index));

   ImportVPP(szFileName[0]);

   return S_OK;
}

void PinTable::ImportVPP(const string& filename)
{
   tinyxml2::XMLDocument xmlDoc;
   float FlipperPhysicsMass, FlipperPhysicsStrength, FlipperPhysicsElasticity, FlipperPhysicsScatter, FlipperPhysicsTorqueDamping, FlipperPhysicsTorqueDampingAngle, FlipperPhysicsReturnStrength, FlipperPhysicsElasticityFalloff, FlipperPhysicsFriction, FlipperPhysicsCoilRampUp;
   try
   {
      std::stringstream buffer;
      std::ifstream myFile(filename);
      buffer << myFile.rdbuf();
      myFile.close();
      auto xml = buffer.str();

      if (xmlDoc.Parse(xml.c_str()))
      {
         ShowError("Error parsing VPP XML file");
         return;
      }
      auto root = xmlDoc.FirstChildElement("physics");
      auto physTab = root->FirstChildElement("table");
      auto physFlip = root->FirstChildElement("flipper");

      char str[16];
      float val;

      if(physTab->FirstChildElement("gravityConstant") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("gravityConstant")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_Gravity(val);
      }
      else
          ShowError("gravityConstant is missing");


      if (physTab->FirstChildElement("contactFriction") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("contactFriction")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_Friction(val);
      }
      else
          ShowError("contactFriction is missing");

      if (physTab->FirstChildElement("elasticity") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("elasticity")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_Elasticity(val);
      }
      else
          ShowError("elasticity is missing");

      if (physTab->FirstChildElement("elasticityFalloff") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("elasticityFalloff")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_ElasticityFalloff(val);
      }
      else
          ShowError("elasticityFalloff is missing");

      if (physTab->FirstChildElement("playfieldScatter") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("playfieldScatter")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_Scatter(val);
      }
      else
          ShowError("playfieldScatter is missing");

      if (physTab->FirstChildElement("defaultElementScatter") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("defaultElementScatter")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_DefaultScatter(val);
      }
      else
          ShowError("defaultElementScatter is missing");

      if (physTab->FirstChildElement("playfieldminslope") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("playfieldminslope")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_SlopeMin(val);
      }
      else
      //    ShowError("playfieldminslope is missing"); //was added lateron, so don't error
          put_SlopeMin(DEFAULT_TABLE_MIN_SLOPE);

      if (physTab->FirstChildElement("playfieldmaxslope") != nullptr)
      {
          strncpy_s(str, physTab->FirstChildElement("playfieldmaxslope")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &val);
          put_SlopeMax(val);
      }
      else
      //    ShowError("playfieldmaxslope is missing"); //was added lateron, so don't error
          put_SlopeMax(DEFAULT_TABLE_MAX_SLOPE);

      if(physFlip->FirstChildElement("speed") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("speed")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsMass);
      }
      else
      {
          ShowError("flipper speed is missing");
          FlipperPhysicsMass = 0.0f;
      }

      if(physFlip->FirstChildElement("strength") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("strength")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsStrength);
      }
      else
      {
          ShowError("flipper strength is missing");
          FlipperPhysicsStrength = 0.0f;
      }

      if(physFlip->FirstChildElement("elasticity") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("elasticity")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsElasticity);
      }
      else
      {
          ShowError("flipper elasticity is missing");
          FlipperPhysicsElasticity = 0.0f;
      }

      if(physFlip->FirstChildElement("scatter") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("scatter")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsScatter);
      }
      else
      {
          ShowError("flipper scatter is missing");
          FlipperPhysicsScatter = 0.0f;
      }

      if(physFlip->FirstChildElement("eosTorque") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("eosTorque")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsTorqueDamping);
      }
      else
      {
          ShowError("flipper eosTorque is missing");
          FlipperPhysicsTorqueDamping = 0.0f;
      }

      if(physFlip->FirstChildElement("eosTorqueAngle") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("eosTorqueAngle")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsTorqueDampingAngle);
      }
      else
      {
          ShowError("flipper eosTorqueAngle is missing");
          FlipperPhysicsTorqueDampingAngle = 0.0f;
      }


      if(physFlip->FirstChildElement("returnStrength") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("returnStrength")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsReturnStrength);
      }
      else
      {
          ShowError("flipper returnStrength is missing");
          FlipperPhysicsReturnStrength = 0.0f;
      }


      if(physFlip->FirstChildElement("elasticityFalloff") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("elasticityFalloff")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsElasticityFalloff);
      }
      else
      {
          ShowError("flipper elasticityFalloff is missing");
          FlipperPhysicsElasticityFalloff = 0.0f;
      }

      if(physFlip->FirstChildElement("friction") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("friction")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsFriction);
      }
      else
      {
          ShowError("flipper friction is missing");
          FlipperPhysicsFriction = 0.0f;
      }

      if(physFlip->FirstChildElement("coilRampUp") != nullptr)
      {
          strncpy_s(str, physFlip->FirstChildElement("coilRampUp")->GetText(), sizeof(str)-1);
          sscanf_s(str, "%f", &FlipperPhysicsCoilRampUp);
      }
      else
      {
          ShowError("flipper coilRampUp is missing");
          FlipperPhysicsCoilRampUp = 0.0f;
      }
   }
   catch (...)
   {
      ShowError("Error parsing physics settings file");
   }
   xmlDoc.Clear();

   for (size_t i = 0; i < m_vedit.size(); i++)
      if (m_vedit[i]->GetItemType() == eItemFlipper)
      {
         Flipper * const flipper = (Flipper *)m_vedit[i];
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
}

STDMETHODIMP PinTable::ExportPhysics()
{
   bool foundflipper = false;
   size_t i;
   for (i = 0; i < m_vedit.size(); i++)
   {
      if (m_vedit[i]->GetItemType() == eItemFlipper)
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

   Flipper * const flipper = (Flipper *)m_vedit[i];

   char szFileName[MAXSTRING];
   strncpy_s(szFileName, m_szFileName.c_str(), sizeof(szFileName)-1);
   const size_t idx = m_szFileName.find_last_of('.');
   if (idx != string::npos && idx < MAXSTRING)
      szFileName[idx] = '\0';

   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = m_vpinball->theInstance;
   ofn.hwndOwner = m_vpinball->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = sizeof(szFileName);
   ofn.lpstrDefExt = "vpp";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   string szInitialDir;
   if (!m_settings.LoadValue(Settings::RecentDir, "PhysicsDir"s, szInitialDir))
      szInitialDir = PATH_TABLES;

   ofn.lpstrInitialDir = szInitialDir.c_str();

   const int ret = GetSaveFileName(&ofn);
   if (ret == 0)
      return S_OK;

   const string szFilename(ofn.lpstrFile);
   const size_t index = szFilename.find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
   {
       const string newInitDir(szFilename.substr(0, index));
       g_pvp->m_settings.SaveValue(Settings::RecentDir, "PhysicsDir"s, newInitDir);
   }

   tinyxml2::XMLDocument xmlDoc;

   auto root = xmlDoc.NewElement("physics");
   auto physFlip = xmlDoc.NewElement("flipper");
   auto physTab = xmlDoc.NewElement("table");

   float val;

   flipper->get_Mass(&val); // was speed
   auto node = xmlDoc.NewElement("speed");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_Strength(&val);
   node = xmlDoc.NewElement("strength");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_Elasticity(&val);
   node = xmlDoc.NewElement("elasticity");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_Scatter(&val); // was scatter angle
   node = xmlDoc.NewElement("scatter");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_EOSTorque(&val);
   node = xmlDoc.NewElement("eosTorque");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_EOSTorqueAngle(&val);
   node = xmlDoc.NewElement("eosTorqueAngle");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_Return(&val);
   node = xmlDoc.NewElement("returnStrength");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_ElasticityFalloff(&val);
   node = xmlDoc.NewElement("elasticityFalloff");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_Friction(&val);
   node = xmlDoc.NewElement("friction");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   flipper->get_RampUp(&val);
   node = xmlDoc.NewElement("coilRampUp");
   node->SetText(val);
   physFlip->InsertEndChild(node);

   get_Gravity(&val);
   node = xmlDoc.NewElement("gravityConstant");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_Friction(&val);
   node = xmlDoc.NewElement("contactFriction");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_Elasticity(&val);
   node = xmlDoc.NewElement("elasticity");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_ElasticityFalloff(&val);
   node = xmlDoc.NewElement("elasticityFalloff");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_DefaultScatter(&val);
   node = xmlDoc.NewElement("defaultElementScatter");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_Scatter(&val);
   node = xmlDoc.NewElement("playfieldScatter");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_SlopeMin(&val);
   node = xmlDoc.NewElement("playfieldminslope");
   node->SetText(val);
   physTab->InsertEndChild(node);

   get_SlopeMax(&val);
   node = xmlDoc.NewElement("playfieldmaxslope");
   node->SetText(val);
   physTab->InsertEndChild(node);

   auto settingName = xmlDoc.NewElement("name");
   settingName->SetText(m_szTitle.c_str());
   root->InsertEndChild(settingName);
   root->InsertEndChild(physTab);
   root->InsertEndChild(physFlip);
   xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
   xmlDoc.InsertEndChild(root);

   tinyxml2::XMLPrinter prn;
   xmlDoc.Print(&prn);

   std::ofstream myfile(ofn.lpstrFile);
   myfile << prn.CStr();
   myfile.close();

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableDecals(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_renderDecals);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableDecals(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_renderDecals = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ShowDT(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_BG_current_set == BG_DESKTOP || m_BG_current_set == BG_FSS); // DT & FSS

   return S_OK;
}

STDMETHODIMP PinTable::get_ReflectElementsOnPlayfield(VARIANT_BOOL *pVal)
{
   // FIXME Deprecated
   *pVal = FTOVB(true);

   return S_OK;
}

STDMETHODIMP PinTable::put_ReflectElementsOnPlayfield(VARIANT_BOOL newVal)
{
   // FIXME deprecated
   //STARTUNDO
   //m_reflectElementsOnPlayfield = VBTOb(newVal);
   //STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableEMReels(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_renderEMReels);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableEMReels(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_renderEMReels = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

float PinTable::GetGlobalDifficulty() const
{
    return m_globalDifficulty * 100.f;
}

void PinTable::SetGlobalDifficulty(const float value)
{
   m_difficulty = value;
   m_globalDifficulty = m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, m_difficulty);
}

STDMETHODIMP PinTable::get_GlobalDifficulty(float *pVal)
{
    *pVal = GetGlobalDifficulty();

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDifficulty(float newVal)
{
   if (!g_pplayer) // VP Editor
   {
       STARTUNDO
       SetGlobalDifficulty(newVal);
       STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Accelerometer(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_tblAccelerometer);

   return S_OK;
}

STDMETHODIMP PinTable::put_Accelerometer(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_tblAccelerometer = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_AccelNormalMount(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_tblAccelNormalMount);

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelNormalMount(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_tblAccelNormalMount = VBTOb(newVal);
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
   *pVal = m_settings.LoadValueWithDefault(Settings::Player, "DeadZone"s, 0);

   return S_OK;
}

STDMETHODIMP PinTable::put_DeadZone(int newVal)
{
   g_pvp->m_settings.SaveValue(Settings::Player, "DeadZone"s, clamp(newVal, 0,100));

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
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_ballImageDecal.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallFrontDecal(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO
   m_ballImageDecal = szImage;
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
   return m_vpinball->QuitPlayer(CloseType);
}

STDMETHODIMP PinTable::get_Version(int *pVal)
{
   *pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
   return S_OK;
}

STDMETHODIMP PinTable::get_VPBuildVersion(double *pVal)
{
   *pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV + GIT_REVISION / 10000.0;
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

STDMETHODIMP PinTable::get_Option(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[out, retval]*/ float* param)
{
   if (V_VT(&values) != VT_ERROR && V_VT(&values) != VT_EMPTY && V_VT(&values) != (VT_ARRAY | VT_VARIANT))
      return S_FALSE;
   if (minValue >= maxValue || step <= 0.f || defaultValue < minValue || defaultValue > maxValue)
      return S_FALSE;

   vector<string> literals;
   if (V_VT(&values) == (VT_ARRAY | VT_VARIANT))
   {
      if (V_VT(&values) != (VT_ARRAY | VT_VARIANT) || step != 1.f || (minValue - (float)(int)minValue) != 0.f || (maxValue - (float)(int)maxValue) != 0.f)
         return S_FALSE;
      int nValues = 1 + (int)maxValue - (int)minValue;
      SAFEARRAY *psa = V_ARRAY(&values);
      LONG lbound, ubound;
      if (SafeArrayGetLBound(psa, 1, &lbound) != S_OK || SafeArrayGetUBound(psa, 1, &ubound) != S_OK || ubound != lbound + nValues - 1)
         return S_FALSE;
      VARIANT *p;
      SafeArrayAccessData(psa, (void **)&p);
      for (int i = 0; i < nValues; i++)
      {
         char buf[MAXTOKEN];
         WideCharToMultiByteNull(CP_ACP, 0, V_BSTR(&p[i]), -1, buf, MAXTOKEN, nullptr, nullptr);
         literals.push_back(buf);
      }
      SafeArrayUnaccessData(psa);
   }
   string name = MakeString(optionName);
   m_settings.RegisterSetting(Settings::TableOption, name, minValue, maxValue, step, defaultValue, (Settings::OptionUnit)unit, literals);

   float value = m_settings.LoadValueWithDefault(Settings::TableOption, name, defaultValue);
   *param = clamp(minValue + step * roundf((value - minValue) / step), minValue, maxValue);

   return S_OK;
}

STDMETHODIMP PinTable::put_Option(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[in]*/ float val)
{
   if (V_VT(&values) != VT_ERROR && V_VT(&values) != VT_EMPTY && V_VT(&values) != (VT_ARRAY | VT_VARIANT))
      return S_FALSE;
   if (minValue >= maxValue || step <= 0.f || defaultValue < minValue || defaultValue > maxValue)
      return S_FALSE;

   vector<string> literals;
   if (V_VT(&values) == (VT_ARRAY | VT_VARIANT))
   {
      if (V_VT(&values) != (VT_ARRAY | VT_VARIANT) || step != 1.f || (minValue - (float)(int)minValue) != 0.f || (maxValue - (float)(int)maxValue) != 0.f)
         return S_FALSE;
      int nValues = 1 + (int)maxValue - (int)minValue;
      SAFEARRAY *psa = V_ARRAY(&values);
      LONG lbound, ubound;
      if (SafeArrayGetLBound(psa, 1, &lbound) != S_OK || SafeArrayGetUBound(psa, 1, &ubound) != S_OK || ubound != lbound + nValues - 1)
         return S_FALSE;
      VARIANT *p;
      SafeArrayAccessData(psa, (void **)&p);
      for (int i = 0; i < nValues; i++)
      {
         char buf[MAXTOKEN];
         WideCharToMultiByteNull(CP_ACP, 0, V_BSTR(&p[i]), -1, buf, MAXTOKEN, nullptr, nullptr);
         literals.push_back(buf);
      }
      SafeArrayUnaccessData(psa);
   }
   string name = MakeString(optionName);
   m_settings.RegisterSetting(Settings::TableOption, name, minValue, maxValue, step, defaultValue, (Settings::OptionUnit)unit, literals);
   
   m_settings.SaveValue(Settings::TableOption, name, val);

   return S_OK;
}

void PinTable::InvokeBallBallCollisionCallback(const Ball *b1, const Ball *b2, float hitVelocity)
{
   if (g_pplayer)
   {
      CComPtr<IDispatch> disp;
      m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

      static wchar_t FnName[] = L"OnBallBallCollision";
      LPOLESTR fnNames = FnName;

      DISPID dispid;
      const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &dispid);

      if (SUCCEEDED(hr))  // did we find the collision callback function?
      {
         // note: arguments are passed in reverse order
         CComVariant rgvar[3] = {
            CComVariant(hitVelocity),
            CComVariant(static_cast<IDispatch*>(b2->m_pballex)),
            CComVariant(static_cast<IDispatch*>(b1->m_pballex))
         };
         DISPPARAMS dispparams = { rgvar, nullptr, 3, 0 };

         disp->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparams, nullptr, nullptr, nullptr);
      }
   }
}

void PinTable::OnInitialUpdate()
{
    PLOGI << "PinTable OnInitialUpdate"; // For profiling

    BeginAutoSaveCounter();
    SetWindowText(m_szFileName.c_str());
    SetCaption(m_szTitle);
    m_vpinball->SetEnableMenuItems();
}

BOOL PinTable::OnEraseBkgnd(CDC& dc)
{
   return TRUE;
}

void PinTable::SetMouseCursor()
{
    HINSTANCE hinst = m_vpinball->theInstance;
    static int oldTool = -1;

    if(oldTool!=m_vpinball->m_ToolCur)
    {
        char *cursorid;
        if (m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY)
        {
            cursorid = MAKEINTRESOURCE(IDC_MAGNIFY);
        }
        else if (m_vpinball->m_ToolCur == ID_INSERT_TARGET)
        {
            // special case for targets, which are particular walls
            cursorid = MAKEINTRESOURCE(IDC_TARGET);
        }
        else
        {
            const ItemTypeEnum type = EditableRegistry::TypeFromToolID(m_vpinball->m_ToolCur);
            if (type != eItemInvalid)
                cursorid = MAKEINTRESOURCE(EditableRegistry::GetCursorID(type));
            else
            {
                hinst = nullptr;
                cursorid = IDC_ARROW;
            }
        }
        const HCURSOR hcursor = LoadCursor(hinst, cursorid);
        SetClassLongPtr(GCLP_HCURSOR, (LONG_PTR)hcursor);
        SetCursor(hcursor);
        oldTool = m_vpinball->m_ToolCur;
    }
}

void PinTable::OnLeftButtonDown(const short x, const short y)
{
    if ((m_vpinball->m_ToolCur == IDC_SELECT) || (m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY))
    {
        DoLeftButtonDown(x, y, true);
    }
    else if (!IsLocked())
    {
        UseTool(x, y, m_vpinball->m_ToolCur);
    }
    SetFocus();
}

void PinTable::OnMouseMove(const short x, const short y)
{
    const bool middleMouseButtonPressed = ((GetKeyState(VK_MBUTTON) & 0x100) != 0);  //((GetKeyState(VK_MENU) & 0x80000000) != 0);
    if (middleMouseButtonPressed)
    {
        // panning feature starts here...if the user holds the middle mouse button and moves the mouse 
        // everything is moved in the direction of the mouse was moved
        const int dx = abs(m_oldMousePos.x - x);
        const int dy = abs(m_oldMousePos.y - y);
        if (m_oldMousePos.x > x) m_offset.x += (float)dx;
        if (m_oldMousePos.x < x) m_offset.x -= (float)dx;
        if (m_oldMousePos.y > y) m_offset.y += (float)dy;
        if (m_oldMousePos.y < y) m_offset.y -= (float)dy;

        SetDirtyDraw();
        SetMyScrollInfo();

        m_oldMousePos.x = x;
        m_oldMousePos.y = y;
        return;
    }

    DoMouseMove(x, y);
    m_oldMousePos.x = x;
    m_oldMousePos.y = y;
}

void PinTable::OnMouseWheel(const short x, const short y, const short zDelta)
{
    const int ksctrl = GetKeyState(VK_CONTROL);
    if ((ksctrl & 0x80000000))
    {
        POINT curpt;
        curpt.x = x;
        curpt.y = y;
        m_mdiTable->ScreenToClient(curpt);
        const short x2 = (short)curpt.x;
        const short y2 = (short)curpt.y;
        if ((m_vpinball->m_ToolCur == IDC_SELECT) || (m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY))
        {
            DoLeftButtonDown(x2, y2, zDelta != -120);
        }
    }
    else
    {
        m_offset.y -= (float)zDelta / m_zoom; // change to orientation to match windows default
        SetDirtyDraw();
        SetMyScrollInfo();
    }
}

void PinTable::OnSize()
{
    SetMyScrollInfo();
    SetDirtyDraw();
}

#pragma endregion

#pragma region PinTableMDI

PinTableMDI::PinTableMDI(VPinball *vpinball)
{
    CComObject<PinTable>::CreateInstance(&m_table);
    m_vpinball = vpinball;

    m_table->AddRef();

    m_table->SetMDITable(this);
    SetView(*m_table);

    //m_menu.LoadMenu(IDR_APPMENU);
    SetHandles(m_vpinball->GetMenu(), nullptr);
}

PinTableMDI::~PinTableMDI()
{
    m_vpinball->CloseAllDialogs();

    if (m_table != nullptr)
    {
        if (m_table->m_searchSelectDlg.IsWindow())
           m_table->m_searchSelectDlg.Destroy();

        m_table->FVerifySaveToClose();

        RemoveFromVectorSingle(m_vpinball->m_vtable, (CComObject<PinTable>*)m_table);

        m_table->Release();
    }
}

bool PinTableMDI::CanClose() const
{
    if (m_table != nullptr && m_table->FDirty() && !g_pvp->m_povEdit)
    {
        const LocalString ls1(IDS_SAVE_CHANGES1);
        const LocalString ls2(IDS_SAVE_CHANGES2);
        const string szText = ls1.m_szbuffer/*"Do you want to save the changes you made to '"*/ + m_table->m_szTitle + ls2.m_szbuffer;
        const int result = MessageBox(szText.c_str(), "Visual Pinball", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONWARNING);

        if (result == IDCANCEL)
            return false;

        if (result == IDYES)
        {
            if (m_table->TableSave() != S_OK)
            {
                const LocalString ls3(IDS_SAVEERROR);
                MessageBox(ls3.m_szbuffer, "Visual Pinball", MB_ICONERROR);
            }
        }
    }
    return true;
}

void PinTableMDI::PreCreate(CREATESTRUCT &cs)
{
    cs.x = 20;
    cs.y = 20;
    cs.cx = 400;
    cs.cy = 400;
    cs.style = WS_MAXIMIZE;
    cs.hwndParent = m_vpinball->GetHwnd();
    cs.lpszClass = _T("PinTable");
    cs.lpszName = _T(m_table->m_szFileName.c_str());
}

int PinTableMDI::OnCreate(CREATESTRUCT &cs)
{
    SetWindowText(m_table->m_szTitle.c_str());
    SetIconLarge(IDI_TABLE);
    SetIconSmall(IDI_TABLE);
    return CMDIChild::OnCreate(cs);
}

void PinTableMDI::OnClose()
{
    if(m_vpinball->IsClosing() || CanClose())
    {
        if(g_pvp->GetNotesDocker() != nullptr)
        {
           g_pvp->GetNotesDocker()->UpdateText();
           g_pvp->GetNotesDocker()->CleanText();
        }
        m_table->KillTimer(VPinball::TIMER_ID_AUTOSAVE);
        CMDIChild::OnClose();
    }
}

LRESULT PinTableMDI::OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
   //wparam holds HWND of the MDI frame that is about to be deactivated
   //lparam holds HWND of the MDI frame that is about to be activated
   if ((GetHwnd() == (HWND)wparam) && !m_table->m_szFileName.empty())
   {
      const string szINIFilename = m_table->GetSettingsFileName();
      if (!szINIFilename.empty())
         m_table->m_settings.SaveToFile(szINIFilename);
      if (g_pvp->m_ptableActive == m_table)
         g_pvp->m_ptableActive = nullptr;
   }
   if(GetHwnd()==(HWND)lparam)
   {
      g_pvp->m_ptableActive = m_table;
      if (g_pvp->GetLayersDocker() != nullptr)
      {
         g_pvp->GetLayersDocker()->GetContainLayers()->GetLayersDialog()->SetActiveTable(m_table);
         g_pvp->GetLayersDocker()->GetContainLayers()->GetLayersDialog()->UpdateLayerList();
         g_pvp->SetPropSel(m_table->m_vmultisel);
      }
      m_vpinball->m_currentTablePath = PathFromFilename(m_table->m_szFileName);
   }
   return CMDIChild::OnMDIActivate(msg, wparam, lparam);
}

BOOL PinTableMDI::OnEraseBkgnd(CDC& dc)
{
   return TRUE;
}

#pragma endregion

ProgressDialog::ProgressDialog() : CDialog(IDD_PROGRESS)
{
}

BOOL ProgressDialog::OnInitDialog()
{
   AttachItem(IDC_PROGRESS2, m_progressBar);
   AttachItem(IDC_STATUSNAME, m_progressName);

   return TRUE;
}
