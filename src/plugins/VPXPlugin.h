// license:GPLv3+

#pragma once

#include "MsgPlugin.h"

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
// This API takes for granted that at any time, only one game can be played. 
// All calls related to a running game may only be called between the 'OnGameStart' 
// and 'OnGameEnd' events.


#define VPXPI_NAMESPACE "VPX" // Namespace used for all VPX message definition

// Core VPX messages
#define VPXPI_MSG_GET_API               "GetAPI"              // Get the main VPX plugin API

// Core VPX events
#define VPXPI_EVT_ON_GAME_START         "OnGameStart"         // Broadcasted during player creation, before script initialization
#define VPXPI_EVT_ON_GAME_END           "OnGameEnd"           // Broadcasted during player shutdown
#define VPXPI_EVT_ON_PREPARE_FRAME      "OnPrepareFrame"      // Broadcasted when player starts preparing a new frame
#define VPXPI_EVT_ON_SETTINGS_CHANGED   "OnSettingsChanged"   // Broadcasted when settings have been changed


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
   // General information API
   void (MSGPIAPI *GetTableInfo)(VPXTableInfo* info);

   // User Interface
   enum OptionUnit { NONE, PERCENT };
   float (MSGPIAPI *GetOption)(const char* pageId, const char* optionId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step, const float defaultValue, const enum OptionUnit unit, const char** values);
   void* (MSGPIAPI *PushNotification)(const char* msg, const unsigned int lengthMs);
   void (MSGPIAPI *UpdateNotification)(const void* handle, const char* msg, const unsigned int lengthMs);

   // View management
   void (MSGPIAPI *DisableStaticPrerendering)(const int /* bool */ disable);
   void (MSGPIAPI *GetActiveViewSetup)(VPXViewSetupDef* view);
   void (MSGPIAPI *SetActiveViewSetup)(VPXViewSetupDef* view);

} VPXPluginAPI;
