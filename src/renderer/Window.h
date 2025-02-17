// license:GPLv3+

#pragma once

#ifdef ENABLE_SDL_VIDEO // SDL Windowing
#include <SDL3/SDL.h>
#else // Win32 Windowing
#include <windows.h>
#endif

#include "core/Settings.h"

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

   void GetPos(int& x, int& y) const { x = m_x; y = m_y; }
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

private:
   int m_x, m_y;
   int m_width, m_height;
};

// All coordinates are given in pixels, not logical units.
// FIXME for MacOS/IOS HiDPI is applied (so giving 'logical' units with Retina scale factor but not user DPI)
class Window final
{
public:
   Window(const string &title, const Settings::Section section, const string &settingsPrefix); // OS Window
   Window(const int width, const int height); // VR Output
   ~Window();

   void GetPos(int&x, int &y) const;
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   float GetRefreshRate() const { return m_refreshrate; } // Refresh rate of the device displaying the window. Window spread over multiple devices are not supported.
   bool IsFullScreen() const { return m_fullscreen; }
   int GetAdapterId() const { return m_adapter; }
   int GetBitDepth() const { return m_bitdepth; }
   float GetHiDPIScale() const { return m_hidpiScale; } // HiDPI scale on Apple devices
   bool IsWCGDisplay() const { return m_wcgDisplay; } // Wether this window is on a WCG enabled display
   float GetSDRWhitePoint() const { return m_sdrWhitePoint; } // Selected SDR White Point of display in multiple of 80nits (so 3 gives 240nits for SDR white)
   float GetHDRHeadRoom() const { return m_hdrHeadRoom; } // Maximum luminance of display expressed in multiple of SDRWhitePoint (so 6 means 6 times the SDR whitepoint)

   void SetPos(const int x, const int y);
   void Show(const bool show = true);
   void RaiseAndFocus(const bool raise = true);

   void SetBackBuffer(RenderTarget* rt, const bool wcgBackbuffer = false) { assert(rt == nullptr || (rt->GetWidth() == m_width && rt->GetHeight() == m_height)); m_backBuffer = rt; m_wcgBackbuffer = wcgBackbuffer; }
   RenderTarget* GetBackBuffer() const { return m_backBuffer; }
   bool IsWCGBackBuffer() const { return m_wcgBackbuffer; } // Return true for HDR10/BT.2100 colorspace, otherwise Rec 709 colorspace

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_Window * GetCore() const { return m_nwnd; }

      #ifdef _WIN32
         HWND GetNativeHWND() const;
      #endif
   #else // Win32 Windowing
      HWND GetCore() const { return m_nwnd; }
      HWND GetNativeHWND() const { return m_nwnd; }
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
      int display; // Window Display identifier (the number that appears in the native Windows settings)
   #ifdef ENABLE_SDL_VIDEO 
      SDL_DisplayID adapter; // SDL display/adapter identifier
   #else
      int adapter; // DirectX9 display/adapter identifier
   #endif
      int top;
      int left;
      int width;
      int height;
      bool isPrimary;
      char DeviceName[CCHDEVICENAME]; // Device native identifier, e.g. "\\\\.\\DISPLAY1"
      char GPU_Name[MAX_DEVICE_IDENTIFIER_STRING]; // GPU name if available, device (monitor) name otherwise
   };

   static int GetDisplays(vector<DisplayConfig>& displays);
   static void GetDisplayModes(const int display, vector<VideoMode>& modes);

private:
   float m_hidpiScale = 1.f;
   int m_width, m_height;
   int m_display, m_adapter;
   int m_screenwidth, m_screenheight;
   bool m_fullscreen;
   float m_refreshrate;
   int m_bitdepth;
   const Settings::Section m_settingsSection;
   const string m_settingsPrefix;
   float m_sdrWhitePoint = 1.f;
   float m_hdrHeadRoom = 1.f;
   bool m_wcgDisplay = false;
   bool m_wcgBackbuffer = false;
   const bool m_isVR;

   class RenderTarget* m_backBuffer = nullptr;

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_Window *m_nwnd = nullptr;
   #else // Win32 Windowing
      HWND m_nwnd = nullptr;
   #endif
};

class RenderOutput final
{
public:
   RenderOutput(const string& title, Settings& settings, const Settings::Section section, const string& settingsPrefix)
      : m_settingsSection(section)
      , m_settingsPrefix(settingsPrefix)
   {
      m_mode = static_cast<OutputMode>(settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "Output", OM_DISABLED));
      if (m_mode == OM_EMBEDDED)
      {
         const int x = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "WndX", 0);
         const int y = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "WndY", 0);
         const int width = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "Width", 640);
         const int height = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "Height", 480);
         m_embeddedWindow = new EmbeddedWindow(x, y, width, height);
      }
      else if (m_mode == OM_WINDOW)
         m_window = new Window(title, section, settingsPrefix);
   }

   ~RenderOutput()
   {
      delete m_embeddedWindow;
      delete m_window;
   }

   enum OutputMode
   {
      OM_DISABLED, // Output is disabled
      OM_EMBEDDED, // Output is embedded in another output
      OM_WINDOW // Output is a native system window (maybe a window, exclusive fullscreen, VR headset,...)
   };

   OutputMode GetMode() const { return m_mode; }
   Window* GetWindow() const { return m_window; }
   EmbeddedWindow* GetEmbeddedWindow() const { return m_embeddedWindow; }

private:
   const Settings::Section m_settingsSection;
   const string m_settingsPrefix;
   OutputMode m_mode = OutputMode::OM_DISABLED;
   Window* m_window = nullptr;
   EmbeddedWindow* m_embeddedWindow = nullptr;
};

}
