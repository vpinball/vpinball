#include "core/stdafx.h"

#include "FormWindow.h"
#include "Form.h"

int FormWindow::Render(VPXRenderContext2D* const renderCtx, void* context)
{
   return static_cast<FormWindow*>(context)->Render(renderCtx);
}

void FormWindow::OnGetRenderer(const unsigned int eventId, void* context, void* msgData)
{
   FormWindow* me = static_cast<FormWindow*>(context);
   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);
   if (msg->window == me->m_anciliaryWindow) {
      if (msg->count < msg->maxEntryCount) {
         msg->entries[msg->count].id = me->m_szTitle.c_str();
         msg->entries[msg->count].name = me->m_szTitle.c_str();
         msg->entries[msg->count].description = me->m_szDescription.c_str();
         msg->entries[msg->count].context = me;
         msg->entries[msg->count].Render = &FormWindow::Render;
      }
      msg->count++;
   }
}

FormWindow::FormWindow(const string& szTitle, Form* pForm, VPXAnciliaryWindow anciliaryWindow)
{
   m_szTitle = szTitle;
   m_pForm = pForm;
   m_anciliaryWindow = anciliaryWindow;

   m_szDescription = "Renderer for B2S: " + szTitle;

   m_pMsgApi = (MsgPluginAPI*)&MsgPluginManager::GetInstance().GetMsgAPI();

   m_plugin = MsgPluginManager::GetInstance().RegisterPlugin(m_szTitle.c_str(), "VPX", "Visual Pinball X", "", "", "https://github.com/vpinball/vpinball",
      [](const uint32_t pluginId, const MsgPluginAPI* api) {},
      []() {});
   m_plugin->Load(m_pMsgApi);

   m_endpointId = m_plugin->m_endpointId;

   unsigned int getVpxApiId = m_pMsgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   m_pMsgApi->BroadcastMsg(m_endpointId, getVpxApiId, &m_pVpxApi);
   m_pMsgApi->ReleaseMsgID(getVpxApiId);

   m_pMsgApi->SubscribeMsg(m_endpointId, m_getAuxRendererId = m_pMsgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), &FormWindow::OnGetRenderer, this);
   m_pMsgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId = m_pMsgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);

   m_pGraphics = nullptr;
}

FormWindow::~FormWindow()
{
   m_pMsgApi->UnsubscribeMsg(m_getAuxRendererId, OnGetRenderer);
   m_pMsgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_pMsgApi->ReleaseMsgID(m_getAuxRendererId);
   m_pMsgApi->ReleaseMsgID(m_onAuxRendererChgId);

   SDL_DestroyRenderer(m_pRenderer);
   SDL_DestroySurface(m_pSurface);

   m_pVpxApi->DeleteTexture(m_vpxTexture);

   m_plugin->Unload();

   delete m_pGraphics;
}

int FormWindow::Render(VPXRenderContext2D* const renderCtx)
{
   renderCtx->srcWidth = renderCtx->outWidth;
   renderCtx->srcHeight = renderCtx->outHeight;

   if (!m_pRenderer) {
      m_pSurface = SDL_CreateSurface(renderCtx->srcWidth, renderCtx->srcHeight, SDL_PIXELFORMAT_RGBA32);
      if (m_pSurface) {
         m_pRenderer = SDL_CreateSoftwareRenderer(m_pSurface);
         if (m_pRenderer) {
            m_pGraphics = new VP::RendererGraphics(m_pRenderer);
            m_pForm->SetGraphics(m_pGraphics);
         }
         else {
            SDL_DestroySurface(m_pSurface);
            m_pSurface = nullptr;

            return false;
         }
      }
      else
         return false;
   }

   SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
   SDL_RenderClear(m_pRenderer);
   bool update = m_pForm->Render();
   SDL_RenderPresent(m_pRenderer);

   if (update) {
      SDL_LockSurface(m_pSurface);
      m_pVpxApi->UpdateTexture(&m_vpxTexture, m_pSurface->w, m_pSurface->h, VPXTextureFormat::VPXTEXFMT_sRGBA8, static_cast<uint8_t*>(m_pSurface->pixels));
      SDL_UnlockSurface(m_pSurface);
   }

   int texWidth, texHeight;
   m_pVpxApi->GetTextureInfo(m_vpxTexture, &texWidth, &texHeight);

   SDL_Rect rect = m_pForm->GetRect();
   renderCtx->DrawImage(renderCtx, m_vpxTexture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f,
      static_cast<float>(texWidth), static_cast<float>(texHeight), rect.x, rect.y, rect.w, rect.h);

   return true;
}

void FormWindow::Show()
{
   PLOGW.printf("FormWindow::Show() not implemented: title=%s", m_szTitle.c_str());
}

void FormWindow::Hide()
{
   PLOGW.printf("FormWindow::Hide() not implemented: title=%s", m_szTitle.c_str());
}
