#pragma once

#include "RendererGraphics.h"
#include "Timer.h"

#include "DMDUtil/DMDUtil.h"

namespace VP {

class Window
{
public:
   Window(const string& szTitle, int x, int y, int w, int h, int z, int rotation);
   ~Window();

   virtual bool Init();
   const string& GetTitle() { return m_szTitle; }
   Uint32 GetId() { return m_id; }
   int GetZ() { return m_z; }
   int GetRotation() { return m_rotation; }
   int GetWidth() { return m_w; }
   int GetHeight() { return m_h; }
   void Show();
   void Hide();
   void OnUpdate();
   void OnRender();
   virtual void Render() { };

private:
   SDL_Window* m_pWindow;
   string m_szTitle;
   int m_id;
   int m_x;
   int m_y;
   int m_w;
   int m_h;
   int m_z;
   int m_rotation;
   bool m_init;

protected:
   bool m_visible;
   SDL_Renderer* m_pRenderer;
};

}
