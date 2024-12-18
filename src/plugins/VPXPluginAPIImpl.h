// license:GPLv3+

#pragma once

class VPXPluginAPIImpl
{
public:
   static VPXPluginAPIImpl& GetInstance();

   const VPXPluginAPI& getAPI() const { return m_api; };
   unsigned int GetVPXEndPointId() const { return m_vpxEndpointId; }

   unsigned int GetMsgID(const char* name_space, const char* name) const { return MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(name_space, name); };
   void ReleaseMsgID(const unsigned int msgId) const { MsgPluginManager::GetInstance().GetMsgAPI().ReleaseMsgID(msgId); };

   void BroadcastVPXMsg(const unsigned int msgId, void* data) const { MsgPluginManager::GetInstance().GetMsgAPI().BroadcastMsg(m_vpxEndpointId, msgId, data); };
   void BroadcastPinMameMsg(const unsigned int msgId, void* data) const { MsgPluginManager::GetInstance().GetMsgAPI().BroadcastMsg(m_pinMameEndpointId, msgId, data); };

   // Helpers method for transitionning from COM object to plugins
   void PinMameOnStart();

private:
   VPXPluginAPIImpl();

   static void OnGetPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void PinMameOnEnd(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData);
   static void ControllerOnGetDMD(const unsigned int msgId, void* userData, void* msgData);

   unsigned int m_getRenderDmdMsgId;
   unsigned int m_getIdentifyDmdMsgId;
   
   unsigned int m_vpxEndpointId;
   unsigned int m_pinMameEndpointId;
   VPXPluginAPI m_api;
};
