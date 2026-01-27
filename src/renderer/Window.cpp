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
#include "lib/src/VPinballLib.h"
#endif

namespace VPX
{

static int GetPixelFormatDepth(SDL_PixelFormat format)
{
   switch (format)
   {
   case SDL_PIXELFORMAT_RGB24:
   case SDL_PIXELFORMAT_BGR24:
   case SDL_PIXELFORMAT_XRGB8888:
   case SDL_PIXELFORMAT_RGBX8888:
   case SDL_PIXELFORMAT_XBGR8888:
   case SDL_PIXELFORMAT_BGRX8888:
   case SDL_PIXELFORMAT_ARGB8888:
   case SDL_PIXELFORMAT_RGBA8888:
   case SDL_PIXELFORMAT_ABGR8888:
   case SDL_PIXELFORMAT_BGRA8888: return 32;
   case SDL_PIXELFORMAT_RGB565:
   case SDL_PIXELFORMAT_BGR565:
   case SDL_PIXELFORMAT_ABGR1555:
   case SDL_PIXELFORMAT_BGRA5551:
   case SDL_PIXELFORMAT_ARGB1555:
   case SDL_PIXELFORMAT_RGBA5551: return 16;
   case SDL_PIXELFORMAT_ARGB2101010: return 30;
   default: return 0;
   }
}

Window::Window(const int width, const int height)
   : m_windowId(VPXWindowId::VPXWINDOW_Playfield)
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

Window::Window(const string& title, const Settings& settings, VPXWindowId windowId)
   : m_windowId(windowId)
   , m_isVR(false)
{
   m_fullscreen = g_isMobile || settings.GetWindow_FullScreen(m_windowId);
   if (!g_isMobile && m_windowId == VPXWindowId::VPXWINDOW_Playfield)
   {
      // FIXME remove command line override => this is hacky and not needed anymore (use INI override instead)
      if (g_pvp->m_disEnableTrueFullscreen == 0)
         m_fullscreen = false;
      else if (g_pvp->m_disEnableTrueFullscreen == 1)
         m_fullscreen = true;
   }

   // Both fullscreen and windowed modes are anchored to a user selected display
   const DisplayConfig selectedDisplay = GetDisplayConfig(settings.GetWindow_Display((int)m_windowId));
   if (selectedDisplay.displayName != settings.GetWindow_Display((int)m_windowId))
   {
      PLOGW << "The selected display \"" << settings.GetWindow_Display((int)m_windowId)
            << "\" is not available. Using display \"" << selectedDisplay.displayName << "\" instead.";
   }
   int wnd_x = selectedDisplay.left;
   int wnd_y = selectedDisplay.top;
   int nDisplayModes;
   SDL_DisplayMode** displayModes = SDL_GetFullscreenDisplayModes(selectedDisplay.display, &nDisplayModes);

   // Search for the request fullscreen exclusive display mode, eventually fallback to windowed mode if we fail
   const SDL_DisplayMode* fullscreenDisplayMode = nullptr;

   const bool isExtCreatedWindow = g_isMobile && g_isIOS;
   if (m_fullscreen && !isExtCreatedWindow)
   {
      if (g_isAndroid) {
        const SDL_DisplayMode* currentMode = SDL_GetCurrentDisplayMode(selectedDisplay.display);
        if (currentMode) {
            fullscreenDisplayMode = currentMode;
            m_screenwidth = currentMode->w;
            m_screenheight = currentMode->h;
            m_width = currentMode->w;
            m_height = currentMode->h;
            m_refreshrate = currentMode->refresh_rate;
            m_bitdepth = GetPixelFormatDepth(currentMode->format);
        }
      }
      else {
          for (int mode = 0; mode < nDisplayModes; mode++) {
              const SDL_DisplayMode *const sdlMode = displayModes[mode];
              const int bitdepth = GetPixelFormatDepth((sdlMode->format));
              if ((sdlMode->w == settings.GetWindow_FSWidth(m_windowId)) && (sdlMode->h == settings.GetWindow_FSHeight(m_windowId)) //
                      && (settings.GetWindow_FSRefreshRate(m_windowId) == 0) || (sdlMode->refresh_rate == settings.GetWindow_FSRefreshRate(m_windowId)) //
                      && (settings.GetWindow_FSColorDepth(m_windowId) == 0) || (bitdepth == settings.GetWindow_FSColorDepth(m_windowId))) {
                  fullscreenDisplayMode = sdlMode;
                  m_screenwidth = sdlMode->w;
                  m_screenheight = sdlMode->h;
                  m_width = sdlMode->w;
                  m_height = sdlMode->h;
                  m_refreshrate = sdlMode->refresh_rate;
                  m_bitdepth = bitdepth;
                  break;
              }
          }
      }
      if (fullscreenDisplayMode == nullptr)
      {
         PLOGE << "Requested fullscreen mode " << settings.GetWindow_Width(m_windowId) << 'x' << settings.GetWindow_Height(m_windowId) << " at "
               << settings.GetWindow_FSRefreshRate(m_windowId) << "Hz, Bit depth: " << settings.GetWindow_FSColorDepth(m_windowId)
               << " is not available. Switching to windowed mode.";
      }
   }

   // Fullscreen failed or was not requested, setup windowed mode
   if (!m_fullscreen || fullscreenDisplayMode == nullptr)
   {
      m_screenwidth = selectedDisplay.width;
      m_screenheight = selectedDisplay.height;
      m_width = m_fullscreen ? m_screenwidth : settings.GetWindow_Width(m_windowId);
      m_height = m_fullscreen ? m_screenheight : settings.GetWindow_Height(m_windowId);
      m_refreshrate = selectedDisplay.refreshrate;
      m_bitdepth = selectedDisplay.depth;
      m_fullscreen = false;

      // Constrain window to screen
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

      // Restore saved position of non fullscreen windows (saved as a relativ eposition inside the selected display)
      if ((m_height != m_screenheight) || (m_width != m_screenwidth))
      {
         Settings::GetRegistry().Register(Settings::GetWindow_WndX_Property(m_windowId)->WithDefault(wnd_x - selectedDisplay.left));
         Settings::GetRegistry().Register(Settings::GetWindow_WndY_Property(m_windowId)->WithDefault(wnd_y - selectedDisplay.top));
         const int xn = settings.GetWindow_WndX(m_windowId);
         const int yn = settings.GetWindow_WndY(m_windowId);
         if (0 <= xn && xn + m_width <= selectedDisplay.width)
            wnd_x = selectedDisplay.left + xn;
         if (0 <= yn && yn + m_height <= selectedDisplay.height)
            wnd_y = selectedDisplay.top + yn;
      }
   }

   // Create the window
   assert(m_width > 0 && m_width <= m_screenwidth);
   assert(m_height > 0 && m_height <= m_screenheight);
   assert(selectedDisplay.left <= wnd_x);
   assert(selectedDisplay.top <= wnd_y);
   assert((wnd_x + m_width) <= (selectedDisplay.left + (m_fullscreen ? fullscreenDisplayMode->w : selectedDisplay.width))); // The fullscreen mode may have a different orientation than the display on mobile devices
   assert((wnd_y + m_height) <= (selectedDisplay.top + (m_fullscreen ? fullscreenDisplayMode->h : selectedDisplay.height)));
   if (m_refreshrate <= 0)
   {
      PLOGE << "Failed to get display refresh rate. VPX will use a 60Hz default which may be wrong and cause bad video synchronization.";
      m_refreshrate = 60;
   }
   SDL_PropertiesID props;
   if (isExtCreatedWindow)
   {
      #ifdef __LIBVPINBALL__
         m_nwnd = VPinballLib::VPinballLib::Instance().GetWindow();
      #endif
   }
   else
   {
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
      if (m_windowId != VPXWindowId::VPXWINDOW_Playfield)
         wnd_flags |= SDL_WINDOW_UTILITY | SDL_WINDOW_ALWAYS_ON_TOP;
      #endif

      // Prevent full screen window from minimizing when re-arranging external windows
      SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

      props = SDL_CreateProperties();
      SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title.c_str());
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, wnd_x);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, wnd_y);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_width);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_height);
      SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, wnd_flags);
      if (g_isAndroid)
         SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN, true);
      m_nwnd = SDL_CreateWindowWithProperties(props);
      SDL_DestroyProperties(props);
   }

   props = SDL_GetWindowProperties(m_nwnd);
   m_wcgDisplay = SDL_GetBooleanProperty(props, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false);
   m_sdrWhitePoint = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 1.0f);
   m_hdrHeadRoom = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f);

   // Switch to request fullscreen display mode (must be done after window creation)
   if (fullscreenDisplayMode)
      SDL_SetWindowFullscreenMode(m_nwnd, fullscreenDisplayMode);
   SDL_free(displayModes);

   SDL_GetWindowSizeInPixels(m_nwnd, &m_pixelWidth, &m_pixelHeight);

   if (auto icon = BaseTexture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "vpinball.png").string()); icon)
   {
      SDL_Surface* pSurface = icon->ToSDLSurface();
      if (pSurface)
      {
         SDL_SetWindowIcon(m_nwnd, pSurface);
         SDL_DestroySurface(pSurface);
      }
   }
   else {
      PLOGE << "Failed to load window icon: " << SDL_GetError();
   }

   const SDL_DisplayMode* const displayMode = SDL_GetDesktopDisplayMode(selectedDisplay.display);
   if (displayMode) {
      PLOGI << "Window #" << m_windowId << " (" << m_width << 'x' << m_height << ") was created on display " << selectedDisplay.displayName 
         << " [" << displayMode->w << 'x' << displayMode->h << ' ' << displayMode->refresh_rate << "Hz " << SDL_GetPixelFormatName(displayMode->format) << ']';
   }
}

Window::~Window()
{
   if (m_isVR)
      return;

   if (!g_isIOS)
      SDL_RunOnMainThread([](void* userdata) { SDL_DestroyWindow(static_cast<SDL_Window*>(userdata)); }, m_nwnd, true);
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
}

void Window::SetSize(const int x, const int y)
{
   if (m_isVR)
      return;
   if (x == GetWidth() && y == GetHeight())
      return;
   SDL_SetWindowSize(m_nwnd, x, y);
   SDL_GetWindowSize(m_nwnd, &m_width, &m_height);
   SDL_GetWindowSizeInPixels(m_nwnd, &m_pixelWidth, &m_pixelHeight);
   #ifdef ENABLE_BGFX
   // The RenderDevice automatically manages the backbuffer resize. For ancillary windows (BGFX only), we need to recreate the swapchain
   if (m_backBuffer && g_pplayer && g_pplayer->m_playfieldWnd != this)
   {
      g_pplayer->m_renderer->m_renderDevice->AddEndOfFrameCmd(
         [this]()
         {
            g_pplayer->m_renderer->m_renderDevice->RemoveWindow(this);
            delete m_backBuffer;
            m_backBuffer = nullptr;
            // We must destroy the swapchain before attaching a new swapchain, so flush and flip now
            g_pplayer->m_renderer->m_renderDevice->SubmitAndFlipFrame();
            g_pplayer->m_renderer->m_renderDevice->AddWindow(this);
         });
   }
   #endif
}

vector<Window::DisplayConfig> Window::GetDisplays()
{
   vector<Window::DisplayConfig> displays;
   const SDL_DisplayID primaryID = SDL_GetPrimaryDisplay();

   int i = 0;
   int displayCount = 0;
   SDL_DisplayID* displayIDs = SDL_GetDisplays(&displayCount);
   for (; i < displayCount; ++i)
   {
      SDL_Rect displayBounds;
      if (SDL_GetDisplayBounds(displayIDs[i], &displayBounds)) {
         DisplayConfig displayConf {};
         displayConf.display = displayIDs[i];
         displayConf.displayName = SDL_GetDisplayName(displayIDs[i]);
         displayConf.top = displayBounds.y;
         displayConf.left = displayBounds.x;
         displayConf.width = displayBounds.w;
         displayConf.height = displayBounds.h;
         displayConf.isPrimary = primaryID != 0 ? displayIDs[i] == primaryID : (displayBounds.x == 0) && (displayBounds.y == 0);
         const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(displayIDs[i]);
         displayConf.depth = GetPixelFormatDepth(mode->format);
         displayConf.refreshrate = mode->refresh_rate;
         displays.push_back(displayConf);
      }
   }
   SDL_free(displayIDs);

   return displays;
}

vector<Window::VideoMode> Window::GetDisplayModes(const DisplayConfig& display)
{
   vector<Window::VideoMode> modes;

   int count;
   SDL_DisplayMode** displayModes = SDL_GetFullscreenDisplayModes(display.display, &count);
   for (int mode = 0; mode < count; ++mode) {
      const SDL_DisplayMode* const sdlMode = displayModes[mode];
      VideoMode vmode = {};
      vmode.width = sdlMode->w;
      vmode.height = sdlMode->h;
      vmode.depth = GetPixelFormatDepth((sdlMode->format));
      vmode.refreshrate = sdlMode->refresh_rate;
      modes.push_back(vmode);
   }
   SDL_free(displayModes);

   return modes;
}

Window::DisplayConfig Window::GetDisplayConfig(const string& display)
{
   DisplayConfig selectedDisplay {};
   vector<DisplayConfig> displays = GetDisplays();
   for (const DisplayConfig& dispConf : displays)
   {
      if (dispConf.displayName == display) // or the display selected in the settings
      {
         selectedDisplay = dispConf;
         if (dispConf.displayName == display)
            break;
      }
      if (dispConf.isPrimary) // Defaults to the primary display
         selectedDisplay = dispConf;
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



RenderOutput::RenderOutput(VPXWindowId windowId)
   : m_windowId(windowId)
   , m_mode(OM_DISABLED)
{
}

RenderOutput::OutputMode RenderOutput::GetMode() const { return m_mode; }

void RenderOutput::SetMode(const Settings& settings, OutputMode mode)
{
#ifdef ENABLE_BGFX
   constexpr bool isSingleView = g_isMobile;
#else
   constexpr bool isSingleView = true;
#endif
   std::unique_ptr<Window> prevWindow;
   if (mode == OM_WINDOW && isSingleView)
      m_mode = OM_EMBEDDED;
   m_mode = mode;
   switch (m_mode)
   {
   case OM_DISABLED:
      m_embeddedWindow = nullptr;
      prevWindow = std::move(m_window);
      m_window = nullptr;
      break;

   case OM_EMBEDDED:
      prevWindow = std::move(m_window);
      m_window = nullptr;
      if (m_embeddedWindow == nullptr)
      {
         int x = settings.GetWindow_WndX(m_windowId);
         int y = settings.GetWindow_WndY(m_windowId);
         int width = settings.GetWindow_Width(m_windowId);
         int height = settings.GetWindow_Height(m_windowId);
         width = min(width, g_pplayer->m_playfieldWnd->GetWidth());
         height = min(height, g_pplayer->m_playfieldWnd->GetHeight());
         x = min(x, g_pplayer->m_playfieldWnd->GetWidth() - width);
         y = min(y, g_pplayer->m_playfieldWnd->GetHeight() - height);
         m_embeddedWindow = std::make_unique<EmbeddedWindow>(x, y, width, height);
      }
      break;

   case OM_WINDOW:
      m_embeddedWindow = nullptr;
      if (m_mode == OM_WINDOW && m_window == nullptr)
      {
         m_window = std::make_unique<Window>(m_windowId == VPXWindowId::VPXWINDOW_Playfield ? "Visual Pinball Player"s
               : m_windowId == VPXWindowId::VPXWINDOW_Backglass                             ? "Visual Pinball Backglass"s
               : m_windowId == VPXWindowId::VPXWINDOW_ScoreView                             ? "Visual Pinball Score View"s
               : m_windowId == VPXWindowId::VPXWINDOW_Topper                                ? "Visual Pinball Topper"s
                                                                                            : "Visual Pinball VR Preview"s,
            settings, m_windowId);
      }
      break;
   }
   if (prevWindow && g_pplayer)
   {
      // Delete window's backbuffer and window at the end of the frame to avoid issues if they are still in use
      g_pplayer->m_renderer->m_renderDevice->AddEndOfFrameCmd(
         [wnd = prevWindow.release()]() mutable
         {
            delete wnd->GetBackBuffer();
            delete wnd;
         });
   }
}

Window* RenderOutput::GetWindow() const { return m_window.get(); }

EmbeddedWindow* RenderOutput::GetEmbeddedWindow() const { return m_embeddedWindow.get(); }

int RenderOutput::GetWidth() const
{
   if (m_mode == OM_EMBEDDED)
      return m_embeddedWindow->GetWidth();
   else if (m_mode == OM_WINDOW)
      return m_window->GetPixelWidth();
   else
      return 0;
}

void RenderOutput::SetWidth(int v) const
{
   if (m_mode == OM_EMBEDDED)
      m_embeddedWindow->SetWidth(v);
   else
      assert(false);
}

int RenderOutput::GetHeight() const
{
   if (m_mode == OM_EMBEDDED)
      return m_embeddedWindow->GetHeight();
   else if (m_mode == OM_WINDOW)
      return m_window->GetPixelHeight();
   else
      return 0;
}

void RenderOutput::SetHeight(int v) const
{
   if (m_mode == OM_EMBEDDED)
      m_embeddedWindow->SetHeight(v);
   else
      assert(false);
}

void RenderOutput::GetPos(int& x, int& y) const
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

void RenderOutput::SetPos(int x, int y) const
{
   if (m_mode == OM_EMBEDDED)
      m_embeddedWindow->SetPos(x, y);
   else if (m_mode == OM_WINDOW)
      m_window->SetPos(x, y);
}

} // namespace VPX
