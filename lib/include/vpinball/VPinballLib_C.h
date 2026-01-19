// license:GPLv3+

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
   VPINBALL_SCRIPT_ERROR_TYPE_COMPILE,
   VPINBALL_SCRIPT_ERROR_TYPE_RUNTIME
} VPINBALL_SCRIPT_ERROR_TYPE;

typedef enum {
   VPINBALL_EVENT_INIT_COMPLETE,
   VPINBALL_EVENT_LOADING_ITEMS,
   VPINBALL_EVENT_LOADING_SOUNDS,
   VPINBALL_EVENT_LOADING_IMAGES,
   VPINBALL_EVENT_LOADING_FONTS,
   VPINBALL_EVENT_LOADING_COLLECTIONS,
   VPINBALL_EVENT_PRERENDERING,
   VPINBALL_EVENT_PLAYER_STARTED,
   VPINBALL_EVENT_RUMBLE,
   VPINBALL_EVENT_SCRIPT_ERROR,
   VPINBALL_EVENT_PLAYER_CLOSED,
   VPINBALL_EVENT_WEB_SERVER,
   VPINBALL_EVENT_COMMAND
} VPINBALL_EVENT;

typedef enum {
   VPINBALL_PATH_ROOT,
   VPINBALL_PATH_TABLES,
   VPINBALL_PATH_PREFERENCES,
   VPINBALL_PATH_ASSETS
} VPINBALL_PATH;

// Callbacks

typedef void (*VPinballEventCallback)(VPINBALL_EVENT, const char*);
typedef void (*VPinballZipCallback)(int current, int total, const char* filename);

// Functions

VPINBALLAPI const char* VPinballGetVersionStringFull();
VPINBALLAPI void VPinballInit(VPinballEventCallback callback);
VPINBALLAPI void VPinballLog(VPINBALL_LOG_LEVEL level, const char* message);
VPINBALLAPI void VPinballResetLog();

// Settings

VPINBALLAPI int VPinballLoadValueInt(const char* pSectionName, const char* pKey, int defaultValue);
VPINBALLAPI void VPinballSaveValueInt(const char* pSectionName, const char* pKey, int value);
VPINBALLAPI float VPinballLoadValueFloat(const char* pSectionName, const char* pKey, float defaultValue);
VPINBALLAPI void VPinballSaveValueFloat(const char* pSectionName, const char* pKey, float value);
VPINBALLAPI const char* VPinballLoadValueString(const char* pSectionName, const char* pKey, const char* pDefaultValue);
VPINBALLAPI void VPinballSaveValueString(const char* pSectionName, const char* pKey, const char* pValue);
VPINBALLAPI int VPinballLoadValueBool(const char* pSectionName, const char* pKey, int defaultValue);
VPINBALLAPI void VPinballSaveValueBool(const char* pSectionName, const char* pKey, int value);
VPINBALLAPI VPINBALL_STATUS VPinballResetIni();

// Web Server

VPINBALLAPI void VPinballUpdateWebServer();

// Paths

VPINBALLAPI const char* VPinballGetPath(VPINBALL_PATH pathType);

// Player

VPINBALLAPI VPINBALL_STATUS VPinballLoadTable(const char* pPath);
VPINBALLAPI VPINBALL_STATUS VPinballExtractTableScript();
VPINBALLAPI VPINBALL_STATUS VPinballPlay();
VPINBALLAPI VPINBALL_STATUS VPinballStop();

// Zip

VPINBALLAPI VPINBALL_STATUS VPinballZipCreate(const char* pSourcePath, const char* pDestPath, VPinballZipCallback callback);
VPINBALLAPI VPINBALL_STATUS VPinballZipExtract(const char* pSourcePath, const char* pDestPath, VPinballZipCallback callback);
