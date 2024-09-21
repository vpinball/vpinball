// license:GPLv3+

#pragma once

#ifdef ENABLE_SDL_VIDEO // SDL Windowing
#include "SDL2/SDL.h"
#else // Win32 Windowing
#include <windows.h>
#endif

namespace VPX
{

// All coordinates are given in pixels, not logical units.
// FIXME for MacOS/IOS HiDPI is applied (so giving 'logical' units with Retina scale factor but not user DPI)
class Window final
{
public:
   Window(const string &title, const Settings::Section section, const string &settingsPrefix);
   ~Window();

   void GetPos(int&x, int &y) const;
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   int GetRefreshRate() const { return m_refreshrate; } // Refresh rate of the device displaying the window. Window spread over multiple devices are not supported.
   bool IsFullScreen() const { return m_fullscreen; }
   int GetAdapterId() const { return m_adapter; }
   int GetBitDepth() const { return m_bitdepth; }
   float GetHiDPIScale() const { return m_hidpiScale; } // HiDPI scale on Apple devices

   void SetPos(const int x, const int y);
   void Show(const bool show = true);
   void RaiseAndFocus(const bool raise = true);

   void SetBackBuffer(RenderTarget* rt) { assert(rt == nullptr || (rt->GetWidth() == m_width && rt->GetHeight() == m_height)); m_backBuffer = rt; }
   RenderTarget* GetBackBuffer() const { return m_backBuffer; }

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
      int refreshrate;
   };

   struct DisplayConfig
   {
      int display; // Window Display identifier (the number that appears in the native Windows settings)
      int adapter; // DirectX9 or SDL display/adapter identifier
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
   int m_refreshrate;
   int m_bitdepth;
   const Settings::Section m_settingsSection;
   const string m_settingsPrefix;

   class RenderTarget* m_backBuffer = nullptr;

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_Window *m_nwnd = nullptr;
   #else // Win32 Windowing
      HWND m_nwnd = nullptr;
   #endif
};

}
