// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/LayeredINIPropertyStore.h"

#include "doctest.h"

using namespace VPX::Properties;

namespace
{
struct TestPropIds
{
   PropertyRegistry::PropId boolProp;
   PropertyRegistry::PropId intProp;
   PropertyRegistry::PropId enumProp;
   PropertyRegistry::PropId floatProp;
   PropertyRegistry::PropId stringProp;
   PropertyRegistry::PropId contextualInt;
};

TestPropIds RegisterTestProps(PropertyRegistry& registry)
{
   TestPropIds ids;
   ids.boolProp = registry.Register(std::make_unique<BoolPropertyDef>("TestGroup"s, "BoolProp"s, "Bool"s, ""s, false, true));
   ids.intProp = registry.Register(std::make_unique<IntPropertyDef>("TestGroup"s, "IntProp"s, "Int"s, ""s, false, 0, 100, 42));
   ids.enumProp = registry.Register(std::make_unique<EnumPropertyDef>("TestGroup"s, "EnumProp"s, "Enum"s, ""s, false, 0, 1, vector<string> { "Zero"s, "One"s, "Two"s }));
   ids.floatProp = registry.Register(std::make_unique<FloatPropertyDef>("TestGroup"s, "FloatProp"s, "Float"s, ""s, false, 0.f, 10.f, 0.5f, 2.5f));
   ids.stringProp = registry.Register(std::make_unique<StringPropertyDef>("TestGroup"s, "StringProp"s, "String"s, ""s, false, "default"s));
   ids.contextualInt = registry.Register(std::make_unique<IntPropertyDef>("TestGroup"s, "CtxInt"s, "Ctx"s, ""s, true, 0, 100, 5));
   return ids;
}
}

TEST_CASE("LayeredINIPropertyStore")
{
   SUBCASE("undefined properties return their default value")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      CHECK(store.GetInt(ids.boolProp) == 1);
      CHECK(store.GetInt(ids.intProp) == 42);
      CHECK(store.GetInt(ids.enumProp) == 1);
      CHECK(store.GetFloat(ids.floatProp) == 2.5f);
      CHECK(store.GetString(ids.stringProp) == "default");
      CHECK(store.GetDefaultInt(ids.intProp) == 42);
      CHECK(store.GetDefaultFloat(ids.floatProp) == 2.5f);
      CHECK(store.GetDefaultString(ids.stringProp) == "default");
      CHECK_FALSE(store.IsModified());
   }

   SUBCASE("set, get and reset")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      store.Set(ids.intProp, 7);
      CHECK(store.GetInt(ids.intProp) == 7);
      CHECK(store.GetDefaultInt(ids.intProp) == 42);
      CHECK(store.IsModified());

      store.Reset(ids.intProp);
      CHECK(store.GetInt(ids.intProp) == 42);

      store.Reset();
      CHECK_FALSE(store.IsModified());
      CHECK(store.GetInt(ids.intProp) == 42);
   }

   SUBCASE("set only marks modified on change")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      store.Set(ids.intProp, 7);
      store.SetModified(false);
      store.Set(ids.intProp, 7); // same defined value: not a change
      CHECK_FALSE(store.IsModified());
      store.Set(ids.intProp, 8);
      CHECK(store.IsModified());
   }

   SUBCASE("ini file save/load round-trip")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      const std::filesystem::path iniPath = GetTestTmpDir() / "layered-store.ini";

      {
         LayeredINIPropertyStore store(registry);
         store.SetIniPath(iniPath);
         store.Set(ids.intProp, 7);
         store.Set(ids.floatProp, 4.5f);
         store.Set(ids.stringProp, "hello"s);
         store.Set(ids.boolProp, 0);
         store.Save();
         CHECK_FALSE(store.IsModified());
      }
      REQUIRE(FileExists(iniPath));

      LayeredINIPropertyStore loaded(registry);
      loaded.SetIniPath(iniPath);
      CHECK(loaded.Load());
      CHECK(loaded.GetInt(ids.intProp) == 7);
      CHECK(loaded.GetFloat(ids.floatProp) == 4.5f);
      CHECK(loaded.GetString(ids.stringProp) == "hello");
      CHECK(loaded.GetInt(ids.boolProp) == 0);
      CHECK(loaded.GetInt(ids.enumProp) == 1); // untouched stays at default
      CHECK(!loaded.IsModified());
   }

   SUBCASE("undefined properties are persisted as empty values")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      const std::filesystem::path iniPath = GetTestTmpDir() / "layered-store-undef.ini";

      LayeredINIPropertyStore store(registry);
      store.SetIniPath(iniPath);
      store.Set(ids.intProp, 7);
      store.Save();

      mINI::INIStructure ini;
      REQUIRE(mINI::INIFile(iniPath).read(ini));
      REQUIRE(ini.has("TestGroup"s));
      CHECK(ini["TestGroup"s]["IntProp"s] == "7");
      CHECK(ini["TestGroup"s].has("BoolProp"s)); // defined key, but no value
      CHECK(ini["TestGroup"s]["BoolProp"s].empty());
   }

   SUBCASE("save removes the ini file when no property is defined")
   {
      PropertyRegistry registry; // no registered property at all
      const std::filesystem::path iniPath = GetTestTmpDir() / "layered-store-empty.ini";
      write_file(iniPath.string(), vector<uint8_t> { 'x' });
      REQUIRE(FileExists(iniPath));

      LayeredINIPropertyStore store(registry);
      store.SetIniPath(iniPath);
      store.Save();
      CHECK_FALSE(FileExists(iniPath));
   }

   SUBCASE("load parses ini values by property type")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      mINI::INIStructure ini;
      ini["TestGroup"s]["BoolProp"s] = "off";
      ini["TestGroup"s]["IntProp"s] = "55";
      ini["TestGroup"s]["EnumProp"s] = "Two";
      ini["TestGroup"s]["FloatProp"s] = "3.25";
      ini["TestGroup"s]["StringProp"s] = "abc";
      ini["TestGroup"s]["CtxInt"s] = "9";
      store.Load(ini);

      CHECK(store.GetInt(ids.boolProp) == 0);
      CHECK(store.GetInt(ids.intProp) == 55);
      CHECK(store.GetInt(ids.enumProp) == 2);
      CHECK(store.GetFloat(ids.floatProp) == 3.25f);
      CHECK(store.GetString(ids.stringProp) == "abc");
      CHECK(store.GetInt(ids.contextualInt) == 9);
   }

   SUBCASE("load accepts bool literals and enum indices")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      for (const char* literal : { "1", "true", "TRUE", "on", "enable" })
      {
         mINI::INIStructure ini;
         ini["TestGroup"s]["BoolProp"s] = literal;
         store.Load(ini);
         CHECK(store.GetInt(ids.boolProp) == 1);
      }
      for (const char* literal : { "0", "false", "off", "disable" })
      {
         mINI::INIStructure ini;
         ini["TestGroup"s]["BoolProp"s] = literal;
         store.Load(ini);
         CHECK(store.GetInt(ids.boolProp) == 0);
      }
      for (const char* literal : { "0", "Zero" })
      {
         mINI::INIStructure ini;
         ini["TestGroup"s]["EnumProp"s] = literal;
         store.Load(ini);
         CHECK(store.GetInt(ids.enumProp) == 0);
      }
   }

   SUBCASE("load ignores invalid values")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      mINI::INIStructure ini;
      ini["TestGroup"s]["BoolProp"s] = "maybe";
      ini["TestGroup"s]["IntProp"s] = "abc";
      ini["TestGroup"s]["EnumProp"s] = "Three";
      ini["TestGroup"s]["FloatProp"s] = "fast";
      store.Load(ini);

      CHECK(store.GetInt(ids.boolProp) == 1); // all invalid values stay undefined
      CHECK(store.GetInt(ids.intProp) == 42);
      CHECK(store.GetInt(ids.enumProp) == 1);
      CHECK(store.GetFloat(ids.floatProp) == 2.5f);
   }

   SUBCASE("empty string ini value means undefined")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore store(registry);

      mINI::INIStructure ini;
      ini["TestGroup"s]["StringProp"s] = "";
      store.Load(ini);
      CHECK(store.GetString(ids.stringProp) == "default");
   }

   SUBCASE("child store inherits and overrides its parent")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      LayeredINIPropertyStore parent(registry);
      LayeredINIPropertyStore child(parent);

      parent.Set(ids.intProp, 7);
      CHECK(child.GetInt(ids.intProp) == 7); // parent's live value is the child's default

      child.Set(ids.intProp, 9);
      CHECK(child.GetInt(ids.intProp) == 9);
      CHECK(parent.GetInt(ids.intProp) == 7);

      child.Reset(ids.intProp);
      CHECK(child.GetInt(ids.intProp) == 7); // reset falls back to parent, parent is not affected
      CHECK(parent.GetInt(ids.intProp) == 7);
   }

   SUBCASE("child store only persists overrides")
   {
      PropertyRegistry registry;
      const auto ids = RegisterTestProps(registry);
      const std::filesystem::path parentPath = GetTestTmpDir() / "layered-parent.ini";
      const std::filesystem::path childPath = GetTestTmpDir() / "layered-child.ini";

      LayeredINIPropertyStore parent(registry);
      parent.SetIniPath(parentPath);
      LayeredINIPropertyStore child(parent);
      child.SetIniPath(childPath);

      parent.Set(ids.intProp, 7);
      child.Set(ids.intProp, 7); // same as parent: not persisted in the child file
      child.Set(ids.floatProp, 4.5f); // different from parent: persisted
      child.Set(ids.contextualInt, 5); // same as parent but contextual: always persisted
      child.Save(); // also saves the parent

      mINI::INIStructure parentIni;
      REQUIRE(mINI::INIFile(parentPath).read(parentIni));
      CHECK(parentIni["TestGroup"s]["IntProp"s] == "7");

      mINI::INIStructure childIni;
      REQUIRE(mINI::INIFile(childPath).read(childIni));
      CHECK_FALSE(childIni["TestGroup"s].has("IntProp"s));
      float f = 0.f;
      REQUIRE(try_parse_float(childIni["TestGroup"s]["FloatProp"s], f));
      CHECK(f == 4.5f);
      CHECK(childIni["TestGroup"s]["CtxInt"s] == "5");
   }
}
