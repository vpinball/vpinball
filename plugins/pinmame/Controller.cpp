// license:GPLv3+

#include "Controller.h"
#include "Game.h"
#include <thread>

#include "plugins/VPXPlugin.h" // Only used for optional feature (visual feedback on error)

namespace PinMAME {

Controller::Controller(const MsgPluginAPI* api, unsigned int endpointId, PinmameConfig& config)
   : m_msgApi(api)
   , m_endpointId(endpointId)
{
   PinmameSetConfig(&config);
   // PinmameSetDmdMode(PINMAME_DMD_MODE_RAW); // Unneeded as we use libpinmame controller messages
   PinmameSetHandleKeyboard(0);
   PinmameSetHandleMechanics(0xFF);

   m_vpmPath = config.vpmPath;

   m_getDmdSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   m_onDmdChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdChangedMsgId, OnDmdSrcChanged, this);
}

Controller::~Controller()
{
   Stop();
   m_msgApi->UnsubscribeMsg(m_onDmdChangedMsgId, OnDmdSrcChanged);
   m_msgApi->ReleaseMsgID(m_onDmdChangedMsgId);
   m_msgApi->ReleaseMsgID(m_getDmdSrcMsgId);
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
   m_szGameName = name;
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

   PinmameSetCheat(m_cheat);

   // Trigger startup, status will be either 2 (staring), 1 (running), 0  (stopped, likely after failure)
   PINMAME_STATUS status = PinmameRun(m_pPinmameGame->name);
   while (PinmameIsRunning() == 2) // Wait until the machine is either running or stopped
      std::this_thread::sleep_for(std::chrono::milliseconds(75)); 

   if ((PinmameIsRunning() == 1) && status == PINMAME_STATUS_OK) {
      if (m_onGameStartHandler)
         m_onGameStartHandler(this);
   }
   else
   {
      LOGE("Failed to start emulation of rom '%s'", m_pPinmameGame->name);
      VPXPluginAPI* vpxApi = nullptr;
      unsigned int getVpxApiId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
      m_msgApi->BroadcastMsg(m_endpointId, getVpxApiId, &vpxApi);
      m_msgApi->ReleaseMsgID(getVpxApiId);
      if (vpxApi)
         vpxApi->PushNotification(("Failed to start emulation of rom '"s + m_pPinmameGame->name + '\'').c_str(), 10000);
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
      m_pPinmameMechConfig = nullptr;
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

vector<uint8_t> Controller::GetNVRAM() const
{
   vector<PinmameNVRAMState> nvramBuffer(PinmameGetMaxNVRAM());
   const int count = PinmameGetNVRAM(nvramBuffer.data());
   if (count <= 0)
      return vector<uint8_t>();

   vector<uint8_t> buffer(count);
   for (int i = 0; i < count; i++)
      buffer[i] = nvramBuffer[i].currStat;

   return buffer;
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

void Controller::OnDmdSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   Controller* me = static_cast<Controller*>(userData);
   me->m_defaultDmd.id.id = 0;
}

void Controller::UpdateDmdSrc()
{
   if (m_defaultDmd.id.id == 0)
   {
      unsigned int largest = 128;
      GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
      m_msgApi->BroadcastMsg(m_endpointId, m_getDmdSrcMsgId, &getSrcMsg);
      vector<DisplaySrcId> displaySources(getSrcMsg.count);
      getSrcMsg = { getSrcMsg.count, 0, displaySources.data() };
      m_msgApi->BroadcastMsg(m_endpointId, m_getDmdSrcMsgId, &getSrcMsg);
      for (const DisplaySrcId& src : displaySources)
      {
         if (src.id.endpointId == m_endpointId && src.width >= largest)
         {
            m_defaultDmd = src;
            largest = src.width;
         }
      }
   }
}

int Controller::GetRawDmdWidth()
{
   UpdateDmdSrc();
   return m_defaultDmd.id.id != 0 ? m_defaultDmd.width : 0;
}

int Controller::GetRawDmdHeight()
{
   UpdateDmdSrc();
   return m_defaultDmd.id.id != 0 ? m_defaultDmd.height : 0;
}

std::vector<uint8_t> Controller::GetRawDmdPixels()
{
   UpdateDmdSrc();
   std::vector<uint8_t> pixels;
   if (m_defaultDmd.id.id == 0)
      return pixels;
   const DisplayFrame frame = m_defaultDmd.GetRenderFrame(m_defaultDmd.id);
   const int size = m_defaultDmd.width * m_defaultDmd.height;
   if (m_defaultDmd.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
   {
      pixels.resize(size);
      for (int i = 0; i < size; i++)
         pixels[i] = static_cast<uint8_t>(static_cast<const float*>(frame.frame)[i] * 100.f);
   }
   else if (m_defaultDmd.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
   {
      pixels.resize(size);
      for (int i = 0; i < size; i++)
         pixels[i] = static_cast<uint8_t>(21.26f * (float)static_cast<const uint8_t*>(frame.frame)[i * 3] + 71.52f * (float)static_cast<const uint8_t*>(frame.frame)[i * 3 + 1]
            + 7.22f * (float)static_cast<const uint8_t*>(frame.frame)[i * 3 + 2]);
   }
   return pixels;
}

std::vector<uint32_t> Controller::GetRawDmdColoredPixels()
{
   UpdateDmdSrc();
   std::vector<uint32_t> pixels;
   if (m_defaultDmd.id.id == 0)
      return pixels;
   const DisplayFrame frame = m_defaultDmd.GetRenderFrame(m_defaultDmd.id);
   const int size = m_defaultDmd.width * m_defaultDmd.height;
   if (m_defaultDmd.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
   {
      pixels.resize(size);
      for (int i = 0; i < size; i++)
      {
         // TODO implement original PinMame / VPinMame coloring
         const uint32_t lum = static_cast<int32_t>(static_cast<const float*>(frame.frame)[i] * 255.f);
         pixels[i] = (lum << 16) | (lum << 8) | lum;
      }
   }
   else if (m_defaultDmd.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
   {
      pixels.resize(size);
      for (int i = 0; i < size; i++)
         pixels[i] = ((uint32_t)static_cast<const uint8_t*>(frame.frame)[i * 3] << 16) 
            | ((uint32_t)static_cast<const uint8_t*>(frame.frame)[i * 3 + 1] << 8)
            | (static_cast<const uint8_t*>(frame.frame)[i * 3 + 2]);
   }
   return pixels;
}

}
