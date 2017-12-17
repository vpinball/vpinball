#pragma once

#define MAX_KEYQUEUE_SIZE 32

#if MAX_KEYQUEUE_SIZE & (MAX_KEYQUEUE_SIZE-1)
#error Note that MAX_KEYQUEUE_SIZE must be power of 2
#endif

#define USHOCKTYPE_PBWIZARD		1
#define USHOCKTYPE_ULTRACADE	2
#define USHOCKTYPE_SIDEWINDER	3
#define USHOCKTYPE_VIRTUAPIN	4
#define USHOCKTYPE_GENERIC		5

#define APP_KEYBOARD 0
#define APP_JOYSTICKMN 1
#define APP_MOUSE 2

// handle multiple joysticks, APP_JOYSTICKMN..APP_JOYSTICKMX
#define PININ_JOYMXCNT 4

#define JOYRANGEMN (-65536)
#define JOYRANGEMX (+65536)

#define JOYRANGE ((JOYRANGEMX) - (JOYRANGEMN) + 1)

#define APP_JOYSTICKMX (APP_JOYSTICKMN + PININ_JOYMXCNT -1)
#define APP_JOYSTICK(n) (APP_JOYSTICKMN + n)

#define USE_DINPUT_FOR_KEYBOARD // can lead to less input lag maybe on some systems if disabled, but can miss input if key is only pressed very very quickly and/or FPS are low

class PinInput
{
public:
   PinInput();
   void LoadSettings();
   ~PinInput() {}

   void Init(const HWND hwnd);
   void UnInit();

   // implicitly sync'd with visuals as each keystroke is applied to the sim
   void FireKeyEvent(const int dispid, int keycode);

   void PushQueue(DIDEVICEOBJECTDATA * const data, const unsigned int app_data/*, const U32 curr_time_msec*/);
   const DIDEVICEOBJECTDATA *GetTail(/*const U32 curr_sim_msec*/);

   void autostart(const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec);
   void button_exit(const U32 msecs, const U32 curr_time_msec);

   void tilt_update();

   void ProcessCameraKeys(const DIDEVICEOBJECTDATA * __restrict input);
   void ProcessKeys(/*const U32 curr_sim_msec,*/ int curr_time_msec);

   void ProcessJoystick(const DIDEVICEOBJECTDATA * __restrict input, int curr_time_msec);

   void ProcessThrowBalls(const DIDEVICEOBJECTDATA * __restrict input);
   int GetNextKey();

   void GetInputDeviceData(/*const U32 curr_time_msec*/);

#ifdef USE_DINPUT8
   LPDIRECTINPUT8       m_pDI;
   LPDIRECTINPUTDEVICE8 m_pJoystick[PININ_JOYMXCNT];
#else
   LPDIRECTINPUT        m_pDI;
   LPDIRECTINPUTDEVICE  m_pJoystick[PININ_JOYMXCNT];
#endif

   HWND m_hwnd;

   UINT64 m_leftkey_down_usec;
   unsigned int m_leftkey_down_frame;
   UINT64 m_leftkey_down_usec_rotate_to_end;
   unsigned int m_leftkey_down_frame_rotate_to_end;
   UINT64 m_leftkey_down_usec_EOS;
   unsigned int m_leftkey_down_frame_EOS;

   int e_JoyCnt;
   int uShockDevice;	// only one uShock device
   int uShockType;

   int mouseX;
   int mouseY;
   long mouseDX;
   long mouseDY;
   bool leftMouseButtonDown;
   bool rightMouseButtonDown;
   bool middleMouseButtonDown;
   BYTE oldMouseButtonState[3];
   bool m_linearPlunger;
   bool m_enable_nudge_filter; // enable new nudge filtering code

private:
   int started();
   void Joy(const unsigned int n, const int updown, const bool start);

   //int InputControlRun;

#ifdef USE_DINPUT8
#ifdef USE_DINPUT_FOR_KEYBOARD
   LPDIRECTINPUTDEVICE8 m_pKeyboard;
#endif
   LPDIRECTINPUTDEVICE8 m_pMouse;
#else
#ifdef USE_DINPUT_FOR_KEYBOARD
   LPDIRECTINPUTDEVICE m_pKeyboard;
#endif
   LPDIRECTINPUTDEVICE m_pMouse;
#endif

   U32 m_firedautostart;

   int m_pressed_start;

   U32 m_first_stamp;
   U32 m_exit_stamp;

   int m_as_down;
   int m_as_didonce;

   int m_tilt_updown;

   DIDEVICEOBJECTDATA m_diq[MAX_KEYQUEUE_SIZE]; // circular queue of direct input events

   STICKYKEYS m_StartupStickyKeys;

   int m_head; // head==tail means empty, (head+1)%MAX_KEYQUEUE_SIZE == tail means full

   int m_tail; // These are integer indices into keyq and should be in domain of 0..MAX_KEYQUEUE_SIZE-1

   int m_plunger_axis, m_lr_axis, m_ud_axis, m_plunger_reverse, m_lr_axis_reverse, m_ud_axis_reverse, m_override_default_buttons, m_disable_esc;
   int m_joylflipkey, m_joyrflipkey, m_joylmagnasave, m_joyrmagnasave, m_joyplungerkey, m_joystartgamekey, m_joyexitgamekey, m_joyaddcreditkey;
   int m_joyaddcreditkey2, m_joyframecount, m_joyvolumeup, m_joyvolumedown, m_joylefttilt, m_joycentertilt, m_joyrighttilt, m_joypmbuyin;
   int m_joypmcoin3, m_joypmcoin4, m_joypmcoindoor, m_joypmcancel, m_joypmdown, m_joypmup, m_joypmenter, m_joydebugballs, m_joydebugger, m_joylockbar, m_joymechtilt;
   int m_joycustom1, m_joycustom1key, m_joycustom2, m_joycustom2key, m_joycustom3, m_joycustom3key, m_joycustom4, m_joycustom4key;
   int m_deadz;
   bool m_enableMouseInPlayer;

   bool m_cameraModeAltKey;
   bool m_enableCameraModeFlyAround;

   int m_cameraMode;
   bool m_keyPressedState[4];
   DWORD m_nextKeyPressedTime;
};

#define VK_TO_DIK_SIZE 105
static const unsigned char VK_TO_DIK[VK_TO_DIK_SIZE][2] =
{
   { VK_BACK, DIK_BACK },
   { VK_TAB, DIK_TAB },
   //{ VK_CLEAR, DIK_NUMPAD5 },      /* Num Lock off */
   { VK_RETURN, DIK_RETURN },
   { VK_RETURN, DIK_NUMPADENTER },
   { VK_LSHIFT, DIK_LSHIFT },
   { VK_RSHIFT, DIK_RSHIFT },
   { VK_LCONTROL, DIK_LCONTROL },
   { VK_RCONTROL, DIK_RCONTROL },
   { VK_LMENU, DIK_LMENU },
   { VK_RMENU, DIK_RMENU },
   { VK_CAPITAL, DIK_CAPITAL },
   { VK_ESCAPE, DIK_ESCAPE },
   { VK_SPACE, DIK_SPACE },
   { VK_PRIOR, DIK_PRIOR },
   { VK_NEXT, DIK_NEXT },
   { VK_END, DIK_END },
   { VK_HOME, DIK_HOME },
   { VK_LEFT, DIK_LEFT },
   { VK_UP, DIK_UP },
   { VK_RIGHT, DIK_RIGHT },
   { VK_DOWN, DIK_DOWN },
   { VK_INSERT, DIK_INSERT },
   { VK_DELETE, DIK_DELETE },
   { '0', DIK_0 },
   { '1', DIK_1 },
   { '2', DIK_2 },
   { '3', DIK_3 },
   { '4', DIK_4 },
   { '5', DIK_5 },
   { '6', DIK_6 },
   { '7', DIK_7 },
   { '8', DIK_8 },
   { '9', DIK_9 },
   { 'A', DIK_A },
   { 'B', DIK_B },
   { 'C', DIK_C },
   { 'D', DIK_D },
   { 'E', DIK_E },
   { 'F', DIK_F },
   { 'G', DIK_G },
   { 'H', DIK_H },
   { 'I', DIK_I },
   { 'J', DIK_J },
   { 'K', DIK_K },
   { 'L', DIK_L },
   { 'M', DIK_M },
   { 'N', DIK_N },
   { 'O', DIK_O },
   { 'P', DIK_P },
   { 'Q', DIK_Q },
   { 'R', DIK_R },
   { 'S', DIK_S },
   { 'T', DIK_T },
   { 'U', DIK_U },
   { 'V', DIK_V },
   { 'W', DIK_W },
   { 'X', DIK_X },
   { 'Y', DIK_Y },
   { 'Z', DIK_Z },
   { VK_LWIN, DIK_LWIN },
   { VK_RWIN, DIK_RWIN },
   { VK_APPS, DIK_APPS },
   { VK_NUMPAD0, DIK_NUMPAD0 },
   { VK_NUMPAD1, DIK_NUMPAD1 },
   { VK_NUMPAD2, DIK_NUMPAD2 },
   { VK_NUMPAD3, DIK_NUMPAD3 },
   { VK_NUMPAD4, DIK_NUMPAD4 },
   { VK_NUMPAD5, DIK_NUMPAD5 },      /* Num Lock on */
   { VK_NUMPAD6, DIK_NUMPAD6 },
   { VK_NUMPAD7, DIK_NUMPAD7 },
   { VK_NUMPAD8, DIK_NUMPAD8 },
   { VK_NUMPAD9, DIK_NUMPAD9 },
   { VK_MULTIPLY, DIK_MULTIPLY },
   { VK_ADD, DIK_ADD },
   { VK_SUBTRACT, DIK_SUBTRACT },
   { VK_DECIMAL, DIK_DECIMAL },
   { VK_DIVIDE, DIK_DIVIDE },
   { VK_F1, DIK_F1 },
   { VK_F2, DIK_F2 },
   { VK_F3, DIK_F3 },
   { VK_F4, DIK_F4 },
   { VK_F5, DIK_F5 },
   { VK_F6, DIK_F6 },
   { VK_F7, DIK_F7 },
   { VK_F8, DIK_F8 },
   { VK_F9, DIK_F9 },
   { VK_F10, DIK_F10 },
   { VK_F11, DIK_F11 },
   { VK_F12, DIK_F12 },
   { VK_F13, DIK_F13 },
   { VK_F14, DIK_F14 },
   { VK_F15, DIK_F15 },
   { VK_NUMLOCK, DIK_NUMLOCK },
   { VK_SCROLL, DIK_SCROLL },
   { VK_OEM_1, DIK_SEMICOLON },
   { VK_OEM_PLUS, DIK_EQUALS },
   { VK_OEM_COMMA, DIK_COMMA },
   { VK_OEM_MINUS, DIK_MINUS },
   { VK_OEM_PERIOD, DIK_PERIOD },
   { VK_OEM_2, DIK_SLASH },
   { VK_OEM_3, DIK_GRAVE },
   { VK_OEM_4, DIK_LBRACKET },
   { VK_OEM_5, DIK_BACKSLASH },
   { VK_OEM_6, DIK_RBRACKET },
   { VK_OEM_7, DIK_APOSTROPHE }
};

inline unsigned int get_vk(const unsigned int dik)
{
   for (unsigned int i = 0; i < VK_TO_DIK_SIZE; ++i)
      if (VK_TO_DIK[i][1] == dik)
         return VK_TO_DIK[i][0];

   return ~0u;
}

inline unsigned int get_dik(const unsigned int vk)
{
   for (unsigned int i = 0; i < VK_TO_DIK_SIZE; ++i)
      if (VK_TO_DIK[i][0] == vk)
         return VK_TO_DIK[i][1];

   return ~0u;
}
