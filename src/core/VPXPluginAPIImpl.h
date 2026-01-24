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
   ~VPXPluginAPIImpl();

   // VPX API
   std::shared_ptr<MsgPI::MsgPlugin> m_vpxPlugin;
   static void OnGetVPXPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   VPXPluginAPI m_api;
   const std::thread::id m_apiThread;
   const unsigned int m_getVPXAPIMsgId;
   const unsigned int m_onGameStartMsgId;
   const unsigned int m_onGameEndMsgId;

   static void MSGPIAPI GetVpxInfo(VPXInfo* info);
   static void MSGPIAPI GetTableInfo(VPXTableInfo* info);

   static unsigned int MSGPIAPI PushNotification(const char* msg, const int lengthMs);
   static void MSGPIAPI UpdateNotification(const unsigned int handle, const char* msg, const int lengthMs);

   static void MSGPIAPI DisableStaticPrerendering(const BOOL disable);
   static void MSGPIAPI GetActiveViewSetup(VPXViewSetupDef* view);
   static void MSGPIAPI SetActiveViewSetup(VPXViewSetupDef* view);

   static void MSGPIAPI SetActionState(const VPXAction actionId, const int isPressed);
   static void MSGPIAPI SetNudgeState(const int stateMask, const float nudgeAccelerationX, const float nudgeAccelerationY); // Bit 0 = override state
   static void MSGPIAPI SetPlungerState(const int stateMask, const float plungerPos, const float plungerSpeed); // Bit 0 = override state, bit 1 = hasSpeedSensor
   ankerl::unordered_dense::map<VPXAction, std::pair<unsigned int, int>> m_actionMap;

   static double MSGPIAPI GetGameTime();

   static VPXTexture MSGPIAPI CreateTexture(uint8_t* rawData, int size);
   static void MSGPIAPI UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image);
   static VPXTextureInfo* MSGPIAPI GetTextureInfo(VPXTexture texture);
   static void MSGPIAPI DeleteTexture(VPXTexture texture);

   // Plugin settings
   void UpdateSetting(const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef);
   vector<PluginSetting> m_pluginSettings;

   // Plugin logging API
   static void MSGPIAPI OnGetLoggingPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void MSGPIAPI PluginLog(unsigned int level, const char* message);

   LoggingPluginAPI m_loggingApi;
   const unsigned int m_getLoggingAPIMsgId;

   // Scriptable plugin API
   static void MSGPIAPI OnGetScriptablePluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void MSGPIAPI RegisterScriptClass(ScriptClassDef* classDef);
   static void MSGPIAPI RegisterScriptTypeAlias(const char* name, const char* aliasedType);
   static void MSGPIAPI RegisterScriptArray(ScriptArrayDef* arrayDef);
   static void MSGPIAPI SubmitTypeLibrary();
   static void MSGPIAPI OnScriptError(unsigned int type, const char* message);
   static void MSGPIAPI SetCOMObjectOverride(const char* className, const ScriptClassDef* classDef);
   static ScriptClassDef* MSGPIAPI GetClassDef(const char* typeName);

   ankerl::unordered_dense::map<string, const ScriptClassDef*> m_scriptCOMObjectOverrides;
   DynamicTypeLibrary m_dynamicTypeLibrary;
   ScriptablePluginAPI m_scriptableApi;
   const unsigned int m_getScriptingAPIMsgId;

   // Contribute VPX script controlled DMD through controller plugin API
   vector<Flasher*> m_dmdSources;
   static void ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData);
   static DisplayFrame ControllerOnGetRenderDMD(const CtlResId id);
   const unsigned int m_onDisplaySrcChgMsgId;
   const unsigned int m_onDisplayGetSrcMsgId;
};
