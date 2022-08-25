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
   static constexpr unsigned int RecentDir = 3;
   static constexpr unsigned int Version = 4;
   static constexpr unsigned int CVEdit = 5;

   // Optional user defaults for each element
   static constexpr unsigned int DefaultPropsBumper = 6;
   static constexpr unsigned int DefaultPropsDecal = 7;
   static constexpr unsigned int DefaultPropsEMReel = 8;
   static constexpr unsigned int DefaultPropsFlasher = 9;
   static constexpr unsigned int DefaultPropsFlipper = 10;
   static constexpr unsigned int DefaultPropsGate = 11;
   static constexpr unsigned int DefaultPropsHitTarget = 12;
   static constexpr unsigned int DefaultPropsKicker = 13;
   static constexpr unsigned int DefaultPropsLight = 14;
   static constexpr unsigned int DefaultPropsLightSequence = 15;
   static constexpr unsigned int DefaultPropsPlunger = 16;
   static constexpr unsigned int DefaultPropsPrimitive = 17;
   static constexpr unsigned int DefaultPropsRamp = 18;
   static constexpr unsigned int DefaultPropsRubber = 19;
   static constexpr unsigned int DefaultPropsSpinner = 20;
   static constexpr unsigned int DefaultPropsWall = 21;
   static constexpr unsigned int DefaultPropsTarget = 22;
   static constexpr unsigned int DefaultPropsTextBox = 23;
   static constexpr unsigned int DefaultPropsTimer = 24;
   static constexpr unsigned int DefaultPropsTrigger = 25;

   static constexpr unsigned int Num = 26;
};

static const string regKey[RegName::Num]
   = { "Controller"s,
       "Editor"s, "Player"s, "RecentDir"s, "Version"s, "CVEdit"s,
       "DefaultProps\\Bumper"s, "DefaultProps\\Decal"s, "DefaultProps\\EMReel"s, "DefaultProps\\Flasher"s, "DefaultProps\\Flipper"s, "DefaultProps\\Gate"s, "DefaultProps\\HitTarget"s, "DefaultProps\\Kicker"s, "DefaultProps\\Light"s, "DefaultProps\\LightSequence"s, "DefaultProps\\Plunger"s, "DefaultProps\\Primitive"s, "DefaultProps\\Ramp"s, "DefaultProps\\Rubber"s, "DefaultProps\\Spinner"s, "DefaultProps\\Wall"s, "DefaultProps\\Target"s, "DefaultProps\\TextBox"s, "DefaultProps\\Timer"s, "DefaultProps\\Trigger"s };


void SaveXMLregistry(const string &path);
void InitXMLregistry(const string &path);
void ClearXMLregistry(); // clears self-allocated strings


HRESULT LoadValue(const string &szKey, const string &zValue, string &buffer);
HRESULT LoadValue(const string &szKey, const string &szValue, void* const szbuffer, const DWORD size);

HRESULT LoadValue(const string &szKey, const string &szValue, float &pfloat);
float   LoadValueFloatWithDefault(const string &szKey, const string &szValue, const float def); 

HRESULT LoadValue(const string &szKey, const string &szValue, int &pint);
HRESULT LoadValue(const string &szKey, const string &szValue, unsigned int &pint);
int     LoadValueIntWithDefault(const string &szKey, const string &szValue, const int def);

bool    LoadValueBoolWithDefault(const string &szKey, const string &szValue, const bool def);


HRESULT SaveValue(const string &szKey, const string &szValue, const char *val);
HRESULT SaveValue(const string &szKey, const string &szValue, const string &val);
HRESULT SaveValueFloat(const string &szKey, const string &szValue, const float val);
HRESULT SaveValueInt(const string &szKey, const string &szValue, const int val);
HRESULT SaveValueBool(const string &szKey, const string &szValue, const bool val);

HRESULT DeleteValue(const string &szKey, const string &szValue);
HRESULT DeleteSubKey(const string &szKey);
