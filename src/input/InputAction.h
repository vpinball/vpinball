// license:GPLv3+

#pragma once

#include "ButtonMapping.h"

// Input action handler
class InputAction final : public ButtonMapping::ButtonMappingHandler
{
public:
   // Input event manager used by input actions to resolve device ids, report state changes and handle repeats
   class EventManager : public ButtonMapping::ButtonInputEventManager
   {
   public:
      virtual uint16_t GetDeviceId(const string& settingsId) = 0;
      virtual const string& GetDeviceSettingId(uint16_t deviceId) const = 0;
      virtual string GetMappingLabel(const vector<ButtonMapping>& mappings) const = 0;
      virtual bool OnInputActionStateChanged(InputAction* action) = 0;
      virtual void RegisterOnUpdate(InputAction* action) = 0;
      virtual void UnregisterOnUpdate(InputAction* action) = 0;
      virtual bool IsUINavigationActionId(unsigned int id) const = 0;
   };

   InputAction(EventManager* eventManager, const string& settingId, const string& label, const string& defaultMappings, const std::function<void(InputAction&, bool, bool)>& onChange)
      : m_settingId(settingId)
      , m_label(label)
      , m_defaultMappings(defaultMappings)
      , m_onStateChange(onChange)
      , m_eventManager(eventManager)
   {
   }
   InputAction(InputAction&& other) = delete;
   ~InputAction() override = default;

   void ClearMapping();
   void LoadMapping(const Settings& settings);
   void SaveMapping(Settings& settings) const;
   void SetMapping(const string& mappingString);
   void AddMapping(const vector<ButtonMapping>& mapping);
   void UnmapDevice(uint16_t deviceId);
   bool IsMappedToDevice(uint16_t deviceId) const;
   bool HasMapping(const vector<ButtonMapping>& mapping) const;
   bool IsMapped() const { return !m_inputBindings.empty(); }
   const string& GetDefaultMappingString() const { return m_defaultMappings; }
   string GetMappingString() const;
   string GetMappingLabel() const;
   static bool IsSameMapping(const vector<ButtonMapping>& mappingA, const vector<ButtonMapping>& mappingB);

   void OnInputChanged(ButtonMapping* mapping) override;

   // Chord priority: when a button press completes several bindings across actions, only the longest
   // one(s) fire. Shorter bindings completed by the same press are vetoed until they are released.
   size_t GetCompletedBindingSize(uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState) const;
   void SuppressCompletedBindings(uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState, size_t minSize);
   template <typename Actions> static void ResolveChordPriority(const Actions& actions, const uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState)
   {
      size_t longest = 0;
      for (const auto& action : actions)
         longest = std::max(longest, action->GetCompletedBindingSize(deviceAndButtonId, memberNewState));
      if (longest <= 1)
         return;
      for (const auto& action : actions)
         action->SuppressCompletedBindings(deviceAndButtonId, memberNewState, longest);
   }

   bool IsPressed() const { return m_isPressed; }
   void SetPressed(bool isPressed) { m_isPressed = isPressed; }
   uint64_t GetLastStateChange() const { return m_lastOnChangeUs; }

   void SetActionId(unsigned int id) { m_actionId = id; }
   unsigned int GetActionId() const { return m_actionId; }

   void OnUpdate();
   void SetRepeatPeriod(int delayMs);

   const string& GetLabel() const { return m_label; }

   bool IsNavigationAction() const;

   int NewDirectStateSlot();
   void SetDirectState(int slot, bool state);
   bool GetDirectState(int slot) const;

private:
   const string m_settingId;
   const string m_label;
   const string m_defaultMappings;
   const std::function<void(InputAction&, bool, bool)> m_onStateChange;
   EventManager* const m_eventManager;

   // A binding is a chord of buttons that must all be pressed together to trigger the action.
   // 'suppressed' vetoes a completed chord in favor of a longer one completed by the same press.
   struct Binding
   {
      vector<ButtonMapping> buttons;
      bool suppressed = false;
   };
   static bool IsCompletedByPress(const Binding& binding, uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState);
   vector<Binding> m_inputBindings;
   vector<bool> m_directStates;
   bool m_isPressed = false;
   uint64_t m_lastOnChangeUs = 0;
   int64_t m_repeatPeriodUs = -1;
   unsigned int m_actionId = 0xFFFFFFFF;
};
