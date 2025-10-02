// license:GPLv3+

#include "core/stdafx.h"
#include "Window.h"

#include <SDL3/SDL_video.h>

#ifdef _MSC_VER
#include <dxgi1_2.h>
#pragma comment(lib, "dxgi.lib")
#endif

#ifdef __STANDALONE__
#include <SDL3_image/SDL_image.h>
#endif

#ifdef __LIBVPINBALL__
#include "standalone/VPinballLib.h"
#endif

namespace VPX
{

Window::Window(const int width, const int height)
   : m_settingsSection(Settings::PlayerVR)
   , m_settingsPrefix("Headset"s)
   , m_isVR(true)
{
   m_width = width;
   m_height = height;
   m_pixelWidth = width;
   m_pixelHeight = height;
   m_screenwidth = width;
   m_screenheight = height;
   m_fullscreen = true;
   //m_refreshrate;
   //m_bitdepth;
   m_sdrWhitePoint = 1.f;
   m_hdrHeadRoom = 1.f;
   m_wcgDisplay = false;
   m_wcgBackbuffer = false;
   m_backBuffer = nullptr;
}

Window::Window(const string &title, const Settings& settings, const Settings::Section section, const string &settingsPrefix)
   : m_settingsSection(section)
   , m_settingsPrefix(settingsPrefix)
   , m_isVR(false)
{
   m_fullscreen = settings.LoadValueBool(m_settingsSection, m_settingsPrefix + "FullScreen");
   // FIXME remove command line override => this is hacky and not needed anymore (use INI override instead)
   if (m_settingsSection == Settings::Player)
   {
      if (g_pvp->m_disEnableTrueFullscreen == 0)
         m_fullscreen = false;
      else if (g_pvp->m_disEnableTrueFullscreen == 1)
         m_fullscreen = true;
   }
   int w = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "Width", m_fullscreen ? -1 : 1024);
   int h = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "Height", w * 9 / 16);
   const bool video10bit = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "Render10Bit", false);
   const float fsRefreshRate = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "RefreshRate", 0.f);
   // FIXME FIXME FIXME
   const int fsBitDeth = (video10bit && m_fullscreen /* && stereo3D != STEREO_VR */) ? 30 : 32;
   if (video10bit && !m_fullscreen)
      ShowError("10Bit-Monitor support requires 'Fullscreen Mode' to be also enabled!");

   const DisplayConfig selectedDisplay = GetDisplayConfig(settings);
   int wnd_x = selectedDisplay.left;
   int wnd_y = selectedDisplay.top;
   m_screenwidth = selectedDisplay.width;
   m_screenheight = selectedDisplay.height;
   m_width = w <= 0 ? m_screenwidth : w;
   m_height = h <= 0 ? m_screenheight : h;

   // FIXME implement bit depth validation and selection (only used for DX9 10bit monitors to limit banding and dithering needs)
   m_bitdepth = fsBitDeth;

   // Validate fullscreen mode, eventually falling back to windowed mode (at desktop screen resolution)
   float validatedFSRefreshRate = -1.f;
   if (m_fullscreen)
   {
      bool fsModeExists = false;
      vector<VideoMode> allVideoModes = GetDisplayModes(selectedDisplay);
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
         PLOGE << "Requested fullscreen mode " << m_width << 'x' << m_height << " at " << fsRefreshRate << "Hz is not available. Switching to windowed mode";
         m_fullscreen = false;
         m_width = m_screenwidth;
         m_height = m_screenheight;
      }
      else
      {
         // For SDL, the display is selected through the window position
         // FIXME wnd_x = wnd_y = 0; // For DX9, the swap chain selects the display, not the window coordinates
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
         const int xn = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "WndX", wnd_x);
         const int yn = settings.LoadValueWithDefault(m_settingsSection, m_settingsPrefix + "WndY", wnd_y);
         // Only apply saved position if they fit inside a display
         int displayCount = 0;
         SDL_DisplayID* displayIDs = SDL_GetDisplays(&displayCount);
         for (int i = 0; i < displayCount; ++i)
         {
            SDL_Rect displayBounds;
            if (SDL_GetDisplayBounds(displayIDs[i], &displayBounds))
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
         SDL_free(displayIDs);
      }
   }

   uint32_t wnd_flags = 0;
   #if defined(ENABLE_OPENGL)
      wnd_flags |= SDL_WINDOW_OPENGL; // Leads to read OpenGL context hint (swapchain backbuffer format, ...)
      // SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true); // Leads SDL_CreateWindowFrom to add SDL_WINDOW_OPENGL flag
   #elif defined(ENABLE_BGFX)
      // BGFX does not need the SDL window to have the SDL_WINDOW_OPENGL / SDL_WINDOW_VULKAN / SDL_WINDOW_METAL flag (see BGFX SDL example)
      // Using these flags would lead SDL to create the swapchain while we want BGFX to do it for us
   #elif defined(ENABLE_DX9)
      // DX9 does not need any special flag either
   #endif
   wnd_flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
   #if defined(_MSC_VER) // Win32 (we use _MSC_VER since standalone also defines WIN32 for non Win32 builds)
      SDL_SetHint(SDL_HINT_FORCE_RAISEWINDOW, "1");
   #endif
   if (m_fullscreen)
      wnd_flags |= SDL_WINDOW_FULLSCREEN;

   #if !defined(_MSC_VER) // Win32 (we use _MSC_VER since standalone also defines WIN32 for non Win32 builds)
   // On Windows, always on top is not always respected and if using SDL_WINDOW_UTILITY windows may end up being hidden with no way to select and move them
   if (m_settingsSection != Settings::Player)
      wnd_flags |= SDL_WINDOW_UTILITY | SDL_WINDOW_ALWAYS_ON_TOP;
   #endif

   // Prevent full screen window from minimizing when re-arranging external windows
   SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
   SDL_PropertiesID props = SDL_CreateProperties();
   SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title.c_str());
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, wnd_x);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, wnd_y);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_width);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_height);
   #if defined(ENABLE_BGFX) && defined(__ANDROID__)
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN, true);
   #endif
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, wnd_flags);
   m_nwnd = SDL_CreateWindowWithProperties(props);
   SDL_DestroyProperties(props);

   props = SDL_GetWindowProperties(m_nwnd);
   m_wcgDisplay = SDL_GetBooleanProperty(props, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false);
   m_sdrWhitePoint = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 1.0f);
   m_hdrHeadRoom = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f);

   const SDL_DisplayMode* mode;

   if (m_fullscreen)
   {
      mode = SDL_GetWindowFullscreenMode(m_nwnd);
      m_refreshrate = mode ? mode->refresh_rate : 0;
      m_screenwidth = m_width;
      m_screenheight = m_height;
      if (fsRefreshRate > 0 && validatedFSRefreshRate != m_refreshrate) // Adjust refresh rate if needed
      {
         uint32_t format = mode ? mode->format : 0;
         bool found = false;
         int num_modes = 0;
         SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(selectedDisplay.display, &num_modes);
         if (modes) {
            for (int index = 0; index < num_modes; ++index) {
               mode = modes[index];
               if ((mode->w == m_width) && (mode->h == m_height) && (mode->refresh_rate == validatedFSRefreshRate) && (mode->format == format || format == 0))
               {
                  SDL_SetWindowFullscreenMode(m_nwnd, mode);
                  m_refreshrate = validatedFSRefreshRate;
                  found = true;
                  break;
               }
            }
            SDL_free(modes);
         }
         if (!found) {
            // Should not happen since the fullscreen values have been validated
            PLOGE << "Failed to find a display mode matching the requested refresh rate [" << validatedFSRefreshRate << ']';
         }
      }
   }
   else
   {
      mode = SDL_GetDesktopDisplayMode(selectedDisplay.display);
      m_refreshrate = mode ? mode->refresh_rate : 0;
   }
   if (mode) {
      PLOGI << "SDL display mode for " << m_width << 'x' << m_height << " window '" << m_settingsPrefix << "' : " << mode->w << 'x' << mode->h << ' ' << mode->refresh_rate << " Hz " << SDL_GetPixelFormatName(mode->format);
   }

   SDL_GetWindowSizeInPixels(m_nwnd, &m_pixelWidth, &m_pixelHeight);

   #ifdef __STANDALONE__
      const string iconPath = g_pvp->m_myPath + "assets" + PATH_SEPARATOR_CHAR + "vpinball.png";
      SDL_Surface* pIcon = IMG_Load(iconPath.c_str());
      if (pIcon) {
         SDL_SetWindowIcon(m_nwnd, pIcon);
         SDL_DestroySurface(pIcon);
      }
      else {
         PLOGE << "Failed to load window icon: " << SDL_GetError();
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
   if (m_isVR)
      return;
   SDL_DestroyWindow(m_nwnd);
}

Window::DisplayConfig Window::GetDisplayConfig(const Settings& settings) const
{
   const string selectedDisplayName = settings.LoadValueString(m_settingsSection, m_settingsPrefix + "Display");
   return GetDisplayConfig(selectedDisplayName);
}

void Window::Show(const bool show)
{
   if (m_isVR)
      return;
   if (show)
      SDL_ShowWindow(m_nwnd);
   else
      SDL_HideWindow(m_nwnd);
}

bool Window::IsVisible() const
{
   if (m_isVR)
      return true;
   return (SDL_GetWindowFlags(m_nwnd) & SDL_WINDOW_HIDDEN) == 0;
}

void Window::RaiseAndFocus()
{
   if (m_isVR)
      return;
   SDL_RaiseWindow(m_nwnd);
}

bool Window::IsFocused() const {
   if (m_isVR)
      return true;
   return m_nwnd == SDL_GetKeyboardFocus();
}

void Window::GetPos(int& x, int& y) const
{
   if (m_isVR)
   {
      x = 0;
      y = 0;
      return;
   }
   SDL_GetWindowPosition(m_nwnd, &x, &y);
}

void Window::SetPos(const int x, const int y)
{
   if (m_isVR)
      return;
   SDL_SetWindowPosition(m_nwnd, x, y);
   Settings* settings = (m_settingsSection == Settings::Player) ? &(g_pvp->m_settings) : &(g_pvp->m_ptableActive->m_settings);
   settings->SaveValue(m_settingsSection, m_settingsPrefix + "WndX", x);
   settings->SaveValue(m_settingsSection, m_settingsPrefix + "WndY", y);
}

vector<Window::DisplayConfig> Window::GetDisplays()
{
   vector<Window::DisplayConfig> displays;
   SDL_DisplayID primaryID = SDL_GetPrimaryDisplay();

   #ifdef _MSC_VER
      // Under Windows, we evaluate which display is connected to which GPU adapter using DXGI
      HRESULT hr;
      IDXGIFactory1* dxgiFactory = nullptr;
      hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&dxgiFactory));
      if (dxgiFactory != nullptr)
      {
         UINT i = 0;
         IDXGIAdapter* pAdapter;
         while (dxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
         {
            DXGI_ADAPTER_DESC adapterDesc;
            hr = pAdapter->GetDesc(&adapterDesc);
            if (SUCCEEDED(hr))
            {
               UINT j = 0;
               IDXGIOutput* pOutput;
               while (pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND)
               {
                  DXGI_OUTPUT_DESC desc;
                  hr = pOutput->GetDesc(&desc);
                  if (SUCCEEDED(hr))
                  {
                     DisplayConfig displayConf {};
                     displayConf.top = desc.DesktopCoordinates.top;
                     displayConf.left = desc.DesktopCoordinates.left;
                     displayConf.width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
                     displayConf.height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
                     SDL_Rect displayRect { displayConf.left, displayConf.top, displayConf.width, displayConf.height };
                     displayConf.display = SDL_GetDisplayForRect(&displayRect);
                     displayConf.adapter = adapterDesc.DeviceId;
                     displayConf.isPrimary = primaryID != 0 ? displayConf.display == primaryID : (displayConf.left == 0) && (displayConf.top == 0);
                     displayConf.displayName = string(SDL_GetDisplayName(displayConf.display)) + " - " + MakeString(adapterDesc.Description);
                     displays.push_back(displayConf);
                  }
                  pOutput->Release();
                  j++;
               }
            }
            pAdapter->Release();
            i++;
         }
      }

   #else
      // On other platforms, we always use the default GPU, since we do not have a cross platform way of selecting the GPU
      int i = 0;
      int displayCount = 0;
      SDL_DisplayID* displayIDs = SDL_GetDisplays(&displayCount);
      for (; i < displayCount; ++i)
      {
         SDL_Rect displayBounds;
         if (SDL_GetDisplayBounds(displayIDs[i], &displayBounds)) {
            DisplayConfig displayConf {};
            displayConf.top = displayBounds.y;
            displayConf.left = displayBounds.x;
            displayConf.width = displayBounds.w;
            displayConf.height = displayBounds.h;
            displayConf.display = displayIDs[i];
            displayConf.displayName = SDL_GetDisplayName(displayIDs[i]);
            displayConf.adapter = 0;
            displayConf.isPrimary = primaryID != 0 ? displayIDs[i] == primaryID : (displayBounds.x == 0) && (displayBounds.y == 0);
            displays.push_back(displayConf);
         }
      }
      SDL_free(displayIDs);
   #endif

   return displays;
}

vector<Window::VideoMode> Window::GetDisplayModes(const DisplayConfig& display)
{
   vector<Window::VideoMode> modes;

   int count;
   SDL_DisplayMode** displayModes = SDL_GetFullscreenDisplayModes(display.display, &count);
   for (int mode = 0; mode < count; ++mode) {
      SDL_DisplayMode* const sdlMode = displayModes[mode];
      VideoMode vmode = {};
      vmode.width = sdlMode->w;
      vmode.height = sdlMode->h;
      switch (sdlMode->format) {
      case SDL_PIXELFORMAT_RGB24:
      case SDL_PIXELFORMAT_BGR24:
      case SDL_PIXELFORMAT_XRGB8888:
      case SDL_PIXELFORMAT_RGBX8888:
      case SDL_PIXELFORMAT_XBGR8888:
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
      vmode.refreshrate = sdlMode->refresh_rate;
      modes.push_back(vmode);
   }
   SDL_free(displayModes);

   return modes;
}

Window::DisplayConfig Window::GetDisplayConfig(const string& display)
{
   bool displayFound = false;
   DisplayConfig selectedDisplay {};
   vector<DisplayConfig> displays = GetDisplays();
   for (const DisplayConfig& dispConf : displays)
   {
      if ((!displayFound && dispConf.isPrimary) // Defaults to the primary display
         || (!displayFound && dispConf.displayName == display)) // or the display selected in the settings
      {
         selectedDisplay = dispConf;
         displayFound |= dispConf.displayName == display;
      }
   }
   assert(selectedDisplay.width > 0); // We should at least have selected the default display
   return selectedDisplay;
}

#if defined(_WIN32)
HWND Window::GetNativeHWND() const
{
   return (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(GetCore()), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
}
#endif

} // namespace VPX
