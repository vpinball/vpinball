#include "StdAfx.h"

#define INPUT_BUFFER_SIZE MAX_KEYQUEUE_SIZE

extern bool gMixerKeyDown;
extern bool gMixerKeyUp;

PinInput::PinInput()
{
   ZeroMemory(this, sizeof(PinInput));

   //InputControlRun = 0;

   m_pDI = NULL;
#ifdef USE_DINPUT_FOR_KEYBOARD
   m_pKeyboard = NULL;
#endif
   m_pMouse = NULL;

   leftMouseButtonDown = false;
   rightMouseButtonDown = false;
   middleMouseButtonDown = false;

   m_head = m_tail = 0;

   ZeroMemory(m_diq, sizeof(m_diq));

   e_JoyCnt = 0;
   for (int k = 0; k < PININ_JOYMXCNT; ++k)
      m_pJoystick[k] = NULL;

   uShockDevice = -1;	// only one uShock device
   uShockType = 0;

   m_plunger_axis = 3;
   m_lr_axis = 1;
   m_ud_axis = 2;
   m_plunger_reverse = 0;
   m_lr_axis_reverse = 0;
   m_ud_axis_reverse = 0;
   m_override_default_buttons = 0;
   m_disable_esc = 0;
   m_joylflipkey = 0;
   m_joyrflipkey = 0;
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
   m_joymechtilt = 0;

   m_firedautostart = 0;

   m_pressed_start = 0;

   m_enableMouseInPlayer = true;
   m_enableCameraModeFlyAround = false;
   m_enable_nudge_filter = false;

   m_cameraModeAltKey = false;
   m_cameraMode = 0;

   LoadSettings();

   m_exit_stamp = 0;
   m_first_stamp = msec();

   m_as_down = 0;
   m_as_didonce = 0;

   m_tilt_updown = DISPID_GameEvents_KeyUp;

   m_linearPlunger = false;

   gMixerKeyDown = false;
   gMixerKeyUp = false;
}

void PinInput::LoadSettings()
{
   HRESULT hr;
   int tmp;

   hr = GetRegInt("Player", "LRAxis", &tmp);
   if (hr == S_OK) m_lr_axis = tmp;

   hr = GetRegInt("Player", "UDAxis", &tmp);
   if (hr == S_OK) m_ud_axis = tmp;

   hr = GetRegInt("Player", "LRAxisFlip", &tmp);
   if (hr == S_OK) m_lr_axis_reverse = tmp;

   hr = GetRegInt("Player", "UDAxisFlip", &tmp);
   if (hr == S_OK) m_ud_axis_reverse = tmp;

   hr = GetRegInt("Player", "PlungerAxis", &tmp);
   if (hr == S_OK) m_plunger_axis = tmp;

   hr = GetRegInt("Player", "ReversePlungerAxis", &tmp);
   if (hr == S_OK) m_plunger_reverse = tmp;

   hr = GetRegInt("Player", "PBWDefaultLayout", &tmp);
   if (hr == S_OK) m_override_default_buttons = tmp;

   hr = GetRegInt("Player", "DisableESC", &tmp);
   if (hr == S_OK) m_disable_esc = tmp;

   hr = GetRegInt("Player", "JoyLFlipKey", &tmp);
   if (hr == S_OK) m_joylflipkey = tmp;

   hr = GetRegInt("Player", "JoyRFlipKey", &tmp);
   if (hr == S_OK) m_joyrflipkey = tmp;

   hr = GetRegInt("Player", "JoyPlungerKey", &tmp);
   if (hr == S_OK) m_joyplungerkey = tmp;

   hr = GetRegInt("Player", "JoyAddCreditKey", &tmp);
   if (hr == S_OK) m_joyaddcreditkey = tmp;

   hr = GetRegInt("Player", "JoyAddCredit2Key", &tmp);
   if (hr == S_OK) m_joyaddcreditkey2 = tmp;

   hr = GetRegInt("Player", "JoyLMagnaSave", &tmp);
   if (hr == S_OK) m_joylmagnasave = tmp;

   hr = GetRegInt("Player", "JoyRMagnaSave", &tmp);
   if (hr == S_OK) m_joyrmagnasave = tmp;

   hr = GetRegInt("Player", "JoyStartGameKey", &tmp);
   if (hr == S_OK) m_joystartgamekey = tmp;

   hr = GetRegInt("Player", "JoyFrameCount", &tmp);
   if (hr == S_OK) m_joyframecount = tmp;

   hr = GetRegInt("Player", "JoyExitGameKey", &tmp);
   if (hr == S_OK) m_joyexitgamekey = tmp;

   hr = GetRegInt("Player", "JoyVolumeUp", &tmp);
   if (hr == S_OK) m_joyvolumeup = tmp;

   hr = GetRegInt("Player", "JoyVolumeDown", &tmp);
   if (hr == S_OK) m_joyvolumedown = tmp;

   hr = GetRegInt("Player", "JoyLTiltKey", &tmp);
   if (hr == S_OK) m_joylefttilt = tmp;

   hr = GetRegInt("Player", "JoyCTiltKey", &tmp);
   if (hr == S_OK) m_joycentertilt = tmp;

   hr = GetRegInt("Player", "JoyRTiltKey", &tmp);
   if (hr == S_OK) m_joyrighttilt = tmp;

   hr = GetRegInt("Player", "JoyPMBuyIn", &tmp);
   if (hr == S_OK) m_joypmbuyin = tmp;

   hr = GetRegInt("Player", "JoyPMCoin3", &tmp);
   if (hr == S_OK) m_joypmcoin3 = tmp;

   hr = GetRegInt("Player", "JoyPMCoin4", &tmp);
   if (hr == S_OK) m_joypmcoin4 = tmp;

   hr = GetRegInt("Player", "JoyPMCoinDoor", &tmp);
   if (hr == S_OK) m_joypmcoindoor = tmp;

   hr = GetRegInt("Player", "JoyPMCancel", &tmp);
   if (hr == S_OK) m_joypmcancel = tmp;

   hr = GetRegInt("Player", "JoyPMDown", &tmp);
   if (hr == S_OK) m_joypmdown = tmp;

   hr = GetRegInt("Player", "JoyPMUp", &tmp);
   if (hr == S_OK) m_joypmup = tmp;

   hr = GetRegInt("Player", "JoyPMEnter", &tmp);
   if (hr == S_OK) m_joypmenter = tmp;

   hr = GetRegInt("Player", "JoyCustom1", &tmp);
   if (hr == S_OK) m_joycustom1 = tmp;

   hr = GetRegInt("Player", "JoyCustom1Key", &tmp);
   if (hr == S_OK) m_joycustom1key = tmp;

   hr = GetRegInt("Player", "JoyCustom2", &tmp);
   if (hr == S_OK) m_joycustom2 = tmp;

   hr = GetRegInt("Player", "JoyCustom2Key", &tmp);
   if (hr == S_OK) m_joycustom2key = tmp;

   hr = GetRegInt("Player", "JoyCustom3", &tmp);
   if (hr == S_OK) m_joycustom3 = tmp;

   hr = GetRegInt("Player", "JoyCustom3Key", &tmp);
   if (hr == S_OK) m_joycustom3key = tmp;

   hr = GetRegInt("Player", "JoyCustom4", &tmp);
   if (hr == S_OK) m_joycustom4 = tmp;

   hr = GetRegInt("Player", "JoyCustom4Key", &tmp);
   if (hr == S_OK) m_joycustom4key = tmp;

   hr = GetRegInt("Player", "JoyMechTiltKey", &tmp);
   if (hr == S_OK) m_joymechtilt = tmp;

   hr = GetRegInt("Player", "JoyDebugKey", &tmp);
   if (hr == S_OK) m_joydebugballs = tmp;

   hr = GetRegInt("Player", "JoyDebuggerKey", &tmp);
   if (hr == S_OK) m_joydebugger = tmp;

   hr = GetRegInt("Player", "JoyLockbarKey", &tmp);
   if (hr == S_OK) m_joylockbar = tmp;

   hr = GetRegInt("Player", "EnableMouseInPlayer", &tmp);
   if (hr == S_OK) m_enableMouseInPlayer = (tmp == fTrue);

   hr = GetRegInt("Player", "EnableCameraModeFlyAround", &tmp);
   if (hr == S_OK) m_enableCameraModeFlyAround = (tmp == fTrue);

   hr = GetRegInt("Player", "EnableNudgeFilter", &tmp);
   if (hr == S_OK) m_enable_nudge_filter = (tmp == fTrue);

   hr = GetRegInt("Player", "DeadZone", &m_deadz);
   if (hr != S_OK)
      m_deadz = 0;
   else
      m_deadz = m_deadz*JOYRANGEMX / 100;
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//		joystick. This function enables user interface elements for objects
//		that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,
   VOID* pContext)
{
   PinInput * const ppinput = (PinInput *)pContext;

#ifdef _DEBUG
   static int nAxis = 0;
   static int nButtons = 0;
   static int nSliderCount = 0;	// Number of returned slider controls
   static int nPOVCount = 0;		// Number of returned POV controls
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
      if (FAILED(ppinput->m_pJoystick[ppinput->e_JoyCnt]->SetProperty(DIPROP_RANGE, &diprg.diph)))
         return DIENUM_STOP;

      // set DEADBAND to Zero
      DIPROPDWORD dipdw;
      dipdw.diph.dwSize = sizeof(DIPROPDWORD);
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      dipdw.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
      dipdw.diph.dwHow = DIPH_BYID;
      dipdw.dwData = 0; // no dead zone at joystick level

      // Set the deadzone
      if (FAILED(ppinput->m_pJoystick[ppinput->e_JoyCnt]->SetProperty(DIPROP_DEADZONE, &dipdw.diph)))
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
BOOL CALLBACK DIEnumJoystickCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
   DIPROPSTRING dstr = { 0 };
   dstr.diph.dwSize = sizeof(DIPROPSTRING);
   dstr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
   dstr.diph.dwObj = 0;
   dstr.diph.dwHow = 0;

   PinInput * const ppinput = (PinInput *)pvRef;

   HRESULT hr;

   hr = ppinput->m_pDI->CreateDevice(lpddi->guidInstance, &ppinput->m_pJoystick[ppinput->e_JoyCnt], NULL);
   if (FAILED(hr))
   {
      ppinput->m_pJoystick[ppinput->e_JoyCnt] = NULL; //make sure no garbage
      return DIENUM_CONTINUE; // try for another joystick
   }

   hr = ppinput->m_pJoystick[ppinput->e_JoyCnt]->GetProperty(DIPROP_PRODUCTNAME, &dstr.diph);
   if (hr == S_OK)
   {
      if (!WzSzStrCmp(dstr.wsz, "PinballWizard"))
      {
         ppinput->uShockDevice = ppinput->e_JoyCnt;	// remember uShock
         ppinput->uShockType = USHOCKTYPE_PBWIZARD;  //set type 1 = PinballWizard
      }
      else if (!WzSzStrCmp(dstr.wsz, "UltraCade Pinball"))
      {
         ppinput->uShockDevice = ppinput->e_JoyCnt;	// remember uShock
         ppinput->uShockType = USHOCKTYPE_ULTRACADE; //set type 2 = UltraCade Pinball
      }
      else if (!WzSzStrCmp(dstr.wsz, "Microsoft SideWinder Freestyle Pro (USB)"))
      {
         ppinput->uShockDevice = ppinput->e_JoyCnt;	// remember uShock
         ppinput->uShockType = USHOCKTYPE_SIDEWINDER;//set type 3 = Microsoft SideWinder Freestyle Pro
      }
      else if (!WzSzStrCmp(dstr.wsz, "VirtuaPin Controller"))
      {
         ppinput->uShockDevice = ppinput->e_JoyCnt;	// remember uShock
         ppinput->uShockType = USHOCKTYPE_VIRTUAPIN; //set type 4 = VirtuaPin Controller
      }
      else if (!WzSzStrCmp(dstr.wsz, "Pinscape Controller"))
      {
         ppinput->uShockDevice = ppinput->e_JoyCnt;  // remember uShock
         ppinput->uShockType = USHOCKTYPE_GENERIC;   //set type = Generic
         ppinput->m_linearPlunger = 1;               // use linear plunger calibration
      }
      else
      {
         ppinput->uShockDevice = ppinput->e_JoyCnt;	// remember uShock
         ppinput->uShockType = USHOCKTYPE_GENERIC;   //Generic Gamepad
      }
   }
   hr = ppinput->m_pJoystick[ppinput->e_JoyCnt]->SetDataFormat(&c_dfDIJoystick);

   // joystick input foreground or background focus
   hr = ppinput->m_pJoystick[ppinput->e_JoyCnt]->SetCooperativeLevel(ppinput->m_hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

   DIPROPDWORD dipdw;
   dipdw.diph.dwSize = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
   dipdw.diph.dwObj = 0;
   dipdw.diph.dwHow = DIPH_DEVICE;
   dipdw.dwData = INPUT_BUFFER_SIZE;

   hr = ppinput->m_pJoystick[ppinput->e_JoyCnt]->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

   // Enumerate the joystick objects. The callback function enabled user
   // interface elements for objects that are found, and sets the min/max
   // values property for discovered axes.
   hr = ppinput->m_pJoystick[ppinput->e_JoyCnt]->EnumObjects(EnumObjectsCallback, (VOID*)pvRef, DIDFT_ALL);

   if (++(ppinput->e_JoyCnt) < PININ_JOYMXCNT)
       return DIENUM_CONTINUE;
   else
       return DIENUM_STOP;			//allocation for only PININ_JOYMXCNT joysticks, ignore any others
}

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
      return NULL; // queue empty?

   const DIDEVICEOBJECTDATA * const ptr = &m_diq[m_tail];

   // If we've simulated to or beyond the timestamp of when this control was received then process the control into the system
   //if( curr_sim_msec >= ptr->dwTimeStamp ) //!! time stamp disabled to save a bit of lag
   {
      m_tail = (m_tail + 1) % MAX_KEYQUEUE_SIZE; // advance tail of queue

      return ptr;
   }
   //else return NULL;
}

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
            if (g_pplayer->m_fThrowBalls) // debug ball throw functionality
            {
               if ((mouseState.rgbButtons[0] & 0x80) && !leftMouseButtonDown && !rightMouseButtonDown && !middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  mouseX = curPos.x;
                  mouseY = curPos.y;
                  leftMouseButtonDown = true;
               }
               if (!(mouseState.rgbButtons[0] & 0x80) && leftMouseButtonDown && !rightMouseButtonDown && !middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  mouseDX = curPos.x - mouseX;
                  mouseDY = curPos.y - mouseY;
                  didod[0].dwData = 1;
                  PushQueue(didod, APP_MOUSE);
                  leftMouseButtonDown = false;
               }
               if ((mouseState.rgbButtons[1] & 0x80) && !rightMouseButtonDown && !leftMouseButtonDown && !middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  mouseX = curPos.x;
                  mouseY = curPos.y;
                  rightMouseButtonDown = true;
               }
               if (!(mouseState.rgbButtons[1] & 0x80) && !leftMouseButtonDown && rightMouseButtonDown && !middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  mouseDX = curPos.x - mouseX;
                  mouseDY = curPos.y - mouseY;
                  didod[0].dwData = 2;
                  PushQueue(didod, APP_MOUSE);
                  rightMouseButtonDown = false;
               }
               if ((mouseState.rgbButtons[2] & 0x80) && !rightMouseButtonDown && !leftMouseButtonDown && !middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  mouseX = curPos.x;
                  mouseY = curPos.y;
                  middleMouseButtonDown = true;
               }
               if (!(mouseState.rgbButtons[2] & 0x80) && !rightMouseButtonDown && !leftMouseButtonDown && middleMouseButtonDown)
               {
                  POINT curPos;
                  GetCursorPos(&curPos);
                  mouseDX = curPos.x - mouseX;
                  mouseDY = curPos.y - mouseY;
                  didod[0].dwData = 3;
                  PushQueue(didod, APP_MOUSE);
                  middleMouseButtonDown = false;
               }
            } //if (g_pplayer->m_fThrowBalls)
            else
            {
               for (DWORD i = 0; i < 3; i++)
               {
                  if (oldMouseButtonState[i] != mouseState.rgbButtons[i])
                  {
                     didod[i].dwData = mouseState.rgbButtons[i];
                     didod[i].dwOfs = i + 1;
                     didod[i].dwSequence = APP_MOUSE;
                     PushQueue(&didod[i], APP_MOUSE);
                     oldMouseButtonState[i] = mouseState.rgbButtons[i];
                  }
               }

            }
         }
      }
   }

   // same for joysticks 
   for (int k = 0; k < e_JoyCnt; ++k)
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
}

void PinInput::Init(const HWND hwnd)
{
   m_hwnd = hwnd;

   HRESULT hr;
#ifdef USE_DINPUT8
   hr = DirectInput8Create(g_hinst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDI, NULL);
#else
   hr = DirectInputCreate(g_hinst, DIRECTINPUT_VERSION, &m_pDI, NULL);
#endif

#ifdef USE_DINPUT_FOR_KEYBOARD
   // Create keyboard device
   hr = m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL); //Standard Keyboard device

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

   if (m_enableMouseInPlayer)
   {
      // Create mouse device
      if (!FAILED(m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, NULL)))
      {
         hr = m_pMouse->SetDataFormat(&c_dfDIMouse2);

         //hr = m_pMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
         DIPROPDWORD dipdwm;
         dipdwm.diph.dwSize = sizeof(DIPROPDWORD);
         dipdwm.diph.dwHeaderSize = sizeof(DIPROPHEADER);
         dipdwm.diph.dwObj = 0;
         dipdwm.diph.dwHow = DIPH_DEVICE;
         dipdwm.dwData = INPUT_BUFFER_SIZE;

         hr = m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdwm.diph);
      }
      else
         m_pMouse = NULL;
   }

   /* Disable Sticky Keys */

   // get the current state
   m_StartupStickyKeys.cbSize = sizeof(STICKYKEYS);
   SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);

   // turn it all OFF
   STICKYKEYS newStickyKeys;
   ZeroMemory(&newStickyKeys, sizeof(STICKYKEYS));
   newStickyKeys.cbSize = sizeof(STICKYKEYS);
   newStickyKeys.dwFlags = 0;
   SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &newStickyKeys, SPIF_SENDCHANGE);

   for (int i = 0; i < 4; i++)
      m_keyPressedState[i] = false;
   m_nextKeyPressedTime = 0;
   uShockDevice = -1;
   uShockType = 0;
#ifdef USE_DINPUT8
   m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);//enum Joysticks
#else
   m_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);//enum Joysticks
#endif

   gMixerKeyDown = false;
   gMixerKeyUp = false;

   //InputControlRun = 1; //0==stalled, 1==run,  0 < shutting down, 2==terminated
   //_beginthread( InputControlProcess, 0, NULL );
}


void PinInput::UnInit()
{
   // Unacquire and release any DirectInputDevice objects.
   //1==run,  0 < shutting down, 2==terminated
   //InputControlRun = -100;	// terminate control thread, force after 500mS

   //while (++InputControlRun < 0) Sleep(5);		// set to exit AND WAIT

   //if (!InputControlRun)	//0 == stalled, 1==run,  0 < shutting down, 2==terminated
   //{exit (-1500);}

#ifdef USE_DINPUT_FOR_KEYBOARD
   if (m_pKeyboard)
   {
      // Unacquire the device one last time just in case 
      // the app tried to exit while the device is still acquired.
      m_pKeyboard->Unacquire();
      m_pKeyboard->Release();
      m_pKeyboard = NULL;
   }
#endif

   if (m_pMouse)
   {
      m_pMouse->Unacquire();
      m_pMouse->Release();
      m_pMouse = NULL;
   }

   // restore the state of the sticky keys
   SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, SPIF_SENDCHANGE);

   for (int k = 0; k < e_JoyCnt; ++k)
   {
      if (m_pJoystick[k])
      {
         // Unacquire the device one last time just in case 
         // the app tried to exit while the device is still acquired.
         m_pJoystick[k]->Unacquire();
         m_pJoystick[k]->Release();
         m_pJoystick[k] = NULL;
      }
   }

   // Release any DirectInput objects.
   if (m_pDI)
   {
      m_pDI->Release();
      m_pDI = NULL;
   }

   m_head = m_tail = 0;

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
      else if (keycode == g_pplayer->m_rgKeys[eLeftMagnaSave]) keycode = g_pplayer->m_rgKeys[eRightMagnaSave];
      else if (keycode == g_pplayer->m_rgKeys[eRightMagnaSave]) keycode = g_pplayer->m_rgKeys[eLeftMagnaSave];
      else if (keycode == DIK_LSHIFT) keycode = DIK_RSHIFT;
      else if (keycode == DIK_RSHIFT) keycode = DIK_LSHIFT;
      else if (keycode == DIK_LEFT)   keycode = DIK_RIGHT;
      else if (keycode == DIK_RIGHT)  keycode = DIK_LEFT;
   }

   if (g_pplayer->cameraMode)
   {
      m_keyPressedState[eLeftFlipperKey] = false;
      m_keyPressedState[eRightFlipperKey] = false;
      m_keyPressedState[eLeftTiltKey] = false;
      m_keyPressedState[eRightTiltKey] = false;
      if (keycode == g_pplayer->m_rgKeys[eLeftFlipperKey] && dispid == DISPID_GameEvents_KeyDown)
      {
         g_pplayer->UpdateBackdropSettings(false);
         m_keyPressedState[eLeftFlipperKey] = true;
      }
      else if (keycode == g_pplayer->m_rgKeys[eRightFlipperKey] && dispid == DISPID_GameEvents_KeyDown)
      {
         g_pplayer->UpdateBackdropSettings(true);
         m_keyPressedState[eRightFlipperKey] = true;
      }
      else if (keycode == g_pplayer->m_rgKeys[eLeftTiltKey] && dispid == DISPID_GameEvents_KeyDown && m_enableCameraModeFlyAround)
      {
         g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] -= 1.0f;
         m_keyPressedState[eLeftTiltKey] = true;
      }
      else if (keycode == g_pplayer->m_rgKeys[eRightTiltKey] && dispid == DISPID_GameEvents_KeyDown && m_enableCameraModeFlyAround)
      {
         g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] += 1.0f;
         m_keyPressedState[eRightTiltKey] = true;
      }
      else if (keycode == g_pplayer->m_rgKeys[eStartGameKey] && dispid == DISPID_GameEvents_KeyDown)
      {
         g_pplayer->m_ptable->m_BG_layback[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupLayback;
         g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupRotation;
         g_pplayer->m_ptable->m_BG_inclination[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupInclination;
         g_pplayer->m_ptable->m_BG_xlatex[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupOffset.x;
         g_pplayer->m_ptable->m_BG_xlatey[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupOffset.y;
         g_pplayer->m_ptable->m_BG_xlatez[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupOffset.z;
         g_pplayer->m_ptable->m_BG_scalex[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupScale.x;
         g_pplayer->m_ptable->m_BG_scaley[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupScale.y;
         g_pplayer->m_ptable->m_BG_scalez[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupScale.z;
         g_pplayer->m_ptable->m_BG_FOV[g_pplayer->m_ptable->m_BG_current_set] = g_pplayer->m_ptable->m_backupFOV;
         g_pplayer->m_ptable->m_lightHeight = g_pplayer->m_ptable->m_backupLightHeight;
         g_pplayer->m_ptable->m_lightRange = g_pplayer->m_ptable->m_backupLightRange;
         g_pplayer->m_ptable->m_lightEmissionScale = g_pplayer->m_ptable->m_backupEmisionScale;
         g_pplayer->m_ptable->m_envEmissionScale = g_pplayer->m_ptable->m_backupEnvEmissionScale;
         g_pplayer->m_pin3d.m_camx = 0;
         g_pplayer->m_pin3d.m_camy = 0;
         g_pplayer->m_pin3d.m_camz = 0;
      }
      else if (keycode == g_pplayer->m_rgKeys[eRightMagnaSave] && dispid == DISPID_GameEvents_KeyDown)
      {
         g_pplayer->backdropSettingActive++;
         if (g_pplayer->backdropSettingActive == 13)
            g_pplayer->backdropSettingActive = 0;
      }
      else if (keycode == g_pplayer->m_rgKeys[eLeftMagnaSave] && dispid == DISPID_GameEvents_KeyDown)
      {
         g_pplayer->backdropSettingActive--;
         if (g_pplayer->backdropSettingActive == -1)
            g_pplayer->backdropSettingActive = 12;
      }
   }
   else
   {
      // Debug only, for testing parts of the left flipper input lag
      if (keycode == g_pplayer->m_rgKeys[eLeftFlipperKey] && dispid == DISPID_GameEvents_KeyDown)
      {
         m_leftkey_down_usec = usec();
         m_leftkey_down_frame = g_pplayer->m_overall_frames;
      }

      // Mixer volume only
      gMixerKeyDown = (keycode == g_pplayer->m_rgKeys[eVolumeDown] && dispid == DISPID_GameEvents_KeyDown);
      gMixerKeyUp   = (keycode == g_pplayer->m_rgKeys[eVolumeUp]   && dispid == DISPID_GameEvents_KeyDown);

      g_pplayer->m_ptable->FireKeyEvent(dispid, keycode);
   }
}

// Returns true if the table has started at least 1 player.
int PinInput::started()
{
   // Was the start button pressed?
   if (m_pressed_start)
      return 1;

   if (g_pplayer->m_vball.size() > 0)
   {
      m_pressed_start = 1;
      return 1;
   }
   else
      return 0;
}

void PinInput::autostart(const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec)
{
   //	if( !VPinball::m_open_minimized ) 
   //		return;

   // Make sure we have a player.
   if (!g_pplayer ||
      // Check if we already started.
      started())
      return;

   if ((m_firedautostart > 0) &&				   // Initialized.
      (m_as_down == 1) &&						   // Start button is down.
      ((curr_time_msec - m_firedautostart) > 100)) // Start button has been down for at least 0.10 seconds.
   {
      // Release start.
      m_firedautostart = curr_time_msec;
      m_as_down = 0;
      FireKeyEvent(DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eStartGameKey]);

#ifdef _DEBUG
      OutputDebugString("Autostart: Release.\n");
#endif
   }

   // Logic to do "autostart"
   if ((m_as_down == 0) &&																			  // Start button is up.
      (((m_as_didonce == 1) && !started() && ((curr_time_msec - m_firedautostart) > retry_msecs))) || // Not started and last attempt was at least AutoStartRetry seconds ago.
      ((m_as_didonce == 0) && ((curr_time_msec - m_firedautostart) > msecs)))						  // Never attempted and autostart time has elapsed.
   {
      // Press start.
      m_firedautostart = curr_time_msec;
      m_as_down = 1;
      m_as_didonce = 1;
      FireKeyEvent(DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eStartGameKey]);

#ifdef _DEBUG
      OutputDebugString("Autostart: Press.\n");
#endif
   }
}

void PinInput::button_exit(const U32 msecs, const U32 curr_time_msec)
{
   // Don't allow button exit until after game has been running for 1 second.
   if (curr_time_msec - m_first_stamp < 1000)
      return;

   // Check if we can exit.
   if (m_exit_stamp &&                         // Initialized.
      (curr_time_msec - m_exit_stamp > msecs)) // Held exit button for number of mseconds.
   {
      if (uShockType == USHOCKTYPE_ULTRACADE)
         ExitApp(); //remove pesky exit button
      else
         //exit(0); //Close out to desktop
         g_pvp->Quit();
   }
}

void PinInput::tilt_update()
{
   if (!g_pplayer) return;

   const int tmp = m_tilt_updown;
   m_tilt_updown = plumb_tilted() ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;

   if (m_tilt_updown != tmp)
      FireKeyEvent(m_tilt_updown, g_pplayer->m_rgKeys[eCenterTiltKey]);
}

void PinInput::ProcessCameraKeys(const DIDEVICEOBJECTDATA * __restrict input)
{
    switch(input->dwOfs)
    {
    case DIK_UP:
        {
            if ((input->dwData & 0x80) != 0)
            {
                if (!m_cameraModeAltKey)
                    g_pplayer->m_pin3d.m_camy += 10.0f;
                else
                    g_pplayer->m_pin3d.m_camz += 10.0f;

                m_cameraMode = 1;
            }
            else
                m_cameraMode = 0;
        }
        break;
    case DIK_DOWN:
        {
            if ((input->dwData & 0x80) != 0)
            {
                if (!m_cameraModeAltKey)
                    g_pplayer->m_pin3d.m_camy -= 10.0f;
                else
                    g_pplayer->m_pin3d.m_camz -= 10.0f;

                m_cameraMode = 2;
            }
            else
                m_cameraMode = 0;
        }
        break;
    case DIK_RIGHT:
        {
            if ((input->dwData & 0x80) != 0)
            {
                if (!m_cameraModeAltKey)
                    g_pplayer->m_pin3d.m_camx -= 10.0f;
                else
                    g_pplayer->m_pin3d.m_inc -= 0.01f;

                m_cameraMode = 3;
            }
            else
                m_cameraMode = 0;
        }
        break;
    case DIK_LEFT:
        {
            if ((input->dwData & 0x80) != 0)
            {
                if (!m_cameraModeAltKey)
                    g_pplayer->m_pin3d.m_camx += 10.0f;
                else
                    g_pplayer->m_pin3d.m_inc += 0.01f;

                m_cameraMode = 4;
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
   if (m_joylflipkey == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftFlipperKey]);
   if (m_joyrflipkey == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightFlipperKey]);
   if (m_joyplungerkey == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[ePlungerKey]);
   if (m_joyaddcreditkey == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey]);
   if (m_joyaddcreditkey2 == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey2]);
   if (m_joylmagnasave == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftMagnaSave]);
   if (m_joyrmagnasave == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightMagnaSave]);
   if (m_joystartgamekey == n)
   {
      if (start)
      {
         m_pressed_start = n;
         FireKeyEvent(updown, g_pplayer->m_rgKeys[eStartGameKey]);
      }
   }
   if (m_joyexitgamekey == n)
   {
      if (DISPID_GameEvents_KeyDown == updown)
         g_pplayer->m_fCloseDown = true;
   }
   if (m_joyframecount == n)
   {
      if (DISPID_GameEvents_KeyDown == updown)
         g_pplayer->ToggleFPS();
   }
   if (m_joyvolumeup == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeUp]);
   if (m_joyvolumedown == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeDown]);
   if (m_joylefttilt == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftTiltKey]);
   if (m_joycentertilt == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eCenterTiltKey]);
   if (m_joyrighttilt == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightTiltKey]);
   if (m_joymechtilt == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eMechanicalTilt]);
   if (m_joydebugballs == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eDBGBalls]);
   if (m_joydebugger == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eDebugger]);
   if (m_joylockbar == n) FireKeyEvent(updown, g_pplayer->m_rgKeys[eLockbarKey]);
   if (m_joycustom1 == n) FireKeyEvent(updown, m_joycustom1key);
   if (m_joycustom2 == n) FireKeyEvent(updown, m_joycustom2key);
   if (m_joycustom3 == n) FireKeyEvent(updown, m_joycustom3key);
   if (m_joycustom4 == n) FireKeyEvent(updown, m_joycustom4key);
   if (m_joypmbuyin == n) FireKeyEvent(updown, DIK_2);
   if (m_joypmcoin3 == n) FireKeyEvent(updown, DIK_5);
   if (m_joypmcoin4 == n) FireKeyEvent(updown, DIK_6);
   if (m_joypmcoindoor == n) FireKeyEvent(updown, DIK_END);
   if (m_joypmcancel == n) FireKeyEvent(updown, DIK_7);
   if (m_joypmdown == n) FireKeyEvent(updown, DIK_8);
   if (m_joypmup == n) FireKeyEvent(updown, DIK_9);
   if (m_joypmenter == n) FireKeyEvent(updown, DIK_0);
}

void PinInput::ProcessThrowBalls(const DIDEVICEOBJECTDATA * __restrict input)
{
    if (input->dwData == 1 || input->dwData == 3)
    {
        POINT point = { mouseX, mouseY };
        ScreenToClient(m_hwnd, &point);
        const Vertex3Ds vertex = g_pplayer->m_pin3d.Get3DPointFrom2D(point);

        float vx = (float)mouseDX*0.1f;
        float vy = (float)mouseDY*0.1f;
        if (g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] != 0.f && g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] != 360.f)
        {
            const float radangle = ANGTORAD(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set]);
            const float sn = sinf(radangle);
            const float cs = cosf(radangle);

            const float vx2 = cs*vx - sn*vy;
            const float vy2 = sn*vx + cs*vy;
            vx = -vx2;
            vy = -vy2;
        }
        bool ballGrabbed = false;
        if (input->dwData == 1)
        {
            for (unsigned i = 0; i < g_pplayer->m_vball.size(); i++)
            {
                Ball * const pBall = g_pplayer->m_vball[i];
                const float dx = fabsf(vertex.x - pBall->m_pos.x);
                const float dy = fabsf(vertex.y - pBall->m_pos.y);
                if (dx < pBall->m_radius*2.f && dy < pBall->m_radius*2.f)
                {
                    POINT newPoint;
                    GetCursorPos(&newPoint);
                    ScreenToClient(m_hwnd, &newPoint);
                    const Vertex3Ds vert = g_pplayer->m_pin3d.Get3DPointFrom2D(newPoint);

                    ballGrabbed = true;
                    pBall->m_pos.x = vert.x;
                    pBall->m_pos.y = vert.y;
                    pBall->m_vel.x = vx;
                    pBall->m_vel.y = vy;
                    pBall->Init();
                    break;
                }
            }
        }
        if (!ballGrabbed)
        {
            const float z = (input->dwData == 3) ? g_pplayer->m_ptable->m_glassheight : g_pplayer->m_ptable->m_tableheight;
            Ball * const pball = g_pplayer->CreateBall(vertex.x, vertex.y, z, vx, vy, 0, (float)g_pplayer->m_DebugBallSize*0.5f);
            pball->m_pballex->AddRef();
        }
    }
    else if (input->dwData == 2)
    {
        POINT point = { mouseX, mouseY };
        ScreenToClient(m_hwnd, &point);
        const Vertex3Ds vertex = g_pplayer->m_pin3d.Get3DPointFrom2D(point);

        for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
        {
            Ball * const pBall = g_pplayer->m_vball[i];
            const float dx = fabsf(vertex.x - pBall->m_pos.x);
            const float dy = fabsf(vertex.y - pBall->m_pos.y);
            if (dx < pBall->m_radius*2.f && dy < pBall->m_radius*2.f)
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
    static const bool rotLeftManual = false; //!! delete

    if (input->dwOfs >= DIJOFS_BUTTON0 && input->dwOfs <= DIJOFS_BUTTON31)
    {
        const int updown = (input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;
        const bool start = ((curr_time_msec - m_firedautostart) > g_pplayer->m_ptable->m_tblAutoStart) || m_pressed_start || started();
        if (input->dwOfs == DIJOFS_BUTTON0)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // plunge
                FireKeyEvent(updown, g_pplayer->m_rgKeys[ePlungerKey]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // coin 1
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey]);
            else
                Joy(1, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON1)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // right
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightFlipperKey]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // coin 2
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey2]);
            else
                Joy(2, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON2)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_ULTRACADE) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0))
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightMagnaSave]); // right2
            else
                Joy(3, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON3)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // volume down
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeDown]);
            else
                Joy(4, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON4)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // volume up
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeUp]);
            else
                Joy(5, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON5)
        {
            if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // volume up
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeUp]);
            else
                Joy(6, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON6)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // pause menu
            {
                if (DISPID_GameEvents_KeyDown == updown) g_pplayer->m_fCloseDown = true;
            }
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // volume down
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eVolumeDown]);
            else
                Joy(7, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON7)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0) && (m_disable_esc == 0)) // exit
            {	// Check if we have started a game yet.
                if (started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled)
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
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0))
            {
                if (start)
                {
                    m_pressed_start = 1;
                    FireKeyEvent(updown, g_pplayer->m_rgKeys[eStartGameKey]);
                }
            }
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0))	// left
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftFlipperKey]);
            else
                Joy(9, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON9)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0))	// left
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftFlipperKey]);
            else
                Joy(10, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON10)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0))	// left 2
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eLeftMagnaSave]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // right
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eRightFlipperKey]);
            else
                Joy(11, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON11)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // coin 1
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey]);
            else
                Joy(12, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON12)
        {
            if (((uShockType == USHOCKTYPE_PBWIZARD) || (uShockType == USHOCKTYPE_VIRTUAPIN)) && (m_override_default_buttons == 0)) // coin 2
                FireKeyEvent(updown, g_pplayer->m_rgKeys[eAddCreditKey2]);
            else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // start
            { // Check if we can allow the start (table is done initializing).
                if (start)
                {
                    m_pressed_start = 1;
                    FireKeyEvent(updown, g_pplayer->m_rgKeys[eStartGameKey]);
                }
            }
            else
                Joy(13, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON13)
        {
            if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // plunge
                FireKeyEvent(updown, g_pplayer->m_rgKeys[ePlungerKey]);
            else
                Joy(14, updown, start);
        }
        else if (input->dwOfs == DIJOFS_BUTTON14)
        {
            if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // exit
            {
                if (started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled) // Check if we have started a game yet.
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
            switch (input->dwOfs)
            {
                case DIJOFS_BUTTON15: Joy(16, updown, start); break;
                case DIJOFS_BUTTON16: Joy(17, updown, start); break;
                case DIJOFS_BUTTON17: Joy(18, updown, start); break;
                case DIJOFS_BUTTON18: Joy(19, updown, start); break;
                case DIJOFS_BUTTON19: Joy(20, updown, start); break;
                case DIJOFS_BUTTON20: Joy(21, updown, start); break;
                case DIJOFS_BUTTON21: Joy(22, updown, start); break;
                case DIJOFS_BUTTON22: Joy(23, updown, start); break;
                case DIJOFS_BUTTON23: Joy(24, updown, start); break;
                case DIJOFS_BUTTON24: Joy(25, updown, start); break;
                case DIJOFS_BUTTON25: Joy(26, updown, start); break;
                case DIJOFS_BUTTON26: Joy(27, updown, start); break;
                case DIJOFS_BUTTON27: Joy(28, updown, start); break;
                case DIJOFS_BUTTON28: Joy(29, updown, start); break;
                case DIJOFS_BUTTON29: Joy(30, updown, start); break;
                case DIJOFS_BUTTON30: Joy(31, updown, start); break;
                case DIJOFS_BUTTON31: Joy(32, updown, start); break;
            }
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
        if ((deadu<0) && (deadu < -m_deadz))
            deadu += m_deadz;
        if ((deadu > 0) && (deadu>m_deadz))
            deadu -= m_deadz;

        switch (input->dwOfs)	// Axis, Sliders and POV
        {	// with selectable axes added to menu, giving prioity in this order... X Axis (the Left/Right Axis), Y Axis
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
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? deadu : -deadu, joyk);
                        if ((m_lr_axis == 1) && (uShockType == USHOCKTYPE_GENERIC))
                            // giving L/R Axis priority over U/D Axis incase both are assigned to same axis
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if ((m_ud_axis == 1) && (uShockType == USHOCKTYPE_GENERIC))
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 1)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
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
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                        if ((m_lr_axis == 2) && (uShockType == USHOCKTYPE_GENERIC))
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if ((m_ud_axis == 2) && (uShockType == USHOCKTYPE_GENERIC))
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 2)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    }
                }
                break;
            }

            case DIJOFS_Z:
            {
                if (g_pplayer)
                {
                    if (uShockType == USHOCKTYPE_ULTRACADE)
                        g_pplayer->mechPlungerIn((int)input->dwData);
                    if (((m_plunger_axis != 6) && (m_plunger_axis != 0)) || (m_override_default_buttons == 0))
                    {											// with the ability to use rZ for plunger, checks to see if
                        if (uShockType == USHOCKTYPE_PBWIZARD) // the override is used and if so, if Plunger is set to Rz or
                        {										// disabled. If override isn't used, uses default assignment
                            g_pplayer->mechPlungerIn(-(int)input->dwData); // of the Z axis.
                        }
                    }
                    if ((uShockType == USHOCKTYPE_VIRTUAPIN) && (m_plunger_axis != 0))
                        g_pplayer->mechPlungerIn(-(int)input->dwData);
                    if (((m_lr_axis == 3) || (m_ud_axis == 3)) && (uShockType == USHOCKTYPE_GENERIC))
                    {  // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 3)
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 3)
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 3)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    }
                }
                break;
            }

            case DIJOFS_RX:
            {
                if (g_pplayer)
                {
                    if (((m_lr_axis == 4) || (m_ud_axis == 4)) && (uShockType == USHOCKTYPE_GENERIC))
                    {  // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 4)
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 4)
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 4)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    }
                }
                break;
            }

            case DIJOFS_RY:
            {
                if (g_pplayer)
                {
                    if (((m_lr_axis == 5) || (m_ud_axis == 5)) && (uShockType == USHOCKTYPE_GENERIC))
                    {  // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 5)
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 5)
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 5)
                    {	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    }
                }
                break;
            }

            case DIJOFS_RZ:
            {
                if (g_pplayer)
                {
                    if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 1) && (m_plunger_axis == 6))
                        g_pplayer->mechPlungerIn((int)input->dwData);
                    if (((m_lr_axis == 6) || (m_ud_axis == 6)) && (uShockType == USHOCKTYPE_GENERIC))
                    {  // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 6)
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 6)
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 6)
                    {
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    }
                }
                break;
            }

            case DIJOFS_SLIDER(0):
            {
                if (g_pplayer)
                {
                    if (uShockType == USHOCKTYPE_SIDEWINDER)
                        g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    if (((m_lr_axis == 7) || (m_ud_axis == 7)) && (uShockType == USHOCKTYPE_GENERIC))
                    {  // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 7)
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 7)
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 7)
                    {
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
                    }
                }
                break;
            }

            case DIJOFS_SLIDER(1):
            {
                if (g_pplayer)
                {
                    if (((m_lr_axis == 8) || (m_ud_axis == 8)) && (uShockType == USHOCKTYPE_GENERIC))
                    {  // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
                        // Axis Deadzone
                        if (m_lr_axis == 8)
                            g_pplayer->NudgeX((m_lr_axis_reverse == 0) ? -deadu : deadu, joyk);
                        else if (m_ud_axis == 8)
                            g_pplayer->NudgeY((m_ud_axis_reverse == 0) ? deadu : -deadu, joyk);
                    }
                    else if (m_plunger_axis == 8)
                    {
                        if (uShockType == USHOCKTYPE_GENERIC)
                            g_pplayer->mechPlungerIn((m_plunger_reverse == 0) ? -(int)input->dwData : (int)input->dwData);
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

   if (curr_time_msec >= 0)
   {
      // Check if autostart is enabled.
      if (g_pplayer->m_ptable->m_tblAutoStartEnabled)
         // Update autostart.
         autostart(g_pplayer->m_ptable->m_tblAutoStart, g_pplayer->m_ptable->m_tblAutoStartRetry, curr_time_msec);

      button_exit(g_pplayer->m_ptable->m_tblExitConfirm, curr_time_msec);

      // Update tilt.
      tilt_update();
   }
   else
      curr_time_msec = -curr_time_msec; // due to special encoding to not do the stuff above

   // Check if we've been initialized.
   if (m_firedautostart == 0)
      m_firedautostart = curr_time_msec;

   GetInputDeviceData(/*curr_time_msec*/);

   // Camera/Light tweaking mode (F6) incl. fly-around parameters
   if (g_pplayer->cameraMode)
   {
       if (m_head == m_tail) // key queue empty, so just continue using the old pressed key
       {
         if ((curr_time_msec - m_nextKeyPressedTime) > 10) // reduce update rate
         {
           m_nextKeyPressedTime = curr_time_msec;

           // Flying
		   if(m_cameraMode == 1)
		   {
			   if (!m_cameraModeAltKey)
				   g_pplayer->m_pin3d.m_camy += 10.0f;
			   else
				   g_pplayer->m_pin3d.m_camz += 10.0f;
		   }
		   else if(m_cameraMode == 2)
		   {
			   if (!m_cameraModeAltKey)
				   g_pplayer->m_pin3d.m_camy -= 10.0f;
			   else
				   g_pplayer->m_pin3d.m_camz -= 10.0f;
		   }
		   else if(m_cameraMode == 3)
		   {
			   if (!m_cameraModeAltKey)
				   g_pplayer->m_pin3d.m_camx -= 10.0f;
			   else
				   g_pplayer->m_pin3d.m_inc -= 0.01f;
		   }
		   else if(m_cameraMode == 4)
		   {
			   if (!m_cameraModeAltKey)
				   g_pplayer->m_pin3d.m_camx += 10.0f;
			   else
				   g_pplayer->m_pin3d.m_inc += 0.01f;
           }

           // Table tweaks
           if (m_keyPressedState[eLeftFlipperKey])
               g_pplayer->UpdateBackdropSettings(false);
           if (m_keyPressedState[eRightFlipperKey])
               g_pplayer->UpdateBackdropSettings(true);
           if (m_enableCameraModeFlyAround)
           {
              if (m_keyPressedState[eLeftTiltKey])
                 g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] -= 1.0f;
              if (m_keyPressedState[eRightTiltKey])
                 g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] += 1.0f;
           }
         }

         return;
      }
   }

   const DIDEVICEOBJECTDATA * __restrict input;
   while (input = GetTail(/*curr_sim_msec*/))
   {
      if (input->dwSequence == APP_MOUSE && g_pplayer)
      {
         if(g_pplayer->m_fThrowBalls)
         {
             ProcessThrowBalls(input);
         }
         else
         {
            if (input->dwOfs == 1 && m_joylflipkey==25)
            {
               FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eLeftFlipperKey]);
            }
            if(input->dwOfs == 2 && m_joyrflipkey == 26)
            {
               FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[eRightFlipperKey]);
            }
            if(input->dwOfs == 3 && m_joyplungerkey == 27)
            {
               FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, (DWORD)g_pplayer->m_rgKeys[ePlungerKey]);
            }
         }
      }

      if (input->dwSequence == APP_KEYBOARD)
      {
         // Camera mode fly around:
         if (g_pplayer && g_pplayer->cameraMode && m_enableCameraModeFlyAround)
              ProcessCameraKeys(input);

         // Normal game keys:
         if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eFrameCount])
         {
            if ((input->dwData & 0x80) != 0)
               g_pplayer->ToggleFPS();
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eEnable3D])
         {
            if ((input->dwData & 0x80) != 0)
            {
               g_pplayer->m_fStereo3Denabled = !g_pplayer->m_fStereo3Denabled;
               SetRegValueBool("Player", "Stereo3DEnabled", g_pplayer->m_fStereo3Denabled);
            }
         }
         else if (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eDBGBalls])
         {
            // Activate on edge only.
            if ((input->dwData & 0x80) != 0)
            {
               g_pplayer->m_DebugBalls = !(g_pplayer->m_DebugBalls);
               g_pplayer->m_ToggleDebugBalls = true;
            }
         }
         else if(input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eDebugger])
         {
             if(started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled)
             {
                 if ((input->dwData & 0x80) != 0)
                 { //on key down only
                     m_first_stamp = curr_time_msec;
                     m_exit_stamp = curr_time_msec;
                 }
                 else
                 { //on key up only
                     m_exit_stamp = 0;
                     g_pplayer->m_fShowDebugger = true;
                 }
             }
         }
         else if(((input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eEscape]) && (m_disable_esc == 0)) || (input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eExitGame]))
         {
            // Check if we have started a game yet.
            if (started() || !g_pplayer->m_ptable->m_tblAutoStartEnabled)
            {
               if (input->dwData & 0x80) { //on key down only
                  m_first_stamp = curr_time_msec;
                  m_exit_stamp = curr_time_msec;
               }
               else
               { //on key up only
                  m_exit_stamp = 0;
                  g_pplayer->m_fCloseDown = true;
               }
            }
         }
         else
         {
            FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, input->dwOfs);
         }
      }
      else if (input->dwSequence >= APP_JOYSTICKMN && input->dwSequence <= APP_JOYSTICKMX)
          ProcessJoystick(input, curr_time_msec);

   }
}

int PinInput::GetNextKey() // return last valid keyboard key 
{
#ifdef USE_DINPUT_FOR_KEYBOARD
   if (m_pKeyboard != NULL)
   {
      DIDEVICEOBJECTDATA didod[1];  // Receives buffered data
      DWORD dwElements;
      HRESULT hr;
#ifdef USE_DINPUT8
      LPDIRECTINPUTDEVICE8 pkyb = m_pKeyboard;
#else
      LPDIRECTINPUTDEVICE pkyb = m_pKeyboard;
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
