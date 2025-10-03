// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class InGameUIItem final
{
public:
   enum class LabelType
   {
      Info, // Not scrollable basic text
      Header, // Not scrollable basic text, formated to split sections of the item list
      Markdown // Scrollable (therefore selectable), advanced formatting
   };
   explicit InGameUIItem(LabelType type, string label)
      : m_type(Type::Label) // Common
      , m_labelType(type)
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

   explicit InGameUIItem(const string& label, const string& tooltip, class InputAction* inputAction)
      : m_type(Type::ActionInputMapping) // Common
      , m_label(label)
      , m_tooltip(tooltip)
      , m_inputAction(inputAction)
      , m_initialStringValue(inputAction->GetMappingString())
      , m_defStringValue(inputAction->GetDefaultMappingString())
      , m_path(""s) // Unused
      , m_minValue(0.f)
      , m_maxValue(0.f)
      , m_step(0.f)
      , m_defValue(0.f)
      , m_enum()
   {
      Validate();
   }

   explicit InGameUIItem(const string& label, const string& tooltip, class PhysicsSensor* physicsSensor, int typeMask)
      : m_type(Type::PhysicsSensorMapping) // Common
      , m_label(label)
      , m_tooltip(tooltip)
      , m_physicsSensor(physicsSensor)
      , m_physicsSensorTypeMask(typeMask)
      , m_initialStringValue(physicsSensor->GetMappingString())
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
      std::function<void(Settings&)> onResetSave, std::function<void(int, Settings&, bool)> onSave)
      : m_type(Type::IntValue) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_minValue(static_cast<float>(min)) // Item
      , m_maxValue(static_cast<float>(max))
      , m_step(1.f)
      , m_defValue(GetStepAlignedValue(static_cast<float>(defValue)))
      , m_getIntValue(getValue)
      , m_initialValue(static_cast<float>(getValue()))
      , m_format(std::move(format))
      , m_onChangeInt(onChange)
      , m_onResetSave(onResetSave)
      , m_onSaveInt(onSave)
      , m_path(""s) // Unused
      , m_enum()
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, float min, float max, float step, float defValue, string format, std::function<float()> getValue, std::function<void(float, float)> onChange,
      std::function<void(Settings&)> onResetSave, std::function<void(float, Settings&, bool)> onSave)
      : m_type(Type::FloatValue) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_minValue(min) // Item
      , m_maxValue(max)
      , m_step(step)
      , m_defValue(GetStepAlignedValue(defValue))
      , m_getFloatValue(getValue)
      , m_initialValue(getValue())
      , m_format(std::move(format))
      , m_onChangeFloat(onChange)
      , m_onResetSave(onResetSave)
      , m_onSaveFloat(onSave)
      , m_path(""s) // Unused
      , m_enum()
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, std::vector<string> values, int defValue, std::function<int()> getValue, std::function<void(int, int)> onChange,
      std::function<void(Settings&)> onResetSave, std::function<void(int, Settings&, bool)> onSave)
      : m_type(Type::EnumValue) // Common
      , m_label(std::move(label))
      , m_tooltip(std::move(tooltip))
      , m_enum(std::move(values)) // Item
      , m_minValue(0.f)
      , m_maxValue(static_cast<float>(values.size()))
      , m_step(1.f)
      , m_defValue(GetStepAlignedValue(static_cast<float>(defValue)))
      , m_getIntValue(getValue)
      , m_initialValue(static_cast<float>(getValue()))
      , m_onChangeInt(onChange)
      , m_onResetSave(onResetSave)
      , m_onSaveInt(onSave)
      , m_path(""s) // Unused
      , m_format(""s)
   {
      Validate();
   }

   InGameUIItem(string label, string tooltip, bool defValue, std::function<bool()> getValue, std::function<void(bool)> onChange, std::function<void(Settings&)> onResetSave,
      std::function<void(bool, Settings&, bool)> onSave)
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
      , m_onResetSave(onResetSave)
      , m_onSaveBool(onSave)
      , m_path(""s) // Unused
      , m_enum()
   {
      Validate();
   }

   enum class Type
   {
      // Generic items
      Label,
      Navigation,
      FloatValue,
      IntValue,
      EnumValue,
      Toggle,
      ActionInputMapping,
      PhysicsSensorMapping,
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

   bool IsSelectable() const
   {
      return m_type != Type::Label || m_labelType == LabelType::Markdown;
   }
   bool IsAdjustable() const
   {
      return m_type == Type::FloatValue || m_type == Type::IntValue || m_type == Type::EnumValue || m_type == Type::Toggle || m_type == Type::ActionInputMapping
         || m_type == Type::PhysicsSensorMapping;
   }

   float GetFloatValue() const
   {
      float value = m_getFloatValue();
      value = clamp(value, m_minValue, m_maxValue);
      value = GetStepAlignedValue(value);
      return value;
   }
   int GetIntValue() const
   {
      int value = m_getIntValue();
      value = clamp(value, static_cast<int>(m_minValue), static_cast<int>(m_maxValue));
      value = static_cast<int>(GetStepAlignedValue(static_cast<float>(value)));
      return value;
   }
   bool GetBoolValue() const { return m_getBoolValue(); }
   void SetValue(float value) const;
   void SetValue(int value) const;
   void SetValue(bool value) const;

   void SetInitialValue(bool v);
   void SetInitialValue(int v);
   void SetInitialValue(float v);
   void SetInitialValue(const string& s);
   bool IsModified() const;
   void ResetSave(Settings& settings) const;
   void Save(Settings& settings, bool isTableOverride);
   void ResetToInitialValue();

   void SetDefaultValue(bool v);
   void SetDefaultValue(int v);
   void SetDefaultValue(float v);
   void SetDefaultValue(const string& s);
   bool IsDefaultValue() const;
   void ResetToDefault();

   const Type m_type;
   const string m_label;
   const string m_tooltip;

   // Label
   const LabelType m_labelType = LabelType::Header;

   // Input mapping
   class InputAction* m_inputAction;
   class PhysicsSensor* m_physicsSensor;
   int m_physicsSensorTypeMask;

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

   static std::function<void(Settings&)> ResetSetting(const Settings::Section section, const string& key)
   {
      return [section, key](Settings& settings) { settings.DeleteValue(section, key); };
   }

   static std::function<void(int, Settings&, bool)> SaveSettingInt(const Settings::Section section, const string& key)
   {
      return [section, key](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(section, key, v, isTableOverride); };
   }

   static std::function<void(bool, Settings&, bool)> SaveSettingBool(const Settings::Section section, const string& key)
   {
      return [section, key](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(section, key, v, isTableOverride); };
   }

   static std::function<void(float, Settings&, bool)> SaveSettingFloat(const Settings::Section section, const string& key)
   {
      return [section, key](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(section, key, v, isTableOverride); };
   }

private:
   void Validate();
   float GetStepAlignedValue(float v) const { return m_minValue + roundf((v - m_minValue) / m_step) * m_step; }

   float m_defValue;
   float m_initialValue;
   string m_defStringValue;
   string m_initialStringValue;

   const std::function<bool()> m_getBoolValue;
   const std::function<void(bool)> m_onChangeBool;
   const std::function<int()> m_getIntValue;
   const std::function<void(int, int)> m_onChangeInt;
   const std::function<float()> m_getFloatValue;
   const std::function<void(float, float)> m_onChangeFloat;
   const std::function<void(Settings&)> m_onResetSave;
   const std::function<void(int, Settings&, bool)> m_onSaveInt;
   const std::function<void(bool, Settings&, bool)> m_onSaveBool;
   const std::function<void(float, Settings&, bool)> m_onSaveFloat;
};

}
