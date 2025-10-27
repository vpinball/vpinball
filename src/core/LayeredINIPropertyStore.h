// license:GPLv3+

#pragma once

#include "PropertyStore.h"

#include <optional>

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"

namespace VPX::Properties
{

// Property store based on INI text files (sections + key/value pairs), supporting parenting to a base property store
// 
// Properties may either have a defined value, or be in an 'undefined' state (before load or after reset). If undefined, they will
// return the property's default. When saved, undefined state is saved as the property key without any value associated with it.
// 
// When parented to a base property store:
// - the value returned by the parent property store is used as the default value for each property.
// - only property with value that are not equal to parent's value are persisted in the INI (they override the parent)
// - reseting a property of a child store will turn it to an undefined state (returning parent's value), but does not affect the parent
// - saving will also save the parent
class LayeredINIPropertyStore final : public PropertyStore
{
public:
   explicit LayeredINIPropertyStore(PropertyRegistry& registry);
   LayeredINIPropertyStore(LayeredINIPropertyStore& parent);
   ~LayeredINIPropertyStore() override = default;

   bool Load(const string& path);
   void Save();
   bool IsModified() const { return m_modified; }

   void Reset(PropertyRegistry::PropId propId) override;

   int GetDefaultInt(PropertyRegistry::PropId propId) const;
   int GetInt(PropertyRegistry::PropId propId) const override;
   void Set(PropertyRegistry::PropId propId, int value) override;

   float GetDefaultFloat(PropertyRegistry::PropId propId) const;
   float GetFloat(PropertyRegistry::PropId propId) const override;
   void Set(PropertyRegistry::PropId propId, float value) override;

   const string& GetDefaultString(PropertyRegistry::PropId propId) const;
   const string& GetString(PropertyRegistry::PropId propId) const override;
   void Set(PropertyRegistry::PropId propId, const string& value) override;

private:
   std::optional<std::reference_wrapper<LayeredINIPropertyStore>> m_parent;

   // Live data
   bool m_modified = false;
   struct IntValue
   {
      bool defined;
      int value;
   };
   vector<IntValue> m_intValues;
   struct FloatValue
   {
      bool defined;
      float value;
   };
   vector<FloatValue> m_floatValues;
   struct StringValue
   {
      bool defined;
      string value;
   };
   vector<StringValue> m_stringValues;
   void UpdateLiveStore();
   IntValue& GetIntValue(PropertyRegistry::PropId propId);
   FloatValue& GetFloatValue(PropertyRegistry::PropId propId);
   StringValue& GetStringValue(PropertyRegistry::PropId propId);

   // Serialized data
   string m_path;
   mINI::INIStructure m_ini;
   bool LoadFromINI(PropertyRegistry::PropId id);
};

};