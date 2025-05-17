#pragma once

#include "../common/Window.h"
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
   static void onGetIdentifyDMD(const unsigned int eventId, void* userData, void* msgData);
   static void onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData);
   static void onGetRenderDMD(const unsigned int eventId, void* userData, void* msgData);

   string m_szTitle;
   DMDUtil::DMD* m_pDMD;
   DMDUtil::RGB24DMD* m_pRGB24DMD;
   bool m_attached;

   std::shared_ptr<MsgPlugin> m_plugin;
   MsgPluginAPI* m_pMsgPluginAPI;

   uint32_t m_endpointId;
   unsigned int m_getDmdSrcId;
   unsigned int m_getRenderDmdId;
   unsigned int m_getIdentifyDmdId;
   unsigned int m_onDmdSrcChangedId;

   CtlResId m_dmdId;
   int m_frameId;
   unsigned char* m_lastFrameBuffer;
   size_t m_lastFrameSize;

   static int s_instanceId;
};

}