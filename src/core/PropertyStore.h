// license:GPLv3+

#pragma once

#include "PropertyDef.h"
#include "PropertyRegistry.h"

namespace VPX::Properties
{

class PropertyStore
{
public:
   explicit PropertyStore(PropertyRegistry& registry)
      : m_registry(registry)
   {
   }
   virtual ~PropertyStore() = default;

   virtual void Reset(PropertyRegistry::PropId propId) = 0;

   virtual int GetInt(PropertyRegistry::PropId propId) const = 0;
   virtual void Set(PropertyRegistry::PropId propId, int value) = 0;

   virtual float GetFloat(PropertyRegistry::PropId propId) const = 0;
   virtual void Set(PropertyRegistry::PropId propId, float value) = 0;

   virtual const string& GetString(PropertyRegistry::PropId propId) const = 0;
   virtual void Set(PropertyRegistry::PropId propId, const string& value) = 0;

   std::reference_wrapper<PropertyRegistry> m_registry;

private:

};

};