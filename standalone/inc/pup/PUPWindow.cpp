#include "core/stdafx.h"

#include "PUPWindow.h"
#include "PUPScreen.h"

class PUPScreen;

int PUPWindow::Render(VPXRenderContext2D* const renderCtx, void* context)
{ 
   return static_cast<PUPWindow*>(context)->Render(renderCtx);
}

void PUPWindow::OnGetRenderer(const unsigned int eventId, void* context, void* msgData)
{
   PUPWindow* me = static_cast<PUPWindow*>(context);
   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);
   if (msg->window == me->m_anciliaryWindow) {
      if (msg->count < msg->maxEntryCount)
      {
         msg->entries[msg->count].name = me->m_szTitle.c_str();
         msg->entries[msg->count].description = me->m_szDescription.c_str();
         msg->entries[msg->count].context = me;
         msg->entries[msg->count].Render = &PUPWindow::Render;
      }
      msg->count++;
   }
}

PUPWindow::PUPWindow(const string& szTitle, PUPScreen* pScreen, VPXAnciliaryWindow anciliaryWindow)
{
   m_szTitle = szTitle;
   m_pScreen = pScreen;
   m_anciliaryWindow = anciliaryWindow;

   m_szDescription = "Renderer for PuP: " + szTitle;

   m_pMsgApi = (MsgPluginAPI*)&MsgPluginManager::GetInstance().GetMsgAPI();

   m_plugin = MsgPluginManager::GetInstance().RegisterPlugin(m_szTitle.c_str(), "VPX", "Visual Pinball X", "", "", "https://github.com/vpinball/vpinball",
      [](const uint32_t pluginId, const MsgPluginAPI* api) {},
      []() {});
   m_plugin->Load(m_pMsgApi);

   m_endpointId = m_plugin->m_endpointId;

   unsigned int getVpxApiId = m_pMsgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   m_pMsgApi->BroadcastMsg(m_endpointId, getVpxApiId, &m_pVpxApi);
   m_pMsgApi->ReleaseMsgID(getVpxApiId);

   m_pMsgApi->SubscribeMsg(m_endpointId, m_getAuxRendererId = m_pMsgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), &PUPWindow::OnGetRenderer, this);
   m_pMsgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId = m_pMsgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);
}

PUPWindow::~PUPWindow()
{
   m_pMsgApi->UnsubscribeMsg(m_getAuxRendererId, OnGetRenderer);
   m_pMsgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_pMsgApi->ReleaseMsgID(m_getAuxRendererId);
   m_pMsgApi->ReleaseMsgID(m_onAuxRendererChgId);

   SDL_DestroyRenderer(m_pRenderer);
   SDL_DestroySurface(m_pSurface);

   m_pVpxApi->DeleteTexture(m_vpxTexture);

   m_plugin->Unload();
}

int PUPWindow::Render(VPXRenderContext2D* const renderCtx)
{
   renderCtx->srcWidth = renderCtx->outWidth;
   renderCtx->srcHeight = renderCtx->outHeight;

   if (!m_pRenderer) {
      m_pSurface = SDL_CreateSurface(renderCtx->srcWidth, renderCtx->srcHeight, SDL_PIXELFORMAT_RGBA32);
      if (m_pSurface) {
         m_pRenderer = SDL_CreateSoftwareRenderer(m_pSurface);
         if (m_pRenderer)
            m_pScreen->Init(m_pRenderer);
         else {
            SDL_DestroySurface(m_pSurface);
            m_pSurface = nullptr;

            return false;
         }
      }
      else
         return false;
   }

   m_pScreen->SetSize(static_cast<int>(renderCtx->outWidth), static_cast<int>(renderCtx->outHeight));

   SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
   SDL_RenderClear(m_pRenderer);
   m_pScreen->Render();
   SDL_RenderPresent(m_pRenderer);

   SDL_LockSurface(m_pSurface);
   m_pVpxApi->UpdateTexture(&m_vpxTexture, m_pSurface->w, m_pSurface->h, VPXTextureFormat::VPXTEXFMT_sRGBA, static_cast<uint8_t*>(m_pSurface->pixels));
   SDL_UnlockSurface(m_pSurface);

   int texWidth, texHeight;
   m_pVpxApi->GetTextureInfo(m_vpxTexture, &texWidth, &texHeight);

   SDL_Rect rect = m_pScreen->GetRect();
   renderCtx->DrawImage(renderCtx, m_vpxTexture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f,
      static_cast<float>(texWidth), static_cast<float>(texHeight), rect.x, rect.y, rect.w, rect.h);

   return true;
}

void PUPWindow::Show()
{
   PLOGW.printf("PUPWindow::Show() not implemented: title=%s", m_szTitle.c_str());
}

void PUPWindow::Hide()
{
   PLOGW.printf("PUPWindow::Hide() not implemented: title=%s", m_szTitle.c_str());
}
