#pragma once

#include <list>

#ifdef _WIN32
#define ENABLE_XINPUT
#endif

#ifdef ENABLE_XINPUT
#include <XInput.h>
#pragma comment(lib, "XInput.lib")
#endif

#ifdef ENABLE_SDL_INPUT //!! test
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
//#define ENABLE_SDL_GAMECONTROLLER //!! test
#endif

#ifdef ENABLE_IGAMECONTROLLER //!! not implemented yet
#include "windows.gaming.input.h"
#pragma comment(lib, "runtimeobject.lib")
#endif

#define MAX_KEYQUEUE_SIZE 32

#if MAX_KEYQUEUE_SIZE & (MAX_KEYQUEUE_SIZE-1)
#error Note that MAX_KEYQUEUE_SIZE must be power of 2
#endif

#define USHOCKTYPE_PBWIZARD   1
#define USHOCKTYPE_ULTRACADE  2
#define USHOCKTYPE_SIDEWINDER 3
#define USHOCKTYPE_VIRTUAPIN  4
#define USHOCKTYPE_GENERIC    5

// Input type ID - keyboard
#define APP_KEYBOARD   0

// Input type ID - mouse
#define APP_MOUSE      1

// Input type ID - touchscreen
#define APP_TOUCH      2

// Input type ID - joystick 1 through 8
// handle multiple joysticks, APP_JOYSTICKMN..APP_JOYSTICKMX
#define APP_JOYSTICKMN 3
#define PININ_JOYMXCNT 4
#define APP_JOYSTICK(n) (APP_JOYSTICKMN + (n))
#define APP_JOYSTICKMX  (APP_JOYSTICK(PININ_JOYMXCNT - 1))


// Joystick axis normalized input range
#define JOYRANGEMN (-65536)
#define JOYRANGEMX (+65536)
#define JOYRANGE ((JOYRANGEMX) - (JOYRANGEMN) + 1)


#ifdef _WIN32
#define USE_DINPUT_FOR_KEYBOARD // can lead to less input lag maybe on some systems if disabled, but can miss input if key is only pressed very very quickly and/or FPS are low
#endif

class PinInput
{
public:
   PinInput();
   void LoadSettings(const Settings& settings);
   ~PinInput();

   void Init(const HWND hwnd);
   void UnInit();

   // implicitly sync'd with visuals as each keystroke is applied to the sim
   void FireKeyEvent(const int dispid, int keycode);

   void PushQueue(DIDEVICEOBJECTDATA * const data, const unsigned int app_data/*, const U32 curr_time_msec*/);
   const DIDEVICEOBJECTDATA *GetTail(/*const U32 curr_sim_msec*/);

   void ProcessCameraKeys(const DIDEVICEOBJECTDATA * __restrict input);
   void ProcessKeys(/*const U32 curr_sim_msec,*/ int curr_time_msec);

   void ProcessJoystick(const DIDEVICEOBJECTDATA * __restrict input, int curr_time_msec);

   void ProcessThrowBalls(const DIDEVICEOBJECTDATA * __restrict input);
   void ProcessBallControl(const DIDEVICEOBJECTDATA * __restrict input);

   // Speed: 0..1
   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration);

   int GetNextKey();

   void GetInputDeviceData(/*const U32 curr_time_msec*/);

#ifdef _WIN32
#ifdef USE_DINPUT8
   LPDIRECTINPUT8       m_pDI;
   LPDIRECTINPUTDEVICE8 m_pJoystick[PININ_JOYMXCNT];
#else
   LPDIRECTINPUT        m_pDI;
   LPDIRECTINPUTDEVICE  m_pJoystick[PININ_JOYMXCNT];
#endif
#endif

   HWND m_hwnd;

   uint64_t m_leftkey_down_usec;
   unsigned int m_leftkey_down_frame;
   uint64_t m_leftkey_down_usec_rotate_to_end;
   unsigned int m_leftkey_down_frame_rotate_to_end;
   uint64_t m_leftkey_down_usec_EOS;
   unsigned int m_leftkey_down_frame_EOS;
   uint64_t m_lastclick_ballcontrol_usec;

   int m_num_joy;
   int uShockType;

   bool m_mixerKeyDown;
   bool m_mixerKeyUp;

   bool m_linearPlunger;
   bool m_plunger_retract; // enable 1s retract phase for button/key plunger
   bool m_enable_nudge_filter; // enable new nudge filtering code

   int m_joycustom1key, m_joycustom2key, m_joycustom3key, m_joycustom4key;

private:
   int Started();

   void Autostart(const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec);
   void ButtonExit(const U32 msecs, const U32 curr_time_msec);

   void TiltUpdate();

   void Joy(const unsigned int n, const int updown, const bool start);

   void InitOpenPinballDevices();
   void ReadOpenPinballDevices(const U32 cur_time_msec);

   void HandleInputDI(DIDEVICEOBJECTDATA *didod);
   void HandleInputXI(DIDEVICEOBJECTDATA *didod);
   void HandleInputSDL(DIDEVICEOBJECTDATA *didod);
   void HandleInputIGC(DIDEVICEOBJECTDATA *didod);

#ifdef _WIN32
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
#endif

   // Open Pinball Device input report v1.0 - input report structure.
   // Fields are packed with no padding bytes; integer fields are little-endian.
   struct __pragma(pack(push, 1)) OpenPinballDeviceReport
   {
      uint64_t timestamp; // report time, in microseconds since an arbitrary zero point
      uint32_t genericButtons; // button states for 32 general-purpose on/off buttons
      uint32_t pinballButtons; // button states for pre-defined pinball simulator function buttons
      uint8_t llFlipper; // lower left flipper button duty cycle
      uint8_t lrFlipper; // lower right flipper button duty cycle
      uint8_t ulFlipper; // upper left flipper button duty cycle
      uint8_t urFlipper; // upper right flipper button duty cycle
      int16_t axNudge; // instantaneous nudge acceleration, X axis (left/right)
      int16_t ayNudge; // instantaneous nudge acceleration, Y axis (front/back)
      int16_t vxNudge; // instantaneous nudge velocity, X axis
      int16_t vyNudge; // instantaneous nudge velocity, Y axis
      int16_t plungerPos; // current plunger position
      int16_t plungerSpeed; // instantaneous plunger speed
   }
   __pragma(pack(pop));

   // active Open Pinball Device interfaces
   struct OpenPinDev
   {
      OpenPinDev(HANDLE h, BYTE reportID, DWORD reportSize, const wchar_t *deviceStructVersionString);
      ~OpenPinDev();
      HANDLE Release()
      {
         HANDLE ret = hDevice;
         hDevice = INVALID_HANDLE_VALUE;
         return ret;
      }
      HANDLE hDevice;
      BYTE reportID;
      DWORD deviceStructVersion;

      // overlapped read
      OVERLAPPED ov { 0 };
      HANDLE hIOEvent { CreateEvent(NULL, TRUE, FALSE, NULL) };
      DWORD readStatus = 0;
      DWORD bytesRead = 0;

      // overlapped read buffer - space for the HID report ID prefix and the report struct
      size_t reportSize;
      std::vector<BYTE> buf;

      // start an overlapped read
      void StartRead();

      // read a report into 'r'; returns true if a new report was available
      bool ReadReport();

      // last report read
      OpenPinballDeviceReport r;
   };
   std::list<OpenPinDev> m_openPinDevs;

   // Open Pinball Device button status, for detecting button up/down events
   uint32_t m_openPinDev_generic_buttons;
   uint32_t m_openPinDev_pinball_buttons;
   bool m_openPinDev_flipper_l, m_openPinDev_flipper_r;

   int m_mouseX;
   int m_mouseY;
   int m_mouseDX;
   int m_mouseDY;
   bool m_leftMouseButtonDown;
   bool m_rightMouseButtonDown;
   bool m_middleMouseButtonDown;
   BYTE m_oldMouseButtonState[3];

   U32 m_firedautostart;

   U32 m_first_stamp;
   U32 m_exit_stamp;

   bool m_pressed_start;

   bool m_as_down;
   bool m_as_didonce;

   bool m_tilt_updown;

   DIDEVICEOBJECTDATA m_diq[MAX_KEYQUEUE_SIZE]; // circular queue of direct input events

#ifdef _WIN32
   STICKYKEYS m_startupStickyKeys;
#endif

   int m_head; // head==tail means empty, (head+1)%MAX_KEYQUEUE_SIZE == tail means full

   int m_tail; // These are integer indices into keyq and should be in domain of 0..MAX_KEYQUEUE_SIZE-1

   // Axis assignments - these map to the drop-list index in the axis
   // selection combos in the Keys dialog:
   //
   //   0 = Disabled
   //   1 = X
   //   2 = Y
   //   3 = Z
   //   4 = rX
   //   5 = rY
   //   6 = rZ
   //   7 = Slider 1
   //   8 = Slider 2
   //   9 = Open Pinball Device (selects input mapping to the same function as assigned axis)
   //
   int m_plunger_axis, m_plunger_speed_axis, m_lr_axis, m_ud_axis;
   int m_joylflipkey, m_joyrflipkey, m_joystagedlflipkey, m_joystagedrflipkey, m_joylmagnasave, m_joyrmagnasave, m_joyplungerkey, m_joystartgamekey, m_joyexitgamekey, m_joyaddcreditkey;
   int m_joyaddcreditkey2, m_joyframecount, m_joyvolumeup, m_joyvolumedown, m_joylefttilt, m_joycentertilt, m_joyrighttilt, m_joypmbuyin;
   int m_joypmcoin3, m_joypmcoin4, m_joypmcoindoor, m_joypmcancel, m_joypmdown, m_joypmup, m_joypmenter, m_joydebugballs, m_joydebugger, m_joylockbar, m_joymechtilt;
   int m_joycustom1, m_joycustom2, m_joycustom3, m_joycustom4;
   int m_joytablerecenter, m_joytableup, m_joytabledown, m_joypause, m_joytweak;
   int m_deadz;
   bool m_override_default_buttons, m_plunger_reverse, m_disable_esc, m_lr_axis_reverse, m_ud_axis_reverse;
   bool m_enableMouseInPlayer;

   bool m_cameraModeAltKey;
   bool m_enableCameraModeFlyAround;

   int m_cameraMode;
   bool m_keyPressedState[28]; // =EnumAssignKeys::eCKeys
   DWORD m_nextKeyPressedTime;

   int m_inputApi;   // 0=DirectInput 1=XInput, 2=SDL, 3=IGamecontroller
   int m_rumbleMode; // 0=Off, 1=Table only, 2=Generic only, 3=Table with generic as fallback

   static const int m_LeftMouseButtonID = 25;
   static const int m_RightMouseButtonID = 26;
   static const int m_MiddleMouseButtonID = 27;

#ifdef _WIN32
#ifdef ENABLE_XINPUT
   int m_inputDeviceXI;
   XINPUT_STATE m_inputDeviceXIstate;
   DWORD m_rumbleOffTime;
   bool m_rumbleRunning;
#endif
#endif
#ifdef ENABLE_SDL_INPUT
#ifndef ENABLE_SDL_GAMECONTROLLER
   SDL_Joystick* m_inputDeviceSDL;
   SDL_Haptic* m_rumbleDeviceSDL;
#else
   SDL_GameController* m_gameController;
   void SetupSDLGameController();
#endif
#endif
#ifdef ENABLE_IGAMECONTROLLER
#endif
};

#define VK_TO_DIK_SIZE 105
#ifdef _WIN32
static constexpr unsigned char VK_TO_DIK[VK_TO_DIK_SIZE][2] =
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
#endif
