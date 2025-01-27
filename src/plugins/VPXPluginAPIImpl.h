// license:GPLv3+

#pragma once

#include "VPXPlugin.h"
#include "core/DynamicScript.h"

class VPXPluginAPIImpl
{
public:
   static VPXPluginAPIImpl& GetInstance();

   const VPXPluginAPI& getAPI() const { return m_api; };
   unsigned int GetVPXEndPointId() const { return m_vpxEndpointId; }
   unsigned int GetPinMameEndPointId() const { return m_pinMameEndpointId; }

   unsigned int GetMsgID(const char* name_space, const char* name) const { return MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(name_space, name); };
   void ReleaseMsgID(const unsigned int msgId) const { MsgPluginManager::GetInstance().GetMsgAPI().ReleaseMsgID(msgId); };

   void BroadcastVPXMsg(const unsigned int msgId, void* data) const { MsgPluginManager::GetInstance().GetMsgAPI().BroadcastMsg(m_vpxEndpointId, msgId, data); };
   void BroadcastPinMameMsg(const unsigned int msgId, void* data) const { MsgPluginManager::GetInstance().GetMsgAPI().BroadcastMsg(m_pinMameEndpointId, msgId, data); };

   string ApplyScriptCOMObjectOverrides(string& script) const;
   IDispatch* CreateCOMPluginObject(const string& pluginId, const string& classId);

   // Helpers method for transitionning from COM object to plugins
   void PinMameOnStart();

private:
   VPXPluginAPIImpl();

   // VPX API
   unsigned int m_vpxEndpointId;
   static void OnGetVPXPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   VPXPluginAPI m_api;

   static void GetTableInfo(VPXTableInfo* info);

   static float GetOption(const char* pageId, const char* optionId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step, const float defaultValue, const VPXPluginAPI::OptionUnit unit, const char** values);
   static void* PushNotification(const char* msg, const unsigned int lengthMs);
   static void UpdateNotification(const void* handle, const char* msg, const unsigned int lengthMs);

   static void DisableStaticPrerendering(const BOOL disable);
   static void GetActiveViewSetup(VPXViewSetupDef* view);
   static void SetActiveViewSetup(VPXViewSetupDef* view);

   static void SetCOMObjectOverride(const char* className, const char* pluginId, const char* classId);
   struct ScriptCOMObjectOverride
   {
      const char* className;
      const char* pluginId;
      const char* classId;
   };
   std::vector<ScriptCOMObjectOverride> m_scriptCOMObjectOverrides;

   // Scriptable plugin API
   static void OnGetScriptablePluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void RegisterScriptClass(ScriptClassDef* classDef);
   static void RegisterScriptTypeAlias(const char* name, const char* aliasedType);
   static void RegisterScriptArray(ScriptArrayDef *arrayDef);
   DynamicTypeLibrary m_dynamicTypeLibrary;
   ScriptablePluginAPI m_scriptableApi;

   // Controller bridge
   unsigned int m_pinMameEndpointId;
   unsigned int m_getRenderDmdMsgId;
   unsigned int m_getIdentifyDmdMsgId;
   static void PinMameOnEnd(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetRenderDMD(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetIdentifyDMD(const unsigned int msgId, void* userData, void* msgData);
};
