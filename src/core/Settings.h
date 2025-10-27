// license:GPLv3+

#pragma once

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "unordered_dense.h"

#include "PropertyRegistry.h"
#include "LayeredINIPropertyStore.h"

// This class holds the settings registry.
// A setting registry can have a parent, in which case, missing settings will be looked for in the parent.
// This is used to allow overriding part of the settings for a specific table while still using the base application value for others.
// When saving value, an 'override mode' can be used where the value will be saved only if it not the same as the one in the parent.
class Settings final
{
   // New implementation based on property definition/registry/store
   // As changing this API will impact a large part of the codebase, it is planned to be be performed in 3 steps:
   // 1. Create property API [done]
   // 2. Populate shared property registry, and use it where applicable using PropertyDef/Registry [in progress]
   // 3. Persist settings state using PropertyStore [to be done]
   // Note that Get/Set/Reset will only be functionnal when step 3 is done
private:
   VPX::Properties::LayeredINIPropertyStore m_store;

public:
   static VPX::Properties::PropertyRegistry &GetRegistry();

#define PropBool(groupId, propId, label, comment, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>(#groupId, #propId, label, comment, defVal)); \
   static inline const VPX::Properties::BoolPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetBoolProperty(m_prop##groupId##_##propId); } \
   inline bool Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(bool v) { m_store.Set(m_prop##groupId##_##propId, v); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropInt(groupId, propId, label, comment, minVal, maxVal, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(#groupId, #propId, label, comment, minVal, maxVal, defVal)); \
   static inline const VPX::Properties::IntPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetIntProperty(m_prop##groupId##_##propId); } \
   inline int Get##groupId##_##propId() const { return m_store.GetInt(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(int v) { m_store.Set(m_prop##groupId##_##propId, v); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropEnum(groupId, propId, label, comment, type, defVal, ...) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(#groupId, #propId, label, comment, 0, defVal, vector<string> { __VA_ARGS__ })); \
   static inline const VPX::Properties::EnumPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetEnumProperty(m_prop##groupId##_##propId); } \
   inline type Get##groupId##_##propId() const { return (type)(m_store.GetInt(m_prop##groupId##_##propId)); } \
   inline void Set##groupId##_##propId(type v) { m_store.Set(m_prop##groupId##_##propId, (int)v); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

#define PropFloat(groupId, propId, label, comment, minVal, maxVal, step, defVal) \
   static inline const VPX::Properties::PropertyRegistry::PropId m_prop##groupId##_##propId \
      = GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(#groupId, #propId, label, comment, minVal, maxVal, step, defVal)); \
   static inline const VPX::Properties::FloatPropertyDef* Get##groupId##_##propId##_Property() { return GetRegistry().GetFloatProperty(m_prop##groupId##_##propId); } \
   inline float Get##groupId##_##propId() const { return m_store.GetFloat(m_prop##groupId##_##propId); } \
   inline void Set##groupId##_##propId(float v) { m_store.Set(m_prop##groupId##_##propId, v); } \
   inline void Reset##groupId##_##propId() { m_store.Reset(m_prop##groupId##_##propId); }

   // Audio settings
   PropBool(Player, PlayMusic, "Enable Backglass"s, "Enable/Disable backglass game sound & music"s, true);
   PropBool(Player, PlaySound, "Enable Playfield"s, "Enable/Disable playfield mechanical sounds"s, true);
   PropInt(Player, BackglassVolume, "Backglass Volume"s, "Main volume for music and sound played in the backglass speakers"s, 0, 100, 100);
   PropInt(Player, PlayfieldVolume, "Playfield Volume"s, "Main volume for mechanical sounds coming from the playfield"s, 0, 100, 100);

   // Graphics settings
   PropEnum(Player, ShowFPS, "Show FPS"s, "Performance overlay display mode"s, int, 0, "Disable"s, "FPS"s, "Full"s);
   PropEnum(Player, FXAA, "Post processed antialiasing"s, "Select between different antialiasing techniques that offer different quality vs performance balances"s, int, 1, "Disabled"s,
      "Fast FXAA"s, "Standard FXAA"s, "Quality FXAA"s, "Fast NFAA"s, "Standard DLAA"s, "Quality SMAA"s);
   PropEnum(Player, Sharpen, "Post processed sharpening"s, "Select between different sharpening techniques that offer different quality vs performance balances"s, int, 0, "Disabled"s,
      "CAS"s, "Bilateral CAS"s);
   PropBool(Player, HDRDisableToneMapper, "Disable tonemapping on HDR display"s, "Do not perform tonemapping when rendering on a HDR display"s, true);
   PropBool(Player, ForceBloomOff, "Disable Bloom"s, "Disable postprocessed bloom filter"s, false);
   PropBool(Player, ForceMotionBlurOff, "Disable Motion Blur"s, "Disable postprocessed ball motion blur"s, false);

   // Nudge & Plumb settings
   PropFloat(Player, NudgeOrientation0, "Sensor #0 - Orientation", "Define sensor orientation"s, 0.f, 360.f, 1.0f, 0.f);
   PropFloat(Player, NudgeOrientation1, "Sensor #1 - Orientation", "Define sensor orientation"s, 0.f, 360.f, 1.0f, 0.f);
   PropBool(Player, NudgeFilter0, "Sensor #0 - Use Filter", "Enable/Disable filtering acquired value to prevent noise"s, false);
   PropBool(Player, NudgeFilter1, "Sensor #1 - Use Filter", "Enable/Disable filtering acquired value to prevent noise"s, false);
   PropBool(Player, SimulatedPlumb, "Plumb simulation"s, "Enable/Disable mechanical Tilt plumb simulation"s, false);
   PropFloat(Player, PlumbInertia, "Plumb Inertia", ""s, 0.1f, 100.f, 0.01f, 35.f);
   PropFloat(Player, PlumbThresholdAngle, "Plumb Threshold", "Define threshold angle at which a Tilt is caused"s, 5.0f, 60.f, 0.1f, 35.f);
   PropBool(Player, EnableLegacyNudge, "Legacy Keyboard nudge"s, "Enable/Disable legacy keyboard nudge mode"s, false);
   PropFloat(Player, LegacyNudgeStrength, "Legacy Nudge Strength"s, "Strength of nudge when using the legacy keyboard nudge mode"s, 0.f, 90.f, 0.1f, 1.f);

   // Plunger settings
   PropBool(Player, PlungerRetract, "One Second Retract"s, "Enable retracting the plunger after a 1 second press when using the digital plunger emulation through keyboard or joystick button"s, false);
   PropBool(Player, PlungerLinearSensor, "Linear Sensor"s, "Select between symmetric (linear) and assymetric sensor"s, false);

   // VR settings
   PropFloat(PlayerVR, Orientation, "View orientation"s, ""s, -180.f, 180.f, 0.1f, 0.f);
   PropFloat(PlayerVR, TableX, "View Offset X"s, ""s, -100.f, 100.f, 0.1f, 0.f);
   PropFloat(PlayerVR, TableY, "View Offset Y"s, ""s, -100.f, 100.f, 0.1f, 0.f);
   PropFloat(PlayerVR, TableZ, "View Offset Z"s, ""s, -100.f, 100.f, 0.1f, 0.f);
   PropBool(PlayerVR, UsePassthroughColor, "Color Keyed Passthrough"s, ""s, false);
   PropEnum(Player, VRPreview, "Preview mode"s, "Select preview mode"s, int, 1, "Disabled"s, "Left Eye"s, "Right Eye"s, "Both Eyes"s);
   PropBool(PlayerVR, ShrinkPreview, "Shrink preview"s, ""s, false);

   // Real world cabinet & player settings
   PropFloat(Player, ScreenWidth, "Screen Width"s, "Physical width (cm) of the display area of the playfield (main) screen."s, 5.f, 200.f, 0.1f, 95.89f);
   PropFloat(Player, ScreenHeight, "Screen Height"s, "Physical height (cm) of the display area of the playfield (main) screen."s, 5.f, 200.f, 0.1f, 53.94f);
   PropFloat(Player, ScreenInclination, "Screen Inclination"s, "Inclination (degree) of the playfield (main) screen. 0 is horizontal."s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(Player, LockbarWidth, "Lockbar Width"s, "Lockbar width in centimeters (measured on the cabinet)."s, 10.f, 150.f, 0.1f, 70.f);
   PropFloat(Player, LockbarHeight, "Lockbar Height"s, "Lockbar height in centimeters (measured on the cabinet, from ground to top of lockbar)."s, 0.f, 250.f, 0.1f, 85.f);
   PropFloat(Player, ScreenPlayerX, "Player X"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(Player, ScreenPlayerY, "Player Y"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -70.f, 30.f, 0.1f, -10.f);
   PropFloat(Player, ScreenPlayerZ, "Player Z"s, "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, 30.f, 100.f, 0.1f, 70.f);

   // Overall scene lighting settings
   PropBool(Player, OverrideTableEmissionScale, "Override Table Light Level"s, "Replace table light level by a custom one"s, false);
   PropFloat(Player, EmissionScale, "Day/Night"s, "Select a custom level between daylight or night time lighting"s, 0.f, 1.f, 0.001f, 1.f);
   PropBool(Player, DynamicDayNight, "Use Automatic Light Level"s, "Auytomatically compute scene lighting based on sun's position"s, false);
   PropFloat(Player, Latitude, "Latitude"s, "Latitude used to compute sun's position"s, -90.f, 90.f, 0.01f, 52.52f);
   PropFloat(Player, Longitude, "Longitude"s, "Longitude used to compute sun's position"s, -180.f, 180.f, 0.01f, 13.37f);

   // Table override settings
   // These properties are specials as they are meant to override table data at play time. They are not meant to be saved to the application setting file, but
   // only to table override settings. Also, their defaults are redefined when a table is played, depending on the table data and UI options.
   PropEnum(TableOverride, ViewDTMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewDTLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 25.f);
   PropFloat(TableOverride, ViewDTLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewDTScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewDTScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewDTPlayerX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTPlayerY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTPlayerZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(TableOverride, ViewDTHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewDTRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);
   
   PropEnum(TableOverride, ViewFSSMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 1, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewFSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 25.f);
   PropFloat(TableOverride, ViewFSSLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewFSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewFSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewFSSPlayerX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSPlayerY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSPlayerZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(TableOverride, ViewFSSHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewFSSRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropEnum(TableOverride, ViewCabMode, "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      int, 2, "Legacy"s, "Camera"s, "Window"s);
   PropFloat(TableOverride, ViewCabLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, 25.f);
   PropFloat(TableOverride, ViewCabLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewCabScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewCabScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 50.f, 150.f, 0.1f, 100.f);
   PropFloat(TableOverride, ViewCabPlayerX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabPlayerY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabPlayerZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, 70.f);
   PropFloat(TableOverride, ViewCabHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, 0.f);
   PropFloat(TableOverride, ViewCabRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

   PropFloat(TableOverride, Difficulty, "Difficulty"s, "Overall difficulty (slope, flipper size, trajectories scattering,...)"s, 0.f, 100.f, 0.1f, 100.f);
   PropFloat(TableOverride, Exposure, "Camera Exposure"s, "Overall brightness of the rendered scene"s, 0.f, 200.f, 0.1f, 100.f);
#ifdef ENABLE_BGFX
   PropEnum(TableOverride, ToneMapper, "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s,
      "Neutral"s, "AgX Punchy"s);
#else
   PropEnum(
      TableOverride, ToneMapper, "Tonemapper"s, "Select the way colors that are too bright to be rendered by the display are handled"s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s);
#endif

#undef PropBool
#undef PropInt
#undef PropEnum
#undef PropFloat


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
      Input,
      DMD, /* DMD Rendering */
      Alpha, /* Alpha segment Rendering */
      Backglass, /* Backglass Display */
      ScoreView, /* ScoreView Display */
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
      float minValue, maxValue, step, defaultValue, value;
      OptionUnit unit;
      vector<string> literals;
      string tokenizedLiterals;
   };
   OptionDef& RegisterSetting(const Section section, const string &id, const unsigned int showMask, const string &name, float minValue, float maxValue, float step, float defaultValue,
      OptionUnit unit, const vector<string> &literals);
   static const vector<OptionDef>& GetPluginSettings() { return m_pluginOptions; }

private:
#if 0
   bool LoadValue(const Section section, const string &key, void *const szbuffer, const size_t size) const;
   bool SaveValue(const Section section, const string &key, const char *val, const bool overrideMode = false);
#endif

   void RegisterStringSetting(const Section section, const string &key, const string &defVal, const bool addDefaults, const string &comments = string());
   void RegisterBoolSetting(const Section section, const string &key, const bool defVal, const bool addDefaults, const string &comments = string());
   void RegisterIntSetting(const Section section, const string &key, const int defVal, const int minVal, const int maxVal, const bool addDefaults, const string &comments = string());
   void RegisterUIntSetting(const Section section, const string &key, const unsigned int defVal, const unsigned int minVal, const unsigned int maxVal, const bool addDefaults, const string &comments = string());
   void RegisterFloatSetting(const Section section, const string &key, const float defVal, const float minVal, const float maxVal, const bool addDefaults, const string &comments = string());

   bool m_modified = false;
   string m_iniPath;
   mINI::INIStructure m_ini;
   const Settings * m_parent;
   #ifdef DEBUG
      ankerl::unordered_dense::map<Section, ankerl::unordered_dense::set<string>> m_validatedKeys;
   #endif

   // Shared across all settings
   static vector<OptionDef> m_pluginOptions;
   static vector<string> m_settingKeys;
};
