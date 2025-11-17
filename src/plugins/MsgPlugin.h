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
// This file defines a generic plugin API to allow sharing plugins across virtual
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
// async call if delay is zero or positive. Plugins may define and handle functions
// with relaxed threading constraints, as long as this is clearly advertised.
//
// To avoid message collision, each message is defined by a unique name in a 'namespace'
// which is expected to be unique for each host/plugin. MsgId are allocated/retrieved
// by using GetMsgID. If not needed anymore, host/plugin should call 'ReleaseMsgID' to
// avoid reaching the implementation dependent message limits.
//
// Plugins are instantiated on a thread selected by the host which may or may not be 
// the main application thread. Therefore, unless plugin is designed for a single host 
// which explicitly allows it, plugins are not allowed to perform any operation limited
// to the main application thread like most OS/UI methods.
// 
// A basic setting mechanism is also provided to allow easier integration.
//
// Plugins must implement and export the load/unload functions to be valid.
// MSGPI_EXPORT void xxxPluginLoad(const uint32_t endpointId, MsgPluginAPI* api);
// MSGPI_EXPORT void xxxPluginUnload();
// Where xxx is the plugin id (to avoid name conflict when using static linking)
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
      // Other platforms, uses C calling convention as we need variadic
      #define MSGPIAPI __cdecl
   #endif
   #define MSGPI_EXPORT extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
   // GCC
   #ifdef _WIN64
      // Windows x64 standard calling convention (implicit)
      #define MSGPIAPI
   #else
      // Other platforms, uses C calling convention as we need variadic
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
      // Other platforms, uses C calling convention as we need variadic
      #define MSGPIAPI __cdecl
   #endif
   // Hope that all symbols are exported
   #define MSGPI_EXPORT extern "C"
#endif

// Callbacks
typedef void (*msgpi_msg_callback)(const unsigned int msgId, void* context, void* msgData);
typedef void (*msgpi_timer_callback)(void* userData);

typedef struct MsgEndpointInfo
{
   const char* id;          // Unique ID of the plugin, used to identify it
   const char* name;        // Human-readable name of the plugin
   const char* description; // Human-readable description of the plugin intent
   const char* author;      // Human-readable author name
   const char* version;     // Human-readable version
   const char* link;        // Web link to online information
} MsgEndpointInfo;

#define MSGPI_SETTING_TYPE_FLOAT  0
#define MSGPI_SETTING_TYPE_INT    1
#define MSGPI_SETTING_TYPE_BOOL   2
#define MSGPI_SETTING_TYPE_STRING 3

typedef struct MsgSettingDef
{
   const char* propId;       // Unique Id of the property within the plugin
   const char* name;         // Human-readable name of the plugin
   const char* description;  // Human-readable description of the plugin intent
   const int isUserEditable; // If non zero, exposed to user for edition
   const int type;           // See MSGPI_SETTING_TYPE_xxx
   union 
   {
      struct
      {
         const float minVal;
         const float maxVal;
         const float step;   // If non zero, limit values to increments of step
         const float defVal;
         float (MSGPIAPI* Get)();
         void (MSGPIAPI* Set)(float val);
      } floatDef;
      struct
      {
         const int minVal;
         const int maxVal;
         const int defVal;
         const char** values;
         int (MSGPIAPI* Get)();
         void (MSGPIAPI* Set)(int val);
      } intDef;
      struct
      {
         const int defVal;
         int (MSGPIAPI* Get)();
         void (MSGPIAPI* Set)(int val);
      } boolDef;
      struct
      {
         const char* defVal;
         const char* (MSGPIAPI* Get)();
         void (MSGPIAPI* Set)(const char* val);
      } stringDef;
   };
} MsgSettingDef;

// C++ helpers to define settings
#define MSGPI_FLOAT_SETTING(varName, id, propName, propDescription, propEditable, minValue, maxValue, stepVal, defValue, getter, setter) \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_FLOAT, .intDef = { minValue, maxValue, stepVal, defValue, getter, setter } }
#define MSGPI_INT_SETTING(varName, id, propName, propDescription, propEditable, minValue, maxValue, defValue, getter, setter) \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_INT, .intDef = { minValue, maxValue, defValue, nullptr, getter, setter } }
#define MSGPI_ENUM_SETTING(varName, id, propName, propDescription, propEditable, minValue, nValues, values, defValue, getter, setter) \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_INT, .intDef = { minValue, minValue + nValues - 1, defValue, values, getter, setter } }
#define MSGPI_BOOL_SETTING(varName, id, propName, propDescription, propEditable, defValue, getter, setter) \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_BOOL, .boolDef = { defValue?1:0, getter, setter } }
#define MSGPI_STRING_SETTING(varName, id, propName, propDescription, propEditable, defValue, bufferSize, getter, setter) \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_STRING, .stringDef = { defValue, getter, setter } }
#define MSGPI_FLOAT_VAL_SETTING(varName, id, propName, propDescription, propEditable, minValue, maxValue, stepVal, defValue) \
   static float varName##_Val; \
   static float varName##_Get() { return varName##_Val; } \
   static void varName##_Set(float v) { varName##_Val = v; } \
   static MsgSettingDef varName { .propId = id, .name = propName, .description = propDescription, .isUserEditable = propEditable ? 1 : 0, .type = MSGPI_SETTING_TYPE_FLOAT, .intDef = { minValue, maxValue, stepVal, defValue, &varName##_Get, &varName##_Set } }
#define MSGPI_INT_VAL_SETTING(varName, id, propName, propDescription, propEditable, minValue, maxValue, defValue) \
   static int varName##_Val; \
   static int varName##_Get() { return varName##_Val; } \
   static void varName##_Set(int v) { varName##_Val = v; } \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_INT, .intDef = { minValue, maxValue, defValue, nullptr, &varName##_Get, &varName##_Set } }
#define MSGPI_ENUM_VAL_SETTING(varName, id, propName, propDescription, propEditable, minValue, nValues, values, defValue) \
   static int varName##_Val; \
   static int varName##_Get() { return varName##_Val; } \
   static void varName##_Set(int v) { varName##_Val = v; } \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_INT, .intDef = { minValue, minValue + nValues - 1, defValue, values, &varName##_Get, &varName##_Set } }
#define MSGPI_BOOL_VAL_SETTING(varName, id, propName, propDescription, propEditable, defValue) \
   static int varName##_Val; \
   static int varName##_Get() { return varName##_Val; } \
   static void varName##_Set(int v) { varName##_Val = v; } \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_BOOL, .boolDef = { defValue?1:0, &varName##_Get, &varName##_Set } }
#define MSGPI_STRING_VAL_SETTING(varName, id, propName, propDescription, propEditable, defValue, bufferSize) \
   static char* varName##_Val = new char[bufferSize]; \
   static const char* varName##_Get() { return varName##_Val; } \
   static void varName##_Set(const char* v) { snprintf(varName##_Val, bufferSize, "%s", v); } \
   static MsgSettingDef varName { .propId=id, .name=propName, .description=propDescription, .isUserEditable=propEditable?1:0, .type=MSGPI_SETTING_TYPE_STRING, .stringDef = { defValue, &varName##_Get, &varName##_Set } }

typedef struct MsgPluginAPI
{
   // Messaging
   unsigned int(MSGPIAPI* GetPluginEndpoint)(const char* id);
   void(MSGPIAPI* GetEndpointInfo)(const uint32_t endpointId, MsgEndpointInfo* info);
   unsigned int (MSGPIAPI* GetMsgID)(const char* name_space, const char* name);
   void (MSGPIAPI *SubscribeMsg)(const uint32_t endpointId, const unsigned int msgId, const msgpi_msg_callback callback, void* userData);
   void (MSGPIAPI *UnsubscribeMsg)(const unsigned int msgId, const msgpi_msg_callback callback);
   void (MSGPIAPI* BroadcastMsg)(const uint32_t endpointId, const unsigned int msgId, void* data);
   void (MSGPIAPI* SendMsg)(const uint32_t endpointId, const unsigned int msgId, const uint32_t targetEndpointId, void* data);
   void (MSGPIAPI* ReleaseMsgID)(const unsigned int msgId);
   // Setting
   void(MSGPIAPI* RegisterSetting)(const uint32_t endpointId, MsgSettingDef* settingDef); // Register a setting that the host will initialize either from a previously persisted value or the default
   void(MSGPIAPI* SaveSetting)(const uint32_t endpointId, MsgSettingDef* settingDef); // Request the host to persist a setting
   // Threading
   void (MSGPIAPI *RunOnMainThread)(const double delayInS, const msgpi_timer_callback callback, void* userData);
} MsgPluginAPI;
