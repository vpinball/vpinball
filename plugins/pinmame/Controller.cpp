// license:GPLv3+

#include "Controller.h"
#include "Game.h"
#include "Settings.h"
#include <thread>
#include <format>

#include "plugins/VPXPlugin.h" // Only used for optional feature (visual feedback on error)

namespace PinMAME {

__forceinline uint8_t saturatedByte(float v) { return (uint8_t)(255.0f * (v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v)); }

Controller::Controller(const MsgPluginAPI* api, unsigned int endpointId, const PinmameConfig& config)
   : m_msgApi(api)
   , m_endpointId(endpointId)
   , m_threadLock(std::this_thread::get_id())
{
   PinmameSetConfig(&config);
   PinmameSetHandleKeyboard(0);
   PinmameSetHandleMechanics(0xFF);

   m_vpmPath = config.vpmPath;

   m_getInputSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG);
   m_onInputChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onInputChangedMsgId, OnInputSrcChanged, this);

   m_getDeviceSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   m_onDeviceChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDeviceChangedMsgId, OnDeviceSrcChanged, this);
  
   m_getDmdSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   m_onDmdChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdChangedMsgId, OnDmdSrcChanged, this);
}

Controller::~Controller()
{
   assert(m_threadLock == std::this_thread::get_id());

   Stop();
   
   m_msgApi->UnsubscribeMsg(m_onInputChangedMsgId, OnInputSrcChanged, this);
   m_msgApi->ReleaseMsgID(m_onInputChangedMsgId);
   m_msgApi->ReleaseMsgID(m_getInputSrcMsgId);

   m_msgApi->UnsubscribeMsg(m_onDeviceChangedMsgId, OnDeviceSrcChanged, this);
   m_msgApi->ReleaseMsgID(m_onDeviceChangedMsgId);
   m_msgApi->ReleaseMsgID(m_getDeviceSrcMsgId);

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
   PinmameGetGame(name.c_str(), [](PinmameGame* pPinmameGame, void* const pUserData)
      {
         GameCBData* pGame = static_cast<GameCBData*>(pUserData);
         pGame->game = new Game(const_cast<Controller*>(pGame->controller), *pPinmameGame, pGame->settings);
      }, &cbData);
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
   PINMAME_STATUS status = PinmameGetGame(name.c_str(), [](PinmameGame* pPinmameGame, void* const pUserData) {
      Controller* me = static_cast<Controller*>(pUserData);
      me->m_szRomName = pPinmameGame->name;
      LOGI(std::format("Game found: name={}, description={}, manufacturer={}, year={}", pPinmameGame->name, pPinmameGame->description, pPinmameGame->manufacturer, pPinmameGame->year));
   }, this);
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

   // Trigger startup, status will be either 2 (staring), 1 (running), 0 (stopped, likely after failure)
   PINMAME_STATUS status = PinmameRun(m_szGameName.c_str());
   while (PinmameIsRunning() == 2) // Wait until the machine is either running or stopped
      std::this_thread::sleep_for(std::chrono::milliseconds(75)); 

   if ((PinmameIsRunning() == 1) && status == PINMAME_STATUS_OK) {
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
   if (status == PINMAME_STATUS_GAME_ALREADY_RUNNING) {
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

void Controller::OnInputSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   Controller* me = static_cast<Controller*>(userData);
   assert(me->m_threadLock == std::this_thread::get_id());
   me->m_inputUpdatePending = true;
}

void Controller::UpdateInputSrc() const
{
   assert(m_threadLock == std::this_thread::get_id());
   if (!m_inputUpdatePending)
      return;

   m_inputUpdatePending = false;
   m_inputs = { };
   m_switches.clear();
   m_switchMap.clear();
   m_dipSwitches.clear();
   m_dipSwitchMap.clear();

   GetInputSrcMsg getInputMsg = { 0, 0, nullptr };
   m_msgApi->BroadcastMsg(m_endpointId, m_getInputSrcMsgId, &getInputMsg);
   vector<InputSrcId> inputSources(getInputMsg.count);
   getInputMsg = { getInputMsg.count, 0, inputSources.data() };
   m_msgApi->BroadcastMsg(m_endpointId, m_getInputSrcMsgId, &getInputMsg);
   for (const InputSrcId& src : inputSources)
   {
      if (src.id.endpointId == m_endpointId)
      {
         m_inputs = src;
         break;
      }
   }

   for (unsigned int i = 0; i < m_inputs.nInputs; i++)
   {
      switch (m_inputs.inputDefs[i].id.groupId)
      {
      case 0x0001:
      {
         m_switches.push_back(i);
         const int switchOfs = static_cast<int16_t>(m_inputs.inputDefs[i].id.deviceId) + SWITCH_OFFSET;
         assert(switchOfs >= 0);
         if (m_switchMap.size() < switchOfs + 1)
            m_switchMap.resize(switchOfs + 1, UINT_MAX);
         m_switchMap[switchOfs] = i;
         if (switchOfs < m_switchStates.size())
            m_inputs.SetInputState(i, m_switchStates[switchOfs]);
         break;
      }
      case 0x0002:
         m_dipSwitches.push_back(i);
         if (m_dipSwitchMap.size() < m_inputs.inputDefs[i].id.deviceId + 1)
            m_dipSwitchMap.resize(m_inputs.inputDefs[i].id.deviceId + 1, UINT_MAX);
         m_dipSwitchMap[m_inputs.inputDefs[i].id.deviceId] = i;
         if (m_inputs.inputDefs[i].id.deviceId < m_dipSwitchStates.size())
            m_inputs.SetInputState(i, m_dipSwitchStates[m_inputs.inputDefs[i].id.deviceId]);
         break;
      }
   }
}

bool Controller::GetSwitch(int switchNo) const
{
   const int switchNoOfs = switchNo + SWITCH_OFFSET;
   if (switchNoOfs < 0)
      return false;

   UpdateInputSrc();

   if (switchNoOfs < m_switchMap.size())
      if (const unsigned int index = m_switchMap[switchNoOfs]; index < m_inputs.nInputs)
         return m_inputs.GetInputState(index) != 0;

   return switchNoOfs < m_switchStates.size() ? m_switchStates[switchNoOfs] : false;
}

void Controller::SetSwitch(int switchNo, bool state)
{
   const int switchNoOfs = switchNo + SWITCH_OFFSET;
   if (switchNoOfs < 0)
      return;

   UpdateInputSrc();

   if (m_switchStates.size() < switchNoOfs + 1)
      m_switchStates.resize(switchNoOfs + 1, false);
   m_switchStates[switchNoOfs] = state;

   if (switchNoOfs < m_switchMap.size())
      if (const unsigned int index = m_switchMap[switchNoOfs]; index < m_inputs.nInputs)
         m_inputs.SetInputState(index, state ? 1 : 0);
}

int Controller::GetDip(int dipSwitchNo) const
{
   if (dipSwitchNo < 0)
      return false;

   UpdateInputSrc();

   if (dipSwitchNo < m_dipSwitchMap.size())
      if (const unsigned int index = m_dipSwitchMap[dipSwitchNo]; index < m_inputs.nInputs)
         return m_inputs.GetInputState(index) != 0;

   return dipSwitchNo < m_dipSwitchStates.size() ? m_dipSwitchStates[dipSwitchNo] : false;
}

void Controller::SetDip(int dipSwitchNo, int state)
{
   if (dipSwitchNo < 0)
      return;

   UpdateInputSrc();

   if (m_dipSwitchStates.size() < dipSwitchNo + 1)
      m_dipSwitchStates.resize(dipSwitchNo + 1, false);
   m_dipSwitchStates[dipSwitchNo] = state;

   if (dipSwitchNo < m_dipSwitchMap.size())
      if (const unsigned int index = m_dipSwitchMap[dipSwitchNo]; index < m_inputs.nInputs)
         m_inputs.SetInputState(index, state ? 1 : 0);
}


// Devices

void Controller::OnDeviceSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   Controller* me = static_cast<Controller*>(userData);
   assert(me->m_threadLock == std::this_thread::get_id());
   me->m_deviceUpdatePending = true;
}

void Controller::UpdateDeviceSrc() const
{
   assert(m_threadLock == std::this_thread::get_id());
   if (!m_deviceUpdatePending)
      return;

   m_deviceUpdatePending = false;
   m_devices = { };
   m_solenoids.clear();
   m_solenoidMap.clear();
   m_gis.clear();
   m_giMap.clear();
   m_lamps.clear();
   m_lampMap.clear();

   GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
   m_msgApi->BroadcastMsg(m_endpointId, m_getDeviceSrcMsgId, &getSrcMsg);
   vector<DevSrcId> deviceSources(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, deviceSources.data() };
   m_msgApi->BroadcastMsg(m_endpointId, m_getDeviceSrcMsgId, &getSrcMsg);
   for (const DevSrcId& src : deviceSources)
   {
      if (src.id.endpointId == m_endpointId)
      {
         m_devices = src;
         break;
      }
   }

   for (unsigned int i = 0; i < m_devices.nDevices; i++)
   {
      switch (m_devices.deviceDefs[i].id.groupId & 0xFF00)
      {
      case 0x0000:
         m_solenoids.push_back(i);
         if (m_solenoidMap.size() < m_devices.deviceDefs[i].id.deviceId + 1)
            m_solenoidMap.resize(m_devices.deviceDefs[i].id.deviceId + 1, UINT_MAX);
         m_solenoidMap[m_devices.deviceDefs[i].id.deviceId] = i;
         break;
      case 0x0100:
         m_gis.push_back(i);
         if (m_giMap.size() < m_devices.deviceDefs[i].id.deviceId + 1)
            m_giMap.resize(m_devices.deviceDefs[i].id.deviceId + 1, UINT_MAX);
         m_giMap[m_devices.deviceDefs[i].id.deviceId] = i;
         break;
      case 0x0200:
         m_lamps.push_back(i);
         if (m_lampMap.size() < m_devices.deviceDefs[i].id.deviceId + 1)
            m_lampMap.resize(m_devices.deviceDefs[i].id.deviceId + 1, UINT_MAX);
         m_lampMap[m_devices.deviceDefs[i].id.deviceId] = i;
         break;
      case 0x0300: break; // TODO Mech
      }
   }
}

uint8_t Controller::GetGIValue(float value) const
{
   // Recreate legacy VPinMAME values which depends on hardware generation and bulb integration mode
   if (m_deviceMode == DM_PHYSOUT)
      return saturatedByte(value);
   else if (PinmameGetHardwareGen() & 0x00000000000ff) // All WPC: GI level is 0..8
      return static_cast<uint8_t>(value * 8.f);
   else
      return value != 0.f ? 9 : 0; // Whitestar and SAM GI levels are either 0 or 9
}

uint8_t Controller::GetSolenoidValue(int solIndex, uint8_t value) const
{
   // Recreate legacy VPinMAME values which depends on hardware generation, output index and integration mode
   switch (m_deviceMode)
   {
   case DM_BINARY: return value != 0 ? 1 : 0; // Original integration to binary state
   case DM_MODSOL: // Simple linear integration added to the first 32 solenoids of WPC & SAM hardwares
      if (solIndex <= 32 && (PinmameGetHardwareGen() & 0x01000000000ff))
         return value;
      else
         return value != 0 ? 1 : 0;
   case DM_PHYSOUT: return value; // Bulb integration to linear luminance 0..255
   default: assert(false); return 0;
   }
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
   UpdateDeviceSrc();
   
   if (solenoid < 0 || solenoid >= m_solenoidMap.size())
      return 0;
   
   if (const unsigned int index = m_solenoidMap[solenoid]; index < m_devices.nDevices)
      return GetSolenoidValue(solenoid, m_devices.GetByteState(index));

   return 0;
}

int Controller::GetLamp(int lamp) const
{
   UpdateDeviceSrc();

   if (lamp < 0 || lamp >= m_lampMap.size())
      return 0;

   if (const unsigned int index = m_lampMap[lamp]; index < m_devices.nDevices)
      return GetLampValue(m_devices.GetByteState(index));

   return 0;
}

int Controller::GetGIString(int giString) const
{
   UpdateDeviceSrc();

   if (giString < 0 || giString >= m_giMap.size())
      return 0;
   
   if (const unsigned int index = m_giMap[giString]; index < m_devices.nDevices)
      return GetGIValue(m_devices.GetFloatState(index));

   return 0;
}

const vector<PinmameLampState>& Controller::GetChangedLamps()
{
   UpdateDeviceSrc();

   if (m_devices.nDevices > m_prevDeviceState.size())
      m_prevDeviceState.resize(m_devices.nDevices, 0);

   m_lampStates.clear();
   for (int lampIndex : m_lamps)
   {
      const uint8_t state = GetLampValue(m_devices.GetByteState(lampIndex));
      if (m_prevDeviceState[lampIndex] != state)
      {
         m_lampStates.emplace_back(m_devices.deviceDefs[lampIndex].id.deviceId, state);
         m_prevDeviceState[lampIndex] = state;
      }
   }
   return m_lampStates;
}

const vector<PinmameGIState>& Controller::GetChangedGIStrings()
{
   UpdateDeviceSrc();

   if (m_devices.nDevices > m_prevDeviceState.size())
      m_prevDeviceState.resize(m_devices.nDevices, 0);

   m_giStates.clear();
   for (int giIndex : m_gis)
   {
      const uint8_t state = GetGIValue(m_devices.GetFloatState(giIndex));
      if (m_prevDeviceState[giIndex] != state)
      {
         m_giStates.emplace_back(m_devices.deviceDefs[giIndex].id.deviceId, state);
         m_prevDeviceState[giIndex] = state;
      }
   }
   return m_giStates;
}

const vector<PinmameSolenoidState>& Controller::GetChangedSolenoids()
{
   UpdateDeviceSrc();

   if (m_devices.nDevices > m_prevDeviceState.size())
      m_prevDeviceState.resize(m_devices.nDevices, 0);

   m_solenoidStates.clear();
   for (int solIndex : m_solenoids)
   {
      const uint8_t state = GetSolenoidValue(m_devices.deviceDefs[solIndex].id.deviceId, m_devices.GetByteState(solIndex));
      if (m_prevDeviceState[solIndex] != state)
      {
         if (m_devices.deviceDefs[solIndex].id.deviceId >= 64 || (m_solMask & (1ULL << m_devices.deviceDefs[solIndex].id.deviceId)) != 0)
            m_solenoidStates.emplace_back(m_devices.deviceDefs[solIndex].id.deviceId, state);
         m_prevDeviceState[solIndex] = state;
      }
   }
   return m_solenoidStates;
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
         pixels[i] = ((uint32_t)static_cast<const uint8_t*>(frame.frame)[i * 3] << 16) 
            | ((uint32_t)static_cast<const uint8_t*>(frame.frame)[i * 3 + 1] << 8)
            | (static_cast<const uint8_t*>(frame.frame)[i * 3 + 2]);
   }
   return pixels;
}

}
