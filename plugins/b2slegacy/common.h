// license:GPLv3+

#pragma once

#include <cassert>

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <filesystem>

// Shared logging
#include "plugins/LoggingPlugin.h"

// Scriptable API
#include "plugins/ScriptablePlugin.h"

// VPX main API
#include "plugins/VPXPlugin.h"

#include <SDL3/SDL.h>

using std::string;
using namespace std::string_literals;
using std::vector;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace B2SLegacy
{

typedef enum {
    eDualMode_Both = 0,
    eDualMode_Authentic = 1,
    eDualMode_Fantasy = 2
} eDualMode;

typedef enum {
    eDualMode_2_NotSet = 0,
    eDualMode_2_Authentic = 1,
    eDualMode_2_Fantasy = 2
} eDualMode_2;

typedef enum {
    eRomIDType_NotDefined = 0,
    eRomIDType_Lamp = 1,
    eRomIDType_Solenoid = 2,
    eRomIDType_GIString = 3,
    eRomIDType_Mech = 4
} eRomIDType;

typedef enum {
    eDMDType_NotDefined = 0,
    eDMDType_NoB2SDMD = 1,
    eDMDType_B2SAlwaysOnSecondMonitor = 2,
    eDMDType_B2SAlwaysOnThirdMonitor = 3,
    eDMDType_B2SOnSecondOrThirdMonitor = 4
} eDMDType;

typedef enum {
    eDMDTypes_Standard = 0,
    eDMDTypes_TwoMonitorSetup = 1,
    eDMDTypes_ThreeMonitorSetup = 2,
    eDMDTypes_Hidden = 3
} eDMDTypes;

typedef enum {
    eLEDTypes_Undefined = 0,
    eLEDTypes_Rendered = 1,
    eLEDTypes_Dream7 = 2
} eLEDTypes;

// VB uses CheckedState_Indeterminate
// typedef enum {
//     B2SSettingsCheckedState_Unchecked = 0,
//     B2SSettingsCheckedState_Checked = 1,
//     B2SSettingsCheckedState_Indeterminate = 2
// } B2SSettingsCheckedState;

typedef enum {
    eType_Undefined = 0,
    eType_ImageCollectionAtForm = 1,
    eType_ImageCollectionAtPictureBox = 2,
    eType_PictureBoxCollection = 3
} eType;

typedef enum {
    eType_2_NotDefined = 0,
    eType_2_Undefined = 0,
    eType_2_ImageCollectionAtForm = 1,
    eType_2_ImageCollectionAtPictureBox = 2,
    eType_2_PictureBoxCollection = 3,
    eType_2_OnBackglass = 4,
    eType_2_OnDMD = 5
} eType_2;

typedef enum {
    eScoreType_Standard = 0,
    eScoreType_Player = 1,
    eScoreType_Credits = 2,
    eScoreType_NotUsed = 3
} eScoreType;

typedef enum {
    eCollectedDataType_TopImage = 1,
    eCollectedDataType_SecondImage = 2,
    eCollectedDataType_Standard = 4,
    eCollectedDataType_Animation = 8
} eCollectedDataType;

typedef enum {
    eLEDType_Undefined = 0,
    eLEDType_Standard = 1,
    eLEDType_Segment = 2,
    eLEDType_LED8 = 3,
    eLEDType_LED10 = 4,
    eLEDType_LED14 = 5,
    eLEDType_LED16 = 6
} eLEDType;

typedef enum {
    eSnippitRotationStopBehaviour_Undefined = 0,
    eSnippitRotationStopBehaviour_StopImmediately = 1,
    eSnippitRotationStopBehaviour_RunToEnd = 2,
    eSnippitRotationStopBehaviour_SpinOff = 3,
    eSnippitRotationStopBehaviour_RunAnimationTillEnd = 2,
    eSnippitRotationStopBehaviour_RunAnimationToFirstStep = 3
} eSnippitRotationStopBehaviour;

typedef enum {
    SegmentNumberType_7Seg = 0,
    SegmentNumberType_10Seg = 1,
    SegmentNumberType_14Seg = 2,
    SegmentNumberType_16Seg = 3,
    SegmentNumberType_SevenSegment = 0,
    SegmentNumberType_TenSegment = 1,
    SegmentNumberType_FourteenSegment = 2,
    SegmentNumberType_SixteenSegment = 3
} SegmentNumberType;

typedef enum {
    ScaleMode_Uniform = 0,
    ScaleMode_Stretch = 1,
    ScaleMode_Manual = 2,
    ScaleMode_Zoom = 3
} ScaleMode;

typedef enum {
    eDMDViewMode_NotDefined = 0,
    eDMDViewMode_NoDMD = 1,
    eDMDViewMode_Standard = 2,
    eDMDViewMode_Large = 3,
    eDMDViewMode_ShowDMD = 4,
    eDMDViewMode_ShowDMDOnlyAtDefaultLocation = 5,
    eDMDViewMode_DoNotShowDMDAtDefaultLocation = 6
} eDMDViewMode;

typedef enum {
    eControlType_Undefined = 0,
    eControlType_LED = 1,
    eControlType_Reel = 2,
    eControlType_Display = 3,
    eControlType_Dream7LEDDisplay = 4,
    eControlType_LEDDisplay = 5,
    eControlType_ReelDisplay = 6,
    eControlType_LEDBox = 7,
    eControlType_ReelBox = 8
} eControlType;

typedef enum {
    eLightsStateAtAnimationStart_Undefined = 0,
    eLightsStateAtAnimationStart_InvolvedLightsOff = 1,
    eLightsStateAtAnimationStart_InvolvedLightsOn = 2,
    eLightsStateAtAnimationStart_LightsOff = 3,
    eLightsStateAtAnimationStart_NoChange = 4
} eLightsStateAtAnimationStart;

typedef enum {
    eLightsStateAtAnimationEnd_Undefined = 0,
    eLightsStateAtAnimationEnd_InvolvedLightsOff = 1,
    eLightsStateAtAnimationEnd_InvolvedLightsOn = 2,
    eLightsStateAtAnimationEnd_LightsReseted = 3,
    eLightsStateAtAnimationEnd_NoChange = 4
} eLightsStateAtAnimationEnd;

typedef enum {
    eAnimationStopBehaviour_Undefined = 0,
    eAnimationStopBehaviour_StopImmediatelly = 1,
    eAnimationStopBehaviour_RunAnimationTillEnd = 2,
    eAnimationStopBehaviour_RunAnimationToFirstStep = 3
} eAnimationStopBehaviour;

typedef enum {
    ePictureBoxType_StandardImage = 0,
    ePictureBoxType_SelfRotatingImage = 1,
    ePictureBoxType_MechRotatingImage = 2
} ePictureBoxType;

typedef enum {
    eSnippitRotationDirection_Clockwise = 0,
    eSnippitRotationDirection_AntiClockwise = 1
} eSnippitRotationDirection;

LPI_USE();
#define LOGD B2SLegacy::LPI_LOGD
#define LOGI B2SLegacy::LPI_LOGI
#define LOGW B2SLegacy::LPI_LOGW
#define LOGE B2SLegacy::LPI_LOGE

PSC_USE_ERROR();

#ifndef RGB
#define RGB(r,g,b) ((uint32_t)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))
#define GetRValue(rgb) ((uint8_t)(rgb))
#define GetGValue(rgb) ((uint8_t)((rgb)>> 8))
#define GetBValue(rgb) ((uint8_t)((rgb)>>16))
#endif

constexpr inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32; // ASCII convention
   return c;
}

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile);
vector<uint8_t> base64_decode(const char * const __restrict value, const size_t size_bytes);
bool string_starts_with_case_insensitive(const string& str, const string& prefix);
int string_to_int(const string& str, int defaultValue);
bool is_string_numeric(const string& str);

}

template <typename T> constexpr inline T clamp(const T x, const T mn, const T mx) { return std::max(std::min(x, mx), mn); }

class vec3 final
{
public:
   constexpr vec3() { }
   constexpr vec3(float px, float py, float pz) : x(px), y(py), z(pz) { }

   float x = 0.f, y = 0.f, z = 0.f;
};

class vec4 final
{
public:
   constexpr vec4() { }
   constexpr vec4(float px, float py, float pz, float pw) : x(px), y(py), z(pz), w(pw) { }

   float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};

class ivec4 final
{
public:
   constexpr ivec4() { }
   constexpr ivec4(int px, int py, int pz, int pw) : x(px), y(py), z(pz), w(pw) { }

   int x = 0, y = 0, z = 0, w = 0;
};
