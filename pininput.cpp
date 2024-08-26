#include "stdafx.h"
#include <cfgmgr32.h>
#include <SetupAPI.h>

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


#define INPUT_BUFFER_SIZE MAX_KEYQUEUE_SIZE
#define BALLCONTROL_DOUBLECLICK_THRESHOLD_USEC (500 * 1000)


PinInput::PinInput()
{
   ZeroMemory(this, sizeof(PinInput));

   // thanks to the ZeroMemory above, we have to explicitly call member
   // object constructors in-place
   new (&m_openPinDevs) std::list<OpenPinDev>();

#ifdef _WIN32
   m_pDI = nullptr;
#ifdef USE_DINPUT_FOR_KEYBOARD
   m_pKeyboard = nullptr;
#endif
   m_pMouse = nullptr;
#endif

   m_leftMouseButtonDown = false;
   m_rightMouseButtonDown = false;
   m_middleMouseButtonDown = false;

   m_head = m_tail = 0;

   ZeroMemory(m_diq, sizeof(m_diq));

   m_num_joy = 0;
#ifdef _WIN32
   for (int k = 0; k < PININ_JOYMXCNT; ++k)
      m_pJoystick[k] = nullptr;
#endif

   uShockType = 0;

   m_plunger_axis = 3;
   m_plunger_speed_axis = 0;
   m_lr_axis = 1;
   m_ud_axis = 2;
   m_plunger_reverse = false;
   m_plunger_retract = false;
   m_lr_axis_reverse = false;
   m_ud_axis_reverse = false;
   m_override_default_buttons = false;
   m_disable_esc = false;
   m_joylflipkey = 0;
   m_joyrflipkey = 0;
   m_joystagedlflipkey = 0;
   m_joystagedrflipkey = 0;
   m_joylmagnasave = 0;
   m_joyrmagnasave = 0;
   m_joyplungerkey = 0;
   m_joystartgamekey = 0;
   m_joyexitgamekey = 0;
   m_joyaddcreditkey = 0;
   m_joyaddcreditkey2 = 0;
   m_joyframecount = 0;
   m_joyvolumeup = 0;
   m_joyvolumedown = 0;
   m_joylefttilt = 0;
   m_joycentertilt = 0;
   m_joyrighttilt = 0;
   m_joypmbuyin = 0;
   m_joypmcoin3 = 0;
   m_joypmcoin4 = 0;
   m_joypmcoindoor = 0;
   m_joypmcancel = 0;
   m_joypmdown = 0;
   m_joypmup = 0;
   m_joypmenter = 0;
   m_joycustom1 = 0;
   m_joycustom2 = 0;
   m_joycustom3 = 0;
   m_joycustom4 = 0;
   m_joydebugballs = 0;
   m_joydebugger = 0;
   m_joylockbar = 0;
   m_joypause = 0;
   m_joytweak = 0;
   m_joymechtilt = 0;
   m_joytablerecenter = 0;
   m_joytableup = 0;
   m_joytabledown = 0;

   m_firedautostart = 0;

   m_pressed_start = false;

   m_enableMouseInPlayer = true;
   m_enableCameraModeFlyAround = false;
   m_enable_nudge_filter = false;

   m_cameraModeAltKey = false;
   m_cameraMode = 0;

   m_exit_stamp = 0;
   m_first_stamp = msec();

   m_as_down = false;
   m_as_didonce = false;

   m_tilt_updown = false;

   m_linearPlunger = false;

   m_mixerKeyDown = false;
   m_mixerKeyUp = false;
}

PinInput::~PinInput()
{
#ifdef ENABLE_SDL_INPUT
#ifndef ENABLE_SDL_GAMECONTROLLER
   if (m_rumbleDeviceSDL)
      SDL_HapticClose(m_rumbleDeviceSDL);
   if (m_inputDeviceSDL)
      SDL_JoystickClose(m_inputDeviceSDL);
#else
   SDL_GameControllerClose(m_gameController);
#endif
#endif
}

void PinInput::LoadSettings(const Settings& settings)
{
   m_lr_axis = settings.LoadValueWithDefault(Settings::Player, "LRAxis"s, m_lr_axis);
   m_ud_axis = settings.LoadValueWithDefault(Settings::Player, "UDAxis"s, m_ud_axis);
   m_lr_axis_reverse = settings.LoadValueWithDefault(Settings::Player, "LRAxisFlip"s, m_lr_axis_reverse);
   m_ud_axis_reverse = settings.LoadValueWithDefault(Settings::Player, "UDAxisFlip"s, m_ud_axis_reverse);
   m_plunger_axis = settings.LoadValueWithDefault(Settings::Player, "PlungerAxis"s, m_plunger_axis);
   m_plunger_speed_axis = settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedAxis"s, m_plunger_speed_axis);
   m_plunger_reverse = settings.LoadValueWithDefault(Settings::Player, "ReversePlungerAxis"s, m_plunger_reverse);
   m_plunger_retract = settings.LoadValueWithDefault(Settings::Player, "PlungerRetract"s, m_plunger_retract);
   m_override_default_buttons = settings.LoadValueWithDefault(Settings::Player, "PBWDefaultLayout"s, m_override_default_buttons);
   m_disable_esc = settings.LoadValueWithDefault(Settings::Player, "DisableESC"s, m_disable_esc);
   m_joylflipkey = settings.LoadValueWithDefault(Settings::Player, "JoyLFlipKey"s, m_joylflipkey);
   m_joyrflipkey = settings.LoadValueWithDefault(Settings::Player, "JoyRFlipKey"s, m_joyrflipkey);
   m_joystagedlflipkey = settings.LoadValueWithDefault(Settings::Player, "JoyStagedLFlipKey"s, m_joystagedlflipkey);
   m_joystagedrflipkey = settings.LoadValueWithDefault(Settings::Player, "JoyStagedRFlipKey"s, m_joystagedrflipkey);
   m_joyplungerkey = settings.LoadValueWithDefault(Settings::Player, "JoyPlungerKey"s, m_joyplungerkey);
   m_joyaddcreditkey = settings.LoadValueWithDefault(Settings::Player, "JoyAddCreditKey"s, m_joyaddcreditkey);
   m_joyaddcreditkey2 = settings.LoadValueWithDefault(Settings::Player, "JoyAddCredit2Key"s, m_joyaddcreditkey2);
   m_joylmagnasave = settings.LoadValueWithDefault(Settings::Player, "JoyLMagnaSave"s, m_joylmagnasave);
   m_joyrmagnasave = settings.LoadValueWithDefault(Settings::Player, "JoyRMagnaSave"s, m_joyrmagnasave);
   m_joystartgamekey = settings.LoadValueWithDefault(Settings::Player, "JoyStartGameKey"s, m_joystartgamekey);
   m_joyframecount = settings.LoadValueWithDefault(Settings::Player, "JoyFrameCount"s, m_joyframecount);
   m_joyexitgamekey = settings.LoadValueWithDefault(Settings::Player, "JoyExitGameKey"s, m_joyexitgamekey);
   m_joyvolumeup = settings.LoadValueWithDefault(Settings::Player, "JoyVolumeUp"s, m_joyvolumeup);
   m_joyvolumedown = settings.LoadValueWithDefault(Settings::Player, "JoyVolumeDown"s, m_joyvolumedown);
   m_joylefttilt = settings.LoadValueWithDefault(Settings::Player, "JoyLTiltKey"s, m_joylefttilt);
   m_joycentertilt = settings.LoadValueWithDefault(Settings::Player, "JoyCTiltKey"s, m_joycentertilt);
   m_joyrighttilt = settings.LoadValueWithDefault(Settings::Player, "JoyRTiltKey"s, m_joyrighttilt);
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
   m_joymechtilt = settings.LoadValueWithDefault(Settings::Player, "JoyMechTiltKey"s, m_joymechtilt);
   m_joydebugballs = settings.LoadValueWithDefault(Settings::Player, "JoyDebugKey"s, m_joydebugballs);
   m_joydebugger = settings.LoadValueWithDefault(Settings::Player, "JoyDebuggerKey"s, m_joydebugger);
   m_joylockbar = settings.LoadValueWithDefault(Settings::Player, "JoyLockbarKey"s, m_joylockbar);
   m_joypause = settings.LoadValueWithDefault(Settings::Player, "JoyPauseKey"s, m_joypause);
   m_joytweak = settings.LoadValueWithDefault(Settings::Player, "JoyTweakKey"s, m_joytweak);
   m_joytablerecenter = settings.LoadValueWithDefault(Settings::Player, "JoyTableRecenterKey"s, m_joytablerecenter);
   m_joytableup = settings.LoadValueWithDefault(Settings::Player, "JoyTableUpKey"s, m_joytableup);
   m_joytabledown = settings.LoadValueWithDefault(Settings::Player, "JoyTableDownKey"s, m_joytabledown);
   m_enableMouseInPlayer = settings.LoadValueWithDefault(Settings::Player, "EnableMouseInPlayer"s, m_enableMouseInPlayer);
   m_enableCameraModeFlyAround = settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, m_enableCameraModeFlyAround);
   m_enable_nudge_filter = settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, m_enable_nudge_filter);
   m_deadz = settings.LoadValueWithDefault(Settings::Player, "DeadZone"s, 0);
   m_deadz = m_deadz*JOYRANGEMX / 100;
}

#ifdef ENABLE_SDL_GAMECONTROLLER
void PinInput::SetupSDLGameController()
{
   if (m_gameController != nullptr) {
      SDL_GameControllerClose(m_gameController);
      m_gameController = nullptr;
   }

   if(SDL_NumJoysticks() < 1) {
      PLOGI.printf("No joysticks connected!");
   }
   else {
      for (int idx = 0; idx < SDL_NumJoysticks(); idx++) {
         if (!SDL_IsGameController(idx)) {
            PLOGI.printf("Joystick %d is not a game controller", idx);
         }
         else {
#if defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))
            if (!lstrcmpi(SDL_GameControllerNameForIndex(idx), "Remote"))
               continue;
#endif
            m_gameController = SDL_GameControllerOpen(idx);

            PLOGI.printf("Game controller added: name=%s, rumble=%s",
               SDL_GameControllerName(m_gameController),
               SDL_GameControllerHasRumble(m_gameController) ? "true" : "false");

            break;
         }
      }
   }
}
#endif

//
// DirectInput:
//

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallbackDI()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//		joystick. This function enables user interface elements for objects
//		that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
#ifdef _WIN32
BOOL CALLBACK EnumObjectsCallbackDI(const DIDEVICEOBJECTINSTANCE* pdidoi,
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
BOOL CALLBACK EnumJoystickCallbackDI(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
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

   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->GetProperty(DIPROP_PRODUCTNAME, &dstr.diph);
   if (hr == S_OK)
   {
      if (!WzSzStrCmp(dstr.wsz, "PinballWizard"))
         ppinput->uShockType = USHOCKTYPE_PBWIZARD; // set type 1 = PinballWizard
      else if (!WzSzStrCmp(dstr.wsz, "UltraCade Pinball"))
         ppinput->uShockType = USHOCKTYPE_ULTRACADE; // set type 2 = UltraCade Pinball
      else if (!WzSzStrCmp(dstr.wsz, "Microsoft SideWinder Freestyle Pro (USB)"))
         ppinput->uShockType = USHOCKTYPE_SIDEWINDER; // set type 3 = Microsoft SideWinder Freestyle Pro
      else if (!WzSzStrCmp(dstr.wsz, "VirtuaPin Controller"))
         ppinput->uShockType = USHOCKTYPE_VIRTUAPIN; // set type 4 = VirtuaPin Controller
      else if (!WzSzStrCmp(dstr.wsz, "Pinscape Controller") || !WzSzStrCmp(dstr.wsz, "PinscapePico"))
      {
         ppinput->uShockType = USHOCKTYPE_GENERIC;  // set type = Generic
         ppinput->m_linearPlunger = true;           // use linear plunger calibration
      }
      else
         ppinput->uShockType = USHOCKTYPE_GENERIC;  // Generic Gamepad
   }
   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->SetDataFormat(&c_dfDIJoystick);

   // joystick input foreground or background focus
   hr = ppinput->m_pJoystick[ppinput->m_num_joy]->SetCooperativeLevel(ppinput->m_hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

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

   if (++(ppinput->m_num_joy) < PININ_JOYMXCNT)
       return DIENUM_CONTINUE;
   else
       return DIENUM_STOP; //allocation for only PININ_JOYMXCNT joysticks, ignore any others
}
#endif

void PinInput::PushQueue(DIDEVICEOBJECTDATA * const data, const unsigned int app_data/*, const U32 curr_time_msec*/)
{
   if ((!data) ||
       (((m_head + 1) % MAX_KEYQUEUE_SIZE) == m_tail)) // queue full?
       return;

   m_diq[m_head] = *data;
   //m_diq[m_head].dwTimeStamp = curr_time_msec; //rewrite time from game start
   m_diq[m_head].dwSequence = app_data;

   m_head = (m_head + 1) % MAX_KEYQUEUE_SIZE; // advance head of queue
}

const DIDEVICEOBJECTDATA *PinInput::GetTail(/*const U32 curr_sim_msec*/)
{
   if (m_head == m_tail)
      return nullptr; // queue empty?

   const DIDEVICEOBJECTDATA * const ptr = &m_diq[m_tail];

   // If we've simulated to or beyond the timestamp of when this control was received then process the control into the system
   //if ( curr_sim_msec >= ptr->dwTimeStamp ) //!! time stamp disabled to save a bit of lag
   {
      m_tail = (m_tail + 1) % MAX_KEYQUEUE_SIZE; // advance tail of queue

      return ptr;
   }
   //else return nullptr;
}

//
// End of Direct Input specific code
//

void PinInput::GetInputDeviceData(/*const U32 curr_time_msec*/)
{
   DIDEVICEOBJECTDATA didod[INPUT_BUFFER_SIZE]; // Receives buffered data 

#ifdef USE_DINPUT_FOR_KEYBOARD
   // keyboard
#ifdef USE_DINPUT8
   const LPDIRECTINPUTDEVICE8 pkyb = m_pKeyboard;
#else
   const LPDIRECTINPUTDEVICE pkyb = m_pKeyboard;
#endif
   if (pkyb)
   {
      HRESULT hr = pkyb->Acquire();				// try to acquire keyboard input
      if (hr == S_OK || hr == S_FALSE)
      {
         DWORD dwElements = INPUT_BUFFER_SIZE;
         hr = pkyb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);

         if (hr == S_OK || hr == DI_BUFFEROVERFLOW)
         {
            if (m_hwnd == GetForegroundWindow())
               for (DWORD i = 0; i < dwElements; i++)
                  PushQueue(&didod[i], APP_KEYBOARD/*, curr_time_msec*/);
         }
      }
   }
#else
   // cache to avoid double key triggers
   static bool oldKeyStates[eCKeys] = { false };

#ifdef _WIN32
   unsigned int i2 = 0;
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

      didod[i2].dwOfs = rgk;
      didod[i2].dwData = keyDown ? 0x80 : 0;
      //didod[i2].dwTimeStamp = curr_time_msec;
      didod[i2].dwSequence = APP_KEYBOARD;
      PushQueue(&didod[i2], APP_KEYBOARD/*, curr_time_msec*/);
      ++i2;
   }
#endif
#endif

#ifdef _WIN32
   // mouse
   if (m_pMouse && m_enableMouseInPlayer)
   {
      HRESULT hr = m_pMouse->Acquire();	// try to acquire mouse input
      if (hr == S_OK || hr == S_FALSE)
      {
         DIMOUSESTATE2 mouseState;
         hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState);

         if ((hr == S_OK || hr == DI_BUFFEROVERFLOW) && (m_hwnd == GetForegroundWindow()))
         {
            if (g_pplayer->m_throwBalls || g_pplayer->m_ballControl) // debug ball throw functionality
            {
               if ((mouseState.rgbButtons[0] & 0x80) && !m_leftMouseButtonDown && !m_rightMouseButtonDown && !m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  m_mouseX = curPos.x;
                  m_mouseY = curPos.y;
                  m_leftMouseButtonDown = true;
               }
               if (!(mouseState.rgbButtons[0] & 0x80) && m_leftMouseButtonDown && !m_rightMouseButtonDown && !m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  m_mouseDX = curPos.x - m_mouseX;
                  m_mouseDY = curPos.y - m_mouseY;
                  didod[0].dwData = 1;
                  PushQueue(didod, APP_MOUSE);
                  m_leftMouseButtonDown = false;
               }
               if ((mouseState.rgbButtons[1] & 0x80) && !m_rightMouseButtonDown && !m_leftMouseButtonDown && !m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  m_mouseX = curPos.x;
                  m_mouseY = curPos.y;
                  m_rightMouseButtonDown = true;
               }
               if (!(mouseState.rgbButtons[1] & 0x80) && !m_leftMouseButtonDown && m_rightMouseButtonDown && !m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  m_mouseDX = curPos.x - m_mouseX;
                  m_mouseDY = curPos.y - m_mouseY;
                  didod[0].dwData = 2;
                  PushQueue(didod, APP_MOUSE);
                  m_rightMouseButtonDown = false;
               }
               if ((mouseState.rgbButtons[2] & 0x80) && !m_rightMouseButtonDown && !m_leftMouseButtonDown && !m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  m_mouseX = curPos.x;
                  m_mouseY = curPos.y;
                  m_middleMouseButtonDown = true;
               }
               if (!(mouseState.rgbButtons[2] & 0x80) && !m_rightMouseButtonDown && !m_leftMouseButtonDown && m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  m_mouseDX = curPos.x - m_mouseX;
                  m_mouseDY = curPos.y - m_mouseY;
                  didod[0].dwData = 3;
                  PushQueue(didod, APP_MOUSE);
                  m_middleMouseButtonDown = false;
               }
               if (g_pplayer->m_ballControl && !g_pplayer->m_throwBalls && m_leftMouseButtonDown && !m_rightMouseButtonDown && !m_middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  didod[0].dwData = 4;
                  m_mouseX = curPos.x;
                  m_mouseY = curPos.y;
                  PushQueue(didod, APP_MOUSE);
               }

            } //if (g_pplayer->m_fThrowBalls)
            else
            {
               for (DWORD i = 0; i < 3; i++)
               {
                  if (m_oldMouseButtonState[i] != mouseState.rgbButtons[i])
                  {
                     didod[i].dwData = mouseState.rgbButtons[i];
                     didod[i].dwOfs = i + 1;
                     didod[i].dwSequence = APP_MOUSE;
                     PushQueue(&didod[i], APP_MOUSE);
                     m_oldMouseButtonState[i] = mouseState.rgbButtons[i];
                  }
               }

            }
         }
      }
   }
#endif

   // same for joysticks 
   switch (m_inputApi) {
   case 1:
      HandleInputXI(didod);
      break;
   case 2:
      HandleInputSDL(didod);
      break;
   case 3:
      HandleInputIGC(didod);
      break;
   case 0:
   default:
      HandleInputDI(didod);
      break;
   }
}

void PinInput::HandleInputDI(DIDEVICEOBJECTDATA *didod)
{
#ifdef _WIN32
   for (int k = 0; k < m_num_joy; ++k)
   {
#ifdef USE_DINPUT8
      const LPDIRECTINPUTDEVICE8 pjoy = m_pJoystick[k];
#else
      const LPDIRECTINPUTDEVICE pjoy = m_pJoystick[k];
#endif
      if (pjoy)
      {
         HRESULT hr = pjoy->Acquire();		// try to acquire joystick input
         if (hr == S_OK || hr == S_FALSE)
         {
            DWORD dwElements = INPUT_BUFFER_SIZE;
            hr = pjoy->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);

            if (hr == S_OK || hr == DI_BUFFEROVERFLOW)
            {
               if (m_hwnd == GetForegroundWindow())
                  for (DWORD i = 0; i < dwElements; i++)
                     PushQueue(&didod[i], APP_JOYSTICK(k)/*, curr_time_msec*/);
            }
         }
      }
   }
#endif
}

void PinInput::HandleInputXI(DIDEVICEOBJECTDATA *didod)
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
   if (m_inputDeviceXI != -2 && (m_inputDeviceXI == -1 || (xie = XInputGetState(m_inputDeviceXI, &state)) != ERROR_SUCCESS)) {
      m_inputDeviceXI = -1;
      m_num_joy = 0;
      for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
      {
         ZeroMemory(&state, sizeof(XINPUT_STATE));
         if ((xie = XInputGetState(i, &state)) == ERROR_SUCCESS) {
            m_inputDeviceXI = i;
            m_num_joy = 1;
            break;
         }
      }
   }
   if (xie == ERROR_DEVICE_NOT_CONNECTED) { // XInputGetState can cause quite some overhead, especially if no devices connected! Thus disable the polling if nothing connected
      m_inputDeviceXI = -2;
      m_num_joy = 0;
   }
   if (m_rumbleRunning && m_inputDeviceXI >= 0) {
      const DWORD now = timeGetTime();
      if (m_rumbleOffTime <= now || m_rumbleOffTime - now > 65535) {
         m_rumbleRunning = false;
         XINPUT_VIBRATION vibration = {};
         XInputSetState(m_inputDeviceXI, &vibration);
      }
   }
   int i = 0;
   int j = 0;
   while (mappingTable[i].xi != 0) {
      if ((m_inputDeviceXIstate.Gamepad.wButtons & mappingTable[i].xi) != (state.Gamepad.wButtons & mappingTable[i].xi)) {
         didod[j].dwOfs = mappingTable[i].di;
         didod[j].dwData = (state.Gamepad.wButtons & mappingTable[i].xi) > 0 ? 0x80 : 0x00;
         PushQueue(&didod[j], APP_JOYSTICK(0));
         j++;
      }
      i++;
   }
   if (m_inputDeviceXIstate.Gamepad.bLeftTrigger != state.Gamepad.bLeftTrigger) {
      didod[j].dwOfs = DIJOFS_Z;
      const int value = (int)state.Gamepad.bLeftTrigger * 512;
      didod[j].dwData = (DWORD)(value);
      PushQueue(&didod[j], APP_JOYSTICK(0));
      j++;
   }
   if (m_inputDeviceXIstate.Gamepad.bRightTrigger != state.Gamepad.bRightTrigger) {
      didod[j].dwOfs = DIJOFS_RZ;
      const int value = (int)state.Gamepad.bRightTrigger * 512;
      didod[j].dwData = (DWORD)(value);
      PushQueue(&didod[j], APP_JOYSTICK(0));
      j++;
   }
   if (m_inputDeviceXIstate.Gamepad.sThumbLX != state.Gamepad.sThumbLX) {
      didod[j].dwOfs = DIJOFS_X;
      const int value = (int)state.Gamepad.sThumbLX * -2;
      didod[j].dwData = (DWORD)(value);
      PushQueue(&didod[j], APP_JOYSTICK(0));
      j++;
   }
   if (m_inputDeviceXIstate.Gamepad.sThumbLY != state.Gamepad.sThumbLY) {
      didod[j].dwOfs = DIJOFS_Y;
      const int value = (int)state.Gamepad.sThumbLY * -2;
      didod[j].dwData = (DWORD)(value);
      PushQueue(&didod[j], APP_JOYSTICK(0));
      j++;
   }
   if (m_inputDeviceXIstate.Gamepad.sThumbRX != state.Gamepad.sThumbRX) {
      didod[j].dwOfs = DIJOFS_RX;
      const int value = (int)state.Gamepad.sThumbRX * -2;
      didod[j].dwData = (DWORD)(value);
      PushQueue(&didod[j], APP_JOYSTICK(0));
      j++;
   }
   if (m_inputDeviceXIstate.Gamepad.sThumbRY != state.Gamepad.sThumbRY) {
      didod[j].dwOfs = DIJOFS_RY;
      const int value = (int)state.Gamepad.sThumbRY * -2;
      didod[j].dwData = (DWORD)(value);
      PushQueue(&didod[j], APP_JOYSTICK(0));
      j++;
   }
   memcpy(&m_inputDeviceXIstate, &state, sizeof(XINPUT_STATE));
#endif
}

void PinInput::HandleInputSDL(DIDEVICEOBJECTDATA *didod)
{
#ifdef ENABLE_SDL_INPUT
   static constexpr DWORD axes[] = { DIJOFS_X, DIJOFS_Y, DIJOFS_RX, DIJOFS_RY, DIJOFS_Z , DIJOFS_RZ };
   static constexpr int axisMultiplier[] = { 2, 2, 2, 2, 256 , 256 };
   SDL_Event e;
   int j = 0;
   while (SDL_PollEvent(&e) != 0 && j<32)
   {
      //User requests quit
      switch (e.type) {
      case SDL_QUIT:
         //Open Exit dialog
         break;
#ifndef ENABLE_SDL_GAMECONTROLLER
      case SDL_JOYDEVICEADDED:
         if (!m_inputDeviceSDL) {
            m_inputDeviceSDL = SDL_JoystickOpen(0);
            if (m_inputDeviceSDL) {
               m_num_joy = 1;
               if (SDL_JoystickIsHaptic(m_inputDeviceSDL)) {
                  m_rumbleDeviceSDL = SDL_HapticOpenFromJoystick(m_inputDeviceSDL);
                  const int error = SDL_HapticRumbleInit(m_rumbleDeviceSDL);
                  if (error < 0) {
                     ShowError(SDL_GetError());
                     SDL_HapticClose(m_rumbleDeviceSDL);
                     m_rumbleDeviceSDL = nullptr;
                  }
               }
            }
         }
         break;
      case SDL_JOYDEVICEREMOVED:
         if (m_rumbleDeviceSDL)
            SDL_HapticClose(m_rumbleDeviceSDL);
         m_rumbleDeviceSDL = nullptr;
         if (m_inputDeviceSDL)
            SDL_JoystickClose(m_inputDeviceSDL);
         if (SDL_NumJoysticks() > 0) {
            m_inputDeviceSDL = SDL_JoystickOpen(0);
            if (m_inputDeviceSDL) {
               m_num_joy = 1;
               if (SDL_JoystickIsHaptic(m_inputDeviceSDL)) {
                  m_rumbleDeviceSDL = SDL_HapticOpenFromJoystick(m_inputDeviceSDL);
                  const int error = SDL_HapticRumbleInit(m_rumbleDeviceSDL);
                  if (error < 0) {
                     ShowError(SDL_GetError());
                     SDL_HapticClose(m_rumbleDeviceSDL);
                     m_rumbleDeviceSDL = nullptr;
                  }
               }
            }
         }
         else {
            m_inputDeviceSDL = nullptr;
            m_num_joy = 0;
         }
         break;
      case SDL_JOYAXISMOTION:
         if (e.jaxis.axis < 6) {
            didod[j].dwOfs = axes[e.jaxis.axis];
            const int value = e.jaxis.value * axisMultiplier[e.jaxis.axis];
            didod[j].dwData = (DWORD)(value);
            PushQueue(&didod[j], APP_JOYSTICK(0));
            j++;
         }
         break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
         if (e.jbutton.button < 32) {
            didod[j].dwOfs = DIJOFS_BUTTON0 + (DWORD)e.jbutton.button;
            didod[j].dwData = e.type == SDL_JOYBUTTONDOWN ? 0x80 : 0x00;
            PushQueue(&didod[j], APP_JOYSTICK(0));
            j++;
         }
         break;
#else
      case SDL_CONTROLLERDEVICEADDED:
      case SDL_CONTROLLERDEVICEREMOVED:
         SetupSDLGameController();
         break;
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
         if (e.cbutton.button < 32) {
            didod[j].dwOfs = DIJOFS_BUTTON0 + (DWORD)e.cbutton.button;
            didod[j].dwData = e.type == SDL_CONTROLLERBUTTONDOWN ? 0x80 : 0x00;
            PushQueue(&didod[j], APP_JOYSTICK(0));
            j++;
         }
         break;
#endif
      }
   }
#endif
}

void PinInput::HandleInputIGC(DIDEVICEOBJECTDATA *didod)
{
#ifdef ENABLE_IGAMECONTROLLER
#endif
}

void PinInput::PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration)
{
   if (m_rumbleMode == 0) return;

   switch (m_inputApi) {
   case 1: //XInput
#ifdef ENABLE_XINPUT
      if (m_inputDeviceXI >= 0) {
         m_rumbleOffTime = ms_duration + timeGetTime();
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
   case 2: //SDL2
#ifdef ENABLE_SDL_INPUT
#ifndef ENABLE_SDL_GAMECONTROLLER
      if (m_rumbleDeviceSDL)
         SDL_HapticRumblePlay(m_rumbleDeviceSDL, saturate(max(lowFrequencySpeed, highFrequencySpeed)), ms_duration); //!! meh
#else
      if (m_gameController && SDL_GameControllerHasRumble(m_gameController))
         SDL_GameControllerRumble(m_gameController, (Uint16)(saturate(lowFrequencySpeed) * 65535.f), (Uint16)(saturate(highFrequencySpeed) * 65535.f), ms_duration);
#endif
#endif
      break;
   case 3: //IGameController
#ifdef ENABLE_IGAMECONTROLLER
#endif
      break;
   }
}

void PinInput::Init(const HWND hwnd)
{
   m_hwnd = hwnd;

#if defined(ENABLE_SDL_INPUT)
#ifndef ENABLE_SDL_GAMECONTROLLER
   SDL_InitSubSystem(SDL_INIT_JOYSTICK);
#else
   SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
#endif
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

   hr = m_pKeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); //!! exclusive necessary??

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
   if (m_enableMouseInPlayer && !FAILED(m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
   {
      hr = m_pMouse->SetDataFormat(&c_dfDIMouse2);
      hr = m_pMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
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

   for (int i = 0; i < eCKeys; i++)
      m_keyPressedState[i] = false;
   m_nextKeyPressedTime = 0;
   uShockType = 0;

   m_inputApi = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "InputApi"s, 0);

   switch (m_inputApi) {
   case 1: //xInput
#ifdef ENABLE_XINPUT
      m_inputDeviceXI = -1;
      uShockType = USHOCKTYPE_GENERIC;
      m_rumbleRunning = false;
#else
      m_inputApi = 0;
#endif
      break;
   case 2: //SDL2
#ifdef ENABLE_SDL_INPUT
#ifndef ENABLE_SDL_GAMECONTROLLER
      m_inputDeviceSDL = SDL_JoystickOpen(0);
      if (m_inputDeviceSDL) {
         m_num_joy = 1;
         if (SDL_JoystickIsHaptic(m_inputDeviceSDL)) {
            m_rumbleDeviceSDL = SDL_HapticOpenFromJoystick(m_inputDeviceSDL);
            int error = SDL_HapticRumbleInit(m_rumbleDeviceSDL);
            if (error < 0) {
               ShowError(SDL_GetError());
               SDL_HapticClose(m_rumbleDeviceSDL);
               m_rumbleDeviceSDL = nullptr;
            }
         }
      }
      uShockType = USHOCKTYPE_GENERIC;
#else
      SetupSDLGameController();
#endif
#else
      m_inputApi = 0;
#endif
      break;
   case 3: //iGameController
#ifdef ENABLE_IGAMECONTROLLER
#else
      m_inputApi = 0;
#endif
      break;
   default:
      m_inputApi = 0;
      break;
   }

   m_rumbleMode = (m_inputApi > 0) ? g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "RumbleMode"s, 3) : 0;

#ifdef _WIN32
   if (m_inputApi == 0) {
#ifdef USE_DINPUT8
      m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoystickCallbackDI, this, DIEDFL_ATTACHEDONLY); //enum Joysticks
#else
      m_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoystickCallbackDI, this, DIEDFL_ATTACHEDONLY);   //enum Joysticks
#endif
   }
#endif

   // initialize Open Pinball Device HIDs
   InitOpenPinballDevices();

   m_mixerKeyDown = false;
   m_mixerKeyUp = false;
}


void PinInput::UnInit()
{
   m_head = m_tail = 0;

#if defined(ENABLE_SDL_INPUT)
   SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
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
      if (m_pJoystick[k])
      {
         // Unacquire the device one last time just in case 
         // the app tried to exit while the device is still acquired.
         m_pJoystick[k]->Unacquire();
         m_pJoystick[k]->Release();
         m_pJoystick[k] = nullptr;
      }

   // Release any DirectInput objects.
   SAFE_RELEASE(m_pDI);
#endif

   ZeroMemory(m_diq, sizeof(m_diq));
}

void PinInput::FireKeyEvent(const int dispid, int keycode)
{
   // Check if we are mirrored.
   if (g_pplayer->m_ptable->m_tblMirrorEnabled)
   {
      // Swap left & right input.
      if (keycode == g_pplayer->m_rgKeys[eLeftFlipperKey]) keycode = g_pplayer->m_rgKeys[eRightFlipperKey];
      else if (keycode == g_pplayer->m_rgKeys[eRightFlipperKey]) keycode = g_pplayer->m_rgKeys[eLeftFlipperKey];
      else if (keycode == g_pplayer->m_rgKeys[eStagedLeftFlipperKey]) keycode = g_pplayer->m_rgKeys[eStagedRightFlipperKey];
      else if (keycode == g_pplayer->m_rgKeys[eStagedRightFlipperKey]) keycode = g_pplayer->m_rgKeys[eStagedLeftFlipperKey];
      else if (keycode == g_pplayer->m_rgKeys[eLeftMagnaSave]) keycode = g_pplayer->m_rgKeys[eRightMagnaSave];
      else if (keycode == g_pplayer->m_rgKeys[eRightMagnaSave]) keycode = g_pplayer->m_rgKeys[eLeftMagnaSave];
      else if (keycode == DIK_LSHIFT) keycode = DIK_RSHIFT;
      else if (keycode == DIK_RSHIFT) keycode = DIK_LSHIFT;
      else if (keycode == DIK_LEFT)   keycode = DIK_RIGHT;
      else if (keycode == DIK_RIGHT)  keycode = DIK_LEFT;
   }
#ifdef ENABLE_VR
   if (keycode == g_pplayer->m_rgKeys[eTableRecenter] && dispid == DISPID_GameEvents_KeyUp)
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->recenterTable();
   else if (keycode == g_pplayer->m_rgKeys[eTableUp] && dispid == DISPID_GameEvents_KeyUp)
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->tableUp();
   else if (keycode == g_pplayer->m_rgKeys[eTableDown] && dispid == DISPID_GameEvents_KeyUp)
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->tableDown();
   else
#endif

   for (int i = 0; i < eCKeys; i++)
      if (keycode == g_pplayer->m_rgKeys[i])
         if (dispid == DISPID_GameEvents_KeyDown)
            m_keyPressedState[i] = true;
         else if (dispid == DISPID_GameEvents_KeyUp)
            m_keyPressedState[i] = false;
      
   if (g_pplayer->m_liveUI->IsTweakMode())
   {
      if (dispid == DISPID_GameEvents_KeyDown)
         g_pplayer->m_liveUI->OnTweakModeEvent(1, keycode);
      if (dispid == DISPID_GameEvents_KeyUp)
         g_pplayer->m_liveUI->OnTweakModeEvent(2, keycode);
   }
   else
   {
      // Debug only, for testing parts of the left flipper input lag, also release ball control
      if (keycode == g_pplayer->m_rgKeys[eLeftFlipperKey] && dispid == DISPID_GameEvents_KeyDown)
      {
         m_leftkey_down_usec = usec();
         m_leftkey_down_frame = g_pplayer->m_overall_frames;
         delete g_pplayer->m_pBCTarget;
         g_pplayer->m_pBCTarget = nullptr;
      }

      if ((keycode == g_pplayer->m_rgKeys[eLeftFlipperKey] || keycode == g_pplayer->m_rgKeys[eRightFlipperKey] || keycode == g_pplayer->m_rgKeys[eStagedLeftFlipperKey] || keycode == g_pplayer->m_rgKeys[eStagedRightFlipperKey])
          && dispid == DISPID_GameEvents_KeyDown)
         g_pplayer->m_pininput.PlayRumble(0.f, 0.2f, 150);

      // Mixer volume only
      m_mixerKeyDown = (keycode == g_pplayer->m_rgKeys[eVolumeDown] && dispid == DISPID_GameEvents_KeyDown);
      m_mixerKeyUp   = (keycode == g_pplayer->m_rgKeys[eVolumeUp]   && dispid == DISPID_GameEvents_KeyDown);

      g_pplayer->m_ptable->FireKeyEvent(dispid, keycode);
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
      FireKeyEvent(DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eStartGameKey]);

#ifdef _DEBUG
      OutputDebugString("Autostart: Release.\n");
#endif
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
      FireKeyEvent(DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eStartGameKey]);

#ifdef _DEBUG
      OutputDebugString("Autostart: Press.\n");
#endif
   }
}

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

void PinInput::TiltUpdate()
{
   if (!g_pplayer) return;

   const bool tmp = m_tilt_updown;
   m_tilt_updown = plumb_tilted();

   if (m_tilt_updown != tmp)
      FireKeyEvent(m_tilt_updown ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eCenterTiltKey]);
}

void PinInput::ProcessCameraKeys(const DIDEVICEOBJECTDATA * __restrict input)
{
    switch(input->dwOfs)
    {
    case DIK_UP:
    case DIK_DOWN:
        {
            const bool up = (input->dwOfs == DIK_UP);
            if ((input->dwData & 0x80) != 0)
            {
                if (!m_cameraModeAltKey)
                    g_pplayer->m_pin3d.m_cam.y += up ? 10.0f : -10.0f;
                else
                    g_pplayer->m_pin3d.m_cam.z += up ? 10.0f : -10.0f;

                m_cameraMode = up ? 1 : 2;
            }
            else
                m_cameraMode = 0;
        }
        break;
    case DIK_RIGHT:
    case DIK_LEFT:
        {
            const bool right = (input->dwOfs == DIK_RIGHT);
            if ((input->dwData & 0x80) != 0)
            {
                if (!m_cameraModeAltKey)
                    g_pplayer->m_pin3d.m_cam.x += right ? -10.0f : 10.0f;
                else
                    g_pplayer->m_pin3d.m_inc += right ? -0.01f : 0.01f;

                m_cameraMode = right ? 3 : 4;
            }
            else
                m_cameraMode = 0;
        }
        break;
    case DIK_LALT:
        m_cameraModeAltKey = ((input->dwData & 0x80) != 0);
        break;
    default:
        break;
    }
}

void PinInput::Joy(const unsigned int n, const int updown, const bool start)
{
   if (m_joylflipkey == n)      FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftFlipperKey]);
   if (m_joyrflipkey == n)      FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightFlipperKey]);
   if (m_joystagedlflipkey == n)FireKeyEvent(updown, g_pplayer->m_rgKeys[eStagedLeftFlipperKey]);
   if (m_joystagedrflipkey == n)FireKeyEvent(updown, g_pplayer->m_rgKeys[eStagedRightFlipperKey]);
   if (m_joyplungerkey == n)    FireKeyEvent(updown, g_pplayer->m_rgKeys[ePlungerKey]);
   if (m_joyaddcreditkey == n)  FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey]);
   if (m_joyaddcreditkey2 == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey2]);
   if (m_joylmagnasave == n)    FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftMagnaSave]);
   if (m_joyrmagnasave == n)    FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightMagnaSave]);
   if (m_joytablerecenter == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eTableRecenter]);
   if (m_joytableup == n)       FireKeyEvent(updown, g_pplayer->m_rgKeys[eTableUp]);
   if (m_joytabledown == n)     FireKeyEvent(updown, g_pplayer->m_rgKeys[eTableDown]);
   if (m_joystartgamekey == n)
   {
      if (start)
      {
         m_pressed_start = true;
         FireKeyEvent(updown, g_pplayer->m_rgKeys[eStartGameKey]);
      }
   }
   if (m_joyexitgamekey == n)
   {
      if (DISPID_GameEvents_KeyDown == updown)
         g_pplayer->SetCloseState(Player::CS_USER_INPUT);
   }
   if (m_joyframecount == n)
   {
      if (DISPID_GameEvents_KeyDown == updown)
         g_pplayer->m_liveUI->ToggleFPS();
   }
   if (m_joyvolumeup == n)   FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeUp]);
   if (m_joyvolumedown == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeDown]);
   if (m_joylefttilt == n)   FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftTiltKey]);
   if (m_joycentertilt == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eCenterTiltKey]);
   if (m_joyrighttilt == n)  FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightTiltKey]);
   if (m_joymechtilt == n)   FireKeyEvent(updown, g_pplayer->m_rgKeys[eMechanicalTilt]);
   if (m_joydebugballs == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eDBGBalls]);
   if (m_joydebugger == n)   FireKeyEvent(updown, g_pplayer->m_rgKeys[eDebugger]);
   if (m_joylockbar == n)    FireKeyEvent(updown, g_pplayer->m_rgKeys[eLockbarKey]);
   if (m_joypause == n)      FireKeyEvent(updown, g_pplayer->m_rgKeys[ePause]);
   if (m_joytweak == n)      FireKeyEvent(updown, g_pplayer->m_rgKeys[eTweak]);
   if (m_joycustom1 == n)    FireKeyEvent(updown, m_joycustom1key);
   if (m_joycustom2 == n)    FireKeyEvent(updown, m_joycustom2key);
   if (m_joycustom3 == n)    FireKeyEvent(updown, m_joycustom3key);
   if (m_joycustom4 == n)    FireKeyEvent(updown, m_joycustom4key);
   if (m_joypmbuyin == n)    FireKeyEvent(updown, DIK_2);
   if (m_joypmcoin3 == n)    FireKeyEvent(updown, DIK_5);
   if (m_joypmcoin4 == n)    FireKeyEvent(updown, DIK_6);
   if (m_joypmcoindoor == n) FireKeyEvent(updown, DIK_END);
   if (m_joypmcancel == n)   FireKeyEvent(updown, DIK_7);
   if (m_joypmdown == n)     FireKeyEvent(updown, DIK_8);
   if (m_joypmup == n)       FireKeyEvent(updown, DIK_9);
   if (m_joypmenter == n)    FireKeyEvent(updown, DIK_0);
}

void PinInput::ProcessBallControl(const DIDEVICEOBJECTDATA * __restrict input)
{
	if (input->dwData == 1 || input->dwData == 3 || input->dwData == 4)
	{
		POINT point = { m_mouseX, m_mouseY };
		ScreenToClient(m_hwnd, &point);
		delete g_pplayer->m_pBCTarget;
		g_pplayer->m_pBCTarget = new Vertex3Ds(g_pplayer->m_pin3d.Get3DPointFrom2D(point));
		if (input->dwData == 1 || input->dwData == 3)
		{
			const uint64_t cur = usec();
			if (m_lastclick_ballcontrol_usec + BALLCONTROL_DOUBLECLICK_THRESHOLD_USEC > cur)
			{
				// Double click.  Move the ball directly to the target if possible.   Drop 
				// it fast from the glass height, so it will appear over any object (or on a raised playfield)

				Ball * const pBall = g_pplayer->m_pactiveballBC;
				if (pBall && !pBall->m_d.m_lockedInKicker)
				{
					pBall->m_d.m_pos.x = g_pplayer->m_pBCTarget->x;
					pBall->m_d.m_pos.y = g_pplayer->m_pBCTarget->y;
					pBall->m_d.m_pos.z = g_pplayer->m_ptable->m_glassTopHeight;

					pBall->m_d.m_vel.x = 0.0f;
					pBall->m_d.m_vel.y = 0.0f;
					pBall->m_d.m_vel.z = -1000.0f;
				}
			}
			m_lastclick_ballcontrol_usec = cur;
		}
	}
}

void PinInput::ProcessThrowBalls(const DIDEVICEOBJECTDATA * __restrict input)
{
   if (input->dwData == 1 || input->dwData == 3)
   {
      POINT point = { m_mouseX, m_mouseY };
      ScreenToClient(m_hwnd, &point);
      const Vertex3Ds vertex = g_pplayer->m_pin3d.Get3DPointFrom2D(point);

      float vx = (float)m_mouseDX*0.1f;
      float vy = (float)m_mouseDY*0.1f;
      const float radangle = ANGTORAD(g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].mViewportRotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);
      const float vx2 = cs*vx - sn*vy;
      const float vy2 = sn*vx + cs*vy;
      vx = -vx2;
      vy = -vy2;

		POINT newPoint;
		GetCursorPos(&newPoint);
		ScreenToClient(m_hwnd, &newPoint);
		const Vertex3Ds vert = g_pplayer->m_pin3d.Get3DPointFrom2D(newPoint);

		if (g_pplayer->m_ballControl)
		{
			// If Ball Control and Throw Balls are both checked, that means
			// we want ball throwing behavior with the sensed active ball, instead
			// of creating new ones 
			Ball * const pBall = g_pplayer->m_pactiveballBC;
			if (pBall)
			{
				pBall->m_d.m_pos.x = vert.x;
				pBall->m_d.m_pos.y = vert.y;
				pBall->m_d.m_vel.x = vx;
				pBall->m_d.m_vel.y = vy;
			}
		}
		else
		{
			bool ballGrabbed = false;
			if (input->dwData == 1)
			{
				for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
				{
					Ball * const pBall = g_pplayer->m_vball[i];
					const float dx = fabsf(vertex.x - pBall->m_d.m_pos.x);
					const float dy = fabsf(vertex.y - pBall->m_d.m_pos.y);
					if (dx < pBall->m_d.m_radius*2.f && dy < pBall->m_d.m_radius*2.f)
					{
						ballGrabbed = true;
						pBall->m_d.m_pos.x = vert.x;
						pBall->m_d.m_pos.y = vert.y;
						pBall->m_d.m_vel.x = vx;
						pBall->m_d.m_vel.y = vy;
						pBall->Init(1.f);
						break;
					}
				}
			}
			if (!ballGrabbed)
			{
				const float z = (input->dwData == 3) ? g_pplayer->m_ptable->m_glassTopHeight : 0.f;
				Ball * const pball = g_pplayer->CreateBall(vertex.x, vertex.y, z, vx, vy, 0, (float)g_pplayer->m_debugBallSize*0.5f, g_pplayer->m_debugBallMass);
				pball->m_pballex->AddRef();
			}
		}
    }
    else if (input->dwData == 2)
    {
        POINT point = { m_mouseX, m_mouseY };
        ScreenToClient(m_hwnd, &point);
        const Vertex3Ds vertex = g_pplayer->m_pin3d.Get3DPointFrom2D(point);

        for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
        {
            Ball * const pBall = g_pplayer->m_vball[i];
            const float dx = fabsf(vertex.x - pBall->m_d.m_pos.x);
            const float dy = fabsf(vertex.y - pBall->m_d.m_pos.y);
            if (dx < pBall->m_d.m_radius*2.f && dy < pBall->m_d.m_radius*2.f)
            {
                g_pplayer->DestroyBall(pBall);
                break;
            }
        }
    }
}

void PinInput::ProcessJoystick(const DIDEVICEOBJECTDATA * __restrict input, int curr_time_msec)
{
    const int joyk = input->dwSequence - APP_JOYSTICKMN; // joystick index
    static constexpr bool rotLeftManual = false; //!! delete

    if (input->dwOfs >= DIJOFS_BUTTON0 && input->dwOfs <= DIJOFS_BUTTON31)
    {
        const int updown = (input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;
        const bool start = ((curr_time_msec - m_firedautostart) > g_pplayer->m_ptable->m_tblAutoStart) || m_pressed_start || Started();
        if (input->dwOfs == DIJOFS_BUTTON0)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // plunge
                FireKeyEvent(updown, g_pplayer->m_rgKeys[ePlungerKey]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // coin 1
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey]);
            else
                Joy(1, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON1)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // right
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightFlipperKey]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // coin 2
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey2]);
            else
                Joy(2, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON2)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_ULTRACADE) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons)
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightMagnaSave]); // right2
            else
                Joy(3, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON3)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // volume down
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeDown]);
            else
                Joy(4, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON4)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // volume up
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeUp]);
            else
                Joy(5, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON5)
        {
            if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // volume up
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeUp]);
            else
                Joy(6, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON6)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // pause menu
            {
               if (DISPID_GameEvents_KeyDown == updown)
                  g_pplayer->SetCloseState(Player::CS_USER_INPUT);
            }
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // volume down
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeDown]);
            else
                Joy(7, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON7)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons && !m_disable_esc) // exit
            {   // Check if we have started a game yet.
                if (Started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled)
                {
                    if (DISPID_GameEvents_KeyDown == updown)
                    {
                        m_first_stamp = curr_time_msec;
                        m_exit_stamp = curr_time_msec;
                        FireKeyEvent(DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eExitGame]);
                    }
                    else
                    {
                        FireKeyEvent(DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eExitGame]);
                        m_exit_stamp = 0;
                    }
                }
            }
            else
                Joy(8, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON8)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons)
            {
                if (start)
                {
                    m_pressed_start = true;
                    FireKeyEvent(updown, g_pplayer->m_rgKeys[eStartGameKey]);
                }
            }
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // left
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftFlipperKey]);
            else
                Joy(9, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON9)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // left
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftFlipperKey]);
            else
                Joy(10, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON10)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // left 2
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftMagnaSave]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // right
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightFlipperKey]);
            else
                Joy(11, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON11)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // coin 1
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey]);
            else
                Joy(12, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON12)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && !m_override_default_buttons) // coin 2
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey2]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // start
            { // Check if we can allow the start (table is done initializing).
                if (start)
                {
                    m_pressed_start = true;
                    FireKeyEvent(updown, g_pplayer->m_rgKeys[eStartGameKey]);
                }
            }
            else
                Joy(13, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON13)
        {
            if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // plunge
                FireKeyEvent(updown, g_pplayer->m_rgKeys[ePlungerKey]);
            else
                Joy(14, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON14)
        {
            if ((uShockType == USHOCKTYPE_ULTRACADE) && !m_override_default_buttons) // exit
            {
                if (Started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled) // Check if we have started a game yet.
                {
                    if (DISPID_GameEvents_KeyDown == updown)
                    {
                        m_first_stamp = curr_time_msec;
                        m_exit_stamp = curr_time_msec;
                        FireKeyEvent(DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eExitGame]);
                    }
                    else
                    {
                        FireKeyEvent(DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eExitGame]);
                        m_exit_stamp = 0;
                    }
                }
            }
            else
                Joy(15, updown, start);
        }
        else if (input->dwOfs >= DIJOFS_BUTTON15 && input->dwOfs <= DIJOFS_BUTTON31)
        {
            Joy(16 + input->dwOfs-DIJOFS_BUTTON15, updown, start);
        }
        else
            FireKeyEvent(updown, input->dwOfs | 0x01000000); // unknown button events
    }
    else //end joy buttons
    {
        // Axis Deadzone
        int deadu = (int)input->dwData;
        if (((deadu <= 0) && (deadu >= -m_deadz)) || ((deadu >= 0) && (deadu <= m_deadz)))
            deadu = 0;
        if ((deadu < 0) && (deadu < -m_deadz))
            deadu += m_deadz;
        if ((deadu > 0) && (deadu>m_deadz))
            deadu -= m_deadz;

        switch (input->dwOfs)	// Axis, Sliders and POV
        {   // with selectable axes added to menu, giving priority in this order... X Axis (the Left/Right Axis), Y Axis
            case DIJOFS_X:
            {
                if (g_pplayer) //joyk  rotLeftManual
                {
                    if ((m_lr_axis == 1) || (m_ud_axis == 1) || (uShockType != USHOCKTYPE_GENERIC))
                    { // Check if L/R Axis or U/D Axis is selected (in the case of the Generic controller),
                        // or non Generic controllers are being used...
                        // Axis Deadzone
                        if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_lr_axis != 0))
                            g_pplayer->NudgeX(-deadu, joyk); //rotate to match Pinball Wizard
                        if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_lr_axis != 0))
                        {
                            if (rotLeftManual)
                                g_pplayer->NudgeX(deadu, joyk);
                            else
                                g_pplayer->NudgeY(-deadu, joyk); //rotate to match joystick
                        }
                        if ((uShockType == USHOCKTYPE_SIDEWINDER) && (m_lr_axis != 0))
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? deadu : -deadu, joyk);
                        if ((m_lr_axis == 1) && (uShockType == USHOCKTYPE_GENERIC))
                            // giving L/R Axis priority over U/D Axis in case both are assigned to same axis
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if ((m_ud_axis == 1) && (uShockType == USHOCKTYPE_GENERIC))
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 1)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis and (uShockType == USHOCKTYPE_GENERIC)
                        g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 1)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_Y:
            {
                if (g_pplayer)
                {
                    if ((m_lr_axis == 2) || (m_ud_axis == 2) || (uShockType != USHOCKTYPE_GENERIC))
                    { // Check if L/R Axis or U/D Axis is selected (in the case of the Generic controller),
                        // or non Generic controllers are being used...
                        // Axis Deadzone
                        if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_ud_axis != 0))
                            g_pplayer->NudgeY(deadu, joyk); //rotate to match Pinball Wizard
                        if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_ud_axis != 0))
                        {
                            if (rotLeftManual)
                                g_pplayer->NudgeY(deadu, joyk);
                            else
                                g_pplayer->NudgeX(-deadu, joyk); //rotate to match joystick
                        }
                        if ((uShockType == USHOCKTYPE_SIDEWINDER) && (m_ud_axis != 0))
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                        if ((m_lr_axis == 2) && (uShockType == USHOCKTYPE_GENERIC))
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if ((m_ud_axis == 2) && (uShockType == USHOCKTYPE_GENERIC))
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 2)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis and (uShockType == USHOCKTYPE_GENERIC)
                        g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 2)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_Z:
            {
                if (g_pplayer)
                {
                    if (uShockType == USHOCKTYPE_ULTRACADE)
                        g_pplayer->MechPlungerIn((int)input->dwData, joyk);
                    if (((m_plunger_axis != 6) && (m_plunger_axis != 0)) || !m_override_default_buttons)
                    {                                          // with the ability to use rZ for plunger, checks to see if
                        if (uShockType == USHOCKTYPE_PBWIZARD) // the override is used and if so, if Plunger is set to Rz or
                        {                                      // disabled. If override isn't used, uses default assignment
                            g_pplayer->MechPlungerIn(-(int)input->dwData, joyk); // of the Z axis.
                        }
                    }
                    if ((uShockType == USHOCKTYPE_VIRTUAPIN) && (m_plunger_axis != 0))
                        g_pplayer->MechPlungerIn(-(int)input->dwData, joyk);
                    if (((m_lr_axis == 3) || (m_ud_axis == 3)) && (uShockType == USHOCKTYPE_GENERIC))
                    {   // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 3)
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 3)
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 3)
                    {   // if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 3)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_RX:
            {
                if (g_pplayer)
                {
                    if (((m_lr_axis == 4) || (m_ud_axis == 4)) && (uShockType == USHOCKTYPE_GENERIC))
                    {   // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 4)
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 4)
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 4)
                    {   // if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 4)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_RY:
            {
                if (g_pplayer)
                {
                    if (((m_lr_axis == 5) || (m_ud_axis == 5)) && (uShockType == USHOCKTYPE_GENERIC))
                    {   // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 5)
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 5)
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 5)
                    {   // if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 5)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_RZ:
            {
                if (g_pplayer)
                {
                    if ((uShockType == USHOCKTYPE_PBWIZARD) && m_override_default_buttons && (m_plunger_axis == 6))
                        g_pplayer->MechPlungerIn((int)input->dwData, joyk);
                    if (((m_lr_axis == 6) || (m_ud_axis == 6)) && (uShockType == USHOCKTYPE_GENERIC))
                    {   // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 6)
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 6)
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 6)
                    {
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 6)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_SLIDER(0):
            {
                if (g_pplayer)
                {
                    if (uShockType == USHOCKTYPE_SIDEWINDER)
                        g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    if (((m_lr_axis == 7) || (m_ud_axis == 7)) && (uShockType == USHOCKTYPE_GENERIC))
                    {   // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 7)
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 7)
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 7)
                    {
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 7)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_SLIDER(1):
            {
                if (g_pplayer)
                {
                    if (((m_lr_axis == 8) || (m_ud_axis == 8)) && (uShockType == USHOCKTYPE_GENERIC))
                    {   // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 8)
                            g_pplayer->NudgeX(!m_lr_axis_reverse ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 8)
                            g_pplayer->NudgeY(!m_ud_axis_reverse ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 8)
                    {
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->MechPlungerIn(!m_plunger_reverse ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                    else if (m_plunger_speed_axis == 8)
                    {
                       // not nudge X/Y or plunger
                       if (uShockType == USHOCKTYPE_GENERIC)
                          g_pplayer->MechPlungerSpeedIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData, joyk);
                    }
                }
                break;
            }

            case DIJOFS_POV(0):
            default:
                break;
        }
    }
}


void PinInput::ProcessKeys(/*const U32 curr_sim_msec,*/ int curr_time_msec) // last one is negative if only key events should be fired
{
   if (!g_pplayer || !g_pplayer->m_ptable) return; // only if player is running
   g_frameProfiler.OnProcessInput();

   if (curr_time_msec >= 0)
   {
      // Check if autostart is enabled.
      if (g_pplayer->m_ptable->m_tblAutoStartEnabled)
         // Update autostart.
         Autostart(g_pplayer->m_ptable->m_tblAutoStart, g_pplayer->m_ptable->m_tblAutoStartRetry, curr_time_msec);

      ButtonExit(g_pplayer->m_ptable->m_tblExitConfirm, curr_time_msec);

      // Update tilt.
      TiltUpdate();
   }
   else
      curr_time_msec = -curr_time_msec; // due to special encoding to not do the stuff above

   // Check if we've been initialized.
   if (m_firedautostart == 0)
      m_firedautostart = curr_time_msec;

   GetInputDeviceData(/*curr_time_msec*/);

   ReadOpenPinballDevices(curr_time_msec);

   // Camera/Light tweaking mode (F6) incl. fly-around parameters
   if (g_pplayer->m_liveUI->IsTweakMode())
   {
      if (m_head == m_tail) // key queue empty, so just continue using the old pressed key
      {
         if ((curr_time_msec - m_nextKeyPressedTime) > 10) // reduce update rate
         {
            m_nextKeyPressedTime = curr_time_msec;

            // Flying
            if (m_cameraMode == 1)
            {
			      if (!m_cameraModeAltKey)
			         g_pplayer->m_pin3d.m_cam.y += 10.0f;
			      else
			         g_pplayer->m_pin3d.m_cam.z += 10.0f;
            }
            else if (m_cameraMode == 2)
            {
			      if (!m_cameraModeAltKey)
			         g_pplayer->m_pin3d.m_cam.y -= 10.0f;
			      else
			         g_pplayer->m_pin3d.m_cam.z -= 10.0f;
            }
            else if (m_cameraMode == 3)
            {
			      if (!m_cameraModeAltKey)
			         g_pplayer->m_pin3d.m_cam.x -= 10.0f;
			      else
			         g_pplayer->m_pin3d.m_inc -= 0.01f;
            }
            else if (m_cameraMode == 4)
            {
			      if (!m_cameraModeAltKey)
			         g_pplayer->m_pin3d.m_cam.x += 10.0f;
			      else
			         g_pplayer->m_pin3d.m_inc += 0.01f;
            }

            // Table tweaks, continuous actions
            if (g_pplayer->m_liveUI->IsTweakMode())
               for (int i = 0; i < eCKeys; i++)
                  if (m_keyPressedState[i])
                     g_pplayer->m_liveUI->OnTweakModeEvent(0, g_pplayer->m_rgKeys[i]);
         }
         return;
      }
   }

   const DIDEVICEOBJECTDATA * __restrict input;
   while ((input = GetTail(/*curr_sim_msec*/)))
   {
      if (input->dwSequence == APP_MOUSE && g_pplayer && !g_pplayer->m_liveUI->HasMouseCapture())
      {
         if (g_pplayer->m_throwBalls)
            ProcessThrowBalls(input);
         else if (g_pplayer->m_ballControl)
            ProcessBallControl(input);
         else
         {
            if (input->dwOfs == 1)
            {
               if (m_joylflipkey == m_LeftMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftFlipperKey]);
               else if (m_joyrflipkey == m_LeftMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightFlipperKey]);
               else if (m_joyplungerkey == m_LeftMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[ePlungerKey]);
               else if (m_joylefttilt == m_LeftMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftTiltKey]);
               else if (m_joyrighttilt == m_LeftMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightTiltKey]);
               else if (m_joycentertilt == m_LeftMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eCenterTiltKey]);
            }
            if (input->dwOfs == 2)
            {
               if (m_joylflipkey == m_RightMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftFlipperKey]);
               else if (m_joyrflipkey == m_RightMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightFlipperKey]);
               else if (m_joyplungerkey == m_RightMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[ePlungerKey]);
               else if (m_joylefttilt == m_RightMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftTiltKey]);
               else if (m_joyrighttilt == m_RightMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightTiltKey]);
               else if (m_joycentertilt == m_RightMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eCenterTiltKey]);
            }
            if (input->dwOfs == 3)
            {
               if (m_joylflipkey == m_MiddleMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftFlipperKey]);
               else if (m_joyrflipkey == m_MiddleMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightFlipperKey]);
               else if (m_joyplungerkey == m_MiddleMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[ePlungerKey]);
               else if (m_joylefttilt == m_MiddleMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftTiltKey]);
               else if (m_joyrighttilt == m_MiddleMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightTiltKey]);
               else if (m_joycentertilt == m_MiddleMouseButtonID)
                  FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eCenterTiltKey]);
            }
         }
      }

      if (input->dwSequence == APP_KEYBOARD && (g_pplayer == nullptr || !g_pplayer->m_liveUI->HasKeyboardCapture()))
      {
         // Camera mode fly around:
         if (g_pplayer && g_pplayer->m_liveUI->IsTweakMode() && m_enableCameraModeFlyAround)
            ProcessCameraKeys(input);

         // Normal game keys:
         if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eFrameCount])
         {
            if ((input->dwData & 0x80) != 0)
               g_pplayer->m_liveUI->ToggleFPS();
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[ePause])
         {
            if ((input->dwData & 0x80) != 0)
               g_pplayer->m_liveUI->PausePlayer(!g_pplayer->m_debugWindowActive);
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eTweak])
         {
            if ((input->dwData & 0x80) != 0)
            {
               if (g_pplayer->m_liveUI->IsTweakMode())
                  g_pplayer->m_liveUI->HideUI();
               else
                  g_pplayer->m_liveUI->OpenTweakMode();
            }
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eEnable3D])
         {
            if ((input->dwData & 0x80) != 0)
            {
               if (IsAnaglyphStereoMode(g_pplayer->m_stereo3D))
               {
                  // Select next glasses or toggle stereo on/off
                  int glassesIndex = g_pplayer->m_stereo3D - STEREO_ANAGLYPH_1;
                  if (!g_pplayer->m_stereo3Denabled && glassesIndex != 0)
                  {
                     g_pplayer->m_liveUI->PushNotification("Stereo enabled"s, 2000);
                     g_pplayer->m_stereo3Denabled = true;
                  }
                  else
                  {
                     const int dir = (m_keyPressedState[eLeftFlipperKey] || m_keyPressedState[eRightFlipperKey]) ? -1 : 1;
                     // Loop back with shift pressed
                     if (!g_pplayer->m_stereo3Denabled && glassesIndex <= 0 && dir == -1)
                     {
                        g_pplayer->m_stereo3Denabled = true;
                        glassesIndex = 9;
                     }
                     else if (g_pplayer->m_stereo3Denabled && glassesIndex <= 0 && dir == -1)
                     {
                        g_pplayer->m_liveUI->PushNotification("Stereo disabled"s, 2000);
                        g_pplayer->m_stereo3Denabled = false;
                     }
                     // Loop forward
                     else if (!g_pplayer->m_stereo3Denabled)
                     {
                        g_pplayer->m_liveUI->PushNotification("Stereo enabled"s, 2000);
                        g_pplayer->m_stereo3Denabled = true;
                     }
                     else if (glassesIndex >= 9 && dir == 1)
                     {
                        g_pplayer->m_liveUI->PushNotification("Stereo disabled"s, 2000);
                        glassesIndex = 0;
                        g_pplayer->m_stereo3Denabled = false;
                     }
                     else
                     {
                        glassesIndex += dir;
                     }
                     g_pplayer->m_stereo3D = (StereoMode)(STEREO_ANAGLYPH_1 + glassesIndex);
                     if (g_pplayer->m_stereo3Denabled)
                     {
                        string name;
                        static const string defaultNames[] = { "Red/Cyan"s, "Green/Magenta"s, "Blue/Amber"s, "Cyan/Red"s, "Magenta/Green"s, "Amber/Blue"s, "Custom 1"s, "Custom 2"s, "Custom 3"s, "Custom 4"s };
                        if (!g_pvp->m_settings.LoadValue(Settings::Player, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Name"s), name))
                           name = defaultNames[glassesIndex];
                        g_pplayer->m_liveUI->PushNotification("Profile #"s.append(std::to_string(glassesIndex + 1)).append(" '"s).append(name).append("' activated"s), 2000);
                     }
                  }
               }
               else if (Is3DTVStereoMode(g_pplayer->m_stereo3D))
               {
                  // Toggle stereo on/off
                  g_pplayer->m_stereo3Denabled = !g_pplayer->m_stereo3Denabled;
               }
               else if (g_pplayer->m_stereo3D == STEREO_VR)
               {
                  g_pplayer->m_vrPreview = (VRPreviewMode)((g_pplayer->m_vrPreview + 1) % (VRPREVIEW_BOTH + 1));
                  g_pplayer->m_liveUI->PushNotification(g_pplayer->m_vrPreview == VRPREVIEW_DISABLED ? "Preview disabled"s // Will only display in headset
                                                      : g_pplayer->m_vrPreview == VRPREVIEW_LEFT     ? "Preview switched to left eye"s
                                                      : g_pplayer->m_vrPreview == VRPREVIEW_RIGHT    ? "Preview switched to right eye"s
                                                                                                     : "Preview switched to both eyes"s, 2000);
               }
               g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DEnabled"s, g_pplayer->m_stereo3Denabled);
               g_pplayer->m_pin3d.InitLayout();
               g_pplayer->UpdateStereoShaderState();
            }
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eDBGBalls])
         {
            if ((input->dwData & 0x80) != 0)
               g_pplayer->m_debugBalls = !g_pplayer->m_debugBalls;
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eDebugger])
         {
             if (Started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled)
             {
                 if ((input->dwData & 0x80) != 0)
                 { //on key down only
                     m_first_stamp = curr_time_msec;
                     m_exit_stamp = curr_time_msec;
                 }
                 else
                 { //on key up only
                     m_exit_stamp = 0;
                     g_pplayer->m_showDebugger = true;
                 }
             }
         }
         else if (((input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eEscape]) && !m_disable_esc) || (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eExitGame]))
         {
            // Check if we have started a game yet, and do not trigger if the UI is already opened (keyboard is handled in it)
            if (!g_pplayer->m_liveUI->IsOpened() && (Started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled))
            {
               if ((input->dwData & 0x80) != 0)
               {  //on key down only
                  m_first_stamp = curr_time_msec;
                  m_exit_stamp = curr_time_msec;
               }
               else
               {  //on key up only
                  // Open UI on key up since a long press should not trigger the UI (direct exit from the app)
                  g_pplayer->SetCloseState(Player::CS_USER_INPUT);
                  m_exit_stamp = 0;
               }
            }
         }
         else if ((input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eStartGameKey]) && m_keyPressedState[eLockbarKey])
         {
            if (((input->dwData & 0x80) != 0) && g_pvp->m_ptableActive->TournamentModePossible())
               g_pvp->GenerateTournamentFile();
         }
         else
            FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, input->dwOfs);
      }
      else if (input->dwSequence >= APP_JOYSTICKMN && input->dwSequence <= APP_JOYSTICKMX)
          ProcessJoystick(input, curr_time_msec);
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
      const SHORT keyState = GetAsyncKeyState(i);
      if (keyState & 1)
      {
         const unsigned int dik = get_dik(i);
         if (dik != ~0u)
            return dik;
      }
   }
#endif

   return 0;
}



// ---------------------------------------------------------------------------
//
// Open Pinball Device support
//

// we require direct access to the Windows HID APIs
extern "C"
{
#include "SetupAPI.h"
#include "hidsdi.h"
#include "hidpi.h"
}

// Initialize the Open Pinball Device interface.  Searches for active
// devices and adds them to our internal list.
void PinInput::InitOpenPinballDevices()
{
   // initialize a Device Set with all currently connected HID devices
   GUID hidGuid;
   HidD_GetHidGuid(&hidGuid);
   HDEVINFO hdi = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
   if (hdi == INVALID_HANDLE_VALUE)
      return;

   // iterate over the Device Set members
   SP_DEVICE_INTERFACE_DATA did { sizeof(SP_DEVICE_INTERFACE_DATA) };
   for (DWORD memberIndex = 0; SetupDiEnumDeviceInterfaces(hdi, NULL, &hidGuid, memberIndex, &did); ++memberIndex)
   {
      // retrieve the buffer size needed for device detail
      DWORD diDetailSize = 0;
      DWORD err = 0;
      if (!SetupDiGetDeviceInterfaceDetail(hdi, &did, NULL, 0, &diDetailSize, NULL) && (err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
         break;

      // retrieve the device detail and devinfo data
      std::unique_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA> diDetail(reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA *>(new BYTE[diDetailSize]));
      diDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      SP_DEVINFO_DATA devInfo { sizeof(SP_DEVINFO_DATA) };
      if (!SetupDiGetDeviceInterfaceDetail(hdi, &did, diDetail.get(), diDetailSize, NULL, &devInfo))
         break;

      // open the device desc to access the HID
      HANDLE dev = CreateFile(diDetail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
      if (dev != INVALID_HANDLE_VALUE)
      {
         // get the preparsed HID data, for details about the report format
         PHIDP_PREPARSED_DATA ppd;
         if (HidD_GetPreparsedData(dev, &ppd))
         {
            // check for a generic Pinball Device usage (usage page 0x05 "Game
            // Controls", usage 0x02 "Pinball Device")
            HIDP_CAPS caps;
            constexpr USAGE USAGE_PAGE_GAMECONTROLS = 0x05;
            constexpr USAGE USAGE_GAMECONTROLS_PINBALLDEVICE = 0x02;
            if (HidP_GetCaps(ppd, &caps) == HIDP_STATUS_SUCCESS && caps.UsagePage == USAGE_PAGE_GAMECONTROLS && caps.Usage == USAGE_GAMECONTROLS_PINBALLDEVICE)
            {
               // It's at least a generic Pinball Device.  Check if it's
               // specifically an Open Pinball Device, by checking for the
               // custom string descriptor on the first byte array usage.
               // Windows idiosyncratically represents opaque byte array
               // usages as "buttons", and Open Pinball Device has exactly
               // one such "button" array.
               std::vector<HIDP_BUTTON_CAPS> btnCaps;
               btnCaps.resize(caps.NumberInputButtonCaps);
               USHORT nBtnCaps = caps.NumberInputButtonCaps;
               if (HidP_GetButtonCaps(HIDP_REPORT_TYPE::HidP_Input, btnCaps.data(), &nBtnCaps, ppd) == HIDP_STATUS_SUCCESS && nBtnCaps == 1)
               {
                  // check for a string descriptor attached to the usage
                  USHORT stringIndex = btnCaps[0].NotRange.StringIndex;
                  WCHAR str[128] { 0 };
                  if (stringIndex != 0 && HidD_GetIndexedString(dev, stringIndex, str, sizeof(str)) && wcsncmp(str, L"OpenPinballDeviceStruct/", 24) == 0)
                  {
                     // matched - add it to the active device list
                     m_openPinDevs.emplace_back(dev, btnCaps[0].ReportID, caps.InputReportByteLength, &str[24]);

                     // the device list entry owns the handle now - forget it locally
                     dev = INVALID_HANDLE_VALUE;
                  }
               }

               // done with the preparsed data
               HidD_FreePreparsedData(ppd);
            }
         }

         // close the device handle, if we didn't transfer ownership to the device list
         if (dev != INVALID_HANDLE_VALUE)
            CloseHandle(dev);
      }
   }

   // done with the device list dev
   SetupDiDestroyDeviceInfoList(hdi);
}

// Read input from the Open Pinball Device inputs
void PinInput::ReadOpenPinballDevices(const U32 cur_time_msec)
{
   // Combined report.  In keeping with Visual Pinball's treatment of
   // multiple gamepads, we merge the input across devices if there are
   // multiple Pinball Devices sending us data.
   OpenPinballDeviceReport cr { 0 };

   // read input from each device
   for (auto &p : m_openPinDevs)
   {
      // check for a new report
      if (!p.ReadReport())
         continue;

      // Merge the data into the combined struct.  For the accelerometer
      // and plunger analog quantities, just arbitrarily pick the last
      // input that's sending non-zero values.  Devices that don't have
      // those sensors attached will send zeroes, so this strategy yields
      // sensible results for the sensible case where the user only has
      // one plunger and one accelerometer, but they're attached to
      // separate Open Pinball Device microcontrollers.  If the user has
      // multiple accelerometers in the system, our merge strategy will
      // arbitrarily pick whichever one enumerated last, which isn't
      // necessarily a sensible result, but that seems fair enough
      // because the user's actual configuration isn't sensible either.
      // I mean, what do they expect us to do with two accelerometer
      // inputs?  Note that this is a different situation from the
      // traditional multiple-joysticks case, because in the case of
      // joysticks, there are plenty of good reasons to have more than
      // one attached.  One might be set up as a pinball device, and two
      // more *actual joysticks* might be present as well because the
      // user also plays some non-pinball video games.  Joysticks are
      // generic: we can't tell from the HID descriptor if it's an
      // accelerometer pretending to be a joystick, vs an actual
      // joystick.  The Pinball Device definition doesn't suffer from
      // that ambiguity.  We can be sure that an accelerometer there
      // is an accelerometer, so there aren't any valid use cases where
      // you'd have two or more of them.
      //
      // Merge the buttons by ORing all of the button masks together.
      // If the user has configured the same button number on more than
      // one device, they probably actually want the buttons to perform
      // the same logical function, so ORing them yields the right result.
      //
      // Treat the high-time-resolution flipper buttons like the other
      // analog quantities, since these are more like the analog
      // quantities than like the other buttons.  As with the plunger
      // and accelerometer, it's hard to imagine a sensible use case with
      // multiple devices claiming the same flipper button.
      auto &r = p.r;
      if (r.axNudge != 0)
         cr.axNudge = r.axNudge;
      if (r.ayNudge != 0)
         cr.ayNudge = r.ayNudge;
      if (r.vxNudge != 0)
         cr.vxNudge = r.vxNudge;
      if (r.vyNudge != 0)
         cr.vyNudge = r.vyNudge;
      if (r.plungerPos != 0)
         cr.plungerPos = r.plungerPos;
      if (r.plungerSpeed != 0)
         cr.plungerSpeed = r.plungerSpeed;
      if (r.llFlipper != 0)
         cr.llFlipper = r.llFlipper;
      if (r.lrFlipper != 0)
         cr.lrFlipper = r.lrFlipper;
      if (r.ulFlipper != 0)
         cr.ulFlipper = r.ulFlipper;
      if (r.urFlipper != 0)
         cr.urFlipper = r.urFlipper;
      cr.genericButtons |= r.genericButtons;
      cr.pinballButtons |= r.pinballButtons;
   }

   // Axis scaling factor.  All Open Pinball Device analog axes are
   // INT16's (-32768..+32767).  The VP functional axes are designed
   // for joystick input, so we must rescale to VP's joystick scale.
   constexpr int scaleFactor = (JOYRANGEMX - JOYRANGEMN) / 65536;

   // Process the analog axis inputs.  Each VP functional axis has a
   // Keys dialog mapping to a joystick or OpenPinDev axis.  Axes 1-8
   // are the generic joystick axes (X, Y, Z, RX, RY, RZ, Slider1,
   // Slider2, respectively).  Axis 9 is the OpenPinDev device, and
   // maps to the OpenPinDev input that corresponds to the same VP
   // functional axis.  For example, m_lr_axis is the VP functional
   // axis for Nudge X, so if m_lr_axis == 9, it maps to the Open Pin
   // Dev Nudge X axis.
   //
   // For passing the input to the player, we can simply pretend that
   // we're joystick #0.  A function assigned to OpenPinDev can't also
   // be assigned to a joystick axis, so there won't be any conflicting
   // input from an actual joystick to the same function, hence we can
   // take on the role of the joystick.  We should probably do a little
   // variable renaming in a few places to clarify that the "joystick"
   // number is more like a "device number", which can be a joystick if
   // joysticks are assigned or a PinDev if PinDevs are assigned.
   if (m_lr_axis == 9)
   {
      // Nudge X input - use velocity or acceleration input, according to the user preferences
      int val = (g_pplayer->m_accelInputIsVelocity ? cr.vxNudge : cr.axNudge) * scaleFactor;
      g_pplayer->NudgeX(m_lr_axis_reverse == 0 ? -val : val, 0);
   }
   if (m_ud_axis == 9)
   {
      // Nudge Y input - use velocity or acceleration input, according to the user preferences
      int val = (g_pplayer->m_accelInputIsVelocity ? cr.vyNudge : cr.ayNudge) * scaleFactor;
      g_pplayer->NudgeY(m_ud_axis_reverse == 0 ? -val : val, 0);
   }
   if (m_plunger_axis == 9)
   {
      // Plunger position input
      int val = cr.plungerPos * scaleFactor;
      g_pplayer->MechPlungerIn(m_plunger_reverse == 0 ? -val : val, 0);
   }
   if (m_plunger_speed_axis == 9)
   {
      // Plunger speed input
      int val = cr.plungerSpeed * scaleFactor;
      g_pplayer->MechPlungerSpeedIn(m_plunger_reverse == 0 ? -val : val, 0);
   }

   // Weird special logic for the Start button, to handle auto-start timers,
   // per the regular joystick button input processor
   const bool start = ((cur_time_msec - m_firedautostart) > g_pplayer->m_ptable->m_tblAutoStart) || m_pressed_start || Started();

   // Check for button state changes.  If there are any, generate
   // key up/down events for the button changes.
   if (cr.genericButtons != m_openPinDev_generic_buttons)
   {
      // Visit each button.  VP's internal joystick buttons are
      // numbered #1 to #32.
      for (int buttonNum = 1, bit = 1; buttonNum <= 32; ++buttonNum, bit <<= 1)
      {
         // check for a state change
         DISPID isDown = (cr.genericButtons & bit) != 0 ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;
         DISPID wasDown = (m_openPinDev_generic_buttons & bit) != 0 ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;
         if (isDown != wasDown)
            Joy(buttonNum, isDown, start);
      }

      // remember the new button state
      cr.genericButtons = m_openPinDev_generic_buttons;
   }
   if (cr.pinballButtons != m_openPinDev_pinball_buttons)
   {
      // mapping from Open Pinball Device mask bits to VP/VPM keys
      static const struct KeyMap
      {
         uint32_t mask; // bit for the key in OpenPinballDeviceReportStruct::pinballButtons
         int rgKeyIndex; // g_pplayer->m_rgKeys[] index, or -1 if a direct VPM key is used instead
         BYTE vpmKey; // DIK_xxx key ID of VPM key, or 0 if an m_rgKeys assignment is used instead
      } keyMap[] = {
         { 0x00000001, eStartGameKey }, // Start (start game)
         { 0x00000002, eExitGame }, // Exit (end game)
         { 0x00000004, eAddCreditKey }, // Coin 1 (left coin chute)
         { 0x00000008, eAddCreditKey2 }, // Coin 2 (middle coin chute)
         { 0x00000010, -1, DIK_5 }, // Coin 3 (right coin chute)
         { 0x00000020, -1, DIK_6 }, // Coin 4 (fourth coin chute/dollar bill acceptor)
         { 0x00000040, -1, DIK_2 }, // Extra Ball/Buy-In
         { 0x00000080, ePlungerKey }, // Launch Ball
         { 0x00000100, eLockbarKey }, // Fire button (lock bar top button)
         { 0x00000200, eMechanicalTilt }, // Tilt bob
         { 0x00000400, -1, DIK_HOME }, // Slam tilt
         { 0x00000800, -1, DIK_END }, // Coin door switch
         { 0x00001000, -1, DIK_7 }, // Service panel Cancel
         { 0x00002000, -1, DIK_8 }, // Service panel Down
         { 0x00004000, -1, DIK_9 }, // Service panel Up
         { 0x00008000, -1, DIK_0 }, // Service panel Enter
         { 0x00010000, eLeftMagnaSave }, // MagnaSave left
         { 0x00020000, eRightMagnaSave }, // MagnaSave right
         { 0x00040000, eLeftTiltKey }, // Left Nudge
         { 0x00080000, eCenterTiltKey }, // Forward Nudge
         { 0x00100000, eRightTiltKey }, // Right Nudge
         { 0x00200000, eVolumeUp }, // Audio volume up
         { 0x00400000, eVolumeDown }, // Audio volume down
      };

      // Flipper buttons.  Fold upper and lower into a combined field,
      // and fold the button duty cycle information into a simple on/off.
      //
      // If the simulator is upgraded in the future to accept more detailed
      // timing information, we can convert the duty cycle into a suitable
      // amount of simulation time, using the real time between consecutive
      // HID inputs as the basis, and feed the event into the simulator as
      // a button press for the corresponding number of physics frames.  This
      // is irrelevant to VP 9, which has a physics frame time of 10ms,
      // roughly equal to the HID polling time.  But VP 10 has 1ms frames,
      // so it should be possible to profitably use the timing info there.
      const bool newFlipperLeft = cr.llFlipper != 0 || cr.ulFlipper != 0;
      const bool newFlipperRight = cr.lrFlipper != 0 || cr.urFlipper != 0;
      if (newFlipperLeft != m_openPinDev_flipper_l)
         FireKeyEvent(m_openPinDev_flipper_l = newFlipperLeft, g_pplayer->m_rgKeys[eLeftFlipperKey]);
      if (newFlipperRight != m_openPinDev_flipper_r)
         FireKeyEvent(m_openPinDev_flipper_r = newFlipperRight, g_pplayer->m_rgKeys[eRightFlipperKey]);

      // Visit each pre-assigned button
      const KeyMap *m = keyMap;
      for (size_t i = 0; i < _countof(keyMap); ++i, ++m)
      {
         // check for a state change
         const uint32_t mask = m->mask;
         const DISPID isDown = (cr.pinballButtons & mask) != 0 ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;
         const DISPID wasDown = (m_openPinDev_pinball_buttons & mask) != 0 ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;
         if (isDown != wasDown)
            FireKeyEvent(isDown, m->rgKeyIndex != -1 ? g_pplayer->m_rgKeys[m->rgKeyIndex] : m->vpmKey);
      }

      // remember the new button state
      cr.pinballButtons = m_openPinDev_pinball_buttons;
   }
}

PinInput::OpenPinDev::OpenPinDev(HANDLE hDevice, BYTE reportID, DWORD reportSize, const wchar_t *deviceStructVersionString)
   : hDevice(hDevice)
   , reportID(reportID)
   , reportSize(reportSize)
{
   // Parse the device struct version string into a DWORD, with the major
   // version in the high word and the minor version in the low word.
   // This format can be compared against a reference version with a
   // simple DWORD comparison: deviceStructVersion > 0x00020005 tests
   // for something newer than 2.5.
   const wchar_t *dot = wcschr(deviceStructVersionString, L'.');
   deviceStructVersion = (_wtoi(deviceStructVersionString) << 16) | (dot != nullptr ? _wtoi(dot + 1) : 0);

   // allocate space for the report
   buf.resize(reportSize);

   // Zero our internal report copy - this will clear out any newer
   // fields that aren't in the version of the structure that the
   // device sends us.
   memset(&r, 0, sizeof(r));

   // kick off the first overlapped read - we always keep one read
   // outstanding, so that we don't have to wait when we're ready to
   // process a report
   StartRead();
}

PinInput::OpenPinDev::~OpenPinDev()
{
   // close handles
   CloseHandle(hDevice);
   CloseHandle(hIOEvent);
}

bool PinInput::OpenPinDev::ReadReport()
{
   // Read reports until the buffer is empty, to be sure we have the latest.
   // The reason we loop as long as new reports are available is that the HID
   // driver buffers a number of reports, and returns one on each request.  So
   // if we just read the reports sequentially, we'd always have a lag time of
   // (number of internally buffered reports) * (time per report).  The default
   // buffer is 32 reports, and the HID polling time is usually about 10ms, so
   // we'd always see input that's about 1/3 of a second old, which is quite
   // noticeable on a human scale.  So we always hoover up all available reports
   // until a read blocks, at which point we know that we have the latest one.
   // While looping, we keep track of whether or not we've seen any new reports
   // at all, since it's possible that we're still waiting for the final read
   // that we started on the last polling cycle.
   bool newReport = false;
   for (;;)
   {
      // Check the status on the last read.  If the read completed synchronously,
      // or it was queued with pending status and has since completed, we have
      // a new report ready.  Otherwise the pipe is empty.
      if (readStatus == ERROR_SUCCESS || (readStatus == ERROR_IO_PENDING && GetOverlappedResult(hDevice, &ov, &bytesRead, FALSE)))
      {
         // Read completed - extract the Open Pinball Device struct from
         // the HID report byte block.  Copy the smaller of the actual
         // report data sent from the device or our version of the struct.
         // If the device reports a NEWER struct than the one we're compiled
         // against, it will have extra fields at the end, which we will
         // ignore by virtue of copying only the struct size we know about.
         // If the device reports an OLDER struct than the one we use, we'll
         // only populate the portion of our struct that the device provides,
         // leaving the other fields zeroed.
         memcpy(&r, &buf[1], std::min(reportSize, sizeof(r)));
         newReport = true;

         // start the next read - we always maintain one outstanding read
         StartRead();
      }
      else if (readStatus == ERROR_IO_PENDING)
      {
         // still waiting for the last read to complete - stop here
         return newReport;
      }
      else
      {
         // Error other than pending status - start a new read, and stop here.
         // We could check for immediate completion, but that could get us into
         // an infinite loop if the handle has a persistent error (e.g., the
         // device has been disconnected), since we'd be right back here with
         // another error for the new read.  Returning ensures that we don't get
         // stuck.  The caller will still keep trying to poll the device in
         // such a case, but each new read will just fail immediately, so it
         // will do no harm other than the slight overhead of one failed read
         // per polling cycle.  And if the fault is temporary, this will pick
         // up where we left off as soon as the fault clears.
         StartRead();
         return newReport;
      }
   }
}

void PinInput::OpenPinDev::StartRead()
{
   // set up the OVERLAPPED struct
   memset(&ov, 0, sizeof(ov));
   ov.hEvent = hIOEvent;

   // start the read
   if (ReadFile(hDevice, buf.data(), reportSize, &bytesRead, &ov))
      readStatus = ERROR_SUCCESS;
   else
      readStatus = GetLastError();
}
