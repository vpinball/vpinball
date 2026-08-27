// license:GPLv3+

#include "Controller.h"
#include "Game.h"
#include "Settings.h"
#include "pinmame/PinMAMEPlugin.h"

#include "nlohmann/json.hpp"

#include <thread>
#include <format>
#include <fstream>

#include "plugins/VPXPlugin.h" // Only used for optional feature (visual feedback on error)
#include <climits>

using json = nlohmann::json;

namespace PinMAME
{

__forceinline uint8_t saturatedByte(float v) { return (uint8_t)(255.0f * (v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v)); }

Controller::Controller(const MsgPluginAPI* api, unsigned int endpointId, const PinmameConfig& config, const std::filesystem::path& memmapPath)
   : m_vpmPath(config.vpmPath)
   , m_memmapPath(memmapPath)
   , m_msgApi(api)
   , m_endpointId(endpointId)
   , m_threadLock(std::this_thread::get_id())
   , m_pinmameConfig({ })
   , m_stateSources(
        api, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG,
        [this](std::vector<StateSrcId>& stateSources) { std::erase_if(stateSources, [this](const StateSrcId& src) { return src.id.endpointId != m_endpointId; }); },
        [this]() { m_stateSources.With([this](const std::vector<StateSrcId>& stateSources) { OnStateSrcChanged({ }); }); },
        [this]() { m_stateSources.With([this](const std::vector<StateSrcId>& stateSources) { OnStateSrcChanged(stateSources); }); })
   {
   memcpy(&m_pinmameConfig, &config, sizeof(m_pinmameConfig));
   memcpy(const_cast<char*>(m_pinmameConfig.vpmPath), config.vpmPath, sizeof(m_pinmameConfig.vpmPath));

   PinmameSetConfig(&m_pinmameConfig);
   PinmameSetHandleKeyboard(0);
   PinmameSetHandleMechanics(0xFF);

   m_getDmdSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   m_onDmdChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdChangedMsgId, OnDmdSrcChanged, this);
}

Controller::~Controller()
{
   assert(m_threadLock == std::this_thread::get_id());

   Stop();

   m_msgApi->UnsubscribeMsg(m_onDmdChangedMsgId, OnDmdSrcChanged, this);
   m_msgApi->ReleaseMsgID(m_onDmdChangedMsgId);
   m_msgApi->ReleaseMsgID(m_getDmdSrcMsgId);

   if (m_onDestroyHandler)
      m_onDestroyHandler(this);
   for (const auto& settings : m_gameSettings)
      settings.second->Release();
   if (m_settings)
      m_settings->Release();
   delete m_pPinmameMechConfig;
}

string Controller::GetVersion() const
{
   // TODO libpinmame should report its version
   constexpr int nVersionNo0 = 03;
   constexpr int nVersionNo1 = 07;
   constexpr int nVersionNo2 = 00;
   constexpr int nVersionNo3 = 00;
   static const string version = std::format("{:02d}{:02d}{:02d}{:02d}", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);
   return version;
}

Game* Controller::GetGames(const string& name) const
{
   GameSettings* settings;
   if (const auto it = m_gameSettings.find(name); it != m_gameSettings.end())
      settings = it->second;
   else
   {
      // shared settings instance so values written through one Game object
      // are seen by later Games(name) accesses
      settings = new GameSettings();
      m_gameSettings[name] = settings;
   }
   struct GameCBData
   {
      const Controller* controller;
      GameSettings* settings;
      Game* game;
   };
   GameCBData cbData { this, settings, nullptr };
   PinmameGetGame(
      name.c_str(),
      [](PinmameGame* pPinmameGame, void* const pUserData)
      {
         GameCBData* pGame = static_cast<GameCBData*>(pUserData);
         pGame->game = new Game(const_cast<Controller*>(pGame->controller), *pPinmameGame, pGame->settings);
      },
      &cbData);
   return cbData.game;
}

Settings* Controller::GetSettings()
{
   if (m_settings == nullptr)
      m_settings = new Settings();
   m_settings->AddRef();
   return m_settings;
}

void Controller::SetGameName(const string& name)
{
   m_szGameName = name;
   m_szRomName.clear();
   PINMAME_STATUS status = PinmameGetGame(
      name.c_str(),
      [](PinmameGame* pPinmameGame, void* const pUserData)
      {
         Controller* me = static_cast<Controller*>(pUserData);
         me->m_szRomName = pPinmameGame->name;
         LOGI(std::format("Game found: name={}, description={}, manufacturer={}, year={}", pPinmameGame->name, pPinmameGame->description, pPinmameGame->manufacturer, pPinmameGame->year));
      },
      this);
   if (status == PINMAME_STATUS_OK)
   {
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
   if (m_szRomName.empty())
      return;

   PinmameSetCheat(m_cheat);

   // Disable sound if requested through game's settings object
   Game* game = GetGames(m_szGameName);
   if (game)
   {
      GameSettings* settings = game->GetSettings();
      int sound = settings->GetValue("sound");
      if (sound == 0)
      {
         PinmameConfig* pinmameConfig = new PinmameConfig({ });
         memcpy(pinmameConfig, &m_pinmameConfig, sizeof(m_pinmameConfig));
         memcpy(const_cast<char*>(pinmameConfig->vpmPath), m_pinmameConfig.vpmPath, sizeof(m_pinmameConfig.vpmPath));
         *const_cast<int*>(&pinmameConfig->sampleRate) = 0;
         PinmameSetConfig(pinmameConfig);
         delete pinmameConfig;
      }
      settings->Release();
      game->Release();
   }

   // Search and load a memory map with its platform if provided (see https://github.com/tomlogic/pinmame-nvram-maps)
   if (std::error_code ec; std::filesystem::exists(m_memmapPath, ec))
   {
      std::filesystem::recursive_directory_iterator it(m_memmapPath, std::filesystem::directory_options::skip_permission_denied);
      std::filesystem::recursive_directory_iterator end;
      for (; it != end; ++it)
      {
         if (!it->is_regular_file(ec) || ec)
            continue;
         if (it->path().filename() == "index.json")
         {
            std::ifstream indexFile(it->path());
            if (indexFile.is_open())
            {
               json index;
               try
               {
                  indexFile >> index;
                  if (index.is_object() && index.contains(m_szRomName) && index[m_szRomName].is_string())
                  {
                     // Load memmap
                     const std::filesystem::path subPath(index[m_szRomName].get<string>());
                     std::ifstream memmapFile(m_memmapPath / subPath, std::ios::binary | std::ios::ate);
                     std::streamsize memmapSize = memmapFile.tellg();
                     memmapFile.seekg(0, std::ios::beg);
                     vector<uint8_t> memmap(memmapSize);
                     memmapFile.read(reinterpret_cast<char*>(memmap.data()), memmapSize);

                     // Find platform reference and loads it (if any)
                     vector<uint8_t> platform;
                     string memmapString(memmap.data(), memmap.data() + memmapSize);
                     json memMapDef = json::parse(memmapString);
                     if (memMapDef.is_object() && memMapDef.contains("_metadata") && memMapDef["_metadata"].is_object() && memMapDef["_metadata"].contains("platform")
                        && memMapDef["_metadata"]["platform"].is_string())
                     {
                        string platformFilename = memMapDef["_metadata"]["platform"].get<string>() + ".json";
                        if (!platformFilename.empty() && std::filesystem::exists(m_memmapPath / "platforms" / platformFilename))
                        {
                           std::ifstream platformFile(m_memmapPath / "platforms" / platformFilename, std::ios::binary | std::ios::ate);
                           std::streamsize platformSize = platformFile.tellg();
                           platformFile.seekg(0, std::ios::beg);
                           platform.resize(platformSize);
                           platformFile.read(reinterpret_cast<char*>(platform.data()), platformSize);
                        }
                     }

                     PinmameSetMemMap(platform.data(), platform.size(), memmap.data(), memmap.size());
                  }
               }
               catch (const json::parse_error& e)
               {
                  LOGE("JSON parse error while parsing memmap in "s + it->path().string() + ": " + e.what());
               }
            }
            break;
         }
      }
   }

   // Trigger startup, status will be either 2 (starting), 1 (running), 0 (stopped, likely after failure)
   PINMAME_STATUS status = PinmameRun(m_szGameName.c_str());
   while (PinmameIsRunning() == 2) // Wait until the machine is either running or stopped
      std::this_thread::sleep_for(std::chrono::milliseconds(75));

   if ((PinmameIsRunning() == 1) && status == PINMAME_STATUS_OK)
   {
      if (m_onGameStartHandler)
         m_onGameStartHandler(this);
   }
   else
   {
      LOGE("Failed to start emulation of rom '"s + m_szRomName + '\'');
      VPXPluginAPI* vpxApi = nullptr;
      unsigned int getVpxApiId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
      m_msgApi->BroadcastMsg(m_endpointId, getVpxApiId, &vpxApi);
      m_msgApi->ReleaseMsgID(getVpxApiId);
      if (vpxApi)
         vpxApi->PushNotification(("Failed to start emulation of rom '"s + m_szRomName + '\'').c_str(), 10000);
   }
   if (status == PINMAME_STATUS_GAME_ALREADY_RUNNING)
   {
      LOGE("Game already running."s);
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

// Inputs

// Some PinMAME drivers defines a virtual matrix column for cabinet switches and use negative indices to access it (Whitestar for example)
static constexpr int SWITCH_OFFSET = 16;

void Controller::OnStateSrcChanged(const std::vector<StateSrcId>& stateSources)
{
   m_switches = { };
   m_switchMap.clear();
   m_dipSwitches = { };
   m_dipSwitchMap.clear();
   m_solenoids = { };
   m_solenoidMap.clear();
   m_gis = { };
   m_giMap.clear();
   m_lamps = { };
   m_lampMap.clear();
   for (const StateSrcId& src : stateSources)
   {
      switch (src.id.resId)
      {
      case PMPI_GROUP_SWITCH:
         m_switches = src;
         for (unsigned int i = 0; i < m_switches.nStates; i++)
         {
            if (m_switches.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_UINT8 && m_switches.stateDefs[i].GetState != nullptr && m_switches.stateDefs[i].SetState != nullptr)
            {
               const int switchOfs = static_cast<int16_t>(m_switches.stateDefs[i].mappingId) + SWITCH_OFFSET;
               assert(switchOfs >= 0);
               if (m_switchMap.size() < switchOfs + 1)
                  m_switchMap.resize(switchOfs + 1, UINT_MAX);
               m_switchMap[switchOfs] = i;
               if (switchOfs < m_switchStates.size())
               {
                  uint8_t bv = m_switchStates[switchOfs] ? 0xFF : 0;
                  m_switches.stateDefs[i].SetState(m_switches.id, i, &bv);
               }
            }
         }
         break;

      case PMPI_GROUP_DIPSWITCH:
         m_dipSwitches = src;
         for (unsigned int i = 0; i < src.nStates; i++)
         {
            if (src.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_UINT8 && m_dipSwitches.stateDefs[i].GetState != nullptr && m_dipSwitches.stateDefs[i].SetState != nullptr)
            {
               if (m_dipSwitchMap.size() < m_dipSwitches.stateDefs[i].mappingId + 1)
                  m_dipSwitchMap.resize(m_dipSwitches.stateDefs[i].mappingId + 1, UINT_MAX);
               m_dipSwitchMap[m_dipSwitches.stateDefs[i].mappingId] = i;
               if (m_dipSwitches.stateDefs[i].mappingId < m_dipSwitchStates.size())
               {
                  // Applied cached DIP switch states that may have been defined before starting the machine
                  uint8_t bv = m_dipSwitchStates[m_dipSwitches.stateDefs[i].mappingId] ? 0xFF : 0;
                  m_dipSwitches.stateDefs[i].SetState(m_dipSwitches.id, i, &bv);
               }
            }
         }
         break;

      case PMPI_GROUP_VPM_SOLENOID:
         m_solenoids = src;
         m_prevSolenoidStates.assign(m_solenoids.nStates, 0);
         for (unsigned int i = 0; i < m_solenoids.nStates; i++)
         {
            if (m_solenoids.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_UINT8 && m_solenoids.stateDefs[i].GetState != nullptr)
            {
               if (m_solenoidMap.size() < m_solenoids.stateDefs[i].mappingId + 1)
                  m_solenoidMap.resize(m_solenoids.stateDefs[i].mappingId + 1, UINT_MAX);
               m_solenoidMap[m_solenoids.stateDefs[i].mappingId] = i;
            }
         }
         break;

      case PMPI_GROUP_VPM_GI:
         m_gis = src;
         m_prevGIStates.assign(m_gis.nStates, 0);
         for (unsigned int i = 0; i < m_gis.nStates; i++)
         {
            if (m_gis.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_UINT8 && m_gis.stateDefs[i].GetState != nullptr)
            {
               if (m_giMap.size() < m_gis.stateDefs[i].mappingId + 1)
                  m_giMap.resize(m_gis.stateDefs[i].mappingId + 1, UINT_MAX);
               m_giMap[m_gis.stateDefs[i].mappingId] = i;
            }
         }
         break;

      case PMPI_GROUP_VPM_LAMP:
         m_lamps = src;
         m_prevLampStates.assign(m_lamps.nStates, 0);
         for (unsigned int i = 0; i < m_lamps.nStates; i++)
         {
            if (m_lamps.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_UINT8 && m_lamps.stateDefs[i].GetState != nullptr)
            {
               if (m_lampMap.size() < m_lamps.stateDefs[i].mappingId + 1)
                  m_lampMap.resize(m_lamps.stateDefs[i].mappingId + 1, UINT_MAX);
               m_lampMap[m_lamps.stateDefs[i].mappingId] = i;
            }
         }
         break;

      // TODO Mech
      case PMPI_GROUP_MECH: break;

      case PMPI_GROUP_VPM_MECH: break;
      }
   }
}

bool Controller::GetSwitch(int switchNo) const
{
   const int switchNoOfs = switchNo + SWITCH_OFFSET;
   if (switchNoOfs < 0)
      return false;

   bool swState = false;
   m_stateSources.With(
      [this, &switchNoOfs, &swState](const std::vector<StateSrcId>& states)
      {
         if (switchNoOfs < m_switchMap.size())
         {
            if (const unsigned int index = m_switchMap[switchNoOfs]; index < m_switches.nStates && m_switches.stateDefs[index].GetState != nullptr)
            {
               uint8_t state = 0;
               m_switches.stateDefs[index].GetState(m_switches.id, index, &state);
               swState = state != 0;
               return;
            }
         }
         if (switchNoOfs < m_switchStates.size())
            swState = m_switchStates[switchNoOfs];
      });
   return swState;
}

void Controller::SetSwitch(int switchNo, bool state)
{
   const int switchNoOfs = switchNo + SWITCH_OFFSET;
   if (switchNoOfs < 0)
      return;

   m_stateSources.With(
      [this, &switchNoOfs, &state](const std::vector<StateSrcId>& states)
      {
         if (m_switchStates.size() < switchNoOfs + 1)
            m_switchStates.resize(switchNoOfs + 1, false);
         m_switchStates[switchNoOfs] = state;

         if (switchNoOfs < m_switchMap.size())
            if (const unsigned int index = m_switchMap[switchNoOfs]; index < m_switches.nStates && m_switches.stateDefs[index].SetState != nullptr)
            {
               uint8_t bv = state ? 0xFF : 0;
               m_switches.stateDefs[index].SetState(m_switches.id, index, &bv);
            }
      });
}

int Controller::GetDip(int nDipBank) const
{
   if (nDipBank < 0)
      return false;

   uint8_t result = 0;
   m_stateSources.With(
      [this, &nDipBank, &result](const std::vector<StateSrcId>& states)
      {
         uint8_t state = 0;
         for (int i = 0; i < 8; i++)
         {
            if (const int dipSwitchNo = nDipBank * 8 + i; dipSwitchNo < m_dipSwitchMap.size())
            {
               if (const unsigned int index = m_dipSwitchMap[dipSwitchNo]; index < m_dipSwitches.nStates && m_dipSwitches.stateDefs[index].GetState != nullptr)
               {
                  m_dipSwitches.stateDefs[index].GetState(m_dipSwitches.id, index, &state);
                  if (state != 0)
                     result |= 1 << i;
               }
            }
            else if (dipSwitchNo < m_dipSwitchStates.size())
            {
               if (m_dipSwitchStates[dipSwitchNo])
                  result |= 1 << i;
            }
         }
      });
   return result;
}

void Controller::SetDip(int nDipBank, int byteState)
{
   if (nDipBank < 0)
      return;

   m_stateSources.With(
      [this, &nDipBank, &byteState](const std::vector<StateSrcId>& states)
      {
         for (int i = 0; i < 8; i++)
         {
            const int dipSwitchNo = nDipBank * 8 + i;
            bool state = (byteState & (1 << i)) != 0;

            if (m_dipSwitchStates.size() < dipSwitchNo + 1)
               m_dipSwitchStates.resize(dipSwitchNo + 1, false);
            m_dipSwitchStates[dipSwitchNo] = state;

            if (dipSwitchNo < m_dipSwitchMap.size())
            {
               if (const unsigned int index = m_dipSwitchMap[dipSwitchNo]; index < m_dipSwitches.nStates && m_dipSwitches.stateDefs[index].SetState != nullptr)
               {
                  uint8_t bv = (state != 0) ? 0xFF : 0;
                  m_dipSwitches.stateDefs[index].SetState(m_dipSwitches.id, index, &bv);
               }
            }
         }
      });
}

long Controller::GetSolMask(int nLow) const
{
   switch (nLow)
   {
   case 0: return m_solMask & 0x0FFFFFFFFULL;
   case 1: return (m_solMask >> 32) & 0x0FFFFFFFFULL;
   case 2: return m_deviceMode;
   default: return -1;
   }
}

void Controller::SetSolMask(int nLow, long newVal)
{
   switch (nLow)
   {
   case 0: m_solMask = (m_solMask & 0xFFFFFFFF00000000ULL) | newVal; break;
   case 1: m_solMask = (m_solMask & 0x00000000FFFFFFFFULL) | (((uint64_t)newVal) << 32); break;
   case 2:
      if (DM_BINARY <= newVal && newVal <= DM_PHYSOUT)
      {
         m_deviceMode = (DeviceMode)newVal;
         PinmameSetSolenoidMask(2, newVal);
      }
      break;
   }
}

int Controller::GetModOutputType(int output, int no) const
{
   return output != static_cast<PINMAME_MOD_OUTPUT_TYPE>(PINMAME_MOD_OUTPUT_TYPE_SOLENOID) ? 0 : PinmameGetModOutputType(output, no);
}

void Controller::SetModOutputType(int output, int no, int newVal)
{
   if (output == static_cast<PINMAME_MOD_OUTPUT_TYPE>(PINMAME_MOD_OUTPUT_TYPE_SOLENOID))
      PinmameSetModOutputType(output, no, static_cast<PINMAME_MOD_OUTPUT_TYPE>(newVal));
}

int Controller::GetSolenoid(int solenoid) const
{
   uint8_t solState = 0;
   m_stateSources.With([this, &solenoid, &solState](const std::vector<StateSrcId>& states) {
      if (solenoid >= 0 && solenoid < m_solenoidMap.size())
         if (const unsigned int index = m_solenoidMap[solenoid]; index < m_solenoids.nStates)
               m_solenoids.stateDefs[index].GetState(m_solenoids.id, index, &solState);
      });
   return solState;
}

int Controller::GetLamp(int lamp) const
{
   uint8_t lampState = 0;
   m_stateSources.With(
      [this, &lamp, &lampState](const std::vector<StateSrcId>& states)
      {
         if (lamp >= 0 && lamp < m_lampMap.size())
            if (const unsigned int index = m_lampMap[lamp]; index < m_lamps.nStates)
               m_lamps.stateDefs[index].GetState(m_lamps.id, index, &lampState);
      });
   return lampState;
}

int Controller::GetGIString(int giString) const
{
   uint8_t giState = 0;
   m_stateSources.With(
      [this, &giString, &giState](const std::vector<StateSrcId>& states)
      {
         if (giString >= 0 && giString < m_giMap.size())
            if (const unsigned int index = m_giMap[giString]; index < m_gis.nStates)
               m_gis.stateDefs[index].GetState(m_gis.id, index, &giState);
      });
   return giState;
}

const vector<PinmameLampState>& Controller::GetChangedLamps() const
{
   m_changedLamps.clear();
   m_stateSources.With(
      [this](const std::vector<StateSrcId>& states)
      {
         uint8_t state = 0;
         for (unsigned int lampIndex = 0; lampIndex < m_lamps.nStates; lampIndex++)
         {
            const StateDef& def = m_lamps.stateDefs[lampIndex];
            if (def.dataFormat != CTLPI_STATE_FORMAT_UINT8 || def.GetState == nullptr)
               continue;
            if (def.GetState(m_lamps.id, lampIndex, &state); m_prevLampStates[lampIndex] == state)
               continue;
            m_prevLampStates[lampIndex] = state;
            m_changedLamps.emplace_back(def.mappingId, state);
         }
      });
   return m_changedLamps;
}

const vector<PinmameGIState>& Controller::GetChangedGIStrings() const
{
   m_changedGIs.clear();
   m_stateSources.With(
      [this](const std::vector<StateSrcId>& states)
      {
         uint8_t state = 0;
         for (unsigned int giIndex = 0; giIndex < m_gis.nStates; giIndex++)
         {
            const StateDef& def = m_gis.stateDefs[giIndex];
            if (def.dataFormat != CTLPI_STATE_FORMAT_UINT8 || def.GetState == nullptr)
               continue;
            if (def.GetState(m_gis.id, giIndex, &state); m_prevGIStates[giIndex] == state)
               continue;
            m_prevGIStates[giIndex] = state;
            m_changedGIs.emplace_back(def.mappingId, state);
         }
      });
   return m_changedGIs;
}

const vector<PinmameSolenoidState>& Controller::GetChangedSolenoids() const
{
   m_changedSolenoids.clear();
   m_stateSources.With(
      [this](const std::vector<StateSrcId>& states)
      {
         uint8_t state = 0;
         for (unsigned int solIndex = 0; solIndex < m_solenoids.nStates; solIndex++)
         {
            const StateDef& def = m_solenoids.stateDefs[solIndex];
            if (def.dataFormat != CTLPI_STATE_FORMAT_UINT8 || def.GetState == nullptr)
               continue;
            if (def.GetState(m_solenoids.id, solIndex, &state); m_prevSolenoidStates[solIndex] == state)
               continue;
            m_prevSolenoidStates[solIndex] = state;
            if (def.mappingId == 0 || def.mappingId > 64 || (m_solMask & (1ULL << (def.mappingId - 1))) != 0)
               m_changedSolenoids.emplace_back(def.mappingId, state);
         }
      });
   return m_changedSolenoids;
}


// Segment Displays

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


// DMD Displays

void Controller::OnDmdSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   Controller* me = static_cast<Controller*>(userData);
   assert(me->m_threadLock == std::this_thread::get_id());
   me->m_dmdUpdatePending = true;
}

void Controller::UpdateDmdSrc()
{
   assert(m_threadLock == std::this_thread::get_id());
   if (!m_dmdUpdatePending)
      return;

   m_dmdUpdatePending = false;
   m_defaultDmd = { };
   unsigned int largest = 128;
   GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
   m_msgApi->BroadcastMsg(m_endpointId, m_getDmdSrcMsgId, &getSrcMsg);
   vector<DisplaySrcId> displaySources(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, displaySources.data() };
   m_msgApi->BroadcastMsg(m_endpointId, m_getDmdSrcMsgId, &getSrcMsg);
   for (const DisplaySrcId& src : displaySources)
   {
      // A video screen is not a DMD, and selection here is only by width, so e.g. Pinball 2000's 640x480 CRT would
      // otherwise win outright and be handed to table scripts as the game's DMD. Those machines have none, so the
      // accessors below report a size of 0 rather than a frame in whatever format that screen happens to use
      const unsigned int family = src.hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK;
      if (family == CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY || family == CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY)
         continue;
      if (src.id.endpointId == m_endpointId && src.width >= largest)
      {
         m_defaultDmd = src;
         largest = src.width;
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
         // TODO implement original PinMAME / VPinMAME coloring
         const uint32_t lum = static_cast<int32_t>(static_cast<const float*>(frame.frame)[i] * 255.f);
         pixels[i] = (lum << 16) | (lum << 8) | lum;
      }
   }
   else if (m_defaultDmd.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
   {
      pixels.resize(size);
      for (int i = 0; i < size; i++)
         pixels[i] = ((uint32_t)static_cast<const uint8_t*>(frame.frame)[i * 3] << 16) | ((uint32_t)static_cast<const uint8_t*>(frame.frame)[i * 3 + 1] << 8)
            | (static_cast<const uint8_t*>(frame.frame)[i * 3 + 2]);
   }
   return pixels;
}

}
