#include "core/stdafx.h"

#include "Window.h"
#include "WindowManager.h"

namespace VP {

Window::Window(const string& szTitle, int z, int x, int y, int w, int h)
{
   m_szTitle = szTitle;
   m_z = z;

#ifdef __LIBVPINBALL__
   VPX::RenderOutput::OutputMode mode = VPX::RenderOutput::OutputMode::OM_EMBEDDED;
#else
   VPX::RenderOutput::OutputMode mode = VPX::RenderOutput::OutputMode::OM_WINDOW;
#endif

   m_pRenderOutput = new VPX::RenderOutput(m_szTitle, g_pplayer->m_ptable->m_settings, Settings::Standalone, m_szTitle, mode, x, y, w, h);

   m_pWindow = nullptr;
   m_pEmbeddedWindow = nullptr;

   m_pRenderer = NULL;
   m_pSurface = NULL;
   m_pTexture = nullptr;
   m_visible = false;
   m_init = false;

   VP::WindowManager::GetInstance()->RegisterWindow(this);
}

bool Window::Init()
{
   if (!m_pRenderOutput) {
      PLOGE.printf("Failed to create render output: title=%s", m_szTitle.c_str());
      return false;
   }

   VPX::RenderOutput::OutputMode mode = m_pRenderOutput->GetMode();
   int x = 0;
   int y = 0;
   int width = 0;
   int height = 0;

   switch (mode) {
      case VPX::RenderOutput::OutputMode::OM_DISABLED:
         PLOGE.printf("Render output disabled: title=%s", m_szTitle.c_str());
         return false;
      case VPX::RenderOutput::OutputMode::OM_EMBEDDED:
         m_pEmbeddedWindow = m_pRenderOutput->GetEmbeddedWindow();
         m_pEmbeddedWindow->GetPos(x, y);
         width = m_pEmbeddedWindow->GetWidth();
         height = m_pEmbeddedWindow->GetHeight();
         break;
      case VPX::RenderOutput::OutputMode::OM_WINDOW:
         m_pWindow = m_pRenderOutput->GetWindow();
         g_pplayer->m_renderer->m_renderDevice->AddWindow(m_pWindow);
         m_pWindow->Show(m_visible);
         m_pWindow->GetPos(x, y);
         width = m_pWindow->GetWidth();
         height = m_pWindow->GetHeight();
         break;
   }

   m_pRenderer = SDL_GetRenderer(g_pplayer->m_playfieldWnd->GetCore());

   if (!m_pRenderer)
      m_pRenderer = SDL_CreateRenderer(g_pplayer->m_playfieldWnd->GetCore(), NULL);

   if (!m_pRenderer) {
      m_pSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ABGR8888);
      m_pRenderer = SDL_CreateSoftwareRenderer(m_pSurface);
   }

   if (!m_pRenderer) {
      PLOGE.printf("Failed to create renderer: title=%s", m_szTitle.c_str());
      return false;
   }

   const char* pRendererName = SDL_GetRendererName(m_pRenderer);

   m_init = true;

   PLOGI.printf("Window initialized: title=%s, z=%d, visible=%d, mode=%d, size=%dx%d, pos=%d,%d, renderer=%s", m_szTitle.c_str(), m_z, m_visible, mode, width, height, x, y, pRendererName);

   return true;
}

Window::~Window()
{
   VP::WindowManager::GetInstance()->UnregisterWindow(this);

   if (m_pTexture)
      delete m_pTexture;

   if (m_pRenderOutput)
      delete m_pRenderOutput;

   if (m_pSurface)
      SDL_DestroySurface(m_pSurface);

   if (m_pRenderer && m_pRenderer != SDL_GetRenderer(g_pplayer->m_playfieldWnd->GetCore()))
      SDL_DestroyRenderer(m_pRenderer);
}

int Window::GetWidth()
{
   return m_pWindow ? m_pWindow->GetWidth() : m_pEmbeddedWindow ? m_pEmbeddedWindow->GetWidth() : 0;
}

int Window::GetHeight()
{
   return m_pWindow ? m_pWindow->GetHeight() : m_pEmbeddedWindow ? m_pEmbeddedWindow->GetHeight() : 0;
}

void Window::Show()
{
   if (m_visible)
      return;

   m_visible = true;

   if (m_init) {
      if (m_pWindow)
         m_pWindow->Show(true);

      PLOGI.printf("Window updated: title=%s, z=%d, visible=%d", m_szTitle.c_str(), m_z, m_visible);
   }
}

void Window::Hide()
{
   if (!m_visible)
      return;

   m_visible = false;

   if (m_pWindow)
      m_pWindow->Show(false);

   PLOGI.printf("Window updated: title=%s, z=%d, visible=%d", m_szTitle.c_str(), m_z, m_visible);
}

void Window::Render()
{
   if (!m_pTexture)
      return;

   Renderer* renderer = g_pplayer->m_renderer;
   RenderTarget* sceneRT = renderer->m_renderDevice->GetCurrentRenderTarget();
   RenderTarget* windowRT = nullptr;
   int windowX, windowY, windowW, windowH;

   if (m_pWindow)
   {
      m_pWindow->Show();
      windowRT = m_pWindow->GetBackBuffer();
      windowX = windowY = 0;
      windowW = windowRT->GetWidth();
      windowH = windowRT->GetHeight();
   }
   else if (m_pEmbeddedWindow)
   {
      m_pEmbeddedWindow->GetPos(windowX, windowY);
      windowRT = g_pplayer->m_playfieldWnd->GetBackBuffer();
      windowW = m_pEmbeddedWindow->GetWidth();
      windowH = m_pEmbeddedWindow->GetHeight();
      windowY = windowRT->GetHeight() - windowY - windowH;
   }

   if (windowRT == nullptr)
      return;

   const float rtAR = static_cast<float>(windowW) / static_cast<float>(windowH);
   const float spriteAR = static_cast<float>(m_pTexture->width()) / static_cast<float>(m_pTexture->height());
   const float pw = 2.f * (rtAR > spriteAR ? spriteAR / rtAR : 1.f) * static_cast<float>(windowW) / static_cast<float>(windowRT->GetWidth());
   const float ph = 2.f * (rtAR < spriteAR ? rtAR / spriteAR : 1.f) * static_cast<float>(windowH) / static_cast<float>(windowRT->GetHeight());
   const float px = static_cast<float>(windowX + windowW / 2) / static_cast<float>(windowRT->GetWidth()) * 2.f - 1.f - pw * 0.5f;
   const float py = static_cast<float>(windowY + windowH / 2) / static_cast<float>(windowRT->GetHeight()) * 2.f - 1.f - ph * 0.5f;
   const float sx = pw / m_pTexture->width();
   const float sy = ph / m_pTexture->height();

   renderer->m_renderDevice->SetRenderTarget("WindowView_"s + GetTitle(), windowRT, true, true);
   if (windowRT != sceneRT)
   {
      renderer->m_renderDevice->AddRenderTargetDependency(sceneRT, false);
      renderer->m_renderDevice->Clear(clearType::TARGET, 0);
   }

   renderer->m_renderDevice->ResetRenderState();
   renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   renderer->m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   renderer->m_renderDevice->m_DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f);
   renderer->m_renderDevice->m_DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_noDMD);
   const vec4 c = convertColor(0xFFFFFFFF, 1.f);
   renderer->m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, &c);
   renderer->m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_sprite, m_pTexture, SF_TRILINEAR, SA_CLAMP, SA_CLAMP, true);

   const float vx1 = px + 0 * sx;
   const float vy1 = py + 0 * sy;
   const float vx2 = vx1 + m_pTexture->width() * sx;
   const float vy2 = vy1 + m_pTexture->height() * sy;
   const Vertex3D_NoTex2 vertices[4] = { 
      { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, 
      { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, 
      { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
   renderer->m_renderDevice->DrawTexturedQuad(renderer->m_renderDevice->m_DMDShader, vertices);
}

}
