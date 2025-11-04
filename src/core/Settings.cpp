// license:GPLv3+

#include "core/stdafx.h"
#include "Settings.h"

#include <cstdio>
#include <filesystem>


VPX::Properties::PropertyRegistry &Settings::GetRegistry()
{
   static VPX::Properties::PropertyRegistry registry;
   return registry;
}

string Settings::GetBackwardCompatibleSection(const string &groupId)
{
   if (groupId.starts_with("Plugin"s))
   {
      return "Plugin."s + groupId.substr(6);
   }
   else if (groupId.starts_with("DefaultProps"s))
   {
      return "DefaultProps\\"s + groupId.substr(12);
   }
   return groupId;
}

static const string regKey[Settings::Plugin00] = { "Controller"s, "Editor"s, "Standalone"s, "Player"s, "Input"s, "DMD"s, "Alpha"s, 
      "Backglass"s, "ScoreView"s, "Topper"s, "PlayerVR"s, "RecentDir"s, "Version"s, "CVEdit"s, "TableOverride"s, "TableOption"s, "ControllerDevices"s,
      "DefaultProps\\Ball"s, "DefaultProps\\Bumper"s, "DefaultProps\\Decal"s, "DefaultProps\\EMReel"s, "DefaultProps\\Flasher"s, "DefaultProps\\Flipper"s,
      "DefaultProps\\Gate"s, "DefaultProps\\HitTarget"s, "DefaultProps\\Kicker"s, "DefaultProps\\Light"s, "DefaultProps\\LightSequence"s,
      "DefaultProps\\Plunger"s, "DefaultProps\\Primitive"s, "DefaultProps\\Ramp"s, "DefaultProps\\Rubber"s, "DefaultProps\\Spinner"s,
      "DefaultProps\\Wall"s, "DefaultProps\\Target"s, "DefaultProps\\TextBox"s, "DefaultProps\\Timer"s, "DefaultProps\\Trigger"s,
      "Defaults\\Camera"s, "Defaults\\PartGroup"s
   };
vector<string> Settings::m_settingKeys = vector<string>(regKey, regKey + Settings::Section::Plugin00);
vector<Settings::OptionDef> Settings::m_pluginOptions;

Settings::Section Settings::GetSection(const string& name)
{
   int index = FindIndexOf(m_settingKeys, name);
   if (index < 0)
   {
      PLOGI << "Creating new setting section '" << name << "' for custom table/plugin use";
      index = (int)m_settingKeys.size();
      m_settingKeys.push_back(name);
   }
   return (Settings::Section)index;
}

const string &Settings::GetSectionName(const Section section)
{
   assert(0 <= section && section < (int)m_settingKeys.size());
   return m_settingKeys[section];
}

Settings::Settings(Settings* parent)
   : m_parent(parent)
   , m_store(this)
{
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, float v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Float);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, int v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Int || GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Enum);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, bool v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::Bool);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::Set(VPX::Properties::PropertyRegistry::PropId propId, const string &v, bool asTableOverride)
{
   assert(GetRegistry().GetProperty(propId)->m_type == VPX::Properties::PropertyDef::Type::String);
   if (asTableOverride)
   {
      assert(m_parent != nullptr);
      m_store.Set(propId, v);
   }
   else if (m_parent)
   {
      m_store.Reset(propId);
      m_parent->Set(propId, v, false);
   }
   else
   {
      m_store.Set(propId, v);
   }
}

void Settings::RegisterBoolSetting(const Section section, const string &key, const bool defVal, const bool addDefaults, const string &comments)
{
   int val;
   bool present = LoadValue(section, key, val);
   if (present && !((val == 0) || (val == 1)))
   {
      DeleteValue(section, key);
      present = false;
   }
   if (!present && addDefaults)
      SaveValue(section, key, defVal);
}

void Settings::RegisterIntSetting(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults, const string &comments)
{
   assert((minVal <= defVal) && (defVal <= maxVal));
   int val;
   bool present = LoadValue(section, key, val);
   if (present && ((val < minVal) || (val > maxVal)))
   {
      DeleteValue(section, key);
      present = false;
   }
   if (!present && addDefaults)
      SaveValue(section, key, defVal);
}


bool Settings::LoadFromFile(const string& path, const bool createDefault)
{
   m_modified = false;
   m_iniPath = path;

   mINI::INIFile file(path);
   if (file.read(m_ini))
   {
      PLOGI << "Settings file was loaded from '" << path << '\'';
      for (const auto &[key, value] : m_ini)
      {
         if (FindIndexOf(m_settingKeys, key) < 0)
         {
            PLOGI << "Registering section '" << key << "' used in settings file '" << path << '\'';
            m_settingKeys.push_back(key);
         }
      }
      return true;
   }
   else if (createDefault)
   {
      PLOGI << "Settings file was not found at '" << path << "', creating a default one";

      // Load failed: generate a default setting file
      try
      {
         m_store.GenerateTemplate(path);
      }
      catch (const std::exception&)
      {
         ShowError("Failed to generate the default setting file");
      }
      if (!file.read(m_ini))
      {
         PLOGE << "Loading of default settings file failed";
      }

      #ifdef _WIN32
      // For Windows, get settings values from windows registry (which was used to store settings before 10.8)
      for (unsigned int j = 0; j < Section::Plugin00; j++)
      {
         // We do not save version of played tables in the ini file
         if (j == Section::Version)
            continue;

         const string regpath = (j == Section::Controller ? "Software\\Visual Pinball\\" : "Software\\Visual Pinball\\VP10\\") + m_settingKeys[j];

         HKEY hk;
         LSTATUS res = RegOpenKeyEx(HKEY_CURRENT_USER, regpath.c_str(), 0, KEY_READ, &hk);
         if (res != ERROR_SUCCESS)
            continue;

         for (DWORD Index = 0;; ++Index)
         {
            DWORD dwSize = MAX_PATH;
            TCHAR szName[MAX_PATH];
            res = RegEnumValue(hk, Index, szName, &dwSize, nullptr, nullptr, nullptr, nullptr);
            if (res == ERROR_NO_MORE_ITEMS)
               break;
            if (res != ERROR_SUCCESS || dwSize == 0 || szName[0] == '\0')
               continue;

            BYTE pvalue[MAXSTRING];
            dwSize = std::size(pvalue);
            DWORD type = REG_NONE;
            res = RegQueryValueEx(hk, szName, nullptr, &type, pvalue, &dwSize);
            if (res != ERROR_SUCCESS)
            {
               PLOGI << "Settings '" << m_settingKeys[j] << '/' << szName << "' was not imported. Failure cause: failed to get value";
               continue;
            }

            // old Win32xx and Win32xx 9+ docker keys
            if ((char *)pvalue == "Dock Windows"s) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
               continue;
            if ((char *)pvalue == "Dock Settings"s) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
               continue;

            string copy;
            if (type == REG_SZ)
               copy = reinterpret_cast<char*>(pvalue);
            else if (type == REG_DWORD)
               copy = std::to_string(*reinterpret_cast<uint32_t *>(pvalue));
            else
            {
               continue;
               assert(!"Bad Registry Key");
            }

            string name(szName);
            if (!m_ini[m_settingKeys[j]].has(name))
            {
               // Search for a case insensitive match
               for (const auto& item : m_ini[m_settingKeys[j]])
               {
                  if (StrCompareNoCase(name, item.first))
                  {
                     name = item.first;
                     break;
                  }
               }
            }

            if (m_ini[m_settingKeys[j]].has(name))
               m_ini[m_settingKeys[j]][name] = copy;
            else
            {
               PLOGI << "Settings '" << m_settingKeys[j] << '/' << szName << "' was not imported (value in registry: " << copy << "). Failure cause: name not found";
            }
         }
         RegCloseKey(hk);
      }
      #endif
      return true;
   }
   else
   {
      PLOGI << "Settings file was not found at '" << path << '\'';
      return false;
   }
}

void Settings::SaveToFile(const string &path)
{
   m_iniPath = path;
   if (!m_modified || m_iniPath.empty())
      return;

   // Discard overrides that match parent value or the property default's value
   if (m_parent)
   {
      vector<std::pair<string, string>> discardList;
      for (const auto &[section, values] : m_ini)
      {
         const bool parentHasSection = m_parent->m_ini.has(section);
         for (const auto &[key, value] : values)
         {
            if (parentHasSection && m_parent->m_ini[section].has(key) && m_parent->m_ini[section][key] == value)
               discardList.emplace_back(section, key);
            else if (auto prop = Settings::GetRegistry().GetPropertyId(section, key); prop.has_value())
            {
               bool isDefault = false;
               int intVal;
               switch (GetRegistry().GetProperty(prop.value())->m_type)
               {
               case VPX::Properties::PropertyDef::Type::Float: isDefault = GetRegistry().GetFloatProperty(prop.value())->m_def == sz2f(value); break;
               case VPX::Properties::PropertyDef::Type::Int: isDefault = !try_parse_int(value, intVal) || GetRegistry().GetIntProperty(prop.value())->m_def == intVal; break;
               case VPX::Properties::PropertyDef::Type::Enum: isDefault = !try_parse_int(value, intVal) || GetRegistry().GetEnumProperty(prop.value())->m_def == intVal; break;
               case VPX::Properties::PropertyDef::Type::Bool: isDefault = !try_parse_int(value, intVal) || GetRegistry().GetBoolProperty(prop.value())->m_def == (intVal != 0); break;
               case VPX::Properties::PropertyDef::Type::String: isDefault = GetRegistry().GetStringProperty(prop.value())->m_def == value; break;
               }
               if (isDefault)
                  discardList.emplace_back(section, key);
            }
         }
      }
      for (const auto& [section, key] : discardList)
         m_ini[section].remove(key);
   }

   m_modified = false;
   size_t size = 0;
   for (const auto& section : m_ini)
      size += section.second.size();
   if (size > 0)
   {
      mINI::INIFile file(path);
      file.write(m_ini, true);
   }
   else if (FileExists(path))
   {
      // Remove empty settings file
      remove(path.c_str());
   }
}

void Settings::Save()
{
   SaveToFile(m_iniPath);
}

void Settings::Copy(const Settings& settings)
{
   m_ini = settings.m_ini;
   m_iniPath = settings.m_iniPath;
   m_modified = settings.m_modified;
   m_modificationIndex = settings.m_modificationIndex;
   m_store.UpdateParent();
}

bool Settings::HasValue(const Section section, const string& key, const bool searchParent) const
{
   bool hasInIni = m_ini.get(m_settingKeys[section]).has(key);
   if (!hasInIni && m_parent && searchParent)
      hasInIni = m_parent->HasValue(section, key, searchParent);
   return hasInIni;
}

bool Settings::LoadValue(const Section section, const string &key, float &pfloat) const
{
   string val;
   if (!LoadValue(section, key, val))
      return false;
   pfloat = sz2f(val,true);
   return true;
}

bool Settings::LoadValue(const Section section, const string &key, int &pint) const
{
   unsigned int val;
   if (!LoadValue(section, key, val))
      return false;
   pint = val;
   return true;
}

bool Settings::LoadValue(const Section section, const string &key, string &val) const
{
   const string value = m_ini.get(m_settingKeys[section]).get(key);
   if (!value.empty())
   {
      val = value;
      return true;
   }

   if (m_parent != nullptr)
      return m_parent->LoadValue(section, key, val);

   return false;
}

bool Settings::LoadValue(const Section section, const string &key, unsigned int &val) const
{
   const string value = m_ini.get(m_settingKeys[section]).get(key);
   if (!value.empty())
   {
      unsigned int result;
      if (std::from_chars(value.c_str(), value.c_str()+value.length(), result).ec != std::errc{})
         return false;

      val = result;
      return true;
   }

   if (m_parent != nullptr)
      return m_parent->LoadValue(section, key, val);

   return false;
}

int Settings::LoadValueWithDefault(const Section section, const string &key, const int def) const
{
   int val;
   return LoadValue(section, key, val) ? val : def;
}

float Settings::LoadValueWithDefault(const Section section, const string &key, const float def) const
{
   float val;
   return LoadValue(section, key, val) ? val : def;
}

bool Settings::LoadValueWithDefault(const Section section, const string &key, const bool def) const
{
   return LoadValueWithDefault(section, key, (int)def) != 0;
}

string Settings::LoadValueWithDefault(const Section section, const string &key, const string &def) const
{
   string val;
   return LoadValue(section, key, val) ? val : def;
}

bool Settings::SaveValue(const Section section, const string &key, const string &val, const bool overrideMode)
{
   if (key.empty())
      return false;
   m_modified = true;
   m_modificationIndex++;
   if (m_parent && overrideMode)
   {
      // Handle overriding parent's settings
      if (m_ini.get(m_settingKeys[section]).has(key))
         m_ini[m_settingKeys[section]].remove(key);

      // If it has the same value as parent, remove and rely on parent
      string parentValue;
      const bool isDefinedInParent = m_parent->LoadValue(section, key, parentValue);
      if (isDefinedInParent && parentValue == val)
         return true;
   }
   m_ini[m_settingKeys[section]][key] = val;
   return true;
}

bool Settings::SaveValue(const Section section, const string &key, const bool val, const bool overrideMode)
{
   return SaveValue(section, key, val ? "1"s : "0"s, overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const int val, const bool overrideMode)
{
   return SaveValue(section, key, std::to_string(val), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const unsigned int val, const bool overrideMode)
{
   return SaveValue(section, key, std::to_string(val), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const float val, const bool overrideMode)
{
   return SaveValue(section, key, f2sz(val,false), overrideMode);
}

bool Settings::DeleteValue(const Section section, const string &key, const bool deleteFromParent)
{
   bool success = true;
   if (m_parent && deleteFromParent)
      success &= m_parent->DeleteValue(section, key, deleteFromParent);
   if (m_ini.get(m_settingKeys[section]).has(key))
   {
      m_modified = true;
      m_modificationIndex++;
      success &= m_ini[m_settingKeys[section]].remove(key);
   }
   return success;
}

bool Settings::DeleteSubKey(const Section section, const bool deleteFromParent)
{
   bool success = true;
   if (m_parent && deleteFromParent)
      success &= DeleteSubKey(section, deleteFromParent);
   if (m_ini.has(m_settingKeys[section]))
   {
      m_modified = true;
      m_modificationIndex++;
      success &= m_ini.remove(m_settingKeys[section]);
   }
   return success;
}

Settings::OptionDef &Settings::RegisterSetting(const Section section, const string &id, const unsigned int showMask, const string &name, float minValue, float maxValue, float step,
   float defaultValue,
   OptionUnit unit, const vector<string> &literals)
{
   assert(section >= Plugin00); // For the time being, this system is only used for custom table and plugin options (could be extend for all options to get the benefit of validation, fast access, and remove unneeded copied states...)

   vector<OptionDef> &options = m_pluginOptions;
   OptionDef *opt = nullptr;
   bool isNew = false;
   for (auto &option : options)
   {
      if (option.section == section && option.id == id)
      {
         opt = &option;
         break;
      }
   }
   if (opt == nullptr)
   {
      isNew = true;
      options.push_back(OptionDef());
      opt = &options.back();
   }

   opt->section = section;
   opt->id = id;
   opt->showMask = showMask;
   opt->name = name;
   opt->minValue = minValue;
   opt->maxValue = maxValue;
   opt->step = step;
   opt->defaultValue = defaultValue;
   opt->literals = literals;
   opt->tokenizedLiterals.clear();
   for (size_t i = 0; i < literals.size(); ++i) {
      if (i > 0)
         opt->tokenizedLiterals += "||";
      opt->tokenizedLiterals += literals[i];
   }
   opt->unit = unit;

   if (isNew)
      opt->value = LoadValueWithDefault(section, id, defaultValue);

   opt->value = clamp(minValue + step * roundf((opt->value - minValue) / step), minValue, maxValue);

   return *opt;
}
