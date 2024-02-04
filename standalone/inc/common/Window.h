#pragma once

#include "Graphics.h"
#include "Timer.h"

#include "DMDUtil/DMDUtil.h"

namespace VP {

class Window
{
public:
   Window(const string& szTitle, int x, int y, int w, int h, int z, bool highDpi);
   virtual ~Window();

   Uint32 GetId() { return m_id; }
   int GetX() { return m_x; }
   int GetY() { return m_y; }
   int GetWidth() { return m_w; }
   int GetHeight() { return m_h; }
   bool IsVisible() { return m_visible; }
   virtual void Render() { };
   void Show();
   void Hide();
   void HandleUpdate();

private:
   void Run();

   string m_szTitle;
   int m_id;
   int m_x;
   int m_y;
   int m_w;
   int m_h;
   bool m_visible;

protected:
   SDL_Renderer* m_pRenderer;
   SDL_Window* m_pWindow;

   bool m_running;
   std::thread* m_pThread;
};

}
