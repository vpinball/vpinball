// license:GPLv3+

#include "core/stdafx.h"
#include "core/VPXPluginAPIImpl.h"
#include "renderer/VRDevice.h"

#include "ScanCodes.h"

#ifdef __LIBVPINBALL__
   #include "lib/src/VPinballLib.h"
#endif

#include "input/SDLInputHandler.h"

#ifndef __LIBVPINBALL__
   #include "input/OpenPinDevHandler.h"
#endif


InputManager::InputManager()
   : m_onActionEventMsgId(VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_ACTION_CHANGED))
   , m_keyboardDeviceId(RegisterDevice("Key"s, InputManager::DeviceType::Keyboard, "Keyboards"s)) // Base device: merge inputs from all connected keyboards
   , m_mouseDeviceId(RegisterDevice("Mouse"s, InputManager::DeviceType::Mouse, "Mouse"s)) // Base device: merge inputs from all connected mice
{
   const Settings& settings = g_pvp->m_settings;

   m_inputDevices[m_keyboardDeviceId].m_connected = true;
   m_inputDevices[m_mouseDeviceId].m_connected = true;

   CreateInputActions();

   // Touch screen support
   int nTouchDevices;
   SDL_TouchID* touchDevices = SDL_GetTouchDevices(&nTouchDevices);
   for (int i = 0; i < nTouchDevices; i++)
   {
      PLOGI << "Touch device detected: '" << SDL_GetTouchDeviceName(touchDevices[i]) << "' "
            << ((SDL_GetTouchDeviceType(touchDevices[i]) == SDL_TOUCH_DEVICE_DIRECT) ? " - Enabling touch support" : " - Skipping (not a touch screen)");
      if (SDL_GetTouchDeviceType(touchDevices[i]) == SDL_TOUCH_DEVICE_DIRECT)
         m_supportsTouch = true;
   }
   if (touchDevices)
      SDL_free(touchDevices);
   auto addTouchRegion = [this](const RECT& region, unsigned int actionId) { m_touchRegionMap.emplace_back(region, actionId, m_inputActions[actionId]->NewDirectStateSlot()); };
   // RECT definition is left, top, right, bottom in % of screen
   addTouchRegion(RECT { 0, 0, 50, 10 }, GetAddCreditActionId(0));
   addTouchRegion(RECT { 50, 0, 100, 10 }, GetExitInteractiveActionId());
   addTouchRegion(RECT { 0, 10, 50, 30 }, GetLeftMagnaActionId());
   addTouchRegion(RECT { 50, 10, 100, 30 }, GetRightMagnaActionId());
   addTouchRegion(RECT { 0, 30, 50, 60 }, GetLeftNudgeActionId());
   addTouchRegion(RECT { 50, 30, 100, 60 }, GetRightNudgeActionId());
   addTouchRegion(RECT { 0, 60, 30, 90 }, GetLeftFlipperActionId());
   addTouchRegion(RECT { 0, 60, 30, 90 }, GetStagedLeftFlipperActionId());
   addTouchRegion(RECT { 30, 60, 70, 100 }, GetCenterNudgeActionId());
   addTouchRegion(RECT { 70, 60, 100, 90 }, GetRightFlipperActionId());
   addTouchRegion(RECT { 70, 60, 100, 90 }, GetStagedRightFlipperActionId());
   addTouchRegion(RECT { 0, 90, 30, 100 }, GetStartActionId());
   addTouchRegion(RECT { 70, 90, 100, 100 }, GetLaunchBallActionId());

   // Analog sensors for plunger and nudge
   for (int i = 0; i < 2; i++)
   {
      m_nudgeXSensor[i] = std::make_unique<PhysicsSensor>(this, "NudgeX" + std::to_string(i + 1), "Sensor " + std::to_string(i + 1) + " - Nudge Side", SensorMapping::Type::Acceleration);
      m_nudgeYSensor[i] = std::make_unique<PhysicsSensor>(this, "NudgeY" + std::to_string(i + 1), "Sensor " + std::to_string(i + 1) + " - Nudge Front", SensorMapping::Type::Acceleration);
      m_nudgeFilter[i] = !settings.GetPlayer_NudgeFilter(i);
      SetNudgeFiltered(i, !m_nudgeFilter[i]);
   }
   m_plungerPositionSensor = std::make_unique<PhysicsSensor>(this, "PlungerPos"s, "Plunger Position"s, SensorMapping::Type::Position);
   m_plungerVelocitySensor = std::make_unique<PhysicsSensor>(this, "PlungerVel"s, "Plunger Velocity"s, SensorMapping::Type::Velocity);
   m_plungerPositionSensor->SetFilter(std::make_unique<PlungerPositionFilter>());

   m_exitPressTimestamp = 0;
   m_exitAppPressLengthMs = settings.GetPlayer_Exitconfirm() * 1000 / 60;

   m_rumbleMode = g_pvp->m_settings.GetPlayer_RumbleMode();

   // Load settings
   {
      LoadDevicesFromSettings();

      for (const auto& action : m_inputActions)
         action->LoadMapping(settings);

      m_plungerPositionSensor->LoadMapping(settings);
      m_plungerVelocitySensor->LoadMapping(settings);
      for (int i = 0; i < 2; i++)
      {
         m_nudgeXSensor[i]->LoadMapping(settings);
         m_nudgeYSensor[i]->LoadMapping(settings);
         m_nudgeOrientation[i] = ANGTORAD(settings.GetPlayer_NudgeOrientation(i));
      }

      m_linearPlunger = settings.GetPlayer_PlungerLinearSensor();
      m_plunger_retract = settings.GetPlayer_PlungerRetract();
   }

   // Initialize device handlers
   m_inputHandlers.push_back(std::make_unique<SDLInputHandler>(*this));
   m_sdlHandler = static_cast<SDLInputHandler*>(m_inputHandlers.back().get());
   #ifndef __LIBVPINBALL__
      m_inputHandlers.push_back(std::make_unique<OpenPinDevHandler>(*this));
   #endif

   // Apply initial keyboard state (if any key is already in a pressed state when the object is created)
   SDL_ResetKeyboard();
   int nSDLKeys;
   const bool* sdlKeyStates = SDL_GetKeyboardState(&nSDLKeys);
   for (const auto& [deviceAndButtonId, mappings] : m_buttonMappings)
   {
      if (((deviceAndButtonId & 0xFFFF0000u) >> 16) == m_keyboardDeviceId)
      {
         int sdlScancode = deviceAndButtonId & 0x0000FFFFu;
         if (sdlScancode < nSDLKeys && sdlKeyStates[sdlScancode])
            for (auto& mapping : mappings)
               mapping->SetPressed(true);
      }
   }

   #ifdef _WIN32
      // Save the current sticky/toggle/filter key settings so they can be restored them later
      SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);
      SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_StartupToggleKeys, 0);
      SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &m_StartupFilterKeys, 0);
      AllowAccessibilityShortcutKeys(false);
      // Disable Windows UI keyboard shortcut through a global hook
      // Do not apply when debugging as this causes massive mouse lags
      #ifndef _DEBUG
      m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
      #endif
   #endif
}

InputManager::~InputManager()
{
   try
   {
      SaveDevicesToSettings();
   }
   catch (...)
   {
      PLOGE << "Failed to persist input devices in settings";
   }
   m_inputHandlers.clear();
   m_sdlHandler = nullptr;
   VPXPluginAPIImpl::ReleaseMsgID(m_onActionEventMsgId);

   #ifdef _WIN32
      if (m_hKeyboardHook)
         UnhookWindowsHookEx(m_hKeyboardHook);
      AllowAccessibilityShortcutKeys(true);
   #endif
}




#pragma region Device and Element Management
//////////////////////////////////////////////////////////////////
// Device and Element Management

uint16_t InputManager::RegisterDevice(const string& settingsId, InputManager::DeviceType type, const string& name)
{
   PLOGD << "Input device '" << name << "' using settings id '" << settingsId << "' is connected";
   uint16_t deviceId = GetDeviceId(settingsId);
   assert(!m_inputDevices[deviceId].m_connected);
   m_inputDevices[deviceId].m_name = name;
   m_inputDevices[deviceId].m_type = type;
   m_inputDevices[deviceId].m_connected = true;
   Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Device." + settingsId + ".Name", "Device Name"s, ""s, true, name));
   Settings::GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(
      "Input"s, "Device." + settingsId + ".Type", "Device Type"s, ""s, true, 0, (int)type, vector { "Unknown"s, "Keyboard"s, "Joystick"s, "Mouse"s, "VRController"s, "OpenPinDev"s }));
   Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>("Input"s, "Device." + settingsId + ".NoAutoLayout", "Disable Automatic Layout"s, ""s, false, false));
   return deviceId;
}

void InputManager::UnregisterDevice(uint16_t deviceId)
{
   assert(deviceId < m_inputDevices.size());
   assert(m_inputDevices[deviceId].m_connected);
   m_inputDevices[deviceId].m_connected = false;
   PLOGD << "Input device '" << m_inputDevices[deviceId].m_name << "' using settings id '" << m_inputDevices[deviceId].m_settingsId << "' is disconnected";
}

void InputManager::RegisterElementName(uint16_t deviceId, bool isAxis, uint16_t buttonOrAxisId, const string& name)
{
   assert(deviceId < m_inputDevices.size());
   m_inputDevices[deviceId].m_buttonOrAxisNames[buttonOrAxisId] = { isAxis, name };
}

void InputManager::RegisterDefaultMapping(uint16_t deviceId,
   const std::function<bool( // Function to either apply or evaluate applying the default mapping of the given controller
      std::function<bool(const vector<ButtonMapping>&, unsigned int)>, // Map Button
      std::function<bool(const SensorMapping&, SensorMapping::Type, bool)>, // Map plunger
      std::function<bool(const SensorMapping&, const SensorMapping&)> // Map nudge
      )>& mapper)
{
   assert(deviceId < m_inputDevices.size());
   // Register device for auto layout, performed after all buttons/axis have been registered and when user interaction is possible, which may happen after device is registered
   m_inputDevices[deviceId].m_defaultMapping = mapper;
   auto isButtonMapped = [this](const vector<ButtonMapping>& mapping, unsigned int actionId)
   {
      return m_inputActions[actionId]->HasMapping(mapping);
   };
   auto isPlungerMapped = [this](const SensorMapping& sensorPos, SensorMapping::Type type, bool isLinear)
   {
      const std::unique_ptr<PhysicsSensor>& plungerPos = type == SensorMapping::Type::Position ? GetPlungerPositionSensor() : GetPlungerVelocitySensor();
      return plungerPos->IsMapped() && plungerPos->GetMapping().IsSame(sensorPos);
   };
   auto isNudgeMapped = [this](const SensorMapping& sensorX, const SensorMapping& sensorY)
   {
      for (int i = 0; i < 2; i++)
      {
         const std::unique_ptr<PhysicsSensor>& nudgeX = GetNudgeXSensor(i);
         const std::unique_ptr<PhysicsSensor>& nudgeY = GetNudgeYSensor(i);
         if (nudgeX->IsMapped() && nudgeX->GetMapping().IsSame(sensorX) && nudgeY->IsMapped() && nudgeY->GetMapping().IsSame(sensorY))
            return true;
      }
      return false;
   };
   m_inputDevices[deviceId].m_hasPendingLayoutApply |= !mapper(isButtonMapped, isPlungerMapped, isNudgeMapped);
   m_hasPendingLayoutApply |= m_inputDevices[deviceId].m_hasPendingLayoutApply;
}

void InputManager::ApplyDefaultDeviceMapping(uint16_t deviceId)
{
   assert(deviceId < m_inputDevices.size());
   auto mapButton = [this](const vector<ButtonMapping>& mapping, unsigned int actionId)
   {
      m_inputActions[actionId]->AddMapping(mapping);
      return true;
   };
   auto mapPlunger = [this](const SensorMapping& sensorPos, SensorMapping::Type type, bool isLinear)
   {
      const std::unique_ptr<PhysicsSensor>& plungerPos = type == SensorMapping::Type::Position ? GetPlungerPositionSensor() : GetPlungerVelocitySensor();
      plungerPos->SetMapping(sensorPos);
      return true;
   };
   auto mapNudge = [this](const SensorMapping& sensorX, const SensorMapping& sensorY)
   {
      int map = -1;
      for (int i = 0; i < 2; i++)
      {
         const std::unique_ptr<PhysicsSensor>& nudgeX = GetNudgeXSensor(i);
         const std::unique_ptr<PhysicsSensor>& nudgeY = GetNudgeYSensor(i);
         if (nudgeX->IsMapped() && nudgeY->IsMapped())
         {
            if (nudgeX->GetMapping().IsSame(sensorX) && nudgeY->GetMapping().IsSame(sensorY))
               return true;
         }
         else if (map == -1)
            map = i;
      }
      if (map == -1)
         return false; // No free nudge sensor available
      const std::unique_ptr<PhysicsSensor>& nudgeX = GetNudgeXSensor(map);
      const std::unique_ptr<PhysicsSensor>& nudgeY = GetNudgeYSensor(map);
      nudgeX->SetMapping(sensorX);
      nudgeY->SetMapping(sensorY);
      return true;
   };
   m_inputDevices[deviceId].m_defaultMapping(mapButton, mapPlunger, mapNudge);

   // Save mapping after applying them
   Settings& settings = g_pvp->m_settings;
   for (const auto& action : m_inputActions)
      action->SaveMapping(settings);

   m_plungerPositionSensor->SaveMapping(settings);
   m_plungerVelocitySensor->SaveMapping(settings);
   for (int i = 0; i < 2; i++)
   {
      m_nudgeXSensor[i]->SaveMapping(settings);
      m_nudgeYSensor[i]->SaveMapping(settings);
      settings.SetPlayer_NudgeOrientation(i, RADTOANG(m_nudgeOrientation[i]), false);
   }
}

void InputManager::LoadDevicesFromSettings()
{
   const Settings& settings = g_pvp->m_settings;
   std::istringstream deviceStream(settings.GetInput_Devices());
   std::string deviceSettingId;
   while (std::getline(deviceStream, deviceSettingId, ';'))
   {
      // Always prefer live device informations above persisted ones
      if (const auto it = std::ranges::find_if(m_inputDevices, [&deviceSettingId](auto& deviceDef) { return deviceDef.m_settingsId == deviceSettingId; }); it != m_inputDevices.end())
         continue;

      m_inputDevices.emplace_back(static_cast<uint16_t>(m_inputDevices.size()), "Unknown Device"s, deviceSettingId);
      m_inputDevices.back().m_connected = false;
      DeviceDef& device = m_inputDevices.back();
      const auto namePropId
         = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Device." + deviceSettingId + ".Name", "Device Name"s, ""s, true, ""s));
      const auto typePropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(
         "Input"s, "Device." + deviceSettingId + ".Type", "Device Type"s, ""s, true, 0, 0, vector { "Unknown"s, "Keyboard"s, "Joystick"s, "Mouse"s, "VRController"s, "OpenPinDev"s }));
      Settings::GetRegistry().Register(
         std::make_unique<VPX::Properties::BoolPropertyDef>("Input"s, "Device." + deviceSettingId + ".NoAutoLayout", "Disable Automatic Layout"s, ""s, false, false));
      device.m_name = settings.GetString(namePropId);
      device.m_type = static_cast<InputManager::DeviceType>(settings.GetInt(typePropId));

      int index = 0;
      while (true)
      {
         const auto elementPropId = Settings::GetRegistry().Register(
            std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Device." + deviceSettingId + ".Element" + std::to_string(index), ""s, ""s, false, ""s));
         string inputName = settings.GetString(elementPropId);
         if (inputName.empty())
            break;
         if (size_t pos = inputName.find(';'); pos != std::string::npos)
            if (int button; try_parse_int(inputName.substr(0, pos), button) && inputName.length() > pos + 3)
            {
               if (inputName[pos + 1] == 'A')
                  device.m_buttonOrAxisNames[button] = { true, trim_string(inputName.substr(pos + 3)) };
               else if (inputName[pos + 1] == 'B')
                  device.m_buttonOrAxisNames[button] = { false, trim_string(inputName.substr(pos + 3)) };
            }
         index++;
      }
   }
}

void InputManager::SaveDevicesToSettings() const
{
   Settings& settings = g_pvp->m_settings;
   std::stringstream deviceList;
   for (size_t i = 0; i < m_inputDevices.size(); ++i)
   {
      if (i > 0)
         deviceList << ';';
      deviceList << m_inputDevices[i].m_settingsId;
   }
   settings.SetInput_Devices(deviceList.str(), false);

   for (const auto& device : m_inputDevices)
   {
      const auto namePropId
         = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Device." + device.m_settingsId + ".Name", "Device Name"s, ""s, true, ""s));
      const auto typePropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(
         "Input"s, "Device." + device.m_settingsId + ".Type", "Device Type"s, ""s, true, 0, 0,
         vector { "Unknown"s, "Keyboard"s, "Joystick"s, "Mouse"s, "VRController"s, "OpenPinDev"s }));
      settings.Set(namePropId, device.m_name, false);
      settings.Set(typePropId, static_cast<int>(device.m_type), false);
      int index = 0;
      for (auto& [button, def] : device.m_buttonOrAxisNames)
      {
         const auto elementPropId = Settings::GetRegistry().Register(
            std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Device." + device.m_settingsId + ".Element" + std::to_string(index), ""s, ""s, false, ""s));
         settings.Set(elementPropId, std::to_string(button) + ';' + (def.isAxis ? 'A' : 'B') + ';' + def.name, false);
         index++;
      }
   }
}

uint16_t InputManager::GetDeviceId(const string& settingsId)
{
   if (const auto it = std::ranges::find_if(m_inputDevices, [&settingsId](auto& deviceDef) { return deviceDef.m_settingsId == settingsId; }); it != m_inputDevices.end())
      return it->m_id;
   m_inputDevices.emplace_back(static_cast<uint16_t>(m_inputDevices.size()), "Unknown Device"s, settingsId);
   m_inputDevices.back().m_connected = false;
   return m_inputDevices.back().m_id;
}

const string& InputManager::GetDeviceSettingId(uint16_t deviceId) const
{
   assert(deviceId < m_inputDevices.size());
   return m_inputDevices[deviceId].m_settingsId;
}

const string& InputManager::GetDeviceName(uint16_t deviceId) const
{
   assert(deviceId < m_inputDevices.size());
   return m_inputDevices[deviceId].m_name;
}

string InputManager::GetDeviceElementName(uint16_t deviceId, uint16_t buttonOrAxisId) const
{
   assert(deviceId < m_inputDevices.size());
   const DeviceDef& device = m_inputDevices[deviceId];
   if (auto it = device.m_buttonOrAxisNames.find(buttonOrAxisId); it != device.m_buttonOrAxisNames.end())
      return it->second.name;
   return "Button #" + std::to_string(buttonOrAxisId);
}

InputManager::DeviceType InputManager::GetDeviceType(uint16_t deviceId) const
{
   assert(deviceId < m_inputDevices.size());
   return m_inputDevices[deviceId].m_type;
}

vector<uint32_t> InputManager::GetAllAxis() const
{
   vector<uint32_t> axis;
   for (const auto& device : m_inputDevices)
      if (device.m_connected)
         for (const auto& [axisOrButtonId, elementDef] : device.m_buttonOrAxisNames)
            if (elementDef.isAxis)
               axis.push_back(device.m_id << 16 | axisOrButtonId);
   return axis;
}

#pragma endregion


#pragma region Events
////////////////////////////////////////////////////////////////////////////
// Events pushed by the input handlers

void InputManager::ProcessInput()
{
   if (!g_pplayer || !g_pplayer->m_ptable)
      return; // only if player is running
   g_pplayer->m_logicProfiler.OnProcessInput();

   // Gather input from all handlers
   for (const auto& handler : m_inputHandlers)
      handler->Update();

   // Handle automatic start
   if (g_pplayer->m_ptable->m_tblAutoStartEnabled)
      Autostart(g_pplayer->m_ptable->m_tblAutoStart, g_pplayer->m_ptable->m_tblAutoStartRetry);
   if (m_autoStartTimestamp == 0) // Check if we've been initialized.
      m_autoStartTimestamp = msec();

   // Update continuous actions
   for (const auto action : m_onUpdateActions)
      action->OnUpdate();

   // Process axis capture: if we have rested long enough, register the button release and stop capture
   if (m_buttonCaptureState == 1)
   {
      const uint32_t ms = msec();
      for (ButtonMapping& bm : m_buttonCapture)
      {
         if (bm.m_buttonCapturePos != 0 && ms > bm.m_buttonCaptureStartMs + 200)
         {
            switch (bm.m_buttonCapturePos)
            {
            case -1: // Validate Pressed at -1
               bm.SetAxisReversed(true);
               bm.m_buttonCapturePos = 0;
               break;

            case -3: // Validate Pressed at 1
               bm.SetAxisReversed(false);
               bm.m_buttonCapturePos = 0;
               break;

            case 1: // Validate Released at -1 => Rest = -1, Pressed = 1
               assert(!bm.IsAxisReversed());
               bm.SetAxisThreshold(0.f);
               m_buttonCaptureState = 2;
               break;

            case 2: // Validate Released at 0 => Rest = 0, Pressed = 1 or -1
               bm.SetAxisThreshold(bm.IsAxisReversed() ? -0.5f : 0.5f);
               m_buttonCaptureState = 2;
               break;

            case 3: // Validate Released at 1 => Rest = 1, Pressed = -1
               assert(bm.IsAxisReversed());
               bm.SetAxisThreshold(0.f);
               m_buttonCaptureState = 2;
               break;
            }
         }
      }
   }

   // Perform pending device auto detection (deferred until in game UI is available)
   if (m_hasPendingLayoutApply && g_pplayer->m_liveUI)
   {
      m_hasPendingLayoutApply = false;
      for (auto& device : m_inputDevices)
      {
         if (device.m_hasPendingLayoutApply)
         {
            const auto noAutoLayoutId = Settings::GetRegistry().GetPropertyId("Input"s, "Device." + device.m_settingsId + ".NoAutoLayout").value();
            if (g_pvp->m_settings.GetBool(noAutoLayoutId))
            {
               device.m_hasPendingLayoutApply = false;
               continue;
            }
            const uint16_t deviceId = device.m_id;
            if (g_pplayer->m_liveUI->ProposeInputLayout(device.m_name,
                   [this, deviceId, noAutoLayoutId](bool isOk, bool isDontAskAnymore)
                   {
                      if (isOk)
                         ApplyDefaultDeviceMapping(deviceId);
                      if (isDontAskAnymore)
                         g_pvp->m_settings.Set(noAutoLayoutId, true, false);
                   }))
            {
               device.m_hasPendingLayoutApply = false;
               m_hasPendingLayoutApply = true; // As only one UI interaction may be done at a given time
               break;
            }
         }
      }
   }
}

void InputManager::HandleSDLEvent(const SDL_Event& e) { m_sdlHandler->HandleSDLEvent(e); }

void InputManager::PushButtonEvent(uint16_t deviceId, uint16_t buttonId, uint64_t timestampNs, bool isPressed)
{
   // Discard keyboard events when the UI is capturing the keyboard (e.g. for control input)
   if (deviceId == m_keyboardDeviceId && ImGui::GetIO().WantCaptureKeyboard)
      return;

   uint32_t id = deviceId << 16 | buttonId;
   if (auto it = m_buttonMappings.find(id); it != m_buttonMappings.end())
      for (ButtonMapping* mapping : it->second)
         mapping->SetPressed(isPressed);

   // Special handling for keyboard events to trigger custom KeyDown/KeyUp events in the script based on Windows DirectInput key codes
   if (deviceId == m_keyboardDeviceId && !g_pplayer->m_liveUI->IsInGameUIOpened())
   {
      const unsigned char dik = GetDirectInputKeyFromSDLScancode(static_cast<SDL_Scancode>(buttonId));
      if (dik != 0)
      {
         CComVariant rgvar[1] = { CComVariant(dik) };
         DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
         g_pplayer->m_ptable->FireDispID(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, &dispparams);
      }
   }

   if (m_buttonCaptureState == 1)
   {
      const bool wasCaptured = std::ranges::find_if(m_buttonCapture,
                                  [deviceId, buttonId](const ButtonMapping& bm) { return bm.GetDeviceId() == deviceId && bm.GetAxisOrButtonId() == buttonId; })
         != m_buttonCapture.end();
      if (!wasCaptured && isPressed)
      {
         m_buttonCapture.emplace_back(nullptr, nullptr, deviceId, buttonId);
      }
      else if (wasCaptured)
      {
         m_buttonCaptureState = 2;
      }
   }
}

void InputManager::PushAxisEvent(uint16_t deviceId, uint16_t axisId, uint64_t timestampNs, float position)
{
   assert(-1.f <= position && position <= 1.f);

   uint32_t id = deviceId << 16 | axisId;
   if (auto it = m_sensorMappings.find(id); it != m_sensorMappings.end())
      for (auto mapping : it->second)
         mapping->SetAxisPosition(timestampNs, position);

   if (auto it = m_buttonMappings.find(id); it != m_buttonMappings.end())
      for (ButtonMapping* mapping : it->second)
         mapping->SetAxisPosition(position);

   for (const auto& listener : m_axisListeners)
      listener();

   if (m_buttonCaptureState == 1)
   {
      // Capture is somewhat complex since we can't know beforehand if the range will be 1..-1, or -1..1, or 0..1, or 0..-1
      // Therefore:
      // . we identify the 3 possible positions (-1 / 0 / 1)
      // . when a pressed position (-1 / 1) is reached we register the axis in the capture with its pressed position and validate it against a given duration to avoid capturing the move away from the initial released position
      // . when a release one (-1 / 0 / 1) is reached for a lengthy enough rest time, we capture it as the released position (and end up the capture)
      auto it
         = std::ranges::find_if(m_buttonCapture, [deviceId, axisId](const ButtonMapping& bm) { return bm.GetDeviceId() == deviceId && bm.GetAxisOrButtonId() == axisId; });
      const bool wasCaptured = it != m_buttonCapture.end();
      const int posId = (position < -0.8f) ? 1 : (-0.2f < position && position < 0.2f) ? 2 : (position > 0.8f) ? 3 : 0;
      if (!wasCaptured && (posId == 1 || posId == 3)) // We have a pressed position (either -1 or 1), but we need to keep it long enough to validate it
      {
         m_buttonCapture.emplace_back(nullptr, nullptr, deviceId, axisId);
         m_buttonCapture.back().m_buttonCaptureStartMs = msec();
         m_buttonCapture.back().m_buttonCapturePos = -posId;
      }
      else if (wasCaptured)
      {
         if (it->m_buttonCapturePos == -posId)
         {
            // We are still waiting for the validation of the pressed position (player has slightly moved). Just wait a bit more
         }
         else if (it->m_buttonCapturePos < 0)
         {
            // We have moved away from the pressed position before validating it. Discard this button from the capture
            m_buttonCapture.erase(it);
         }
         else if ((posId == 1 && !it->IsAxisReversed()) || (posId == 2) || (posId == 3 && it->IsAxisReversed()))
         {
            // We have reached a sensible rest position (either -1, 0 or 1), but we need to keep it long enough to validate it
            it->m_buttonCaptureStartMs = msec();
            it->m_buttonCapturePos = posId;
         }
         else
         {
            // We have moved away from the released position before validating it. Wait for another rest position
            it->m_buttonCapturePos = 0;
         }
      }
   }
}

void InputManager::PushTouchEvent(float relativeX, float relativeY, uint64_t timestampNs, bool isPressed)
{
   POINT point;
   point.x = (int)((float)g_pplayer->m_playfieldWnd->GetWidth() * relativeX);
   point.y = (int)((float)g_pplayer->m_playfieldWnd->GetHeight() * relativeY);
   for (auto& region : m_touchRegionMap)
   {
      if (const bool wasPressed = m_inputActions[region.actionId]->GetDirectState(region.directStateSlot); wasPressed == isPressed)
         continue;
      if (!Intersect(region.region, g_pplayer->m_playfieldWnd->GetWidth(), g_pplayer->m_playfieldWnd->GetHeight(), point,
             fmodf(g_pplayer->m_ptable->GetViewSetup().mViewportRotation, 360.0f) != 0.f))
         continue;
      m_inputActions[region.actionId]->SetDirectState(region.directStateSlot, isPressed);
   }
}

#pragma endregion


#pragma region Action Management

void InputManager::CreateInputActions()
{

   auto keyMapping = [](const SDL_Scancode sdlScancode) { return "Key;" + std::to_string(static_cast<int>(sdlScancode)); };

   auto addKeyAction = [this, keyMapping](const string& settingId, const string& label, const SDL_Scancode sdlScancode)
   {
      auto newAction = AddAction(std::make_unique<InputAction>(this, settingId, label, sdlScancode == SDL_SCANCODE_UNKNOWN ? ""s : keyMapping(sdlScancode),
         [](const InputAction& action, bool, bool isPressed)
         {
            if (g_pplayer->m_liveUI->IsInGameUIOpened())
               return;
            CComVariant rgvar[1] = { CComVariant(0x10000 | static_cast<int>(action.GetActionId())) };
            DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
            g_pplayer->m_ptable->FireDispID(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, &dispparams);
         }));
      return newAction->GetActionId();
   };

   auto addFlipperKeyAction = [this, keyMapping](const string& settingId, const string& label, const SDL_Scancode sdlScancode)
   {
      auto newAction = AddAction(std::make_unique<InputAction>(this, settingId, label, keyMapping(sdlScancode),
         [this](const InputAction& action, bool, bool isPressed)
         {
            if (g_pplayer->m_liveUI->IsInGameUIOpened())
               return;
            if (isPressed)
            {
               g_pplayer->m_pininput.PlayRumble(0.f, 0.2f, 150);
               // Debug only, for testing parts of the flipper input lag (note that it excludes device lag, device to computer lag, OS lag, and VPX polling lag)
               m_leftkey_down_usec = usec();
               m_leftkey_down_frame = g_pplayer->m_overall_frames;
            }
            CComVariant rgvar[1] = { CComVariant(0x10000 | static_cast<int>(action.GetActionId())) };
            DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
            g_pplayer->m_ptable->FireDispID(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, &dispparams);
         }));
      return newAction->GetActionId();
   };

   m_leftFlipperActionId = addFlipperKeyAction("LeftFlipper"s, "Left Flipper"s, SDL_SCANCODE_LSHIFT);
   m_rightFlipperActionId = addFlipperKeyAction("RightFlipper"s, "Right Flipper"s, SDL_SCANCODE_RSHIFT);
   m_stagedLeftFlipperActionId = addFlipperKeyAction("LeftStagedFlipper"s, "Left Staged Flipper"s, SDL_SCANCODE_LSHIFT); // SDL_SCANCODE_LGUI
   m_stagedRightFlipperActionId = addFlipperKeyAction("RightStagedFlipper"s, "Right Staged Flipper"s, SDL_SCANCODE_RSHIFT); // SDL_SCANCODE_RALT
   m_leftMagnaActionId = addKeyAction("LeftMagna"s, "Left Magna"s, SDL_SCANCODE_LCTRL);
   m_rightMagnaActionId = addKeyAction("RightMagna"s, "Right Magna"s, SDL_SCANCODE_RCTRL);
   m_launchBallActionId = addKeyAction("LaunchBall"s, "Launch Ball"s, SDL_SCANCODE_RETURN);
   m_leftNudgeActionId = addKeyAction("LeftNudge"s, "Left Nudge"s, SDL_SCANCODE_Z);
   m_rightNudgeActionId = addKeyAction("RightNudge"s, "Right Nudge"s, SDL_SCANCODE_SLASH);
   m_centerNudgeActionId = addKeyAction("CenterNudge"s, "Center Nudge"s, SDL_SCANCODE_SPACE);
   m_tiltActionId = addKeyAction("Tilt"s, "Tilt"s, SDL_SCANCODE_T);
   m_addCreditActionId[0] = addKeyAction("Credit1"s, "Credit (1)"s, SDL_SCANCODE_5);
   m_addCreditActionId[1] = addKeyAction("Credit2"s, "Credit (2)"s, SDL_SCANCODE_4);
   m_addCreditActionId[2] = addKeyAction("Credit3"s, "Credit (3)"s, SDL_SCANCODE_3);
   m_addCreditActionId[3] = addKeyAction("Credit4"s, "Credit (4)"s, SDL_SCANCODE_6);
   m_startActionId = addKeyAction("Start"s, "Start"s, SDL_SCANCODE_1);
   m_extraBallActionId = addKeyAction("ExtraBall"s, "Extra Ball"s, SDL_SCANCODE_B);
   m_lockbarActionId = addKeyAction("Lockbar"s, "Lockbar"s, SDL_SCANCODE_LALT);

   auto pause = AddAction(std::make_unique<InputAction>(this, "Pause"s, "Pause Game"s, keyMapping(SDL_SCANCODE_P),
      [](const InputAction&, bool, bool isPressed)
      {
         if (!isPressed)
            return;
         g_pplayer->SetPlayState(!g_pplayer->IsPlaying());
      }));

   auto perfOverlay = AddAction(std::make_unique<InputAction>(this, "PerfOverlay"s, "Toggle Perf. Overlay"s, keyMapping(SDL_SCANCODE_F11),
      [](const InputAction&, bool, bool isPressed)
      {
         if (!isPressed)
            return;
         g_pplayer->m_liveUI->ToggleFPS();
      }));

   auto exitAction = AddAction(std::make_unique<InputAction>(this, "ExitInteractive"s, "Interactive Exit"s, keyMapping(SDL_SCANCODE_ESCAPE),
      [this](const InputAction&, bool wasPressed, bool isPressed)
      {
         if (!isPressed)
         {
            m_exitPressTimestamp = 0; // Discard long press exit
            return;
         }
         else if (wasPressed // Is this a repeat event (long press)?
            && m_exitPressTimestamp // Exit has not been discarded
            && (g_pplayer->m_time_msec > 1000) // Game has been played at least 1 second
            && ((msec() - m_exitPressTimestamp) > m_exitAppPressLengthMs)) // Exit button has been pressed continuously long enough
         { // Close app if pressed long enough
            g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         }
         else if (g_pplayer->m_liveUI->IsOpened())
         {
            // Discard event as the UI is already opened and will process it
         }
         else if (!wasPressed)
         { // Open interactive UI
            m_exitPressTimestamp = msec();
            g_pplayer->SetCloseState(Player::CS_USER_INPUT);
         }
      }));
   exitAction->SetRepeatPeriod(0);
   m_exitInteractiveActionId = exitAction->GetActionId();

   m_exitGameActionId = AddAction(
      std::make_unique<InputAction>(this, "ExitGame"s, "Exit Game"s, keyMapping(SDL_SCANCODE_Q),
         [](const InputAction& action, bool, bool isPressed)
         {
            // Discard event as the UI is already opened and will process it, except while on the Exit splash where this action is still sensible
            if (g_pplayer->m_liveUI->IsOpened() && !g_pplayer->m_liveUI->m_inGameUI.IsOpened("exit"s))
               return;
            CComVariant rgvar[1] = { CComVariant(0x10000 | static_cast<int>(action.GetActionId())) };
            DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
            g_pplayer->m_ptable->FireDispID(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, &dispparams);
            #ifdef __STANDALONE__
               g_pplayer->SetCloseState(Player::CS_CLOSE_APP);
            #else
               g_pplayer->SetCloseState(Player::CS_STOP_PLAY);
            #endif
         }))->GetActionId();

   auto inGameUI = AddAction(std::make_unique<InputAction>(this, "InGameUI"s, "Toggle InGame UI"s, keyMapping(SDL_SCANCODE_F12),
      [](const InputAction&, bool, bool isPressed)
      {
         if (!isPressed)
            return;
         if (g_pplayer->m_liveUI->IsInGameUIOpened())
            g_pplayer->m_liveUI->HideUI();
         else
            g_pplayer->m_liveUI->OpenInGameUI();
      }));

   auto volumeDown = AddAction(std::make_unique<InputAction>(this, "VolumeDown"s, "Volume Down"s, keyMapping(SDL_SCANCODE_MINUS),
      [this](const InputAction&, bool, bool isPressed)
      {
         if (!isPressed)
            return;
         g_pplayer->m_MusicVolume = clamp(g_pplayer->m_MusicVolume - 1, 0, 100);
         g_pplayer->m_SoundVolume = clamp(g_pplayer->m_SoundVolume - 1, 0, 100);
         g_pplayer->UpdateVolume();
         m_volumeNotificationId = g_pplayer->m_liveUI->PushNotification("Volume: " + std::to_string(g_pplayer->m_MusicVolume) + '%', 500, m_volumeNotificationId);
      }));
   volumeDown->SetRepeatPeriod(75);
   m_volumeDownActionId = volumeDown->GetActionId();

   auto volumeUp = AddAction(std::make_unique<InputAction>(this, "VolumeUp"s, "Volume Up"s, keyMapping(SDL_SCANCODE_EQUALS),
      [this](const InputAction&, bool, bool isPressed)
      {
         if (!isPressed)
            return;
         g_pplayer->m_MusicVolume = clamp(g_pplayer->m_MusicVolume + 1, 0, 100);
         g_pplayer->m_SoundVolume = clamp(g_pplayer->m_SoundVolume + 1, 0, 100);
         g_pplayer->UpdateVolume();
         m_volumeNotificationId = g_pplayer->m_liveUI->PushNotification("Volume: " + std::to_string(g_pplayer->m_MusicVolume) + '%', 500, m_volumeNotificationId);
      }));
   volumeUp->SetRepeatPeriod(75);
   m_volumeUpActionId = volumeUp->GetActionId();

   auto showRules = AddAction(std::make_unique<InputAction>(this, "ShowRules"s, "Show Rules"s, ""s,
      [](const InputAction&, bool, bool isPressed)
      {
         if (!isPressed)
            return;
         if (g_pplayer->m_liveUI->m_inGameUI.IsOpened("table/rules"s))
            g_pplayer->m_liveUI->HideUI();
         else
         {
            g_pplayer->m_liveUI->HideUI();
            g_pplayer->m_liveUI->OpenInGameUI("table/rules"s);
         }
      }));

   m_slamTiltActionId = addKeyAction("SlamTilt"s, "Slam Tilt"s, SDL_SCANCODE_HOME);
   m_coinDoorActionId = addKeyAction("CoinDoor"s, "Coin Door"s, SDL_SCANCODE_END);
   m_resetActionId = addKeyAction("Reset"s, "Reset"s, SDL_SCANCODE_F3);
   static constexpr SDL_Scancode serviceKeys[8] = { SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0, SDL_SCANCODE_6, SDL_SCANCODE_PAGEUP, SDL_SCANCODE_MINUS, SDL_SCANCODE_UNKNOWN };
   for (int i = 0; i < 8; ++i)
      m_serviceActionId[i] = addKeyAction("Service" + std::to_string(i + 1), "Service Button #" + std::to_string(i + 1), serviceKeys[i]);

   auto vrCenter = AddAction(std::make_unique<InputAction>(this, "VRCenter"s, "Align VR view"s, keyMapping(SDL_SCANCODE_KP_5),
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed)
            return;
         if (g_pplayer->m_vrDevice)
            g_pplayer->m_vrDevice->RecenterTable();
      }));

   auto vrUp = AddAction(std::make_unique<InputAction>(this, "VRUp"s, "Move VR view up"s, keyMapping(SDL_SCANCODE_KP_8),
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed)
            return;
         if (g_pplayer->m_vrDevice)
            g_pplayer->m_vrDevice->OffsetTable(0.f, 0.f, 1.f);
      }));

   auto vrDown = AddAction(std::make_unique<InputAction>(this, "VRDown"s, "Move VR view down"s, keyMapping(SDL_SCANCODE_KP_2),
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed)
            return;
         if (g_pplayer->m_vrDevice)
            g_pplayer->m_vrDevice->OffsetTable(0.f, 0.f, -1.f);
      }));

   auto vrFront = AddAction(std::make_unique<InputAction>(this, "VRFront"s, "Move VR view to the front"s, ""s,
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed || !g_pplayer->m_vrDevice)
            return;
         g_pplayer->m_vrDevice->OffsetTable(0.f, 1.f, 0.f);
      }));

   auto vrBack = AddAction(std::make_unique<InputAction>(this, "VRBack"s, "Move VR view to the back"s, ""s,
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed || !g_pplayer->m_vrDevice)
            return;
         g_pplayer->m_vrDevice->OffsetTable(0.f, -1.f, 0.f);
      }));

   auto vrLeft = AddAction(std::make_unique<InputAction>(this, "VRLeft"s, "Move VR view to the left"s, ""s,
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed || !g_pplayer->m_vrDevice)
            return;
         g_pplayer->m_vrDevice->OffsetTable(-1.f, 0.f, 0.f);
      }));

   auto vrRight = AddAction(std::make_unique<InputAction>(this, "VRRight"s, "Move VR view to the right"s, ""s,
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed || !g_pplayer->m_vrDevice)
            return;
         g_pplayer->m_vrDevice->OffsetTable(1.f, 0.f, 0.f);
      }));

   AddAction(std::make_unique<InputAction>(this, "GenTournament"s, "Create Tournament File"s, keyMapping(SDL_SCANCODE_LALT) + " & " + keyMapping(SDL_SCANCODE_1),
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed)
            return;
         if (g_pvp->m_ptableActive->TournamentModePossible())
            g_pvp->GenerateTournamentFile();
      }));

   AddAction(std::make_unique<InputAction>(this, "DebugBalls"s, "Debug Balls"s, keyMapping(SDL_SCANCODE_O),
      [](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed)
            return;
         g_pplayer->m_debugBalls = !g_pplayer->m_debugBalls;
      }));

   AddAction(std::make_unique<InputAction>(this, "Debugger"s, "Open Debugger"s, keyMapping(SDL_SCANCODE_D),
      [this](const InputAction&, bool, bool isPressed)
      {
         m_gameStartedOnce = true; // disable autostart as player as requested debugger instead
         if (g_pplayer->m_liveUI->IsInGameUIOpened() || !isPressed)
            return;
         g_pplayer->m_showDebugger = true;
      }));

   AddAction(std::make_unique<InputAction>(this, "ToggleStereo"s, "Select Stereo Mode"s, keyMapping(SDL_SCANCODE_F10),
      [this](const InputAction&, bool, bool isPressed)
      {
         if (g_pplayer->m_liveUI->IsEditorUIOpened() || !isPressed)
            return;
         if (Is3DTVStereoMode(g_pplayer->m_renderer->m_stereo3D) || IsAnaglyphStereoMode(g_pplayer->m_renderer->m_stereo3D))
         { // Toggle stereo on/off
            g_pplayer->m_renderer->m_stereo3Denabled = !g_pplayer->m_renderer->m_stereo3Denabled;
         }
         else if (g_pplayer->m_renderer->m_stereo3D == STEREO_VR)
         { // Toggle preview mode
            g_pplayer->m_renderer->m_vrPreview = (VRPreviewMode)((g_pplayer->m_renderer->m_vrPreview + 1) % (VRPREVIEW_BOTH + 1));
            g_pplayer->m_liveUI->PushNotification(g_pplayer->m_renderer->m_vrPreview == VRPREVIEW_DISABLED ? "Preview disabled"s // Will only display in headset
                  : g_pplayer->m_renderer->m_vrPreview == VRPREVIEW_LEFT                                   ? "Preview switched to left eye"s
                  : g_pplayer->m_renderer->m_vrPreview == VRPREVIEW_RIGHT                                  ? "Preview switched to right eye"s
                                                                                                           : "Preview switched to both eyes"s,
               2000);
         }
         g_pplayer->m_renderer->InitLayout();
         g_pplayer->m_renderer->UpdateStereoShaderState();
         if (g_pplayer->m_renderer->IsUsingStaticPrepass())
         {
            g_pplayer->m_renderer->DisableStaticPrePass(true);
            g_pplayer->m_renderer->DisableStaticPrePass(false);
         }
      }));

   auto addJoyCustomAction = [this](const string& settingId, const string& label)
   {
      auto newAction = AddAction(std::make_unique<InputAction>(this, settingId, label, ""s,
         [](const InputAction& action, bool, bool isPressed)
         {
            if (g_pplayer->m_liveUI->IsInGameUIOpened())
               return;
            CComVariant rgvar[1] = { CComVariant(0x10000 | static_cast<int>(action.GetActionId())) };
            DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
            g_pplayer->m_ptable->FireDispID(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, &dispparams);
         }));
      return newAction->GetActionId();
   };
   m_joyCustomActionId[0] = addJoyCustomAction("Custom1"s, "Custom Button #1"s);
   m_joyCustomActionId[1] = addJoyCustomAction("Custom2"s, "Custom Button #2"s);
   m_joyCustomActionId[2] = addJoyCustomAction("Custom3"s, "Custom Button #3"s);
   m_joyCustomActionId[3] = addJoyCustomAction("Custom4"s, "Custom Button #4"s);
}

InputAction* InputManager::AddAction(std::unique_ptr<InputAction>&& action)
{
   action->SetActionId(static_cast<int>(m_inputActions.size()));
   m_inputActions.push_back(std::move(action));
   return m_inputActions.back().get();
}

bool InputManager::IsPressed(int actionId) const
{
   assert(0 <= actionId && actionId < static_cast<int>(m_inputActions.size()));
   return m_inputActions[actionId]->IsPressed();
}

int InputManager::GetWindowVirtualKeyForAction(unsigned int actionId) const
{
   #ifndef __STANDALONE__
      // Very basic and inefficient way of searching for a keyboard mapping, but this is only used once to detect table mirroring
      assert(0 <= actionId && actionId < static_cast<int>(m_inputActions.size()));
      vector<ButtonMapping> mapping;
      for (unsigned char vk = VK_ESCAPE; vk <= VK_OEM_3; vk++)
      {
         SDL_Scancode scancode = GetSDLScancodeFromWin32VirtualKey(vk);
         if (scancode == SDL_SCANCODE_UNKNOWN)
            continue;
         mapping.clear();
         mapping.emplace_back(nullptr, nullptr, m_keyboardDeviceId, static_cast<uint16_t>(scancode));
         if (m_inputActions[actionId]->HasMapping(mapping))
            return vk;
      }
   #endif
   return 0;
}

void InputManager::Register(ButtonMapping* mapping)
{
   const uint32_t deviceAndButtonId = mapping->GetDeviceId() << 16 | mapping->GetAxisOrButtonId();
   if (auto it = m_buttonMappings.find(deviceAndButtonId); it != m_buttonMappings.end())
   {
      assert(std::ranges::find(it->second, mapping) == it->second.end());
      it->second.push_back(mapping);
   }
   else
      m_buttonMappings.emplace(deviceAndButtonId, vector { mapping });
}

void InputManager::Unregister(ButtonMapping* mapping)
{
   const uint32_t deviceAndButtonId = mapping->GetDeviceId() << 16 | mapping->GetAxisOrButtonId();
   if (auto it = m_buttonMappings.find(deviceAndButtonId); it != m_buttonMappings.end())
      std::erase(it->second, mapping);
}

void InputManager::OnInputActionStateChanged(InputAction* action)
{
   // Allow plugins to react to action event, filter, ...
   VPXActionEvent event { static_cast<VPXAction>(action->GetActionId()), action->IsPressed() };
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_onActionEventMsgId, &event);

   // Update input state
   if (action->GetActionId() < 64)
   {
      if (action->IsPressed())
         m_inputActionstate.SetPressed(action->GetActionId());
      else
         m_inputActionstate.SetReleased(action->GetActionId());
   }
}

void InputManager::Register(SensorMapping* mapping)
{
   const uint32_t deviceAndAxisId = mapping->GetDeviceId() << 16 | mapping->GetAxisId();
   auto it = m_sensorMappings.find(deviceAndAxisId);
   if (it != m_sensorMappings.end())
   {
      assert(std::ranges::find(it->second, mapping) == it->second.end());
      it->second.push_back(mapping);
   }
   else
      m_sensorMappings.emplace(deviceAndAxisId, vector { mapping });
}

void InputManager::Unregister(SensorMapping* mapping)
{
   const uint32_t deviceAndAxisId = mapping->GetDeviceId() << 16 | mapping->GetAxisId();
   auto it = m_sensorMappings.find(deviceAndAxisId);
   if (it != m_sensorMappings.end())
      std::erase(it->second, mapping);
}

string InputManager::GetMappingLabel(const vector<ButtonMapping>& mappings) const
{
   std::stringstream result;
   bool firstAnd = true;
   for (const ButtonMapping& mapping : mappings)
   {
      if (!firstAnd)
         result << " and ";
      switch (GetDeviceType(mapping.GetDeviceId()))
      {
      case DeviceType::Keyboard:
         result << '\'';
         // SDL_GetScancodeName does not apply the active keyboard layout, therefore not corresponding to what the user expects
         //result << SDL_GetScancodeName(static_cast<SDL_Scancode>(mapping.m_buttonId));
         result << SDL_GetKeyName(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(mapping.GetAxisOrButtonId()), SDL_KMOD_NONE, false));
         result << '\'';
         break;

      default:
         result << GetDeviceName(mapping.GetDeviceId());
         result << " - ";
         result << GetDeviceElementName(mapping.GetDeviceId(), mapping.GetAxisOrButtonId());
         break;
      }
      firstAnd = false;
   }
   return result.str();
}

void InputManager::RegisterOnUpdate(InputAction* action)
{
   m_onUpdateActions.push_back(action);
}

void InputManager::UnregisterOnUpdate(InputAction* action)
{
   std::erase(m_onUpdateActions, action);
}

void InputManager::StartButtonCapture()
{
   assert(m_buttonCaptureState != 1);
   m_buttonCaptureState = 1;
   m_buttonCapture.clear();
}

#pragma endregion


#pragma region Plunger and Nudge

////////////////////////////////////////////////////////////////////
// Plunger and Nudge

void InputManager::SetNudgeFiltered(int index, bool enable)
{
   if (m_nudgeFilter[index] == enable)
      return;
   m_nudgeFilter[index] = enable;
   if (enable)
   {
      m_nudgeXSensor[index]->SetFilter(std::make_unique<NudgeAccelerationFilter>());
      m_nudgeYSensor[index]->SetFilter(std::make_unique<NudgeAccelerationFilter>());
   }
   else
   {
      m_nudgeXSensor[index]->SetFilter(std::make_unique<NoOpSensorFilter>());
      m_nudgeYSensor[index]->SetFilter(std::make_unique<NoOpSensorFilter>());
   }
}

Vertex2D InputManager::GetNudge() const
{
   float weight = 0.f;
   Vertex2D nudge { 0.f, 0.f };
   for (int i = 0; i < 2; i++)
   {
      const std::unique_ptr<PhysicsSensor>& sensorX = GetNudgeXSensor(i);
      const std::unique_ptr<PhysicsSensor>& sensorY = GetNudgeYSensor(i);
      if (!sensorX->IsMapped() || !sensorY->IsMapped())
         continue;
      const float sensorAngle = m_nudgeOrientation[i];
      const float cna = cosf(sensorAngle);
      const float sna = sinf(sensorAngle);
      const float dx = sensorX->GetValue();
      const float dy = sensorY->GetValue();
      nudge.x += dx * cna + dy * sna;
      nudge.y += dy * cna - dx * sna;
      weight += 1.f;
   }
   if (weight < 1.f)
      return nudge;
   return nudge / weight;
}

void InputManager::SetPlungerPos(bool override, const float pos)
{
   // FIXME
}

void InputManager::SetPlungerSpeed(bool override, const float speed)
{
   // FIXME
}

void InputManager::SetNudge(bool override, const float nudgeAccelerationX, const float nudgeAccelerationY)
{
   // FIXME
}

#pragma endregion



void InputManager::PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration)
{
   if (m_rumbleMode == 0)
      return;

   for (const auto& handler : m_inputHandlers)
      handler->PlayRumble(lowFrequencySpeed, highFrequencySpeed, ms_duration);

   #ifdef __LIBVPINBALL__
      if (!g_pvp->m_settings.GetStandalone_Haptics())
         return;

      VPinballLib::RumbleData rumbleData = {
         (uint16_t)(saturate(lowFrequencySpeed) * 65535.f),
         (uint16_t)(saturate(highFrequencySpeed) * 65535.f),
         (uint32_t)ms_duration
      };
      VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_RUMBLE, &rumbleData);
   #endif
}

void InputManager::Autostart(const uint32_t initialDelayMs, const uint32_t retryDelayMs)
{
   // Don't perform autostart if a game has been started once.
   // Note that this is hacky/buggy as it rely on the table to create ball only on game start
   // while lots of tables do create balls on startup instead.
   if (m_gameStartedOnce)
      return;
   if (!g_pplayer->m_vball.empty())
   {
      m_gameStartedOnce = true;
      return;
   }

   const uint32_t now = msec();
   if (m_autoStartTimestamp == 0)
   {
      m_autoStartTimestamp = now;
      return;
   }

   const uint32_t elapsed = now - m_autoStartTimestamp;
   if (m_autoStartPressed // Start button is down.
      && (elapsed > 100)) // Start button has been down for at least 0.10 seconds.
   {
      // Release start.
      m_autoStartTimestamp = now;
      m_autoStartPressed = false;
      if (m_autoStartDirectStateSlot != -1)
         m_inputActions[m_startActionId]->SetDirectState(m_autoStartDirectStateSlot, false);
      PLOGD << "Autostart: Release";
   }
   else if (!m_autoStartPressed                                       // Start button is up.
       && (    ( m_autoStartDoneOnce && (elapsed > retryDelayMs))     // Not started and last attempt was at least AutoStartRetry seconds ago.
            || (!m_autoStartDoneOnce && (elapsed > initialDelayMs)))) // Never attempted and autostart time has elapsed.
   {
      // Press start.
      m_autoStartTimestamp = now;
      m_autoStartPressed = true;
      m_autoStartDoneOnce = true;
      if (m_autoStartDirectStateSlot == -1)
         m_autoStartDirectStateSlot = m_inputActions[m_startActionId]->NewDirectStateSlot();
      m_inputActions[m_startActionId]->SetDirectState(m_autoStartDirectStateSlot, true);
      PLOGD << "Autostart: Press";
   }
}


#ifdef _WIN32
// Somewhat hacky code to handle accessibility features and OS levels shortcuts but done as per Microsoft recommendations
// See: https://learn.microsoft.com/en-us/windows/win32/dxtecharts/disabling-shortcut-keys-in-games

LRESULT CALLBACK InputManager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   if (auto p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam); //
      nCode == HC_ACTION //
      && (wParam == WM_KEYDOWN || wParam == WM_KEYUP) //
      && (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN) //
      && g_pplayer //
      && g_pplayer->m_playfieldWnd->IsFocused())
   {
      // Block further processing and directly send event to SDL for internal processing
      // Note that this will not block hotkeys (for example Xbox Game Bar: Win+G, Win+Alt+R, etc.)
      // Also requesting keyboard list on each press is somewhat overkill, but kept for the sake of simplicity
      int nKeyboards;
      if (SDL_KeyboardID* keyboards = SDL_GetKeyboards(&nKeyboards); keyboards != nullptr && nKeyboards > 0)
      {
         SDL_Event event;
         event.type = wParam == WM_KEYDOWN ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
         event.common.timestamp = SDL_GetTicksNS();
         event.key.scancode = p->vkCode == VK_LWIN ? SDL_SCANCODE_LGUI : SDL_SCANCODE_RGUI;
         event.key.key = p->vkCode == VK_LWIN ? SDLK_LGUI : SDLK_RGUI;
         event.key.mod = SDL_GetModState();
         event.key.raw = (Uint16)p->vkCode;
         event.key.down = wParam == WM_KEYDOWN;
         event.key.repeat = false;
         event.key.windowID = SDL_GetWindowID(g_pplayer->m_playfieldWnd->GetCore());
         event.key.which = keyboards[0];
         SDL_PushEvent(&event);
         SDL_free(keyboards);
      }
      return 1;
   }
   return CallNextHookEx(g_pplayer->m_pininput.m_hKeyboardHook, nCode, wParam, lParam);
}

void InputManager::AllowAccessibilityShortcutKeys(bool bAllowKeys)
{
   if (bAllowKeys)
   {
      // Restore StickyKeys/etc to original state and enable Windows key
      SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);
      SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_StartupToggleKeys, 0);
      SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &m_StartupFilterKeys, 0);
   }
   else
   {
      // Disable StickyKeys/etc shortcuts, even if the accessibility feature has been turned on by the user

      STICKYKEYS skOff = m_StartupStickyKeys;
      //if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0)
      {
         // Disable the hotkey and the confirmation
         skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
         skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

         SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
      }

      TOGGLEKEYS tkOff = m_StartupToggleKeys;
      //if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0)
      {
         // Disable the hotkey and the confirmation
         tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
         tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

         SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
      }

      FILTERKEYS fkOff = m_StartupFilterKeys;
      //if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0)
      {
         // Disable the hotkey and the confirmation
         fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
         fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

         SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
      }
   }
}
#endif
