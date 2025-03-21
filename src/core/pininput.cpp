// license:GPLv3+

#include "core/stdafx.h"
#include "renderer/VRDevice.h"

#ifdef ENABLE_SDL_VIDEO
#include "imgui/imgui_impl_sdl3.h"
#endif

#ifdef __LIBVPINBALL__
#include "standalone/VPinballLib.h"
#endif

// from dinput.h, modernized to please clang
#undef DIJOFS_X
#undef DIJOFS_Y
#undef DIJOFS_Z
#undef DIJOFS_RX
#undef DIJOFS_RY
#undef DIJOFS_RZ
#undef DIJOFS_SLIDER
#undef DIJOFS_POV
#undef DIJOFS_BUTTON
#define DIJOFS_X            offsetof(DIJOYSTATE, lX)
#define DIJOFS_Y            offsetof(DIJOYSTATE, lY)
#define DIJOFS_Z            offsetof(DIJOYSTATE, lZ)
#define DIJOFS_RX           offsetof(DIJOYSTATE, lRx)
#define DIJOFS_RY           offsetof(DIJOYSTATE, lRy)
#define DIJOFS_RZ           offsetof(DIJOYSTATE, lRz)
#define DIJOFS_SLIDER(n)   (offsetof(DIJOYSTATE, rglSlider) + (n) * sizeof(LONG))
#define DIJOFS_POV(n)      (offsetof(DIJOYSTATE, rgdwPOV) + (n) * sizeof(DWORD))
#define DIJOFS_BUTTON(n)   (offsetof(DIJOYSTATE, rgbButtons) + (n))
// end


// Input type IDs
enum
{
   APP_KEYBOARD,
   APP_MOUSE,
   APP_ACTION, // for touchscreen, directly mapped to corresponding binary action
   APP_JOYSTICK0, // First joystick, with a maximum of PININ_JOYMXCNT joysticks
};

#define INPUT_BUFFER_SIZE MAX_KEYQUEUE_SIZE
#define BALLCONTROL_DOUBLECLICK_THRESHOLD_USEC (500 * 1000)


PinInput::PinInput()
   : m_first_stamp(msec())
   #ifdef ENABLE_SDL_INPUT
      , m_joypmcancel(SDL_GAMEPAD_BUTTON_NORTH + 1)
   #endif
{
}

PinInput::~PinInput()
{
   #ifdef ENABLE_SDL_INPUT
      if (m_pSDLGamePad)
         SDL_CloseGamepad(m_pSDLGamePad);
      if (m_pSDLRumbleDevice)
         SDL_CloseHaptic(m_pSDLRumbleDevice);
      if (m_pSDLJoystick)
         SDL_CloseJoystick(m_pSDLJoystick); 
   #endif
   TerminateOpenPinballDevices();
}

void PinInput::UnmapAllJoy()
{
   std::erase_if(m_actionMappings, [](ActionMapping am) {
      return (am.type == ActionMapping::AM_Joystick); });
   m_analogActionMappings.clear();
}

void PinInput::UnmapAction(EnumAssignKeys action, bool fromKeyboard, bool fromJoystick)
{
   std::erase_if(m_actionMappings, [action, fromKeyboard, fromJoystick](ActionMapping am) {
      return (am.action == action)
         && ((fromKeyboard && (am.type == ActionMapping::AM_Keyboard)) || (fromJoystick && (am.type == ActionMapping::AM_Joystick)));
      });
}

void PinInput::MapActionToKeyboard(EnumAssignKeys action, int keycode, bool replace)
{
   if (replace)
      UnmapAction(action, true, false);
   ActionMapping mapping;
   mapping.action = action;
   mapping.type = ActionMapping::AM_Keyboard;
   mapping.keycode = keycode;
   m_actionMappings.push_back(mapping);
}

void PinInput::MapActionToJoystick(EnumAssignKeys action, int joystickId, int buttonId, bool replace)
{
   if (replace)
      UnmapAction(action, false, true);
   ActionMapping mapping;
   mapping.action = action;
   mapping.type = ActionMapping::AM_Joystick;
   mapping.joystickId = joystickId;
   mapping.buttonId = buttonId;
   m_actionMappings.push_back(mapping);
}

void PinInput::MapAnalogActionToJoystick(AnalogActionMapping::AMOutput output, int joystickId, int axisId, bool revert, bool replace)
{
   if (replace)
      std::erase_if(m_analogActionMappings, [output](AnalogActionMapping am) { return (am.output == output); });
   AnalogActionMapping mapping;
   mapping.joystickId = joystickId;
   mapping.axisId = axisId;
   mapping.revert = revert;
   mapping.output = output;
   m_analogActionMappings.push_back(mapping);
}

#ifdef _WIN32 // DirectInput

// Callback function for enumerating objects (axes, buttons, POVs) on a 
// joystick. This function enables user interface elements for objects
// that are found to exist, and scales axes min/max values.
BOOL CALLBACK PinInput::EnumObjectsCallbackDI(const DIDEVICEOBJECTINSTANCE* pdidoi,
   VOID* pContext)
{
   const PinInput * const ppinput = (PinInput *)pContext;

#ifdef _DEBUG
   static int nAxis = 0;
   static int nButtons = 0;
   static int nSliderCount = 0; // Number of returned slider controls
   static int nPOVCount = 0;    // Number of returned POV controls
   static int nKey = 0;
   static int nUnknown = 0;
#endif

   // For axes that are returned, set the DIPROP_RANGE property for the
   // enumerated axis in order to scale min/max values.
   if (pdidoi->dwType & DIDFT_AXIS)
   {
      DIPROPRANGE diprg;
      diprg.diph.dwSize = sizeof(DIPROPRANGE);
      diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
      diprg.diph.dwHow = DIPH_BYID;
      diprg.lMin = JOYRANGEMN;
      diprg.lMax = JOYRANGEMX;

      // Set the range for the axis
      if (FAILED(ppinput->m_pJoystick[ppinput->m_num_joy]->SetProperty(DIPROP_RANGE, &diprg.diph)))
         return DIENUM_STOP;

      // set DEADBAND to Zero
      DIPROPDWORD dipdw;
      dipdw.diph.dwSize = sizeof(DIPROPDWORD);
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdw.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
      dipdw.diph.dwHow = DIPH_BYID;
      dipdw.dwData = 0; // no dead zone at joystick level

      // Set the deadzone
      if (FAILED(ppinput->m_pJoystick[ppinput->m_num_joy]->SetProperty(DIPROP_DEADZONE, &dipdw.diph)))
         return DIENUM_STOP;
   }

#ifdef _DEBUG
   if (pdidoi->guidType == GUID_XAxis)			{ ++nAxis; }
   else if (pdidoi->guidType == GUID_YAxis)		{ ++nAxis; }
   else if (pdidoi->guidType == GUID_ZAxis)		{ ++nAxis; }
   else if (pdidoi->guidType == GUID_RxAxis)	{ ++nAxis; }
   else if (pdidoi->guidType == GUID_RyAxis)	{ ++nAxis; }
   else if (pdidoi->guidType == GUID_RzAxis)	{ ++nAxis; }
   else if (pdidoi->guidType == GUID_Slider)	{ ++nSliderCount; }
   else if (pdidoi->guidType == GUID_Button)	{ ++nButtons; }
   else if (pdidoi->guidType == GUID_Key)		{ ++nKey; }
   else if (pdidoi->guidType == GUID_POV)		{ ++nPOVCount; }
   else if (pdidoi->guidType == GUID_Unknown)	{ ++nUnknown; }
#endif

   return DIENUM_CONTINUE;
}

// Callback for enumerating joysticks (gamepads)
BOOL CALLBACK PinInput::EnumJoystickCallbackDI(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
   DIPROPSTRING dstr;
   dstr.diph.dwSize = sizeof(DIPROPSTRING);
   dstr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
   dstr.diph.dwObj = 0;
   dstr.diph.dwHow = DIPH_DEVICE;

   PinInput * const ppinput = (PinInput *)pvRef;

   HRESULT hr;

   hr = ppinput->m_pDI->CreateDevice(lpddi->guidInstance, &ppinput->m_pJoystick[ppinput->m_num_joy], nullptr);
   if (FAILED(hr))
   {
      ppinput->m_pJoystick[ppinput->m_num_joy] = nullptr; // make sure no garbage
      return DIENUM_CONTINUE;                             // try for another joystick
   }

   int joystickType = USHOCKTYPE_GENERIC;
   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->GetProperty(DIPROP_PRODUCTNAME, &dstr.diph);
   if (hr == S_OK)
   {
      if (!WzSzStrCmp(dstr.wsz, "PinballWizard"))
         joystickType = USHOCKTYPE_PBWIZARD;
      else if (!WzSzStrCmp(dstr.wsz, "UltraCade Pinball"))
         joystickType = USHOCKTYPE_ULTRACADE;
      else if (!WzSzStrCmp(dstr.wsz, "Microsoft SideWinder Freestyle Pro (USB)"))
         joystickType = USHOCKTYPE_SIDEWINDER;
      else if (!WzSzStrCmp(dstr.wsz, "VirtuaPin Controller"))
         joystickType = USHOCKTYPE_VIRTUAPIN;
      else if (!WzSzStrCmp(dstr.wsz, "Pinscape Controller") || !WzSzStrCmp(dstr.wsz, "PinscapePico"))
      {
         joystickType = USHOCKTYPE_GENERIC;
         // FIXME rewrite as linear plunger is ill designed (declared here, for all inputs and not for one, used in player)
         ppinput->m_linearPlunger = true; // use linear plunger calibration
      }
      PLOGI << "Joystick detected: " << dstr.wsz << ", using input mode #" << joystickType;
   }
   ppinput->uShockType = joystickType;
   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->SetDataFormat(&c_dfDIJoystick);

   // joystick input foreground or background focus
   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->SetCooperativeLevel(ppinput->m_focusHWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

   DIPROPDWORD dipdw;
   dipdw.diph.dwSize = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
   dipdw.diph.dwObj = 0;
   dipdw.diph.dwHow = DIPH_DEVICE;
   dipdw.dwData = INPUT_BUFFER_SIZE;

   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

   // Enumerate the joystick objects. The callback function enabled user
   // interface elements for objects that are found, and sets the min/max
   // values property for discovered axes.
   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->EnumObjects(EnumObjectsCallbackDI, (VOID*)pvRef, DIDFT_ALL);

   ppinput->SetupJoyMapping(ppinput->m_num_joy, joystickType);

   if (++(ppinput->m_num_joy) < PININ_JOYMXCNT)
       return DIENUM_CONTINUE;
   else
       return DIENUM_STOP; //allocation for only PININ_JOYMXCNT joysticks, ignore any others
}
#endif


#ifdef ENABLE_SDL_INPUT
void PinInput::RefreshSDLDevices()
{
   // Close any previously opened devices
   if (m_pSDLGamePad) {
      SDL_CloseGamepad(m_pSDLGamePad);
      m_pSDLGamePad = nullptr;
   }
   if (m_pSDLRumbleDevice) {
      SDL_CloseHaptic(m_pSDLRumbleDevice);
      m_pSDLRumbleDevice = nullptr;
   }
   if (m_pSDLJoystick) {
      SDL_CloseJoystick(m_pSDLJoystick);
      m_pSDLJoystick = nullptr;
   }
   m_num_joy = 0;
   UnmapAllJoy();

   // Get list of all connected devices
   int joystick_count = 0;
   SDL_JoystickID* joystick_ids = SDL_GetJoysticks(&joystick_count);

   if (joystick_count > 0) {
      for (int idx = 0; idx < joystick_count; ++idx) {
         // Log the device we found
         PLOGI.printf("Input device found: id=%d name=%s",
            joystick_ids[idx],
            SDL_GetJoystickNameForID(joystick_ids[idx]));

         // Process each device as either a gamepad or joystick
         if (SDL_IsGamepad(joystick_ids[idx])) {
            // Skip Apple Remote on iOS/tvOS devices
            #if defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))
            if (!lstrcmpi(SDL_GetGamepadNameForID(joystick_ids[idx]), "Remote")) continue;
            #endif

            // Try to open as gamepad
            m_pSDLGamePad = SDL_OpenGamepad(joystick_ids[idx]);
            if (m_pSDLGamePad) {
               SetupJoyMapping(m_num_joy, USHOCKTYPE_GENERIC);
               m_num_joy++;
               SDL_PropertiesID props = SDL_GetGamepadProperties(m_pSDLGamePad);
               PLOGI.printf("Processing as Gamepad: %d axes, %d buttons, rumble=%s",
                  5,  // Standard gamepad has 6 axes
                  15, // Standard gamepad has 15 buttons
                  SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false) ? "true" : "false");
            }
         } else {
            // Try to open as standard joystick
            m_pSDLJoystick = SDL_OpenJoystick(joystick_ids[idx]);
            if (m_pSDLJoystick) {
               SetupJoyMapping(m_num_joy, USHOCKTYPE_GENERIC);
               m_num_joy++;

               // Check if joystick supports force feedback
               if (SDL_IsJoystickHaptic(m_pSDLJoystick)) {
                  m_pSDLRumbleDevice = SDL_OpenHapticFromJoystick(m_pSDLJoystick);
                  if (!SDL_InitHapticRumble(m_pSDLRumbleDevice)) {
                     ShowError(SDL_GetError());
                     SDL_CloseHaptic(m_pSDLRumbleDevice);
                     m_pSDLRumbleDevice = nullptr;
                  }
               }
               PLOGI.printf("Processing as Joystick: %d axes, %d buttons, rumble=%s",
                  SDL_GetNumJoystickAxes(m_pSDLJoystick),
                  SDL_GetNumJoystickButtons(m_pSDLJoystick),
                  m_pSDLRumbleDevice ? "true" : "false");
            }
         }
      }
   }

   // No devices were successfully opened
   if (m_num_joy == 0) {
      PLOGI.printf("No game controllers or joysticks connected!");
   }
}
#endif

void PinInput::PushActionEvent(EnumAssignKeys action, bool isPressed)
{
   DIDEVICEOBJECTDATA didod;
   didod.dwOfs = action;
   didod.dwData = isPressed ? 0x80 : 0x00;
   PushQueue(&didod, APP_ACTION);
}

void PinInput::PushKeyboardEvent(int keycode, bool isPressed)
{
   DIDEVICEOBJECTDATA didod;
   didod.dwOfs = keycode;
   didod.dwData = isPressed ? 0x80 : 0x00;
   PushQueue(&didod, APP_KEYBOARD);
}

void PinInput::PushJoystickButtonEvent(int joystickId, int buttonId, bool isPressed)
{
   DIDEVICEOBJECTDATA didod;
   didod.dwOfs = DIJOFS_BUTTON0 + static_cast<DWORD>(buttonId);
   didod.dwData = isPressed ? 0x80 : 0x00;
   PushQueue(&didod, APP_JOYSTICK0 + joystickId);
}

void PinInput::PushJoystickAxisEvent(int joystickId, int axisId, int value)
{
   DIDEVICEOBJECTDATA didod;
   didod.dwOfs = axisId;
   didod.dwData = value;
   PushQueue(&didod, APP_JOYSTICK0 + joystickId);
}

void PinInput::PushQueue(DIDEVICEOBJECTDATA * const data, const unsigned int app_data)
{
   if ((!data) || (((m_head + 1) % MAX_KEYQUEUE_SIZE) == m_tail)) // queue full?
       return;
   m_diq[m_head] = *data;
   m_diq[m_head].dwSequence = app_data;
   m_head = (m_head + 1) % MAX_KEYQUEUE_SIZE; // advance head of queue
}

const DIDEVICEOBJECTDATA *PinInput::GetTail()
{
   if (m_head == m_tail)
      return nullptr; // queue empty?
   const DIDEVICEOBJECTDATA * const ptr = &m_diq[m_tail];
   m_tail = (m_tail + 1) % MAX_KEYQUEUE_SIZE; // advance tail of queue
   return ptr;
}

void PinInput::GetInputDeviceData()
{
   #if defined(_WIN32) && defined(USE_DINPUT_FOR_KEYBOARD)
      // keyboard
      #ifdef USE_DINPUT8
         const LPDIRECTINPUTDEVICE8 pkyb = m_pKeyboard;
      #else
         const LPDIRECTINPUTDEVICE pkyb = m_pKeyboard;
      #endif
      if (pkyb)
      {
         HRESULT hr = pkyb->Acquire(); // try to acquire keyboard input
         if (hr == S_OK || hr == S_FALSE)
         {
            DWORD dwElements = INPUT_BUFFER_SIZE;
            DIDEVICEOBJECTDATA didod[INPUT_BUFFER_SIZE];
            hr = pkyb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);
            if ((m_focusHWnd == GetForegroundWindow()) && (hr == S_OK || hr == DI_BUFFEROVERFLOW))
            {
               for (DWORD i = 0; i < dwElements; i++)
                  PushKeyboardEvent(didod[i].dwOfs, (didod[i].dwData & 0x80) != 0);
            }
         }
      }
   #else
      // cache to avoid double key triggers
      static bool oldKeyStates[eCKeys] = { false };

      #ifdef _WIN32
         for (unsigned int i = 0; i < eCKeys; ++i)
         {
            const unsigned int rgk = (unsigned int)g_pplayer->m_rgKeys[i];
            const unsigned int vk = get_vk(rgk);
            if (vk == ~0u)
               continue;

            const SHORT keyState = GetAsyncKeyState(vk);
            const bool keyDown = !!((1 << 16) & keyState);

            if (oldKeyStates[i] == keyDown)
               continue;
            oldKeyStates[i] = keyDown;

            PushActionEvent(static_cast<EnumAssignKeys>(i), keyDown);
         }
      #endif
   #endif

   #ifdef _WIN32
      // mouse
      if (m_pMouse && m_enableMouseInPlayer)
      {
         HRESULT hr = m_pMouse->Acquire(); // try to acquire mouse input
         if (hr == S_OK || hr == S_FALSE)
         {
            DIMOUSESTATE2 mouseState;
            hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState);
            if ((hr == S_OK || hr == DI_BUFFEROVERFLOW) && (m_focusHWnd == GetForegroundWindow()))
            {
               for (DWORD i = 0; i < 3; i++)
               {
                  if (m_oldMouseButtonState[i] != mouseState.rgbButtons[i])
                  {
                     DIDEVICEOBJECTDATA didod;
                     didod.dwData = mouseState.rgbButtons[i];
                     didod.dwOfs = i + 1;
                     PushQueue(&didod, APP_MOUSE);
                     m_oldMouseButtonState[i] = mouseState.rgbButtons[i];
                  }
               }
            }
         }
      }
   #endif

   // same for joysticks 
   switch (m_inputApi) {
   case PI_XINPUT:      HandleInputXI(); break;
   case PI_SDL:         HandleInputSDL(); break;
   case PI_DIRECTINPUT: HandleInputDI(); break;
   }
}

void PinInput::HandleInputDI()
{
   #ifdef _WIN32
      DIDEVICEOBJECTDATA didod[INPUT_BUFFER_SIZE];

      for (int k = 0; k < m_num_joy; ++k)
      {
         #ifdef USE_DINPUT8
            const LPDIRECTINPUTDEVICE8 pjoy = m_pJoystick[k];
         #else
            const LPDIRECTINPUTDEVICE pjoy = m_pJoystick[k];
         #endif

         if (pjoy)
         {
            // Ignore any disabled input devices.
            // Unfortunately, there's no guarantee that the order of the devices in settings is the same as the currently enumerated ones.
            // Therefore we can't simply use the joystick index here; that's why we saved the device setting into a map and the device info
            // into a new array.
            bool inputDeviceState = true;
            const auto deviceIt = m_inputDeviceSettingsInfo.find(m_attachedDeviceInfo[k]->tszInstanceName);
            if (deviceIt != m_inputDeviceSettingsInfo.end())
            {
               inputDeviceState = deviceIt->second;
            }

            // Bail out here if the device is disabled
            if (!inputDeviceState) continue;

            HRESULT hr = pjoy->Acquire(); // try to acquire joystick input
            if (hr == S_OK || hr == S_FALSE)
            {
               DWORD dwElements = INPUT_BUFFER_SIZE;
               hr = pjoy->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);
               if ((hr == S_OK || hr == DI_BUFFEROVERFLOW) && (m_focusHWnd == GetForegroundWindow()))
               {
                  for (DWORD i = 0; i < dwElements; i++)
                  {
                     if ((didod[i].dwOfs >= DIJOFS_BUTTON0) && (didod[i].dwOfs <= DIJOFS_BUTTON31))
                        PushJoystickButtonEvent(k, didod[i].dwOfs - DIJOFS_BUTTON0, (didod[i].dwData & 0x80) != 0);
                     else
                     {
                        switch (didod[i].dwOfs)
                        {
                        case DIJOFS_X: PushJoystickAxisEvent(k, 1, didod[i].dwData); break;
                        case DIJOFS_Y: PushJoystickAxisEvent(k, 2, didod[i].dwData); break;
                        case DIJOFS_Z: PushJoystickAxisEvent(k, 3, didod[i].dwData); break;
                        case DIJOFS_RX: PushJoystickAxisEvent(k, 4, didod[i].dwData); break;
                        case DIJOFS_RY: PushJoystickAxisEvent(k, 5, didod[i].dwData); break;
                        case DIJOFS_RZ: PushJoystickAxisEvent(k, 6, didod[i].dwData); break;
                        case DIJOFS_SLIDER(0): PushJoystickAxisEvent(k, 7, didod[i].dwData); break;
                        case DIJOFS_SLIDER(1): PushJoystickAxisEvent(k, 8, didod[i].dwData); break;
                        }
                     }
                  }
               }
            }
         }
      }
   #endif
}

void PinInput::HandleInputXI()
{
#ifdef ENABLE_XINPUT
   typedef struct {
      WORD xi; DWORD di;
   } mappingElement;
   static constexpr mappingElement mappingTable[] = {
      {XINPUT_GAMEPAD_A, DIJOFS_BUTTON0 },
      {XINPUT_GAMEPAD_B, DIJOFS_BUTTON1 },
      {XINPUT_GAMEPAD_X, DIJOFS_BUTTON2 },
      {XINPUT_GAMEPAD_Y, DIJOFS_BUTTON3 },
      {XINPUT_GAMEPAD_LEFT_SHOULDER, DIJOFS_BUTTON4 },
      {XINPUT_GAMEPAD_RIGHT_SHOULDER, DIJOFS_BUTTON5 },
      {XINPUT_GAMEPAD_BACK, DIJOFS_BUTTON6},
      {XINPUT_GAMEPAD_START, DIJOFS_BUTTON7},
      {XINPUT_GAMEPAD_LEFT_THUMB, DIJOFS_BUTTON8},
      {XINPUT_GAMEPAD_RIGHT_THUMB, DIJOFS_BUTTON9},
      {XINPUT_GAMEPAD_DPAD_LEFT, DIJOFS_BUTTON10},
      {XINPUT_GAMEPAD_DPAD_RIGHT, DIJOFS_BUTTON11},
      {XINPUT_GAMEPAD_DPAD_UP, DIJOFS_BUTTON12},
      {XINPUT_GAMEPAD_DPAD_DOWN, DIJOFS_BUTTON13},
      {0, 0} };
   XINPUT_STATE state = {};
   unsigned int xie = ERROR_DEVICE_NOT_CONNECTED;
   if (m_inputDeviceXI != -2)
   {
      if (m_inputDeviceXI != -1)
      {
         xie = XInputGetState(m_inputDeviceXI, &state);
         if (xie != ERROR_SUCCESS)
            m_inputDeviceXI = -1;
      }
      if (m_inputDeviceXI == -1)
      {
         m_num_joy = 0;
         for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
         {
            ZeroMemory(&state, sizeof(XINPUT_STATE));
            if ((xie = XInputGetState(i, &state)) == ERROR_SUCCESS)
            {
               m_inputDeviceXI = i;
               m_num_joy = 1;
               UnmapAllJoy();
               SetupJoyMapping(0, USHOCKTYPE_GENERIC);
               break;
            }
         }
      }
      if (xie == ERROR_DEVICE_NOT_CONNECTED)
      { // XInputGetState can cause quite some overhead, especially if no devices connected! Thus disable the polling if nothing connected
         m_inputDeviceXI = -2;
         m_num_joy = 0;
      }
   }
   if (m_rumbleRunning && m_inputDeviceXI >= 0) {
      const DWORD now = msec();
      if (m_rumbleOffTime <= now || m_rumbleOffTime - now > 65535) {
         m_rumbleRunning = false;
         XINPUT_VIBRATION vibration = {};
         XInputSetState(m_inputDeviceXI, &vibration);
      }
   }
   int i = 0;
   while (mappingTable[i].xi != 0) {
      if ((m_inputDeviceXIstate.Gamepad.wButtons & mappingTable[i].xi) != (state.Gamepad.wButtons & mappingTable[i].xi))
         PushJoystickButtonEvent(0, mappingTable[i].di - DIJOFS_BUTTON0, (state.Gamepad.wButtons & mappingTable[i].xi) > 0);
      i++;
   }
   if (m_inputDeviceXIstate.Gamepad.sThumbLX != state.Gamepad.sThumbLX)
      PushJoystickAxisEvent(0, 1, state.Gamepad.sThumbLX * -2); // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   if (m_inputDeviceXIstate.Gamepad.sThumbLY != state.Gamepad.sThumbLY)
      PushJoystickAxisEvent(0, 2, state.Gamepad.sThumbLY * -2); // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   if (m_inputDeviceXIstate.Gamepad.bLeftTrigger != state.Gamepad.bLeftTrigger)
      PushJoystickAxisEvent(0, 3, state.Gamepad.bLeftTrigger * 512); // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   if (m_inputDeviceXIstate.Gamepad.sThumbRX != state.Gamepad.sThumbRX)
      PushJoystickAxisEvent(0, 4, state.Gamepad.sThumbRX * -2); // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   if (m_inputDeviceXIstate.Gamepad.sThumbRY != state.Gamepad.sThumbRY)
      PushJoystickAxisEvent(0, 5, state.Gamepad.sThumbRY * -2); // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   if (m_inputDeviceXIstate.Gamepad.bRightTrigger != state.Gamepad.bRightTrigger)
      PushJoystickAxisEvent(0, 6, state.Gamepad.bRightTrigger * 512); // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   memcpy(&m_inputDeviceXIstate, &state, sizeof(XINPUT_STATE));
#endif
}

#if defined(ENABLE_SDL_INPUT)
void PinInput::HandleSDLEvent(SDL_Event &e)
{
   assert(m_inputApi == PI_SDL);
   static constexpr DWORD axes[] = { 1, 2, 3, 4, 5, 6 };
   static constexpr int axisMultiplier[] = { 2, 2, 2, 2, 256, 256 }; // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
   switch (e.type) 
   {
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
         if (e.key.repeat == 0) {
            const unsigned int dik = get_dik_from_sdlk(e.key.key);
            if (dik != ~0u)
               PushKeyboardEvent(dik, e.type == SDL_EVENT_KEY_DOWN);
         }
         break;

      #if (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS)) || defined(__ANDROID__)
      case SDL_EVENT_FINGER_DOWN:
      case SDL_EVENT_FINGER_UP:
         {
            POINT point;
            point.x = (int)((float)g_pplayer->m_playfieldWnd->GetWidth() * e.tfinger.x);
            point.y = (int)((float)g_pplayer->m_playfieldWnd->GetHeight() * e.tfinger.y);
            for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
            {
               if ((g_pplayer->m_touchregion_pressed[i] != (e.type == SDL_EVENT_FINGER_DOWN))
                  && Intersect(touchregion[i], g_pplayer->m_playfieldWnd->GetWidth(), g_pplayer->m_playfieldWnd->GetHeight(), point,
                     fmodf(g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].mViewportRotation, 360.0f) != 0.f))
               {
                  g_pplayer->m_touchregion_pressed[i] = (e.type == SDL_EVENT_FINGER_DOWN);
                  PushActionEvent(touchActionMap[i], g_pplayer->m_touchregion_pressed[i]);
               }
            }
         }
         break;
      #endif

      case SDL_EVENT_GAMEPAD_ADDED:
      case SDL_EVENT_GAMEPAD_REMOVED:
         RefreshSDLDevices();
         break;
      case SDL_EVENT_GAMEPAD_AXIS_MOTION:
         if (e.gaxis.axis < 6)
            PushJoystickAxisEvent(0, axes[e.gaxis.axis], e.gaxis.value * axisMultiplier[e.gaxis.axis]); // Index 0 for gamepad
         break;
      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      case SDL_EVENT_GAMEPAD_BUTTON_UP:
         if (e.gbutton.button < 32)
            PushJoystickButtonEvent(0, e.gbutton.button, e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN); // Index 0 for GamePad
         break;
      case SDL_EVENT_JOYSTICK_ADDED:
      case SDL_EVENT_JOYSTICK_REMOVED:
         RefreshSDLDevices();
         break;
      case SDL_EVENT_JOYSTICK_AXIS_MOTION:
         if (e.jaxis.axis < 6)
            PushJoystickAxisEvent(1, axes[e.gaxis.axis], e.gaxis.value * axisMultiplier[e.gaxis.axis]); // Index 1 for Joystick
         break;
      case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      case SDL_EVENT_JOYSTICK_BUTTON_UP:
         if (e.jbutton.button < 32)
            PushJoystickButtonEvent(1, e.gbutton.button, e.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN); // Index 1 for Joystick
         break;
   }
}
#endif

void PinInput::HandleInputSDL()
{
   // When SDL Video is used, SDL events are processed during the main application message loop, so we do not do it again here
#if defined(ENABLE_SDL_INPUT) && !defined(ENABLE_SDL_VIDEO)
   if (m_inputApi != PI_SDL)
      return;
   SDL_Event e;
   while (SDL_PollEvent(&e) != 0)
      HandleSDLEvent(e);
#endif
}

void PinInput::PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration)
{
   if (m_rumbleMode == 0) return;

   switch (m_inputApi) {
   case PI_XINPUT:
#ifdef ENABLE_XINPUT
      if (m_inputDeviceXI >= 0) {
         m_rumbleOffTime = ms_duration + msec();
         m_rumbleRunning = true;
         XINPUT_VIBRATION vibration = {};
         // On both PS4 and X360:
         // The left motor is the low - frequency rumble motor. (explosions, etc)
         // The right motor is the high - frequency rumble motor. (subtle stuff)
         // The two motors are not the same, and they create different vibration effects.
         vibration.wLeftMotorSpeed = (WORD)(saturate(lowFrequencySpeed) * 65535.f);
         vibration.wRightMotorSpeed = (WORD)(saturate(highFrequencySpeed) * 65535.f);
         XInputSetState(m_inputDeviceXI, &vibration);
      }
#endif
      break;
   case PI_SDL:
   {
#ifdef ENABLE_SDL_INPUT
      SDL_PropertiesID props = SDL_GetGamepadProperties(m_pSDLGamePad);
      if (m_pSDLGamePad && SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false))
         SDL_RumbleGamepad(m_pSDLGamePad, (Uint16)(saturate(lowFrequencySpeed) * 65535.f), (Uint16)(saturate(highFrequencySpeed) * 65535.f), ms_duration);
      if (m_pSDLRumbleDevice)
         SDL_PlayHapticRumble(m_pSDLRumbleDevice, saturate(max(lowFrequencySpeed, highFrequencySpeed)), ms_duration); //!! meh
#endif
#ifdef __LIBVPINBALL__
      VPinballLib::RumbleData rumbleData = {
         (Uint16)(saturate(lowFrequencySpeed) * 65535.f),
         (Uint16)(saturate(highFrequencySpeed) * 65535.f),
         (Uint32)ms_duration
      };
      VPinballLib::VPinball::SendEvent(VPinballLib::Event::Rumble, &rumbleData);
#endif
   }
      break;
   default:
      break;
   }
}

#ifdef _WIN32
void PinInput::SetFocusWindow(HWND focusWnd)
{
   m_focusHWnd = focusWnd;
}
#endif

void PinInput::Init()
{
   UnmapAll();

   const Settings& settings = g_pvp->m_settings;
   m_override_default_buttons = settings.LoadValueWithDefault(Settings::Player, "PBWDefaultLayout"s, m_override_default_buttons);
   m_disable_esc = settings.LoadValueWithDefault(Settings::Player, "DisableESC"s, m_disable_esc);
   m_enableMouseInPlayer = settings.LoadValueWithDefault(Settings::Player, "EnableMouseInPlayer"s, m_enableMouseInPlayer);
   m_deadz = settings.LoadValueWithDefault(Settings::Player, "DeadZone"s, 0) * JOYRANGEMX / 100;

   m_lr_axis = settings.LoadValueWithDefault(Settings::Player, "LRAxis"s, m_lr_axis);
   m_ud_axis = settings.LoadValueWithDefault(Settings::Player, "UDAxis"s, m_ud_axis);
   m_lr_axis_reverse = settings.LoadValueWithDefault(Settings::Player, "LRAxisFlip"s, m_lr_axis_reverse);
   m_ud_axis_reverse = settings.LoadValueWithDefault(Settings::Player, "UDAxisFlip"s, m_ud_axis_reverse);
   m_plunger_axis = settings.LoadValueWithDefault(Settings::Player, "PlungerAxis"s, m_plunger_axis);
   m_plunger_speed_axis = settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedAxis"s, m_plunger_speed_axis);
   m_plunger_reverse = settings.LoadValueWithDefault(Settings::Player, "ReversePlungerAxis"s, m_plunger_reverse);
   m_plunger_retract = settings.LoadValueWithDefault(Settings::Player, "PlungerRetract"s, m_plunger_retract);
   
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
   
   m_joylflipkey = settings.LoadValueInt(Settings::Player, "JoyLFlipKey"s);
   m_joyrflipkey = settings.LoadValueInt(Settings::Player, "JoyRFlipKey"s);
   m_joyplungerkey = settings.LoadValueInt(Settings::Player, "JoyPlungerKey"s);
   m_joylefttilt = settings.LoadValueInt(Settings::Player, "JoyLTiltKey"s);
   m_joycentertilt = settings.LoadValueInt(Settings::Player, "JoyCTiltKey"s);
   m_joyrighttilt = settings.LoadValueInt(Settings::Player, "JoyRTiltKey"s);
   
   #ifdef _WIN32
      // Load input device settings
      static const string kDefaultName("None"s);

      m_inputDeviceSettingsInfo.clear();

      for (int i = 0; i < PININ_JOYMXCNT; i++)
      {
         const string deviceName = "Device" + std::to_string(i) + "_Name";
         const string name = settings.LoadValueWithDefault(Settings::ControllerDevices, deviceName, kDefaultName);

         if (!m_inputDeviceSettingsInfo.contains(name))
         {
            const string deviceState = "Device" + std::to_string(i) + "_State";
            const bool state = settings.LoadValueWithDefault(Settings::ControllerDevices, deviceState, true);

            m_inputDeviceSettingsInfo.insert(std::pair(name, state));
         }
      }
   #endif

   for (unsigned int i = 0; i < eCKeys; ++i)
      MapActionToKeyboard(static_cast<EnumAssignKeys>(i), g_pvp->m_settings.LoadValueInt(Settings::Player, regkey_string[i]), true);

#if defined(ENABLE_SDL_INPUT)
   if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK))
   {
      PLOGE << "SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK) failed: " << SDL_GetError();
      exit(1);
   }
   const string path = g_pvp->m_szMyPrefPath + "gamecontrollerdb.txt";
   if (!std::filesystem::exists(path))
      std::filesystem::copy(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "Default_gamecontrollerdb.txt", path);
   int count = SDL_AddGamepadMappingsFromFile(path.c_str());
   if (count > 0) {
      PLOGI.printf("Game controller mappings added: count=%d, path=%s", count, path.c_str());
   }
   else {
      PLOGI.printf("No game controller mappings added: path=%s", path.c_str());
   }
   RefreshSDLDevices();
#endif

#ifdef _WIN32
   HRESULT hr;
   #ifdef USE_DINPUT8
      hr = DirectInput8Create(g_pvp->theInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDI, nullptr);
   #else
      hr = DirectInputCreate(g_pvp->theInstance, DIRECTINPUT_VERSION, &m_pDI, nullptr);
   #endif

   #ifdef USE_DINPUT_FOR_KEYBOARD
      // Create keyboard device
      hr = m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr); //Standard Keyboard device
      hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
      hr = m_pKeyboard->SetCooperativeLevel(m_focusHWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); //!! exclusive necessary??

      DIPROPDWORD dipdw;
      dipdw.diph.dwSize = sizeof(DIPROPDWORD);
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdw.diph.dwObj = 0;
      dipdw.diph.dwHow = DIPH_DEVICE;
      dipdw.dwData = INPUT_BUFFER_SIZE;

      hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
   #endif

   // Create mouse device
   m_pMouse = nullptr;
   if (m_enableMouseInPlayer && SUCCEEDED(m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
   {
      hr = m_pMouse->SetDataFormat(&c_dfDIMouse2);
      hr = m_pMouse->SetCooperativeLevel(m_focusHWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
      DIPROPDWORD dipdwm;
      dipdwm.diph.dwSize = sizeof(DIPROPDWORD);
      dipdwm.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdwm.diph.dwObj = 0;
      dipdwm.diph.dwHow = DIPH_DEVICE;
      dipdwm.dwData = INPUT_BUFFER_SIZE;
      hr = m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdwm.diph);
   }

   /* Disable Sticky Keys */

   // get the current state
   m_startupStickyKeys.cbSize = sizeof(STICKYKEYS);
   SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_startupStickyKeys, 0);

   // turn it all OFF
   STICKYKEYS newStickyKeys = {};
   newStickyKeys.cbSize = sizeof(STICKYKEYS);
   newStickyKeys.dwFlags = 0;
   SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &newStickyKeys, SPIF_SENDCHANGE);
#endif

   ZeroMemory(&m_inputState, sizeof(m_inputState));
   m_nextKeyPressedTime = 0;
   uShockType = 0;

   #if defined(ENABLE_SDL_INPUT)
      m_inputApi = (InputAPI)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "InputApi"s, PI_SDL);
   #else
      m_inputApi = (InputAPI)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "InputApi"s, PI_DIRECTINPUT);
   #endif

   switch (m_inputApi) {
   case PI_XINPUT:
      #ifdef ENABLE_XINPUT
         m_inputDeviceXI = -1;
         uShockType = USHOCKTYPE_GENERIC;
         m_rumbleRunning = false;
      #else
         m_inputApi = PI_DIRECTINPUT;
      #endif
      break;

   case PI_SDL:
      #ifdef ENABLE_SDL_INPUT
         uShockType = USHOCKTYPE_GENERIC;
      #else
         m_inputApi = PI_DIRECTINPUT;
      #endif
      break;

   default:
      m_inputApi = PI_DIRECTINPUT;
      break;
   }

   m_rumbleMode = (m_inputApi != PI_DIRECTINPUT) ? g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "RumbleMode"s, 3) : 0;

   #ifdef _WIN32
      if (m_inputApi == PI_DIRECTINPUT) {
         #ifdef USE_DINPUT8
            m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoystickCallbackDI, this, DIEDFL_ATTACHEDONLY); //enum Joysticks
         #else
            m_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoystickCallbackDI, this, DIEDFL_ATTACHEDONLY);   //enum Joysticks
         #endif
      }

      // Store input device info
      for (int i = 0; i < PININ_JOYMXCNT; i++)
      {
            #ifdef USE_DINPUT8
            LPDIRECTINPUTDEVICE8 joystick = m_pJoystick[i];
            #else
            LPDIRECTINPUTDEVICE joystick = m_pJoystick[i];
            #endif

         if (joystick != nullptr)
         {
             LPDIDEVICEINSTANCE deviceInfo = new DIDEVICEINSTANCE;
             deviceInfo->dwSize = sizeof(DIDEVICEINSTANCE);
             joystick->GetDeviceInfo(deviceInfo);
             m_attachedDeviceInfo[i] = deviceInfo;
         }
      }
   #endif

   // initialize Open Pinball Device HIDs
   InitOpenPinballDevices();
}


void PinInput::UnInit()
{
   m_head = m_tail = 0;
   
   UnmapAll();

#if defined(ENABLE_SDL_INPUT)
   SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK);
#endif

#ifdef _WIN32
#ifdef USE_DINPUT_FOR_KEYBOARD
   if (m_pKeyboard)
   {
      // Unacquire the device one last time just in case 
      // the app tried to exit while the device is still acquired.
      m_pKeyboard->Unacquire();
      m_pKeyboard->Release();
      m_pKeyboard = nullptr;
   }
#endif

   if (m_pMouse)
   {
      m_pMouse->Unacquire();
      m_pMouse->Release();
      m_pMouse = nullptr;
   }

   // restore the state of the sticky keys
   SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_startupStickyKeys, SPIF_SENDCHANGE);

   //
   // DirectInput:
   //

   for (int k = 0; k < m_num_joy; ++k)
   {
      if (m_pJoystick[k])
      {
         // Unacquire the device one last time just in case
         // the app tried to exit while the device is still acquired.
         m_pJoystick[k]->Unacquire();
         m_pJoystick[k]->Release();
         m_pJoystick[k] = nullptr;
      }

      if (m_attachedDeviceInfo[k])
      {
         delete m_attachedDeviceInfo[k];
         m_attachedDeviceInfo[k] = nullptr;
      }
   }

   m_inputDeviceSettingsInfo.clear();

   // Release any DirectInput objects.
   SAFE_RELEASE(m_pDI);
#endif

   ZeroMemory(m_diq, sizeof(m_diq));

   // close Open Pinball Device interfaces
   TerminateOpenPinballDevices();
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

   if (isPressed)
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
      if (Started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled)
      {
         if (isPressed)
         {
            m_first_stamp = g_pplayer->m_last_frame_time_msec;
            m_exit_stamp = g_pplayer->m_last_frame_time_msec;
         }
         else
         {
            m_exit_stamp = 0;
            g_pplayer->m_showDebugger = true;
         }
      }
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

   if (!g_pplayer->m_liveUI->IsTweakMode())
   {
      if ((action == eLeftFlipperKey || action == eRightFlipperKey || action == eStagedLeftFlipperKey || action == eStagedRightFlipperKey) && isPressed)
      {
         g_pplayer->m_pininput.PlayRumble(0.f, 0.2f, 150);
         // Debug only, for testing parts of the flipper input lag
         m_leftkey_down_usec = usec();
         m_leftkey_down_frame = g_pplayer->m_overall_frames;
      }

      g_pplayer->m_ptable->FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[action]);
   }

   if (((action == eEscape) && !m_disable_esc) || (action == eExitGame))
   {
      // Check if we have started a game yet, and do not trigger if the UI is already opened (keyboard is handled in it)
      if (!g_pplayer->m_liveUI->IsOpened() && (Started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled))
      {
         if (isPressed)
         { //on key down only
            m_first_stamp = g_pplayer->m_last_frame_time_msec;
            m_exit_stamp = g_pplayer->m_last_frame_time_msec;
         }
         else
         {  //on key up only
            // Open UI on key up since a long press should not trigger the UI (direct exit from the app)
            g_pplayer->SetCloseState(Player::CS_USER_INPUT);
            m_exit_stamp = 0;
            #ifdef __STANDALONE__
               if (action == eExitGame)
                  g_pplayer->SetCloseState(Player::CS_CLOSE_APP);
            #endif
         }
      }
   }
}


// Returns true if the table has started at least 1 player.
int PinInput::Started()
{
   // Was the start button pressed?
   if (m_pressed_start)
      return 1;

   if (!g_pplayer->m_vball.empty())
   {
      m_pressed_start = true;
      return 1;
   }
   else
      return 0;
}

void PinInput::Autostart(const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec)
{
   //if (!g_pvp->m_open_minimized)
   //   return;

   // Make sure we have a player.
   if (!g_pplayer ||
      // Check if we already started.
      Started())
      return;

   if ((m_firedautostart > 0) &&                    // Initialized.
       m_as_down &&                                 // Start button is down.
       ((curr_time_msec - m_firedautostart) > 100)) // Start button has been down for at least 0.10 seconds.
   {
      // Release start.
      m_firedautostart = curr_time_msec;
      m_as_down = false;
      FireActionEvent(eStartGameKey, false);

      PLOGD << "Autostart: Release.";
   }

   // Logic to do "autostart"
   if (!m_as_down &&                                                                            // Start button is up.
       (( m_as_didonce && !Started() && ((curr_time_msec - m_firedautostart) > retry_msecs)) || // Not started and last attempt was at least AutoStartRetry seconds ago.
        (!m_as_didonce               && ((curr_time_msec - m_firedautostart) > msecs))))        // Never attempted and autostart time has elapsed.
   {
      // Press start.
      m_firedautostart = curr_time_msec;
      m_as_down = true;
      m_as_didonce = true;
      FireActionEvent(eStartGameKey, true);

      PLOGD << "Autostart: Press.\n";
   }
}

// FIXME Remove as this is also implemented in LiveUI
void PinInput::ButtonExit(const U32 msecs, const U32 curr_time_msec)
{
   // Don't allow button exit until after game has been running for 1 second.
   if (curr_time_msec - m_first_stamp < 1000)
      return;

   // Check if we can exit.
   if (m_exit_stamp &&                         // Initialized.
      (curr_time_msec - m_exit_stamp > msecs)) // Held exit button for number of mseconds.
   {
      g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
   }
}

// Setup an hardware device button and analog input mapping
// For the time being, an action may only be bound to one button as we do not handle combination of multiple sources
// For analog input, multiple source are supported, averaging for nudge and suming for plunger (assuming there is only one non 0)
void PinInput::SetupJoyMapping(int joystickId, int inputLayout)
{
   int lr_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LRAxis"s, 1);
   int ud_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "UDAxis"s, 2);
   bool lr_axis_reverse = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LRAxisFlip"s, false);
   bool ud_axis_reverse = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "UDAxisFlip"s, false);
   int plunger_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerAxis"s, 3);
   int plunger_speed_axis = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedAxis"s, 0);
   bool plunger_reverse = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ReversePlungerAxis"s, false);

   switch (inputLayout)
   {
   case USHOCKTYPE_PBWIZARD:
      SetupJoyMapping(joystickId, USHOCKTYPE_GENERIC);
      MapActionToJoystick(ePlungerKey, joystickId, DIJOFS_BUTTON0, true);
      MapActionToJoystick(eRightFlipperKey, joystickId, DIJOFS_BUTTON1, true);
      MapActionToJoystick(eRightMagnaSave, joystickId, DIJOFS_BUTTON2, true);
      MapActionToJoystick(eVolumeDown, joystickId, DIJOFS_BUTTON3, true);
      MapActionToJoystick(eVolumeUp, joystickId, DIJOFS_BUTTON4, true);
      // Button 5 is not mapped
      MapActionToJoystick(eEscape, joystickId, DIJOFS_BUTTON6, true);
      MapActionToJoystick(eExitGame, joystickId, DIJOFS_BUTTON7, true);
      MapActionToJoystick(eStartGameKey, joystickId, DIJOFS_BUTTON8, true);
      MapActionToJoystick(eLeftFlipperKey, joystickId, DIJOFS_BUTTON9, true);
      MapActionToJoystick(eLeftMagnaSave, joystickId, DIJOFS_BUTTON10, true);
      MapActionToJoystick(eAddCreditKey, joystickId, DIJOFS_BUTTON11, true);
      MapActionToJoystick(eAddCreditKey2, joystickId, DIJOFS_BUTTON12, true);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeX, joystickId, 1, true, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeY, joystickId, 2, false, false);
      if (plunger_axis != 0)
      { // This can be overriden and assigned to Rz instead of Z axis
         if (m_override_default_buttons && (plunger_axis == 6))
            MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, 6, false, false);
         else
            MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, 3, true, false);
      }
      break;

   case USHOCKTYPE_ULTRACADE:
      SetupJoyMapping(joystickId, USHOCKTYPE_GENERIC);
      MapActionToJoystick(eAddCreditKey, joystickId, DIJOFS_BUTTON11, true);
      MapActionToJoystick(eAddCreditKey2, joystickId, DIJOFS_BUTTON12, true);
      MapActionToJoystick(eRightMagnaSave, joystickId, DIJOFS_BUTTON2, true);
      // Button 3 is not mapped
      // Button 4 is not mapped
      MapActionToJoystick(eVolumeUp, joystickId, DIJOFS_BUTTON5, true);
      MapActionToJoystick(eVolumeDown, joystickId, DIJOFS_BUTTON6, true);
      // Button 7 is not mapped
      MapActionToJoystick(eLeftFlipperKey, joystickId, DIJOFS_BUTTON8, true);
      // Button 9 is not mapped
      MapActionToJoystick(eRightFlipperKey, joystickId, DIJOFS_BUTTON10, true);
      // Button 11 is not mapped
      MapActionToJoystick(eStartGameKey, joystickId, DIJOFS_BUTTON12, true);
      MapActionToJoystick(ePlungerKey, joystickId, DIJOFS_BUTTON13, true);
      MapActionToJoystick(eExitGame, joystickId, DIJOFS_BUTTON14, true);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeX, joystickId, 2, true, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeY, joystickId, 1, true, false);
      if (plunger_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, 3, false, false);
      break;

   case USHOCKTYPE_SIDEWINDER:
      SetupJoyMapping(joystickId, USHOCKTYPE_GENERIC);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeX, joystickId, 1, lr_axis_reverse, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeY, joystickId, 2, ud_axis_reverse, false);
      if (plunger_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, 7, !plunger_reverse, false);
      break;

   case USHOCKTYPE_VIRTUAPIN:
      SetupJoyMapping(joystickId, USHOCKTYPE_GENERIC);
      MapActionToJoystick(ePlungerKey, joystickId, DIJOFS_BUTTON0, true);
      MapActionToJoystick(eRightFlipperKey, joystickId, DIJOFS_BUTTON1, true);
      MapActionToJoystick(eRightMagnaSave, joystickId, DIJOFS_BUTTON2, true);
      MapActionToJoystick(eVolumeDown, joystickId, DIJOFS_BUTTON3, true);
      MapActionToJoystick(eVolumeUp, joystickId, DIJOFS_BUTTON4, true);
      // Button 5 is not mapped
      MapActionToJoystick(eEscape, joystickId, DIJOFS_BUTTON6, true);
      MapActionToJoystick(eExitGame, joystickId, DIJOFS_BUTTON7, true);
      MapActionToJoystick(eStartGameKey, joystickId, DIJOFS_BUTTON8, true);
      MapActionToJoystick(eLeftFlipperKey, joystickId, DIJOFS_BUTTON9, true);
      MapActionToJoystick(eLeftMagnaSave, joystickId, DIJOFS_BUTTON10, true);
      MapActionToJoystick(eAddCreditKey, joystickId, DIJOFS_BUTTON11, true);
      MapActionToJoystick(eAddCreditKey2, joystickId, DIJOFS_BUTTON12, true);

      if (lr_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeX, joystickId, 1, true, false);
      if (ud_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeY, joystickId, 2, false, false);
      if (plunger_axis != 0)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, 3, true, false);
      break;

   case USHOCKTYPE_OPENPINDEV:
      SetupJoyMapping(joystickId, USHOCKTYPE_GENERIC);

      // FIXME remove OpenPinDev hack
      // OpenPinDev has an ugly hack that it uses the generic axis mapping to identify itself by using a custom hardware code...
      // So for backward compatibility, we define virtual axis only used by this device
      m_analogActionMappings.clear();
      if (lr_axis == 9)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeX, joystickId, 10, lr_axis_reverse, false);
      if (ud_axis == 9)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeY, joystickId, 11, ud_axis_reverse, false);
      if (plunger_axis == 9)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, 12, plunger_reverse, false);
      if (plunger_speed_axis == 9)
         MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerSpeed, joystickId, 13, false, false);
      break;

   case USHOCKTYPE_GENERIC:
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
            MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeX, joystickId, lr_axis, lr_axis_reverse, false);
         if (ud_axis != 0)
            MapAnalogActionToJoystick(AnalogActionMapping::AM_NudgeY, joystickId, ud_axis, ud_axis_reverse, false);
         if (plunger_axis != 0)
            MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerPos, joystickId, plunger_axis, plunger_reverse, false);
         if (plunger_speed_axis != 0)
            MapAnalogActionToJoystick(AnalogActionMapping::AM_PlungerSpeed, joystickId, plunger_speed_axis, false, false);
      }
      break;
   }
}

void PinInput::ProcessKeys(int curr_time_msec, bool handleStartExit)
{
   if (!g_pplayer || !g_pplayer->m_ptable) return; // only if player is running
   g_pplayer->m_logicProfiler.OnProcessInput();

   GetInputDeviceData();

   ReadOpenPinballDevices(curr_time_msec);

   // Wipe key state if we're not the foreground window as we miss key-up events
   #ifdef _WIN32
   if (m_focusHWnd != GetForegroundWindow())
      ZeroMemory(&m_inputState, sizeof(m_inputState));
   #endif

   // Handle automatic start and exit on long press
   if (handleStartExit)
   {
      if (g_pplayer->m_ptable->m_tblAutoStartEnabled)
         Autostart(g_pplayer->m_ptable->m_tblAutoStart, g_pplayer->m_ptable->m_tblAutoStartRetry, curr_time_msec);
      ButtonExit(g_pplayer->m_ptable->m_tblExitConfirm, curr_time_msec);
   }
   if (m_firedautostart == 0) // Check if we've been initialized.
      m_firedautostart = curr_time_msec;

   // Global Backglass/Playfield sound volume
   if ((m_head == m_tail) && (curr_time_msec - m_nextKeyPressedTime) > 75)
   {
      static unsigned int lastVolumeNotifId = 0;
      m_nextKeyPressedTime = curr_time_msec;
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

   const DIDEVICEOBJECTDATA * __restrict input;
   while ((input = GetTail()))
   {
      if (input->dwSequence == APP_MOUSE && !g_pplayer->m_liveUI->HasMouseCapture() && !g_pplayer->m_throwBalls && !g_pplayer->m_ballControl)
      {
         for (int i = 1; i <= 3; ++i)
         {
            const int mouseButton = (i == 1) ? m_LeftMouseButtonID : ((i == 2) ? m_RightMouseButtonID : m_MiddleMouseButtonID);
            if (input->dwOfs == i)
            {
               const bool isDown = (input->dwData & 0x80) != 0;
               if (m_joylflipkey == mouseButton)
                  FireActionEvent(eLeftFlipperKey, isDown);
               else if (m_joyrflipkey == mouseButton)
                  FireActionEvent(eRightFlipperKey, isDown);
               else if (m_joyplungerkey == mouseButton)
                  FireActionEvent(ePlungerKey, isDown);
               else if (m_joylefttilt == mouseButton)
                  FireActionEvent(eLeftTiltKey, isDown);
               else if (m_joyrighttilt == mouseButton)
                  FireActionEvent(eRightTiltKey, isDown);
               else if (m_joycentertilt == mouseButton)
                  FireActionEvent(eCenterTiltKey, isDown);
            }
         }
      }
      else if (input->dwSequence == APP_KEYBOARD && !g_pplayer->m_liveUI->HasKeyboardCapture())
      {
         const auto keycode = static_cast<int>(input->dwOfs);
         const auto it = std::find_if(m_actionMappings.begin(), m_actionMappings.end(), [keycode](const ActionMapping& mapping) { 
            return (mapping.type == ActionMapping::AM_Keyboard) && (mapping.keycode == keycode); });
         if (it != m_actionMappings.end())
            FireActionEvent(it->action, (input->dwData & 0x80) != 0);
         else
            FireGenericKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, input->dwOfs);
      }
      else if (input->dwSequence == APP_ACTION)
      {
         const auto action = static_cast<EnumAssignKeys>(input->dwOfs);
         const bool isPressed = input->dwData != 0;
         FireActionEvent(action, isPressed);
      }
      else if (input->dwSequence >= APP_JOYSTICK0 && input->dwSequence <= APP_JOYSTICK0 + PININ_JOYMXCNT)
      {
         const int joystickId = input->dwSequence - APP_JOYSTICK0;
         if (input->dwOfs >= DIJOFS_BUTTON0 && input->dwOfs <= DIJOFS_BUTTON31)
         {
            const auto buttonId = static_cast<int>(input->dwOfs);
            const bool isPressed = (input->dwData & 0x80) != 0;
            const auto it = std::find_if(m_actionMappings.begin(), m_actionMappings.end(), [buttonId, joystickId](const ActionMapping& mapping) {
               return (mapping.type == ActionMapping::AM_Joystick) && (mapping.joystickId == buttonId) && (mapping.joystickId == joystickId); });
            if (it != m_actionMappings.end())
               FireActionEvent(it->action, isPressed);
            else
            {
               const int n = input->dwOfs - DIJOFS_BUTTON0 + 1;
               if (m_joycustom1 == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom1key);
               if (m_joycustom2 == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom2key);
               if (m_joycustom3 == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom3key);
               if (m_joycustom4 == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, m_joycustom4key);
               if (m_joypmbuyin == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_2);
               if (m_joypmcoin3 == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_5);
               if (m_joypmcoin4 == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_6);
               if (m_joypmcoindoor == n) FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_END);
               if (m_joypmcancel == n)   FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_7);
               if (m_joypmdown == n)     FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_8);
               if (m_joypmup == n)       FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_9);
               if (m_joypmenter == n)    FireGenericKeyEvent(isPressed ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, DIK_0);
            }
         }
         else
         {
            const int axis = input->dwOfs;
            const auto it = std::find_if(m_analogActionMappings.begin(), m_analogActionMappings.end(),
               [joystickId, axis](const AnalogActionMapping& mapping) { return (mapping.joystickId == joystickId) && (mapping.axisId == axis); });
            if (it != m_analogActionMappings.end())
            {
               const int rawValue = it->revert ? -static_cast<int>(input->dwData) : static_cast<int>(input->dwData);
               int value = rawValue;
               if (m_deadz > 0)
               {
                  if (((value <= 0) && (value >= -m_deadz)) || ((value >= 0) && (value <= m_deadz)))
                     value = 0;
                  else if (value < 0)
                     value = static_cast<int>(static_cast<float>(value + m_deadz) * static_cast<float>(JOYRANGEMX) / static_cast<float>(JOYRANGEMX - m_deadz));
                  else if (value > 0)
                     value = static_cast<int>(static_cast<float>(value - m_deadz) * static_cast<float>(JOYRANGEMX) / static_cast<float>(JOYRANGEMX - m_deadz));
               }
               switch (it->output)
               {
               case AnalogActionMapping::AM_NudgeX: g_pplayer->SetNudgeX(value, joystickId); break;
               case AnalogActionMapping::AM_NudgeY: g_pplayer->SetNudgeY(value, joystickId); break;
               case AnalogActionMapping::AM_PlungerPos: g_pplayer->MechPlungerIn(rawValue, joystickId); break;
               case AnalogActionMapping::AM_PlungerSpeed: g_pplayer->MechPlungerSpeedIn(rawValue, joystickId); break;
               }
            }
         }
      }
   }
}

int PinInput::GetNextKey() // return last valid keyboard key 
{
#ifdef USE_DINPUT_FOR_KEYBOARD
   if (m_pKeyboard != nullptr)
   {
      DIDEVICEOBJECTDATA didod[1];  // Receives buffered data
      DWORD dwElements;
      HRESULT hr;
#ifdef USE_DINPUT8
      LPDIRECTINPUTDEVICE8 pkyb = m_pKeyboard;
#else
      const LPDIRECTINPUTDEVICE pkyb = m_pKeyboard;
#endif
      for (int j = 0; j < 2; ++j)
      {
         dwElements = 1;
         hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);

         if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            pkyb->Acquire();
         else
            break;
      }

      if ((hr == S_OK || hr == DI_BUFFEROVERFLOW) && dwElements != 0)
         return didod[0].dwOfs;
   }
#else
   for (unsigned int i = 0; i < 0xFF; ++i)
   {
#ifndef __STANDALONE__
      const SHORT keyState = GetAsyncKeyState(i);
      if (keyState & 1)
      {
         const unsigned int dik = get_dik(i);
         if (dik != ~0u)
            return dik;
      }
#endif
   }
#endif

   return 0;
}

#ifdef _WIN32
#ifdef USE_DINPUT8
LPDIRECTINPUTDEVICE8 PinInput::GetJoystick(int index)
#else
LPDIRECTINPUTDEVICE PinInput::GetJoystick(int index)
#endif
{
   return (index < PININ_JOYMXCNT) ? m_pJoystick[index] : nullptr;
}
#endif


