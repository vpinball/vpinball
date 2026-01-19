// license:GPLv3+

#include "core/stdafx.h"

#include "ScriptGlobalTable.h"

#include "core/vpversion.h"
#include "core/VPXPluginAPIImpl.h"

#ifdef EXT_CAPTURE
#include "renderer/captureExt.h"
#endif
#ifndef __STANDALONE__
#include <atlsafe.h>
#endif

#include "serial.h"
static serial Serial;


ScriptGlobalTable::~ScriptGlobalTable()
{
}

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
   if (g_pplayer)
      g_pplayer->m_physics->Nudge(Angle, Force);
   return S_OK;
}


STDMETHODIMP ScriptGlobalTable::NudgeGetCalibration(VARIANT *XMax, VARIANT *YMax, VARIANT *XGain, VARIANT *YGain, VARIANT *DeadZone, VARIANT *TiltSensitivity)
{
   // Deprecated
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::NudgeSetCalibration(int XMax, int YMax, int XGain, int YGain, int DeadZone, int TiltSensitivity)
{
   // Deprecated
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

STDMETHODIMP ScriptGlobalTable::PlaySound(BSTR bstr, LONG LoopCount, float volume, float pan, float randompitch, LONG pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade)
{
   m_pt->PlaySound(bstr, LoopCount, volume, pan, randompitch, pitch, usesame, restart, front_rear_fade);
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

STDMETHODIMP ScriptGlobalTable::StopSound(BSTR soundName)
{
   m_pt->StopSound(soundName);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlayMusic(BSTR str, float volume)
{
   if (g_pplayer && g_pplayer->m_PlayMusic)
   {
      EndMusic();

      const string musicNameStr = MakeString(str);
      if (musicNameStr.empty())
         return S_OK;

      const std::filesystem::path musicPath = normalize_path_separators(musicNameStr);

      std::filesystem::path musicDir = g_pvp->GetTablePath(g_pplayer ? g_pplayer->m_ptable : g_pvp->GetActiveTable(), VPinball::TableSubFolder::Music, false);
      const std::filesystem::path path = find_case_insensitive_file_path(musicDir / musicPath);
      if (!path.empty() && g_pplayer->m_audioPlayer->PlayMusic(path.string()))
      {
         g_pplayer->m_audioPlayer->SetMusicVolume(m_pt->m_TableMusicVolume * volume);
      }
      else
      {
         PLOGE << "Failed to stream music: " << musicNameStr;
      }
   }
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndMusic()
{
   if (g_pplayer)
      g_pplayer->m_audioPlayer->PauseMusic();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_MusicVolume(float volume)
{
   if (g_pplayer)
      g_pplayer->m_audioPlayer->SetMusicVolume(volume);
   return S_OK;
}

const WCHAR *ScriptGlobalTable::get_Name() const
{
   return L"Global";
}

STDMETHODIMP ScriptGlobalTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Global");
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftFlipperKey(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetRightFlipperActionId() :  g_pplayer->m_pininput.GetLeftFlipperActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightFlipperKey(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetLeftFlipperActionId() : g_pplayer->m_pininput.GetRightFlipperActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StagedLeftFlipperKey(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetStagedRightFlipperActionId() : g_pplayer->m_pininput.GetStagedLeftFlipperActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StagedRightFlipperKey(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetStagedLeftFlipperActionId() : g_pplayer->m_pininput.GetStagedRightFlipperActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftTiltKey(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetRightNudgeActionId() : g_pplayer->m_pininput.GetLeftNudgeActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightTiltKey(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetLeftNudgeActionId() : g_pplayer->m_pininput.GetRightNudgeActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_CenterTiltKey(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetCenterNudgeActionId();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlungerKey(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetLaunchBallActionId();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StartGameKey(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetStartActionId();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetAddCreditActionId(0);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey2(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetAddCreditActionId(1);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_MechanicalTilt(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetTiltActionId();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftMagnaSave(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetRightMagnaActionId() : g_pplayer->m_pininput.GetLeftMagnaActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightMagnaSave(LONG *pVal)
{
   *pVal = 0x10000 | (g_pplayer->m_ptable->m_tblMirrorEnabled ? g_pplayer->m_pininput.GetLeftMagnaActionId() : g_pplayer->m_pininput.GetRightMagnaActionId());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ExitGame(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetExitGameActionId();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LockbarKey(LONG *pVal)
{
   *pVal = 0x10000 | g_pplayer->m_pininput.GetLockbarActionId();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_JoyCustomKey(LONG index, LONG *pVal)
{
   if (index < 1 || index > 4)
      return E_FAIL;
   *pVal = 0x10000 | g_pplayer->m_pininput.GetJoyCustomActionId(index - 1);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VPXActionKey(LONG index, LONG *pVal)
{
   switch (index)
   {
   case 0: *pVal = 0x10000 | g_pplayer->m_pininput.GetLeftFlipperActionId(); break;
   case 1: *pVal = 0x10000 | g_pplayer->m_pininput.GetRightFlipperActionId(); break;
   case 2: *pVal = 0x10000 | g_pplayer->m_pininput.GetStagedLeftFlipperActionId(); break;
   case 3: *pVal = 0x10000 | g_pplayer->m_pininput.GetStagedRightFlipperActionId(); break;
   case 4: *pVal = 0x10000 | g_pplayer->m_pininput.GetLeftNudgeActionId(); break;
   case 5: *pVal = 0x10000 | g_pplayer->m_pininput.GetRightNudgeActionId(); break;
   case 6: *pVal = 0x10000 | g_pplayer->m_pininput.GetCenterNudgeActionId(); break;
   case 7: *pVal = 0x10000 | g_pplayer->m_pininput.GetLaunchBallActionId(); break;
   case 8: *pVal = 0x10000 | g_pplayer->m_pininput.GetStartActionId(); break;
   case 9: *pVal = 0x10000 | g_pplayer->m_pininput.GetAddCreditActionId(0); break;
   case 10: *pVal = 0x10000 | g_pplayer->m_pininput.GetAddCreditActionId(1); break;
   case 11: *pVal = 0x10000 | g_pplayer->m_pininput.GetAddCreditActionId(2); break;
   case 12: *pVal = 0x10000 | g_pplayer->m_pininput.GetAddCreditActionId(3); break;
   case 13: *pVal = 0x10000 | g_pplayer->m_pininput.GetTiltActionId(); break;
   case 14: *pVal = 0x10000 | g_pplayer->m_pininput.GetLeftMagnaActionId(); break;
   case 15: *pVal = 0x10000 | g_pplayer->m_pininput.GetRightMagnaActionId(); break;
   case 16: *pVal = 0x10000 | g_pplayer->m_pininput.GetExitGameActionId(); break;
   case 17: *pVal = 0x10000 | g_pplayer->m_pininput.GetExitInteractiveActionId(); break;
   case 18: *pVal = 0x10000 | g_pplayer->m_pininput.GetLockbarActionId(); break;
   case 19: *pVal = 0x10000 | g_pplayer->m_pininput.GetResetActionId(); break;
   case 20: *pVal = 0x10000 | g_pplayer->m_pininput.GetVolumeDownActionId(); break;
   case 21: *pVal = 0x10000 | g_pplayer->m_pininput.GetVolumeUpActionId(); break;
   case 22: *pVal = 0x10000 | g_pplayer->m_pininput.GetExtraBallActionId(); break;
   case 23: *pVal = 0x10000 | g_pplayer->m_pininput.GetSlamTiltActionId(); break;
   case 24: *pVal = 0x10000 | g_pplayer->m_pininput.GetCoinDoorActionId(); break;
   case 25: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(0); break;
   case 26: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(1); break;
   case 27: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(2); break;
   case 28: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(3); break;
   case 29: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(4); break;
   case 30: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(5); break;
   case 31: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(6); break;
   case 32: *pVal = 0x10000 | g_pplayer->m_pininput.GetServiceActionId(7); break;
   // 33-63 reserved for future use
   case 64: *pVal = 0x10000 | g_pplayer->m_pininput.GetJoyCustomActionId(0); break;
   case 65: *pVal = 0x10000 | g_pplayer->m_pininput.GetJoyCustomActionId(1); break;
   case 66: *pVal = 0x10000 | g_pplayer->m_pininput.GetJoyCustomActionId(2); break;
   case 67: *pVal = 0x10000 | g_pplayer->m_pininput.GetJoyCustomActionId(3); break;
   default: return E_FAIL;
   }
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetCustomParam(LONG index, BSTR *param)
{
   if (index <= 0 || index > MAX_CUSTOM_PARAM_INDEX)
      return E_FAIL;

   *param = SysAllocString(m_vpinball->m_customParameters[index-1].c_str());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Setting(BSTR Section, BSTR SettingName, BSTR *param)
{
   const string sectionSz = MakeString(Section);
   const string settingSz = MakeString(SettingName);
   Settings &settings = g_pplayer ? g_pplayer->m_ptable->m_settings : g_pvp->m_settings;
   const auto propId = Settings::GetRegistry().GetPropertyId(sectionSz, settingSz);
   if (propId.has_value())
   {
      string value;
      switch (Settings::GetRegistry().GetProperty(propId.value())->m_type)
      {
      case VPX::Properties::PropertyDef::Type::Float: value = f2sz(settings.GetBool(propId.value()), false); break;
      case VPX::Properties::PropertyDef::Type::Int: value = std::to_string(settings.GetInt(propId.value())); break;
      case VPX::Properties::PropertyDef::Type::Bool: value = settings.GetBool(propId.value()) ? "1"s : "0"s; break;
      case VPX::Properties::PropertyDef::Type::Enum: value = std::to_string(settings.GetInt(propId.value())); break;
      case VPX::Properties::PropertyDef::Type::String: value = settings.GetString(propId.value()); break;
      default: return E_FAIL;
      }
      *param = MakeWideBSTR(value);
      return S_OK;
   }
   return E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::GetTextFile(BSTR FileName, BSTR *pContents)
{
   const string szFileName = MakeString(FileName);
   const std::filesystem::path filepath = normalize_path_separators(szFileName);
   if (std::filesystem::path file = g_pvp->SearchScript(g_pplayer ? g_pplayer->m_ptable : g_pvp->GetActiveTable(), filepath); !file.empty())
   {
      std::ifstream scriptFile;
      scriptFile.open(file, std::ifstream::in);
      if (scriptFile.is_open())
      {
         std::stringstream buffer;
         buffer << scriptFile.rdbuf();
         string content = buffer.str();
         if (szFileName.ends_with(".vbs"))
            content = VPXPluginAPIImpl::GetInstance().ApplyScriptCOMObjectOverrides(content);
         *pContents = MakeWideBSTR(content);
         return S_OK;
      }
   }
   PLOGE << "Unable to load file: " << szFileName;
   return E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::get_UserDirectory(BSTR *pVal)
{
   auto table = g_pplayer ? g_pplayer->m_ptable : g_pvp->GetActiveTable();
   if (table == nullptr)
      return E_FAIL;
   const string path = g_pvp->GetTablePath(table, VPinball::TableSubFolder::User, true).string() + PATH_SEPARATOR_CHAR;
   if (!DirExists(path))
      return E_FAIL;
   *pVal = MakeWideBSTR(path);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_TablesDirectory(BSTR *pVal)
{
   string szPath = m_vpinball->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR;
   if (!DirExists(szPath))
      return E_FAIL;
   *pVal = MakeWideBSTR(szPath);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_MusicDirectory(VARIANT pSubDir, BSTR *pVal)
{
   // Optional sub directory parameter must be either missing or a string
   if (V_VT(&pSubDir) != VT_ERROR && V_VT(&pSubDir) != VT_EMPTY && V_VT(&pSubDir) != VT_BSTR)
      return E_FAIL;
   const string childDir = V_VT(&pSubDir) == VT_BSTR ? (MakeString(V_BSTR(&pSubDir)) + PATH_SEPARATOR_CHAR) : string();
   PinTable* table = g_pplayer ? g_pplayer->m_ptable : g_pvp->GetActiveTable();
   if (table == nullptr)
      return E_FAIL;
   const string path = (g_pvp->GetTablePath(table, VPinball::TableSubFolder::Music, false) / childDir).string() + PATH_SEPARATOR_CHAR;
   if (!DirExists(path))
      return E_FAIL;
   *pVal = MakeWideBSTR(path);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ScriptsDirectory(BSTR *pVal)
{
   const string path = g_pvp->GetAppPath(VPinball::AppSubFolder::Scripts).string() + PATH_SEPARATOR_CHAR;
   if (!DirExists(path))
      return E_FAIL;
   *pVal = MakeWideBSTR(path);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlatformOS(BSTR *pVal)
{
   *pVal = MakeWideBSTR(GET_PLATFORM_OS);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlatformCPU(BSTR *pVal)
{
   *pVal = MakeWideBSTR(GET_PLATFORM_CPU);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlatformBits(BSTR *pVal)
{
   *pVal = MakeWideBSTR(GET_PLATFORM_BITS);
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
STDMETHODIMP ScriptGlobalTable::get_GetPlayerHWnd(LONG *pVal)
#endif
{
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   #ifdef _WIN32
      *pVal = (size_t)g_pplayer->m_playfieldWnd->GetNativeHWND();
   #else
      *pVal = NULL;
   #endif
   return S_OK; // returning E_FAIL would break all PinMAME tables that starts PinMAME through 'Controller.Run GetPlayerHWnd'
}

STDMETHODIMP ScriptGlobalTable::AddObject(BSTR Name, IDispatch *pdisp)
{
   if (!g_pplayer)
      return E_FAIL;

   m_pt->m_pcv->AddTemporaryItem(Name, pdisp);

   return S_OK;
}

// Variant helpers
#define SetVarBstr(_pvar,_bstr) {VariantClear(_pvar);V_VT(_pvar)=VT_BSTR;V_BSTR(_pvar)=(_bstr);}

static BSTR BstrFromVariant(VARIANT *pvar, LCID lcid)
{
   VARIANT var;
   VariantInit(&var);

   if (V_VT(pvar) == VT_BOOL)
   {
      const OLECHAR *pszBool = (V_BOOL(pvar)) ? L"True" : L"False";
      V_BSTR(&var) = SysAllocString(pszBool);

      return V_BSTR(&var);
   }
   else
   {
      // Ask OLEAUT32 to handle this.
      const HRESULT hr = ::VariantChangeTypeEx(&var, pvar, lcid, 0, VT_BSTR);
      if (hr == S_OK)
      {
         assert(V_VT(&var) == VT_BSTR);
         return V_BSTR(&var);
      }
      else
         return nullptr;
   }
}

STDMETHODIMP ScriptGlobalTable::SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value)
{
   HRESULT hr;

#ifndef __STANDALONE__
   const wstring wzPath = MakeWString((m_vpinball->GetTablePath(g_pplayer->m_ptable, VPinball::TableSubFolder::User, true) / "VPReg.stg").string());

   IStorage *pstgRoot;
   if (FAILED(hr = StgOpenStorage(wzPath.c_str(), nullptr, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgRoot)))
   {
      // Registry file does not exist - create it
      if (FAILED(hr = StgCreateDocfile(wzPath.c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
      {
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
   pstmValue->Write((WCHAR *)bstr, (uint32_t)/*wcslen*/ SysStringLen(bstr) * (uint32_t)sizeof(WCHAR), &writ);

   SysFreeString(bstr);

   pstmValue->Release();

   pstgTable->Commit(STGC_DEFAULT);
   pstgTable->Release();

   pstgRoot->Commit(STGC_DEFAULT);
   pstgRoot->Release();
#else
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   string szIniPath = pSettings->GetStandalone_VPRegPath();
   if (!szIniPath.empty()) {
      if (szIniPath == "."s + PATH_SEPARATOR_CHAR)
         szIniPath = PathFromFilename(g_pplayer->m_ptable->m_filename);
      else if (!szIniPath.ends_with(PATH_SEPARATOR_CHAR))
         szIniPath += PATH_SEPARATOR_CHAR;
   }
   else
      szIniPath = m_vpinball->GetAppPath(VPinball::AppSubFolder::Preferences).string() + PATH_SEPARATOR_CHAR;

   mINI::INIStructure ini;
   mINI::INIFile file(szIniPath + "VPReg.ini");
   file.read(ini);

   string szTableName = MakeString(TableName);
   string szValueName = MakeString(ValueName);
   string szValue;

   BSTR bstr = BstrFromVariant(&Value, 0x409);

   if (bstr) {
      szValue = MakeString(bstr);
      SysFreeString(bstr);
   }

   ini[szTableName][szValueName] = szValue;

   file.write(ini);

   PLOGD << "TableName=" << szTableName << ", ValueName=" << szValueName << ", Value=" << szValue;
#endif
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::LoadValue(BSTR TableName, BSTR ValueName, VARIANT *Value)
{
   HRESULT hr;

#ifndef __STANDALONE__
   const wstring wzPath = MakeWString((m_vpinball->GetTablePath(g_pplayer->m_ptable, VPinball::TableSubFolder::User, false) / "VPReg.stg").string());

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

   const unsigned int size = statstg.cbSize.LowPart / sizeof(WCHAR);

   BSTR wzT = SysAllocStringLen(nullptr,size);

   DWORD read;
   hr = pstmValue->Read(wzT, size * (int)sizeof(WCHAR), &read);
   wzT[size] = L'\0';

   pstmValue->Release();

   pstgTable->Commit(STGC_DEFAULT);
   pstgTable->Release();

   pstgRoot->Commit(STGC_DEFAULT);
   pstgRoot->Release();

   SetVarBstr(Value, wzT);
#else
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   string szIniPath = pSettings->GetStandalone_VPRegPath();
   if (!szIniPath.empty()) {
      if (szIniPath == "."s + PATH_SEPARATOR_CHAR)
         szIniPath = PathFromFilename(g_pplayer->m_ptable->m_filename);
      else if (!szIniPath.ends_with(PATH_SEPARATOR_CHAR))
         szIniPath += PATH_SEPARATOR_CHAR;
   }
   else
      szIniPath = m_vpinball->GetAppPath(VPinball::AppSubFolder::Preferences).string() + PATH_SEPARATOR_CHAR;

   mINI::INIStructure ini;
   mINI::INIFile file(szIniPath + "VPReg.ini");
   file.read(ini);

   string szTableName = MakeString(TableName);
   string szValueName = MakeString(ValueName);

   if (ini.has(szTableName) && ini[szTableName].has(szValueName)) {
      SetVarBstr(Value, MakeWideBSTR(ini[szTableName][szValueName]));
   }
   else
      SetVarBstr(Value, SysAllocString(L""));

   string szValue = MakeString(V_BSTR(Value));

   PLOGD << "TableName=" << szTableName << ", ValueName=" << szValueName << ", Value=" << szValue;
#endif
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ActiveBall(IBall **pVal)
{
   if (!pVal || !g_pplayer || !g_pplayer->m_pactiveball)
      return E_POINTER;

   Ball *pBall = g_pplayer->m_pactiveball->m_pBall;

   if (!pBall)
      return E_POINTER;

   pBall->QueryInterface(IID_IBall, (void **)pVal);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_FrameIndex(LONG *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = g_pplayer->m_overall_frames;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_GameTime(LONG *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = g_pplayer->m_time_msec;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PreciseGameTime(double *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = g_pplayer->m_time_sec;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_SystemTime(LONG *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = msec();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_NightDay(int *pVal)
{
   if (g_pplayer)
      *pVal = quantizeUnsignedPercent(g_pplayer->m_renderer->m_sceneLighting.GetGlobalEmissionScale());

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ShowDT(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pt->GetViewMode() == BG_DESKTOP || m_pt->GetViewMode() == BG_FSS); // DT & FSS
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ShowFSS(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pt->GetViewMode() == BG_FSS);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::UpdateMaterial(BSTR pVal, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity,
   OLE_COLOR base, OLE_COLOR glossy, OLE_COLOR clearcoat, VARIANT_BOOL isMetal, VARIANT_BOOL opacityActive,
   float elasticity, float elasticityFalloff, float friction, float scatterAngle)
{
   if (!g_pplayer)
      return E_POINTER;

   const string Name = MakeString(pVal);

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

   const string Name = MakeString(pVal);

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

   const string Name = MakeString(pVal);

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

   const string Name = MakeString(pVal);

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

   Material * const pMat = m_pt->GetMaterial(MakeString(pVal));
   if (pMat != &m_vpinball->m_dummyMaterial)
      pMat->m_cBase = newVal;
   else
      return E_FAIL;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::CreatePluginObject(/*[in]*/ BSTR classId, /*[out, retval]*/ IDispatch** pVal)
{
   VPXPluginAPIImpl &pi = VPXPluginAPIImpl::GetInstance();
   *pVal = pi.CreateCOMPluginObject(MakeString(classId));
   return (*pVal != nullptr) ? S_OK : E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::LoadTexture(BSTR imageName, BSTR fileName)
{
   if (!g_pplayer)
      return E_FAIL;

   const string szImageName = MakeString(imageName);
   // Do not allow to load an image with the same name as one of the edited table as they would conflict
   if (m_pt->GetImage(szImageName))
      return E_FAIL;

   Texture *image = m_pt->ImportImage(MakeString(fileName), szImageName);
   return image == nullptr ? E_FAIL : S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_WindowWidth(int *pVal)
{
   if (g_pplayer)
      *pVal = g_pplayer->m_playfieldWnd->GetWidth();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_WindowHeight(int *pVal)
{
   if (g_pplayer)
      *pVal = g_pplayer->m_playfieldWnd->GetHeight();
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDWidth(int pVal)
{
   if (g_pplayer)
      g_pplayer->m_dmdSize.x = pVal;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDHeight(int pVal)
{
   if (g_pplayer)
      g_pplayer->m_dmdSize.y = pVal;
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDPixels(VARIANT pVal) // assumes VT_UI1 as input //!! use 64bit instead of 8bit to reduce overhead??
{
   #ifdef EXT_CAPTURE
      if (HasDMDCapture()) // If DMD capture is enabled check if external DMD exists
         return S_OK;
   #endif

   SAFEARRAY *psa = V_ARRAY(&pVal);
   if (psa == nullptr || g_pplayer ==nullptr || g_pplayer->m_dmdSize.x <= 0 || g_pplayer->m_dmdSize.y <= 0)
      return E_FAIL;

   BaseTexture::Update(g_pplayer->m_dmdFrame, g_pplayer->m_dmdSize.x, g_pplayer->m_dmdSize.y, BaseTexture::BW_FP32, nullptr);
   const int size = g_pplayer->m_dmdSize.x * g_pplayer->m_dmdSize.y;
   // Convert from linear [0..100] luminance
   VARIANT *p;
   SafeArrayAccessData(psa, (void **)&p);
   float *const __restrict data = static_cast<float*>(g_pplayer->m_dmdFrame->data());
   for (int ofs = 0; ofs < size; ++ofs)
      data[ofs] = (float)V_UI4(&p[ofs]) * (float)(1.0 / 100.);
   SafeArrayUnaccessData(psa);
   g_pplayer->m_dmdFrameId++;
   VPXPluginAPIImpl::GetInstance().UpdateDMDSource(nullptr, true);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DMDColoredPixels(VARIANT pVal) //!! assumes VT_UI4 as input //!! use 64bit instead of 32bit to reduce overhead??
{
   #ifdef EXT_CAPTURE
      if (HasDMDCapture()) // If DMD capture is enabled check if external DMD exists
         return S_OK;
   #endif

   SAFEARRAY *psa = V_ARRAY(&pVal);
   if (psa == nullptr || g_pplayer ==nullptr || g_pplayer->m_dmdSize.x <= 0 || g_pplayer->m_dmdSize.y <= 0)
      return E_FAIL;

   BaseTexture::Update(g_pplayer->m_dmdFrame, g_pplayer->m_dmdSize.x, g_pplayer->m_dmdSize.y, BaseTexture::SRGBA, nullptr);
   const int size = g_pplayer->m_dmdSize.x * g_pplayer->m_dmdSize.y;
   uint32_t *const __restrict data = reinterpret_cast<uint32_t *>(g_pplayer->m_dmdFrame->data());
   // gamma compressed [0..255] sRGB
   VARIANT *p;
   SafeArrayAccessData(psa, (void **)&p);
   for (int ofs = 0; ofs < size; ++ofs)
      data[ofs] = V_UI4(&p[ofs]) | 0xFF000000u;
   SafeArrayUnaccessData(psa);
   g_pplayer->m_dmdFrameId++;
   VPXPluginAPIImpl::GetInstance().UpdateDMDSource(nullptr, true);
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_DisableStaticPrerendering(VARIANT_BOOL *pVal)
{
   if (g_pplayer == nullptr)
      return E_FAIL;

   *pVal = FTOVB(!g_pplayer->m_renderer->IsUsingStaticPrepass());
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::put_DisableStaticPrerendering(VARIANT_BOOL newVal)
{
   if (g_pplayer == nullptr)
      return E_FAIL;

   g_pplayer->m_renderer->DisableStaticPrePass(VBTOb(newVal));
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetBalls(LPSAFEARRAY *pVal)
{
   if (!pVal || !g_pplayer)
      return E_POINTER;

   CComSafeArray<VARIANT> balls((ULONG)g_pplayer->m_vball.size());

   for (size_t i = 0; i < g_pplayer->m_vball.size(); ++i)
   {
      Ball *pBall = g_pplayer->m_vball[i]->m_pBall;

      if (!pBall)
         return E_POINTER;

      CComVariant v = static_cast<IDispatch*>(pBall);
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
#ifndef __STANDALONE__
   return Serial.open(MakeString(device)) ? S_OK : E_FAIL;
#else
   return E_FAIL;
#endif
}

STDMETHODIMP ScriptGlobalTable::CloseSerial()
{
#ifndef __STANDALONE__
   Serial.close();
#endif
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::FlushSerial()
{
#ifndef __STANDALONE__
   Serial.flush();
#endif
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::SetupSerial(int baud, int bits, int parity, int stopbit, VARIANT_BOOL rts, VARIANT_BOOL dtr)
{
#ifndef __STANDALONE__
   Serial.setup(serial::get_baud(baud),serial::get_bits(bits),parity == 0 ? SERIAL_PARITY_NONE : (parity == 1 ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD),serial::get_stopbit(stopbit));
   Serial.set_rts(VBTOb(rts));
   Serial.set_dtr(VBTOb(dtr));
#endif

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::ReadSerial(int size, VARIANT *pVal)
{
#ifndef __STANDALONE__
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
#endif

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::WriteSerial(VARIANT pVal)
{
#ifndef __STANDALONE__
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
#endif
   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::GetSerialDevices(VARIANT *pVal)
{
#ifndef __STANDALONE__
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
#endif

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RenderingMode(int *pVal)
{
   if (g_pplayer->m_renderer->m_stereo3D == STEREO_VR)
      *pVal = 2; // VR
   else if ((g_pplayer->m_renderer->m_stereo3D != STEREO_OFF) && g_pplayer->m_renderer->m_stereo3Denabled)
      *pVal = 1; // Stereo 3D (3DTV or anaglyph)
   else {
#ifndef __STANDALONE__
      *pVal = 0; // 2D
#else
      int val = g_pplayer->m_ptable->m_settings.GetStandalone_RenderingModeOverride();
      *pVal = (val == -1) ? 0 : val;
#endif
   }

   return S_OK;
}
