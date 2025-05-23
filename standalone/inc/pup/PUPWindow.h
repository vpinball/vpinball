#pragma once

#include "PUPManager.h"

class PUPScreen;

class PUPWindow
{
public:
   PUPWindow(const string& szTitle, PUPScreen* pScreen, VPXAnciliaryWindow anciliaryWindow);
   ~PUPWindow();

   int Render(VPXRenderContext2D* const renderCtx);

   void Show();
   void Hide();

private:
   static void OnGetRenderer(const unsigned int eventId, void* context, void* msgData);
   static int Render(VPXRenderContext2D* const renderCtx, void* context);

   PUPScreen* m_pScreen = nullptr;
   string m_szTitle;
   string m_szDescription;
   MsgPluginAPI* m_pMsgApi = nullptr;
   VPXPluginAPI* m_pVpxApi = nullptr;
   std::shared_ptr<MsgPlugin> m_plugin;
   uint32_t m_endpointId;
   unsigned int m_getAuxRendererId;
   unsigned int m_onAuxRendererChgId;
   VPXAnciliaryWindow m_anciliaryWindow;
   bool m_init = false;
};