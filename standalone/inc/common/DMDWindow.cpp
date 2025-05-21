#include "core/stdafx.h"

#include "DMDWindow.h"

namespace VP {

int DMDWindow::s_instanceId = 0;
ankerl::unordered_dense::map<uint32_t, DMDWindow*> DMDWindow::s_dmdWindowMap;

DisplayFrame DMDWindow::GetRenderFrame(const CtlResId id)
{
   auto it = DMDWindow::s_dmdWindowMap.find(id.resId);
   if (it == DMDWindow::s_dmdWindowMap.end())
      return { 0, nullptr };

   DMDWindow* pDMDWindow = it->second;

   if (!pDMDWindow->IsDMDAttached())
       return { 0, nullptr };

   const UINT8* pRGB24Data = pDMDWindow->m_pRGB24DMD->GetData();
   if (pRGB24Data) {
      memcpy(pDMDWindow->m_lastFrameBuffer, pRGB24Data, pDMDWindow->m_lastFrameSize);
      pDMDWindow->m_frameId++;
   }

   return { pDMDWindow->m_frameId, pDMDWindow->m_lastFrameBuffer };
}

void DMDWindow::onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   DMDWindow* pDMDWindow = (DMDWindow*)userData;

   if (!pDMDWindow->m_attached)
      return;

   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);

   if (msg.count < msg.maxEntryCount) {
      msg.entries[msg.count] = { 0 };
      msg.entries[msg.count].id = { pDMDWindow->m_endpointId, pDMDWindow->m_dmdId };
      msg.entries[msg.count].frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888;
      msg.entries[msg.count].width = pDMDWindow->m_pRGB24DMD->GetWidth();
      msg.entries[msg.count].height = pDMDWindow->m_pRGB24DMD->GetHeight();
      msg.entries[msg.count].GetRenderFrame = &DMDWindow::GetRenderFrame;
   }
   msg.count++;
}

DMDWindow::DMDWindow(const string& szTitle)
{
   m_dmdId = ++s_instanceId;

   m_szTitle = "DMDWindow_" + szTitle + "_" + std::to_string(m_dmdId);

   m_pDMD = nullptr;
   m_pRGB24DMD = nullptr;
   m_attached = false;
   m_frameId = 0;
   m_lastFrameBuffer = nullptr;
   m_lastFrameSize = 0;

   m_pMsgApi = (MsgPluginAPI*)&MsgPluginManager::GetInstance().GetMsgAPI();

   m_plugin = MsgPluginManager::GetInstance().RegisterPlugin(m_szTitle.c_str(), "VPX", "Visual Pinball X", "", "", "https://github.com/vpinball/vpinball",
      [](const uint32_t pluginId, const MsgPluginAPI* api) {},
      []() {});
   m_plugin->Load(m_pMsgApi);

   m_endpointId = m_plugin->m_endpointId;

   m_onDmdSrcChangedId = m_pMsgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_getDmdSrcId = m_pMsgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);

   m_pMsgApi->SubscribeMsg(m_endpointId, m_getDmdSrcId, onGetRenderDMDSrc, this);
}

DMDWindow::~DMDWindow()
{
   if (m_pDMD) {
      PLOGE.printf("Destructor called without first detaching DMD.");
   }

   m_pMsgApi->UnsubscribeMsg(m_getDmdSrcId, onGetRenderDMDSrc);

   m_pMsgApi->ReleaseMsgID(m_getDmdSrcId);
   m_pMsgApi->ReleaseMsgID(m_onDmdSrcChangedId);

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

      m_lastFrameSize = width * height * 3;
      m_lastFrameBuffer = static_cast<unsigned char*>(malloc(m_lastFrameSize));
      memset(m_lastFrameBuffer, 0, m_lastFrameSize);

      s_dmdWindowMap[m_dmdId] = this;

      m_pMsgApi->RunOnMainThread(0, [](void* userData) {
         DMDWindow* pDMDWindow = static_cast<DMDWindow*>(userData);
         pDMDWindow->m_pMsgApi->BroadcastMsg(pDMDWindow->m_endpointId, pDMDWindow->m_onDmdSrcChangedId, nullptr);
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

   s_dmdWindowMap.erase(m_dmdId);

   if (m_pRGB24DMD) {
      PLOGI.printf("Detaching DMD");
      m_pDMD->DestroyRGB24DMD(m_pRGB24DMD);
      m_pRGB24DMD = nullptr;
   }

   m_pDMD = nullptr;

   if (m_lastFrameBuffer) {
      free(m_lastFrameBuffer);
      m_lastFrameBuffer = nullptr;
      m_lastFrameSize = 0;
   }

   m_pMsgApi->RunOnMainThread(0, [](void* userData) {
      DMDWindow* pDMDWindow = static_cast<DMDWindow*>(userData);
      pDMDWindow->m_pMsgApi->BroadcastMsg(pDMDWindow->m_endpointId, pDMDWindow->m_onDmdSrcChangedId, nullptr);
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