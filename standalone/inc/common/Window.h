#pragma once

#include "RendererGraphics.h"
#include "Timer.h"

#include "DMDUtil/DMDUtil.h"

namespace VP {

class Window
{
public:
   Window(const string& szTitle, int z, int x, int y, int w, int h);
   ~Window();

   virtual bool Init();
   const string& GetTitle() { return m_szTitle; }
   int GetZ() { return m_z; }
   int GetWidth() { return m_pWindow ? m_pWindow->GetWidth() : m_pEmbeddedWindow ? m_pEmbeddedWindow->GetWidth() : 0; }
   int GetHeight() { return m_pWindow ? m_pWindow->GetHeight() : m_pEmbeddedWindow ? m_pEmbeddedWindow->GetHeight() : 0; }
   int GetDrawableWidth() { return m_pWindow ? m_pWindow->GetDrawableWidth() : m_pEmbeddedWindow ? m_pEmbeddedWindow->GetWidth() : 0; }
   int GetDrawableHeight() { return m_pWindow ? m_pWindow->GetDrawableHeight() : m_pEmbeddedWindow ? m_pEmbeddedWindow->GetHeight() : 0; }
   void Show();
   void Hide();
   bool CanRender() { return m_init && m_visible; }
   virtual void Render();
   VPX::Window* GetWindow() { return m_pWindow; }

private:
   string m_szTitle;
   int m_z;
   bool m_init;

protected:
   bool m_visible;
   VPX::RenderOutput* m_pRenderOutput;
   VPX::Window* m_pWindow;
   VPX::EmbeddedWindow* m_pEmbeddedWindow;
   SDL_Renderer* m_pRenderer;
   SDL_Surface* m_pSurface;
   BaseTexture* m_pTexture;
};

}
