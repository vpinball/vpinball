#pragma once

#include "DMDUtil/DMDUtil.h"

namespace VP {

class DMDWindow
{
public:
   DMDWindow(const string& szTitle);
   ~DMDWindow();

   void AttachDMD(DMDUtil::DMD* pDMD, int width, int height);
   void DetachDMD();
   bool IsDMDAttached() const { return m_attached; }
   void Show();
   void Hide();

private:
   static DisplayFrame GetRenderFrame(const CtlResId id);
   static void onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData);

   static int s_instanceId;
   static ankerl::unordered_dense::map<uint32_t, DMDWindow*> s_dmdWindowMap;
   uint32_t m_dmdId;
   string m_szTitle;
   DMDUtil::DMD* m_pDMD;
   DMDUtil::RGB24DMD* m_pRGB24DMD;
   bool m_attached;
   unsigned int m_frameId;
   unsigned char* m_lastFrameBuffer;
   size_t m_lastFrameSize;
   MsgPluginAPI* m_pMsgApi;
   std::shared_ptr<MsgPlugin> m_plugin;
   uint32_t m_endpointId;
   unsigned int m_onDmdSrcChangedId;
   unsigned int m_getDmdSrcId;
};

}