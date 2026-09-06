// license:GPLv3+

#pragma once

#include <SDL3/SDL.h>
#include <mutex>

#include "input/InputAction.h"
#include "input/PhysicsSensor.h"
#include "math/vector.h"

class PlungerSensor;
class PlungerHandler;
namespace VPX::Physics
{
class NudgeSensor;
class NudgeHandler;
};


class InputManager final
   : public ButtonMapping::ButtonInputEventManager
   , public SensorMapping::AxisInputEventManager
{
public:
   InputManager(class Player* player);
   InputManager(const InputManager&) = delete;
   InputManager& operator=(const InputManager&) = delete;
   ~InputManager() override;

   ///// Game Actions
   const vector<std::unique_ptr<InputAction>>& GetInputActions() { return m_inputActions; }
   unsigned int GetLeftFlipperActionId() const { return m_leftFlipperActionId; }
   unsigned int GetRightFlipperActionId() const { return m_rightFlipperActionId; }
   unsigned int GetStagedLeftFlipperActionId() const { return m_stagedLeftFlipperActionId; }
   unsigned int GetStagedRightFlipperActionId() const { return m_stagedRightFlipperActionId; }
   unsigned int GetLeftNudgeActionId() const { return m_leftNudgeActionId; }
   unsigned int GetRightNudgeActionId() const { return m_rightNudgeActionId; }
   unsigned int GetCenterNudgeActionId() const { return m_centerNudgeActionId; }
   unsigned int GetLaunchBallActionId() const { return m_launchBallActionId; }
   unsigned int GetStartActionId() const { return m_startActionId; }
   unsigned int GetAddCreditActionId(int idx) const { assert(0 <= idx && idx < 4); return m_addCreditActionId[idx]; }
   unsigned int GetTiltActionId() const { return m_tiltActionId; }
   unsigned int GetLeftMagnaActionId() const { return m_leftMagnaActionId; }
   unsigned int GetRightMagnaActionId() const { return m_rightMagnaActionId; }
   unsigned int GetUIUpActionId() const { return m_uiUpActionId; }
   unsigned int GetUIDownActionId() const { return m_uiDownActionId; }
   unsigned int GetUILeftActionId() const { return m_uiLeftActionId; }
   unsigned int GetUIRightActionId() const { return m_uiRightActionId; }
   unsigned int GetExitGameActionId() const { return m_exitGameActionId; }
   unsigned int GetOpenInGameUIActionId() const { return m_openInGameUIActionId; }
   unsigned int GetLockbarActionId() const { return m_lockbarActionId; }
   unsigned int GetJoyCustomActionId(int idx) const { assert(0 <= idx && idx < 4); return m_joyCustomActionId[idx]; }
   unsigned int GetVolumeDownActionId() const { return m_volumeDownActionId; }
   unsigned int GetVolumeUpActionId() const { return m_volumeUpActionId; }
   unsigned int GetExtraBallActionId() const { return m_extraBallActionId; }
   unsigned int GetSlamTiltActionId() const { return m_slamTiltActionId; }
   unsigned int GetCoinDoorActionId() const { return m_coinDoorActionId; }
   unsigned int GetResetActionId() const { return m_resetActionId; }
   unsigned int GetServiceActionId(int idx) const { assert(0 <= idx && idx < 8); return m_serviceActionId[idx]; }
   unsigned int GetVRControllerViewCenteringActionId() const { return m_vrControllerViewCenteringActionId; }
   unsigned int GetVRViewCenterActionId() const { return m_vrViewCenterActionId; }
   unsigned int GetVRViewUpActionId() const { return m_vrViewUpActionId; }
   unsigned int GetVRViewDownActionId() const { return m_vrViewDownActionId; }
   bool IsUINavigationActionId(unsigned int id) const;
   bool IsPressed(int actionId) const;
   int GetWindowVirtualKeyForAction(unsigned int actionId) const;

   void EnableRumbleFeedback(bool enable) { m_rumbleMode = enable; }
   bool IsRumbleFeedbackEnabled() const { return m_rumbleMode; }

   ///// Input devices
   enum class DeviceType
   {
      Unknown,
      Keyboard,
      Joystick,
      Mouse,
      VRController,
      OpenPinDev
   };
   class MappingSetupHandler
   {
   public:
      virtual void MapAction(const vector<ButtonMapping>& input, unsigned int action) = 0;
      virtual void MapPlunger(std::unique_ptr<PlungerSensor> sensor) = 0;
      virtual void MapNudge(std::unique_ptr<VPX::Physics::NudgeSensor> sensor) = 0;
   };
   uint16_t RegisterDevice(const string& settingsId, DeviceType type, const string& name);
   void SetDeviceDefaultMapping(uint16_t deviceId, const std::function<void(MappingSetupHandler&)>& mapper);
   void RegisterElementName(uint16_t deviceId, bool isAxis, uint16_t buttonOrAxisId, const string& name);
   void UnregisterDevice(uint16_t deviceId);
   void ClearDeviceMappings(uint16_t deviceId);
   uint16_t GetDeviceId(const string& settingsId);
   const string& GetDeviceSettingId(uint16_t deviceId) const;
   const string& GetDeviceName(uint16_t deviceId) const;
   bool IsDeviceConnected(uint16_t deviceId) const;
   bool IsDeviceMapped(uint16_t deviceId) const;
   string GetDeviceElementName(uint16_t deviceId, uint16_t buttonOrAxisId) const;
   DeviceType GetDeviceType(uint16_t deviceId) const;
   uint16_t GetKeyboardDeviceId() const { return m_keyboardDeviceId; }
   uint16_t GetMouseDeviceId() const { return m_mouseDeviceId; }
   vector<uint16_t> GetAllDevices() const;
   vector<uint32_t> GetAllAxis() const;

   void ProcessInput();

   void HandleSDLEvent(const SDL_Event& e);
   void PushButtonEvent(uint16_t deviceId, uint16_t buttonId, uint64_t timestampNs, bool isPressed);
   void PushAxisEvent(uint16_t deviceId, uint16_t axisId, uint64_t timestampNs, float position);
   void PushTouchEvent(float relativeX, float relativeY, uint64_t timestampNs, bool isPressed);

   void AddAxisListener(std::function<void()> listener) { m_axisListeners.push_back(std::move(listener)); }
   void ClearAxisListeners() { m_axisListeners.clear(); }

   void RegisterOnUpdate(InputAction* action);
   void UnregisterOnUpdate(InputAction* action);

   struct TouchRegionDef
   {
      RECT region;
      unsigned int actionId;
      int directStateSlot;
   };
   bool HasTouchInput() const { return m_supportsTouch; }
   const vector<TouchRegionDef>& GetTouchState() const { return m_touchRegionMap; }

   void StartButtonCapture();
   const vector<ButtonMapping>& GetButtonCapture() const { return m_buttonCapture; }
   string GetMappingLabel(const vector<ButtonMapping>& mappings) const;
   bool IsButtonCaptureDone() const { return m_buttonCaptureState == 2; }

   // ButtonInputEventManager
   void Register(ButtonMapping* mapping) override;
   void Unregister(ButtonMapping* mapping) override;

   // SensorMappingHandler
   void Register(SensorMapping* mapping) override;
   void Unregister(SensorMapping* mapping) override;

   class InputHandler
   {
   public:
      virtual ~InputHandler() = default;
      virtual void Update() = 0;
      virtual void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration) { }
   };

   // Used by actions to report state changes and query if local processing should be performed
   bool OnInputActionStateChanged(InputAction* action);

   // Speed: 0..1. Pulses are mixed, not replaced: the device plays the strongest active pulse per motor, and
   // falls back to the next one when that runs out (see UpdateRumble).
   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration);
   void UpdateRumble(); // Called once per frame: drops expired pulses and re-evaluates the output

   // Rumble on flipper/ball contact, scaled by the relative normal velocity of the impact
   void PlayFlipperContactRumble(const float normalImpactSpeed);
   float GetFlipperContactRumbleStrength() const { return m_rumbleFlipperContact; }
   void SetFlipperContactRumbleStrength(const float strength) { m_rumbleFlipperContact = strength; }

   // The generic rumbles that used to be fixed-strength calls at their sites; the strength settings scale
   // them, with 0 disabling the effect.
   void PlayBumperRumble();
   void PlaySlingshotRumble();
   void PlayPlungerRumble(const float fireSpeed);
   void PlayPlungerLaunchRumble(const float impact); // 0..1 from the closing speed of the tip and the ball
   void PlayFlipperButtonRumble();
   float GetBumperRumbleStrength() const { return m_rumbleBumper; }
   void SetBumperRumbleStrength(const float strength) { m_rumbleBumper = strength; }
   float GetSlingshotRumbleStrength() const { return m_rumbleSlingshot; }
   void SetSlingshotRumbleStrength(const float strength) { m_rumbleSlingshot = strength; }
   float GetPlungerRumbleStrength() const { return m_rumblePlunger; }
   void SetPlungerRumbleStrength(const float strength) { m_rumblePlunger = strength; }
   float GetFlipperButtonRumbleStrength() const { return m_rumbleFlipperButton; }
   void SetFlipperButtonRumbleStrength(const float strength) { m_rumbleFlipperButton = strength; }
   // Rumble on cabinet nudge, scaled by the cabinet acceleration (m/s^2). Called once per physics millisecond.
   void PlayNudgeRumble(const Vertex2D& cabinetAcceleration);
   float GetNudgeRumbleStrength() const { return m_rumbleNudge; }
   void SetNudgeRumbleStrength(const float strength) { m_rumbleNudge = strength; }

   int m_leftFlipperLastChangePollDelay = 0;

   // Used to add/remove the OpenXR input handler
   void AddInputHandler(std::unique_ptr<InputHandler> handler);
   std::unique_ptr<InputHandler> RemoveInputHandler(InputHandler* handler);

   std::unique_ptr<VPX::Physics::NudgeHandler> m_nudgeHandler;

   std::unique_ptr<PlungerHandler> m_plungerHandler;

private:
   class Player* m_player;

   void CreateInputActions();
   InputAction* AddAction(std::unique_ptr<InputAction>&& action);
   vector<std::unique_ptr<InputAction>> m_inputActions;
   unsigned int m_leftFlipperActionId;
   unsigned int m_rightFlipperActionId;
   unsigned int m_stagedLeftFlipperActionId;
   unsigned int m_stagedRightFlipperActionId;
   unsigned int m_leftNudgeActionId;
   unsigned int m_rightNudgeActionId;
   unsigned int m_centerNudgeActionId;
   unsigned int m_launchBallActionId;
   unsigned int m_startActionId;
   unsigned int m_addCreditActionId[4];
   unsigned int m_tiltActionId;
   unsigned int m_leftMagnaActionId;
   unsigned int m_rightMagnaActionId;
   unsigned int m_uiUpActionId;
   unsigned int m_uiDownActionId;
   unsigned int m_uiLeftActionId;
   unsigned int m_uiRightActionId;
   unsigned int m_exitGameActionId;
   unsigned int m_openInGameUIActionId;
   unsigned int m_lockbarActionId;
   unsigned int m_joyCustomActionId[4];
   unsigned int m_volumeDownActionId;
   unsigned int m_volumeUpActionId;
   unsigned int m_extraBallActionId;
   unsigned int m_slamTiltActionId;
   unsigned int m_coinDoorActionId;
   unsigned int m_resetActionId;
   unsigned int m_serviceActionId[8];
   unsigned int m_vrViewCenterActionId;
   unsigned int m_vrViewUpActionId;
   unsigned int m_vrViewDownActionId;
   unsigned int m_vrControllerViewCenteringActionId;
   ankerl::unordered_dense::map<uint32_t, vector<ButtonMapping*>> m_buttonMappings;
   vector<InputAction*> m_onUpdateActions;
   const unsigned int m_onActionEventMsgId;

   ankerl::unordered_dense::map<uint32_t, vector<SensorMapping*>> m_sensorMappings;
   vector<std::function<void()>> m_axisListeners;

   class DeviceDef final
   {
   public:
      DeviceDef(uint16_t id, const string& name, const string& settingsId)
         : m_id(id)
         , m_settingsId(settingsId)
         , m_name(name)
      {
      }
      const uint16_t m_id;
      const string m_settingsId;
      string m_name;
      DeviceType m_type = DeviceType::Unknown;
      bool m_connected = false;
      struct ElementDef
      {
         bool isAxis;
         string name;
      };
      ankerl::unordered_dense::map<uint16_t, ElementDef> m_buttonOrAxisNames;

      bool m_hasPendingLayoutApply = false;
      std::function<void(MappingSetupHandler&)> m_defaultMapping;
   };
   vector<DeviceDef> m_inputDevices;
   const uint16_t m_keyboardDeviceId;
   const uint16_t m_mouseDeviceId;
   bool m_hasPendingLayoutApply = false;
   void LoadDevicesFromSettings();
   void SaveDevicesToSettings() const;
   void ApplyDefaultDeviceMapping(uint16_t deviceId);

   int m_buttonCaptureState = 0;
   vector<ButtonMapping> m_buttonCapture;

   unsigned int m_volumeNotificationId = 0;

   bool m_supportsTouch;
   vector<TouchRegionDef> m_touchRegionMap;

   vector<std::unique_ptr<InputHandler>> m_inputHandlers;
   class SDLInputHandler* m_sdlHandler = nullptr;

   void Autostart(const uint32_t initialDelayMs, const uint32_t retryDelayMs);
   uint32_t m_autoStartTimestamp = 0;
   bool m_gameStartedOnce = false;
   bool m_autoStartPressed = false;
   bool m_autoStartDoneOnce = false;
   int m_autoStartDirectStateSlot = -1;

   int m_rumbleMode = 0; // 0=Off, 1=Table only, 2=Generic only, 3=Table with generic as fallback

   // Active rumble pulses. Called from the physics thread (collisions, solenoids) and the OS thread (UpdateRumble
   // once per frame), hence the mutex. Eight slots are plenty: pulses last 60..250 ms and rarely more than three overlap.
   struct RumblePulse
   {
      float low = 0.f;
      float high = 0.f;
      uint32_t endMs = 0;
   };
   static constexpr int RUMBLE_PULSE_SLOTS = 8;
   RumblePulse m_rumblePulses[RUMBLE_PULSE_SLOTS];
   std::mutex m_rumbleMutex;
   float m_rumbleSentLow = 0.f; // What the device is currently playing
   float m_rumbleSentHigh = 0.f;
   uint32_t m_rumbleSentEndMs = 0;
   // Measured with an accelerometer on the pad: eccentric mass motors need well over 100 ms from rest to full
   // amplitude, do not move below 0.3 and are full at about 0.86. So every level is mapped onto the usable range
   // (RUMBLE_MOTOR_FLOOR), and a step up of the mix by RUMBLE_KICK_STEP to at least RUMBLE_KICK_MIN_LEVEL is
   // driven at twice the mapped level (capped) for RUMBLE_KICK_MS. Lower levels are meant as a light touch.
   static constexpr float RUMBLE_OFF_LEVEL = 0.01f; // below this a strength setting or a pulse level counts as off
   static constexpr float RUMBLE_MOTOR_FLOOR = 0.3f;
   static constexpr uint32_t RUMBLE_KICK_MS = 80;
   static constexpr float RUMBLE_KICK_GAIN = 2.f;
   static constexpr float RUMBLE_KICK_MIN_LEVEL = 0.6f; // mapped level from which a pulse gets the kick; pulses meant as a light touch stay below it
   static constexpr float RUMBLE_KICK_STEP = 0.2f; // minimum rise of the output that triggers a kick
   float m_rumbleMixLow = 0.f; // The mix before the kick, to tell a real step up from a kick ending
   float m_rumbleMixHigh = 0.f;
   uint32_t m_rumbleKickLowEndMs = 0;
   uint32_t m_rumbleKickHighEndMs = 0;
   void UpdateRumbleOutput(const uint32_t now); // m_rumbleMutex must be held
   void SendRumble(const float low, const float high, const int ms_duration);
   float m_rumbleFlipperContact = 1.f; // Strength of the rumble played on flipper/ball contact, 0 disables it
   float m_rumbleBumper = 1.f; // Strength of the bumper rumble, 0 disables it
   float m_rumbleSlingshot = 1.f; // Strength of the slingshot rumble, 0 disables it
   float m_rumblePlunger = 1.f; // Strength of the plunger rumble, 0 disables it
   float m_rumbleFlipperButton = 1.f; // Strength of the flipper solenoid pulse, 0 disables it
   float m_rumbleNudge = 1.f; // Strength of the rumble played on cabinet nudge, 0 disables it
   int m_nudgeRumbleCooldownMs = 0; // Physics milliseconds left before another nudge rumble may be played

#ifdef _WIN32
   HHOOK m_hKeyboardHook = nullptr;
   static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
   STICKYKEYS m_StartupStickyKeys = { };
   TOGGLEKEYS m_StartupToggleKeys = { };
   FILTERKEYS m_StartupFilterKeys = { };
   void AllowAccessibilityShortcutKeys(bool allowKeys);
#endif
};
