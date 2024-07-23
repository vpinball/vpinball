#include "stdafx.h"

#include "VPinMAMEController.h"
#include "VPinMAMEGames.h"

#include "mINI/ini.h"

#include "../common/WindowManager.h"

#define PINMAME_SETTINGS_WINDOW_X      15
#define PINMAME_SETTINGS_WINDOW_Y      30 + 218 + 5 + 75 + 5
#define PINMAME_SETTINGS_WINDOW_WIDTH  290
#define PINMAME_SETTINGS_WINDOW_HEIGHT 75
#define PINMAME_ZORDER                 350

void PINMAMECALLBACK VPinMAMEController::GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   if (pController->m_pPinmameGame)
      delete pController->m_pPinmameGame;

   pController->m_pPinmameGame = new PinmameGame();
   memcpy(pController->m_pPinmameGame, pPinmameGame, sizeof(PinmameGame));
}

void PINMAMECALLBACK VPinMAMEController::OnDisplayAvailable(int index, int displayCount, PinmameDisplayLayout *p_displayLayout, const void* pUserData)
{
   PLOGI.printf("index=%d, displayCount=%d, type=%d, top=%d, left=%d, width=%d, height=%d, depth=%d, length=%d", 
      index,
      displayCount,
      p_displayLayout->type, 
      p_displayLayout->top,
      p_displayLayout->left, 
      p_displayLayout->width, 
      p_displayLayout->height, 
      p_displayLayout->depth, 
      p_displayLayout->length);

   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   VPinMAMEDisplay* pDisplay = new VPinMAMEDisplay();
   memset(pDisplay, 0, sizeof(VPinMAMEDisplay));
   pDisplay->layout = *p_displayLayout;
   pDisplay->r = GetRValue(pController->m_dmdColor);
   pDisplay->g = GetGValue(pController->m_dmdColor);
   pDisplay->b = GetBValue(pController->m_dmdColor);

   if (p_displayLayout->type == PINMAME_DISPLAY_TYPE_DMD
      || p_displayLayout->type == (PINMAME_DISPLAY_TYPE_DMD | PINMAME_DISPLAY_TYPE_DMDSEG)
      || p_displayLayout->type == (PINMAME_DISPLAY_TYPE_DMD | PINMAME_DISPLAY_TYPE_DMDNOAA)) {
      pDisplay->pDMD = new DMDUtil::DMD();
      pDisplay->pDMD->SetRomName(pController->m_pPinmameGame->name);

      if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "AltColor"s, true)) {
         string szAltColorPath = find_directory_case_insensitive(pController->m_szPath, "AltColor");
         if (!szAltColorPath.empty())
            pDisplay->pDMD->SetAltColorPath(szAltColorPath.c_str());
      }

      if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "PUPCapture"s, false)) {
         string szPupVideosPath = find_directory_case_insensitive(g_pvp->m_currentTablePath, "pupvideos");
         if (!szPupVideosPath.empty())
            pDisplay->pDMD->SetPUPVideosPath(szPupVideosPath.c_str());
      }

      if (!pController->m_pActiveDisplay) {
         pController->m_pActiveDisplay = pDisplay;
         if (pController->m_pDMDWindow)
            pController->m_pDMDWindow->AttachDMD(pDisplay->pDMD, pDisplay->layout.width, pDisplay->layout.height);

         if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "FindDisplays"s, true))
            pDisplay->pDMD->FindDisplays();

         if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DumpDMDTxt"s, false))
            pDisplay->pDMD->DumpDMDTxt();

         if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DumpDMDRaw"s, false))
            pDisplay->pDMD->DumpDMDRaw();
      }
   }

   pController->m_displays.push_back(pDisplay);
}

void PINMAMECALLBACK VPinMAMEController::OnDisplayUpdated(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout, const void* pUserData)
{
   VPinMAMEDisplay* pDisplay = ((VPinMAMEController*)pUserData)->m_displays[index];

   if (pDisplay->pDMD && p_displayData) {
      VPinMAMEController* pController = (VPinMAMEController*)pUserData;
      pDisplay->pDMD->UpdateData((const UINT8*)p_displayData, pDisplay->layout.depth, pDisplay->layout.width, pDisplay->layout.height, pDisplay->r, pDisplay->g, pDisplay->b);
   }
}

int PINMAMECALLBACK VPinMAMEController::OnAudioAvailable(PinmameAudioInfo* p_audioInfo, const void* pUserData)
{
   PLOGI.printf("format=%d, channels=%d, sampleRate=%.2f, framesPerSecond=%.2f, samplesPerFrame=%d, bufferSize=%d", 
      p_audioInfo->format,
      p_audioInfo->channels,
      p_audioInfo->sampleRate,
      p_audioInfo->framesPerSecond,
      p_audioInfo->samplesPerFrame,
      p_audioInfo->bufferSize);

   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   pController->m_pAudioPlayer = new AudioPlayer();
   pController->m_pAudioPlayer->StreamInit(p_audioInfo->sampleRate, p_audioInfo->channels, 1.);
   pController->m_audioChannels = p_audioInfo->channels;

   return p_audioInfo->samplesPerFrame;
}

int PINMAMECALLBACK VPinMAMEController::OnAudioUpdated(void* p_buffer, int samples, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   if (pController->m_enableSound)
      pController->m_pAudioPlayer->StreamUpdate(p_buffer, samples * pController->m_audioChannels * sizeof(int16_t));

   return samples;
}

void PINMAMECALLBACK VPinMAMEController::OnLogMessage(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, const void* pUserData)
{
   char buffer[4096];
   vsnprintf(buffer, sizeof(buffer), format, args);

   if (logLevel == PINMAME_LOG_LEVEL_INFO) {
      PLOGI.printf("%s", buffer);
   }
   else if (logLevel == PINMAME_LOG_LEVEL_ERROR) {
      PLOGE.printf("%s", buffer);
   }
}

void PINMAMECALLBACK VPinMAMEController::OnSoundCommand(int boardNo, int cmd, const void* pUserData)
{
   AltsoundProcessCommand(cmd, 0);
}

VPinMAMEController::VPinMAMEController()
{
   PinmameConfig config = { 
      PINMAME_AUDIO_FORMAT_INT16,
      44100, 
      "", 
      NULL,
      &VPinMAMEController::OnDisplayAvailable,
      &VPinMAMEController::OnDisplayUpdated,
      &VPinMAMEController::OnAudioAvailable,
      &VPinMAMEController::OnAudioUpdated,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      &VPinMAMEController::OnLogMessage,
      &VPinMAMEController::OnSoundCommand,
   };

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   m_szPath = find_directory_case_insensitive(g_pvp->m_currentTablePath, "pinmame");
   if (m_szPath.empty()) {
      m_szPath = pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEPath"s, ""s);

      if (!m_szPath.empty()) {
         if (!m_szPath.ends_with(PATH_SEPARATOR_CHAR))
            m_szPath += PATH_SEPARATOR_CHAR;
      }
      else {
#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
         m_szPath = find_directory_case_insensitive(g_pvp->m_szMyPath, "pinmame");
#else
         m_szPath = string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".pinmame" + PATH_SEPARATOR_CHAR;
#endif
      }
   }

   strncpy_s((char*)config.vpmPath, m_szPath.c_str(), PINMAME_MAX_PATH - 1);

   PLOGI.printf("PinMAME path set to: %s", m_szPath.c_str());

   m_szIniPath = find_directory_case_insensitive(m_szPath, "ini");
   if (m_szIniPath.empty()) {
      m_szIniPath = m_szPath + "ini" + PATH_SEPARATOR_CHAR;
      std::filesystem::create_directory(m_szIniPath);
   }

   PLOGI.printf("PinMAME ini path set to: %s", m_szIniPath.c_str());

   PinmameSetConfig(&config);
   PinmameSetUserData((void*)this);
   PinmameSetDmdMode(PINMAME_DMD_MODE_RAW);
   PinmameSetSoundMode(PINMAME_SOUND_MODE_DEFAULT);

   PinmameSetHandleKeyboard(0);
   PinmameSetHandleMechanics(0xFF);

   m_pSolenoidBuffer = new PinmameSolenoidState[PinmameGetMaxSolenoids()];
   m_pLampBuffer = new PinmameLampState[PinmameGetMaxLamps()];
   m_pGIBuffer = new PinmameGIState[PinmameGetMaxGIs()];
   m_pLEDBuffer = new PinmameLEDState[PinmameGetMaxLEDs()];
   m_pSoundCommandBuffer = new PinmameSoundCommand[PinmameGetMaxSoundCommands()];
   m_pNVRAMBuffer = new PinmameNVRAMState[PinmameGetMaxNVRAM()];

   m_pGames = new VPinMAMEGames(this);
   m_pGames->AddRef();

   m_pPinmameGame = NULL;
   m_pPinmameMechConfig = NULL;

   m_pActiveDisplay = nullptr;

   m_pDMDWindow = nullptr;

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindow"s, true)) {
      m_pDMDWindow = new VP::DMDWindow("PinMAME",
         pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowX"s, PINMAME_SETTINGS_WINDOW_X),
         pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowY"s, PINMAME_SETTINGS_WINDOW_Y),
         pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowWidth"s, PINMAME_SETTINGS_WINDOW_WIDTH),
         pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowHeight"s, PINMAME_SETTINGS_WINDOW_HEIGHT),
         PINMAME_ZORDER,
         pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowRotation"s, 0));
   }
   else {
      PLOGI.printf("PinMAME window disabled");
   }

   m_pAudioPlayer = nullptr;

   m_hidden = true;

   m_pRGB24DMD = nullptr;
   m_pLevelDMD = nullptr;

   m_running = false;
   m_pThread = nullptr;
}

VPinMAMEController::~VPinMAMEController()
{
   if (PinmameIsRunning())
      PinmameStop();

   m_running = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }

   if (m_pDMDWindow) {
      if (m_pDMDWindow->IsDMDAttached())
         m_pDMDWindow->DetachDMD();

      delete m_pDMDWindow;
   }

   for (auto pDisplay : m_displays) {
      if (pDisplay->pDMD)
         delete pDisplay->pDMD;
   }

   m_displays.clear();

   delete m_pSolenoidBuffer;
   delete m_pLampBuffer;
   delete m_pGIBuffer;
   delete m_pLEDBuffer;
   delete m_pSoundCommandBuffer;
   delete m_pNVRAMBuffer;
   delete m_pPinmameGame;
   delete m_pPinmameMechConfig;
   delete m_pAudioPlayer;

   m_pGames->Release();
}

STDMETHODIMP VPinMAMEController::Run(/*[in]*/ LONG_PTR hParentWnd, /*[in,defaultvalue(100)]*/ int nMinVersion)
{
   if (m_pPinmameGame) {
      mINI::INIStructure ini;
      mINI::INIFile file(m_szIniPath + m_pPinmameGame->name + ".ini");

      int cheat = 0;
      m_enableSound = 1;
      m_dmdColor = RGB(255, 88, 32);

      if (file.read(ini)) {
         if (ini.has("settings")) {
            if (ini["settings"].has("cheat"))
               cheat = atoll(ini["settings"]["cheat"].c_str());

            if (ini["settings"].has("sound"))
               m_enableSound = atoll(ini["settings"]["sound"].c_str());

            if (ini["settings"].has("dmd_red") && ini["settings"].has("dmd_green") && ini["settings"].has("dmd_blue"))
               m_dmdColor = RGB(atoll(ini["settings"]["dmd_red"].c_str()), atoll(ini["settings"]["dmd_green"].c_str()), atoll(ini["settings"]["dmd_blue"].c_str()));
         }
      }

      ini["settings"]["cheat"] = std::to_string(cheat);
      ini["settings"]["sound"] = std::to_string(m_enableSound);

      ini["settings"]["dmd_red"] = std::to_string(GetRValue(m_dmdColor));
      ini["settings"]["dmd_green"] = std::to_string(GetGValue(m_dmdColor));
      ini["settings"]["dmd_blue"] = std::to_string(GetBValue(m_dmdColor));

      file.write(ini);

      PinmameSetCheat(cheat);

      PINMAME_STATUS status = PinmameRun(m_pPinmameGame->name);

      if (status == PINMAME_STATUS_OK) {
         int timeout = 0;

         while (!PinmameIsRunning() && timeout < 20) {
            SDL_Delay(75);
            timeout++;
         }

         Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

         if (pSettings->LoadValueWithDefault(Settings::Standalone, "AltSound"s, true)) {
             AltsoundSetLogger(g_pvp->m_szMyPrefPath, ALTSOUND_LOG_LEVEL_INFO, false);

             if (AltsoundInit(m_szPath, string(m_pPinmameGame->name))) {
                AltsoundSetHardwareGen((ALTSOUND_HARDWARE_GEN)PinmameGetHardwareGen());
                PinmameSetSoundMode(PINMAME_SOUND_MODE_ALTSOUND);

                PLOGI.printf("Altsound initialized successfully.");
             }
         }

         if (m_pDMDWindow) {
            if (!m_hidden)
               m_pDMDWindow->Show();
            else
               m_pDMDWindow->Hide();
         }

         return S_OK;
      }

      if (status == PINMAME_STATUS_GAME_ALREADY_RUNNING) {
         PLOGE.printf("Game already running.");
      }
   }

   return S_FALSE;
}

STDMETHODIMP VPinMAMEController::Stop()
{
   PinmameSetTimeFence(0.0);

   if (PinmameIsRunning())
      PinmameStop();

   if (m_pThread) {
      m_running = false;

      m_pThread->join();
      delete m_pThread;
      m_pThread = nullptr;
   }

   if (m_pDMDWindow) {
      if (m_pDMDWindow->IsDMDAttached())
         m_pDMDWindow->DetachDMD();
   }

   for (auto pDisplay : m_displays) {
      if (pDisplay->pDMD)
         delete pDisplay->pDMD;
   }

   m_pActiveDisplay = nullptr;
   m_pLevelDMD = nullptr;
   m_pRGB24DMD = nullptr;

   delete m_pAudioPlayer;
   m_pAudioPlayer = nullptr;

   m_displays.clear();

   m_hidden = true;

   if (m_pDMDWindow)
      m_pDMDWindow->Hide();

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Lamp(int nLamp, VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetLamp(nLamp) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Solenoid(int nSolenoid, VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetSolenoid(nSolenoid) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Switch(int nSwitchNo, VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetSwitch(nSwitchNo) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Switch(int nSwitchNo, VARIANT_BOOL newVal)
{
   PinmameSetSwitch(nSwitchNo, (newVal == VARIANT_TRUE) ? 1 : 0);

   PLOGD.printf("switchNo=%d, newVal=%d", nSwitchNo, newVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Pause(VARIANT_BOOL *pVal)
{
   *pVal = PinmameIsPaused() ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Pause(VARIANT_BOOL newVal)
{
   PinmamePause((newVal == VARIANT_TRUE) ? 1 : 0);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_WPCNumbering(/*[out, retval]*/ VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Lamps(VARIANT* pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_RawDmdWidth(int *pVal)
{
   *pVal = m_pActiveDisplay ? m_pActiveDisplay->layout.width : 0;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_RawDmdHeight(int *pVal)
{
   *pVal = m_pActiveDisplay ? m_pActiveDisplay->layout.height : 0;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_NVRAM(VARIANT* pVal)
{
   int uCount = PinmameGetNVRAM(m_pNVRAMBuffer);
   if (uCount <= 0)
      return S_FALSE;

   SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, uCount);
   VARIANT* pData;

   SafeArrayAccessData(psa, (void **)&pData);

   for (int i = 0; i < uCount; i++) {
      V_VT(&pData[i]) = VT_UI1;
      V_UI1(&pData[i]) = m_pNVRAMBuffer[i].currStat;
   }

   SafeArrayUnaccessData(psa);

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedNVRAM(VARIANT* pVal)
{
   int uCount = PinmameGetChangedNVRAM(m_pNVRAMBuffer);
   if (uCount <= 0) {
      if (uCount == 0) {
         V_VT(pVal) = VT_EMPTY;
         return S_OK;
      }

      return S_FALSE;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 3, 0 } };
   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[3];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pNVRAMBuffer[ix[0]].nvramNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pNVRAMBuffer[ix[0]].currStat;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 2;
      V_I4(&varValue) = m_pNVRAMBuffer[ix[0]].oldStat;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_RawDmdPixels(VARIANT* pVal)
{
   if (!m_pActiveDisplay)
      return S_FALSE;

   if (!m_pLevelDMD) {
        bool isSAM = ((PinmameGetHardwareGen() & (PINMAME_HARDWARE_GEN_SAM | PINMAME_HARDWARE_GEN_SPA)) > 0);
        m_pLevelDMD = m_pActiveDisplay->pDMD->CreateLevelDMD(m_pActiveDisplay->layout.width, m_pActiveDisplay->layout.height, isSAM);
        return S_FALSE;
   }

   const UINT8* pLevelData = m_pLevelDMD->GetData();
 
   if (!pLevelData)
      return S_FALSE;

   const int end = m_pLevelDMD->GetLength();

   SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, end);
   VARIANT* pData;

   SafeArrayAccessData(psa, (void **)&pData);

   for (int i = 0; i < end; i++) {
      V_VT(&pData[i]) = VT_UI1;
      V_UI1(&pData[i]) = pLevelData[i];
   }

   SafeArrayUnaccessData(psa);

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_RawDmdColoredPixels(VARIANT* pVal)
{
   if (!m_pActiveDisplay)
      return S_FALSE;

   if (!m_pRGB24DMD) {
      m_pRGB24DMD = m_pActiveDisplay->pDMD->CreateRGB24DMD(m_pActiveDisplay->layout.width, m_pActiveDisplay->layout.height);
      return S_FALSE;
   }

   const UINT8* pRGB24Data = m_pRGB24DMD->GetData();

   if (!pRGB24Data)
      return S_FALSE;

   const int end = m_pRGB24DMD->GetLength();

   SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, end);
   VARIANT* pData;

   SafeArrayAccessData(psa, (void **)&pData);

   for (int i = 0; i < end; i++) {
      int pos = i * 3;
      UINT8 r = pRGB24Data[pos];
      UINT8 g = pRGB24Data[pos + 1];
      UINT8 b = pRGB24Data[pos + 2];

      V_VT(&pData[i]) = VT_UI4;
      V_UI4(&pData[i]) = r | g << 8 | b << 16;
   }

   SafeArrayUnaccessData(psa);

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_DmdWidth(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_DmdHeight(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_DmdPixel(int x, int y, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_updateDmdPixels(int **buf, int width, int height, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedLampsState(int **buf, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_LampsState(int **buf, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedSolenoidsState(int **buf, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_SolenoidsState(int **buf, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedGIsState(int **buf, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Switches(VARIANT* pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Switches(VARIANT newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GameName(BSTR *pVal)
{
   const WCHAR *const wzGameName = MakeWide(string(m_pPinmameGame->name));
   *pVal = SysAllocString(wzGameName);
   delete [] wzGameName;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_GameName(BSTR newVal)
{
   char szGameName[MAXSTRING];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szGameName, sizeof(szGameName), NULL, NULL);
   PLOGI.printf("newVal=%s", szGameName);

   PINMAME_STATUS status = PinmameGetGame(szGameName, &VPinMAMEController::GetGameCallback, this);

   if (status == PINMAME_STATUS_OK) {
      PLOGI.printf("Game found: name=%s, description=%s, manufacturer=%s, year=%s",
         m_pPinmameGame->name, m_pPinmameGame->description, m_pPinmameGame->manufacturer, m_pPinmameGame->year);

      m_hidden = false;

      return S_OK;
   }

   if (status == PINMAME_STATUS_GAME_ALREADY_RUNNING) {
      PLOGE.printf("Game already running.");
   }
   else if (status == PINMAME_STATUS_GAME_NOT_FOUND) {
      PLOGE.printf("Game name not found.");
   }

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ROMName(BSTR *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_HandleKeyboard(VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetHandleKeyboard() ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_HandleKeyboard(VARIANT_BOOL newVal)
{
   PinmameSetHandleKeyboard((newVal == VARIANT_TRUE));

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Machines(BSTR sMachine, VARIANT* pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Running(VARIANT_BOOL *pVal) { 
   return PinmameIsRunning() ? VARIANT_TRUE : VARIANT_FALSE; 
}

STDMETHODIMP VPinMAMEController::get_ChangedLamps(VARIANT* pVal)
{
   int uCount = PinmameGetChangedLamps(m_pLampBuffer);
   if (uCount <= 0) {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pLampBuffer[ix[0]].lampNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pLampBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedLEDs(int nHigh, int nLow, int nnHigh, int nnLow, VARIANT* pVal)
{
   uint64_t mask = ((uint64_t)nHigh << 32) | (uint64_t)nLow;
   uint64_t mask2 = ((uint64_t)nnHigh << 32) | (uint64_t)nnLow;
    
   int uCount = PinmameGetChangedLEDs(mask, mask2, m_pLEDBuffer);
   if (uCount <= 0) {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 3, 0 } };
   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[3];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pLEDBuffer[ix[0]].ledNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pLEDBuffer[ix[0]].chgSeg;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 2;
      V_I4(&varValue) = m_pLEDBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedLEDsState(int nHigh, int nLow, int nnHigh, int nnLow, int **buf, int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::ShowAboutDialog(LONG_PTR hParentWnd)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_HandleMechanics(int *pVal)
{
   *pVal = PinmameGetHandleMechanics();

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_HandleMechanics(int newVal)
{
   PinmameSetHandleMechanics(newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GetMech(int mechNo, int *pVal)
{
   *pVal = PinmameGetMech(mechNo);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Mech(int mechNo, int newVal) {
   if (!m_pPinmameMechConfig)
      m_pPinmameMechConfig = new PinmameMechConfig();

   switch(mechNo) {
      case 0:
         PinmameSetMech(newVal, m_pPinmameMechConfig);

         delete m_pPinmameMechConfig;
         m_pPinmameMechConfig = NULL;

         break;
      case 1:
         m_pPinmameMechConfig->sol1 = newVal;
         break;
      case 2:
         m_pPinmameMechConfig->sol2 = newVal;
         break;
      case 3:
         m_pPinmameMechConfig->length = newVal;
         break;
      case 4:
         m_pPinmameMechConfig->steps = newVal;
         break;
      case 5:
         m_pPinmameMechConfig->type = newVal;
         break;
      case 6:
         m_pPinmameMechConfig->acc = newVal;
         break;
      case 7:
         m_pPinmameMechConfig->ret = newVal;
         break;
      case 8:
         m_pPinmameMechConfig->initialPos = newVal + 1;
         break;
      default:
         if (mechNo % 10 == 0)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].swNo = newVal;
         else if (mechNo % 10 == 1)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].startPos = newVal;
         else if (mechNo % 10 == 2)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].endPos = newVal;
         else if (mechNo % 10 == 3)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].pulse = newVal;
         break;
   }

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GIString(int nString, int *pVal)
{
   *pVal = PinmameGetGI(nString);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedGIStrings(VARIANT* pVal)
{
   int uCount = PinmameGetChangedGIs(m_pGIBuffer);
   if (uCount <= 0) {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pGIBuffer[ix[0]].giNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pGIBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedSolenoids(VARIANT* pVal)
{
   int uCount = PinmameGetChangedSolenoids(m_pSolenoidBuffer);
   if (uCount <= 0)
   {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pSolenoidBuffer[ix[0]].solNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pSolenoidBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_SplashInfoLine(BSTR *pVal)
{
   const WCHAR *const wzSplashInfoLine = MakeWide(m_szSplashInfoLine);
   *pVal = SysAllocString(wzSplashInfoLine);
   delete [] wzSplashInfoLine;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_SplashInfoLine(BSTR newVal)
{
   m_szSplashInfoLine = MakeString(newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Solenoids(VARIANT* pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Dip(int nNo, int *pVal)
{
   *pVal = PinmameGetDIP(nNo);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Dip(int nNo, int newVal)
{
   PinmameSetDIP(nNo, newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GIStrings(VARIANT* pVal)
{
   int maxGIs = PinmameGetMaxGIs();

   SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, maxGIs);
   VARIANT* pData;

   SafeArrayAccessData(psa, (void**)&pData);

   for (int i = 0; i < maxGIs; ++i) {
      V_VT(&pData[i]) = VT_I4;
      V_I4(&pData[i]) = PinmameGetGI(i);
   }

   SafeArrayUnaccessData(psa);

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_LockDisplay(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_LockDisplay(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_SolMask(int nLow, LONG *pVal)
{
   if (!pVal)
      return S_FALSE;

   *pVal = PinmameGetSolenoidMask(nLow);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_SolMask(int nLow, LONG newVal)
{
   if (!((0 <= nLow && nLow <= 2) || (1000 <= nLow && nLow < 2999)))
      return S_FALSE;

   PinmameSetSolenoidMask(nLow, newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ModOutputType(int output, int no, int* pVal)
{
   if (output != (int)PINMAME_MOD_OUTPUT_TYPE_SOLENOID)
      return S_FALSE;

   *pVal = (int)PinmameGetModOutputType(output, no);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ModOutputType(int output, int no, int newVal)
{
   if (output != (int)PINMAME_MOD_OUTPUT_TYPE_SOLENOID)
      return S_FALSE;

   PinmameSetModOutputType(output, no, (PINMAME_MOD_OUTPUT_TYPE)newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_TimeFence(double timeInS)
{
   PinmameSetTimeFence(timeInS);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Version(BSTR *pVal)
{
   int nVersionNo0 = 99;
   int nVersionNo1 = 99;
   int nVersionNo2 = 99;
   int nVersionNo3 = 99;

   char szVersion[MAXTOKEN];
   sprintf_s(szVersion, sizeof(szVersion), "%02i%02i%02i%02i", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);

   WCHAR wzVersion[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, szVersion, -1, wzVersion, MAXTOKEN);

   *pVal = SysAllocString(wzVersion);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Games(IGames **pVal) 
{ 
   return m_pGames->QueryInterface(IID_IGames, (void**) pVal);
}

STDMETHODIMP VPinMAMEController::get_Settings(IControllerSettings **pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_NewSoundCommands(VARIANT* pVal)
{
   int uCount = PinmameGetNewSoundCommands(m_pSoundCommandBuffer);
   if (uCount <= 0)
   {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pSoundCommandBuffer[ix[0]].sndNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = 0;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::SetDisplayPosition(int x, int y, LONG_PTR hParentWindow)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::ShowOptsDialog(LONG_PTR hParentWnd)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowDMDOnly(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowDMDOnly(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowTitle(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowTitle(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowFrame(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowFrame(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_DoubleSize(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_DoubleSize(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::CheckROMS(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::ShowPathesDialog(LONG_PTR hParentWnd)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Hidden(VARIANT_BOOL *pVal)
{
   *pVal = m_hidden ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Hidden(VARIANT_BOOL newVal)
{
   m_hidden = (newVal == VARIANT_TRUE);

   if (m_pDMDWindow) {
      if (!m_hidden)
         m_pDMDWindow->Show();
      else
         m_pDMDWindow->Hide();
   }

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Game(IGame **pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_MechSamples(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_MechSamples(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::GetWindowRect(LONG_PTR hWnd, VARIANT* pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::GetClientRect(LONG_PTR hWnd, VARIANT* pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_MasterVolume(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_MasterVolume(int newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_EnumAudioDevices(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_AudioDevicesCount(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_AudioDeviceDescription(int num, BSTR *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_AudioDeviceModule(int num, BSTR *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_CurrentAudioDevice(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_CurrentAudioDevice(int newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_FastFrames(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_FastFrames(int newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_IgnoreRomCrc(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_IgnoreRomCrc(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_SoundMode(int *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_SoundMode(int newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_CabinetMode(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_CabinetMode(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowPinDMD(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowPinDMD(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowWinDMD(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowWinDMD(VARIANT_BOOL newVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}
