#pragma once

#include "Graphics.h"
#include "Timer.h"

#include "DMDUtil/DMDUtil.h"

namespace VP {

class Window
{
public:
   enum RenderMode : int
   {
      RenderMode_Default = 0,
      RenderMode_MainThread
   };

   Window(const string& szTitle, RenderMode renderMode, int x, int y, int w, int h, int z, bool highDpi);
   virtual ~Window();

   Uint32 GetId() { return m_id; }
   RenderMode GetRenderMode() { return m_renderMode; }
   int GetX() { return m_x; }
   int GetY() { return m_y; }
   int GetWidth() { return m_w; }
   int GetHeight() { return m_h; }
   int GetZ() { return m_z; }
   bool IsVisible() { return m_visible; }
   void Show();
   void Hide();
   virtual void Render() { };
   void HandleUpdate();

private:
   void Run();

   string m_szTitle;
   int m_id;
   RenderMode m_renderMode;
   int m_x;
   int m_y;
   int m_w;
   int m_h;
   int m_z;
   bool m_visible;

protected:
   SDL_Renderer* m_pRenderer;
   SDL_Window* m_pWindow;

   bool m_running;
   std::thread* m_pThread;
};

}
