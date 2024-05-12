#include "core/stdafx.h"
#include "Window.h"

#ifdef __STANDALONE__
#include <SDL2/SDL_image.h>
#endif

namespace VPX
{

Window::Window(const string& title, const string& settingsId, const int display, const int w, const int h, const bool fullscreen, const int fsBitDeth, const int fsRefreshRate)
{
   int wnd_x, wnd_y;
   vector<DisplayConfig> displays;
   GetDisplays(displays);
   m_display = display < (int)displays.size() ? display : -1;
   // Look for 'NVIDIA PerfHUD' adapter. If it is present, override default settings (this only takes effect if run under NVPerfHud)
   for (const DisplayConfig& dispConf : displays)
   {
      if (strstr(dispConf.GPU_Name, "PerfHUD") != 0)
      {
         m_display = dispConf.display;
         break;
      }
   }
   for (const DisplayConfig& dispConf : displays)
   {
      if (dispConf.isPrimary || dispConf.display == m_display)
      {
         wnd_x = dispConf.left;
         wnd_y = dispConf.top;
         m_screenwidth = dispConf.width;
         m_screenheight = dispConf.height;
         m_adapter = dispConf.adapter;
         m_display = dispConf.display;
         if (dispConf.display == display)
            break;
      }
   }
   assert(m_display != -1); // This should not be possible since we use either the requested display or at least the primary one
   
   m_width = w <= 0 ? m_screenwidth : w;
   m_height = h <= 0 ? m_screenheight : h;
   m_fullscreen = fullscreen;
   m_settingsId = settingsId;
   
   // FIXME implement bit depth validation and selection (only used for DX9 10bit monitors to limit banding and dithering needs)
   m_bitdepth = fsBitDeth;

   // Validate fullscreen mode, eventually falling back to windowed mode (at desktop screen resolution)
   int validatedFSRefreshRate = -1;
   if (m_fullscreen)
   {
      bool fsModeExists = false;
      vector<VideoMode> allVideoModes;
      GetDisplayModes(m_display, allVideoModes);
      for (VideoMode mode : allVideoModes)
      {
         if ((mode.width == m_width) && (mode.height == m_height))
         {
            if (fsRefreshRate == 0)
            {
               fsModeExists = true;
               validatedFSRefreshRate = mode.refreshrate;
               break;
            }
            else if (mode.refreshrate == fsRefreshRate)
            {
               fsModeExists = true;
               validatedFSRefreshRate = fsRefreshRate;
               break;
            }
         }
      }
      if (!fsModeExists)
      {
         PLOGE << "Requested fullscreen mode " << m_width << "x" << m_height << " at " << fsRefreshRate << "Hz is not available. Switching to windowed mode";
         m_fullscreen = false;
         m_width = m_screenwidth;
         m_height = m_screenheight;
      }
      else
      {
         #ifdef ENABLE_SDL_VIDEO // SDL Windowing
            // For SDL, the display is selected through the window position
         #else // Win32 Windowing
            wnd_x = wnd_y = 0; // For DX9, the swap chain selects the display, not the window coordinates
         #endif
      }
   }

   // Constrain window to screen and restore its position (defaults to centered on screen)
   if (!m_fullscreen)
   {
      if (m_width > m_screenwidth)
      {
         m_height = (m_height * m_screenwidth) / m_width;
         m_width = m_screenwidth;
      }
      if (m_height > m_screenheight)
      {
         m_width = (m_width * m_screenheight) / m_height;
         m_height = m_screenheight;
      }
      wnd_x += (m_screenwidth - m_width) / 2;
      wnd_y += (m_screenheight - m_height) / 2;

      // Restore saved position of non fullscreen windows
      if ((m_height != m_screenheight) || (m_width != m_screenwidth))
      {
         const int xn = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, m_settingsId + "WndX"s, wnd_x);
         const int yn = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, m_settingsId + "WndY"s, wnd_y);
         // Only apply saved position if they fit inside a display
         #ifdef ENABLE_SDL_VIDEO // SDL Windowing
            for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
            {
               SDL_Rect displayBounds;
               if (SDL_GetDisplayBounds(i, &displayBounds) == 0)
               {
                  if (displayBounds.x <= wnd_x && wnd_x + m_width <= displayBounds.x + displayBounds.w
                   && displayBounds.y <= wnd_y && wnd_y + m_height <= displayBounds.y + displayBounds.h)
                  {
                     wnd_x = xn;
                     wnd_y = yn;
                     break;
                  }
               }
            }
         #else // Win32 Windowing
            RECT r;
            r.left = xn;
            r.top = yn;
            r.right = xn + m_width;
            r.bottom = yn + m_height;
            if (MonitorFromRect(&r, MONITOR_DEFAULTTONULL) != nullptr) // window is visible somewhere, so use the coords from the registry
            {
               wnd_x = xn;
               wnd_y = yn;
            }
         #endif
      }
   }

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      Uint32 wnd_flags = 0;
      #if defined(ENABLE_OPENGL)
         wnd_flags |= SDL_WINDOW_OPENGL;
      #elif defined(ENABLE_BGFX)
         // BGFX does not need the SDL window to have the SDL_WINDOW_OPENGL / SDL_WINDOW_VULKAN / SDL_WINDOW_METAL flag (see BGFX SDL example)
      #elif defined(ENABLE_DX9)
         // DX9 does not need any special flag either
      #endif
      #if defined(_MSC_VER) // Win32 (we use _MSC_VER since standalone also defines WIN32 for non Win32 builds)
         wnd_flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI;
         SDL_SetHint(SDL_HINT_FORCE_RAISEWINDOW, "1");
      #elif defined(__STANDALONE__)
         if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true))
            wnd_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
      #endif
      if (m_fullscreen)
         wnd_flags |= SDL_WINDOW_FULLSCREEN;
      else if (m_width == m_screenwidth && m_height == m_screenheight)
         wnd_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
         
      // Prevent full screen window from minimizing when re-arranging external windows
      SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

      m_nwnd = SDL_CreateWindow(title.c_str(), wnd_x, wnd_y, m_width, m_height, wnd_flags);
      
      SDL_DisplayMode mode;
      SDL_GetWindowDisplayMode(m_nwnd, &mode);
      m_refreshrate = mode.refresh_rate;
      if (m_fullscreen)
      {
         m_screenwidth = m_width;
         m_screenheight = m_height;
         if (fsRefreshRate > 0 && validatedFSRefreshRate != m_refreshrate) // Adjust refresh rate if needed
         {
            Uint32 format = mode.format;
            bool found = false;
            for (int index = 0; index < SDL_GetNumDisplayModes(m_adapter); index++)
            {
               SDL_GetDisplayMode(m_adapter, index, &mode);
               if ((mode.w == m_width) && (mode.h == m_height) && (mode.refresh_rate == validatedFSRefreshRate) && (mode.format == format))
               {
                  SDL_SetWindowDisplayMode(m_nwnd, &mode);
                  m_refreshrate = validatedFSRefreshRate;
                  found = true;
                  break;
               }
            }
            if (!found) {
               // Should not happen since the fullscreen values have been validated
               PLOGE << "Failed to find a display mode matching the requested refresh rate [" << validatedFSRefreshRate << ']';
            }
         }
      }
      PLOGI << "SDL display mode for window '" << settingsId << "': " << mode.w << 'x' << mode.h << ' ' << mode.refresh_rate << "Hz " << SDL_GetPixelFormatName(mode.format);

      #if defined(__APPLE__)
         // FIXME remove when porting to SDL3: horrible hack to handle the (strange) way Apple apply DPI: user DPI is applied as other OS but HiDPI of Retina display is applied internally
         // FIXME this only solves the window size, not its position which is in HiDPI units while it should be in pixel units
         SDL_GLContext sdl_context = SDL_GL_CreateContext(m_nwnd);
         SDL_GL_MakeCurrent(m_nwnd, sdl_context);
         int drawableWidth, drawableHeight;
         SDL_GL_GetDrawableSize(m_nwnd, &drawableWidth, &drawableHeight); // Size in pixels
         SDL_GL_DeleteContext(sdl_context);
         m_hidpiScale = (float)drawableWidth / (float)m_width;
         PLOGI << "SDL HiDPI defined to " << m_hidpiScale;
         m_width = drawableWidth;
         m_height = drawableHeight;
      #endif

      #ifdef __STANDALONE__
         const string iconPath = g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "vpinball.png";
         SDL_Surface* pIcon = IMG_Load(iconPath.c_str());
         if (pIcon) {
            SDL_SetWindowIcon(m_nwnd, pIcon);
            SDL_FreeSurface(pIcon);
         }
         else {
            PLOGE << "Failed to load window icon: " << SDL_GetError();
         }
      #endif

   #else // Win32 Windowing
      // TODO this window class is only suitable for main playfield window, use the right one depending on the use scheme
      m_nwnd = ::CreateWindowEx(0, _T("VPPlayer"), title.c_str(), WS_POPUP, wnd_x, wnd_y, m_width, m_height, NULL, NULL, g_pvp->theInstance, NULL);
      
      if (m_fullscreen)
      {
         // These values have been validated, so we consider that they will be the right ones when we will switch to fullscreen upon DX9 device creation
         m_screenwidth = m_width;
         m_screenheight = m_height;
         m_refreshrate = validatedFSRefreshRate;
      }
      else
      {
         IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
         if (d3d == nullptr)
         {
            ShowError("Could not create D3D9 object.");
            return;
         }
         D3DDISPLAYMODE mode;
         CHECKD3D(d3d->GetAdapterDisplayMode(m_adapter, &mode));
         m_refreshrate = mode.RefreshRate;
         SAFE_RELEASE(d3d);
      }
      
   #endif
   
   assert(m_width > 0);
   assert(m_height > 0);
   if (m_refreshrate <= 0)
   {
      PLOGE << "Failed to get display refresh rate, defaulting to 60Hz";
      m_refreshrate = 60;
   }
}

Window::~Window()
{
   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_DestroyWindow(m_nwnd);
   #else // Win32 Windowing
      DestroyWindow(m_nwnd);
   #endif
}

void Window::ShowAndFocus()
{
   #if defined(ENABLE_SDL_VIDEO) // SDL Windowing
      SDL_ShowWindow(m_nwnd);
      SDL_RaiseWindow(m_nwnd);
      //SDL_SetWindowInputFocus(m_nwnd);
   #else // Win32 Windowing
      ShowWindow(m_nwnd, SW_SHOW);
      SetForegroundWindow(m_nwnd);
      SetFocus(m_nwnd);
   #endif
}

void Window::GetPos(int& x, int& y) const
{
   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_GetWindowPosition(m_nwnd, &x, &y);
   #else // Win32 Windowing
      RECT rect;
      GetWindowRect(m_nwnd, &rect);
      x = rect.left;
      y = rect.top;
   #endif
}

void Window::SetPos(const int x, const int y)
{
   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_SetWindowPosition(m_nwnd, x, y);
   #else // Win32 Windowing
      RECT rect;
      GetWindowRect(m_nwnd, &rect);
      rect.left = x;
      rect.top = y;
      SetWindowPos(m_nwnd, nullptr, x, y, m_width, m_height, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
   #endif
   g_pvp->m_settings.SaveValue(Settings::Player, m_settingsId + "WndX"s, x);
   g_pvp->m_settings.SaveValue(Settings::Player, m_settingsId + "WndY"s, y);
}



#if defined(_WIN32) || !defined(ENABLE_SDL_VIDEO)
BOOL CALLBACK MonitorEnumList(__in  HMONITOR hMonitor, __in  HDC hdcMonitor, __in  LPRECT lprcMonitor, __in  LPARAM dwData)
{
   std::map<string, VPX::Window::DisplayConfig>* data = reinterpret_cast<std::map<string, VPX::Window::DisplayConfig>*>(dwData);
   MONITORINFOEX info;
   info.cbSize = sizeof(MONITORINFOEX);
   GetMonitorInfo(hMonitor, &info);
   VPX::Window::DisplayConfig config = {};
   config.top = info.rcMonitor.top;
   config.left = info.rcMonitor.left;
   config.width = info.rcMonitor.right - info.rcMonitor.left;
   config.height = info.rcMonitor.bottom - info.rcMonitor.top;
   config.isPrimary = (config.top == 0) && (config.left == 0);
   config.display = -1;
   config.adapter = -1;
   memcpy(config.DeviceName, info.szDevice, CCHDEVICENAME); // Internal display name e.g. "\\\\.\\DISPLAY1"
   data->insert(std::pair<string, VPX::Window::DisplayConfig>(config.DeviceName, config));
   return TRUE;
}
#endif

int Window::GetDisplays(vector<DisplayConfig>& displays)
{
   displays.clear();

#if defined(ENABLE_SDL_VIDEO) && defined(_WIN32)
   // Windows and SDL order of display enumeration do not match, therefore the display identifier will not match between DX and OpenGL version
   // SDL2 display identifier do not match the id of the native Windows settings
   // SDL2 does not offer a way to get the adapter (i.e. Graphics Card) associated with a display (i.e. Monitor) so we use the monitor name for both
   // Get the resolution of all enabled displays as they appear in the Windows settings UI.
   std::map<string, DisplayConfig> displayMap;
   EnumDisplayMonitors(nullptr, nullptr, MonitorEnumList, reinterpret_cast<LPARAM>(&displayMap));
   for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
   {
      SDL_Rect displayBounds;
      if (SDL_GetDisplayBounds(i, &displayBounds) == 0)
      {
         for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
         {
            if (display->second.left == displayBounds.x && display->second.top == displayBounds.y && display->second.width == displayBounds.w && display->second.height == displayBounds.h)
            {
               display->second.adapter = i;
               strncpy_s(display->second.GPU_Name, SDL_GetDisplayName(display->second.adapter), MAX_DEVICE_IDENTIFIER_STRING - 1);
            }
         }
      }
   }

   // Apply the same numbering as windows
   int i = 0;
   for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
   {
      if (display->second.adapter >= 0)
      {
         display->second.display = i;
         displays.push_back(display->second);
      }
      i++;
   }
#elif defined(ENABLE_SDL_VIDEO)
   int i = 0;
   for (; i < SDL_GetNumVideoDisplays(); ++i)
   {
      SDL_Rect displayBounds;
      if (SDL_GetDisplayBounds(i, &displayBounds) == 0) {
         DisplayConfig displayConf;
         displayConf.display = i; // Window Display identifier (the number that appears in the native Windows settings)
         displayConf.adapter = i; // SDL Display identifier. Will be used for creating the display
         displayConf.isPrimary = (displayBounds.x == 0) && (displayBounds.y == 0);
         displayConf.top = displayBounds.y;
         displayConf.left = displayBounds.x;
         displayConf.width = displayBounds.w;
         displayConf.height = displayBounds.h;
         const string devicename = "\\\\.\\DISPLAY"s.append(std::to_string(i));
         strncpy_s(displayConf.DeviceName, devicename.c_str(), CCHDEVICENAME - 1);
         strncpy_s(displayConf.GPU_Name, SDL_GetDisplayName(displayConf.display), MAX_DEVICE_IDENTIFIER_STRING - 1);
         displays.push_back(displayConf);
      }
   }
#else
   // Get the resolution of all enabled displays as they appear in the Windows settings UI.
   std::map<string, DisplayConfig> displayMap;
   EnumDisplayMonitors(nullptr, nullptr, MonitorEnumList, reinterpret_cast<LPARAM>(&displayMap));

   IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
   if (pD3D == nullptr)
   {
      ShowError("Could not create D3D9 object.");
      return -1;
   }
   // Map the displays to the DX9 adapter. Otherwise this leads to an performance impact on systems with multiple GPUs
   const int adapterCount = pD3D->GetAdapterCount();
   for (int i = 0; i < adapterCount; ++i)
   {
      D3DADAPTER_IDENTIFIER9 adapter;
      pD3D->GetAdapterIdentifier(i, 0, &adapter);
      std::map<string, DisplayConfig>::iterator display = displayMap.find(adapter.DeviceName);
      if (display != displayMap.end())
      {
         display->second.adapter = i;
         strncpy_s(display->second.GPU_Name, adapter.Description, sizeof(display->second.GPU_Name) - 1);
      }
   }
   SAFE_RELEASE(pD3D);
   
   // Apply the same numbering as windows
   int i = 0;
   for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
   {
      if (display->second.adapter >= 0) {
         display->second.display = i;
         displays.push_back(display->second);
      }
      i++;
   }
#endif

   return i;
}

void Window::GetDisplayModes(const int display, vector<VideoMode>& modes)
{
   modes.clear();

   vector<DisplayConfig> displays;
   GetDisplays(displays);
   if (display >= (int)displays.size())
      return;
   const int adapter = displays[display].adapter;

   #if defined(ENABLE_SDL_VIDEO)
      const int amount = SDL_GetNumDisplayModes(adapter);
      for (int mode = 0; mode < amount; ++mode) {
         SDL_DisplayMode sdlMode;
         SDL_GetDisplayMode(adapter, mode, &sdlMode);
         VideoMode vmode = {};
         vmode.width = sdlMode.w;
         vmode.height = sdlMode.h;
         switch (sdlMode.format) {
         case SDL_PIXELFORMAT_RGB24:
         case SDL_PIXELFORMAT_BGR24:
         case SDL_PIXELFORMAT_RGB888:
         case SDL_PIXELFORMAT_RGBX8888:
         case SDL_PIXELFORMAT_BGR888:
         case SDL_PIXELFORMAT_BGRX8888:
         case SDL_PIXELFORMAT_ARGB8888:
         case SDL_PIXELFORMAT_RGBA8888:
         case SDL_PIXELFORMAT_ABGR8888:
         case SDL_PIXELFORMAT_BGRA8888:
            vmode.depth = 32;
            break;
         case SDL_PIXELFORMAT_RGB565:
         case SDL_PIXELFORMAT_BGR565:
         case SDL_PIXELFORMAT_ABGR1555:
         case SDL_PIXELFORMAT_BGRA5551:
         case SDL_PIXELFORMAT_ARGB1555:
         case SDL_PIXELFORMAT_RGBA5551:
            vmode.depth = 16;
            break;
         case SDL_PIXELFORMAT_ARGB2101010:
            vmode.depth = 30;
            break;
         default:
            vmode.depth = 0;
         }
         vmode.refreshrate = sdlMode.refresh_rate;
         modes.push_back(vmode);
      }
   #else
      IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
      if (d3d == nullptr)
      {
         ShowError("Could not create D3D9 object.");
         return;
      }

      //for (int j = 0; j < 2; ++j)
      constexpr int j = 0; // limit to 32bit only nowadays
      {
         const D3DFORMAT fmt = (D3DFORMAT)((j == 0) ? colorFormat::RGB8 : colorFormat::RGB5);
         const unsigned numModes = d3d->GetAdapterModeCount(adapter, fmt);

         for (unsigned i = 0; i < numModes; ++i)
         {
            D3DDISPLAYMODE d3dmode;
            d3d->EnumAdapterModes(adapter, fmt, i, &d3dmode);

            if (d3dmode.Width >= 640)
            {
               VideoMode mode;
               mode.width = d3dmode.Width;
               mode.height = d3dmode.Height;
               mode.depth = (fmt == (D3DFORMAT)colorFormat::RGB5) ? 16 : 32;
               mode.refreshrate = d3dmode.RefreshRate;
               modes.push_back(mode);
            }
         }
      }

      SAFE_RELEASE(d3d);
   #endif
}

} // namespace VPX
