// license:GPLv3+

#pragma once

#ifdef _WIN32
#define ENABLE_XINPUT
#endif

#ifdef ENABLE_XINPUT
#include <XInput.h>
#pragma comment(lib, "XInput.lib")
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>

#include "ScanCodes.h"

// Maximum number of joysticks
#define PININ_JOYMXCNT 8


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
#define JOYRANGEMX (+65536)
#define JOYRANGE   (2 * JOYRANGEMX + 1)


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

class InputHandler
{
public:
   virtual ~InputHandler() = default;
   virtual void Update(const HWND foregroundWindow) = 0;
   virtual void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration) { }
};

class PinInput final
{
public:
   PinInput();
   ~PinInput();

   void Init();
   void ReInit() { UnInit(); Init(); }
   void UnInit();

   enum InputAPI
   {
      PI_DIRECTINPUT, PI_XINPUT, PI_SDL
   };

   // Mapping between input event and game actions
   enum class AnalogAction
   {
      AM_NudgeX, AM_NudgeY, AM_PlungerPos, AM_PlungerSpeed,
   };
   enum class InputLayout
   {
      Generic, PBWizard, UltraCade, Sidewinder, VirtuaPin, OpenPinDev
   };
   void SetupJoyMapping(uint64_t joystickId, InputLayout inputLayout);
   void UnmapJoy(uint64_t joyId);
   void MapActionToMouse(EnumAssignKeys action, int button, bool replace);
   void MapActionToKeyboard(EnumAssignKeys action, SDL_Scancode scancode, bool replace);
   void MapActionToJoystick(EnumAssignKeys action, uint64_t joystickId, int buttonId, bool replace);
   void MapAnalogActionToJoystick(AnalogAction output, uint64_t joystickId, int axisId, bool revert, bool replace);

   // Enqueue events for processing
   struct InputEvent
   {
      enum class Type
      {
         Action, Mouse, Keyboard, JoyButton, JoyAxis
      };
      Type type;
      EnumAssignKeys action; // Type::Action
      uint64_t joystickId; // Type::JoyButton, Type::JoyAxis
      int axisId; // Type::JoyAxis
      int value;  // Type::JoyAxis
      int buttonId; // Type::JoyButton, Type::Mouse
      SDL_Keycode keycode; // Type::Keyboard
      SDL_Scancode scancode; // Type::Keyboard
      bool isPressed; // Type::Keyboard, Type::Action, Type::Mouse
   };
   void PushActionEvent(EnumAssignKeys action, bool isPressed);
   void PushMouseEvent(int button, bool isPressed);
   void PushKeyboardEvent(SDL_Keycode keycode, SDL_Scancode scancode, bool isPressed);
   void PushJoystickButtonEvent(uint64_t joystickId, int buttonId, bool isPressed);
   void PushJoystickAxisEvent(uint64_t joystickId, int axisId, float value);

   void ProcessInput(); // Gather and process events

   void FireActionEvent(EnumAssignKeys action, bool isPressed);
   static void FireGenericKeyEvent(const int dispid, SDL_Scancode scancode);

   bool HasMechPlungerSpeed() const;
   float GetPlungerSpeed() const;
   void SetPlungerSpeed(const float speed);

   bool HasMechPlunger() const;
   float GetPlungerPos() const;
   void SetPlungerPos(const float pos);

   const Vertex2D& GetNudge() const;
   void SetNudge(const Vertex2D& nudge);

   // Speed: 0..1
   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration);

   struct InputState
   {
      uint64_t actionState;

      void SetPressed(EnumAssignKeys key)
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         actionState |= mask;
      }

      void SetReleased(EnumAssignKeys key)
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         actionState &= ~mask;
      }

      bool IsKeyPressed(EnumAssignKeys key, const InputState &prev) const
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         return (actionState & mask) != 0 && (prev.actionState & mask) == 0;
      }

      bool IsKeyDown(EnumAssignKeys key) const
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         return (actionState & mask) != 0;
      }

      bool IsKeyReleased(EnumAssignKeys key, const InputState &prev) const
      {
         uint64_t mask = static_cast<uint64_t>(1) << static_cast<int>(key);
         return (actionState & mask) == 0 && (prev.actionState & mask) != 0;
      }
   };
   const InputState& GetInputState() const;
   void SetInputState(const InputState& state);

   uint64_t m_leftkey_down_usec = 0;
   unsigned int m_leftkey_down_frame = 0;
   uint64_t m_leftkey_down_usec_rotate_to_end = 0;
   unsigned int m_leftkey_down_frame_rotate_to_end = 0;
   uint64_t m_leftkey_down_usec_EOS = 0;
   unsigned int m_leftkey_down_frame_EOS = 0;

   bool m_linearPlunger = false;
   bool m_plunger_retract = false; // enable 1s retract phase for button/key plunger

   SDL_Scancode m_joycustom1key = SDL_SCANCODE_UNKNOWN;
   SDL_Scancode m_joycustom2key = SDL_SCANCODE_UNKNOWN;
   SDL_Scancode m_joycustom3key = SDL_SCANCODE_UNKNOWN;
   SDL_Scancode m_joycustom4key = SDL_SCANCODE_UNKNOWN;

private:
   vector<std::unique_ptr<InputHandler>> m_inputHandlers;

   void ProcessEvent(const InputEvent& event);

   uint32_t m_exitPressTimestamp = 0;
   uint32_t m_exitAppPressLengthMs = 0;

   void Autostart(const uint32_t initialDelayMs, const uint32_t retryDelayMs);
   uint32_t m_autoStartTimestamp = 0;
   bool m_gameStartedOnce = false;
   bool m_autoStartPressed = false;
   bool m_autoStartDoneOnce = false;

   int m_deadz = 0;

   int m_joypmbuyin = 0;
   int m_joypmcoin3 = 0;
   int m_joypmcoin4 = 0;
   int m_joypmcoindoor = 0;
   int m_joypmcancel = 0;
   int m_joypmdown = 0;
   int m_joypmup = 0;
   int m_joypmenter = 0;
   int m_joycustom1 = 0;
   int m_joycustom2 = 0;
   int m_joycustom3 = 0;
   int m_joycustom4 = 0;
   
   const unsigned int m_onActionEventMsgId;

   struct ActionMapping
   {
      EnumAssignKeys action = EnumAssignKeys::eCKeys;

      enum AMType
      {
         AM_Keyboard, AM_Joystick, AM_Mouse
      } type;

      // Keyboard input
      SDL_Scancode scancode;
      
      // Joystick and gamepads, also mouse (buttonId)
      uint64_t joystickId;
      int buttonId;
   };
   vector<ActionMapping> m_actionMappings;
   InputState m_inputState { 0 };

   struct AnalogActionMapping
   {
      AnalogAction output;
      // Joystick and gamepads
      uint64_t joystickId = 0;
      // Axis assignments - these map to the drop-list index in the axis selection combos in the Keys dialog:
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
      int axisId = 0;
      bool revert = false;
      // Live state
      float value = 0.f;
   };
   vector<AnalogActionMapping> m_analogActionMappings;

   mutable Vertex2D m_accelerometer; // Lazily accumulated nudge from mapped analog inputs
   mutable bool m_accelerometerDirty = true;
   Vertex2D m_accelerometerMax; // Accelerometer max value X/Y axis (in -JOYRANGEMX..JOYRANGEMX range)
   bool m_accelerometerEnabled = false; // true if electronic accelerometer enabled
   bool m_accelerometerFaceUp = false; // true is Normal Mounting (Left Hand Coordinates)
   float m_accelerometerAngle = 0.f; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   float m_accelerometerSensitivity = 1.f;
   Vertex2D m_accelerometerGain; // Accelerometer gain X/Y axis

   mutable float m_plungerPos = 0.f; // Lazily accumulated plunger position from mapped analog inputs
   mutable bool m_plungerPosDirty = true;
   mutable float m_plungerSpeed = 0.f; // Lazily accumulated plunger speed from mapped analog inputs
   mutable bool m_plungerSpeedDirty = true;

   bool m_override_default_buttons = false;
   bool m_disable_esc = false;

   uint32_t m_nextKeyPressedTime = 0;

   int m_rumbleMode = 0; // 0=Off, 1=Table only, 2=Generic only, 3=Table with generic as fallback

public:
   void HandleSDLEvent(SDL_Event& e);

private:
   class SDLInputHandler* m_sdlHandler = nullptr;

#ifdef _WIN32
public:
   void SetFocusWindow(HWND focusWnd);
   class DirectInputJoystickHandler* GetDirectInputJoystickHandler() const;
   
private:
   HWND m_focusHWnd = nullptr;
   STICKYKEYS m_startupStickyKeys { 0 };
   class DirectInputJoystickHandler* m_joystickDIHandler = nullptr;
#endif
};
