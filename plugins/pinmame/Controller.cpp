#include "Controller.h"
#include "Game.h"
#include <thread>

Controller::Controller(PinmameConfig& config)
{
   PinmameSetConfig(&config);
   PinmameSetSoundMode(PINMAME_SOUND_MODE_DEFAULT);
   // PinmameSetDmdMode(PINMAME_DMD_MODE_RAW); // Unneeded as we use state blocks
   PinmameSetHandleKeyboard(0);
   PinmameSetHandleMechanics(0xFF);

   m_vpmPath = config.vpmPath;
}

Controller::~Controller()
{
   if (m_onDestroyHandler)
      m_onDestroyHandler(this);
   delete m_pPinmameGame;
   delete m_pPinmameMechConfig;
}

string Controller::GetVersion() const
{
   // TODO libpinmame should report its version
   int nVersionNo0 = 03;
   int nVersionNo1 = 07;
   int nVersionNo2 = 00;
   int nVersionNo3 = 00;
   char szVersion[8 + 1];
   #ifdef _MSC_VER
   snprintf(szVersion, sizeof(szVersion), "%02i%02i%02i%02i", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);
   #else
   snprintf(szVersion, sizeof(szVersion), "%02i%02i%02i%02i", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);
   #endif
   return string(szVersion);
}

Game* Controller::GetGames(const string& name) const
{
   struct GameCBData
   {
      const Controller* controller;
      Game* game;
   };
   GameCBData cbData { this, nullptr };
   PinmameGetGame(name.c_str(), [](PinmameGame* pPinmameGame, void* const pUserData)
      {
         GameCBData* pGame = static_cast<GameCBData*>(pUserData);
         pGame->game = new Game(const_cast<Controller*>(pGame->controller), *pPinmameGame);
      }, &cbData);
   return cbData.game;
}

void Controller::SetGameName(const string& name)
{
   delete m_pPinmameGame;
   m_pPinmameGame = nullptr;
   PINMAME_STATUS status = PinmameGetGame(name.c_str(), [](PinmameGame* pPinmameGame, void* const pUserData) {
      Controller* me = static_cast<Controller*>(pUserData);
      me->m_pPinmameGame = new PinmameGame();
      memcpy(me->m_pPinmameGame, pPinmameGame, sizeof(PinmameGame));
   }, this);
   if (status == PINMAME_STATUS_OK)
   {
      LOGI("Game found: name=%s, description=%s, manufacturer=%s, year=%s", m_pPinmameGame->name, m_pPinmameGame->description, m_pPinmameGame->manufacturer, m_pPinmameGame->year);
      //m_hidden = false;
   }
   else if (status == PINMAME_STATUS_GAME_ALREADY_RUNNING)
   {
      PSC_FAIL("Game already running.");
   }
   else if (status == PINMAME_STATUS_GAME_NOT_FOUND)
   {
      PSC_FAIL("Game name not found.");
   }
   else if (status == PINMAME_STATUS_CONFIG_NOT_SET)
   {
      PSC_FAIL("Config is not set.");
   }
}

void Controller::Run(long hParentWnd, int nMinVersion)
{
   if (m_pPinmameGame == nullptr)
      return;
   
   /*mINI::INIStructure ini;
   mINI::INIFile file(m_szIniPath + m_pPinmameGame->name + ".ini");

   int cheat = 0;
   m_enableSound = 1;
   m_dmdColor = RGB(255, 88, 32);

   if (file.read(ini)) {
      if (ini.has("settings"s)) {
         if (ini["settings"s].has("cheat"s))
            cheat = atoll(ini["settings"s]["cheat"s].c_str());

         if (ini["settings"s].has("sound"s))
            m_enableSound = atoll(ini["settings"s]["sound"s].c_str());

         if (ini["settings"s].has("dmd_red"s) && ini["settings"s].has("dmd_green"s) && ini["settings"s].has("dmd_blue"s))
            m_dmdColor = RGB(atoll(ini["settings"s]["dmd_red"s].c_str()), atoll(ini["settings"s]["dmd_green"s].c_str()), atoll(ini["settings"s]["dmd_blue"s].c_str()));
      }
   }

   ini["settings"s]["cheat"s] = std::to_string(cheat);
   ini["settings"s]["sound"s] = std::to_string(m_enableSound);

   ini["settings"s]["dmd_red"s] = std::to_string(GetRValue(m_dmdColor));
   ini["settings"s]["dmd_green"s] = std::to_string(GetGValue(m_dmdColor));
   ini["settings"s]["dmd_blue"s] = std::to_string(GetBValue(m_dmdColor));

   file.write(ini);

   PinmameSetCheat(cheat);*/

   // Trigger startup, status will be either 2 (staring), 1 (running), 0  (stopped, likely after failure)
   PINMAME_STATUS status = PinmameRun(m_pPinmameGame->name);
   while (PinmameIsRunning() == 2) // Wait until the machine is either running or stopped
      std::this_thread::sleep_for(std::chrono::milliseconds(75)); 

   if ((PinmameIsRunning() == 1) && status == PINMAME_STATUS_OK) {
      /*Settings* const pSettings = &g_pplayer->m_ptable->m_settings;
      if (pSettings->LoadValueWithDefault(Settings::Standalone, "AltSound"s, true)) {
          AltsoundSetLogger(g_pvp->m_szMyPrefPath, ALTSOUND_LOG_LEVEL_INFO, false);

          if (AltsoundInit(m_szPath, string(m_pPinmameGame->name))) {
             AltsoundSetHardwareGen((ALTSOUND_HARDWARE_GEN)PinmameGetHardwareGen());
             PinmameSetSoundMode(PINMAME_SOUND_MODE_ALTSOUND);

             LOGI("Altsound initialized successfully.");
          }
      }*/

      if (m_onGameStartHandler)
         m_onGameStartHandler(this);
   }
   if (status == PINMAME_STATUS_GAME_ALREADY_RUNNING) {
      LOGE("Game already running.");
   }
}

void Controller::Stop()
{
   PinmameSetTimeFence(0.0);
   if (PinmameIsRunning())
   {
      PinmameStop();
      while (PinmameIsRunning() != 0) // Wait until the machine is stopped
         std::this_thread::sleep_for(std::chrono::milliseconds(75));
      if (m_onGameEndHandler)
         m_onGameEndHandler(this);
   }
   m_stateBlock = nullptr;
}

void Controller::SetMech(int mechNo, int newVal)
{
   if (!m_pPinmameMechConfig)
      m_pPinmameMechConfig = new PinmameMechConfig();

   switch (mechNo)
   {
   case 0:
      PinmameSetMech(newVal, m_pPinmameMechConfig);
      delete m_pPinmameMechConfig;
      m_pPinmameMechConfig = NULL;
      break;
   case 1: m_pPinmameMechConfig->sol1 = newVal; break;
   case 2: m_pPinmameMechConfig->sol2 = newVal; break;
   case 3: m_pPinmameMechConfig->length = newVal; break;
   case 4: m_pPinmameMechConfig->steps = newVal; break;
   case 5: m_pPinmameMechConfig->type = newVal; break;
   case 6: m_pPinmameMechConfig->acc = newVal; break;
   case 7: m_pPinmameMechConfig->ret = newVal; break;
   case 8: m_pPinmameMechConfig->initialPos = newVal + 1; break;
   default:
      if (mechNo % 10 == 0)
         m_pPinmameMechConfig->sw[(mechNo / 10) - 1].swNo = newVal;
      else if (mechNo % 10 == 1)
         m_pPinmameMechConfig->sw[(mechNo / 10) - 1].startPos = newVal;
      else if (mechNo % 10 == 2)
         m_pPinmameMechConfig->sw[(mechNo / 10) - 1].endPos = newVal;
      else if (mechNo % 10 == 3)
         m_pPinmameMechConfig->sw[(mechNo / 10) - 1].pulse = newVal;
      break;
   }
}

const vector<PinmameNVRAMState>& Controller::GetChangedNVRAM()
{
   m_nvramStates.resize(PinmameGetMaxNVRAM()); // TODO we should use the actual size of the running machine
   int count = PinmameGetChangedNVRAM(m_nvramStates.data());
   if (count < 0) // report error ?
      count = 0;
   m_nvramStates.resize(count);
   return m_nvramStates;
}

const vector<PinmameSoundCommand>& Controller::GetNewSoundCommands()
{
   m_soundCommands.resize(PinmameGetMaxSoundCommands());
   int count = PinmameGetNewSoundCommands(m_soundCommands.data());
   if (count < 0) // report error ?
      count = 0;
   m_soundCommands.resize(count);
   return m_soundCommands;
}

const vector<PinmameLampState>& Controller::GetChangedLamps()
{
   m_lampStates.resize(PinmameGetMaxLamps()); // TODO we should use the actual size of the running machine
   int count = PinmameGetChangedLamps(m_lampStates.data());
   if (count < 0) // report error ?
      count = 0;
   m_lampStates.resize(count);
   return m_lampStates;
}

const vector<PinmameLEDState>& Controller::GetChangedLEDs(int nHigh, int nLow, int nnHigh, int nnLow)
{
   m_ledStates.resize(PinmameGetMaxLEDs()); // TODO we should use the actual size of the running machine
   uint64_t mask = ((uint64_t)nHigh << 32) | (uint64_t)nLow;
   uint64_t mask2 = ((uint64_t)nnHigh << 32) | (uint64_t)nnLow;
   int count = PinmameGetChangedLEDs(mask, mask2, m_ledStates.data());
   if (count < 0) // report error ?
      count = 0;
   m_ledStates.resize(count);
   return m_ledStates;
}

const vector<PinmameGIState>& Controller::GetChangedGIStrings()
{
   m_giStates.resize(PinmameGetMaxGIs()); // TODO we should use the actual size of the running machine
   int count = PinmameGetChangedGIs(m_giStates.data());
   if (count < 0) // report error ?
      count = 0;
   m_giStates.resize(count);
   return m_giStates;
}

const vector<PinmameSolenoidState>& Controller::GetChangedSolenoids()
{
   m_solenoidStates.resize(PinmameGetMaxSolenoids()); // TODO we should use the actual size of the running machine
   int count = PinmameGetChangedSolenoids(m_solenoidStates.data());
   if (count < 0) // report error ?
      count = 0;
   m_solenoidStates.resize(count);
   return m_solenoidStates;
}

int Controller::GetRawDmdWidth() const
{
   GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (m_stateBlock == nullptr)
      return 0;
   pinmame_tDisplayStates* state = m_stateBlock->displayStates;
   if (state == nullptr)
      return 0;
   const pinmame_tFrameState* const frame = (pinmame_tFrameState*)((uint8_t*)state + sizeof(pinmame_tDisplayStates));
   for (unsigned int index = 0; index < state->nDisplays; index++)
      if (frame->width >= 128)
         return frame->width;
   return 0;
}

int Controller::GetRawDmdHeight() const
{
   GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (m_stateBlock == nullptr)
      return 0;
   pinmame_tDisplayStates* state = m_stateBlock->displayStates;
   if (state == nullptr)
      return 0;
   const pinmame_tFrameState* const frame = (pinmame_tFrameState*)((uint8_t*)state + sizeof(pinmame_tDisplayStates));
   for (unsigned int index = 0; index < state->nDisplays; index++)
      if (frame->width >= 128)
         return frame->height;
   return 0;
}

std::vector<uint8_t> Controller::GetRawDmdPixels() const
{
   std::vector<uint8_t> pixels;
   GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (m_stateBlock == nullptr)
      return pixels;
   pinmame_tDisplayStates* state = m_stateBlock->displayStates;
   if (state == nullptr)
      return pixels;
   const pinmame_tFrameState* const frame = (pinmame_tFrameState*)((uint8_t*)state + sizeof(pinmame_tDisplayStates));
   for (unsigned int index = 0; index < state->nDisplays; index++)
      if (frame->width >= 128)
      {
         const int size = frame->width * frame->height;
         pixels.resize(size);
         switch (frame->dataFormat)
         {
         case PINMAME_FRAME_FORMAT_LUM:
            for (int i = 0; i < size; i++)
               pixels[i] = (uint32_t)frame->frameData[i] * 100u / 255u;
            break;
         case PINMAME_FRAME_FORMAT_RGB:
            for (int i = 0; i < size; i++)
               pixels[i] = static_cast<uint8_t>(21.26f * (float)frame->frameData[i * 3] + 71.52f * (float)frame->frameData[i * 3 + 1] + 7.22f * (float)frame->frameData[i * 3 + 2]);
            break;
         default:
            pixels.resize(0);
            break;
         }
         break;
      }
   return pixels;
}

std::vector<uint32_t> Controller::GetRawDmdColoredPixels() const
{
   std::vector<uint32_t> pixels;
   GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (m_stateBlock == nullptr)
      return pixels;
   pinmame_tDisplayStates* state = m_stateBlock->displayStates;
   if (state == nullptr)
      return pixels;
   const pinmame_tFrameState* const frame = (pinmame_tFrameState*)((uint8_t*)state + sizeof(pinmame_tDisplayStates));
   for (unsigned int index = 0; index < state->nDisplays; index++)
      if (frame->width >= 128)
      {
         const int size = frame->width * frame->height;
         pixels.resize(size);
         switch (frame->dataFormat)
         {
         case PINMAME_FRAME_FORMAT_LUM:
            for (int i = 0; i < size; i++)
            {
               // TODO implement original PinMame / VPinMame coloring
               const uint32_t lum = frame->frameData[i];
               pixels[i] = (lum << 16) | (lum << 8) | lum;
            }
            break;
         case PINMAME_FRAME_FORMAT_RGB:
            for (int i = 0; i < size; i++)
               pixels[i] = ((uint32_t)frame->frameData[i * 3] << 16) | ((uint32_t)frame->frameData[i * 3 + 1] << 8) | (frame->frameData[i * 3 + 2]);
            break;
         default:
            pixels.resize(0);
            break;
         }
         break;
      }
   return pixels;
}
