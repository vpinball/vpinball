// license:GPLv3+

#pragma once

#ifdef _WIN32
#define ENABLE_XINPUT
#endif

#ifdef ENABLE_XINPUT
#include <XInput.h>
#pragma comment(lib, "XInput.lib")
#endif

#ifdef ENABLE_SDL_INPUT //!! test
#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>
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
#define PININ_JOYMXCNT 8
#define APP_JOYSTICK(n) (APP_JOYSTICKMN + (n))
#define APP_JOYSTICKMX  (APP_JOYSTICK(PININ_JOYMXCNT - 1))


// Joystick axis normalized input range.  This is the range for joystick
// axis values returned from the "Input API" functions (HandleInputDI,
// HandleInputSDL, etc).  This range is used to represent raw joystick
// inputs in the PinInput code in the internal event structures.
//
// This range isn't exposed to users or scripting anywhere; it's only
// used for internal device event processing.  Joystick axis inputs are
// ultimately interpreted as either Nudge X/Y values or Plunger Position
// or Speed values, all of which are translated to internal units before
// they affect anything that a user or script sees.
//
// WARNING!! These constants CANNOT BE CHANGED without breaking SDL and
// XInput (at least; there might be others).  The SDL and XInput code
// contain hard-coded formulas with naked integer constants that HAPPEN
// TO scale their input to this range, but without reference to the
// JOYRANGE symbols, so you'd have to make corresponding changes in
// the SDL and XInput code (at least) if JOYRANGE ever changed.  If
// anyone ever changes the range, be sure to test all of the input APIs
// to make sure all of those hidden assumptions are fixed up.
#define JOYRANGEMN (-65536)
#define JOYRANGEMX (+65536)
#define JOYRANGE ((JOYRANGEMX) - (JOYRANGEMN) + 1)


#ifdef _WIN32
#define USE_DINPUT_FOR_KEYBOARD // can lead to less input lag maybe on some systems if disabled, but can miss input if key is only pressed very very quickly and/or FPS are low
#endif


// NOTE that the following 3 definitions need to be in sync in their order!
enum EnumAssignKeys
{
   eLeftFlipperKey,
   eRightFlipperKey,
   eStagedLeftFlipperKey,
   eStagedRightFlipperKey,
   eLeftTiltKey,
   eRightTiltKey,
   eCenterTiltKey,
   ePlungerKey,
   eFrameCount,
   eDBGBalls,
   eDebugger,
   eAddCreditKey,
   eAddCreditKey2,
   eStartGameKey,
   eMechanicalTilt,
   eRightMagnaSave,
   eLeftMagnaSave,
   eExitGame,
   eVolumeUp,
   eVolumeDown,
   eLockbarKey,
   eEnable3D,
   eTableRecenter,
   eTableUp,
   eTableDown,
   eEscape,
   ePause,
   eTweak,
   eCKeys
};

static const string regkey_string[eCKeys] = {
   "LFlipKey"s,
   "RFlipKey"s,
   "StagedLFlipKey"s,
   "StagedRFlipKey"s,
   "LTiltKey"s,
   "RTiltKey"s,
   "CTiltKey"s,
   "PlungerKey"s,
   "FrameCount"s,
   "DebugBalls"s,
   "Debugger"s,
   "AddCreditKey"s,
   "AddCreditKey2"s,
   "StartGameKey"s,
   "MechTilt"s,
   "RMagnaSave"s,
   "LMagnaSave"s,
   "ExitGameKey"s,
   "VolumeUp"s,
   "VolumeDown"s,
   "LockbarKey"s,
   "Enable3DKey"s,
   "TableRecenterKey"s,
   "TableUpKey"s,
   "TableDownKey"s,
   "EscapeKey"s,
   "PauseKey"s,
   "TweakKey"s
};

static constexpr int regkey_idc[eCKeys] = {
   IDC_LEFTFLIPPER,
   IDC_RIGHTFLIPPER,
   IDC_STAGEDLEFTFLIPPER,
   IDC_STAGEDRIGHTFLIPPER,
   IDC_LEFTTILT,
   IDC_RIGHTTILT,
   IDC_CENTERTILT,
   IDC_PLUNGER_TEXT,
   IDC_FRAMECOUNT,
   IDC_DEBUGBALL,
   IDC_DEBUGGER,
   IDC_ADDCREDIT,
   IDC_ADDCREDITKEY2,
   IDC_STARTGAME,
   IDC_MECHTILT,
   IDC_RMAGSAVE,
   IDC_LMAGSAVE,
   IDC_EXITGAME,
   IDC_VOLUMEUP,
   IDC_VOLUMEDN,
   IDC_LOCKBAR,
   -1, //!! missing in key dialog! (Enable/disable 3D stereo)
   IDC_TABLEREC_TEXT,
   IDC_TABLEUP_TEXT,
   IDC_TABLEDOWN_TEXT,
   -1, // Escape
   IDC_PAUSE,
   IDC_TWEAK
};

// Open Pinball Device context (defined in the OPD implementation module)
class OpenPinDevContext;

class PinInput
{
public:
   PinInput();
   void LoadSettings(const Settings& settings);
   ~PinInput();

   #ifdef _WIN32
   void Init(HWND focusWnd);
   #else
   void Init();
   #endif
   void UnInit();

   enum InputAPI
   {
      PI_DIRECTINPUT, PI_XINPUT, PI_SDL, PI_GAMECONTROLLER
   };
   InputAPI GetInputAPI() const { return m_inputApi; }

   void FireActionEvent(EnumAssignKeys key, bool isPressed);
   void FireGenericKeyEvent(const int dispid, int keycode);

   void PushQueue(DIDEVICEOBJECTDATA * const data, const unsigned int app_data);
   const DIDEVICEOBJECTDATA *GetTail();

   void ProcessKeys(int curr_time_msec, bool handleStartExit);

   void ProcessJoystick(const DIDEVICEOBJECTDATA * __restrict input, int curr_time_msec);

   // Speed: 0..1
   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration);

   int GetNextKey();

   struct InputState
   {
      uint64_t keyState;

      void SetPressed(EnumAssignKeys key)
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         keyState |= mask;
      }

      void SetReleased(EnumAssignKeys key)
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         keyState &= ~mask;
      }

      bool IsKeyPressed(EnumAssignKeys key, const InputState &prev) const
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         return (keyState & mask) != 0 && (prev.keyState & mask) == 0;
      }

      bool IsKeyDown(EnumAssignKeys key) const
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         return (keyState & mask) != 0;
      }

      bool IsKeyReleased(EnumAssignKeys key, const InputState &prev) const
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         return (keyState & mask) == 0 && (prev.keyState & mask) != 0;
      }
   };
   const InputState& GetInputState() const { return m_inputState; }

   #ifdef _WIN32
   #ifdef USE_DINPUT8
   LPDIRECTINPUTDEVICE8 GetJoystick(int index);
   #else
   LPDIRECTINPUTDEVICE GetJoystick(int index);
   #endif
   #endif

   uint64_t m_leftkey_down_usec;
   unsigned int m_leftkey_down_frame;
   uint64_t m_leftkey_down_usec_rotate_to_end;
   unsigned int m_leftkey_down_frame_rotate_to_end;
   uint64_t m_leftkey_down_usec_EOS;
   unsigned int m_leftkey_down_frame_EOS;
   uint64_t m_lastclick_ballcontrol_usec;

   int m_num_joy;
   int uShockType;

   bool m_linearPlunger;
   bool m_plunger_retract; // enable 1s retract phase for button/key plunger

   int m_joycustom1key, m_joycustom2key, m_joycustom3key, m_joycustom4key;

private:
   int Started();

   void Autostart(const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec);
   void ButtonExit(const U32 msecs, const U32 curr_time_msec);

   void Joy(const unsigned int n, const bool isPressed, const bool start);

   void InitOpenPinballDevices();
   void ReadOpenPinballDevices(const U32 cur_time_msec);
   void TerminateOpenPinballDevices();

   void GetInputDeviceData();
   void HandleInputDI(DIDEVICEOBJECTDATA *didod);
   void HandleInputXI(DIDEVICEOBJECTDATA *didod);
   void HandleInputIGC(DIDEVICEOBJECTDATA *didod);
   void HandleInputSDL(DIDEVICEOBJECTDATA *didod);

#if defined(ENABLE_SDL_INPUT)
public:
   void HandleSDLEvent(SDL_Event& e);
private:
#endif

#ifdef _WIN32
   HWND m_focusHWnd = nullptr;
   STICKYKEYS m_startupStickyKeys;
   static BOOL CALLBACK EnumObjectsCallbackDI(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);
   static BOOL CALLBACK EnumJoystickCallbackDI(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
   #ifdef USE_DINPUT8
      LPDIRECTINPUT8 m_pDI = nullptr;
      LPDIRECTINPUTDEVICE8 m_pJoystick[PININ_JOYMXCNT];
      LPDIRECTINPUTDEVICE8 m_pMouse = nullptr;
      #ifdef USE_DINPUT_FOR_KEYBOARD
         LPDIRECTINPUTDEVICE8 m_pKeyboard = nullptr;
      #endif
   #else
      LPDIRECTINPUT m_pDI = nullptr;
      LPDIRECTINPUTDEVICE m_pJoystick[PININ_JOYMXCNT];
      LPDIRECTINPUTDEVICE m_pMouse = nullptr;
      #ifdef USE_DINPUT_FOR_KEYBOARD
         LPDIRECTINPUTDEVICE m_pKeyboard = nullptr;
      #endif
   #endif

   LPDIDEVICEINSTANCE m_attachedDeviceInfo[PININ_JOYMXCNT];

   std::unique_ptr<std::map<string, bool>> m_pInputDeviceSettingsInfo;
#endif

   // Open Pinball Device context.  This is an opaque object managed
   // by the OPD implementation module, so that the whole implementation
   // can be detached at the build script level.
   OpenPinDevContext *m_OpenPinDevContext = nullptr;

   // Open Pinball Device button status, for detecting button up/down events
   uint32_t m_openPinDev_generic_buttons = 0;
   uint32_t m_openPinDev_pinball_buttons = 0;
   bool m_openPinDev_flipper_l = false, m_openPinDev_flipper_r = false;

   BYTE m_oldMouseButtonState[3];

   U32 m_firedautostart;

   U32 m_first_stamp;
   U32 m_exit_stamp;

   bool m_pressed_start;

   bool m_as_down;
   bool m_as_didonce;

   bool m_tilt_updown;

   DIDEVICEOBJECTDATA m_diq[MAX_KEYQUEUE_SIZE]; // circular queue of direct input events

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

   InputState m_inputState { 0 };

   DWORD m_nextKeyPressedTime;

   InputAPI m_inputApi;
   int m_rumbleMode; // 0=Off, 1=Table only, 2=Generic only, 3=Table with generic as fallback

   static constexpr int m_LeftMouseButtonID = 25;
   static constexpr int m_RightMouseButtonID = 26;
   static constexpr int m_MiddleMouseButtonID = 27;

#ifdef ENABLE_XINPUT
   int m_inputDeviceXI;
   XINPUT_STATE m_inputDeviceXIstate;
   DWORD m_rumbleOffTime;
   bool m_rumbleRunning;
#endif

#ifdef ENABLE_SDL_INPUT
   SDL_Gamepad* m_pSDLGamePad = nullptr;
   SDL_Joystick* m_pSDLJoystick = nullptr; 
   SDL_Haptic* m_pSDLRumbleDevice = nullptr;
   void RefreshSDLDevices();
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

#ifdef ENABLE_SDL_VIDEO
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
   { SDLK_A, DIK_A },
   { SDLK_B, DIK_B },
   { SDLK_C, DIK_C },
   { SDLK_D, DIK_D },
   { SDLK_E, DIK_E },
   { SDLK_F, DIK_F },
   { SDLK_G, DIK_G },
   { SDLK_H, DIK_H },
   { SDLK_I, DIK_I },
   { SDLK_J, DIK_J },
   { SDLK_K, DIK_K },
   { SDLK_L, DIK_L },
   { SDLK_M, DIK_M },
   { SDLK_N, DIK_N },
   { SDLK_O, DIK_O },
   { SDLK_P, DIK_P },
   { SDLK_Q, DIK_Q },
   { SDLK_R, DIK_R },
   { SDLK_S, DIK_S },
   { SDLK_T, DIK_T },
   { SDLK_U, DIK_U },
   { SDLK_V, DIK_V },
   { SDLK_W, DIK_W },
   { SDLK_X, DIK_X },
   { SDLK_Y, DIK_Y },
   { SDLK_Z, DIK_Z },
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
   { SDLK_GRAVE, DIK_GRAVE },
   { SDLK_LEFTBRACKET, DIK_LBRACKET },
   { SDLK_BACKSLASH, DIK_BACKSLASH },
   { SDLK_RIGHTBRACKET, DIK_RBRACKET },
   { SDLK_APOSTROPHE, DIK_APOSTROPHE }
};

inline unsigned int get_dik_from_sdlk(const Sint32 sdlk)
{
   for (unsigned int i = 0; i < SDLK_TO_DIK_SIZE; ++i)
      if (SDLK_TO_DIK[i][0] == sdlk)
         return SDLK_TO_DIK[i][1];

   return ~0u;
}
#endif
