// license:GPLv3+

#include "core/stdafx.h"
#include "Settings.h"

#include <cstdio>
#include <filesystem>

static const string regKey[Settings::Plugin00] = { "Controller"s, "Editor"s, "Standalone"s, "Player"s, "DMD"s, "Alpha"s, "Backglass"s,
      "PlayerVR"s, "RecentDir"s, "Version"s, "CVEdit"s, "TableOverride"s, "TableOption"s, "ControllerDevices"s,
      "DefaultProps\\Ball"s, "DefaultProps\\Bumper"s, "DefaultProps\\Decal"s, "DefaultProps\\EMReel"s, "DefaultProps\\Flasher"s, "DefaultProps\\Flipper"s,
      "DefaultProps\\Gate"s, "DefaultProps\\HitTarget"s, "DefaultProps\\Kicker"s, "DefaultProps\\Light"s, "DefaultProps\\LightSequence"s,
      "DefaultProps\\Plunger"s, "DefaultProps\\Primitive"s, "DefaultProps\\Ramp"s, "DefaultProps\\Rubber"s, "DefaultProps\\Spinner"s,
      "DefaultProps\\Wall"s, "DefaultProps\\Target"s, "DefaultProps\\TextBox"s, "DefaultProps\\Timer"s, "DefaultProps\\Trigger"s,
      "Defaults\\Camera"s
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

Settings::Settings(const Settings* parent)
   : m_parent(parent)
{
}

void Settings::Validate(const Section section, const string &key, const string& defVal, const bool addDefaults)
{
   #ifdef DEBUG
      m_validatedKeys[section].insert(key);
   #endif
   string val;
   bool present = LoadValue(section, key, val);
   if (!present && addDefaults)
      SaveValue(section, key, defVal);
}

void Settings::Validate(const Section section, const string &key, const bool defVal, const bool addDefaults)
{
   #ifdef DEBUG
      m_validatedKeys[section].insert(key);
   #endif
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

void Settings::Validate(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults)
{
   #ifdef DEBUG
      m_validatedKeys[section].insert(key);
   #endif
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

void Settings::Validate(const Section section, const string &key, const float defVal, const float minVal, const float maxVal, const bool addDefaults)
{
   #ifdef DEBUG
      m_validatedKeys[section].insert(key);
   #endif
   float val;
   bool present = LoadValue(section, key, val);
   if (present && ((val < minVal) || (val > maxVal)))
   {
      DeleteValue(section, key);
      present = false;
   }
   if (!present && addDefaults)
      SaveValue(section, key, defVal);
}

void Settings::Validate(const bool addDefaults)
{
   //////////////////////////////////////////////////////////////////////////
   // DMD section

   /* Validate(Section::DMD, "DMDOutput"s, 0, 0, 100, addDefaults);
   Validate(Section::DMD, "DMDWndX"s, 0, -10000, 10000, addDefaults);
   Validate(Section::DMD, "DMDWndY"s, 0, -10000, 10000, addDefaults);
   Validate(Section::DMD, "DMDWidth"s, 512, 1, 16384, addDefaults);
   Validate(Section::DMD, "DMDHeight"s, 128, 1, 16384, addDefaults);*/

   Validate(Section::DMD, "DefaultProfile"s, 1, 0, 7, addDefaults);
   Validate(Section::DMD, "DefaultTint"s, 0x00FFFFFF, 0x00000000, 0x00FFFFFF, addDefaults);
   for (int i = 1; i <= 7; i++)
   {
      const string prefix = "Profile." + std::to_string(i) + '.';
      int dotColor;
      switch (i)
      {
      case 1: dotColor = 0x002D52FF; break; // Legacy
      case 2: dotColor = 0x002D52FF; break; // Classic Neon plasma DMD
      case 3: dotColor = 0x001523FF; break; // Red Led DMD (used after RoHS regulation entry into force)
      case 4: dotColor = 0x0023FF15; break; // Green Led
      case 5: dotColor = 0x00FF2315; break; // Blue Led
      case 6: dotColor = 0x00FFFFFF; break; // Generic Plasma
      case 7: dotColor = 0x00FFFFFF; break; // Generic Led
      default: assert(false);
      }
      Validate(Section::DMD, prefix + "Legacy",        i == 1, addDefaults);
      Validate(Section::DMD, prefix + "ScaleFX",       false, addDefaults);
      Validate(Section::DMD, prefix + "DotTint",       dotColor,   0x00000000, 0x00FFFFFF, addDefaults);
      Validate(Section::DMD, prefix + "UnlitDotColor", 0x00202020, 0x00000000, 0x00FFFFFF, addDefaults);
      Validate(Section::DMD, prefix + "DotBrightness", 5.0f,   0.001f, 10.f, addDefaults);
      Validate(Section::DMD, prefix + "DotSize",       0.85f,  0.001f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "DotSharpness",  0.80f,  0.f,    1.f, addDefaults);
      Validate(Section::DMD, prefix + "DotRounding",   0.85f,  0.f,    1.f, addDefaults);
      Validate(Section::DMD, prefix + "DotGlow",       0.015f, 0.f,    1.f, addDefaults);
      Validate(Section::DMD, prefix + "BackGlow",      0.005f, 0.f,    1.f, addDefaults);

      Validate(Section::DMD, prefix + "GlassImage", ""s, addDefaults);
      Validate(Section::DMD, prefix + "GlassAmbientLight", 0x00010101, 0x00000000, 0x00FFFFFF, addDefaults);
      Validate(Section::DMD, prefix + "GlassDotLight",  0.4f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "GlassPadLeft",   0.f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "GlassPadRight",  0.f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "GlassPadBottom", 0.f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "GlassPadTop",    0.f, 0.f, 1.f, addDefaults);

      Validate(Section::DMD, prefix + "FrameImage", ""s, addDefaults);
      Validate(Section::DMD, prefix + "FramePadLeft",   0.f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "FramePadRight",  0.f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "FramePadBottom", 0.f, 0.f, 1.f, addDefaults);
      Validate(Section::DMD, prefix + "FramePadTop",    0.f, 0.f, 1.f, addDefaults);
   }
}

bool Settings::LoadFromFile(const string& path, const bool createDefault)
{
   m_modified = false;
   m_iniPath = path;
   mINI::INIFile file(path);
   if (file.read(m_ini))
   {
      PLOGI << "Settings file was loaded from '" << path << '\'';
      Validate(createDefault);
      for (const auto& entry : m_ini)
      {
         if (FindIndexOf(m_settingKeys, entry.first) < 0)
         {
            PLOGI << "Registering section '" << entry.first << "' used in settings file '" << path << "'";
            m_settingKeys.push_back(entry.first);
         }
      }
      return true;
   }
   else if (createDefault)
   {
      PLOGI << "Settings file was not found at '" << path << "', creating a default one";

      // Load failed: initialize from the default setting file
      try
      {
         std::filesystem::copy(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "Default_VPinballX.ini", path);
      }
      catch (const std::exception&)
      {
         ShowError("Copying of default settings file 'Default_VPinballX.ini' from the 'assets' folder failed");
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

         string regpath(j == Section::Controller ? "Software\\Visual Pinball\\"s : "Software\\Visual Pinball\\VP10\\"s);
         regpath += m_settingKeys[j];

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

            dwSize = MAXSTRING;
            BYTE pvalue[MAXSTRING];
            DWORD type = REG_NONE;
            res = RegQueryValueEx(hk, szName, nullptr, &type, pvalue, &dwSize);
            if (res != ERROR_SUCCESS)
            {
               PLOGI << "Settings '" << m_settingKeys[j] << '/' << szName << "' was not imported. Failure cause: failed to get value";
               continue;
            }

            // old Win32xx and Win32xx 9+ docker keys
            if (strcmp((char *)pvalue, "Dock Windows") == 0) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
               continue;
            if (strcmp((char *)pvalue, "Dock Settings") == 0) // should not happen, as a folder, not value.. BUT also should save these somehow and restore for Win32++, or not ?
               continue;

            char *copy;
            if (type == REG_SZ)
            {
               const size_t size = strlen((char *)pvalue);
               copy = new char[size + 1];
               memcpy(copy, pvalue, size);
               copy[size] = '\0';
            }
            else if (type == REG_DWORD)
            {
               const string tmp = std::to_string(*(DWORD *)pvalue);
               const size_t len = tmp.length() + 1;
               copy = new char[len];
               strcpy_s(copy, len, tmp.c_str());
            }
            else
            {
               copy = nullptr;
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
      Validate(true); // Guarantee a valid setting setup with defaults defined
      return true;
   }
   else
   {
      PLOGI << "Settings file was not found at '" << path << '\'';
      Validate(false); // Still guarantee a valid setting setup
      return false;
   }
}

void Settings::SaveToFile(const string &path)
{
   m_iniPath = path;
   if (!m_modified || m_iniPath.empty())
      return;
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

void Settings::CopyOverrides(const Settings& settings)
{
   assert(m_parent != nullptr); // Overrides are defined relatively to a parent
   for (const auto& section : settings.m_ini)
   {
      for (const auto& item : section.second)
      {
         if (m_parent->m_ini.get(section.first).has(item.first))
         { // Value stored in parent setting block
            if (m_parent->m_ini.get(section.first).get(item.first) != item.second)
            {
               m_ini[section.first][item.first] = item.second;
               m_modified = true;
            }
            else
            {
               m_modified |= m_ini[section.first].remove(item.first);
            }
         }
         else
         { // Value missing from parent (so an override)
            m_modified |= (m_ini[section.first][item.first] != item.second);
            m_ini[section.first][item.first] = item.second;
         }
      }
   }
}

bool Settings::HasValue(const Section section, const string& key, const bool searchParent) const
{
   bool hasInIni = m_ini.get(m_settingKeys[section]).has(key);
   if (!hasInIni && m_parent && searchParent)
      hasInIni = m_parent->HasValue(section, key, searchParent);
   return hasInIni;
}

bool Settings::LoadValue(const Section section, const string &key, string &buffer) const
{
   DataType type = DT_SZ;
   char szbuffer[MAXSTRING];
   szbuffer[0] = '\0';
   const bool success = LoadValue(section, key, type, szbuffer, MAXSTRING);
   if (success)
      buffer = szbuffer;
   return success && (type == DT_SZ);
}

bool Settings::LoadValue(const Section section, const string &key, void *const szbuffer, const DWORD size) const
{
   if (size > 0) // clear string in case of reg value being set, but being null string which results in szbuffer being kept as-is
      ((char *)szbuffer)[0] = '\0';
   DataType type = DT_SZ;
   const bool success = LoadValue(section, key, type, szbuffer, size);
   return success && (type == DT_SZ);
}

bool Settings::LoadValue(const Section section, const string &key, float &pfloat) const
{
   DataType type = DT_SZ;
   char szbuffer[16];
   const bool success = LoadValue(section, key, type, szbuffer, sizeof(szbuffer));
   if (!success || (type != DT_SZ))
      return false;
   const int len = lstrlen(szbuffer);
   if (len == 0)
      return false;
   char *const fo = strchr(szbuffer, ',');
   if (fo != nullptr)
      *fo = '.';
   if (szbuffer[0] == '-')
   {
      if (len < 2)
         return false;
      pfloat = (float)atof(&szbuffer[1]);
      pfloat = -pfloat;
   }
   else
      pfloat = (float)atof(szbuffer);
   return true;
}

bool Settings::LoadValue(const Section section, const string &key, int &pint) const
{
   DataType type = DT_DWORD;
   const bool success = LoadValue(section, key, type, (void *)&pint, 4);
   return success && (type == DT_DWORD);
}

bool Settings::LoadValue(const Section section, const string &key, unsigned int &pint) const
{
   DataType type = DT_DWORD;
   const bool success = LoadValue(section, key, type, (void *)&pint, 4);
   return success && (type == DT_DWORD);
}

bool Settings::LoadValue(const Section section, const string &key, DataType &type, void *pvalue, const DWORD size) const
{
   if (size == 0)
   {
      type = DT_ERROR;
      return false;
   }

      const string value = m_ini.get(m_settingKeys[section]).get(key);
      if (!value.empty())
      {
         // Value is empty (just a marker for text formatting), consider it as undefined
         if (type == DT_SZ)
         {
            const DWORD len = (DWORD)value.length() + 1;
            const DWORD len_min = min(len, size) - 1;
            memcpy(pvalue, value.c_str(), len_min);
            ((char *)pvalue)[len_min] = '\0';
            return true;
         }
         else if (type == DT_DWORD)
         {
            *((DWORD *)pvalue) = (DWORD)atoll(value.c_str());
            return true;
         }
         else
         {
            assert(!"Bad Type");
            type = DT_ERROR;
            return false;
         }
      }

   if (m_parent != nullptr)
      return m_parent->LoadValue(section, key, type, pvalue, size);

   type = DT_ERROR;
   return false;
}

int Settings::LoadValueWithDefault(const Section section, const string &key, const int def) const
{
   #ifdef DEBUG
      assert(!m_validatedKeys.contains(section) || !m_validatedKeys.at(section).contains(key)); // Redefinition
   #endif
   int val;
   return LoadValue(section, key, val) ? val : def;
}

float Settings::LoadValueWithDefault(const Section section, const string &key, const float def) const
{
   #ifdef DEBUG
      assert(!m_validatedKeys.contains(section) || !m_validatedKeys.at(section).contains(key)); // Redefinition
   #endif
   float val;
   return LoadValue(section, key, val) ? val : def;
}

bool Settings::LoadValueWithDefault(const Section section, const string &key, const bool def) const
{
   return LoadValueWithDefault(section, key, (int)def) != 0;
}

string Settings::LoadValueWithDefault(const Section section, const string &key, const string &def) const
{
   #ifdef DEBUG
      assert(!m_validatedKeys.contains(section) || !m_validatedKeys.at(section).contains(key)); // Redefinition
   #endif
   string val;
   return LoadValue(section, key, val) ? val : def;
}

bool Settings::SaveValue(const Section section, const string &key, const DataType type, const void *pvalue, const DWORD size, const bool overrideMode)
{
   assert(type == DT_SZ || type == DT_DWORD);
   if (key.empty() || (type != DT_SZ && type != DT_DWORD))
      return false;
   const string copy = type == DT_SZ ? std::string((char*)pvalue) : std::to_string(*(DWORD *)pvalue);
   if (m_parent && overrideMode)
   {
      string value;
      if (m_parent->LoadValue(section, key, value) && value == copy)
      {
         // This is an override and it has the same value as parent: remove it and rely on parent
         if (m_ini.get(m_settingKeys[section]).has(key))
         {
            m_modified = true;
            m_ini[m_settingKeys[section]].remove(key);
         }
         return true;
      }
   }
   m_modified = true;
   m_ini[m_settingKeys[section]][key] = copy;
   return true;
}

bool Settings::SaveValue(const Section section, const string &key, const bool val, const bool overrideMode)
{
   const DWORD dwval = val ? 1 : 0;
   return SaveValue(section, key, DT_DWORD, &dwval, sizeof(DWORD), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const int val, const bool overrideMode)
{
   return SaveValue(section, key, DT_DWORD, &val, sizeof(DWORD), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const unsigned int val, const bool overrideMode)
{
   return SaveValue(section, key, DT_DWORD, &val, sizeof(DWORD), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const float val, const bool overrideMode)
{
   char buf[16];
   sprintf_s(buf, sizeof(buf), "%f", val);
   return SaveValue(section, key, DT_SZ, buf, lstrlen(buf), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const char *val, const bool overrideMode)
{
   return SaveValue(section, key, DT_SZ, val, lstrlen(val), overrideMode);
}

bool Settings::SaveValue(const Section section, const string &key, const string &val, const bool overrideMode)
{
   return SaveValue(section, key, DT_SZ, val.c_str(), (DWORD)val.length(), overrideMode);
}

bool Settings::DeleteValue(const Section section, const string &key, const bool deleteFromParent)
{
   bool success = true;
   if (m_parent && deleteFromParent)
      success &= DeleteValue(section, key, deleteFromParent);
   if (m_ini.get(m_settingKeys[section]).has(key))
   {
      m_modified = true;
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
      success &= m_ini.remove(m_settingKeys[section]);
   }
   return success;
}

void Settings::RegisterSetting(const Section section, const string& id, const unsigned int showMask, const string &name, float minValue, float maxValue, float step, float defaultValue, OptionUnit unit, const vector<string> &literals)
{
   assert(section == TableOption || section >= Plugin00); // For the time being, this system is only used for custom table and plugin options (could be extend for all options to get the benefit of validation, fast access, and remove unneeded copied states...)
   OptionDef opt;
   opt.section = section;
   opt.id = id;
   opt.showMask = showMask;
   opt.name = name;
   opt.minValue = minValue;
   opt.maxValue = maxValue;
   opt.step = step;
   opt.defaultValue = defaultValue;
   opt.literals = literals;
   opt.tokenizedLiterals.clear();
   for (size_t i = 0; i < literals.size(); ++i) {
      if (i > 0)
          opt.tokenizedLiterals += "||";
      opt.tokenizedLiterals += literals[i];
   }
   opt.unit = unit;
   vector<OptionDef> &options = section == TableOption ? m_tableOptions : m_pluginOptions;
   for (auto& option : options)
   {
      if (option.section == section && option.id == id)
      {
         option = opt;
         return;
      }
   }
   options.push_back(opt);
}
