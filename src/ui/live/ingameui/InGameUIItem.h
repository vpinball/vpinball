// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class InGameUIItem
{
public:
   explicit InGameUIItem(string label)
      : m_type(Type::Info) // Common
      , m_label(std::move(label))
      , m_path(""s) // Unused
      , m_minValue(0.f)
      , m_maxValue(0.f)
      , m_step(0.f)
      , m_defValue(0.f)
      , m_initialValue(0.f)
      , m_enum()
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, string path)
      : m_type(Type::Navigation) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_path(std::move(path)) // Item
      , m_minValue(0.f) // Unused
      , m_maxValue(0.f)
      , m_step(0.f)
      , m_defValue(0.f)
      , m_initialValue(0.f)
      , m_enum()
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, int min, int max, int defValue, string format, std::function<int()> getValue, std::function<void(int, int)> onChange,
      std::function<void(int, Settings&, bool)> onSave)
      : m_type(Type::IntValue) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_minValue(static_cast<float>(min)) // Item
      , m_maxValue(static_cast<float>(max))
      , m_step(1.f)
      , m_defValue(GetStepAlignedValue(static_cast<float>(defValue)))
      , m_getIntValue(getValue)
      , m_initialValue(static_cast<float>(getValue()))
      , m_format(format)
      , m_onChangeInt(onChange)
      , m_onSaveInt(onSave)
      , m_path(""s) // Unused
      , m_enum()
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, float min, float max, float step, float defValue, string format, std::function<float()> getValue, std::function<void(float, float)> onChange,
      std::function<void(float, Settings&, bool)> onSave)
      : m_type(Type::FloatValue) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_minValue(min) // Item
      , m_maxValue(max)
      , m_step(step)
      , m_defValue(GetStepAlignedValue(defValue))
      , m_getFloatValue(getValue)
      , m_initialValue(getValue())
      , m_format(format)
      , m_onChangeFloat(onChange)
      , m_onSaveFloat(onSave)
      , m_path(""s) // Unused
      , m_enum()
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, std::vector<string> values, int defValue, std::function<int()> getValue, std::function<void(int, int)> onChange,
      std::function<void(int, Settings&, bool)> onSave)
      : m_type(Type::EnumValue) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_enum(values) // Item
      , m_minValue(0.f)
      , m_maxValue(static_cast<float>(values.size()))
      , m_step(1.f)
      , m_defValue(GetStepAlignedValue(static_cast<float>(defValue)))
      , m_getIntValue(getValue)
      , m_initialValue(static_cast<float>(getValue()))
      , m_onChangeInt(onChange)
      , m_onSaveInt(onSave)
      , m_path(""s) // Unused
      , m_format(""s)
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, bool defValue, std::function<bool()> getValue, std::function<void(bool)> onChange, std::function<void(bool, Settings&, bool)> onSave)
      : m_type(Type::Toggle) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_minValue(0.f) // Item
      , m_maxValue(1.f)
      , m_step(1.f)
      , m_defValue(defValue ? 1.f : 0.f)
      , m_getBoolValue(getValue)
      , m_initialValue(getValue() ? 1.f : 0.f)
      , m_onChangeBool(onChange)
      , m_onSaveBool(onSave)
      , m_path(""s) // Unused
      , m_enum()
   {
      Validate();
   }

   enum class Type
   {
      // Generic items
      Info,
      Navigation,
      FloatValue,
      IntValue,
      EnumValue,
      Toggle,
      // Core actions
      Back,
      ResetToDefaults,
      ResetToInitialValues,
      SaveChanges
   };

   explicit InGameUIItem(Type type)
      : m_type(type) // Common
      , m_label(""s)
      , m_path(""s) // Unused
      , m_minValue(0.f)
      , m_maxValue(0.f)
      , m_step(0.f)
      , m_defValue(0.f)
      , m_initialValue(0.f)
      , m_enum()
   {
      Validate();
   }

   bool IsSelectable()
   {
      return true;
      //m_type != Type::Info;
   }
   bool IsAdjustable() { return m_type == Type::FloatValue || m_type == Type::IntValue || m_type == Type::EnumValue || m_type == Type::Toggle; }

   float GetFloatValue() const
   {
      float value = m_getFloatValue();
      value = clamp(value, m_minValue, m_maxValue);
      value = GetStepAlignedValue(value);
      return value;
   }
   int GetIntValue() const
   {
      float value = static_cast<float>(m_getIntValue());
      value = clamp(value, m_minValue, m_maxValue);
      value = GetStepAlignedValue(value);
      return static_cast<int>(value);
   }
   bool GetBoolValue() const { return m_getBoolValue() ? 1.f : 0.f; }
   void SetValue(float value);
   void SetValue(int value);
   void SetValue(bool value);

   void SetInitialValue(bool v);
   void SetInitialValue(int v);
   void SetInitialValue(float v);
   bool IsModified() const;
   void Save(Settings& settings, bool isTableOverride);
   void ResetToInitialValue();

   void SetDefaultValue(bool v);
   void SetDefaultValue(int v);
   void SetDefaultValue(float v);
   bool IsDefaultValue() const;
   void ResetToDefault();

   const Type m_type;
   const string m_label;
   const string m_tooltip;

   // Navigation item
   const string m_path;

   // Ranged value item
   const float m_minValue;
   const float m_maxValue;
   const float m_step;

   // Enum value item
   const vector<string> m_enum;

   // Properties shared by value items
   const string m_format;

private:
   void Validate();
   float GetStepAlignedValue(float v) const { return m_minValue + roundf((v - m_minValue) / m_step) * m_step; }

   float m_defValue;
   float m_initialValue;

   const std::function<bool()> m_getBoolValue;
   const std::function<void(bool)> m_onChangeBool;
   const std::function<int()> m_getIntValue;
   const std::function<void(int, int)> m_onChangeInt;
   const std::function<float()> m_getFloatValue;
   const std::function<void(float, float)> m_onChangeFloat;
   const std::function<void(int, Settings&, bool)> m_onSaveInt;
   const std::function<void(bool, Settings&, bool)> m_onSaveBool;
   const std::function<void(float, Settings&, bool)> m_onSaveFloat;
};

};