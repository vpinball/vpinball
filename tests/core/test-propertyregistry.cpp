// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/PropertyRegistry.h"

#include "doctest.h"

using namespace VPX::Properties;

TEST_CASE("PropertyRegistry")
{
   SUBCASE("register assigns per-type consecutive indices")
   {
      PropertyRegistry registry;
      const auto intProp1 = registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, false, 0, 10, 5));
      const auto floatProp = registry.Register(std::make_unique<FloatPropertyDef>("g"s, "b"s, "l"s, "d"s, false, 0.f, 1.f, 0.f, 0.5f));
      const auto intProp2 = registry.Register(std::make_unique<IntPropertyDef>("g"s, "c"s, "l"s, "d"s, false, 0, 10, 5));
      const auto stringProp = registry.Register(std::make_unique<StringPropertyDef>("g"s, "d"s, "l"s, "d"s, false, "x"s));
      const auto boolProp = registry.Register(std::make_unique<BoolPropertyDef>("g"s, "e"s, "l"s, "d"s, false, true));

      CHECK(intProp1.type == PropertyRegistry::StoreType::Int);
      CHECK(intProp1.index == 0);
      CHECK(intProp2.type == PropertyRegistry::StoreType::Int);
      CHECK(intProp2.index == 1);
      CHECK(boolProp.type == PropertyRegistry::StoreType::Int); // bools share the int store
      CHECK(boolProp.index == 2);
      CHECK(floatProp.type == PropertyRegistry::StoreType::Float);
      CHECK(floatProp.index == 0); // each store type has its own index space
      CHECK(stringProp.type == PropertyRegistry::StoreType::String);
      CHECK(stringProp.index == 0);
   }

   SUBCASE("property lookup")
   {
      PropertyRegistry registry;
      const auto intProp = registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, false, 0, 10, 5));
      const auto enumProp = registry.Register(std::make_unique<EnumPropertyDef>("g"s, "b"s, "l"s, "d"s, false, 0, 1, vector<string> { "x"s, "y"s }));

      CHECK(registry.GetProperty(intProp)->m_propId == "a");
      CHECK(registry.GetProperty(intProp)->m_groupId == "g");

      // Typed getters only match their own property type
      CHECK(registry.GetIntProperty(intProp) != nullptr);
      CHECK(registry.GetIntProperty(enumProp) == nullptr); // enums are not ints for the typed getters
      CHECK(registry.GetEnumProperty(enumProp) != nullptr);
      CHECK(registry.GetEnumProperty(intProp) == nullptr);
      CHECK(registry.GetFloatProperty(intProp) == nullptr);
      CHECK(registry.GetBoolProperty(intProp) == nullptr);
      CHECK(registry.GetStringProperty(intProp) == nullptr);
   }

   SUBCASE("lookup by group and property id")
   {
      PropertyRegistry registry;
      const auto intProp = registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, false, 0, 10, 5));
      registry.Register(std::make_unique<FloatPropertyDef>("g"s, "b"s, "l"s, "d"s, false, 0.f, 1.f, 0.f, 0.5f));

      const auto found = registry.GetPropertyId("g"s, "a"s);
      REQUIRE(found.has_value());
      CHECK(found->type == intProp.type);
      CHECK(found->index == intProp.index);
      CHECK_FALSE(registry.GetPropertyId("g"s, "missing"s).has_value());
      CHECK_FALSE(registry.GetPropertyId("other"s, "a"s).has_value());
   }

   SUBCASE("GetPropertyIds returns all registered ids")
   {
      PropertyRegistry registry;
      registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, false, 0, 10, 5));
      registry.Register(std::make_unique<FloatPropertyDef>("g"s, "b"s, "l"s, "d"s, false, 0.f, 1.f, 0.f, 0.5f));
      registry.Register(std::make_unique<StringPropertyDef>("h"s, "c"s, "l"s, "d"s, false, "x"s));
      CHECK(registry.GetPropertyIds().size() == 3);
   }

   SUBCASE("re-registering an identical definition returns the existing id")
   {
      PropertyRegistry registry;
      const auto first = registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, false, 0, 10, 5));
      const auto second = registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, false, 0, 10, 5));
      CHECK(first.type == second.type);
      CHECK(first.index == second.index);
      CHECK(registry.GetPropertyIds().size() == 1);
   }

   SUBCASE("contextual property definitions can be updated")
   {
      PropertyRegistry registry;
      const auto id = registry.Register(std::make_unique<IntPropertyDef>("g"s, "a"s, "l"s, "d"s, true, 0, 10, 5));
      registry.Register(registry.GetIntProperty(id)->WithDefault(9));
      CHECK(registry.GetIntProperty(id)->m_def == 9);
      CHECK(registry.GetPropertyIds().size() == 1);
   }
}
