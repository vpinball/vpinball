// license:GPLv3+

#include "core/stdafx.h"
#include "InputAction.h"

#include <sstream>


void InputAction::ClearMapping() { m_inputBindings.clear(); }

void InputAction::LoadMapping(const Settings& settings)
{
   const auto propId
      = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Mapping." + m_settingId, "Mapping." + m_settingId, ""s, false, m_defaultMappings));
   SetMapping(settings.GetString(propId));
}

void InputAction::SaveMapping(Settings& settings) const
{
   const auto propId
      = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Mapping." + m_settingId, "Mapping." + m_settingId, ""s, false, m_defaultMappings));
   settings.Set(propId, GetMappingString(), false);
}

void InputAction::SetMapping(const string& mappingString)
{
   ClearMapping();

   // Split by '|' which corresponds to different key bindings (any of them triggers the action, so it is a 'or')
   std::istringstream outerStream(mappingString);
   string outerToken;
   while (std::getline(outerStream, outerToken, '|'))
   {
      // Split by '&' which corresponds to the key that must be pressed together to fullfill the binding
      std::istringstream innerStream(outerToken);
      vector<ButtonMapping> innerVector;
      string innerToken;
      while (std::getline(innerStream, innerToken, '&'))
      {
         // Format is deviceId;axisOrButtonId[optional ;reversed;threshold]
         if (size_t pos = innerToken.find(';'); pos != std::string::npos)
         {
            string deviceIdString = trim_string(innerToken.substr(0, pos));
            string axisOrButtonIdString;
            string thresholdString;
            if (size_t pos2 = innerToken.substr(pos + 1).find(';'); pos2 != std::string::npos)
            {
               axisOrButtonIdString = innerToken.substr(pos + 1).substr(0, pos2);
               thresholdString = innerToken.substr(pos + 1).substr(pos2 + 1);
            }
            else
            {
               axisOrButtonIdString = innerToken.substr(pos + 1);
            }
            
            uint16_t deviceId = m_eventManager->GetDeviceId(deviceIdString);
            if (int axisOrButtonId; try_parse_int(axisOrButtonIdString, axisOrButtonId) && (axisOrButtonId & 0xFFFF) == axisOrButtonId)
            {
               innerVector.emplace_back(nullptr, nullptr, deviceId, axisOrButtonId);
               if (float threshold; thresholdString.length() > 2 && try_parse_float(thresholdString.substr(2), threshold))
               {
                  innerVector.back().SetAxisThreshold(threshold);
                  innerVector.back().SetAxisReversed(thresholdString[0] == 'x');
               }
            }
            else
            {
               PLOGE << "Invalid input mapping: '" << innerToken << '\'';
            }
         }
         else
         {
            PLOGE << "Invalid input mapping: '" << innerToken << '\'';
         }
      }
      AddMapping(innerVector);
   }
}

void InputAction::AddMapping(const vector<ButtonMapping>& mapping)
{
   if (HasMapping(mapping))
      return;

   // Recreate the mapping, tied to this InputAction and registered in the event manager
   Binding newBinding;
   newBinding.buttons.reserve(mapping.size());
   for (const ButtonMapping& map : mapping)
      newBinding.buttons.push_back(map.With(m_eventManager, this));
   m_inputBindings.push_back(std::move(newBinding));
}

void InputAction::UnmapDevice(uint16_t deviceId)
{
   for (auto it = m_inputBindings.begin(); it < m_inputBindings.end();)
   {
      bool unmap = false;
      for (const ButtonMapping& exMapping : it->buttons)
      {
         if (exMapping.GetDeviceId() == deviceId)
         {
            unmap = true;
            break;
         }
      }
      if (unmap)
         it = m_inputBindings.erase(it);
      else
         ++it;
   }
}

bool InputAction::IsMappedToDevice(uint16_t deviceId) const
{
   for (const auto& binding : m_inputBindings)
      for (const ButtonMapping& mapping : binding.buttons)
         if (mapping.GetDeviceId() == deviceId)
            return true;
   return false;
}

bool InputAction::HasMapping(const vector<ButtonMapping>& mapping) const
{
   if (mapping.empty())
      return false;

   for (const Binding& exBinding : m_inputBindings)
      if (IsSameMapping(mapping, exBinding.buttons))
         return true;

   return false;
}

bool InputAction::IsSameMapping(const vector<ButtonMapping>& mappingA, const vector<ButtonMapping>& mappingB)
{
   if (mappingA.size() != mappingB.size())
      return false;

   for (const ButtonMapping& a : mappingA)
   {
      bool hasIt = false;
      for (const ButtonMapping& b : mappingB)
         if (a.GetDeviceId() == b.GetDeviceId() && a.GetAxisOrButtonId() == b.GetAxisOrButtonId() && a.GetAxisThreshold() == b.GetAxisThreshold() && a.IsAxisReversed() == b.IsAxisReversed())
         {
            hasIt = true;
            break;
         }
      if (!hasIt)
         return false;
   }

   return true;
}

string InputAction::GetMappingString() const
{
   std::stringstream result;
   bool firstOr = true;
   for (const auto& binding : m_inputBindings)
   {
      if (!firstOr)
         result << " | ";
      firstOr = false;
      bool firstAnd = true;
      for (const ButtonMapping& mapping : binding.buttons)
      {
         if (!firstAnd)
            result << " & ";
         result << m_eventManager->GetDeviceSettingId(mapping.GetDeviceId());
         result << ';';
         result << mapping.GetAxisOrButtonId(); 
         if (mapping.GetAxisThreshold() != 0.0f || mapping.IsAxisReversed())
            result << ';' << (mapping.IsAxisReversed() ? 'x' : 'o') << ';' << std::to_string(mapping.GetAxisThreshold());
         firstAnd = false;
      }
   }
   return result.str();
}

string InputAction::GetMappingLabel() const
{
   if (m_inputBindings.empty())
      return "Not Defined"s;
   std::stringstream result;
   bool firstOr = true;
   bool withParenthesis = m_inputBindings.size() > 1;
   for (const auto& binding : m_inputBindings)
   {
      if (!firstOr)
         result << " or ";
      if (withParenthesis && binding.buttons.size() > 1)
         result << '[';
      result << m_eventManager->GetMappingLabel(binding.buttons);
      if (withParenthesis && binding.buttons.size() > 1)
         result << ']';
      firstOr = false;
   }
   return result.str();
}

int InputAction::NewDirectStateSlot()
{
   m_directStates.push_back(false);
   return static_cast<int>(m_directStates.size() - 1);
}

bool InputAction::GetDirectState(int slot) const
{
   assert(0 <= slot && slot < static_cast<int>(m_directStates.size()));
   return m_directStates[slot];
}

void InputAction::SetDirectState(int slot, bool state)
{
   assert(0 <= slot && slot < static_cast<int>(m_directStates.size()));
   if (m_directStates[slot] != state)
   {
      m_directStates[slot] = state;
      OnInputChanged(nullptr);
   }
}

// A binding is completed by a press on 'deviceAndButtonId' if all of its members are pressed once
// this event is applied (memberNewState gives the post-event state of members on that input) and at
// least one of them transitions to pressed, meaning the chord was not already complete before.
bool InputAction::IsCompletedByPress(const Binding& binding, uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState)
{
   bool triggered = false;
   for (const ButtonMapping& member : binding.buttons)
   {
      if ((member.GetDeviceId() << 16 | member.GetAxisOrButtonId()) == deviceAndButtonId)
      {
         if (!memberNewState(member))
            return false;
         triggered |= !member.IsPressed();
      }
      else if (!member.IsPressed())
         return false;
   }
   return triggered;
}

size_t InputAction::GetCompletedBindingSize(uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState) const
{
   size_t longest = 0;
   for (const Binding& binding : m_inputBindings)
      if (IsCompletedByPress(binding, deviceAndButtonId, memberNewState))
         longest = std::max(longest, binding.buttons.size());
   return longest;
}

void InputAction::SuppressCompletedBindings(uint32_t deviceAndButtonId, const std::function<bool(const ButtonMapping&)>& memberNewState, size_t minSize)
{
   for (Binding& binding : m_inputBindings)
      if (binding.buttons.size() < minSize && IsCompletedByPress(binding, deviceAndButtonId, memberNewState))
         binding.suppressed = true;
}

void InputAction::OnInputChanged(ButtonMapping*)
{
   const bool wasPressed = m_isPressed;
   m_isPressed = false;

   // Evaluate direct state
   for (bool state : m_directStates)
      m_isPressed |= state;

   // Evaluate input bindings. Always evaluated (not only when no direct state is active) so that
   // a suppressed chord's veto is reliably cleared once its buttons are no longer held together.
   for (Binding& binding : m_inputBindings)
   {
      bool satisfied = true;
      for (const ButtonMapping& mapping : binding.buttons)
      {
         satisfied &= mapping.IsPressed();
         if (!satisfied)
            break;
      }
      if (satisfied)
         m_isPressed |= !binding.suppressed;
      else
         binding.suppressed = false;
   }

   if (m_isPressed != wasPressed)
   {
      m_lastOnChangeUs = usec();
      if (m_eventManager->OnInputActionStateChanged(this))
         m_onStateChange(*this, wasPressed, m_isPressed);
      if (m_isPressed && m_repeatPeriodUs >= 0)
         m_eventManager->RegisterOnUpdate(this);
      else if (m_repeatPeriodUs >= 0)
         m_eventManager->UnregisterOnUpdate(this);
   }
}

void InputAction::OnUpdate()
{
   if (m_isPressed)
   {
      const uint64_t now = usec();
      if (now >= m_lastOnChangeUs + m_repeatPeriodUs)
      {
         m_lastOnChangeUs = now;
         m_onStateChange(*this, m_isPressed, m_isPressed);
      }
   }
}

void InputAction::SetRepeatPeriod(int delayMs)
{
   if (const int64_t targetDelay = delayMs * 1000; m_repeatPeriodUs != targetDelay)
   {
      if (m_isPressed && m_repeatPeriodUs >= 0)
         m_eventManager->UnregisterOnUpdate(this);
      m_repeatPeriodUs = targetDelay;
      if (m_isPressed && m_repeatPeriodUs >= 0)
         m_eventManager->RegisterOnUpdate(this);
   }
}

bool InputAction::IsNavigationAction() const
{
   return m_eventManager->IsUINavigationActionId(GetActionId());
}
