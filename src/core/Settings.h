// license:GPLv3+

#pragma once

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "unordered_dense.h"

// This class holds the settings registry.
// A setting registry can have a parent, in which case, missing settings will be looked for in the parent.
// This is used to allow overriding part of the settings for a specific table while still using the base application value for others.
// When saving value, an 'override mode' can be used where the value will be saved only if it not the same as the one in the parent.
class Settings final
{
public:
   Settings(const Settings* parent = nullptr);

   void SetParent(const Settings *parent) { m_parent = parent; }

   void SetIniPath(const string &path) { m_iniPath = path; }
   bool LoadFromFile(const string &path, const bool createDefault);
   void SaveToFile(const string &path);
   void Save();

   void CopyOverrides(const Settings &settings);

   // Only actually save the settings if they have been modified. If you want to force a save (for example if filepath has changed), you need to explicitly set the modified flag
   bool IsModified() const { return m_modified; }
   void SetModified(const bool modified) { m_modified = modified; }

   enum Section
   {
      Controller,

      // UI and Player stuff
      Editor,
      Standalone,
      Player, /* Main playfield Rendering & Display */
      DMD, /* DMD Rendering */
      Alpha, /* Alpha segment Rendering */
      Backglass, /* Backglass Display */
      ScoreView, /* Scoreview Display */
      Topper, /* Topper Display */
      PlayerVR,
      RecentDir,
      Version,
      CVEdit,
      TableOverride,
      TableOption,
      ControllerDevices,

      // Optional user defaults for each element
      DefaultPropsBall,
      DefaultPropsBumper,
      DefaultPropsDecal,
      DefaultPropsEMReel,
      DefaultPropsFlasher,
      DefaultPropsFlipper,
      DefaultPropsGate,
      DefaultPropsHitTarget,
      DefaultPropsKicker,
      DefaultPropsLight,
      DefaultPropsLightSequence,
      DefaultPropsPlunger,
      DefaultPropsPrimitive,
      DefaultPropsRamp,
      DefaultPropsRubber,
      DefaultPropsSpinner,
      DefaultPropsWall,
      DefaultPropsTarget,
      DefaultPropsTextBox,
      DefaultPropsTimer,
      DefaultPropsTrigger,
      DefaultCamera,
      DefaultPropsPartGroup,

      // Plugin pages
      Plugin00
   };

   static Section GetSection(const string& szName);
   static const string& GetSectionName(const Section section);
   static int GetNPluginSections() { return (int)m_settingKeys.size() - Plugin00; }

   bool HasValue(const Section section, const string &key, const bool searchParent = false) const;

   bool LoadValue(const Section section, const string &key, string &val) const;
   bool LoadValue(const Section section, const string &key, float &pfloat) const;
   bool LoadValue(const Section section, const string &key, int &pint) const;
   bool LoadValue(const Section section, const string &key, unsigned int &val) const;

   // The following method must only be used for settings previously validated to guarantee successfull loading
   void Validate(const bool addDefaults);
   void ResetValue(const Section section, const string &key);
   string LoadValueString(const Section section, const string &key) const { string v; LoadValue(section, key, v); return v; }
   float LoadValueFloat(const Section section, const string &key) const { float v; bool ok = LoadValue(section, key, v); assert(ok); return v; }
   bool LoadValueBool(const Section section, const string &key) const { unsigned int v; bool ok = LoadValue(section, key, v); assert(ok); return !!v; }
   int LoadValueInt(const Section section, const string &key) const { int v; bool ok = LoadValue(section, key, v); assert(ok); return v; }
   unsigned int LoadValueUInt(const Section section, const string &key) const { unsigned int v; bool ok = LoadValue(section, key, v); assert(ok); return v; }

   float LoadValueWithDefault(const Section section, const string &key, const float def) const;
   int LoadValueWithDefault(const Section section, const string &key, const int def) const;
   bool LoadValueWithDefault(const Section section, const string &key, const bool def) const;
   string LoadValueWithDefault(const Section section, const string &key, const string &def) const;

   bool SaveValue(const Section section, const string &key, const string &val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const float val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const int val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const unsigned int val, const bool overrideMode = false);
   bool SaveValue(const Section section, const string &key, const bool val, const bool overrideMode = false);
    
   bool DeleteValue(const Section section, const string &key, const bool deleteFromParent = false);
   bool DeleteSubKey(const Section section, const bool deleteFromParent = false);

   enum OptionUnit
   {
      OT_NONE, // Display without a unit
      OT_PERCENT, // Shows valut multiplied by 100, with % as the unit
   };
   struct OptionDef
   {
      Section section;
      string id, name;
      int showMask;
      float minValue, maxValue, step, defaultValue;
      OptionUnit unit;
      vector<string> literals;
      string tokenizedLiterals;
   };
   void RegisterSetting(const Section section, const string &id, const unsigned int showMask, const string &name, float minValue, float maxValue, float step, float defaultValue, OptionUnit unit, const vector<string> &literals);
   const vector<OptionDef>& GetTableSettings() const { return m_tableOptions; }
   static const vector<OptionDef>& GetPluginSettings() { return m_pluginOptions; }

private:
#if 0
   bool LoadValue(const Section section, const string &key, void *const szbuffer, const size_t size) const;
   bool SaveValue(const Section section, const string &key, const char *val, const bool overrideMode = false);
#endif

   void RegisterStringSetting(const Section section, const string &key, const string &defVal, const bool addDefaults, const string &comments = string());
   void RegisterBoolSetting(const Section section, const string &key, const bool defVal, const bool addDefaults, const string &comments = string());
   void RegisterIntSetting(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults, const string &comments = string());
   void RegisterFloatSetting(const Section section, const string &key, const float defVal, const float minVal, const float maxVal, const bool addDefaults, const string &comments = string());

   bool m_modified = false;
   string m_iniPath;
   mINI::INIStructure m_ini;
   const Settings * m_parent;
   vector<OptionDef> m_tableOptions;
   #ifdef DEBUG
      ankerl::unordered_dense::map<Section, ankerl::unordered_dense::set<string>> m_validatedKeys;
   #endif

   // Shared across all settings
   static vector<OptionDef> m_pluginOptions;
   static vector<string> m_settingKeys;
};
