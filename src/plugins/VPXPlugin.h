// license:GPLv3+

#pragma once

///////////////////////////////////////////////////////////////////////////////
// VPX plugins
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Plugins are a simple way to extend VPX core features. VPinballX application
// will scan for plugins in its 'plugin' folder, searching for subfolders
// containing a 'plugin.cfg' file. When found, the file will be read to find
// the needed metadata to present to the user as well as path for the native
// builds of the plugins for each supported platform. Then, the plugin will be
// available for the end user to enable it from the application settings.
// Another option is to statically build plugins into the application.
//
// For the sake of simplicity and portability, the API only use C definitions.
//
// The plugin API is not thread safe. If a plugin uses multithreading, it must
// perform all needed synchronization and data copies. Plugins share the same
// memory space.
//
// Plugins communicates between each others and with VPX using a basic event
// system. Events are identified by their name. They are registered by requesting 
// an id corresponding to this unique name. After registration, they may be 
// used by subscribing to them or broadcasting them. Events can be broadcasted 
// with an optional datablock. It is the task of the publisher & subscribers to 
// avoid any misunderstanding on the exchanged data. This mechanism can be used
// to share a data block that persists past the evennt broadcast. To avoid name
// conflicts, prefixing each plugin name by a dedicated prefix is encouraged.
// For VPX, this prefix is 'VPX.'.
//
// When loaded, plugins are provided a pointer table to the VPX API. This API
// takes for granted that at any time, only one game can be played. All calls
// related to a running game may only be called between the 'OnGameStart' & 
// 'OnGameEnd' events.
//
// Plugins are instantiated on the 'game thread' which may or may not be the
// main application thread. Therefore plugins are not allowed to perform any
// operation only allowed on the main application thread like creating OS
// windows.
// 
// Plugins must implement and export the load/unload functions to be valid.
// VPX_EXPORT void PluginLoad(VPXPluginAPI* api);
// VPX_EXPORT void PluginUnload();
//
// This header is a common header to be used both by VPX and its plugins.


#if defined(_MSC_VER)
   // Microsoft
#define VPX_EXPORT extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
   // GCC
#define VPX_EXPORT extern "C" __attribute__((visibility("default")))
#else
   // Others: hope that all symbols are exported
#define VPX_EXPORT extern "C" 
#endif

///////////////////////////////////////////////////////////////////////////////
// VPX Plugin API

// Callbacks
typedef void (*vpxpi_event_callback)(const unsigned int eventId, void* userData, void* eventData);

// Core VPX events
#define VPX_EVT_ON_GAME_START       "VPX.OnGameStart"       // Broadcasted during player creation, before script initialization
#define VPX_EVT_ON_GAME_END         "VPX.OnGameEnd"         // Broadcasted during player shutdown
#define VPX_EVT_ON_PREPARE_FRAME    "VPX.OnPrepareFrame"    // Broadcasted when player starts preparing a new frame
#define VPX_EVT_ON_SETTINGS_CHANGED "VPX.OnSettingsChanged" // Broadcasted when settings have been changed

// Core VPX settings pages
// GetOption 'pageId' parameter is either the id of a loaded plugin or the id of one of the core VPX pages defined below
#define VPX_TWEAK_VIEW           "VPX.tweak.view"
#define VPX_TWEAK_TABLE          "VPX.tweak.table"
// GetOption bitmask flags defining where the setting should be displayed
#define VPX_OPT_SHOW_UI          1
#define VPX_OPT_SHOW_TWEAK       2

// Helper defines

// Conversions to/from VP units (50 VPU = 1.0625 inches which is 1"1/16, the default size of a ball, 1 inch is 2.54cm)
// These value are very slightly off from original values which used a VPU to MM of 0.540425 instead of 0.53975 (result of the following formula)
// So it used to be 0.125% larger which is not noticeable but makes it difficult to have perfect matches when playing between apps
#ifndef MMTOVPU
#define MMTOVPU(x) ((x) * (float)(50. / (25.4 * 1.0625)))
#define CMTOVPU(x) ((x) * (float)(50. / (2.54 * 1.0625)))
#define VPUTOMM(x) ((x) * (float)(25.4 * 1.0625 / 50.))
#define VPUTOCM(x) ((x) * (float)(2.54 * 1.0625 / 50.))
#define INCHESTOVPU(x) ((x) * (float)(50. / 1.0625))
#define VPUTOINCHES(x) ((x) * (float)(1.0625 / 50.))
#endif


// For compability and ease of binding reasons we stick to C 89 syntax: no bool type, no nested typedef,...
#ifndef BOOL
typedef int                 BOOL;
#endif


typedef struct VPXTableInfo
{
   const char* path;              // [R_]
   float tableWidth, tableHeight; // [R_]
} VPXTableInfo;


typedef struct VPXViewSetupDef
{
   // See ViewSetup class for member description
   int viewMode;                                       // [R_]
   float sceneScaleX, sceneScaleY, sceneScaleZ;        // [R_]
   float viewX, viewY, viewZ;                          // [RW]
   float lookAt;                                       // [R_]
   float viewportRotation;                             // [R_]
   float FOV;                                          // [R_]
   float layback;                                      // [R_]
   float viewHOfs, viewVOfs;                           // [R_]
   float windowTopZOfs, windowBottomZOfs;              // [R_]
   // Following fields are defined in user settings
   float screenWidth, screenHeight, screenInclination; // [R_]
   float realToVirtualScale;                           // [R_]
   float interpupillaryDistance;                       // [R_] TODO upgrade to RW to allow head tracking to measure and adjust accordingly
} VPXViewSetupDef;


typedef struct VPXPluginAPI
{
   // Communication bus
   unsigned int (*GetEventID)(const char* name);
   void (*SubscribeEvent)(const unsigned int eventId, const vpxpi_event_callback callback, void* userData);
   void (*UnsubscribeEvent)(const unsigned int eventId, const vpxpi_event_callback callback);
   void (*BroadcastEvent)(const unsigned int eventId, void* data);

   // General information API
   void (*GetTableInfo)(VPXTableInfo* info);

   // User Interface
   enum OptionUnit { NONE, PERCENT };
   float (*GetOption)(const char* pageId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step, const float defaultValue, const OptionUnit unit, const char** values);
   void* (*PushNotification)(const char* msg, const unsigned int lengthMs);
   void (*UpdateNotification)(const void* handle, const char* msg, const unsigned int lengthMs);

   // View management
   void (*DisableStaticPrerendering)(const BOOL disable);
   void (*GetActiveViewSetup)(VPXViewSetupDef* view);
   void (*SetActiveViewSetup)(VPXViewSetupDef* view);

} VPXPluginAPI;
