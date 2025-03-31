#include "core/stdafx.h"

#include "DMDWindow.h"

namespace VP {

int DMDWindow::s_instanceId = 0;

void DMDWindow::onGetIdentifyDMD(const unsigned int eventId, void* userData, void* msgData)
{
}

void DMDWindow::onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   DMDWindow* pDMDWindow = (DMDWindow*)userData;

   if (!pDMDWindow->m_attached)
      return;

   GetDmdSrcMsg& msg = *static_cast<GetDmdSrcMsg*>(msgData);

   msg.entries[msg.count].id = pDMDWindow->m_dmdId;
   msg.entries[msg.count].format = CTLPI_GETDMD_FORMAT_SRGB888;
   msg.entries[msg.count].width = pDMDWindow->m_pRGB24DMD->GetWidth();
   msg.entries[msg.count].height = pDMDWindow->m_pRGB24DMD->GetHeight();
   msg.count++;
}

void DMDWindow::onGetRenderDMD(const unsigned int eventId, void* userData, void* msgData)
{
   DMDWindow* pDMDWindow = (DMDWindow*)userData;

   if (!pDMDWindow->m_attached)
      return;

   const UINT8* pRGB24Data = pDMDWindow->m_pRGB24DMD->GetData();
   if (pRGB24Data) {
      GetDmdMsg& getDmdMsg = *static_cast<GetDmdMsg*>(msgData);

      getDmdMsg.frameId = pDMDWindow->m_frameId++;
      getDmdMsg.frame = (unsigned char*)pRGB24Data;
   }
}

DMDWindow::DMDWindow(const string& szTitle)
{
   m_pDMD = nullptr;
   m_pRGB24DMD = nullptr;
   m_attached = false;
   m_frameId = 0;

   m_pMsgPluginAPI = (MsgPluginAPI*)&MsgPluginManager::GetInstance().GetMsgAPI();

   m_szTitle = "DMDWindow_" + szTitle + "_" + std::to_string(s_instanceId++);
   m_plugin = MsgPluginManager::GetInstance().RegisterPlugin(m_szTitle.c_str(), "VPX", "Visual Pinball X", "", "", "https://github.com/vpinball/vpinball",
      [](const uint32_t pluginId, const MsgPluginAPI* api) {},
      []() {});
   m_plugin->Load(m_pMsgPluginAPI);

   m_endpointId = m_plugin->m_endpointId;
   m_dmdId = { m_endpointId, 1 };

   m_getDmdSrcId = m_pMsgPluginAPI->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG);
   m_getRenderDmdId = m_pMsgPluginAPI->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   m_getIdentifyDmdId = m_pMsgPluginAPI->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_IDENTIFY_MSG);
   m_onDmdSrcChangedId = m_pMsgPluginAPI->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG);

   m_pMsgPluginAPI->SubscribeMsg(m_endpointId, m_getDmdSrcId, onGetRenderDMDSrc, this);
   m_pMsgPluginAPI->SubscribeMsg(m_endpointId, m_getRenderDmdId, onGetRenderDMD, this);
   m_pMsgPluginAPI->SubscribeMsg(m_endpointId, m_getIdentifyDmdId, onGetIdentifyDMD, this);
}

DMDWindow::~DMDWindow()
{
   if (m_pDMD) {
      PLOGE.printf("Destructor called without first detaching DMD.");
   }

   m_pMsgPluginAPI->UnsubscribeMsg(m_getDmdSrcId, onGetRenderDMDSrc);
   m_pMsgPluginAPI->UnsubscribeMsg(m_getRenderDmdId, onGetRenderDMD);
   m_pMsgPluginAPI->UnsubscribeMsg(m_getIdentifyDmdId, onGetIdentifyDMD);

   m_pMsgPluginAPI->ReleaseMsgID(m_getDmdSrcId);
   m_pMsgPluginAPI->ReleaseMsgID(m_getRenderDmdId);
   m_pMsgPluginAPI->ReleaseMsgID(m_getIdentifyDmdId);
   m_pMsgPluginAPI->ReleaseMsgID(m_onDmdSrcChangedId);

   m_plugin->Unload();
}

void DMDWindow::AttachDMD(DMDUtil::DMD* pDMD, int width, int height)
{
   if (m_pDMD) {
      PLOGE.printf("Unable to attach DMD: message=Detach existing DMD first.");
      return;
   }

   if (!pDMD) {
      PLOGE.printf("Unable to attach DMD: message=DMD is null.");
      return;
   }

   PLOGI.printf("Attaching DMD: width=%d, height=%d", width, height);

   m_pRGB24DMD = pDMD->CreateRGB24DMD(width, height);

   if (m_pRGB24DMD) {
      m_pDMD = pDMD;
      m_attached = true;

      m_pMsgPluginAPI->RunOnMainThread(0, [](void* userData) {
         DMDWindow* pDMDWindow = static_cast<DMDWindow*>(userData);
         pDMDWindow->m_pMsgPluginAPI->BroadcastMsg(pDMDWindow->m_endpointId, pDMDWindow->m_onDmdSrcChangedId, nullptr);
      }, this);
   }
   else {
      PLOGE.printf("Failed to attach DMD: message=Failed to create RGB24DMD.");
   }
}

void DMDWindow::DetachDMD()
{
   if (!m_pDMD) {
      PLOGE.printf("Unable to detach DMD: message=No DMD attached.");
      return;
   }

   m_attached = false;
   m_frameId = 0;

   if (m_pRGB24DMD) {
      PLOGI.printf("Detaching DMD");
      m_pDMD->DestroyRGB24DMD(m_pRGB24DMD);
      m_pRGB24DMD = nullptr;
   }

   m_pDMD = nullptr;

   m_pMsgPluginAPI->RunOnMainThread(0, [](void* userData) {
      DMDWindow* pDMDWindow = static_cast<DMDWindow*>(userData);
      pDMDWindow->m_pMsgPluginAPI->BroadcastMsg(pDMDWindow->m_endpointId, pDMDWindow->m_onDmdSrcChangedId, nullptr);
   }, this);
}

void DMDWindow::Show()
{
   PLOGW.printf("DMDWindow::Show() not implemented: title=%s", m_szTitle.c_str());
}

void DMDWindow::Hide()
{
   PLOGW.printf("DMDWindow::Hide() not implemented: title=%s", m_szTitle.c_str());
}

}