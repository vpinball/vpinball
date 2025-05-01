// license:GPLv3+

#include "core/stdafx.h"
#include "Settings.h"

#include <cstdio>
#include <filesystem>

static const string regKey[Settings::Plugin00] = { "Controller"s, "Editor"s, "Standalone"s, "Player"s, "DMD"s, "Alpha"s, 
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

Settings::Settings(const Settings* parent)
   : m_parent(parent)
{
}

void Settings::RegisterStringSetting(const Section section, const string &key, const string &defVal, const bool addDefaults, const string &comments)
{
   #ifdef DEBUG
      m_validatedKeys[section].insert(key);
   #endif
   string val;
   bool present = LoadValue(section, key, val);
   if (!present && addDefaults)
      SaveValue(section, key, defVal);
}

void Settings::RegisterBoolSetting(const Section section, const string &key, const bool defVal, const bool addDefaults, const string &comments)
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

void Settings::RegisterIntSetting(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults, const string &comments)
{
   assert((minVal <= defVal) && (defVal <= maxVal));
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

void Settings::RegisterFloatSetting(const Section section, const string &key, const float defVal, const float minVal, const float maxVal, const bool addDefaults, const string &comments)
{
   assert((minVal <= defVal) && (defVal <= maxVal));
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


// This methods declares the static settings supported by VPX.
//
// All static (known at compile time) settings should be declared here with their corresponding properties, allowing 
// to improve setting management by:
// - adding the ability to validate all settings against a validity range, and therefore guarantee a valid value for any setting
// - adding the ability to generate a friendly up-to-date default setting file with usage comments
// - adding static, explicit typîng to avoid unexpected data conversions
// - remove duplicated default values spread all over the codebase which happens to not always be equals
// - allow to move to a simple array based access, improving performance and lowering the need to cache settings into local unsinced fields
//
// To allow a slow and smooth transition, settings are declared here little by little. Once registered here, settings may not be accessed
// using LoadWithDefaultValue (it is asserted in Debug builds).
//
// We use macro definition instead of directly calling Validate function in order to be able to duplicate this definition list into 
// a static enum definition in Settings.h which will be used for array access.
void Settings::Validate(const bool addDefaults)
{
   #define SettingString(section, name, defVal, comment) RegisterStringSetting(section, name, defVal, addDefaults, comment);
   #define SettingBool(section, name, defVal, comment) RegisterBoolSetting(section, name, defVal, addDefaults, comment);
   #define SettingFloat(section, name, defVal, minVal, maxVal, comment) RegisterFloatSetting(section, name, defVal, minVal, maxVal, addDefaults, comment);
   #define SettingInt(section, name, defVal, minVal, maxVal, comment) RegisterIntSetting(section, name, defVal, minVal, maxVal, addDefaults, comment);
   
   //////////////////////////////////////////////////////////////////////////
   // Input section

   SettingInt(Section::Player, "Exitconfirm"s, 120, 0, 30*60, "Length of a long ESC press that directly closes the app, (sadly) expressed in seconds * 60."s);
   SettingBool(Section::Player, "PBWDefaultLayout"s, false, "Disable default layout of recognized Pincab controllers (DirectInput only)."s);
   SettingBool(Section::Player, "DisableESC"s, false, "Disable ESC key as exit action button."s);

   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eLeftFlipperKey], DIK_LSHIFT, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eRightFlipperKey], DIK_RSHIFT, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eStagedLeftFlipperKey], DIK_LWIN, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eStagedRightFlipperKey], DIK_RALT, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eLeftTiltKey], DIK_Z, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eRightTiltKey], DIK_SLASH, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eCenterTiltKey], DIK_SPACE, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::ePlungerKey], DIK_RETURN, 0x00, 0xFFFF, ""s);
   #if !defined(__APPLE__) && !defined(__ANDROID__)
      SettingInt(Section::Player, regkey_string[EnumAssignKeys::eFrameCount], DIK_F11, 0x00, 0xFFFF, ""s);
   #else
      SettingInt(Section::Player, regkey_string[EnumAssignKeys::eFrameCount], DIK_F1, 0x00, 0xFFFF, ""s);
   #endif
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eDBGBalls], DIK_O, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eDebugger], DIK_D, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eAddCreditKey], DIK_5, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eAddCreditKey2], DIK_4, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eStartGameKey], DIK_1, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eMechanicalTilt], DIK_T, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eRightMagnaSave], DIK_RCONTROL, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eLeftMagnaSave], DIK_LCONTROL, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eExitGame], DIK_Q, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eVolumeUp], DIK_EQUALS, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eVolumeDown], DIK_MINUS, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eLockbarKey], DIK_LALT, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eEnable3D], DIK_F10, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eTableRecenter], DIK_NUMPAD5, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eTableUp], DIK_NUMPAD8, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eTableDown], DIK_NUMPAD2, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eEscape], DIK_ESCAPE, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::ePause], DIK_P, 0x00, 0xFFFF, ""s);
   SettingInt(Section::Player, regkey_string[EnumAssignKeys::eTweak], DIK_F12, 0x00, 0xFFFF, ""s);

#ifdef ENABLE_SDL_INPUT
   SettingInt(Settings::Player, "JoyLFlipKey"s, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyRFlipKey"s, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyStagedLFlipKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyStagedRFlipKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyLTiltKey"s, SDL_GAMEPAD_BUTTON_DPAD_LEFT + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyRTiltKey"s, SDL_GAMEPAD_BUTTON_DPAD_RIGHT + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyCTiltKey"s, SDL_GAMEPAD_BUTTON_DPAD_UP + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyPlungerKey"s, SDL_GAMEPAD_BUTTON_DPAD_DOWN + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyFrameCount"s, SDL_GAMEPAD_BUTTON_WEST + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyDebugKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyDebuggerKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyAddCreditKey"s, SDL_GAMEPAD_BUTTON_SOUTH + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyAddCredit2Key"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyStartGameKey"s, SDL_GAMEPAD_BUTTON_EAST + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyMechTiltKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyRMagnaSave"s, SDL_GAMEPAD_BUTTON_RIGHT_STICK + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyLMagnaSave"s, SDL_GAMEPAD_BUTTON_LEFT_STICK + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyExitGameKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyVolumeUp"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyVolumeDown"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyLockbarKey"s, SDL_GAMEPAD_BUTTON_GUIDE + 1, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTableRecenterKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTableUpKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTableDownKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyPauseKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTweakKey"s, 0, 0x00, 0xFFFF, ""s);
#else
   SettingInt(Settings::Player, "JoyLFlipKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyRFlipKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyStagedLFlipKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyStagedRFlipKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyLTiltKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyRTiltKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyCTiltKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyPlungerKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyFrameCount"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyDebugKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyDebuggerKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyAddCreditKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyAddCredit2Key"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyStartGameKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyMechTiltKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyRMagnaSave"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyLMagnaSave"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyExitGameKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyVolumeUp"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyVolumeDown"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyLockbarKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTableRecenterKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTableUpKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTableDownKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyPauseKey"s, 0, 0x00, 0xFFFF, ""s);
   SettingInt(Settings::Player, "JoyTweakKey"s, 0, 0x00, 0xFFFF, ""s);
#endif


   //////////////////////////////////////////////////////////////////////////
   // Rendering section

   SettingFloat(Section::Player, "EmissionScale"s, 0.5f, 0.f, 1.f, ""s);
   SettingInt(Section::Player, "MaxTexDimension"s, 0, 0, 16384, "Maximum texture dimension. Images sized above this limit will be automatically scaled down on load."s);


   //////////////////////////////////////////////////////////////////////////
   // VR Player section

   SettingFloat(Settings::PlayerVR, "TableX"s, 0.f, -300.f, 300.f, "VR scene horizontal X offset (cm)."s);
   SettingFloat(Settings::PlayerVR, "TableY"s, 0.f, -300.f, 300.f, "VR scene horizontal Y offset (cm)."s);
   SettingFloat(Settings::PlayerVR, "TableZ"s, 0.f, -300.f, 300.f, "VR scene vertical offset (cm)s"s);


   //////////////////////////////////////////////////////////////////////////
   // Cabinet section

   SettingFloat(Section::Player, "ScreenPlayerX"s, 0.f, -100.f, 100.f, ""s);
   SettingFloat(Section::Player, "ScreenPlayerY"s, 0.f, -100.f, 100.f, ""s);
   SettingFloat(Section::Player, "ScreenPlayerZ"s, 70.f, -100.f, 200.f, ""s);
   SettingFloat(Section::Player, "ScreenWidth"s, 95.89f, 1.f, 500.f, "Physical width (cm) of the display area of the playfield (main) screen."s);
   SettingFloat(Section::Player, "ScreenHeight"s, 53.94f, 1.f, 500.f, "Physical height (cm) of the display area of the playfield (main) screen."s);
   SettingFloat(Section::Player, "ScreenInclination"s, 0.0f, -15.f, 15.f, "Inclination (degree) of the playfield (main) screen. 0 is horizontal."s);
   SettingFloat(Section::Player, "LockbarWidth"s, 70.f, 10.f, 150.f, "Lockbar width in centimeters (measured on the cabinet)."s);
   SettingFloat(Section::Player, "LockbarHeight"s, 85.f, 0.f, 250.f, "Lockbar height in centimeters (measured on the cabinet, from ground to top of lockbar)."s);


   //////////////////////////////////////////////////////////////////////////
   // ScoreView section


   //////////////////////////////////////////////////////////////////////////
   // Playfield view section

   SettingFloat(Section::Player, "MaxFramerate"s, -1.f, -1.f, 1000.f, "Maximum FPS of playfield view (minimum: 24FPS), 0 is unlimited, < 0 is limited to the display refresh rate."s);
   SettingInt(Section::Player, "SyncMode"s, VSM_NONE, VSM_NONE, VSM_FRAME_PACING, "Hardware video sync mode to use: None / Vertical Sync / Adaptative Sync / Frame Pacing."s);


   //////////////////////////////////////////////////////////////////////////
   // DMD section

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
      case 5: dotColor = 0x0023FFFF; break; // Yellow Led
      case 6: dotColor = 0x00FFFFFF; break; // Generic Plasma
      case 7: dotColor = 0x00FFFFFF; break; // Generic Led
      default: assert(false);
      }
      SettingBool(Section::DMD, prefix + "Legacy",       i == 1, ""s);
      SettingBool(Section::DMD, prefix + "ScaleFX",      false, ""s);
      SettingInt(Section::DMD, prefix + "DotTint",       dotColor,   0x00000000, 0x00FFFFFF, ""s);
      SettingInt(Section::DMD, prefix + "UnlitDotColor", 0x00404040, 0x00000000, 0x00FFFFFF, ""s);
      SettingFloat(Section::DMD, prefix + "DotBrightness", 25.0f,  0.001f, 100.f, ""s);
      SettingFloat(Section::DMD, prefix + "DotSize",       (i == 2 || i == 6) ? 0.6f : 0.75f,  0.001f, 1.f, ""s); // WPC & GTS3 Neon Plasma: 0.6, Stern Red LED: 0.75
      SettingFloat(Section::DMD, prefix + "DotSharpness",  0.90f, 0.f, 1.f, ""s);
      SettingFloat(Section::DMD, prefix + "DiffuseGlow",   0.10f, 0.f, 10.f, ""s);
   }


   //////////////////////////////////////////////////////////////////////////
   // Alpha segment section

   for (int i = 1; i <= 8; i++)
   {
      const string prefix = "Profile." + std::to_string(i) + '.';
      int color;
      switch (i)
      {
      //case 1: color = 0x002D52FF; break; // Neon plasma
      case 1: color = 0x001E96FF; break; // Neon plasma
      //case 2: color = 0x00FFEFBF; break; // VFD Blueish
      case 2: color = 0x00FFEF3F; break; // VFD Blueish
      case 3: color = 0x00ECFF48; break; // VFD Greenish
      case 4: color = 0x001523FF; break; // Red Led
      case 5: color = 0x0023FF15; break; // Green Led
      case 6: color = 0x0023FFFF; break; // Yellow Led
      case 7: color = 0x00FFFFFF; break; // Generic Plasma
      case 8: color = 0x00FFFFFF; break; // Generic Led
      default: assert(false);
      }
      SettingInt(Section::Alpha, prefix + "Color", color, 0x00000000, 0x00FFFFFF, ""s);
      SettingInt(Section::Alpha, prefix + "Unlit", 0x00404040, 0x00000000, 0x00FFFFFF, ""s);
      SettingFloat(Section::Alpha, prefix + "Brightness", 5.0f, 0.001f, 100.f, ""s);
      SettingFloat(Section::Alpha, prefix + "DiffuseGlow", 1.0f, 0.f, 10.f, ""s);
   }


   //////////////////////////////////////////////////////////////////////////
   // Sound section

   SettingInt(Settings::Player, "Sound3D"s, SoundConfigTypes::SNDCFG_SND3D2CH, SoundConfigTypes::SNDCFG_SND3D2CH, SoundConfigTypes::SNDCFG_SND3DSSF, ""s);


   //////////////////////////////////////////////////////////////////////////
   // Player misc. section
   SettingBool(Settings::Player, "EnableCameraModeFlyAround"s, false, "Enable moving camera when using Tweak menu (legacy, replaced by LiveUI fly mode)");


   //////////////////////////////////////////////////////////////////////////
   // Editor section

   SettingBool(Settings::Editor, "EnableLog"s, true, "Enable general logging to the vinball.log file."s);

   #undef SettingString
   #undef SettingBool
   #undef SettingFloat
   #undef SettingInt
}

void Settings::ResetValue(const Section section, const string& key)
{
   // FIXME implement
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
            PLOGI << "Registering section '" << entry.first << "' used in settings file '" << path << '\'';
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

#if 0
bool Settings::LoadValue(const Section section, const string &key, void *const szbuffer, const size_t size) const
{
   if (size == 0)
      return false;

   string val;
   const bool success = LoadValue(section, key, val);
   if (success)
      strncpy_s((char *)szbuffer, size, val.c_str(), size - 1);
   else
      ((char *)szbuffer)[0] = '\0';

   return success;
}
#endif

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
      const char *const szp = value.c_str();
      char *sze;
      const unsigned int tmp = (unsigned int)std::strtoll(szp, &sze, 10);

      if (szp == sze)
         return false;

      val = tmp;
      return true;
   }

   if (m_parent != nullptr)
      return m_parent->LoadValue(section, key, val);

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

bool Settings::SaveValue(const Section section, const string &key, const string &val, const bool overrideMode)
{
   if (key.empty())
      return false;
   if (m_parent && overrideMode)
   {
      string value;
      if (m_parent->LoadValue(section, key, value) && value == val)
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

#if 0
bool Settings::SaveValue(const Section section, const string &key, const char *val, const bool overrideMode)
{
   return SaveValue(section, key, string(val), overrideMode);
}
#endif

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
