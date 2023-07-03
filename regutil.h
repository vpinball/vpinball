#pragma once

class RegName // do not change order/values in here unless you know what you're doing
{
public:
   // "Controller" is top level (to share data with VP9)
   static constexpr unsigned int Controller = 0;

   // All below is under the "VP10"-top level

   // UI and Player stuff
   static constexpr unsigned int Editor = 1;
   static constexpr unsigned int Player = 2;
   static constexpr unsigned int PlayerVR = 3;
   static constexpr unsigned int RecentDir = 4;
   static constexpr unsigned int Version = 5;
   static constexpr unsigned int CVEdit = 6;

   // Optional user defaults for each element
   static constexpr unsigned int DefaultPropsBumper = 7;
   static constexpr unsigned int DefaultPropsDecal = 8;
   static constexpr unsigned int DefaultPropsEMReel = 9;
   static constexpr unsigned int DefaultPropsFlasher = 10;
   static constexpr unsigned int DefaultPropsFlipper = 11;
   static constexpr unsigned int DefaultPropsGate = 12;
   static constexpr unsigned int DefaultPropsHitTarget = 13;
   static constexpr unsigned int DefaultPropsKicker = 14;
   static constexpr unsigned int DefaultPropsLight = 15;
   static constexpr unsigned int DefaultPropsLightSequence = 16;
   static constexpr unsigned int DefaultPropsPlunger = 17;
   static constexpr unsigned int DefaultPropsPrimitive = 18;
   static constexpr unsigned int DefaultPropsRamp = 19;
   static constexpr unsigned int DefaultPropsRubber = 20;
   static constexpr unsigned int DefaultPropsSpinner = 21;
   static constexpr unsigned int DefaultPropsWall = 22;
   static constexpr unsigned int DefaultPropsTarget = 23;
   static constexpr unsigned int DefaultPropsTextBox = 24;
   static constexpr unsigned int DefaultPropsTimer = 25;
   static constexpr unsigned int DefaultPropsTrigger = 26;

   static constexpr unsigned int DefaultCamera = 27;

   static constexpr unsigned int Num = 28;
};

static const string regKey[RegName::Num] =
   {
      "Controller"s, "Editor"s, "Player"s, "PlayerVR"s, "RecentDir"s, "Version"s, "CVEdit"s,
      "DefaultProps\\Bumper"s, "DefaultProps\\Decal"s, "DefaultProps\\EMReel"s, "DefaultProps\\Flasher"s, "DefaultProps\\Flipper"s,
      "DefaultProps\\Gate"s, "DefaultProps\\HitTarget"s, "DefaultProps\\Kicker"s, "DefaultProps\\Light"s, "DefaultProps\\LightSequence"s,
      "DefaultProps\\Plunger"s, "DefaultProps\\Primitive"s, "DefaultProps\\Ramp"s, "DefaultProps\\Rubber"s, "DefaultProps\\Spinner"s,
      "DefaultProps\\Wall"s, "DefaultProps\\Target"s, "DefaultProps\\TextBox"s, "DefaultProps\\Timer"s, "DefaultProps\\Trigger"s,
      "Defaults\\Camera"s
   };


void InitRegistry(const string &path);
void InitRegistryOverride(const string &path);
void SaveRegistry();


HRESULT LoadValue(const string &szKey, const string &zValue, string &buffer);
HRESULT LoadValue(const string &szKey, const string &szValue, void* const szbuffer, const DWORD size);

HRESULT LoadValue(const string &szKey, const string &szValue, float &pfloat);
float   LoadValueWithDefault(const string &szKey, const string &szValue, const float def); 

HRESULT LoadValue(const string &szKey, const string &szValue, int &pint);
HRESULT LoadValue(const string &szKey, const string &szValue, unsigned int &pint);
int     LoadValueWithDefault(const string &szKey, const string &szValue, const int def);

bool    LoadValueWithDefault(const string &szKey, const string &szValue, const bool def);


HRESULT SaveValue(const string &szKey, const string &szValue, const char *val);
HRESULT SaveValue(const string &szKey, const string &szValue, const string &val);
HRESULT SaveValue(const string &szKey, const string &szValue, const float val);
HRESULT SaveValue(const string &szKey, const string &szValue, const int val);
HRESULT SaveValue(const string &szKey, const string &szValue, const bool val);

HRESULT DeleteValue(const string &szKey, const string &szValue);
HRESULT DeleteSubKey(const string &szKey);
