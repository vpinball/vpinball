// license:GPLv3+

#pragma once

#include "plugins/VPXPlugin.h"
#include "plugins/ScriptablePlugin.h"
#include "plugins/LoggingPlugin.h"
#include "core/DynamicScript.h"
#include "robin_hood.h"

// VPX serves as a plugin host, using the generic messaging plugin API
// 
// It provides the following features:
// - Plugin host: discover & manage plugin lifecycle, offer core messaging API implementation
// - Shared logging: allow plugins to perform logging usiong a shared log
// - Scripting host: support plugins contributing scriptable objects
// - Expose part of VPX API to plugins
// - Share running table content like a normal controller with plugins
//
class VPXPluginAPIImpl
{
public:
   static VPXPluginAPIImpl& GetInstance();

   const VPXPluginAPI& getAPI() const { return m_api; }
   unsigned int GetVPXEndPointId() const { return m_vpxPlugin->m_endpointId; }
   void BroadcastVPXMsg(const unsigned int msgId, void* data) const { MsgPluginManager::GetInstance().GetMsgAPI().BroadcastMsg(m_vpxPlugin->m_endpointId, msgId, data); }

   unsigned int GetMsgID(const char* name_space, const char* name) const { return MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(name_space, name); }
   void ReleaseMsgID(const unsigned int msgId) const { MsgPluginManager::GetInstance().GetMsgAPI().ReleaseMsgID(msgId); }

   string ApplyScriptCOMObjectOverrides(string& script) const;
   IDispatch* CreateCOMPluginObject(const string& classId);

private:
   VPXPluginAPIImpl();

   // VPX API
   std::shared_ptr<MsgPlugin> m_vpxPlugin;
   static void OnGetVPXPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   VPXPluginAPI m_api;

   static void GetTableInfo(VPXTableInfo* info);

   static float GetOption(const char* pageId, const char* optionId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step, const float defaultValue, const VPXPluginAPI::OptionUnit unit, const char** values);
   static void* PushNotification(const char* msg, const unsigned int lengthMs);
   static void UpdateNotification(const void* handle, const char* msg, const unsigned int lengthMs);

   static void DisableStaticPrerendering(const BOOL disable);
   static void GetActiveViewSetup(VPXViewSetupDef* view);
   static void SetActiveViewSetup(VPXViewSetupDef* view);

   // Plugin logging API
   static void OnGetLoggingPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void PluginLog(unsigned int level, const char* message);
   LoggingPluginAPI m_loggingApi;

   // Scriptable plugin API
   static void OnGetScriptablePluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void RegisterScriptClass(ScriptClassDef* classDef);
   static void RegisterScriptTypeAlias(const char* name, const char* aliasedType);
   static void RegisterScriptArray(ScriptArrayDef *arrayDef);
   static void SubmitTypeLibrary();
   static void OnScriptError(unsigned int type, const char* message);
   static void SetCOMObjectOverride(const char* className, const ScriptClassDef* classDef);
   robin_hood::unordered_map<string, const ScriptClassDef*> m_scriptCOMObjectOverrides;
   DynamicTypeLibrary m_dynamicTypeLibrary;
   ScriptablePluginAPI m_scriptableApi;

   // Contribute VPX API through plugin API
   unsigned int m_getRenderDmdMsgId;
   unsigned int m_getIdentifyDmdMsgId;
   static void ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetRenderDMD(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetIdentifyDMD(const unsigned int msgId, void* userData, void* msgData);
};
