// license:GPLv3+

#pragma once

#include <SDL3/SDL.h>

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
   bool IsPressed(int actionId) const;
   int GetWindowVirtualKeyForAction(unsigned int actionId) const;

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

   // Speed: 0..1
   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration);

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

#ifdef _WIN32
   HHOOK m_hKeyboardHook = nullptr;
   static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
   STICKYKEYS m_StartupStickyKeys = { };
   TOGGLEKEYS m_StartupToggleKeys = { };
   FILTERKEYS m_StartupFilterKeys = { };
   void AllowAccessibilityShortcutKeys(bool allowKeys);
#endif
};
