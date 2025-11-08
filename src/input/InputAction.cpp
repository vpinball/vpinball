// license:GPLv3+

#include "core/stdafx.h"
#include "InputAction.h"

#include <sstream>


void InputAction::ClearMapping()
{
   m_inputMappings.clear();
}

void InputAction::LoadMapping(const Settings& settings)
{
   const auto propId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Mapping." + m_settingId, "Mapping." + m_settingId, ""s, m_defaultMappings));
   SetMapping(settings.GetString(propId));
}

void InputAction::SaveMapping(Settings& settings) const
{
   const auto propId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Input"s, "Mapping." + m_settingId, "Mapping." + m_settingId, ""s, m_defaultMappings));
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
   vector<ButtonMapping> newMapping;
   newMapping.reserve(mapping.size());
   for (const ButtonMapping& map : mapping)
      newMapping.push_back(map.With(m_eventManager, this));
   m_inputMappings.push_back(std::move(newMapping));
}

bool InputAction::HasMapping(const vector<ButtonMapping>& mapping) const
{
   if (mapping.empty())
      return false;

   for (const vector<ButtonMapping>& exMapping : m_inputMappings)
      if (IsSameMapping(mapping, exMapping))
         return true;

   return false;
}

bool InputAction::IsSameMapping(const vector<ButtonMapping>& mappingA, const vector<ButtonMapping>& mappingB)
{
   if (mappingA.size() != mappingB.size())
      return false;

   for (const ButtonMapping& a : mappingA)
      for (const ButtonMapping& b : mappingB)
         if (a.GetDeviceId() != b.GetDeviceId()
            || a.GetAxisOrButtonId() != b.GetAxisOrButtonId()
            || a.GetAxisThreshold() != b.GetAxisThreshold()
            || a.IsAxisReversed() != b.IsAxisReversed())
            return false;

   return true;
}

string InputAction::GetMappingString() const
{
   std::stringstream result;
   bool firstOr = true;
   for (const auto& mappings : m_inputMappings)
   {
      if (!firstOr)
         result << " | ";
      firstOr = false;
      bool firstAnd = true;
      for (const ButtonMapping& mapping : mappings)
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
   if (m_inputMappings.empty())
      return "Not Defined"s;
   std::stringstream result;
   bool firstOr = true;
   bool withParenthesis = m_inputMappings.size() > 1;
   for (const auto& mappings : m_inputMappings)
   {
      if (!firstOr)
         result << " or ";
      if (withParenthesis && mappings.size() > 1)
         result << '[';
      result << m_eventManager->GetMappingLabel(mappings);
      if (withParenthesis && mappings.size() > 1)
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

void InputAction::OnInputChanged(ButtonMapping*)
{
   const bool wasPressed = m_isPressed;
   m_isPressed = false;
   
   // Evaluate direct state
   for (bool state : m_directStates)
      m_isPressed |= state;
   
   // Evaluate input mappings
   if (!m_isPressed)
   {
      for (const auto& mappings : m_inputMappings)
      {
         m_isPressed = true;
         for (const auto& mapping : mappings)
         {
            m_isPressed &= mapping.IsPressed();
            if (!m_isPressed)
               break;
         }
         if (m_isPressed)
            break;
      }
   }

   if (m_isPressed != wasPressed)
   {
      m_eventManager->OnInputActionStateChanged(this);
      m_lastOnChangeMs = msec();
      m_onStateChange(*this, wasPressed, m_isPressed);
      if (m_isPressed && m_repeatPeriodMs >= 0)
         m_eventManager->RegisterOnUpdate(this);
      else if (m_repeatPeriodMs >= 0)
         m_eventManager->UnregisterOnUpdate(this);
   }
}

void InputAction::OnUpdate()
{
   if (m_isPressed)
   {
      const unsigned int now = msec();
      if (now >= m_lastOnChangeMs + m_repeatPeriodMs)
      {
         m_lastOnChangeMs = now;
         m_onStateChange(*this, m_isPressed, m_isPressed);
      }
   }
}

void InputAction::SetRepeatPeriod(int delayMs)
{
   if (m_isPressed && m_repeatPeriodMs >= 0)
      m_eventManager->UnregisterOnUpdate(this);
   m_repeatPeriodMs = delayMs;
   if (m_isPressed && m_repeatPeriodMs >= 0)
      m_eventManager->RegisterOnUpdate(this);
}

bool InputAction::IsNavigationAction() const
{
   return GetActionId() <= m_eventManager->GetLaunchBallActionId();
}
