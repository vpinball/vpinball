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

// Open Pinball Device context (defined in the OPD implementation module)
class OpenPinDevContext;

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
   void TerminateOpenPinballDevices();

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

   // Open Pinball Device context.  This is an opaque object managed
   // by the OPD implementation module, so that the whole implementation
   // can be detached at the build script level.
   OpenPinDevContext *m_OpenPinDevContext;

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
   static void SdlScaleHidpi(Sint32 x, Sint32 y, Sint32 *ox, Sint32 *oy);
#ifdef ENABLE_SDL_GAMECONTROLLER
   SDL_GameController* m_pSDLGameController;
   void RefreshSDLGameController();
#else
   SDL_Joystick* m_pSDLJoystick;
   SDL_Haptic* m_pSDLRumbleDevice;
   void RefreshSDLJoystick();
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

#ifdef __STANDALONE__
#define SDLK_TO_DIK_SIZE 105
static constexpr Sint32 SDLK_TO_DIK[VK_TO_DIK_SIZE][2] =
{
   { SDLK_BACKSPACE, DIK_BACK },
   { SDLK_TAB, DIK_TAB },
   //{ SDLK_CLEAR, DIK_NUMPAD5 },      /* Num Lock off */
   { SDLK_RETURN, DIK_RETURN },
   { SDLK_RETURN, DIK_NUMPADENTER },
   { SDLK_LSHIFT, DIK_LSHIFT },
   { SDLK_RSHIFT, DIK_RSHIFT },
   { SDLK_LCTRL, DIK_LCONTROL },
   { SDLK_RCTRL, DIK_RCONTROL },
   { SDLK_LALT, DIK_LMENU },
   { SDLK_RALT, DIK_RMENU },
   { SDLK_CAPSLOCK, DIK_CAPITAL },
   { SDLK_ESCAPE, DIK_ESCAPE },
   { SDLK_SPACE, DIK_SPACE },
   { SDLK_PAGEUP, DIK_PRIOR },
   { SDLK_PAGEDOWN, DIK_NEXT },
   { SDLK_END, DIK_END },
   { SDLK_HOME, DIK_HOME },
   { SDLK_LEFT, DIK_LEFT },
   { SDLK_UP, DIK_UP },
   { SDLK_RIGHT, DIK_RIGHT },
   { SDLK_DOWN, DIK_DOWN },
   { SDLK_INSERT, DIK_INSERT },
   { SDLK_DELETE, DIK_DELETE },
   { SDLK_0, DIK_0 },
   { SDLK_1, DIK_1 },
   { SDLK_2, DIK_2 },
   { SDLK_3, DIK_3 },
   { SDLK_4, DIK_4 },
   { SDLK_5, DIK_5 },
   { SDLK_6, DIK_6 },
   { SDLK_7, DIK_7 },
   { SDLK_8, DIK_8 },
   { SDLK_9, DIK_9 },
   { SDLK_a, DIK_A },
   { SDLK_b, DIK_B },
   { SDLK_c, DIK_C },
   { SDLK_d, DIK_D },
   { SDLK_e, DIK_E },
   { SDLK_f, DIK_F },
   { SDLK_g, DIK_G },
   { SDLK_h, DIK_H },
   { SDLK_i, DIK_I },
   { SDLK_j, DIK_J },
   { SDLK_k, DIK_K },
   { SDLK_l, DIK_L },
   { SDLK_m, DIK_M },
   { SDLK_n, DIK_N },
   { SDLK_o, DIK_O },
   { SDLK_p, DIK_P },
   { SDLK_q, DIK_Q },
   { SDLK_r, DIK_R },
   { SDLK_s, DIK_S },
   { SDLK_t, DIK_T },
   { SDLK_u, DIK_U },
   { SDLK_v, DIK_V },
   { SDLK_w, DIK_W },
   { SDLK_x, DIK_X },
   { SDLK_y, DIK_Y },
   { SDLK_z, DIK_Z },
   { SDLK_LGUI, DIK_LWIN },
   { SDLK_RGUI, DIK_RWIN },
   { SDLK_APPLICATION, DIK_APPS },
   { SDLK_KP_0, DIK_NUMPAD0 },
   { SDLK_KP_1, DIK_NUMPAD1 },
   { SDLK_KP_2, DIK_NUMPAD2 },
   { SDLK_KP_3, DIK_NUMPAD3 },
   { SDLK_KP_4, DIK_NUMPAD4 },
   { SDLK_KP_5, DIK_NUMPAD5 },      /* Num Lock on */
   { SDLK_KP_6, DIK_NUMPAD6 },
   { SDLK_KP_7, DIK_NUMPAD7 },
   { SDLK_KP_8, DIK_NUMPAD8 },
   { SDLK_KP_9, DIK_NUMPAD9 },
   { SDLK_KP_MULTIPLY, DIK_MULTIPLY },
   { SDLK_KP_PLUS, DIK_ADD },
   { SDLK_KP_MINUS, DIK_SUBTRACT },
   { SDLK_KP_PERIOD, DIK_DECIMAL },
   { SDLK_KP_DIVIDE, DIK_DIVIDE },
   { SDLK_F1, DIK_F1 },
   { SDLK_F2, DIK_F2 },
   { SDLK_F3, DIK_F3 },
   { SDLK_F4, DIK_F4 },
   { SDLK_F5, DIK_F5 },
   { SDLK_F6, DIK_F6 },
   { SDLK_F7, DIK_F7 },
   { SDLK_F8, DIK_F8 },
   { SDLK_F9, DIK_F9 },
   { SDLK_F10, DIK_F10 },
   { SDLK_F11, DIK_F11 },
   { SDLK_F12, DIK_F12 },
   { SDLK_F13, DIK_F13 },
   { SDLK_F14, DIK_F14 },
   { SDLK_F15, DIK_F15 },
   { SDLK_NUMLOCKCLEAR, DIK_NUMLOCK },
   { SDLK_SCROLLLOCK, DIK_SCROLL },
   { SDLK_SEMICOLON, DIK_SEMICOLON },
   { SDLK_PLUS, DIK_EQUALS },
   { SDLK_COMMA, DIK_COMMA },
   { SDLK_MINUS, DIK_MINUS },
   { SDLK_PERIOD, DIK_PERIOD },
   { SDLK_SLASH, DIK_SLASH },
   { SDLK_BACKQUOTE, DIK_GRAVE },
   { SDLK_LEFTBRACKET, DIK_LBRACKET },
   { SDLK_BACKSLASH, DIK_BACKSLASH },
   { SDLK_RIGHTBRACKET, DIK_RBRACKET },
   { SDLK_QUOTE, DIK_APOSTROPHE }
};

inline unsigned int get_dik_from_sdlk(const Sint32 sdlk)
{
   for (unsigned int i = 0; i < SDLK_TO_DIK_SIZE; ++i)
      if (SDLK_TO_DIK[i][0] == sdlk)
         return SDLK_TO_DIK[i][1];

   return ~0u;
}
#endif
