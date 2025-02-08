// license:GPLv3+

#pragma once

#ifdef __cplusplus
 #include <cstdint>
#else
 #include <stdint.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Generic Message Plugin API
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This file defines a generic plugin API to allow sharing plugins accross virtual
// pinball application. It is minimalistic and only defines the core collaboration
// mechanism. Then each application/plugin should define its API & messages.
//
// For the sake of simplicity and portability, the API only uses C definitions.
//
// Plugins are designed to be hosted into the main application process. Therefore
// they share the same memory space. Passing data pointer is safe as long as the
// lifecycle and ownership is clearly defined and ensured by the host/plugin.
//
// The plugin API is not thread safe. If a plugin uses multithreading, it must
// perform all needed synchronization and data copies. The 'RunOnMainThread' method
// is the only one that may be called from any thread to request a callback to be
// ran on the main thread, either as a blocking call if delay is negative, or as an
// async call if delay is zero or positive.
//
// To avoid message collision, each message is defined by a unique name in a 'namespace'
// which is expected to be unique for each host/plugin. MsgId are allocated/retrieved
// by using GetMsgID. If not needed anymore, host/plugin should call 'ReleaseMsgID' to
// avoid reaching the implementation dependent message limits.
//
// Plugins are instantiated on a thread selected by the host which may or may not be 
// the main application thread. Therefore, unless plugin is designed for a single host 
// which explicitely allows it, plugins are not allowed to perform any operation limited
// to the main application thread like most OS/UI methods.
// 
// A basic setting mechanism is also provided to allow easier integration.
//
// Plugins must implement and export the load/unload functions to be valid.
// MSGPI_EXPORT void PluginLoad(const uint32_t endpointId, MsgPluginAPI* api);
// MSGPI_EXPORT void PluginUnload();
//
// Plugins can be statically linked to host application on platforms requiring it or loaded
// dynamically. For dynamic loading, plugins are expected to define a 'plugin.cfg' file that
// will be scanned. When found, the file will be read to find the needed metadata to present
// to the user as well as path for the native builds of the plugins for each supported platform.
// Then, the plugin will be available for the end user to enable it from the host application.
// 
// This header is a common header to be used both by host and plugins.


#if defined(_MSC_VER)
   // Microsoft
   #ifdef _WIN64
      // Windows x64 standard calling convention (implicit)
      #define MSGPIAPI
   #else
      // Other platforms, uses C calling convetion as we need variadic
      #define MSGPIAPI __cdecl
   #endif
   #define MSGPI_EXPORT extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
   // GCC
   #ifdef _WIN64
      // Windows x64 standard calling convention (implicit)
      #define MSGPIAPI
   #else
      // Other platforms, uses C calling convetion as we need variadic
      // TODO: #define MSGPIAPI __attribute__((__cdecl))
      #define MSGPIAPI
   #endif
   #define MSGPI_EXPORT extern "C" __attribute__((visibility("default"))) MSGPIAPI
#else
   // Others
   #ifdef _WIN64
      // Windows x64 standard calling convention (implicit)
      #define MSGPIAPI
   #else
      // Other platforms, uses C calling convetion as we need variadic
      #define MSGPIAPI __cdecl
   #endif
   // Hope that all symbols are exported
   #define MSGPI_EXPORT extern "C"
#endif

// Callbacks
typedef void (*msgpi_msg_callback)(const unsigned int msgId, void* userData, void* msgData);
typedef void (*msgpi_timer_callback)(void* userData);

typedef struct MsgPluginAPI
{
   // Messageing
   unsigned int (MSGPIAPI *GetMsgID)(const char* name_space, const char* name);
   void (MSGPIAPI *SubscribeMsg)(const uint32_t endpointId, const unsigned int msgId, const msgpi_msg_callback callback, void* userData);
   void (MSGPIAPI *UnsubscribeMsg)(const unsigned int msgId, const msgpi_msg_callback callback);
   void (MSGPIAPI *BroadcastMsg)(const uint32_t endpointId, const unsigned int msgId, void* data);
   void (MSGPIAPI *ReleaseMsgID)(const unsigned int msgId);
   // Setting
   void (MSGPIAPI *GetSetting)(const char* name_space, const char* name, char* valueBuf, unsigned int valueBufSize);
   // Threading
   void (MSGPIAPI *RunOnMainThread)(const double delayInS, const msgpi_timer_callback callback, void* userData);
} MsgPluginAPI;
