// license:GPLv3+

#pragma once

#include "ButtonMapping.h"

// Input action handler
class InputAction final : public ButtonMapping::ButtonMappingHandler
{
public:
   InputAction(class InputManager* eventManager, const string& settingId, const string& label, const string& defaultMappings, const std::function<void(InputAction&, bool, bool)>& onChange)
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
   bool HasMapping(const vector<ButtonMapping>& mapping) const;
   bool IsMapped() const { return !m_inputMappings.empty(); }
   const string& GetDefaultMappingString() const { return m_defaultMappings; }
   string GetMappingString() const;
   string GetMappingLabel() const;
   static bool IsSameMapping(const vector<ButtonMapping>& mappingA, const vector<ButtonMapping>& mappingB);

   void OnInputChanged(ButtonMapping* mapping) override;
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
   class InputManager* const m_eventManager;
   
   vector<vector<ButtonMapping>> m_inputMappings;
   vector<bool> m_directStates;
   bool m_isPressed = false;
   uint64_t m_lastOnChangeUs = 0;
   int64_t m_repeatPeriodUs = -1;
   unsigned int m_actionId = 0xFFFFFFFF;
};
