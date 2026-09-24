// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/PropertyDef.h"
#include "core/PropertyRegistry.h"

#include "doctest.h"

using namespace VPX::Properties;

TEST_CASE("PropertyDef")
{
   SUBCASE("int property")
   {
      const IntPropertyDef prop("group"s, "prop"s, "label"s, "desc"s, false, 0, 100, 42);
      CHECK(prop.m_type == PropertyDef::Type::Int);
      CHECK(prop.m_def == 42);

      CHECK(prop.GetClamped(50) == 50);
      CHECK(prop.GetClamped(-5) == 0);
      CHECK(prop.GetClamped(150) == 100);

      CHECK(prop.GetValid(50) == 50);
      CHECK(prop.GetValid(-5) == 42); // out of range falls back to default
      CHECK(prop.GetValid(150) == 42);

      CHECK(prop.WithDefault(7)->m_def == 7);
      const auto ranged = prop.WithRange(10, 20);
      CHECK(ranged->m_min == 10);
      CHECK(ranged->m_max == 20);
   }

   SUBCASE("int property clamps its default to the range")
   {
      CHECK(IntPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0, 100, 150).m_def == 100);
      CHECK(IntPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0, 100, -5).m_def == 0);
   }

   SUBCASE("float property")
   {
      const FloatPropertyDef prop("group"s, "prop"s, "label"s, "desc"s, false, 0.f, 1.f, 0.25f, 0.5f);
      CHECK(prop.m_type == PropertyDef::Type::Float);
      CHECK(prop.m_def == 0.5f);

      CHECK(prop.GetStepped(0.3f) == doctest::Approx(0.25f)); // snapped to step grid
      CHECK(prop.GetStepped(0.4f) == doctest::Approx(0.5f));
      CHECK(prop.GetClamped(2.f) == 1.f);
      CHECK(prop.GetSteppedClamped(1.3f) == 1.f);
      CHECK(prop.GetSteppedClamped(-0.4f) == 0.f);

      CHECK(prop.GetValid(0.75f) == doctest::Approx(0.75f));
      CHECK(prop.GetValid(2.f) == 0.5f); // out of range falls back to default
      CHECK(prop.GetValid(-1.f) == 0.5f);

      CHECK(prop.WithDefault(0.75f)->m_def == 0.75f);
   }

   SUBCASE("float property without step is not snapped")
   {
      const FloatPropertyDef prop("g"s, "p"s, "l"s, "d"s, false, 0.f, 1.f, 0.f, 0.5f);
      CHECK(prop.GetStepped(0.333f) == doctest::Approx(0.333f));
      CHECK(prop.GetSteppedClamped(0.333f) == doctest::Approx(0.333f));
   }

   SUBCASE("float property normalizes its default to the step grid")
   {
      CHECK(FloatPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0.f, 1.f, 0.25f, 0.6f).m_def == doctest::Approx(0.5f));
      CHECK(FloatPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0.f, 1.f, 0.f, 1.5f).m_def == 1.f);
   }

   SUBCASE("enum property")
   {
      EnumPropertyDef prop("group"s, "prop"s, "label"s, "desc"s, false, 0, 1, vector<string> { "A"s, "B"s, "C"s });
      CHECK(prop.m_type == PropertyDef::Type::Enum);
      CHECK(prop.m_def == 1);

      CHECK(prop.IsValid(0));
      CHECK(prop.IsValid(2));
      CHECK_FALSE(prop.IsValid(3));
      CHECK_FALSE(prop.IsValid(-1));
      CHECK(prop.IsValid("B"s));
      CHECK_FALSE(prop.IsValid("X"s));

      CHECK(prop.GetEnum("A"s) == 0);
      CHECK(prop.GetEnum("C"s) == 2);
      CHECK(prop.GetEnum("X"s) == -1); // unknown name maps below min
      CHECK(prop.GetEnum(0) == "A");
      CHECK(prop.GetEnum(99) == "B"); // invalid index falls back to default

      CHECK(prop.ParseEnum("B"s) == 1);
      CHECK(prop.ParseEnum("2"s) == 2); // numeric literal is accepted
      CHECK(prop.ParseEnum("5"s) == -1);
      CHECK(prop.ParseEnum("nope"s) == -1);

      CHECK(prop.GetValid(2) == 2);
      CHECK(prop.GetValid(9) == 1); // invalid falls back to default

      CHECK(prop.WithDefault(2)->m_def == 2);
   }

   SUBCASE("enum property with non-zero minimum")
   {
      EnumPropertyDef prop("g"s, "p"s, "l"s, "d"s, false, 1, 2, vector<string> { "X"s, "Y"s });
      CHECK(prop.GetEnum("X"s) == 1);
      CHECK(prop.GetEnum("Y"s) == 2);
      CHECK_FALSE(prop.IsValid(0));
      CHECK(prop.IsValid(2));
      CHECK(prop.ParseEnum("1"s) == 1);
      CHECK(prop.ParseEnum("0"s) == 0); // m_min - 1, i.e. invalid
      CHECK_FALSE(prop.IsValid(prop.ParseEnum("0"s)));
   }

   SUBCASE("bool and string properties")
   {
      const BoolPropertyDef boolProp("g"s, "p"s, "l"s, "d"s, false, true);
      CHECK(boolProp.m_type == PropertyDef::Type::Bool);
      CHECK(boolProp.m_def == true);
      CHECK(boolProp.WithDefault(false)->m_def == false);

      const StringPropertyDef stringProp("g"s, "p"s, "l"s, "d"s, false, "abc"s);
      CHECK(stringProp.m_type == PropertyDef::Type::String);
      CHECK(stringProp.m_def == "abc");
      CHECK(stringProp.WithDefault("xyz"s)->m_def == "xyz");
   }

   SUBCASE("equality compares definition fields")
   {
      const IntPropertyDef a("g"s, "p"s, "l"s, "d"s, false, 0, 100, 42);
      CHECK(a == IntPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0, 100, 42));
      CHECK_FALSE(a == IntPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0, 100, 43));
      CHECK_FALSE(a == IntPropertyDef("g"s, "p"s, "l"s, "d"s, false, 0, 99, 42));
      CHECK_FALSE(a == IntPropertyDef("g"s, "q"s, "l"s, "d"s, false, 0, 100, 42));

      // Different property types never compare equal
      const BoolPropertyDef b("g"s, "p"s, "l"s, "d"s, false, true);
      CHECK_FALSE(a == b);
   }

   SUBCASE("store type mapping")
   {
      CHECK(PropertyRegistry::GetStoreType(PropertyDef::Type::Float) == PropertyRegistry::StoreType::Float);
      CHECK(PropertyRegistry::GetStoreType(PropertyDef::Type::Int) == PropertyRegistry::StoreType::Int);
      CHECK(PropertyRegistry::GetStoreType(PropertyDef::Type::Bool) == PropertyRegistry::StoreType::Int);
      CHECK(PropertyRegistry::GetStoreType(PropertyDef::Type::Enum) == PropertyRegistry::StoreType::Int);
      CHECK(PropertyRegistry::GetStoreType(PropertyDef::Type::String) == PropertyRegistry::StoreType::String);
   }
}
