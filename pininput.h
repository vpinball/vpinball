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

class PinInput
{
public:
   PinInput();
   ~PinInput();

   void Init(const HWND hwnd);
   void UnInit();

   // implicitly sync'd with visuals as each keystroke is applied to the sim
   void FireKeyEvent(const int dispid, const int keycode);
   int  QueueFull() const;
   int  QueueEmpty() const;
   void AdvanceHead();
   void AdvanceTail();

   void PushQueue(DIDEVICEOBJECTDATA * const data, const unsigned int app_data/*, const U32 curr_time_msec*/);
   const DIDEVICEOBJECTDATA *GetTail( /*const U32 curr_sim_msec*/);

   void autostart(const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec);
   void button_exit(const U32 msecs, const U32 curr_time_msec);

   void tilt_update();

   void ProcessKeys(PinTable * const ptable/*, const U32 curr_sim_msec*/, const U32 curr_time_msec);

   int GetNextKey();

   void GetInputDeviceData(/*const U32 curr_time_msec*/);

#if 0
   U32 Pressed ( const U32 mask ) const;
   U32 Released( const U32 mask ) const;
   U32 Held    ( const U32 mask ) const;
   U32 Changed ( const U32 mask ) const;
#endif
   U32 Down(const U32 mask) const; //!! only still used by mixer

#ifdef VP10
   LPDIRECTINPUT8       m_pDI;
   LPDIRECTINPUTDEVICE8 m_pJoystick[PININ_JOYMXCNT];
#else
   LPDIRECTINPUT        m_pDI;
   LPDIRECTINPUTDEVICE  m_pJoystick[PININ_JOYMXCNT];
#endif

   HWND m_hwnd;

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

#ifdef VP10
   LPDIRECTINPUTDEVICE8 m_pKeyboard;
   LPDIRECTINPUTDEVICE8 m_pMouse;
#else
   LPDIRECTINPUTDEVICE m_pKeyboard;
   LPDIRECTINPUTDEVICE m_pMouse;
#endif

   U32 m_PreviousKeys;	// Masks of PININ_* inputs used by ultracade - AMH
   U32 m_ChangedKeys;

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

   PinTable *m_ptable;

   int m_plunger_axis, m_lr_axis, m_ud_axis, m_plunger_reverse, m_lr_axis_reverse, m_ud_axis_reverse, m_override_default_buttons, m_disable_esc;
   int m_joylflipkey, m_joyrflipkey, m_joylmagnasave, m_joyrmagnasave, m_joyplungerkey, m_joystartgamekey, m_joyexitgamekey, m_joyaddcreditkey;
   int m_joyaddcreditkey2, m_joyframecount, m_joyvolumeup, m_joyvolumedown, m_joylefttilt, m_joycentertilt, m_joyrighttilt, m_joypmbuyin;
   int m_joypmcoin3, m_joypmcoin4, m_joypmcoindoor, m_joypmcancel, m_joypmdown, m_joypmup, m_joypmenter, m_joydebug, m_joymechtilt;
   int m_joycustom1, m_joycustom1key, m_joycustom2, m_joycustom2key, m_joycustom3, m_joycustom3key, m_joycustom4, m_joycustom4key;
   int m_deadz;
   bool m_enableMouseInPlayer;
};

// - Added by AMH --modified by Koadic (not sure if necessary, but doesn't hurt anything)
#define PININ_LEFT           0x00000001
#define PININ_RIGHT          0x00000002
#define PININ_LEFT2          0x00000004
#define PININ_RIGHT2         0x00000008
#define PININ_PLUNGE         0x00000010
#define PININ_START          0x00000020
#define PININ_BUYIN          0x00000040
#define PININ_COIN1          0x00000080
#define PININ_COIN2		     0x00000100
#define PININ_COIN3          0x00000200
#define PININ_COIN4          0x00000400
#define PININ_SERVICECANCEL  0x00000800
#define PININ_SERVICE1       0x00001000
#define PININ_SERVICE2       0x00002000
#define PININ_SERVICEENTER   0x00004000
#define PININ_COINDOOR       0x00008000
#define PININ_VOL_UP         0x00010000
#define PININ_VOL_DOWN       0x00020000
#define PININ_EXITGAME       0x00040000
#define PININ_LTILT          0x00080000
#define PININ_CTILT          0x00100000
#define PININ_RTILT          0x00200000
#define PININ_MTILT          0x00400000
#define PININ_FRAMES         0x00800000
#define PININ_ENABLE3D       0x01000000

#define PININ_ANY            0xffffffff
// - end input routines added by AMH

static const unsigned char VK_TO_DIK[][2] =
{
   { VK_BACK, DIK_BACK },
   { VK_TAB, DIK_TAB },
   { VK_CLEAR, DIK_NUMPAD5 },      /* Num Lock off */
   { VK_RETURN, DIK_NUMPADENTER },
   { VK_SHIFT, DIK_LSHIFT },
   { VK_CONTROL, DIK_LCONTROL },
   { VK_MENU, DIK_LMENU },
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
   for (unsigned int i = 0; i < (sizeof(VK_TO_DIK) / sizeof(VK_TO_DIK[0])); ++i)
      if (VK_TO_DIK[i][1] == dik)
         return VK_TO_DIK[i][0];

   return ~0u;
}
