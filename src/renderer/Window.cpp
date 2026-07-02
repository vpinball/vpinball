// license:GPLv3+

#include "core/stdafx.h"
#include "Window.h"

#include "core/VPApp.h"
#include "parts/Collection.h"
#include "renderer/Renderer.h"

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
   m_windowMode = WindowMode::BorderlessFullscreen;
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
   m_windowMode = (WindowMode) settings.GetWindow_FullScreen(m_windowId);
   if (g_isMobile)
      m_windowMode = WindowMode::BorderlessFullscreen;
   
   // Both fullscreen and windowed modes are anchored to a user selected display
   const string configuredDisplay = settings.GetWindow_Display((int)m_windowId);
   const DisplayConfig selectedDisplay = GetDisplayConfig(configuredDisplay);
   if (configuredDisplay.empty())
   {
      PLOGI << "No display configured. Using display \"" << selectedDisplay.displayName << "\".";
   }
   else if (selectedDisplay.displayName != configuredDisplay)
   {
      PLOGW << "The selected display \"" << configuredDisplay << "\" is not available. Using display \"" << selectedDisplay.displayName << "\" instead.";
   }
   int wnd_x = selectedDisplay.left;
   int wnd_y = selectedDisplay.top;

   // Search for the request fullscreen exclusive display mode, eventually falling back to windowed mode
   const SDL_DisplayMode* fullscreenDisplayMode = nullptr;
   if (m_windowMode == WindowMode::ExclusiveFullscreen)
   {
      int nDisplayModes;
      SDL_DisplayMode** displayModes = SDL_GetFullscreenDisplayModes(selectedDisplay.display, &nDisplayModes);
      const int requestedW = settings.GetWindow_FSWidth(m_windowId); // Pixel size
      const int requestedH = settings.GetWindow_FSHeight(m_windowId);
      const float requestedHz = settings.GetWindow_FSRefreshRate(m_windowId);
      const int requestedDepth = settings.GetWindow_FSColorDepth(m_windowId);
      for (int mode = 0; mode < nDisplayModes; mode++)
      {
         const SDL_DisplayMode* const sdlMode = displayModes[mode];
         const VideoMode videomode = SDLtoVPXVideoMode(sdlMode);
         if ((videomode.GetPixelWidth() == requestedW) && (videomode.GetPixelHeight() == requestedH) //
            && ((requestedHz == 0.f) || (videomode.refreshrate == requestedHz)) //
            && ((requestedDepth == 0) || (videomode.depth == requestedDepth)))
         {
            fullscreenDisplayMode = sdlMode;
            m_screenwidth = videomode.GetPixelWidth();
            m_screenheight = videomode.GetPixelHeight();
            m_refreshrate = videomode.refreshrate;
            m_bitdepth = videomode.depth;
            m_width = videomode.width;
            m_height = videomode.height;
            break;
         }
      }
      if (fullscreenDisplayMode == nullptr)
      {
         PLOGE << std::format("Requested fullscreen mode {}x{} at {}Hz, Bit depth: {} is not available. Switching to windowed mode.", requestedW, requestedH, requestedHz, requestedDepth);
         m_windowMode = WindowMode::BorderlessFullscreen;
      }
      SDL_free(displayModes);
   }

   // Setup windowed mode
   if (m_windowMode != WindowMode::ExclusiveFullscreen)
   {
      m_screenwidth = selectedDisplay.videomode.GetPixelWidth();
      m_screenheight = selectedDisplay.videomode.GetPixelHeight();
      m_refreshrate = selectedDisplay.videomode.refreshrate;
      m_bitdepth = selectedDisplay.videomode.depth;
      if (m_windowMode == WindowMode::BorderlessFullscreen)
      {
         m_width = selectedDisplay.videomode.width;
         m_height = selectedDisplay.videomode.height;
      }
      else
      {
         m_width = static_cast<int>(roundf(static_cast<float>(settings.GetWindow_Width(m_windowId)) / selectedDisplay.videomode.pixelDensity));
         m_height = static_cast<int>(roundf(static_cast<float>(settings.GetWindow_Height(m_windowId)) / selectedDisplay.videomode.pixelDensity));
      }

      // Constrain window to screen
      if (m_width > selectedDisplay.videomode.width)
      {
         m_height = (m_height * selectedDisplay.videomode.width) / m_width;
         m_width = selectedDisplay.videomode.width;
      }
      if (m_height > selectedDisplay.videomode.height)
      {
         m_width = (m_width * selectedDisplay.videomode.height) / m_height;
         m_height = selectedDisplay.videomode.height;
      }
      wnd_x += (selectedDisplay.videomode.width - m_width) / 2;
      wnd_y += (selectedDisplay.videomode.height - m_height) / 2;

      // Restore saved position of non fullscreen windows (saved as a relative position inside the selected display)
      if ((m_height != selectedDisplay.videomode.height) || (m_width != selectedDisplay.videomode.width))
      {
         Settings::GetRegistry().Register(Settings::GetWindow_WndX_Property(m_windowId)
               ->WithDefault(static_cast<int>(roundf(static_cast<float>((wnd_x - selectedDisplay.left) * selectedDisplay.videomode.pixelDensity)))));
         Settings::GetRegistry().Register(
            Settings::GetWindow_WndY_Property(m_windowId)->WithDefault(static_cast<int>(roundf(static_cast<float>((wnd_y - selectedDisplay.top) * selectedDisplay.videomode.pixelDensity)))));
         const int xn = static_cast<int>(roundf(static_cast<float>(settings.GetWindow_WndX(m_windowId)) / selectedDisplay.videomode.pixelDensity));
         const int yn = static_cast<int>(roundf(static_cast<float>(settings.GetWindow_WndY(m_windowId)) / selectedDisplay.videomode.pixelDensity));
         if (0 <= xn && xn + m_width <= selectedDisplay.videomode.width)
            wnd_x = selectedDisplay.left + xn;
         if (0 <= yn && yn + m_height <= selectedDisplay.videomode.height)
            wnd_y = selectedDisplay.top + yn;
      }
   }

   if (m_refreshrate <= 0)
   {
      PLOGE << "Failed to get display refresh rate. VPX will use a 60Hz default which may be wrong and cause bad video synchronization.";
      m_refreshrate = 60;
   }

   // Create the window
   assert(selectedDisplay.left <= wnd_x);
   assert(selectedDisplay.top <= wnd_y);
   assert(fullscreenDisplayMode || (m_width > 0 && m_width <= selectedDisplay.videomode.width));
   assert(fullscreenDisplayMode || (m_height > 0 && m_height <= selectedDisplay.videomode.height));
   assert(fullscreenDisplayMode || (wnd_x + m_width) <= (selectedDisplay.left + selectedDisplay.videomode.width));
   assert(fullscreenDisplayMode || (wnd_y + m_height) <= (selectedDisplay.top + selectedDisplay.videomode.height));
   SDL_PropertiesID props;
   if (g_isMobile && g_isIOS) // Window is already externally created
   {
      #ifdef __LIBVPINBALL__
         m_nwnd = VPinballLib::VPinballLib::Instance().GetWindow();
      #endif
   }
   else
   {
      uint32_t wnd_flags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
      // So far, the only way to get a clean focus management on all platforms with fullscreen/windowed mode
      // is to make ancillary windows (backglass, score view, topper) output only. They must never grab input
      // focus, otherwise showing them (eventually lazily, when the script starts feeding them content) would
      // steal focus from the playfield and pause the table.
      if (m_windowId != VPXWindowId::VPXWINDOW_Playfield && m_windowId != VPXWindowId::VPXWINDOW_VRPreview)
         wnd_flags |= SDL_WINDOW_NOT_FOCUSABLE;

      #if defined(ENABLE_OPENGL)
         wnd_flags |= SDL_WINDOW_OPENGL; // Leads to read OpenGL context hint (swapchain backbuffer format, ...)
         // SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true); // Leads SDL_CreateWindowFrom to add SDL_WINDOW_OPENGL flag
      #elif defined(ENABLE_BGFX)
         // BGFX does not need the SDL window to have the SDL_WINDOW_OPENGL / SDL_WINDOW_VULKAN / SDL_WINDOW_METAL flag (see BGFX SDL example)
         // Using these flags would lead SDL to create the swapchain while we want BGFX to do it for us
      #elif defined(ENABLE_DX9)
         // DX9 does not need any special flag either
      #endif

      // Request forced raising (standard behavior except on Windows)
      SDL_SetHint(SDL_HINT_FORCE_RAISEWINDOW, "1");

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

   // On Wayland applications may not position windows themselves, so the drag support of
   // the display settings UI (see Player::ProcessOSMessages) cannot work; declaring the
   // undecorated floating ancillary windows draggable lets the window manager move them
   // when dragged anywhere instead. Other platforms keep the settings UI drag (a full
   // surface hit test would swallow the mouse events it relies on, and make the windows
   // movable outside of the settings UI, e.g. by stray touches on a cabinet).
   if (m_windowId != VPXWindowId::VPXWINDOW_Playfield && m_windowMode == Windowed && SDL_GetCurrentVideoDriver() == "wayland"sv)
      SDL_SetWindowHitTest(m_nwnd, [](SDL_Window*, const SDL_Point*, void*) -> SDL_HitTestResult { return SDL_HITTEST_DRAGGABLE; }, nullptr);

   props = SDL_GetWindowProperties(m_nwnd);
   m_wcgDisplay = SDL_GetBooleanProperty(props, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false);
   m_sdrWhitePoint = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 1.0f);
   m_hdrHeadRoom = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f);

   // Define exclusive fullscreen mode if any, then switch to fullscreen
   SDL_SetWindowFullscreenMode(m_nwnd, fullscreenDisplayMode);
   if (m_windowMode != WindowMode::Windowed)
      SDL_SetWindowFullscreen(m_nwnd, true);
   if (m_windowMode != WindowMode::ExclusiveFullscreen)
      SDL_SyncWindow(m_nwnd); // Wait for mode switch before gathering the window pixel size

   SDL_GetWindowSizeInPixels(m_nwnd, &m_pixelWidth, &m_pixelHeight);
   m_pixelDensity = SDL_GetWindowPixelDensity(m_nwnd);
   if (m_pixelDensity == 0.f)
   {
      PLOGE << "Failed to get pixel density, defaulting to 1";
      m_pixelDensity = 1.f;
   }

   if (auto icon = BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "vpinball.png")); icon)
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

   // Check if the platform allows positioning windows (as Wayland forbids it...)
   {
      int x, y;
      m_isPositioningSupported = true;
      m_isPositioningSupported &= SDL_GetWindowPosition(m_nwnd, &x, &y);
      m_isPositioningSupported &= SDL_SetWindowPosition(m_nwnd, x, y);
   }

   if (const SDL_DisplayMode* const displayMode = SDL_GetDesktopDisplayMode(selectedDisplay.display); displayMode)
   {
      PLOGI << std::format("Window #{} ({}x{}) was created on display {} [{}x{} {}Hz {}]", (int)m_windowId, m_pixelWidth, m_pixelHeight, selectedDisplay.displayName.c_str(),
         selectedDisplay.videomode.GetPixelWidth(), selectedDisplay.videomode.GetPixelHeight(), selectedDisplay.videomode.refreshrate, SDL_GetPixelFormatName(displayMode->format));
   }
}

Window::~Window()
{
   if (!m_isVR && !(g_isMobile && g_isIOS))
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
   if (m_windowMode != WindowMode::Windowed) // When window loose focus, it may loose its fullscreen state, so restore it
      SDL_SetWindowFullscreen(m_nwnd, true);
}

bool Window::IsFocused() const {
   if (m_isVR)
      return true;
   return m_nwnd == SDL_GetKeyboardFocus();
}

void Window::SetFocusable(const bool focusable)
{
   if (m_isVR)
      return;
   SDL_SetWindowFocusable(m_nwnd, focusable);
}
 
void Window::GetPos(int& x, int& y)const
{
   if (m_isVR)
   {
      x = 0;
      y = 0;
      return;
   }
   SDL_GetWindowPosition(m_nwnd, &x, &y);
}

void Window::GetPixelPos(int& x, int& y) const
{
   GetPos(x, y);
   x = LogicalToPixel(x);
   y = LogicalToPixel(y);
}

void Window::SetPos(const int x, const int y)
{
   if (m_isVR)
      return;
   SDL_SetWindowPosition(m_nwnd, x, y);
}

void Window::SetPixelPos(const int x, const int y) { SetPos(PixelToLogical(x), PixelToLogical(y)); }

void Window::SetSize(const int x, const int y)
{
   if (m_isVR)
      return;
   if (x == GetWidth() && y == GetHeight())
      return;
   SDL_SetWindowSize(m_nwnd, x, y);
}

void Window::SetPixelSize(const int w, const int h) { SetSize(PixelToLogical(w), PixelToLogical(h)); }

void Window::OnResized()
{
   if (m_isVR)
      return;
   SDL_GetWindowSize(m_nwnd, &m_width, &m_height);
   SDL_GetWindowSizeInPixels(m_nwnd, &m_pixelWidth, &m_pixelHeight);
}

VPX::Window::VideoMode Window::SDLtoVPXVideoMode(const SDL_DisplayMode* mode)
{
   VPX::Window::VideoMode videomode;
   videomode.pixelDensity = mode->pixel_density;
   videomode.width = mode->w;
   videomode.height = mode->h;
   videomode.depth = GetPixelFormatDepth(mode->format);
   videomode.refreshrate = mode->refresh_rate;
   return videomode;
}

vector<Window::DisplayConfig> Window::GetDisplays()
{
   vector<Window::DisplayConfig> displays;
   const SDL_DisplayID primaryID = SDL_GetPrimaryDisplay();

   int displayCount = 0;
   SDL_DisplayID* displayIDs = SDL_GetDisplays(&displayCount);
   for (int i = 0; i < displayCount; ++i)
   {
      SDL_Rect displayBounds;
      if (const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(displayIDs[i]); SDL_GetDisplayBounds(displayIDs[i], &displayBounds) && mode != nullptr)
      {
         DisplayConfig displayConf {};
         displayConf.display = displayIDs[i];
         displayConf.displayName = SDL_GetDisplayName(displayIDs[i]);
         displayConf.left = displayBounds.x; // Logical position
         displayConf.top = displayBounds.y;
         displayConf.isPrimary = primaryID != 0 ? displayIDs[i] == primaryID : (displayBounds.x == 0) && (displayBounds.y == 0);
         displayConf.videomode = SDLtoVPXVideoMode(mode);
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
   for (int mode = 0; mode < count; ++mode)
      modes.push_back(SDLtoVPXVideoMode(displayModes[mode]));
   SDL_free(displayModes);
   return modes;
}

Window::DisplayConfig Window::GetDisplayConfig(const string& display)
{
   DisplayConfig selectedDisplay {};
   vector<DisplayConfig> displays = GetDisplays();
   for (const DisplayConfig& dispConf : displays)
   {
      if (dispConf.displayName == display) // Defaults to the display selected in the settings
      {
         selectedDisplay = dispConf;
         break;
      }
      if (dispConf.isPrimary) // Otherwise, try to default to the primary display
         selectedDisplay = dispConf;
   }
   assert(selectedDisplay.videomode.width > 0); // We should at least have selected the default display
   return selectedDisplay;
}

void Window::SetBackBuffer(RenderTarget* rt, const bool wcgBackbuffer)
{
   assert(rt == nullptr || (rt->GetWidth() == m_pixelWidth && rt->GetHeight() == m_pixelHeight));
   m_backBuffer = rt;
   m_wcgBackbuffer = wcgBackbuffer;
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
