#pragma once

class RegName // do not change order/values in here unless you know what you're doing
{
public:
   // "Controller" is top level (to share data with VP9)
   static constexpr unsigned int Controller = 0;

   // All below is under the "VP10"-top level

   // UI and Player stuff
   static constexpr unsigned int Editor = 1;
   static constexpr unsigned int Standalone = 2;
   static constexpr unsigned int Player = 3;
   static constexpr unsigned int PlayerVR = 4;
   static constexpr unsigned int RecentDir = 5;
   static constexpr unsigned int Version = 6;
   static constexpr unsigned int CVEdit = 7;

   // Optional user defaults for each element
   static constexpr unsigned int DefaultPropsBumper = 8;
   static constexpr unsigned int DefaultPropsDecal = 9;
   static constexpr unsigned int DefaultPropsEMReel = 10;
   static constexpr unsigned int DefaultPropsFlasher = 11;
   static constexpr unsigned int DefaultPropsFlipper = 12;
   static constexpr unsigned int DefaultPropsGate = 13;
   static constexpr unsigned int DefaultPropsHitTarget = 14;
   static constexpr unsigned int DefaultPropsKicker = 15;
   static constexpr unsigned int DefaultPropsLight = 16;
   static constexpr unsigned int DefaultPropsLightSequence = 17;
   static constexpr unsigned int DefaultPropsPlunger = 18;
   static constexpr unsigned int DefaultPropsPrimitive = 19;
   static constexpr unsigned int DefaultPropsRamp = 20;
   static constexpr unsigned int DefaultPropsRubber = 21;
   static constexpr unsigned int DefaultPropsSpinner = 22;
   static constexpr unsigned int DefaultPropsWall = 23;
   static constexpr unsigned int DefaultPropsTarget = 24;
   static constexpr unsigned int DefaultPropsTextBox = 25;
   static constexpr unsigned int DefaultPropsTimer = 26;
   static constexpr unsigned int DefaultPropsTrigger = 27;

   static constexpr unsigned int DefaultCamera = 28;

   static constexpr unsigned int Num = 29;
};

static const string regKey[RegName::Num] =
   {
      "Controller"s, "Editor"s, "Standalone"s, "Player"s, "PlayerVR"s, "RecentDir"s, "Version"s, "CVEdit"s,
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
HRESULT LoadValueWithDefault(const string &szKey, const string &zValue, string &buffer, const string& def);

string  LoadValueWithDefault(const string &szKey, const string &szValue, const string def);

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
