// license:GPLv3+

#pragma once

#include <SDL3/SDL.h>

#include "core/Settings.h"
#include "plugins/VPXPlugin.h"

class RenderTarget;

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

   enum WindowMode
   {
      Windowed,
      BorderlessFullscreen, // Broderless window, covering the entire display, including operating system decorations
      ExclusiveFullscreen // Not supported on all systems
   };

   float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
   float GetRefreshRate() const { return m_refreshrate; } // Refresh rate of the device displaying the window. Window spread over multiple devices are not supported.
   WindowMode GetWindowMode() const { return m_windowMode; }
   int GetBitDepth() const { return m_bitdepth; }
   bool IsWCGDisplay() const { return m_wcgDisplay; } // Whether this window is on a WCG enabled display
   float GetSDRWhitePoint() const { return m_sdrWhitePoint; } // Selected SDR White Point of display in multiple of 80nits (so 3 gives 240nits for SDR white)
   float GetHDRHeadRoom() const { return m_hdrHeadRoom; } // Maximum luminance of display expressed in multiple of SDRWhitePoint (so 6 means 6 times the SDR whitepoint)
   // Logical units
   void GetPos(int& x, int& y) const;
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   // Pixel units
   void GetPixelPos(int& x, int& y) const;
   int GetPixelWidth() const { return m_pixelWidth; }
   int GetPixelHeight() const { return m_pixelHeight; }
   float GetPixelDensity() const { return m_pixelDensity; } // Logical size to pixels (e.g. a 1920x1080 logical size with 2.0 pixel density would have 3840x2160 pixels)
   int LogicalToPixel(int v) const { return static_cast<int>(roundf(static_cast<float>(v) * m_pixelDensity)); }
   int PixelToLogical(int v) const { return static_cast<int>(roundf(static_cast<float>(v) / m_pixelDensity)); }

   void SetPos(const int x, const int y);
   void SetPixelPos(const int x, const int y); // Position will be rounded
   void SetSize(const int w, const int h); // performed asynchronously
   void SetPixelSize(const int w, const int h); // Performed asynchronously, size will be rounded
   void OnResized(); // To be called when the window manager resized the window
   void Show(const bool show = true);
   bool IsVisible() const;
   void RaiseAndFocus();
   bool IsFocused() const;
   void SetFocusable(const bool focusable);

   bool IsPositioningSupported() const { return m_isPositioningSupported; } // If false, GetPos/GetPixelPos/SetPos/SetPixelPos will all fail, thanks Wayland

   void SetBackBuffer(RenderTarget* rt, const bool wcgBackbuffer = false);
   RenderTarget* GetBackBuffer() const { return m_backBuffer; }
   bool IsWCGBackBuffer() const { return m_wcgBackbuffer; } // Return true for HDR10/BT.2100 colorspace, otherwise Rec 709 colorspace

   SDL_Window* GetCore() const { return m_nwnd; }

#ifdef _WIN32
   HWND GetNativeHWND() const;
#endif

   struct VideoMode
   {
      int width; // Logical units
      int height; // Logical units
      float pixelDensity; // Scale converting logical size to pixels (e.g. a 1920x1080 mode with 2.0 scale would have 3840x2160 pixels)
      int depth;
      float refreshrate;

      bool operator==(const VideoMode& o) { return o.width == width && o.height == height && o.depth == depth && o.refreshrate == refreshrate; }
      int GetPixelWidth() const { return static_cast<int>(roundf(static_cast<float>(width) * pixelDensity)); }
      int GetPixelHeight() const { return static_cast<int>(roundf(static_cast<float>(height) * pixelDensity)); }
   };

   struct DisplayConfig
   {
      int top; // Logical position
      int left; // Logical position
      VideoMode videomode;
      bool isPrimary; // Default display (used when no display is selected in the settings)
      string displayName; // User friendly display name, should be stable accross runs, therefore used for settings
      SDL_DisplayID display; // SDL display identifier (only valid for the lifetime of the SDL session)
   };

   static vector<DisplayConfig> GetDisplays();
   static vector<VideoMode> GetDisplayModes(const DisplayConfig& display);
   static DisplayConfig GetDisplayConfig(const string& displayName);

private:
   static VideoMode SDLtoVPXVideoMode(const SDL_DisplayMode* mode);
   
   int m_width, m_height; // Logical units
   float m_pixelDensity; // Scale converting logical size to pixels (e.g. a 1920x1080 mode with 2.0 scale would have 3840x2160 pixels)
   int m_pixelWidth, m_pixelHeight; // Pixels
   int m_screenwidth, m_screenheight; // Pixels
   WindowMode m_windowMode;
   float m_refreshrate;
   int m_bitdepth;
   const VPXWindowId m_windowId;
   float m_sdrWhitePoint = 1.f;
   float m_hdrHeadRoom = 1.f;
   bool m_wcgDisplay = false;
   bool m_wcgBackbuffer = false;
   bool m_isPositioningSupported = true;
   const bool m_isVR;

   class RenderTarget* m_backBuffer = nullptr;

   SDL_Window* m_nwnd = nullptr;
};

class RenderOutput final
{
public:
   RenderOutput(VPXWindowId windowId);
   ~RenderOutput() = default;

   enum OutputMode
   {
      OM_DISABLED, // Output is disabled
      OM_WINDOW, // Output is a native system window (maybe a window, exclusive fullscreen, VR headset,...)
      OM_EMBEDDED, // Output is embedded in the playfield window
   };
   OutputMode GetMode() const;
   void SetMode(const Settings& settings, OutputMode mode);

   Window* GetWindow() const;
   EmbeddedWindow* GetEmbeddedWindow() const;

   // Unit depends on output type: logical for floating window, pixel for embedded
   int GetWidth() const;
   void SetWidth(int v) const;
   int GetHeight() const;
   void SetHeight(int v) const;

   void GetPos(int& x, int& y) const;
   void SetPos(int x, int y) const;

private:
   const VPXWindowId m_windowId;
   OutputMode m_mode = OutputMode::OM_DISABLED;
   std::unique_ptr<Window> m_window = nullptr;
   std::unique_ptr<EmbeddedWindow> m_embeddedWindow = nullptr;
};

}
