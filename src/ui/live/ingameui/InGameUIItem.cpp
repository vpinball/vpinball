// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUIItem.h"

namespace VPX::InGameUI
{

void InGameUIItem::Validate()
{
   switch (m_type)
   {
   case Type::FloatValue:
   case Type::IntValue:
      assert(m_minValue < m_maxValue);
      assert(m_step > 0.f);
      m_initialValue = clamp(m_initialValue, m_minValue, m_maxValue);
      m_initialValue = GetStepAlignedValue(m_initialValue);
      m_defValue = clamp(m_defValue, m_minValue, m_maxValue);
      m_defValue = GetStepAlignedValue(m_defValue);
      assert(m_minValue <= m_initialValue && m_initialValue <= m_maxValue);
      assert(m_minValue <= m_defValue && m_defValue <= m_maxValue);
      break;

   default: break;
   }
}

void InGameUIItem::SetInitialValue(bool v)
{
   m_initialValue = v ? 1.f : 0.f;
   Validate();
}

void InGameUIItem::SetInitialValue(int v)
{
   m_initialValue = static_cast<float>(v);
   Validate();
}

void InGameUIItem::SetInitialValue(float v)
{
   m_initialValue = v;
   Validate();
}

void InGameUIItem::SetInitialValue(const string& v)
{
   m_initialStringValue = v;
   Validate();
}

void InGameUIItem::SetDefaultValue(bool v)
{
   m_defValue = v ? 1.f : 0.f;
   Validate();
}

void InGameUIItem::SetDefaultValue(int v)
{
   m_defValue = static_cast<float>(v);
   Validate();
}

void InGameUIItem::SetDefaultValue(float v)
{
   m_defValue = v;
   Validate();
}

void InGameUIItem::SetDefaultValue(const string& v)
{
   m_defStringValue = v;
   Validate();
}

bool InGameUIItem::IsModified() const
{
   switch (m_type)
   {
   case Type::FloatValue: return GetFloatValue() != m_initialValue;
   case Type::IntValue:
   case Type::EnumValue: return GetIntValue() != static_cast<int>(m_initialValue);
   case Type::Toggle: return GetBoolValue() != (m_initialValue != 0.f);
   case Type::ActionInputMapping: return m_inputAction->GetMappingString() != m_initialStringValue; break;
   case Type::PhysicsSensorMapping: return m_physicsSensor->GetMappingString() != m_initialStringValue; break;
   default: return false;
   }
}

bool InGameUIItem::IsDefaultValue() const
{
   switch (m_type)
   {
   case Type::FloatValue: return GetFloatValue() == m_defValue;
   case Type::IntValue:
   case Type::EnumValue: return GetIntValue() == static_cast<int>(m_defValue);
   case Type::Toggle: return GetBoolValue() == (m_defValue != 0.f);
   case Type::ActionInputMapping: return m_inputAction->GetMappingString() == m_defStringValue; break;
   case Type::PhysicsSensorMapping: return m_physicsSensor->GetMappingString() == m_defStringValue; break;
   default: return true;
   }
}

void InGameUIItem::ResetToInitialValue()
{
   switch (m_type)
   {
   case Type::FloatValue: SetValue(m_initialValue); break;
   case Type::IntValue:
   case Type::EnumValue: SetValue(static_cast<int>(m_initialValue)); break;
   case Type::Toggle: SetValue(m_initialValue != 0.f); break;
   case Type::ActionInputMapping: m_inputAction->SetMapping(m_initialStringValue); break;
   case Type::PhysicsSensorMapping: m_physicsSensor->SetMapping(m_initialStringValue); break;
   default: break;
   }
}

void InGameUIItem::ResetToDefault()
{
   switch (m_type)
   {
   case Type::FloatValue: SetValue(m_defValue); break;
   case Type::IntValue:
   case Type::EnumValue: SetValue(static_cast<int>(m_defValue)); break;
   case Type::Toggle: SetValue(m_defValue != 0.f); break;
   case Type::ActionInputMapping: m_inputAction->SetMapping(m_defStringValue); break;
   case Type::PhysicsSensorMapping: m_physicsSensor->SetMapping(m_defStringValue); break;
   default: break;
   }
}

void InGameUIItem::ResetSave(Settings& settings) const
{ 
   if (m_onResetSave)
      m_onResetSave(settings);
}

void InGameUIItem::Save(Settings& settings, bool isTableOverride)
{
   switch (m_type)
   {
   case Type::FloatValue:
   {
      float value = GetFloatValue();
      m_initialValue = value;
      m_onSaveFloat(value, settings, isTableOverride);
      break;
   }

   case Type::IntValue:
   case Type::EnumValue:
   {
      int value = GetIntValue();
      m_initialValue = static_cast<float>(value);
      m_onSaveInt(value, settings, isTableOverride);
      break;
   }

   case Type::Toggle:
   {
      bool value = GetBoolValue();
      m_initialValue = value ? 1.f : 0.f;
      m_onSaveBool(value, settings, isTableOverride);
      break;
   }

   case Type::ActionInputMapping:
      m_inputAction->SaveMapping(settings);
      m_initialStringValue = m_inputAction->GetMappingString();
      break;

   case Type::PhysicsSensorMapping:
      m_physicsSensor->SaveMapping(settings);
      m_initialStringValue = m_physicsSensor->GetMappingString();
      break;

   default: break;
   }
}

void InGameUIItem::SetValue(float value) const
{
   const float prev = GetFloatValue();
   value = clamp(value, m_minValue, m_maxValue);
   value = GetStepAlignedValue(value);
   if (prev != value)
      m_onChangeFloat(prev, value);
}

void InGameUIItem::SetValue(int value) const
{
   const int prev = GetIntValue();
   value = clamp(value, static_cast<int>(m_minValue), static_cast<int>(m_maxValue));
   value = static_cast<int>(GetStepAlignedValue(static_cast<float>(value)));
   if (prev != value)
      m_onChangeInt(prev, value);
}

void InGameUIItem::SetValue(bool value) const
{
   if (GetBoolValue() != value)
      m_onChangeBool(value);
}

}
