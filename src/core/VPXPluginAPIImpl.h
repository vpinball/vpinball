// license:GPLv3+

#pragma once

#include "plugins/VPXPlugin.h"
#include "plugins/ScriptablePlugin.h"
#include "plugins/LoggingPlugin.h"
#include "core/DynamicScript.h"
#include "unordered_dense.h"

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
   void BroadcastVPXMsg(const unsigned int msgId, void* data) const { MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().BroadcastMsg(m_vpxPlugin->m_endpointId, msgId, data); }

   static unsigned int GetMsgID(const char* name_space, const char* name) { return MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(name_space, name); }
   static void ReleaseMsgID(const unsigned int msgId) { MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().ReleaseMsgID(msgId); }

   string ApplyScriptCOMObjectOverrides(const string& script) const;
   IDispatch* CreateCOMPluginObject(const string& classId);

   std::shared_ptr<BaseTexture> GetTexture(VPXTexture texture) const;

   struct PluginSetting
   {
      string pluginId;
      VPX::Properties::PropertyRegistry::PropId propId;
      MsgSettingDef* setting;
   };
   const vector<PluginSetting>& GetPluginSettings() const { return m_pluginSettings; }

   void OnGameStart();
   void UpdateDMDSource(Flasher* flasher, bool isAdd);
   void OnGameEnd();

private:
   VPXPluginAPIImpl();

   // VPX API
   std::shared_ptr<MsgPI::MsgPlugin> m_vpxPlugin;
   static void OnGetVPXPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   VPXPluginAPI m_api;
   const std::thread::id m_apiThread;

   static void GetVpxInfo(VPXInfo* info);
   static void GetTableInfo(VPXTableInfo* info);

   static unsigned int PushNotification(const char* msg, const int lengthMs);
   static void UpdateNotification(const unsigned int handle, const char* msg, const int lengthMs);

   static void DisableStaticPrerendering(const BOOL disable);
   static void GetActiveViewSetup(VPXViewSetupDef* view);
   static void SetActiveViewSetup(VPXViewSetupDef* view);

   static void SetActionState(const VPXAction actionId, const int isPressed);
   static void SetNudgeState(const int stateMask, const float nudgeAccelerationX, const float nudgeAccelerationY); // Bit 0 = override state
   static void SetPlungerState(const int stateMask, const float plungerPos, const float plungerSpeed); // Bit 0 = override state, bit 1 = hasSpeedSensor
   ankerl::unordered_dense::map<VPXAction, std::pair<unsigned int, int>> m_actionMap;

   static VPXTexture CreateTexture(uint8_t* rawData, int size);
   static void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image);
   static VPXTextureInfo* GetTextureInfo(VPXTexture texture);
   static void DeleteTexture(VPXTexture texture);

   // Plugin settings
   void UpdateSetting(const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef);
   vector<PluginSetting> m_pluginSettings;

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
   static ScriptClassDef* GetClassDef(const char* typeName);

   ankerl::unordered_dense::map<string, const ScriptClassDef*> m_scriptCOMObjectOverrides;
   DynamicTypeLibrary m_dynamicTypeLibrary;
   ScriptablePluginAPI m_scriptableApi;

   // Contribute VPX script controlled DMD through controller plugin API
   unsigned int m_onDisplaySrcChg = 0;
   vector<Flasher*> m_dmdSources;
   static void ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData);
   static DisplayFrame ControllerOnGetRenderDMD(const CtlResId id);
};
