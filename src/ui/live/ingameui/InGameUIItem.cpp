// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUIItem.h"

using namespace VPX::Properties;

namespace VPX::InGameUI
{


InGameUIItem::InGameUIItem(LabelType type, string label)
   : m_type(Type::Label) // Common
   , m_label(std::move(label))
   , m_labelType(type)
{
}

InGameUIItem::InGameUIItem(const string& label, const string& tooltip, std::function<void(void)> runnable)
   : m_type(Type::Runnable) // Common
   , m_label(label)
   , m_tooltip(tooltip)
   , m_runnable(runnable)
{
}

InGameUIItem::InGameUIItem(const string& label, const string& tooltip, class InputAction* inputAction)
   : m_type(Type::ActionInputMapping) // Common
   , m_label(label)
   , m_tooltip(tooltip)
   , m_inputAction(inputAction)
   , m_defMappingString(inputAction->GetDefaultMappingString())
   , m_initialMappingString(inputAction->GetMappingString())
{
}

InGameUIItem::InGameUIItem(const string& label, const string& tooltip, class PhysicsSensor* physicsSensor, int typeMask)
   : m_type(Type::PhysicsSensorMapping) // Common
   , m_label(label)
   , m_tooltip(tooltip)
   , m_physicsSensor(physicsSensor)
   , m_physicsSensorTypeMask(typeMask)
   , m_initialMappingString(physicsSensor->GetMappingString())
{
}

InGameUIItem::InGameUIItem(string label, string tooltip, string path)
   : m_type(Type::Navigation) // Common
   , m_label(std::move(label))
   , m_tooltip(std::move(tooltip))
   , m_path(std::move(path)) // Item
{
}

InGameUIItem::InGameUIItem(const FloatPropertyDef& prop, float displayScale, const string& format, const std::function<float()>& getValue, const std::function<float()>& getStoredValue,
   const std::function<void(float, float)>& onChange, const std::function<void(Settings&)>& onResetSave, const std::function<void(float, Settings&, bool)>& onSave)
   : m_type(Type::Property)
   , m_label(prop.m_label)
   , m_tooltip(prop.m_description)
   , m_property(std::make_unique<FloatPropertyDef>(prop))
   , m_floatValueDisplayScale(displayScale)
   , m_format(std::move(format))
   , m_getFloatValue(getValue)
   , m_getStoredFloatValue(getStoredValue)
   , m_onChangeFloat(onChange)
   , m_onResetSave(onResetSave)
   , m_onSaveFloat(onSave)
{
}

InGameUIItem::InGameUIItem(
   const PropertyRegistry::PropId propId, float displayScale, const string& format, const std::function<float()>& getValue, const std::function<void(float, float)>& onChange)
   : InGameUIItem(
        *Settings::GetRegistry().GetFloatProperty(propId), displayScale, format, getValue, //
        [propId]() { return g_pplayer->m_ptable->m_settings.GetFloat(propId); }, // Get persisted value (to evaluate modified state and implement undo)
        onChange, //
        [propId](Settings& settings) { settings.Reset(propId); }, //
        [propId](float v, Settings& settings, bool isTableOverride) { settings.Set(propId, v, isTableOverride); })
{
}

InGameUIItem::InGameUIItem(const IntPropertyDef& prop, const string& format, const std::function<int()>& getValue, const std::function<int()>& getStoredValue,
   const std::function<void(int, int)>& onChange, const std::function<void(Settings&)>& onResetSave, const std::function<void(int, Settings&, bool)>& onSave)
   : m_type(Type::Property)
   , m_label(prop.m_label)
   , m_tooltip(prop.m_description)
   , m_property(std::make_unique<IntPropertyDef>(prop))
   , m_format(std::move(format))
   , m_getIntValue(getValue)
   , m_getStoredIntValue(getStoredValue)
   , m_onChangeInt(onChange)
   , m_onResetSave(onResetSave)
   , m_onSaveInt(onSave)
{
}

InGameUIItem::InGameUIItem(const PropertyRegistry::PropId propId, const string& format, const std::function<int()>& getValue, const std::function<void(int, int)>& onChange)
   : InGameUIItem(
        *Settings::GetRegistry().GetIntProperty(propId), format, getValue, //
        [propId]() { return g_pplayer->m_ptable->m_settings.GetInt(propId); }, // Get persisted value (to evaluate modified state and implement undo)
        onChange, //
        [propId](Settings& settings) { settings.Reset(propId); }, //
        [propId](int v, Settings& settings, bool isTableOverride) { settings.Set(propId, v, isTableOverride); })
{
}

InGameUIItem::InGameUIItem(const EnumPropertyDef& prop, const std::function<int()>& getValue, const std::function<int()>& getStoredValue, const std::function<void(int, int)>& onChange,
   const std::function<void(Settings&)>& onResetSave, const std::function<void(int, Settings&, bool)>& onSave)
   : m_type(Type::Property)
   , m_label(prop.m_label)
   , m_tooltip(prop.m_description)
   , m_property(std::make_unique<EnumPropertyDef>(prop))
   , m_getIntValue(getValue)
   , m_getStoredIntValue(getStoredValue)
   , m_onChangeInt(onChange)
   , m_onResetSave(onResetSave)
   , m_onSaveInt(onSave)
{
}

InGameUIItem::InGameUIItem(const PropertyRegistry::PropId propId, const std::function<int()>& getValue, const std::function<void(int, int)>& onChange)
   : InGameUIItem(
        *Settings::GetRegistry().GetEnumProperty(propId), getValue, //
        [propId]() { return g_pplayer->m_ptable->m_settings.GetInt(propId); }, // Get persisted value (to evaluate modified state and implement undo)
        onChange, //
        [propId](Settings& settings) { settings.Reset(propId); }, //
        [propId](int v, Settings& settings, bool isTableOverride) { settings.Set(propId, v, isTableOverride); })
{
}

InGameUIItem::InGameUIItem(const BoolPropertyDef& prop, const std::function<bool()>& getValue, const std::function<bool()>& getStoredValue, const std::function<void(bool)>& onChange,
   const std::function<void(Settings&)>& onResetSave, const std::function<void(float, Settings&, bool)>& onSave)
   : m_type(Type::Property)
   , m_label(prop.m_label)
   , m_tooltip(prop.m_description)
   , m_property(std::make_unique<BoolPropertyDef>(prop))
   , m_getBoolValue(getValue)
   , m_getStoredBoolValue(getStoredValue)
   , m_onChangeBool(onChange)
   , m_onResetSave(onResetSave)
   , m_onSaveBool(onSave)
{
}

InGameUIItem::InGameUIItem(const PropertyRegistry::PropId propId, const std::function<bool()>& getValue, const std::function<void(bool)>& onChange)
   : InGameUIItem(
        *Settings::GetRegistry().GetBoolProperty(propId), getValue, //
        [propId]() { return g_pplayer->m_ptable->m_settings.GetBool(propId); }, // Get persisted value (to evaluate modified state and implement undo)
        onChange, //
        [propId](Settings& settings) { settings.Reset(propId); }, //
        [propId](bool v, Settings& settings, bool isTableOverride) { settings.Set(propId, v, isTableOverride); })
{
}

InGameUIItem::InGameUIItem(Type type)
   : m_type(type)
{
}

InGameUIItem::InGameUIItem(string label, string tooltip, std::function<void(int, const InGameUIItem*)> customRender)
   : m_type(Type::CustomRender)
   , m_label(std::move(label))
   , m_tooltip(std::move(tooltip))
   , m_path(""s) // Unused
   , m_floatValueDisplayScale(1.f)
   , m_customRender(customRender)
{
}

bool InGameUIItem::IsModified() const
{
   switch (m_type)
   {
   case Type::Property:
      switch (m_property->m_type)
      {
      case PropertyDef::Type::Int: return GetIntValue() != dynamic_cast<IntPropertyDef*>(m_property.get())->GetValid(m_getStoredIntValue());
      case PropertyDef::Type::Enum: return GetIntValue() != dynamic_cast<EnumPropertyDef*>(m_property.get())->GetValid(m_getStoredIntValue());
      case PropertyDef::Type::Bool: return GetBoolValue() != m_getStoredBoolValue();
      case PropertyDef::Type::Float: return GetFloatValue() != dynamic_cast<FloatPropertyDef*>(m_property.get())->GetValid(m_getStoredFloatValue());
      default: assert(false); return false;
      }
   case Type::ActionInputMapping: return m_inputAction->GetMappingString() != m_initialMappingString; break;
   case Type::PhysicsSensorMapping: return m_physicsSensor->GetMappingString() != m_initialMappingString; break;
   default: return false;
   }
}

bool InGameUIItem::IsDefaultValue() const
{
   switch (m_type)
   {
   case Type::Property:
      switch (m_property->m_type)
      {
      case PropertyDef::Type::Int: return GetIntValue() == dynamic_cast<IntPropertyDef*>(m_property.get())->m_def;
      case PropertyDef::Type::Enum: return GetIntValue() == dynamic_cast<EnumPropertyDef*>(m_property.get())->m_def;
      case PropertyDef::Type::Bool: return GetBoolValue() == dynamic_cast<BoolPropertyDef*>(m_property.get())->m_def;
      case PropertyDef::Type::Float: return GetFloatValue() == dynamic_cast<FloatPropertyDef*>(m_property.get())->m_def;
      default: assert(false); return true;
      }
   case Type::ActionInputMapping: return m_inputAction->GetMappingString() == m_defMappingString; break;
   case Type::PhysicsSensorMapping: return m_physicsSensor->GetMappingString() == m_defMappingString; break;
   default: return true;
   }
}

void InGameUIItem::ResetToStoredValue()
{
   switch (m_type)
   {
   case Type::Property:
      switch (m_property->m_type)
      {
      case PropertyDef::Type::Int:
      case PropertyDef::Type::Enum: SetValue(m_getStoredIntValue()); break;
      case PropertyDef::Type::Bool: SetValue(m_getStoredBoolValue()); break;
      case PropertyDef::Type::Float: SetValue(m_getStoredFloatValue()); break;
      default: assert(false); break;
      }
      break;
   case Type::ActionInputMapping: m_inputAction->SetMapping(m_initialMappingString); break;
   case Type::PhysicsSensorMapping: m_physicsSensor->SetMapping(m_initialMappingString); break;
   default: break;
   }
}

void InGameUIItem::ResetToDefault()
{
   switch (m_type)
   {
   case Type::Property:
      switch (m_property->m_type)
      {
      case PropertyDef::Type::Int: SetValue(dynamic_cast<IntPropertyDef*>(m_property.get())->m_def); break;
      case PropertyDef::Type::Enum: SetValue(dynamic_cast<EnumPropertyDef*>(m_property.get())->m_def); break;
      case PropertyDef::Type::Bool: SetValue(dynamic_cast<BoolPropertyDef*>(m_property.get())->m_def); break;
      case PropertyDef::Type::Float: SetValue(dynamic_cast<FloatPropertyDef*>(m_property.get())->m_def); break;
      default: assert(false);
      }
      break;
   case Type::ActionInputMapping: m_inputAction->SetMapping(m_defMappingString); break;
   case Type::PhysicsSensorMapping: m_physicsSensor->SetMapping(m_defMappingString); break;
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
   case Type::Property:
   {
      switch (m_property->m_type)
      {
      case PropertyDef::Type::Int:
      case PropertyDef::Type::Enum: m_onSaveInt(GetIntValue(), settings, isTableOverride); break;
      case PropertyDef::Type::Bool: m_onSaveBool(GetBoolValue(), settings, isTableOverride); break;
      case PropertyDef::Type::Float: m_onSaveFloat(GetFloatValue(), settings, isTableOverride); break;
      default: assert(false);
      }
      break;
   }

   case Type::ActionInputMapping:
      m_inputAction->SaveMapping(settings);
      m_initialMappingString = m_inputAction->GetMappingString();
      break;

   case Type::PhysicsSensorMapping:
      m_physicsSensor->SaveMapping(settings);
      m_initialMappingString = m_physicsSensor->GetMappingString();
      break;

   default: break;
   }
}

float InGameUIItem::GetFloatValue() const
{
   assert(m_property && m_property->m_type == PropertyDef::Type::Float);
   return dynamic_cast<FloatPropertyDef*>(m_property.get())->GetValid(m_getFloatValue());
}

int InGameUIItem::GetIntValue() const
{
   assert(m_property);
   switch (m_property->m_type)
   {
   case PropertyDef::Type::Int: return dynamic_cast<IntPropertyDef*>(m_property.get())->GetValid(m_getIntValue());
   case PropertyDef::Type::Enum: return dynamic_cast<EnumPropertyDef*>(m_property.get())->GetValid(m_getIntValue());
   default: assert(false); return 0;
   }
}

bool InGameUIItem::GetBoolValue() const
{
   assert(m_property && m_property->m_type == PropertyDef::Type::Bool);
   return m_getBoolValue();
}


void InGameUIItem::SetValue(float value) const
{
   assert(m_property && m_property->m_type == PropertyDef::Type::Float);
   const float prev = GetFloatValue();
   value = dynamic_cast<FloatPropertyDef*>(m_property.get())->GetValid(value);
   if (prev != value)
      m_onChangeFloat(prev, value);
}

void InGameUIItem::SetValue(int value) const
{
   assert(m_property);
   const int prev = GetIntValue();
   switch (m_property->m_type)
   {
   case PropertyDef::Type::Int: value = dynamic_cast<IntPropertyDef*>(m_property.get())->GetValid(value); break;
   case PropertyDef::Type::Enum: value = dynamic_cast<EnumPropertyDef*>(m_property.get())->GetValid(value); break;
   default: assert(false); break;
   }
   if (prev != value)
      m_onChangeInt(prev, value);
}

void InGameUIItem::SetValue(bool value) const
{
   assert(m_property && m_property->m_type == PropertyDef::Type::Bool);
   if (GetBoolValue() != value)
      m_onChangeBool(value);
}

}
