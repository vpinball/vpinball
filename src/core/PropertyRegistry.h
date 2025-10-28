// license:GPLv3+

#pragma once

#include "PropertyDef.h"
#include "unordered_dense.h"

namespace VPX::Properties
{

class PropertyRegistry
{
public:
   enum class StoreType : uint8_t
   {
      Float,
      Int,
      String,
   };

   static StoreType GetStoreType(PropertyDef::Type type)
   {
      switch (type)
      {
      case PropertyDef::Type::Float: return StoreType::Float;
      case PropertyDef::Type::Int: return StoreType::Int;
      case PropertyDef::Type::Bool: return StoreType::Int;
      case PropertyDef::Type::Enum: return StoreType::Int;
      case PropertyDef::Type::String: return StoreType::String;
      default: assert(false);
      }
      return StoreType::Float;
   }

   struct PropId
   {
      StoreType type;
      uint16_t index;
   };

   PropId Register(std::unique_ptr<PropertyDef> prop)
   {
      const string& id = prop->m_groupId + '.' + prop->m_propId;
      const auto it = m_idToIndexMap.find(id);
      PropId propId;
      if (it != m_idToIndexMap.end())
      {
         // Redefining a property is allowed as long as nothing but the default value change
         propId = it->second;
         const PropertyDef* existing = GetProperty(propId);
         assert(existing->IsEqualButDefaultValue(prop.get()));
         switch (propId.type)
         {
         case StoreType::Float: m_floatProperties[propId.index] = std::move(prop); break;
         case StoreType::Int: m_intProperties[propId.index] = std::move(prop); break;
         case StoreType::String: m_stringProperties[propId.index] = std::move(prop); break;
         default: assert(false); break;
         }
         return propId;
      }
      propId.type = GetStoreType(prop->m_type);
      switch (propId.type)
      {
      case StoreType::Float:
         propId.index = static_cast<uint16_t>(m_floatProperties.size());
         m_floatProperties.push_back(std::move(prop));
         break;
      case StoreType::Int:
         propId.index = static_cast<uint16_t>(m_intProperties.size());
         m_intProperties.push_back(std::move(prop));
         break;
      case StoreType::String:
         propId.index = static_cast<uint16_t>(m_stringProperties.size());
         m_stringProperties.push_back(std::move(prop));
         break;
      default: assert(false); break;
      }
      m_idToIndexMap[id] = propId;
      return propId;
   }

   const PropertyDef* GetProperty(PropId propId) const
   {
      switch (propId.type)
      {
      case StoreType::Float:
         if (propId.index < m_floatProperties.size())
            return m_floatProperties[propId.index].get();
         break;
      case StoreType::Int:
         if (propId.index < m_intProperties.size())
            return m_intProperties[propId.index].get();
         break;
      case StoreType::String:
         if (propId.index < m_stringProperties.size())
            return m_stringProperties[propId.index].get();
         break;
      default: assert(false); break;
      }
      return nullptr;
   }

   const FloatPropertyDef* GetFloatProperty(PropId propId) const { return dynamic_cast<const FloatPropertyDef*>(GetProperty(propId)); }
   const IntPropertyDef* GetIntProperty(PropId propId) const { return dynamic_cast<const IntPropertyDef*>(GetProperty(propId)); }
   const EnumPropertyDef* GetEnumProperty(PropId propId) const { return dynamic_cast<const EnumPropertyDef*>(GetProperty(propId)); }
   const BoolPropertyDef* GetBoolProperty(PropId propId) const { return dynamic_cast<const BoolPropertyDef*>(GetProperty(propId)); }
   const StringPropertyDef* GetStringProperty(PropId propId) const { return dynamic_cast<const StringPropertyDef*>(GetProperty(propId)); }

   vector<PropId> GetPropertyIds() const
   {
      vector<PropId> ids;
      for (const auto& pair : m_idToIndexMap)
         ids.push_back(pair.second);
      return ids;
   }

private:
   vector<std::unique_ptr<PropertyDef>> m_floatProperties;
   vector<std::unique_ptr<PropertyDef>> m_intProperties;
   vector<std::unique_ptr<PropertyDef>> m_stringProperties;
   ankerl::unordered_dense::map<string, PropId> m_idToIndexMap;
};

};