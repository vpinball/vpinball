// license:GPLv3+

#include "core/stdafx.h"
#include "core/VPXPluginAPIImpl.h"
#include "renderer/VRDevice.h"

#ifdef ENABLE_SDL_VIDEO
   #include "imgui/imgui_impl_sdl3.h"
#endif

#ifdef __LIBVPINBALL__
   #include "standalone/VPinballLib.h"
#endif

#ifdef _WIN32
   #include "input/Win32InputKeyboardHandler.h"
   #include "input/DirectInputKeyboardHandler.h"
   #include "input/DirectInputMouseHandler.h"
   #include "input/DirectInputJoystickHandler.h"
#endif

#ifdef ENABLE_XINPUT
   #include "input/XInputJoystickHandler.h"
#endif

#ifdef ENABLE_SDL_INPUT
   #include "input/SDLInputHandler.h"
#endif

#ifndef __LIBVPINBALL__
   #include "input/OpenPinDevHandler.h"
#endif


PinInput::PinInput()
   : m_onActionEventMsgId(VPXPluginAPIImpl::GetInstance().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_ACTION_CHANGED))
   #ifdef ENABLE_SDL_INPUT
      , m_joypmcancel(SDL_GAMEPAD_BUTTON_NORTH + 1)
   #endif
{
#ifdef _WIN32
   // Cache the initial state of sticky keys
   m_startupStickyKeys.cbSize = sizeof(STICKYKEYS);
   SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_startupStickyKeys, 0);
#endif
}

PinInput::~PinInput()
{
   UnInit();
   VPXPluginAPIImpl::GetInstance().ReleaseMsgID(m_onActionEventMsgId);
}

#ifdef _WIN32
void PinInput::SetFocusWindow(HWND focusWnd)
{
   m_focusHWnd = focusWnd;
}
#endif

void PinInput::Init()
{
   UnInit();

   const Settings& settings = g_pvp->m_settings;

   m_exitPressTimestamp = 0;
   m_exitAppPressLengthMs = settings.LoadValueWithDefault(Settings::Player, "Exitconfirm"s, 120) * 1000 / 60; // this is supposed to be seconds, but is seconds*60  :/

   m_override_default_buttons = settings.LoadValueWithDefault(Settings::Player, "PBWDefaultLayout"s, m_override_default_buttons);
   m_disable_esc = settings.LoadValueWithDefault(Settings::Player, "DisableESC"s, m_disable_esc);
   m_deadz = settings.LoadValueWithDefault(Settings::Player, "DeadZone"s, 0) * JOYRANGEMX / 100;

   m_linearPlunger = false;
   m_plungerPosDirty = true;
   m_plungerSpeedDirty = true;
   m_plunger_retract = settings.LoadValueWithDefault(Settings::Player, "PlungerRetract"s, m_plunger_retract);

   m_accelerometerDirty = true;
   m_accelerometerEnabled = settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true); // true if electronic accelerometer enabled
   m_accelerometerFaceUp = settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, true); // true is normal mounting (left hand coordinates)
   m_accelerometerAngle = 0.0f; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   const bool accel = settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false);
   if (accel)
      m_accelerometerAngle = (float)settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 0);
   m_accelerometerSensitivity = clamp((float)settings.LoadValueWithDefault(Settings::Player, "NudgeSensitivity"s, 500) * (float)(1.0 / 1000.0), 0.f, 1.f);
   m_accelerometerMax.x = static_cast<float>(settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100) * JOYRANGEMX) / 100.f;
   m_accelerometerMax.y = static_cast<float>(settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100) * JOYRANGEMX) / 100.f;
   m_accelerometerGain.x = dequantizeUnsignedPercentNoClamp(settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150));
   m_accelerometerGain.y = dequantizeUnsignedPercentNoClamp(settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150));

   m_joypmbuyin = settings.LoadValueWithDefault(Settings::Player, "JoyPMBuyIn"s, m_joypmbuyin);
   m_joypmcoin3 = settings.LoadValueWithDefault(Settings::Player, "JoyPMCoin3"s, m_joypmcoin3);
   m_joypmcoin4 = settings.LoadValueWithDefault(Settings::Player, "JoyPMCoin4"s, m_joypmcoin4);
   m_joypmcoindoor = settings.LoadValueWithDefault(Settings::Player, "JoyPMCoinDoor"s, m_joypmcoindoor);
   m_joypmcancel = settings.LoadValueWithDefault(Settings::Player, "JoyPMCancel"s, m_joypmcancel);
   m_joypmdown = settings.LoadValueWithDefault(Settings::Player, "JoyPMDown"s, m_joypmdown);
   m_joypmup = settings.LoadValueWithDefault(Settings::Player, "JoyPMUp"s, m_joypmup);
   m_joypmenter = settings.LoadValueWithDefault(Settings::Player, "JoyPMEnter"s, m_joypmenter);

   m_joycustom1 = settings.LoadValueWithDefault(Settings::Player, "JoyCustom1"s, m_joycustom1);
   m_joycustom1key = settings.LoadValueWithDefault(Settings::Player, "JoyCustom1Key"s, m_joycustom1key);
   m_joycustom2 = settings.LoadValueWithDefault(Settings::Player, "JoyCustom2"s, m_joycustom2);
   m_joycustom2key = settings.LoadValueWithDefault(Settings::Player, "JoyCustom2Key"s, m_joycustom2key);
   m_joycustom3 = settings.LoadValueWithDefault(Settings::Player, "JoyCustom3"s, m_joycustom3);
   m_joycustom3key = settings.LoadValueWithDefault(Settings::Player, "JoyCustom3Key"s, m_joycustom3key);
   m_joycustom4 = settings.LoadValueWithDefault(Settings::Player, "JoyCustom4"s, m_joycustom4);
   m_joycustom4key = settings.LoadValueWithDefault(Settings::Player, "JoyCustom4Key"s, m_joycustom4key);

   for (unsigned int i = 0; i < eCKeys; ++i)
      MapActionToKeyboard(static_cast<EnumAssignKeys>(i), g_pvp->m_settings.LoadValueInt(Settings::Player, regkey_string[i]), true);

   MapActionToMouse(eLeftFlipperKey, settings.LoadValueInt(Settings::Player, "JoyLFlipKey"s), true);
   MapActionToMouse(eRightFlipperKey, settings.LoadValueInt(Settings::Player, "JoyRFlipKey"s), true);
   MapActionToMouse(ePlungerKey, settings.LoadValueInt(Settings::Player, "JoyPlungerKey"s), true);
   MapActionToMouse(eLeftTiltKey, settings.LoadValueInt(Settings::Player, "JoyLTiltKey"s), true);
   MapActionToMouse(eRightTiltKey, settings.LoadValueInt(Settings::Player, "JoyCTiltKey"s), true);
   MapActionToMouse(eCenterTiltKey, settings.LoadValueInt(Settings::Player, "JoyRTiltKey"s), true);

   memset(&m_inputState, 0, sizeof(m_inputState));
   m_nextKeyPressedTime = 0;
   m_rumbleMode = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "RumbleMode"s, 3);

   // Initialize device handlers

   #if defined(ENABLE_SDL_INPUT)
      auto inputAPI = static_cast<InputAPI>(g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "InputApi"s, PI_SDL));
   #else
      auto inputAPI = static_cast<InputAPI>(g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "InputApi"s, PI_DIRECTINPUT));
   #endif

   if (inputAPI == PI_SDL)
   #if defined(ENABLE_SDL_INPUT)
   {
      m_inputHandlers.push_back(std::make_unique<SDLInputHandler>(*this));
      m_sdlHandler = static_cast<SDLInputHandler*>(m_inputHandlers.back().get());
   }
   #else
      inputAPI = PI_DIRECTINPUT;
   #endif

   if (inputAPI == PI_XINPUT)
   #ifdef ENABLE_XINPUT
      m_inputHandlers.push_back(std::make_unique<XInputJoystickHandler>(*this, m_focusHWnd));
   #else
      inputAPI = PI_DIRECTINPUT;
   #endif

   #ifdef _WIN32
      if (inputAPI != PI_SDL) // XInput does not handle keyboard
      #ifdef USE_DINPUT_FOR_KEYBOARD
         m_inputHandlers.push_back(std::make_unique<DirectInputKeyboardHandler>(*this, m_focusHWnd));
      #else
         m_inputHandlers.push_back(std::make_unique<Win32InputKeyboardHandler>(*this, m_focusHWnd));
      #endif
      if (inputAPI == PI_DIRECTINPUT)
      {
         m_inputHandlers.push_back(std::make_unique<DirectInputJoystickHandler>(*this, m_focusHWnd));
         m_joystickDIHandler = static_cast<DirectInputJoystickHandler*>(m_inputHandlers.back().get());
         m_rumbleMode = 0;
      }

      if (settings.LoadValueWithDefault(Settings::Player, "EnableMouseInPlayer"s, true))
         m_inputHandlers.push_back(std::make_unique<DirectInputMouseHandler>(*this, m_focusHWnd));

      // Disable Sticky Keys
      STICKYKEYS newStickyKeys = {};
      newStickyKeys.cbSize = sizeof(STICKYKEYS);
      newStickyKeys.dwFlags = 0;
      SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &newStickyKeys, SPIF_SENDCHANGE);
   #endif

   #ifndef __LIBVPINBALL__
      m_inputHandlers.push_back(std::make_unique<OpenPinDevHandler>(*this));
   #endif
}

void PinInput::UnInit()
{
   m_actionMappings.clear();
   m_analogActionMappings.clear();
   m_inputHandlers.clear();

   #if defined(ENABLE_SDL_INPUT)
      m_sdlHandler = nullptr;
   #endif

   #ifdef _WIN32
      // restore the state of the sticky keys
      SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_startupStickyKeys, SPIF_SENDCHANGE);
      m_joystickDIHandler = nullptr;
   #endif
}


void PinInput::UnmapJoy(uint64_t joyId)
{
   std::erase_if(m_actionMappings, [joyId](const ActionMapping& am) { return (am.type == ActionMapping::AM_Joystick) && (am.joystickId == joyId); });
   std::erase_if(m_analogActionMappings, [joyId](const AnalogActionMapping& am) { return am.joystickId == joyId; });
}

void PinInput::MapActionToMouse(EnumAssignKeys action, int button, bool replace)
{
   if (replace)
      std::erase_if(m_actionMappings, [action](const ActionMapping& am) { return (am.action == action) && (am.type == ActionMapping::AM_Mouse); });
   ActionMapping mapping;
   mapping.action = action;
   mapping.type = ActionMapping::AM_Mouse;
   mapping.buttonId = button;
   m_actionMappings.push_back(mapping);
}

void PinInput::MapActionToKeyboard(EnumAssignKeys action, int keycode, bool replace)
{
   if (replace)
      std::erase_if(m_actionMappings, [action](const ActionMapping& am) { return (am.action == action) && (am.type == ActionMapping::AM_Keyboard); });
   ActionMapping mapping;
   mapping.action = action;
   mapping.type = ActionMapping::AM_Keyboard;
   mapping.keycode = keycode;
   m_actionMappings.push_back(mapping);
}

void PinInput::MapActionToJoystick(EnumAssignKeys action, uint64_t joystickId, int buttonId, bool replace)
{
   if (replace)
      std::erase_if(m_actionMappings, [action](const ActionMapping& am) { return (am.action == action) && (am.type == ActionMapping::AM_Joystick); });
   ActionMapping mapping;
   mapping.action = action;
   mapping.type = ActionMapping::AM_Joystick;
   mapping.joystickId = joystickId;
   mapping.buttonId = buttonId;
   m_actionMappings.push_back(mapping);
}

void PinInput::MapAnalogActionToJoystick(AnalogAction output, uint64_t joystickId, int axisId, bool revert, bool replace)
{
   if (replace)
      std::erase_if(m_analogActionMappings, [output](const AnalogActionMapping& am) { return (am.output == output); });
   AnalogActionMapping mapping;
   mapping.joystickId = joystickId;
   mapping.axisId = axisId;
   mapping.revert = revert;
   mapping.output = output;
   m_analogActionMappings.push_back(mapping);
}


// Since input event processing is single threaded on the main game logic thread, we do not queue events but directly process them

void PinInput::PushActionEvent(EnumAssignKeys action, bool isPressed)
{
   InputEvent e;
   e.type = InputEvent::Type::Action;
   e.action = action;
   e.isPressed = isPressed;
   ProcessEvent(e);
}

void PinInput::PushMouseEvent(int button, bool isPressed)
{
   InputEvent e;
   e.type = InputEvent::Type::Mouse;
   e.buttonId = button;
   e.isPressed = isPressed;
   ProcessEvent(e);
}

void PinInput::PushKeyboardEvent(int keycode, bool isPressed)
{
   InputEvent e;
   e.type = InputEvent::Type::Keyboard;
   e.keycode = keycode;
   e.isPressed = isPressed;
   ProcessEvent(e);
}

void PinInput::PushJoystickButtonEvent(uint64_t joystickId, int buttonId, bool isPressed)
{
   InputEvent e;
   e.type = InputEvent::Type::JoyButton;
   e.joystickId = joystickId;
   e.buttonId = buttonId + 1;
   e.isPressed = isPressed;
   ProcessEvent(e);
}

void PinInput::PushJoystickAxisEvent(uint64_t joystickId, int axisId, int value)
{
   InputEvent e;
   e.type = InputEvent::Type::JoyAxis;
   e.joystickId = joystickId;
   e.axisId = axisId;
   e.value = value;
   ProcessEvent(e);
}


#if defined(ENABLE_SDL_INPUT)
void PinInput::HandleSDLEvent(SDL_Event &e)
{
   if (m_sdlHandler)
      m_sdlHandler->HandleSDLEvent(e);
}
#endif

#if defined(_WIN32)
DirectInputJoystickHandler* PinInput::GetDirectInputJoystickHandler() const
{
   return m_joystickDIHandler;
}
#endif

const PinInput::InputState& PinInput::GetInputState() const
{
   return m_inputState;
}
 
void PinInput::SetInputState(const InputState& state)
{
   uint64_t changes = state.actionState ^ m_inputState.actionState;
   uint64_t mask = 1ull;
   for (int i = 0; i < eCKeys; i++, mask <<= 1)
      if (changes & mask)
         FireActionEvent(static_cast<EnumAssignKeys>(i), (state.actionState & mask) != 0);
   m_inputState = state;
}

const Vertex2D& PinInput::GetNudge() const
{
   if (m_accelerometerDirty)
   {
      // Accumulate over all accelerometer devices
      m_accelerometer.SetZero();
      for (const auto& aam : m_analogActionMappings)
      {
         if (aam.output == AnalogAction::AM_NudgeX)
            m_accelerometer.x += clamp(aam.value, -m_accelerometerMax.x, m_accelerometerMax.x);
         if (aam.output == AnalogAction::AM_NudgeY)
            m_accelerometer.y += clamp(aam.value, -m_accelerometerMax.y, m_accelerometerMax.y);
      }

      // Scale to normalized float range, -1.0f..+1.0f
      float dx = m_accelerometer.x / static_cast<float>(JOYRANGEMX);
      const float dy = m_accelerometer.y / static_cast<float>(JOYRANGEMX);

      // Apply table mirroring
      if (g_pplayer->m_ptable->m_tblMirrorEnabled)
         dx = -dx;

      // Rotate to match hardware mounting orientation, including left or right coordinates (suppose same orientation of all hardwares, which could be improved)
      const float a = ANGTORAD(m_accelerometerAngle);
      const float cna = cosf(a);
      const float sna = sinf(a);
      m_accelerometer.x = m_accelerometerGain.x * (dx * cna + dy * sna) * (1.0f - m_accelerometerSensitivity); // calc Green's transform component for X
      const float nugY   = m_accelerometerGain.y * (dy * cna - dx * sna) * (1.0f - m_accelerometerSensitivity); // calc Green's transform component for Y
      m_accelerometer.y = m_accelerometerFaceUp ? nugY : -nugY; // add as left or right hand coordinate system

      m_accelerometerDirty = false;
   }
   return m_accelerometer;
}

void PinInput::SetNudge(const Vertex2D& nudge)
{
   m_accelerometer = nudge;
   m_accelerometerDirty = false;
}


bool PinInput::HasMechPlunger() const
{
   const auto& it = std::ranges::find_if(
      m_analogActionMappings.begin(), m_analogActionMappings.end(), [](const AnalogActionMapping& mapping) { return (mapping.output == AnalogAction::AM_PlungerPos); });
   return it != m_analogActionMappings.end();
}

float PinInput::GetPlungerPos() const
{
   if (m_plungerPosDirty)
   {
      m_plungerPos = 0.f;
      for (const auto& aam : m_analogActionMappings)
      {
         if (aam.output == AnalogAction::AM_PlungerPos)
            m_plungerPos += aam.value;
      }
      m_plungerPosDirty = false;
      //PLOGD << "Plunger pos  : " << m_plungerPos / static_cast<float>(JOYRANGEMX);
   }
   return m_plungerPos;
}

void PinInput::SetPlungerPos(float pos)
{
   constexpr uint64_t extPlungerId = static_cast<uint64_t>(0xF00000000);
   m_plungerPosDirty = true;
   for (auto& aam : m_analogActionMappings)
      if (aam.output == AnalogAction::AM_PlungerPos && aam.joystickId == extPlungerId)
      {
         aam.value = pos;
         return;
      }
   MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, extPlungerId, 0, false, false);
   SetPlungerPos(pos);
}

bool PinInput::HasMechPlungerSpeed() const
{
   const auto& it = std::ranges::find_if(
      m_analogActionMappings.begin(), m_analogActionMappings.end(), [](const AnalogActionMapping& mapping) { return (mapping.output == AnalogAction::AM_PlungerSpeed); });
   return it != m_analogActionMappings.end();
}

float PinInput::GetPlungerSpeed() const
{
   if (m_plungerSpeedDirty)
   {
      m_plungerSpeed = 0.f;
      for (const auto& aam : m_analogActionMappings)
      {
         if (aam.output == AnalogAction::AM_PlungerSpeed)
            m_plungerSpeed += aam.value;
      }
      m_plungerSpeedDirty = false;
      //PLOGD << "Plunger speed: " << m_plungerSpeed / static_cast<float>(JOYRANGEMX);
   }
   return m_plungerSpeed;
}

void PinInput::SetPlungerSpeed(float speed)
{
   constexpr uint64_t extPlungerId = static_cast<uint64_t>(0xF00000000);
   m_plungerPosDirty = true;
   for (auto& aam : m_analogActionMappings)
      if (aam.output == AnalogAction::AM_PlungerSpeed && aam.joystickId == extPlungerId)
      {
         aam.value = speed;
         return;
      }
   MapAnalogActionToJoystick(AnalogAction::AM_PlungerSpeed, extPlungerId, 1, false, false);
   SetPlungerPos(speed);
}

void PinInput::PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration)
{
   if (m_rumbleMode == 0)
      return;

   for (const auto& handler : m_inputHandlers)
      handler->PlayRumble(lowFrequencySpeed, highFrequencySpeed, ms_duration);

   #ifdef __LIBVPINBALL__
      VPinballLib::RumbleData rumbleData = {
         (Uint16)(saturate(lowFrequencySpeed) * 65535.f),
         (Uint16)(saturate(highFrequencySpeed) * 65535.f),
         (Uint32)ms_duration
      };
      VPinballLib::VPinball::SendEvent(VPinballLib::Event::Rumble, &rumbleData);
   #endif
}

void PinInput::FireGenericKeyEvent(const int dispid, int keycode)
{
   // Check if we are mirrored & Swap left & right input.
   if (g_pplayer->m_ptable->m_tblMirrorEnabled)
   {
      if (keycode == DIK_LSHIFT) keycode = DIK_RSHIFT;
      else if (keycode == DIK_RSHIFT) keycode = DIK_LSHIFT;
      else if (keycode == DIK_LEFT) keycode = DIK_RIGHT;
      else if (keycode == DIK_RIGHT) keycode = DIK_LEFT;
   }
   g_pplayer->m_ptable->FireGenericKeyEvent(dispid, keycode);
}

void PinInput::FireActionEvent(EnumAssignKeys action, bool isPressed)
{
   // Allow plugins to react to action event, filter, ...

   VPXActionEvent event { static_cast<VPXAction>(action), isPressed };
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_onActionEventMsgId, &event);
   isPressed = event.isPressed;

   // Update input state
 
   if (g_pplayer->m_ptable->m_tblMirrorEnabled)
   {
      if (action == eLeftFlipperKey) action = eRightFlipperKey;
      else if (action == eRightFlipperKey) action = eLeftFlipperKey;
      else if (action == eStagedLeftFlipperKey) action = eStagedRightFlipperKey;
      else if (action == eStagedRightFlipperKey) action = eStagedLeftFlipperKey;
      else if (action == eLeftMagnaSave) action = eRightMagnaSave;
      else if (action == eRightMagnaSave) action = eLeftMagnaSave;
   }

   if (isPressed == m_inputState.IsKeyDown(action))
      return; // Action has been discarded by a plugin
   else if (isPressed)
      m_inputState.SetPressed(action);
   else
      m_inputState.SetReleased(action);

   // Process action

   switch (action)
   {
   case eDBGBalls:
      if (isPressed)
         g_pplayer->m_debugBalls = !g_pplayer->m_debugBalls;
      break;

   case eFrameCount:
      if (isPressed)
         g_pplayer->m_liveUI->ToggleFPS();
      break;

   case ePause:
      if (isPressed)
         g_pplayer->SetPlayState(!g_pplayer->IsPlaying());
      break;

   case eTweak:
      if (isPressed)
      {
         if (g_pplayer->m_liveUI->IsTweakMode())
            g_pplayer->m_liveUI->HideUI();
         else
            g_pplayer->m_liveUI->OpenTweakMode();
      }
      break;

   case eDebugger:
      m_gameStartedOnce = true; // disable autostart as player as requested debugger instead
      if (!isPressed)
         g_pplayer->m_showDebugger = true;
      break;

   case eLeftFlipperKey:
      // Left flipper releases ball control
      if (isPressed)
      {
         delete g_pplayer->m_pBCTarget;
         g_pplayer->m_pBCTarget = nullptr;
      }
      break;

   case eStartGameKey:
      if (m_inputState.IsKeyDown(eLockbarKey) && isPressed && g_pvp->m_ptableActive->TournamentModePossible())
         g_pvp->GenerateTournamentFile();
      break;

   case eEnable3D:
      if (isPressed)
      {
         if (IsAnaglyphStereoMode(g_pplayer->m_renderer->m_stereo3D))
         {
            // Select next glasses or toggle stereo on/off
            int glassesIndex = g_pplayer->m_renderer->m_stereo3D - STEREO_ANAGLYPH_1;
            if (!g_pplayer->m_renderer->m_stereo3Denabled && glassesIndex != 0)
            {
               g_pplayer->m_liveUI->PushNotification("Stereo enabled"s, 2000);
               g_pplayer->m_renderer->m_stereo3Denabled = true;
            }
            else
            {
               const int dir = (m_inputState.IsKeyDown(eLeftFlipperKey) || m_inputState.IsKeyDown(eRightFlipperKey)) ? -1 : 1;
               // Loop back with shift pressed
               if (!g_pplayer->m_renderer->m_stereo3Denabled && glassesIndex <= 0 && dir == -1)
               {
                  g_pplayer->m_renderer->m_stereo3Denabled = true;
                  glassesIndex = 9;
               }
               else if (g_pplayer->m_renderer->m_stereo3Denabled && glassesIndex <= 0 && dir == -1)
               {
                  g_pplayer->m_liveUI->PushNotification("Stereo disabled"s, 2000);
                  g_pplayer->m_renderer->m_stereo3Denabled = false;
               }
               // Loop forward
               else if (!g_pplayer->m_renderer->m_stereo3Denabled)
               {
                  g_pplayer->m_liveUI->PushNotification("Stereo enabled"s, 2000);
                  g_pplayer->m_renderer->m_stereo3Denabled = true;
               }
               else if (glassesIndex >= 9 && dir == 1)
               {
                  g_pplayer->m_liveUI->PushNotification("Stereo disabled"s, 2000);
                  glassesIndex = 0;
                  g_pplayer->m_renderer->m_stereo3Denabled = false;
               }
               else
               {
                  glassesIndex += dir;
               }
               g_pplayer->m_renderer->m_stereo3D = (StereoMode)(STEREO_ANAGLYPH_1 + glassesIndex);
               if (g_pplayer->m_renderer->m_stereo3Denabled)
               {
                  string name;
                  static const string defaultNames[]
                     = { "Red/Cyan"s, "Green/Magenta"s, "Blue/Amber"s, "Cyan/Red"s, "Magenta/Green"s, "Amber/Blue"s, "Custom 1"s, "Custom 2"s, "Custom 3"s, "Custom 4"s };
                  if (!g_pvp->m_settings.LoadValue(Settings::Player, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Name"s), name))
                     name = defaultNames[glassesIndex];
                  g_pplayer->m_liveUI->PushNotification("Profile #"s.append(std::to_string(glassesIndex + 1)).append(" '"s).append(name).append("' activated"s), 2000);
               }
            }
         }
         else if (Is3DTVStereoMode(g_pplayer->m_renderer->m_stereo3D))
         {
            // Toggle stereo on/off
            g_pplayer->m_renderer->m_stereo3Denabled = !g_pplayer->m_renderer->m_stereo3Denabled;
         }
         else if (g_pplayer->m_renderer->m_stereo3D == STEREO_VR)
         {
            g_pplayer->m_renderer->m_vrPreview = (VRPreviewMode)((g_pplayer->m_renderer->m_vrPreview + 1) % (VRPREVIEW_BOTH + 1));
            g_pplayer->m_liveUI->PushNotification(g_pplayer->m_renderer->m_vrPreview == VRPREVIEW_DISABLED ? "Preview disabled"s // Will only display in headset
                  : g_pplayer->m_renderer->m_vrPreview == VRPREVIEW_LEFT                                   ? "Preview switched to left eye"s
                  : g_pplayer->m_renderer->m_vrPreview == VRPREVIEW_RIGHT                                  ? "Preview switched to right eye"s
                                                                                                           : "Preview switched to both eyes"s,
               2000);
         }
         g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DEnabled"s, g_pplayer->m_renderer->m_stereo3Denabled);
         g_pplayer->m_renderer->InitLayout();
         g_pplayer->m_renderer->UpdateStereoShaderState();
      }
      break;

   #if defined(ENABLE_VR)
   case eTableRecenter:
      if (g_pplayer->m_vrDevice && !isPressed)
         g_pplayer->m_vrDevice->RecenterTable();
      break;

   case eTableUp:
      if (g_pplayer->m_vrDevice && !isPressed)
         g_pplayer->m_vrDevice->TableUp();
      break;

   case eTableDown:
      if (g_pplayer->m_vrDevice && !isPressed)
         g_pplayer->m_vrDevice->TableDown();
      break;
   #endif
   }

   if (!g_pplayer->m_liveUI->IsTweakMode() && isPressed && (action == eLeftFlipperKey || action == eRightFlipperKey || action == eStagedLeftFlipperKey || action == eStagedRightFlipperKey))
   {
      g_pplayer->m_pininput.PlayRumble(0.f, 0.2f, 150);
      // Debug only, for testing parts of the flipper input lag
      m_leftkey_down_usec = usec();
      m_leftkey_down_frame = g_pplayer->m_overall_frames;
   }

   if (!g_pplayer->m_liveUI->IsTweakMode() && !g_pplayer->m_liveUI->HasKeyboardCapture())
      g_pplayer->m_ptable->FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[action]);

   if (((action == eEscape) && !m_disable_esc) || (action == eExitGame))
   {
      // Do not trigger if the UI is already opened (keyboard is handled in it)
      if (!g_pplayer->m_liveUI->IsOpened())
      {
         m_gameStartedOnce = true; // Disable autostart as player has requested close
         if (isPressed)
         {
            m_exitPressTimestamp = msec();
         }
         else
         {
            // Open UI on key up since a long press should not trigger the UI (direct exit from the app)
            g_pplayer->SetCloseState(Player::CS_USER_INPUT);
            m_exitPressTimestamp = 0;
            #ifdef __STANDALONE__
               if (action == eExitGame)
                  g_pplayer->SetCloseState(Player::CS_CLOSE_APP);
            #endif
         }
      }
   }
}

void PinInput::Autostart(const U32 initialDelayMs, const U32 retryDelayMs)
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

   const U32 now = msec();
   if (m_autoStartTimestamp == 0)
   {
      m_autoStartTimestamp = now;
      return;
   }

   const U32 elapsed = now - m_autoStartTimestamp;
   if (m_autoStartPressed // Start button is down.
      && (elapsed > 100)) // Start button has been down for at least 0.10 seconds.
   {
      // Release start.
      m_autoStartTimestamp = now;
      m_autoStartPressed = false;
      FireActionEvent(eStartGameKey, false);
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
      FireActionEvent(eStartGameKey, true);
      PLOGD << "Autostart: Press";
   }
}

// Setup a hardware device button and analog input mapping
// For the time being, an action may only be bound to one button as we do not handle combination of multiple sources
// For analog input, multiple source are supported, averaging for nudge and summing for plunger (assuming there is only one non 0)
void PinInput::SetupJoyMapping(uint64_t joystickId, InputLayout inputLayout)
{
   const int lr_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LRAxis"s, 1);
   const int ud_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "UDAxis"s, 2);
   const bool lr_axis_reverse = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LRAxisFlip"s, false);
   const bool ud_axis_reverse = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "UDAxisFlip"s, false);
   const int plunger_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerAxis"s, 3);
   const int plunger_speed_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedAxis"s, 0);
   const bool plunger_reverse = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ReversePlungerAxis"s, false);

   switch (inputLayout)
   {
   case InputLayout::PBWizard:
      SetupJoyMapping(joystickId, InputLayout::Generic);
      MapActionToJoystick(ePlungerKey, joystickId, 0, true);
      MapActionToJoystick(eRightFlipperKey, joystickId, 1, true);
      MapActionToJoystick(eRightMagnaSave, joystickId, 2, true);
      MapActionToJoystick(eVolumeDown, joystickId, 3, true);
      MapActionToJoystick(eVolumeUp, joystickId, 4, true);
      // Button 5 is not mapped
      MapActionToJoystick(eEscape, joystickId, 6, true);
      MapActionToJoystick(eExitGame, joystickId, 7, true);
      MapActionToJoystick(eStartGameKey, joystickId, 8, true);
      MapActionToJoystick(eLeftFlipperKey, joystickId, 9, true);
      MapActionToJoystick(eLeftMagnaSave, joystickId, 10, true);
      MapActionToJoystick(eAddCreditKey, joystickId, 11, true);
      MapActionToJoystick(eAddCreditKey2, joystickId, 12, true);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeX, joystickId, 1, true, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeY, joystickId, 2, false, false);
      if (plunger_axis != 0)
      { // This can be overriden and assigned to Rz instead of Z axis
         if (m_override_default_buttons && (plunger_axis == 6))
            MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, 6, false, false);
         else
            MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, 3, true, false);
      }
      break;

   case InputLayout::UltraCade:
      SetupJoyMapping(joystickId, InputLayout::Generic);
      MapActionToJoystick(eAddCreditKey, joystickId, 11, true);
      MapActionToJoystick(eAddCreditKey2, joystickId, 12, true);
      MapActionToJoystick(eRightMagnaSave, joystickId, 2, true);
      // Button 3 is not mapped
      // Button 4 is not mapped
      MapActionToJoystick(eVolumeUp, joystickId, 5, true);
      MapActionToJoystick(eVolumeDown, joystickId, 6, true);
      // Button 7 is not mapped
      MapActionToJoystick(eLeftFlipperKey, joystickId, 8, true);
      // Button 9 is not mapped
      MapActionToJoystick(eRightFlipperKey, joystickId, 10, true);
      // Button 11 is not mapped
      MapActionToJoystick(eStartGameKey, joystickId, 12, true);
      MapActionToJoystick(ePlungerKey, joystickId, 13, true);
      MapActionToJoystick(eExitGame, joystickId, 14, true);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeX, joystickId, 2, true, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeY, joystickId, 1, true, false);
      if (plunger_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, 3, false, false);
      break;

   case InputLayout::Sidewinder:
      SetupJoyMapping(joystickId, InputLayout::Generic);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeX, joystickId, 1, lr_axis_reverse, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeY, joystickId, 2, ud_axis_reverse, false);
      if (plunger_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, 7, !plunger_reverse, false);
      break;

   case InputLayout::VirtuaPin:
      SetupJoyMapping(joystickId, InputLayout::Generic);
      MapActionToJoystick(ePlungerKey, joystickId, 0, true);
      MapActionToJoystick(eRightFlipperKey, joystickId, 1, true);
      MapActionToJoystick(eRightMagnaSave, joystickId, 2, true);
      MapActionToJoystick(eVolumeDown, joystickId, 3, true);
      MapActionToJoystick(eVolumeUp, joystickId, 4, true);
      // Button 5 is not mapped
      MapActionToJoystick(eEscape, joystickId, 6, true);
      MapActionToJoystick(eExitGame, joystickId, 7, true);
      MapActionToJoystick(eStartGameKey, joystickId, 8, true);
      MapActionToJoystick(eLeftFlipperKey, joystickId, 9, true);
      MapActionToJoystick(eLeftMagnaSave, joystickId, 10, true);
      MapActionToJoystick(eAddCreditKey, joystickId, 11, true);
      MapActionToJoystick(eAddCreditKey2, joystickId, 12, true);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeX, joystickId, 1, true, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeY, joystickId, 2, false, false);
      if (plunger_axis != 0)
         MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, 3, true, false);
      break;

   case InputLayout::OpenPinDev:
      SetupJoyMapping(joystickId, InputLayout::Generic);

      // OpenPinDev does not conform to the normal axis mapping. It adds itself to the list of axes
      // we hack this by using virtual axis 10..13 for this device. THis is not clean and should be
      // removed in favor of handling these devices like all others.
      m_analogActionMappings.clear();
      if (lr_axis == 9)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeX, joystickId, 10, lr_axis_reverse, false);
      if (ud_axis == 9)
         MapAnalogActionToJoystick(AnalogAction::AM_NudgeY, joystickId, 11, ud_axis_reverse, false);
      if (plunger_axis == 9)
         MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, 12, plunger_reverse, false);
      if (plunger_speed_axis == 9)
         MapAnalogActionToJoystick(AnalogAction::AM_PlungerSpeed, joystickId, 13, false, false);
      break;

   case InputLayout::Generic:
   default:
      {
         const Settings& settings = g_pvp->m_settings;
         MapActionToJoystick(eLeftFlipperKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyLFlipKey"s), true);
         MapActionToJoystick(eRightFlipperKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyRFlipKey"s), true);
         MapActionToJoystick(eStagedLeftFlipperKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyStagedLFlipKey"s), true);
         MapActionToJoystick(eStagedRightFlipperKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyStagedRFlipKey"s), true);
         MapActionToJoystick(eLeftTiltKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyLTiltKey"s), true);
         MapActionToJoystick(eRightTiltKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyRTiltKey"s), true);
         MapActionToJoystick(eCenterTiltKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyCTiltKey"s), true);
         MapActionToJoystick(ePlungerKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyPlungerKey"s), true);
         MapActionToJoystick(eFrameCount, joystickId, settings.LoadValueInt(Settings::Player, "JoyFrameCount"s), true);
         MapActionToJoystick(eDBGBalls, joystickId, settings.LoadValueInt(Settings::Player, "JoyDebugKey"s), true);
         MapActionToJoystick(eDebugger, joystickId, settings.LoadValueInt(Settings::Player, "JoyDebuggerKey"s), true);
         MapActionToJoystick(eAddCreditKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyAddCreditKey"s), true);
         MapActionToJoystick(eAddCreditKey2, joystickId, settings.LoadValueInt(Settings::Player, "JoyAddCredit2Key"s), true);
         MapActionToJoystick(eStartGameKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyStartGameKey"s), true);
         MapActionToJoystick(eMechanicalTilt, joystickId, settings.LoadValueInt(Settings::Player, "JoyMechTiltKey"s), true);
         MapActionToJoystick(eRightMagnaSave, joystickId, settings.LoadValueInt(Settings::Player, "JoyRMagnaSave"s), true);
         MapActionToJoystick(eLeftMagnaSave, joystickId, settings.LoadValueInt(Settings::Player, "JoyLMagnaSave"s), true);
         MapActionToJoystick(eExitGame, joystickId, settings.LoadValueInt(Settings::Player, "JoyExitGameKey"s), true);
         MapActionToJoystick(eVolumeUp, joystickId, settings.LoadValueInt(Settings::Player, "JoyVolumeUp"s), true);
         MapActionToJoystick(eVolumeDown, joystickId, settings.LoadValueInt(Settings::Player, "JoyVolumeDown"s), true);
         MapActionToJoystick(eLockbarKey, joystickId, settings.LoadValueInt(Settings::Player, "JoyLockbarKey"s), true);
         // eEnable3D (no joystick mapping)
         MapActionToJoystick(eTableRecenter, joystickId, settings.LoadValueInt(Settings::Player, "JoyTableRecenterKey"s), true);
         MapActionToJoystick(eTableUp, joystickId, settings.LoadValueInt(Settings::Player, "JoyTableUpKey"s), true);
         MapActionToJoystick(eTableDown, joystickId, settings.LoadValueInt(Settings::Player, "JoyTableDownKey"s), true);
         // eEscape (no joystick mapping)
         MapActionToJoystick(ePause, joystickId, settings.LoadValueInt(Settings::Player, "JoyPauseKey"s), true);
         MapActionToJoystick(eTweak, joystickId, settings.LoadValueInt(Settings::Player, "JoyTweakKey"s), true);

         // TODO map to corresponding GenericKey (or define actions for these keys)
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMBuyIn"s, 0), true); 2
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMCoin3"s, 0), true); 5
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMCoin4"s, 0), true); 6
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMCoinDoor"s, 0), true); END
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMCancel"s, 0), true); 7
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMDown"s, 0), true); 8
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMUp"s, 0), true); 9
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyPMEnter"s, 0), true); 0

         // TODO map to corresponding GenericKey
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom1"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom1Key"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom2"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom2Key"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom3"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom3Key"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom4"s, 0), true);
         // MapActionToJoystick(, joystickId, settings.LoadValueWithDefault(Settings::Player, "JoyCustom4Key"s, 0), true);

         if (lr_axis != 0)
            MapAnalogActionToJoystick(AnalogAction::AM_NudgeX, joystickId, lr_axis, lr_axis_reverse, false);
         if (ud_axis != 0)
            MapAnalogActionToJoystick(AnalogAction::AM_NudgeY, joystickId, ud_axis, ud_axis_reverse, false);
         if (plunger_axis != 0)
            MapAnalogActionToJoystick(AnalogAction::AM_PlungerPos, joystickId, plunger_axis, plunger_reverse, false);
         if (plunger_speed_axis != 0)
            MapAnalogActionToJoystick(AnalogAction::AM_PlungerSpeed, joystickId, plunger_speed_axis, false, false);
      }
      break;
   }
}

void PinInput::ProcessInput()
{
   if (!g_pplayer || !g_pplayer->m_ptable) return; // only if player is running
   g_pplayer->m_logicProfiler.OnProcessInput();

   const U32 now = msec();

   // Gather input from all handlers
   for (const auto& handler : m_inputHandlers)
      handler->Update();

   // Wipe key state if we're not the foreground window as we miss key-up events
   #ifdef _WIN32
   if (m_focusHWnd != GetForegroundWindow())
      memset(&m_inputState, 0, sizeof(m_inputState));
   #endif

   // Handle automatic start
   if (g_pplayer->m_ptable->m_tblAutoStartEnabled)
      Autostart(g_pplayer->m_ptable->m_tblAutoStart, g_pplayer->m_ptable->m_tblAutoStartRetry);
   if (m_autoStartTimestamp == 0) // Check if we've been initialized.
      m_autoStartTimestamp = now;

   // Handle exit on long press
   if ((g_pplayer->m_time_msec > 1000) // Game has been played at least 1 second
      && (m_exitPressTimestamp != 0) // Exit button is pressed
      && ((now - m_exitPressTimestamp) > m_exitAppPressLengthMs)) // Exit button has been pressed continuously long enough
      g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);

   // Global Backglass/Playfield sound volume
   if ((now - m_nextKeyPressedTime) > 75)
   {
      static unsigned int lastVolumeNotifId = 0;
      m_nextKeyPressedTime = now;
      if (m_inputState.IsKeyDown(eVolumeDown))
      {
         g_pplayer->m_MusicVolume = clamp(g_pplayer->m_MusicVolume - 1, 0, 100);
         g_pplayer->m_SoundVolume = clamp(g_pplayer->m_SoundVolume - 1, 0, 100);
         g_pplayer->UpdateVolume();
         lastVolumeNotifId = g_pplayer->m_liveUI->PushNotification("Volume: " + std::to_string(g_pplayer->m_MusicVolume) + '%', 500, lastVolumeNotifId);
      }
      else if (m_inputState.IsKeyDown(eVolumeUp))
      {
         g_pplayer->m_MusicVolume = clamp(g_pplayer->m_MusicVolume + 1, 0, 100);
         g_pplayer->m_SoundVolume = clamp(g_pplayer->m_SoundVolume + 1, 0, 100);
         g_pplayer->UpdateVolume();
         lastVolumeNotifId = g_pplayer->m_liveUI->PushNotification("Volume: " + std::to_string(g_pplayer->m_MusicVolume) + '%', 500, lastVolumeNotifId);
      }
   }
}

void PinInput::ProcessEvent(const InputEvent& event)
{
   if (event.type == InputEvent::Type::Mouse && !g_pplayer->m_liveUI->HasMouseCapture() && !g_pplayer->m_throwBalls && !g_pplayer->m_ballControl)
   {
      const auto& it = std::ranges::find_if(m_actionMappings.begin(), m_actionMappings.end(),
         [&event](const ActionMapping& mapping) { return (mapping.type == ActionMapping::AM_Mouse) && (mapping.buttonId == event.buttonId); });
      if (it != m_actionMappings.end())
         FireActionEvent(it->action, event.isPressed);
   }
   else if (event.type == InputEvent::Type::Keyboard)
   {
      const auto& it = std::ranges::find_if(m_actionMappings.begin(), m_actionMappings.end(),
         [&event](const ActionMapping& mapping) { return (mapping.type == ActionMapping::AM_Keyboard) && (mapping.keycode == event.keycode); });
      if (it != m_actionMappings.end())
         FireActionEvent(it->action, event.isPressed);
      else if (!g_pplayer->m_liveUI->HasKeyboardCapture())
         FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, event.keycode);
   }
   else if (event.type == InputEvent::Type::Action)
   {
      FireActionEvent(event.action, event.isPressed);
   }
   else if (event.type == InputEvent::Type::JoyButton)
   {
      const auto& it = std::ranges::find_if(m_actionMappings.begin(), m_actionMappings.end(),
         [&event](const ActionMapping& mapping) { return (mapping.type == ActionMapping::AM_Joystick) && (mapping.joystickId == event.joystickId) && (mapping.buttonId == event.buttonId); });
      if (it != m_actionMappings.end())
         FireActionEvent(it->action, event.isPressed);
      else
      {
         if (m_joycustom1 == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom1key);
         else if (m_joycustom2 == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom2key);
         else if (m_joycustom3 == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom3key);
         else if (m_joycustom4 == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom4key);
         else if (m_joypmbuyin == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_2);
         else if (m_joypmcoin3 == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_5);
         else if (m_joypmcoin4 == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_6);
         else if (m_joypmcoindoor == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_END);
         else if (m_joypmcancel == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_7);
         else if (m_joypmdown == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_8);
         else if (m_joypmup == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_9);
         else if (m_joypmenter == event.buttonId)
            FireGenericKeyEvent(event.isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_0);
      }
   }
   else if (event.type == InputEvent::Type::JoyAxis)
   {
      const auto& it = std::ranges::find_if(m_analogActionMappings.begin(), m_analogActionMappings.end(),
         [&event](const AnalogActionMapping& mapping) { return (mapping.joystickId == event.joystickId) && (mapping.axisId == event.axisId); });
      if (it != m_analogActionMappings.end())
      {
         auto newValue = static_cast<float>(it->revert ? -event.value : event.value);
         switch (it->output)
         {
         case AnalogAction::AM_NudgeX:
         case AnalogAction::AM_NudgeY:
            if (newValue < -m_deadz)
               newValue = (newValue + static_cast<float>(m_deadz)) * static_cast<float>(JOYRANGEMX) / static_cast<float>(JOYRANGEMX - m_deadz);
            else if (newValue > m_deadz)
               newValue = (newValue - static_cast<float>(m_deadz)) * static_cast<float>(JOYRANGEMX) / static_cast<float>(JOYRANGEMX - m_deadz);
            else
               newValue = 0.f;
            m_accelerometerDirty = (newValue != it->value);
            break;

         case AnalogAction::AM_PlungerSpeed:
            m_plungerSpeedDirty = (newValue != it->value);
            break;

         case AnalogAction::AM_PlungerPos:
            m_plungerPosDirty = (newValue != it->value);
            break;
         }
         it->value = newValue;
      }
   }
}
