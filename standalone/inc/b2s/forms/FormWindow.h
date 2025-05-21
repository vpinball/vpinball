#pragma once

#include "../../common/RendererGraphics.h"

#include "Form.h"

class FormWindow
{
public:
   FormWindow(const string& szTitle, Form* pForm, VPXAnciliaryWindow anciliaryWindow);
   ~FormWindow();

   int Render(VPXRenderContext2D* const renderCtx);

   void Show();
   void Hide();

private:
   static void OnGetRenderer(const unsigned int eventId, void* context, void* msgData);
   static int Render(VPXRenderContext2D* const renderCtx, void* context);

   string m_szTitle;
   string m_szDescription;
   Form* m_pForm = nullptr;
   MsgPluginAPI* m_pMsgApi = nullptr;
   VPXPluginAPI* m_pVpxApi = nullptr;
   std::shared_ptr<MsgPlugin> m_plugin;
   uint32_t m_endpointId;
   unsigned int m_getAuxRendererId;
   unsigned int m_onAuxRendererChgId;
   SDL_Surface* m_pSurface = nullptr;
   SDL_Renderer* m_pRenderer = nullptr;
   VPXTexture m_vpxTexture = nullptr;
   VPXAnciliaryWindow m_anciliaryWindow;
   VP::RendererGraphics* m_pGraphics = nullptr;
};
