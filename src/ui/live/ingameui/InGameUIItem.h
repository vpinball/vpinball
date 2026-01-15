// license:GPLv3+

#pragma once

#include "core/PropertyDef.h"

namespace VPX::InGameUI
{

class InGameUIItem final
{
public:
   enum class Type
   {
      // Generic items
      Label,
      Runnable,
      Navigation,
      ActionInputMapping,
      PhysicsSensorMapping,
      Property,
      CustomRender,
      // Core actions
      Back,
      ResetToDefaults,
      ResetToStoredValues,
      SaveChanges,
   };

   enum class LabelType
   {
      Info, // Not scrollable basic text
      Header, // Not scrollable basic text, formated to split sections of the item list
      Markdown // Scrollable (therefore selectable), advanced formatting
   };
   InGameUIItem(LabelType type, string label);

   InGameUIItem(const string& label, const string& tooltip, std::function<void(void)> runnable);

   InGameUIItem(string label, string tooltip, string path);

   InGameUIItem(const string& label, const string& tooltip, class InputAction* inputAction);

   InGameUIItem(const string& label, const string& tooltip, class PhysicsSensor* physicsSensor, int typeMask);

   InGameUIItem(string label, string tooltip, std::function<void(int, const InGameUIItem*)> customRender);

   InGameUIItem(Type type); // Core actions

   // Properties stored in the table's settings property store
   explicit InGameUIItem(const VPX::Properties::PropertyRegistry::PropId propId, float displayScale, const string& format, const std::function<float()>& getValue,
      const std::function<void(float, float)>& onChange);
   explicit InGameUIItem(const VPX::Properties::PropertyRegistry::PropId propId, const string& format, const std::function<int()>& getValue, const std::function<void(int, int)>& onChange);
   explicit InGameUIItem(const VPX::Properties::PropertyRegistry::PropId propId, const std::function<int()>& getValue, const std::function<void(int, int)>& onChange);
   explicit InGameUIItem(const VPX::Properties::PropertyRegistry::PropId propId, const std::function<bool()>& getValue, const std::function<void(bool)>& onChange);

   // Properties with custom storage
   explicit InGameUIItem(const VPX::Properties::FloatPropertyDef& prop, float displayScale, const string& format, const std::function<float()>& getValue,
      const std::function<float(Settings&)>& getStoredValue, const std::function<void(float, float)>& onChange, const std::function<void(Settings&)>& onResetSave,
      const std::function<void(float, Settings&, bool)>& onSave);
   explicit InGameUIItem(const VPX::Properties::IntPropertyDef& prop, const string& format, const std::function<int()>& getValue, const std::function<int(Settings&)>& getStoredValue,
      const std::function<void(int, int)>& onChange, const std::function<void(Settings&)>& onResetSave, const std::function<void(int, Settings&, bool)>& onSave);
   explicit InGameUIItem(const VPX::Properties::EnumPropertyDef& prop, const std::function<int()>& getValue, const std::function<int(Settings&)>& getStoredValue,
      const std::function<void(int, int)>& onChange, const std::function<void(Settings&)>& onResetSave, const std::function<void(int, Settings&, bool)>& onSave);
   explicit InGameUIItem(const VPX::Properties::BoolPropertyDef& prop, const std::function<bool()>& getValue, const std::function<bool(Settings&)>& getStoredValue,
      const std::function<void(bool)>& onChange, const std::function<void(Settings&)>& onResetSave, const std::function<void(float, Settings&, bool)>& onSave);

   bool IsSelectable() const { return m_type != Type::Label || m_labelType == LabelType::Markdown; }
   bool IsAdjustable() const { return m_type == Type::Property || m_type == Type::ActionInputMapping || m_type == Type::PhysicsSensorMapping; }

   float GetFloatValue() const;
   int GetIntValue() const;
   bool GetBoolValue() const;
   void SetValue(float value) const;
   void SetValue(int value) const;
   void SetValue(bool value) const;

   bool IsModified() const;
   void ResetToStoredValue();
   void ResetSave(Settings& settings) const;
   void Save(Settings& settings, bool isTableOverride);

   bool IsDefaultValue() const;
   void ResetToDefault();

   // Common definition
   const Type m_type;
   const string m_label;
   const string m_tooltip;

   // Label
   const LabelType m_labelType = LabelType::Header;

   // Input mapping
   class InputAction* m_inputAction = nullptr;
   class PhysicsSensor* m_physicsSensor = nullptr;
   int m_physicsSensorTypeMask = 0;

   // Navigation item
   const string m_path;

   // Generic runnable item
   const std::function<void(void)> m_runnable;

   // Properties
   std::unique_ptr<VPX::Properties::PropertyDef> m_property;
   const float m_floatValueDisplayScale = 1.f;
   const string m_format;

   // Custom render callback for CustomRender items
   const std::function<void(int, const InGameUIItem*)> m_customRender;

   // UI behavior
   bool m_excludeFromDefault = false; // If set this item is not reseted to its default value when user request a reset

private:
   string m_defMappingString;
   string m_initialMappingString;

   const std::function<bool()> m_getBoolValue;
   const std::function<bool(Settings&)> m_getStoredBoolValue;
   const std::function<void(bool)> m_onChangeBool;
   const std::function<int()> m_getIntValue;
   const std::function<int(Settings&)> m_getStoredIntValue;
   const std::function<void(int, int)> m_onChangeInt;
   const std::function<float()> m_getFloatValue;
   const std::function<float(Settings&)> m_getStoredFloatValue;
   const std::function<void(float, float)> m_onChangeFloat;
   const std::function<void(Settings&)> m_onResetSave;
   const std::function<void(int, Settings&, bool)> m_onSaveInt;
   const std::function<void(bool, Settings&, bool)> m_onSaveBool;
   const std::function<void(float, Settings&, bool)> m_onSaveFloat;
};

}
