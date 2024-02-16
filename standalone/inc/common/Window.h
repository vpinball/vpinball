#pragma once

#include "RendererGraphics.h"
#include "Timer.h"

#include "DMDUtil/DMDUtil.h"

namespace VP {

class Window
{
public:
   Window(const string& szTitle, int x, int y, int w, int h, int z);
   ~Window();

   virtual bool Init();
   Uint32 GetId() { return m_id; }
   int GetZ() { return m_z; }
   void Show();
   void Hide();
   void OnUpdate();
   void OnRender();
   virtual void Render() { };
   VP::Graphics* GetGraphics() { return m_pGraphics; }
   SDL_Point GetPosition() { return SDL_Point { m_x, m_y }; }
   int GetWidth();
   int GetHeight();

private:
   SDL_Window* m_pWindow;
   string m_szTitle;
   int m_id;
   int m_x;
   int m_y;
   int m_w;
   int m_h;
   int m_z;
   bool m_visible;
   bool m_init;
   const Uint64 m_frameDuration = 16;
   Uint64 m_lastRenderTime;

protected:
   SDL_Renderer* m_pRenderer;
};

}
