// license:GPLv3+

#pragma once

#include <string>
#include <vector>
#include "core/def.h"

namespace VPX::Properties
{

using std::string;
using std::vector;

class PropertyDef
{
public:
   enum class Type
   {
      Float,
      Int,
      Bool,
      Enum,
      String,
      //Filepath,
      //Dirpath,
   };

   const Type m_type;
   const string m_groupId;
   const string m_propId;
   const string m_label;
   const string m_description;

   // The property definition depends on the live context, therefore it is allowed to change at runtime (range, default,...).
   // Its value must be saved even if equal to the property's default as this default may change over sessions.
   const bool m_contextualProperty;

   virtual ~PropertyDef() = default;
   bool operator==(const PropertyDef& o) const
   {
      return m_type == o.m_type && m_groupId == o.m_groupId && m_propId == o.m_propId && m_label == o.m_label && m_description == o.m_description;
   }

protected:
   PropertyDef(Type type, const string& groupId, const string& propId, const string& label, const string& description, bool isContextual)
      : m_type(type)
      , m_groupId(groupId)
      , m_propId(propId)
      , m_label(label)
      , m_description(description)
      , m_contextualProperty(isContextual)
   {
   }
};


class FloatPropertyDef final : public PropertyDef
{
public:
   FloatPropertyDef(const string& groupId, const string& propId, const string& label, const string& description, bool isContextual, float min, float max, float step, float def)
      : PropertyDef(Type::Float, groupId, propId, label, description, isContextual)
      , m_min(min)
      , m_max(max)
      , m_step(step)
      , m_def(GetSteppedClamped(def))
   {
      assert(m_min <= m_max);
      assert(m_def == GetSteppedClamped(m_def));
   }
   FloatPropertyDef(const FloatPropertyDef& other)
      : FloatPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, other.m_min, other.m_max, other.m_step, other.m_def)
   {
   }
   FloatPropertyDef(const FloatPropertyDef& other, float def)
      : FloatPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, other.m_min, other.m_max, other.m_step, def)
   {
   }
   std::unique_ptr<FloatPropertyDef> WithDefault(float def) const { return std::make_unique<FloatPropertyDef>(*this, def); }
   ~FloatPropertyDef() override = default;

   const float m_min;
   const float m_max;
   const float m_step;
   const float m_def;

   float GetStepped(float v) const { return (m_step != 0.f) ? (m_min + roundf((v - m_min) / m_step) * m_step) : v; }
   float GetClamped(float v) const { return clamp(v, m_min, m_max); }
   float GetSteppedClamped(float v) const { return GetClamped(GetStepped(v)); }
   float GetValid(float v) const { return (v < m_min || v > m_max) ? m_def : GetSteppedClamped(v); }

   bool operator==(const FloatPropertyDef& o) const { return PropertyDef::operator==(o) && (m_min == o.m_min) && (m_max == o.m_max) && (m_step == o.m_def) && (m_def == o.m_def); }
};


class IntPropertyDef final : public PropertyDef
{
public:
   IntPropertyDef(const string& groupId, const string& propId, const string& label, const string& description, bool isContextual, int min, int max, int def)
      : PropertyDef(Type::Int, groupId, propId, label, description, isContextual)
      , m_min(min)
      , m_max(max)
      , m_def(clamp(def, min, max))
   {
      assert(m_min <= m_max);
   }
   IntPropertyDef(const IntPropertyDef& other)
      : IntPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, other.m_min, other.m_max, other.m_def)
   {
   }
   IntPropertyDef(const IntPropertyDef& other, int min, int max)
      : IntPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, min, max, other.m_def)
   {
   }
   IntPropertyDef(const IntPropertyDef& other, int def)
      : IntPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, other.m_min, other.m_max, def)
   {
   }
   std::unique_ptr<IntPropertyDef> WithDefault(int def) const { return std::make_unique<IntPropertyDef>(*this, def); }
   std::unique_ptr<IntPropertyDef> WithRange(int minVal, int maxVal) const { return std::make_unique<IntPropertyDef>(*this, minVal, max(minVal, maxVal)); }
   ~IntPropertyDef() override = default;

   const int m_min;
   const int m_max;
   const int m_def;

   int GetClamped(int v) const { return clamp(v, m_min, m_max); }

   int GetValid(int v) const { return (v < m_min || v > m_max) ? m_def : v; }

   bool operator==(const IntPropertyDef& o) const { return PropertyDef::operator==(o) && (m_min == o.m_min) && (m_max == o.m_max) && (m_def == o.m_def); }
};


class EnumPropertyDef final : public PropertyDef
{
public:
   EnumPropertyDef(const string& groupId, const string& propId, const string& label, const string& description, bool isContextual, int min, int def, vector<string> values)
      : PropertyDef(Type::Enum, groupId, propId, label, description, isContextual)
      , m_min(min)
      , m_values(std::move(values))
      , m_def(def)
   {
      assert(!m_values.empty());
   }
   EnumPropertyDef(const EnumPropertyDef& other)
      : EnumPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, other.m_min, other.m_def, other.m_values)
   {
   }
   EnumPropertyDef(const EnumPropertyDef& other, int def)
      : EnumPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, other.m_min, def, other.m_values)
   {
   }
   std::unique_ptr<EnumPropertyDef> WithDefault(int def) const { return std::make_unique<EnumPropertyDef>(*this, def); }
   ~EnumPropertyDef() override = default;

   int m_min;
   const vector<string> m_values;
   int m_def;

   bool IsValid(int v) const { return m_min <= v && v < (m_min + (int)m_values.size()); }
   bool IsValid(const string& v) const { return GetEnum(v) >= m_min; }
   int GetValid(int v) const { return IsValid(v) ? v : m_def; }
   int GetEnum(const string& v) const
   {
      const auto it = std::ranges::find(m_values, v);
      return m_min + (it == m_values.end() ? -1 : static_cast<int>(std::distance(m_values.begin(), it)));
   }
   int ParseEnum(const string& v) const
   {
      int index = GetEnum(v);
      if (index >= m_min)
         return index;
      if (try_parse_int(v, index) && m_min <= index && index < (m_min + (int)m_values.size()))
         return index;
      return m_min - 1;
   }
   const string& GetEnum(int index) const { return m_values[GetValid(index)]; }

   bool operator==(const EnumPropertyDef& o) const { return PropertyDef::operator==(o) && (m_min == o.m_min) && (m_values == o.m_values) && (m_def == o.m_def); }
};


class BoolPropertyDef final : public PropertyDef
{
public:
   BoolPropertyDef(const string& groupId, const string& propId, const string& label, const string& description, bool isContextual, bool def)
      : PropertyDef(Type::Bool, groupId, propId, label, description, isContextual)
      , m_def(def)
   {
   }
   BoolPropertyDef(const BoolPropertyDef& other, bool def)
      : BoolPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, def)
   {
   }
   std::unique_ptr<BoolPropertyDef> WithDefault(bool def) const { return std::make_unique<BoolPropertyDef>(*this, def); }
   ~BoolPropertyDef() override = default;

   const bool m_def;

   bool operator==(const BoolPropertyDef& o) const { return PropertyDef::operator==(o) && (m_def == o.m_def); }
};


class StringPropertyDef final : public PropertyDef
{
public:
   StringPropertyDef(const string& groupId, const string& propId, const string& label, const string& description, bool isContextual, const string& def)
      : PropertyDef(Type::String, groupId, propId, label, description, isContextual)
      , m_def(def)
   {
   }
   StringPropertyDef(const StringPropertyDef& other, const string& def)
      : StringPropertyDef(other.m_groupId, other.m_propId, other.m_label, other.m_description, other.m_contextualProperty, def)
   {
   }
   std::unique_ptr<StringPropertyDef> WithDefault(const string& def) const { return std::make_unique<StringPropertyDef>(*this, def); }
   ~StringPropertyDef() override = default;

   const string m_def;

   bool operator==(const StringPropertyDef& o) const { return PropertyDef::operator==(o) && (m_def == o.m_def); }
};

}
