#pragma once

#ifdef _MSC_VER
#define VPINBALLAPI extern "C" __declspec(dllexport)
#define VPINBALLCALLBACK __stdcall
#else
#define VPINBALLAPI extern "C" __attribute__((visibility("default")))
#define VPINBALLCALLBACK
#endif

// Enums

typedef enum {
   VPINBALL_LOG_LEVEL_DEBUG,
   VPINBALL_LOG_LEVEL_INFO,
   VPINBALL_LOG_LEVEL_WARN,
   VPINBALL_LOG_LEVEL_ERROR
} VPINBALL_LOG_LEVEL;

typedef enum {
   VPINBALL_STATUS_SUCCESS,
   VPINBALL_STATUS_FAILURE
} VPINBALL_STATUS;

typedef enum {
   VPINBALL_SETTINGS_SECTION_STANDALONE = 2,
   VPINBALL_SETTINGS_SECTION_PLAYER = 3,
   VPINBALL_SETTINGS_SECTION_DMD = 4,
   VPINBALL_SETTINGS_SECTION_ALPHA = 5,
   VPINBALL_SETTINGS_SECTION_BACKGLASS = 6,
   VPINBALL_SETTINGS_SECTION_TABLE_OVERRIDE = 11,
   VPINBALL_SETTINGS_SECTION_TABLE_OPTION = 12
} VPINBALL_SETTINGS_SECTION;

typedef enum {
   VPINBALL_SCRIPT_ERROR_TYPE_COMPILE,
   VPINBALL_SCRIPT_ERROR_TYPE_RUNTIME
} VPINBALL_SCRIPT_ERROR_TYPE;

typedef enum {
   VPINBALL_OPTION_UNIT_NO_UNIT,
   VPINBALL_OPTION_UNIT_PERCENT
} VPINBALL_OPTION_UNIT;

typedef enum {
   VPINBALL_EVENT_ARCHIVE_UNCOMPRESSING,
   VPINBALL_EVENT_ARCHIVE_COMPRESSING,
   VPINBALL_EVENT_LOADING_ITEMS,
   VPINBALL_EVENT_LOADING_SOUNDS,
   VPINBALL_EVENT_LOADING_IMAGES,
   VPINBALL_EVENT_LOADING_FONTS,
   VPINBALL_EVENT_LOADING_COLLECTIONS,
   VPINBALL_EVENT_PLAYER_STARTING,
   VPINBALL_EVENT_WINDOW_CREATED,
   VPINBALL_EVENT_METAL_LAYER_IOS,
   VPINBALL_EVENT_PRERENDERING,
   VPINBALL_EVENT_PLAYER_STARTED,
   VPINBALL_EVENT_RUMBLE,
   VPINBALL_EVENT_SCRIPT_ERROR,
   VPINBALL_EVENT_LIVEUI_TOGGLE,
   VPINBALL_EVENT_LIVEUI_UPDATE,
   VPINBALL_EVENT_PLAYER_CLOSING,
   VPINBALL_EVENT_PLAYER_CLOSED,
   VPINBALL_EVENT_STOPPED,
   VPINBALL_EVENT_WEB_SERVER
} VPINBALL_EVENT;

typedef struct {
   int progress;
} VPinballProgressData;

typedef struct {
   void* pWindow;
   const char* pTitle;
} VPinballWindowCreatedData;

typedef struct {
   VPINBALL_SCRIPT_ERROR_TYPE error;
   int line;
   int position;
   const char* pDescription;
} VPinballScriptErrorData;

typedef struct {
   uint16_t lowFrequencyRumble;
   uint16_t highFrequencyRumble;
   uint32_t durationMs;
} VPinballRumbleData;

typedef struct {
   const char* pUrl;
} VPinballWebServerData;

typedef struct {
   float globalEmissionScale;
   float globalDifficulty;
   float exposure;
   int toneMapper;
   int musicVolume;
   int soundVolume;
} VPinballTableOptions;

typedef struct {
   VPINBALL_SETTINGS_SECTION section;
   const char* id;
   const char* name;
   int showMask;
   float minValue;
   float maxValue;
   float step;
   float defaultValue;
   VPINBALL_OPTION_UNIT unit;
   const char* literals;
   float value;
} VPinballCustomTableOption;

typedef struct {
   int viewMode;
   float sceneScaleX;
   float sceneScaleY;
   float sceneScaleZ;
   float viewX;
   float viewY;
   float viewZ;
   float lookAt;
   float viewportRotation;
   float fov;
   float layback;
   float viewHOfs;
   float viewVOfs;
   float windowTopZOfs;
   float windowBottomZOfs;
} VPinballViewSetup;

// Callbacks

typedef void* (*VPinballEventCallback)(VPINBALL_EVENT, void*);

// Functions

VPINBALLAPI const char* VPinballGetVersionStringFull();
VPINBALLAPI void VPinballInit(VPinballEventCallback callback);
VPINBALLAPI void VPinballLog(VPINBALL_LOG_LEVEL level, const char* pMessage);
VPINBALLAPI void VPinballResetLog();
VPINBALLAPI int VPinballLoadValueInt(VPINBALL_SETTINGS_SECTION section, const char* pKey, int defaultValue);
VPINBALLAPI float VPinballLoadValueFloat(VPINBALL_SETTINGS_SECTION section, const char* pKey, float defaultValue);
VPINBALLAPI const char* VPinballLoadValueString(VPINBALL_SETTINGS_SECTION section, const char* pKey, const char* pDefaultValue);
VPINBALLAPI void VPinballSaveValueInt(VPINBALL_SETTINGS_SECTION section, const char* pKey, int value);
VPINBALLAPI void VPinballSaveValueFloat(VPINBALL_SETTINGS_SECTION section, const char* pKey, float value);
VPINBALLAPI void VPinballSaveValueString(VPINBALL_SETTINGS_SECTION section, const char* pKey, const char* pValue);
VPINBALLAPI VPINBALL_STATUS VPinballUncompress(const char* pSource);
VPINBALLAPI VPINBALL_STATUS VPinballCompress(const char* pSource, const char* pDestination);
VPINBALLAPI void VPinballUpdateWebServer();
VPINBALLAPI VPINBALL_STATUS VPinballResetIni();
VPINBALLAPI VPINBALL_STATUS VPinballLoad(const char* pSource);
VPINBALLAPI VPINBALL_STATUS VPinballExtractScript(const char* pSource);
VPINBALLAPI VPINBALL_STATUS VPinballPlay();
VPINBALLAPI VPINBALL_STATUS VPinballStop();
VPINBALLAPI void VPinballSetPlayState(int enable);
VPINBALLAPI void VPinballToggleFPS();
VPINBALLAPI void VPinballGetTableOptions(VPinballTableOptions* pTableOptions);
VPINBALLAPI void VPinballSetTableOptions(VPinballTableOptions* pTableOptions);
VPINBALLAPI void VPinballSetDefaultTableOptions();
VPINBALLAPI void VPinballResetTableOptions();
VPINBALLAPI void VPinballSaveTableOptions();
VPINBALLAPI int VPinballGetCustomTableOptionsCount();
VPINBALLAPI void VPinballGetCustomTableOption(int index, VPinballCustomTableOption* pCustomTableOption);
VPINBALLAPI void VPinballSetCustomTableOption(VPinballCustomTableOption* pCustomTableOption);
VPINBALLAPI void VPinballSetDefaultCustomTableOptions();
VPINBALLAPI void VPinballResetCustomTableOptions();
VPINBALLAPI void VPinballSaveCustomTableOptions();
VPINBALLAPI void VPinballGetViewSetup(VPinballViewSetup* pViewSetup);
VPINBALLAPI void VPinballSetViewSetup(VPinballViewSetup* pViewSetup);
VPINBALLAPI void VPinballSetDefaultViewSetup();
VPINBALLAPI void VPinballResetViewSetup();
VPINBALLAPI void VPinballSaveViewSetup();