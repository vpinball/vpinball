// license:GPLv3+

#pragma once

#include <SDL3/SDL.h>

#include "core/Settings.h"
#include "plugins/VPXPlugin.h"

namespace VPX
{

class EmbeddedWindow final
{
public:
   EmbeddedWindow(int x, int y, int w, int h)
      : m_x(x)
      , m_y(y)
      , m_width(w)
      , m_height(h)
   {
   }

   void GetPos(int& x, int& y) const
   {
      x = m_x;
      y = m_y;
   }
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   void SetPos(int x, int y)
   {
      m_x = x;
      m_y = y;
   }
   void SetWidth(int v) { m_width = v; }
   void SetHeight(int v) { m_height = v; }
   void SetSize(int width, int height)
   {
      m_width = width;
      m_height = height;
   }

private:
   int m_x, m_y;
   int m_width, m_height;
};

// All coordinates are given in pixels, not logical units.
class Window final
{
public:
   Window(const string& title, const Settings& settings, VPXWindowId windowId); // OS Window
   Window(const int width, const int height); // VR Output
   ~Window();

   void GetPos(int& x, int& y) const;
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   int GetPixelWidth() const { return m_pixelWidth; }
   int GetPixelHeight() const { return m_pixelHeight; }
   float GetRefreshRate() const { return m_refreshrate; } // Refresh rate of the device displaying the window. Window spread over multiple devices are not supported.
   bool IsFullScreen() const { return m_fullscreen; }
   int GetBitDepth() const { return m_bitdepth; }
   bool IsWCGDisplay() const { return m_wcgDisplay; } // Whether this window is on a WCG enabled display
   float GetSDRWhitePoint() const { return m_sdrWhitePoint; } // Selected SDR White Point of display in multiple of 80nits (so 3 gives 240nits for SDR white)
   float GetHDRHeadRoom() const { return m_hdrHeadRoom; } // Maximum luminance of display expressed in multiple of SDRWhitePoint (so 6 means 6 times the SDR whitepoint)

   void SetPos(const int x, const int y);
   void Show(const bool show = true);
   bool IsVisible() const;
   void RaiseAndFocus();
   bool IsFocused() const;

   void SetBackBuffer(RenderTarget* rt, const bool wcgBackbuffer = false)
   {
      assert(rt == nullptr || (rt->GetWidth() == m_pixelWidth && rt->GetHeight() == m_pixelHeight));
      m_backBuffer = rt;
      m_wcgBackbuffer = wcgBackbuffer;
   }
   RenderTarget* GetBackBuffer() const { return m_backBuffer; }
   bool IsWCGBackBuffer() const { return m_wcgBackbuffer; } // Return true for HDR10/BT.2100 colorspace, otherwise Rec 709 colorspace

   SDL_Window* GetCore() const { return m_nwnd; }

#ifdef _WIN32
   HWND GetNativeHWND() const;
#endif

   struct VideoMode
   {
      int width;
      int height;
      int depth;
      float refreshrate;
   };

   struct DisplayConfig
   {
      int top;
      int left;
      int width;
      int height;
      int depth;
      float refreshrate;
      bool isPrimary; // Default display (used when no display is selected in the settings)
      string displayName; // User friendly display name, should be stable accross runs, therefore used for settings
      SDL_DisplayID display; // SDL display identifier (only valid for the lifetime of the SDL session)
   };

   DisplayConfig GetDisplayConfig(const Settings& settings) const;

   static vector<DisplayConfig> GetDisplays();
   static vector<VideoMode> GetDisplayModes(const DisplayConfig& display);
   static DisplayConfig GetDisplayConfig(const string& displayName);

private:
   int m_width, m_height;
   int m_pixelWidth, m_pixelHeight;
   int m_screenwidth, m_screenheight;
   bool m_fullscreen;
   float m_refreshrate;
   int m_bitdepth;
   const VPXWindowId m_windowId;
   float m_sdrWhitePoint = 1.f;
   float m_hdrHeadRoom = 1.f;
   bool m_wcgDisplay = false;
   bool m_wcgBackbuffer = false;
   const bool m_isVR;

   class RenderTarget* m_backBuffer = nullptr;

   SDL_Window* m_nwnd = nullptr;
};

class RenderOutput final
{
public:
   RenderOutput(const Settings& settings, VPXWindowId windowId)
      : m_windowId(windowId)
   {
      m_mode = OM_DISABLED;
      SetMode(settings, static_cast<OutputMode>(settings.GetWindow_Mode(m_windowId)));
   }

   ~RenderOutput() = default;

   enum OutputMode
   {
      OM_DISABLED, // Output is disabled
      OM_WINDOW, // Output is a native system window (maybe a window, exclusive fullscreen, VR headset,...)
      OM_EMBEDDED, // Output is embedded in the playfield window
   };

   OutputMode GetMode() const { return m_mode; }

   void SetMode(const Settings& settings, OutputMode mode)
   {
      m_mode = mode;
      if (m_mode == OM_EMBEDDED && m_embeddedWindow == nullptr)
      {
         const int x = settings.GetWindow_WndX(m_windowId);
         const int y = settings.GetWindow_WndY(m_windowId);
         const int width = settings.GetWindow_Width(m_windowId);
         const int height = settings.GetWindow_Height(m_windowId);
         m_embeddedWindow = std::make_unique<EmbeddedWindow>(x, y, width, height);
         m_window = nullptr;
      }
      else if (m_mode == OM_WINDOW && m_window == nullptr)
      {
         m_embeddedWindow = nullptr;
         m_window = std::make_unique<Window>(m_windowId == VPXWindowId::VPXWINDOW_Playfield ? "Visual Pinball Player"s
               : m_windowId == VPXWindowId::VPXWINDOW_Backglass                             ? "Visual Pinball Backglass"s
               : m_windowId == VPXWindowId::VPXWINDOW_ScoreView                             ? "Visual Pinball Score View"s
               : m_windowId == VPXWindowId::VPXWINDOW_Topper                                ? "Visual Pinball Topper"s
                                                                                            : "Visual Pinball VR Preview"s,
            settings, m_windowId);
      }
   }

   Window* GetWindow() const { return m_window.get(); }

   EmbeddedWindow* GetEmbeddedWindow() const { return m_embeddedWindow.get(); }

   int GetWidth() const
   {
      if (m_mode == OM_EMBEDDED)
         return m_embeddedWindow->GetWidth();
      else if (m_mode == OM_WINDOW)
         return m_window->GetPixelWidth();
      else
         return 0;
   }

   void SetWidth(int v) const
   {
      if (m_mode == OM_EMBEDDED)
         m_embeddedWindow->SetWidth(v);
      else
         assert(false);
   }

   int GetHeight() const
   {
      if (m_mode == OM_EMBEDDED)
         return m_embeddedWindow->GetHeight();
      else if (m_mode == OM_WINDOW)
         return m_window->GetPixelHeight();
      else
         return 0;
   }

   void SetHeight(int v) const
   {
      if (m_mode == OM_EMBEDDED)
         m_embeddedWindow->SetHeight(v);
      else
         assert(false);
   }

   void GetPos(int& x, int& y) const
   {
      if (m_mode == OM_EMBEDDED)
         m_embeddedWindow->GetPos(x, y);
      else if (m_mode == OM_WINDOW)
         m_window->GetPos(x, y);
      else
      {
         x = 0;
         y = 0;
      }
   }

   void SetPos(int x, int y) const
   {
      if (m_mode == OM_EMBEDDED)
         m_embeddedWindow->SetPos(x, y);
      else if (m_mode == OM_WINDOW)
         m_window->SetPos(x, y);
   }

private:
   const VPXWindowId m_windowId;
   OutputMode m_mode = OutputMode::OM_DISABLED;
   std::unique_ptr<Window> m_window = nullptr;
   std::unique_ptr<EmbeddedWindow> m_embeddedWindow = nullptr;
};

}
