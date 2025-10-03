// license:GPLv3+

#pragma once

#include <SDL3/SDL.h>

#include "InputAction.h"
#include "PhysicsSensor.h"


class InputManager final
   : public ButtonMapping::ButtonInputEventManager
   , public SensorMapping::AxisInputEventManager
{
public:
   InputManager();
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
   unsigned int GetExitGameActionId() const { return m_exitGameActionId; }
   unsigned int GetExitInteractiveActionId() const { return m_exitInteractiveActionId; }
   unsigned int GetLockbarActionId() const { return m_lockbarActionId; }
   unsigned int GetJoyCustomActionId(int idx) const { assert(0 <= idx && idx < 4); return m_joyCustomActionId[idx]; }
   unsigned int GetVolumeDownActionId() const { return m_volumeDownActionId; }
   unsigned int GetVolumeUpActionId() const { return m_volumeUpActionId; }
   unsigned int GetExtraBallActionId() const { return m_extraBallActionId; }
   unsigned int GetSlamTiltActionId() const { return m_slamTiltActionId; }
   unsigned int GetCoinDoorActionId() const { return m_coinDoorActionId; }
   unsigned int GetServiceActionId(int idx) const { assert(0 <= idx && idx < 8); return m_serviceActionId[idx]; }
   bool IsPressed(int actionId) const;
   int GetWindowVirtualKeyForAction(unsigned int actionId) const;

   struct ActionState
   {
      uint64_t actionState;

      void SetPressed(unsigned int actionId)
      {
         assert(actionId < 64);
         const uint64_t mask = 1ull << actionId;
         actionState |= mask;
      }

      void SetReleased(unsigned int actionId)
      {
         assert(actionId < 64);
         const uint64_t mask = 1ull << actionId;
         actionState &= ~mask;
      }

      bool IsKeyPressed(int actionId, const ActionState& prev) const
      {
         assert(actionId < 64);
         const uint64_t mask = 1ull << actionId;
         return (actionState & mask) != 0 && (prev.actionState & mask) == 0;
      }

      bool IsKeyDown(int actionId) const
      {
         assert(actionId < 64);
         const uint64_t mask = 1ull << actionId;
         return (actionState & mask) != 0;
      }

      bool IsKeyReleased(int actionId, const ActionState& prev) const
      {
         assert(actionId < 64);
         const uint64_t mask = 1ull << actionId;
         return (actionState & mask) == 0 && (prev.actionState & mask) != 0;
      }
   };
   const ActionState& GetActionState() const { return m_inputActionstate; }


   ///// Nudge & plunger
   const std::unique_ptr<PhysicsSensor>& GetPlungerPositionSensor() const { return m_plungerPositionSensor; }
   const std::unique_ptr<PhysicsSensor>& GetPlungerVelocitySensor() const { return m_plungerVelocitySensor; }
   const std::unique_ptr<PhysicsSensor>& GetNudgeXSensor(int index) const { return m_nudgeXSensor[index]; }
   const std::unique_ptr<PhysicsSensor>& GetNudgeYSensor(int index) const { return m_nudgeYSensor[index]; }
   bool IsNudgeFiltered(int index) const { return m_nudgeFilter[index]; }
   void SetNudgeFiltered(int index, bool enable);
   float GetNudgeOrientation(int index) const { return m_nudgeOrientation[index]; }
   void SetNudgeOrientation(int index, float orientation) { m_nudgeOrientation[index] = orientation; }

   void AddAxisListener(std::function<void()> listener) { m_axisListeners.push_back(std::move(listener)); }
   void ClearAxisListeners() { m_axisListeners.clear(); }

   bool HasMechPlunger() const { return m_plungerPositionSensor->IsMapped(); }
   float GetPlungerPos() const { return m_plungerPositionSensor->GetValue(); }
   bool HasMechPlungerSpeed() const { return m_plungerVelocitySensor->IsMapped(); }
   float GetPlungerSpeed() const { return m_plungerVelocitySensor->GetValue(); }
   Vertex2D GetNudge() const;

   // Allow to override local state for remote control support
   void SetPlungerPos(bool override, const float pos);
   void SetPlungerSpeed(bool override, const float speed);
   void SetNudge(bool override, const float nudgeAccelerationX, const float nudgeAccelerationY);

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
   uint16_t RegisterDevice(const string& settingsId, DeviceType type, const string& name);
   void RegisterDefaultMapping(uint16_t deviceId,
      const std::function<bool( // Function to either apply or evaluate applying the default mapping of the given controller
         std::function<bool(const vector<ButtonMapping>&, unsigned int)>, // Map Button
         std::function<bool(const SensorMapping&, SensorMapping::Type type, bool isLinear)>, // Map plunger
         std::function<bool(const SensorMapping& sensorX, const SensorMapping& sensorY)> // Map nudge
         )>& mapper);
   void RegisterElementName(uint16_t deviceId, bool isAxis, uint16_t buttonOrAxisId, const string& name);
   void UnregisterDevice(uint16_t deviceId);
   uint16_t GetDeviceId(const string& settingsId);
   const string& GetDeviceSettingId(uint16_t deviceId) const;
   const string& GetDeviceName(uint16_t deviceId) const;
   string GetDeviceElementName(uint16_t deviceId, uint16_t buttonOrAxisId) const;
   DeviceType GetDeviceType(uint16_t deviceId) const;
   uint16_t GetKeyboardDeviceId() const { return m_keyboardDeviceId; }
   uint16_t GetMouseDeviceId() const { return m_mouseDeviceId; }
   vector<uint32_t> GetAllAxis() const;

   void ProcessInput();

   void HandleSDLEvent(const SDL_Event& e);
   void PushButtonEvent(uint16_t deviceId, uint16_t buttonId, uint64_t timestampNs, bool isPressed);
   void PushAxisEvent(uint16_t deviceId, uint16_t axisId, uint64_t timestampNs, float position);
   void PushTouchEvent(float relativeX, float relativeY, uint64_t timestampNs, bool isPressed);
   void OnInputActionStateChanged(InputAction* action);

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

   // Speed: 0..1
   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration);

   uint64_t m_leftkey_down_usec = 0;
   unsigned int m_leftkey_down_frame = 0;
   uint64_t m_leftkey_down_usec_rotate_to_end = 0;
   unsigned int m_leftkey_down_frame_rotate_to_end = 0;
   uint64_t m_leftkey_down_usec_EOS = 0;
   unsigned int m_leftkey_down_frame_EOS = 0;

   bool m_linearPlunger = false;
   bool m_plunger_retract = false; // enable 1s retract phase for button/key plunger

private:
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
   unsigned int m_exitGameActionId;
   unsigned int m_exitInteractiveActionId;
   unsigned int m_lockbarActionId;
   unsigned int m_joyCustomActionId[4];
   unsigned int m_volumeDownActionId;
   unsigned int m_volumeUpActionId;
   unsigned int m_extraBallActionId;
   unsigned int m_slamTiltActionId;
   unsigned int m_coinDoorActionId;
   unsigned int m_serviceActionId[8];
   ankerl::unordered_dense::map<uint32_t, vector<ButtonMapping*>> m_buttonMappings;
   vector<InputAction*> m_onUpdateActions;
   ActionState m_inputActionstate {};
   const unsigned int m_onActionEventMsgId;

   std::unique_ptr<PhysicsSensor> m_plungerPositionSensor;
   std::unique_ptr<PhysicsSensor> m_plungerVelocitySensor;
   std::unique_ptr<PhysicsSensor> m_nudgeXSensor[2];
   std::unique_ptr<PhysicsSensor> m_nudgeYSensor[2];
   float m_nudgeOrientation[2] {};
   bool m_nudgeFilter[2] { true, true };
   ankerl::unordered_dense::map<uint32_t, vector<SensorMapping*>> m_sensorMappings;
   vector<std::function<void()>> m_axisListeners;

   class DeviceDef
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
      std::function<bool( // Function to either apply or evaluate applying the default mapping of the given controller
         std::function<bool(const vector<ButtonMapping>&, unsigned int)>, // Map Button
         std::function<bool(const SensorMapping&, SensorMapping::Type type, bool isLinear)>, // Map plunger
         std::function<bool(const SensorMapping&, const SensorMapping&)> // Map nudge
         )> m_defaultMapping;
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

   uint32_t m_exitPressTimestamp = 0;
   uint32_t m_exitAppPressLengthMs = 0;

   void Autostart(const uint32_t initialDelayMs, const uint32_t retryDelayMs);
   uint32_t m_autoStartTimestamp = 0;
   bool m_gameStartedOnce = false;
   bool m_autoStartPressed = false;
   bool m_autoStartDoneOnce = false;
   int m_autoStartDirectStateSlot = -1;

   int m_rumbleMode = 0; // 0=Off, 1=Table only, 2=Generic only, 3=Table with generic as fallback

#ifdef _WIN32
   HHOOK m_hKeyboardHook = nullptr;
   static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
   STICKYKEYS m_StartupStickyKeys = { };
   TOGGLEKEYS m_StartupToggleKeys = { };
   FILTERKEYS m_StartupFilterKeys = { };
   void AllowAccessibilityShortcutKeys(bool allowKeys);
#endif
};
