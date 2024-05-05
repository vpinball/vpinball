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
// perform all needed synchronization and data copies. Plugin share the same
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
// 'OnGameEnd' events. They are marked '[InGame]' in the following comments.
//
// Plugins must implement and export the load/unload functions to be valid.
// For windows, use:
// extern "C" __declspec(dllexport) void PluginLoad(VPXPluginAPI* api);
// extern "C" __declspec(dllexport) void PluginUnload();
//
// This header is a common header to be used both by VPX and its plugins.


///////////////////////////////////////////////////////////////////////////////
// VPX Plugin API

// Callbacks
typedef void (*vpxpi_event_callback)(const unsigned int eventId, void* data);

// Core VPX events:
#define VPX_EVT_ON_GAME_START    "VPX.OnGameStart"    // Broadcasted during player creation, before script initialization
#define VPX_EVT_ON_GAME_END      "VPX.OnGameEnd"      // Broadcasted during player shutdown
#define VPX_EVT_ON_PREPARE_FRAME "VPX.OnPrepareFrame" // Broadcasted when player starts preparing a new frame

typedef struct
{
   // Plugin Events
   unsigned int (*GetEventID)(const char* name);
   void (*SubscribeEvent)(const unsigned int eventId, const vpxpi_event_callback callback);
   void (*UnsubscribeEvent)(const unsigned int eventId, const vpxpi_event_callback callback);
   void (*BroadcastEvent)(const unsigned int eventId, void* data);

   // Core VPX API
   // TODO add VPX API (same as COM ?)

} VPXPluginAPI;
