// license:GPLv3+

#include "core/stdafx.h"
#include "DisplaySettingsPage.h"

namespace VPX::InGameUI
{

template <typename... Args> static std::string string_format(const char* const format, Args... args)
{
   int size_s = std::snprintf(nullptr, 0, format, args...) + 1; // Extra space for '\0'
   if (size_s <= 0)
   {
      throw std::runtime_error("Error during formatting.");
   }
   auto size = static_cast<size_t>(size_s);
   std::unique_ptr<char[]> buf(new char[size]);
   std::snprintf(buf.get(), size, format, args...);
   return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

static constexpr std::array<int2, 13> aspectRatios {
   int2 { 0, 0 }, // Free
   int2 { 4, 3 }, // [Landscape]
   int2 { 16, 10 }, //
   int2 { 16, 9 }, //
   int2 { 21, 10 }, //
   int2 { 21, 9 }, //
   int2 { 4, 1 }, // For DMD
   int2 { 3, 4 }, // [Portrait]
   int2 { 10, 16 }, //
   int2 { 9, 16 }, //
   int2 { 10, 21 }, //
   int2 { 9, 21 }, //
   int2 { 1, 4 }, // For DMD
};


DisplayHomePage::DisplayHomePage()
   : InGameUIPage("Display Settings"s, ""s, SaveMode::None)
{
   // On Phone platform, the main display is always the device screen
   if (!g_isMobile)
   {
      if (m_player->m_vrDevice)
      {
         m_player->m_liveUI->m_inGameUI.AddPage("settings/display_vr_preview"s, []() { return std::make_unique<DisplaySettingsPage>(VPXWindowId::VPXWINDOW_VRPreview); });
         AddItem(std::make_unique<InGameUIItem>("VR PReview Display"s, ""s, "settings/display_vr_preview"s));
      }
      else
      {
         m_player->m_liveUI->m_inGameUI.AddPage("settings/display_playfield"s, []() { return std::make_unique<DisplaySettingsPage>(VPXWindowId::VPXWINDOW_Playfield); });
         AddItem(std::make_unique<InGameUIItem>("Playfield Display"s, ""s, "settings/display_playfield"s));
      }
   }

   m_player->m_liveUI->m_inGameUI.AddPage("settings/display_backglass"s, []() { return std::make_unique<DisplaySettingsPage>(VPXWindowId::VPXWINDOW_Backglass); });
   AddItem(std::make_unique<InGameUIItem>("Backglass Display"s, ""s, "settings/display_backglass"s));

   m_player->m_liveUI->m_inGameUI.AddPage("settings/display_scoreview"s, []() { return std::make_unique<DisplaySettingsPage>(VPXWindowId::VPXWINDOW_ScoreView); });
   AddItem(std::make_unique<InGameUIItem>("ScoreView Display"s, ""s, "settings/display_scoreview"s));

   m_player->m_liveUI->m_inGameUI.AddPage("settings/display_topper"s, []() { return std::make_unique<DisplaySettingsPage>(VPXWindowId::VPXWINDOW_Topper); });
   AddItem(std::make_unique<InGameUIItem>("Topper Display"s, ""s, "settings/display_topper"s));
}


DisplaySettingsPage::DisplaySettingsPage(VPXWindowId wndId)
   : InGameUIPage(wndId == VPXWindowId::VPXWINDOW_Playfield ? "Playfield Display Settings"s
           : wndId == VPXWindowId::VPXWINDOW_VRPreview      ? "VR Preview Display Settings"s
           : wndId == VPXWindowId::VPXWINDOW_Backglass      ? "Backglass Display Settings"s
           : wndId == VPXWindowId::VPXWINDOW_ScoreView      ? "ScoreView Display Settings"s
                                                            : "Topper Display Settings"s,
        "Adjust display mode, size and position"s, SaveMode::Both)
   , m_wndId(wndId)
   , m_isMainWindow(wndId == VPXWindowId::VPXWINDOW_Playfield || wndId == VPXWindowId::VPXWINDOW_VRPreview)
{
   m_displays = VPX::Window::GetDisplays();
   for (const auto& display : m_displays)
      m_displayNames.push_back((display.isPrimary ? '*' : ' ') + std::to_string(display.width) + 'x' + std::to_string(display.height) + " [" + display.displayName + ']');

   // Identify initial AR lock
   double ar;
   if (m_wndId == VPXWindowId::VPXWINDOW_Playfield || m_wndId == VPXWindowId::VPXWINDOW_VRPreview)
      ar = static_cast<double>(m_player->m_playfieldWnd->GetWidth()) / static_cast<double>(m_player->m_playfieldWnd->GetHeight());
   else
      ar = static_cast<double>(GetOutput(m_wndId).GetWidth()) / static_cast<double>(GetOutput(m_wndId).GetHeight());
   double best = 0.1; // Select up to 10% away
   for (size_t j = 1; j < std::size(aspectRatios); j++)
   {
      const double fit = abs(1. - ((double)(aspectRatios[j].x) / (double)(aspectRatios[j].y)) / ar);
      if (fit < best)
      {
         best = fit;
         m_arLock = static_cast<int>(j);
      }
   }

   BuildPage();
}

void DisplaySettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void DisplaySettingsPage::OnStaticRenderDirty()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->InitLayout();
}

VPX::RenderOutput& DisplaySettingsPage::GetOutput(VPXWindowId wndId)
{
   switch (wndId)
   {
   case VPXWINDOW_Backglass: return m_player->m_backglassOutput;
   case VPXWINDOW_ScoreView: return m_player->m_scoreViewOutput;
   case VPXWINDOW_Topper: return m_player->m_topperOutput;
   default: assert(false); return m_player->m_backglassOutput;
   }
}

void DisplaySettingsPage::BuildPage()
{
#ifdef ENABLE_BGFX
   constexpr bool isSingleView = g_isMobile;
#else
   constexpr bool isSingleView = true;
#endif

   ClearItems();
   if (m_wndId == VPXWindowId::VPXWINDOW_Playfield || m_wndId == VPXWindowId::VPXWINDOW_VRPreview)
   {
      BuildWindowPage();
   }
   else if (isSingleView)
   { // For mobile and builds without multiple viewport support, only disabled/embedded mode are supported
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::BoolPropertyDef(""s, ""s, "Enable"s, "Enable rendering this display"s, false, false), //
         [this]() { return GetOutput(m_wndId).GetMode() != VPX::RenderOutput::OM_DISABLED; }, // Live
         [this](Settings& settings) { return settings.GetWindow_Mode(m_wndId) != VPX::RenderOutput::OM_DISABLED; }, // Stored
         [this](bool v)
         {
            GetOutput(m_wndId).SetMode(m_player->m_ptable->m_settings, v ? VPX::RenderOutput::OM_EMBEDDED : VPX::RenderOutput::OM_DISABLED);
            BuildPage();
         }, //
         [this](Settings& settings) { settings.ResetWindow_Mode(m_wndId); }, //
         [this](bool v, Settings& settings, bool asTableOverride)
         { settings.SetWindow_Mode(m_wndId, v ? VPX::RenderOutput::OM_EMBEDDED : VPX::RenderOutput::OM_DISABLED, asTableOverride); }));
      if (GetOutput(m_wndId).GetMode() != VPX::RenderOutput::OM_DISABLED)
         BuildEmbeddedPage();
   }
   else
   {
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propWindow_Mode[m_wndId], //
         [this]() { return GetOutput(m_wndId).GetMode(); },
         [this](int, int v)
         {
            const bool windowWasCreated = GetOutput(m_wndId).GetMode() == VPX::RenderOutput::OM_WINDOW;
            if (windowWasCreated && v != VPX::RenderOutput::OM_WINDOW)
               m_player->m_renderer->m_renderDevice->RemoveWindow(GetOutput(m_wndId).GetWindow());
            GetOutput(m_wndId).SetMode(m_player->m_ptable->m_settings, static_cast<VPX::RenderOutput::OutputMode>(v));
            if (!windowWasCreated && v == VPX::RenderOutput::OM_WINDOW)
            {
               m_player->m_renderer->m_renderDevice->AddWindow(GetOutput(m_wndId).GetWindow());
               GetOutput(m_wndId).GetWindow()->Show();
            }
            BuildPage();
         })).m_excludeFromDefault = true;
      switch (GetOutput(m_wndId).GetMode())
      {
      case VPX::RenderOutput::OM_WINDOW: BuildWindowPage(); break;
      case VPX::RenderOutput::OM_EMBEDDED: BuildEmbeddedPage(); break;
      default: break;
      }
   }
}

void DisplaySettingsPage::BuildWindowPage()
{
   int wndDisplay; // Index of the display hosting the edited window
   SDL_Point wndPos; // Relative position of the window inside the display (0,0 is top,left)
   SDL_Point wndSize;
   {
      const Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
      wnd->GetPos(wndPos.x, wndPos.y);
      wndPos.x += wnd->GetWidth() / 2;
      wndPos.y += wnd->GetHeight() / 2;
      SDL_DisplayID displayId = SDL_GetDisplayForPoint(&wndPos);
      auto it = std::ranges::find_if(m_displays, [&displayId](const Window::DisplayConfig& display) { return display.display == displayId; });
      wndDisplay = it == m_displays.end() ? 0 : (int)std::distance(m_displays.begin(), it);
      wnd->GetPos(wndPos.x, wndPos.y);
      wndPos.x -= m_displays[wndDisplay].left;
      wndPos.y -= m_displays[wndDisplay].top;
      wndSize.x = wnd->GetWidth();
      wndSize.y = wnd->GetHeight();
   }

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Display"s, "Select the display output"s, false, 0, 0, m_displayNames), //
      [wndDisplay]() { return wndDisplay; }, // Live
      [this](Settings& settings)
      {
         const string name = settings.GetWindow_Display(m_wndId);
         auto it = std::ranges::find_if(m_displays, [&name](const Window::DisplayConfig& display) { return display.displayName == name; });
         const int storedDisplay = it == m_displays.end() ? 0 : (int)std::distance(m_displays.begin(), it);
         return storedDisplay;
      }, // Stored
      [this](int, int v)
      {
         Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
         SDL_Point size;
         size.x = wnd->GetWidth();
         size.y = wnd->GetHeight();
         if (!m_isMainWindow)
         { // Adjust window size if it does not fit in the new display
            // FIXME implement for main window (not resizable at runtime for the time being)
            if (size.x > m_displays[v].width)
            {
               size.y = (size.y * m_displays[v].width) / size.x;
               size.x = m_displays[v].width;
            }
            if (size.y > m_displays[v].height)
            {
               size.x = (size.x * m_displays[v].height) / size.y;
               size.y = m_displays[v].height;
            }
            wnd->SetSize(size.x, size.y);
         }
         wnd->SetPos(m_displays[v].left + (m_displays[v].width - size.x) / 2, m_displays[v].top + (m_displays[v].height - size.y) / 2);
         BuildPage();
      }, //
      [this](Settings& settings) { settings.ResetWindow_Display(m_wndId); }, //
      [this](int v, Settings& settings, bool asTableOverride) { settings.SetWindow_Display(m_wndId, m_displays[v].displayName, asTableOverride); }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propWindow_FullScreen[m_wndId], //
      [this]() { return m_player->m_ptable->m_settings.GetWindow_FullScreen(m_wndId); }, //
      [this](bool v)
      {
         m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
         m_player->m_ptable->m_settings.SetWindow_FullScreen(m_wndId, v, false);
         BuildPage();
      })).m_excludeFromDefault = true;

   const bool isFullScreen = m_player->m_ptable->m_settings.GetWindow_FullScreen(m_wndId);
   if (isFullScreen)
   {
      int defaultMode = 0, selectedMode = 0, i = 0;
      vector<Window::VideoMode> modes = VPX::Window::GetDisplayModes(m_displays[wndDisplay]);
      vector<string> modeNames;
      for (const auto& mode : modes)
      {
         double best = DBL_MAX;
         int2 bestAR;
         for (const int2& aspectRatio : aspectRatios)
         {
            const double fit = abs(1. - (double)(mode.height * aspectRatio.x) / (double)(mode.width * aspectRatio.y));
            if (fit < best)
            {
               bestAR = aspectRatio;
               best = fit;
            }
         }
         if (mode.width == m_displays[wndDisplay].width //
            && mode.height == m_displays[wndDisplay].height //
            && mode.depth == m_displays[wndDisplay].depth //
            && mode.refreshrate == m_displays[wndDisplay].refreshrate)
            defaultMode = i;
         if (mode.width == m_player->m_ptable->m_settings.GetWindow_FSWidth(m_wndId) //
            && mode.height == m_player->m_ptable->m_settings.GetWindow_FSHeight(m_wndId) //
            && mode.depth == m_player->m_ptable->m_settings.GetWindow_FSColorDepth(m_wndId) //
            && mode.refreshrate == m_player->m_ptable->m_settings.GetWindow_FSRefreshRate(m_wndId))
            selectedMode = i;
         modeNames.push_back(string_format("%d x %d (%.1fHz %d:%d)", mode.width, mode.height, mode.refreshrate, max(bestAR.y, bestAR.x), min(bestAR.x, bestAR.y)));
         i++;
      }

      // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Video Mode"s, "Video mode"s, false, 0, defaultMode, modeNames), //
         [selectedMode]() { return selectedMode; }, // Live
         [selectedMode](Settings& settings) { return selectedMode; }, // Stored
         [this, wndDisplay](int, int v)
         {
            m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
            vector<Window::VideoMode> modes = VPX::Window::GetDisplayModes(m_displays[wndDisplay]);
            m_player->m_ptable->m_settings.SetWindow_FSWidth(m_wndId, modes[v].width, false);
            m_player->m_ptable->m_settings.SetWindow_FSHeight(m_wndId, modes[v].height, false);
            m_player->m_ptable->m_settings.SetWindow_FSColorDepth(m_wndId, modes[v].depth, false);
            m_player->m_ptable->m_settings.SetWindow_FSRefreshRate(m_wndId, modes[v].refreshrate, false);
         }, //
         [](Settings&) { /* Directly stored on change, nothing to do */ }, //
         [](int, Settings&, bool) { /* Directly stored on change, nothing to do */ }));
   }
   else
   {
      const int containerWidth = m_displays[wndDisplay].width;
      const int containerHeight = m_displays[wndDisplay].height;
      const int maxWidth = m_arLock == 0 ? m_displays[wndDisplay].width : min(containerWidth, (m_displays[wndDisplay].height * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y);
      const int maxHeight = m_arLock == 0 ? m_displays[wndDisplay].height : min(containerHeight, (m_displays[wndDisplay].width * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x);

      vector<string> arNames;
      for (const int2& aspectRatio : aspectRatios)
         if (aspectRatio.x == 0)
            arNames.push_back("Free"s);
         else
            arNames.push_back((aspectRatio.x > aspectRatio.y ? "Landscape - "s : "Portrait  - "s) + std::to_string(aspectRatio.x) + ':' + std::to_string(aspectRatio.y));
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Lock aspect ratio"s, "Limit window size to a predefined aspect ratio"s, false, 0, m_arLock, arNames), //
         [this]() { return m_arLock; }, // Live
         [this](Settings& settings) { return m_arLock; }, // Stored
         [this](int, int v)
         {
            m_arLock = v;
            BuildPage();
         }, //
         [](Settings&) { /* UI state, not persisted */ }, //
         [](int, Settings&, bool) { /* UI state, not persisted */ }));

      // OpenGL and DirectX9 backends do not support dynamic resizing of the main window
      #if !defined(ENABLE_BGFX)
      if (m_isMainWindow)
      { // For main window, we do not dynamically change size as it is not supported and the UI breaks (would require to re-setup everything)
         // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
         Settings::GetRegistry().Register(Settings::GetWindow_Width_Property(m_wndId)->WithRange(0, min(maxWidth, containerWidth - m_player->m_ptable->m_settings.GetWindow_WndX(m_wndId))));
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propWindow_Width[m_wndId], "%d"s, //
            [this]() { return m_player->m_ptable->m_settings.GetWindow_Width(m_wndId); }, //
            [this](int, int v)
            {
               m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
               if (m_arLock != 0)
               {
                  int h = (v * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x;
                  if (h > Settings::GetWindow_Height_Property(m_wndId)->m_max)
                  {
                     h = Settings::GetWindow_Height_Property(m_wndId)->m_max;
                     v = (h * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y;
                  }
                  m_player->m_ptable->m_settings.SetWindow_Height(m_wndId, h, false);
               }
               m_player->m_ptable->m_settings.SetWindow_Width(m_wndId, v, false);
               BuildPage();
            })).m_excludeFromDefault = true;

         // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
         Settings::GetRegistry().Register(Settings::GetWindow_Height_Property(m_wndId)->WithRange(0, min(maxHeight, containerHeight - m_player->m_ptable->m_settings.GetWindow_WndY(m_wndId))));
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propWindow_Height[m_wndId], "%d"s, //
            [this]() { return m_player->m_ptable->m_settings.GetWindow_Height(m_wndId); }, //
            [this](int, int v)
            {
               m_delayApplyNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the game"s, 5000, m_delayApplyNotifId);
               if (m_arLock != 0)
               {
                  int w = (v * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y;
                  if (w > Settings::GetWindow_Width_Property(m_wndId)->m_max)
                  {
                     w = Settings::GetWindow_Width_Property(m_wndId)->m_max;
                     v = (w * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x;
                  }
                  m_player->m_ptable->m_settings.SetWindow_Width(m_wndId, w, false);
               }
               m_player->m_ptable->m_settings.SetWindow_Height(m_wndId, v, false);
               BuildPage();
            })).m_excludeFromDefault = true;
      }
      else
      #endif
      {
         Settings::GetRegistry().Register(Settings::GetWindow_Width_Property(m_wndId)->WithRange(m_isMainWindow ? 320 : 0, min(maxWidth, containerWidth - wndPos.x)));
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propWindow_Width[m_wndId], "%d"s, //
            [this]() { return (m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow())->GetWidth(); }, //
            [this, containerWidth](int prev, int v)
            {
               // Apply AR constraint
               Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
               SDL_Point size { v, wnd->GetHeight() };
               if (m_arLock != 0)
               {
                  int h = (v * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x;
                  if (h > Settings::GetWindow_Height_Property(m_wndId)->m_max)
                  {
                     h = Settings::GetWindow_Height_Property(m_wndId)->m_max;
                     v = (h * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y;
                  }
                  size.y = h;
               }
               size.x = v;

               if (m_isMainWindow)
                  m_delayApplyNotifId = m_player->m_liveUI->PushNotification("You have changed main window size\nRendering will be stretched until you restart the game"s, 5000, m_delayApplyNotifId);

               // Center scale
               SDL_Point pos;
               wnd->GetPos(pos.x, pos.y);
               pos.x -= (v - prev) / 2;
               pos.x = clamp(pos.x, 0, containerWidth - v);

               wnd->SetPos(pos.x, pos.y);
               wnd->SetSize(size.x, size.y);

               OnStaticRenderDirty();
               BuildPage();
            })).m_excludeFromDefault = true;

         Settings::GetRegistry().Register(Settings::GetWindow_Height_Property(m_wndId)->WithRange(m_isMainWindow ? 320 : 0, min(maxHeight, containerHeight - wndPos.y)));
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propWindow_Height[m_wndId], "%d"s, //
            [this]() { return (m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow())->GetHeight(); }, //
            [this, containerHeight](int prev, int v)
            {
               Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
               SDL_Point size { wnd->GetWidth(), v };
               if (m_arLock != 0)
               {
                  int w = (v * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y;
                  if (w > Settings::GetWindow_Width_Property(m_wndId)->m_max)
                  {
                     w = Settings::GetWindow_Width_Property(m_wndId)->m_max;
                     v = (w * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x;
                  }
                  size.x = w;
               }
               size.y = v;

               if (m_isMainWindow)
                  m_delayApplyNotifId = m_player->m_liveUI->PushNotification("You have changed main window size\nRendering will be stretched until you restart the game"s, 5000, m_delayApplyNotifId);

               // Center scale
               SDL_Point pos;
               wnd->GetPos(pos.x, pos.y);
               pos.y -= (v - prev) / 2;
               pos.y = clamp(pos.y, 0, containerHeight - v);

               wnd->SetPos(pos.x, pos.y);
               wnd->SetSize(size.x, size.y);

               OnStaticRenderDirty();
               BuildPage();
            })).m_excludeFromDefault = true;
      }

      Settings::GetRegistry().Register(Settings::GetWindow_WndX_Property(m_wndId)->WithRange(0, containerWidth - wndSize.x));
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propWindow_WndX[m_wndId], "%d"s, //
         [this, wndDisplay]()
         {
            SDL_Point pos;
            const Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
            wnd->GetPos(pos.x, pos.y);
            return pos.x - m_displays[wndDisplay].left;
         }, //
         [this, wndDisplay](int prev, int v)
         {
            Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
            SDL_Point pos;
            wnd->GetPos(pos.x, pos.y);
            wnd->SetPos(m_displays[wndDisplay].left + v, pos.y);
            if (m_isMainWindow)
            { // Warp mouse as if clicked, we would click on the opposite direction, if flipper nav, we would disable flipper nav (due to relative mouse move)
               SDL_FPoint mousePos;
               SDL_GetGlobalMouseState(&mousePos.x, &mousePos.y);
               SDL_WarpMouseGlobal(mousePos.x + static_cast<float>(v - prev), mousePos.y);
            }
            BuildPage();
         })).m_excludeFromDefault = true;

      Settings::GetRegistry().Register(Settings::GetWindow_WndY_Property(m_wndId)->WithRange(0, containerHeight - wndSize.y));
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propWindow_WndY[m_wndId], "%d"s, //
         [this, wndDisplay]()
         {
            SDL_Point pos;
            const Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
            wnd->GetPos(pos.x, pos.y);
            return pos.y - m_displays[wndDisplay].top;
         }, //
         [this, wndDisplay](int prev, int v)
         {
            Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
            SDL_Point pos;
            wnd->GetPos(pos.x, pos.y);
            wnd->SetPos(pos.x, m_displays[wndDisplay].top + v);
            if (m_isMainWindow)
            { // Warp mouse as if clicked, we would click on the opposite direction, if flipper nav, we would disable flipper nav (due to relative mouse move)
               SDL_FPoint mousePos;
               SDL_GetGlobalMouseState(&mousePos.x, &mousePos.y);
               SDL_WarpMouseGlobal(mousePos.x, mousePos.y + static_cast<float>(v - prev));
            }
            BuildPage();
         })).m_excludeFromDefault = true;

      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "You may also drag the window to adjust its position"s));
   }
}

void DisplaySettingsPage::BuildEmbeddedPage()
{
   const int maxWidth = m_player->m_playfieldWnd->GetWidth();
   const int maxHeight = m_player->m_playfieldWnd->GetHeight();
   int wndX, wndY, wndW, wndH;
   GetOutput(m_wndId).GetPos(wndX, wndY);
   wndW = GetOutput(m_wndId).GetWidth();
   wndH = GetOutput(m_wndId).GetHeight();

   vector<string> arNames;
   for (const int2& aspectRatio : aspectRatios)
      if (aspectRatio.x == 0)
         arNames.push_back("Free"s);
      else
         arNames.push_back((aspectRatio.x > aspectRatio.y ? "Landscape - "s : "Portrait  - "s) + std::to_string(aspectRatio.x) + ':' + std::to_string(aspectRatio.y));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Lock aspect ratio"s, "Limit window size to a predefined aspect ratio"s, false, 0, m_arLock, arNames), //
      [this]() { return m_arLock; }, // Live
      [this](Settings& settings) { return m_arLock; }, // Stored
      [this](int, int v)
      {
         m_arLock = v;
         BuildPage();
      }, //
      [](Settings&) { /* UI state, not persisted */ }, //
      [](int, Settings&, bool) { /* UI state, not persisted */ }));

   Settings::GetRegistry().Register(Settings::GetWindow_Width_Property(m_wndId)->WithRange(0, maxWidth - wndX)); // Constrained to container
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propWindow_Width[m_wndId], "%d"s, //
      [this]() { return GetOutput(m_wndId).GetWidth(); }, //
      [this](int, int v)
      {
         if (m_arLock != 0)
         {
            int h = (v * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x;
            if (h > Settings::GetWindow_Height_Property(m_wndId)->m_max)
            {
               h = Settings::GetWindow_Height_Property(m_wndId)->m_max;
               v = (h * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y;
            }
            GetOutput(m_wndId).SetHeight(h);
         }
         GetOutput(m_wndId).SetWidth(v);
         BuildPage();
      })).m_excludeFromDefault = true;

   Settings::GetRegistry().Register(Settings::GetWindow_Height_Property(m_wndId)->WithRange(0, maxHeight - wndY)); // Constrained to container
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propWindow_Height[m_wndId], "%d"s, //
      [this]() { return GetOutput(m_wndId).GetHeight(); }, //
      [this](int, int v)
      {
         if (m_arLock != 0)
         {
            int w = (v * aspectRatios[m_arLock].x) / aspectRatios[m_arLock].y;
            if (w > Settings::GetWindow_Width_Property(m_wndId)->m_max)
            {
               w = Settings::GetWindow_Width_Property(m_wndId)->m_max;
               v = (w * aspectRatios[m_arLock].y) / aspectRatios[m_arLock].x;
            }
            GetOutput(m_wndId).SetWidth(w);
         }
         GetOutput(m_wndId).SetHeight(v);
         BuildPage();
      })).m_excludeFromDefault = true;

   Settings::GetRegistry().Register(Settings::GetWindow_WndX_Property(m_wndId)->WithRange(0, maxWidth - wndW)); // Constrained to container
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propWindow_WndX[m_wndId], "%d"s, //
      [this]()
      {
         SDL_Point pos;
         GetOutput(m_wndId).GetPos(pos.x, pos.y);
         return pos.x;
      }, //
      [this](int, int v)
      {
         SDL_Point pos;
         GetOutput(m_wndId).GetPos(pos.x, pos.y);
         GetOutput(m_wndId).SetPos(v, pos.y);
         BuildPage();
      })).m_excludeFromDefault = true;

   Settings::GetRegistry().Register(Settings::GetWindow_WndY_Property(m_wndId)->WithRange(0, maxHeight - wndH)); // Constrained to container
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propWindow_WndY[m_wndId], "%d"s, //
      [this]()
      {
         SDL_Point pos;
         GetOutput(m_wndId).GetPos(pos.x, pos.y);
         return pos.y;
      }, //
      [this](int, int v)
      {
         SDL_Point pos;
         GetOutput(m_wndId).GetPos(pos.x, pos.y);
         GetOutput(m_wndId).SetPos(pos.x, v);
         BuildPage();
      })).m_excludeFromDefault = true;

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "You may also drag the window to adjust its position"s));
}

void DisplaySettingsPage::Render(float elapsedS)
{
   InGameUIPage::Render(elapsedS);
   if (!IsWindowHovered() // Don't drag when mouse is hovering the InGameUI
      && (SDL_GetMouseFocus() == m_player->m_playfieldWnd->GetCore()) // Only apply for main playfield window (and its embedded windows)
      && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) // Only drag with left mouse button
   {
      if (m_isMainWindow)
      { // Drag main window
         SDL_Point pos;
         m_player->m_playfieldWnd->GetPos(pos.x, pos.y);
         const Window* const wnd = m_isMainWindow ? m_player->m_playfieldWnd : GetOutput(m_wndId).GetWindow();
         wnd->GetPos(pos.x, pos.y);
         pos.x += wnd->GetWidth() / 2;
         pos.y += wnd->GetHeight() / 2;
         SDL_DisplayID displayId = SDL_GetDisplayForPoint(&pos);
         SDL_Rect displayBounds;
         SDL_GetDisplayBounds(displayId, &displayBounds);

         wnd->GetPos(pos.x, pos.y);
         const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
         switch (m_player->m_liveUI->GetUIOrientation())
         {
         case 0:
            pos.x = pos.x + (int)drag.x;
            pos.y = pos.y + (int)drag.y;
            break;
         case 1:
            pos.x = pos.x - (int)drag.y;
            pos.y = pos.y + (int)drag.x;
            break;
         case 2:
            pos.x = pos.x + (int)drag.x;
            pos.y = pos.y - (int)drag.y;
            break;
         case 3:
            pos.x = pos.x + (int)drag.y;
            pos.y = pos.y - (int)drag.x;
            break;
         default: assert(false);
         }

         pos.x = clamp(pos.x, displayBounds.x, displayBounds.x + displayBounds.w - m_player->m_playfieldWnd->GetWidth());
         pos.y = clamp(pos.y, displayBounds.y, displayBounds.y + displayBounds.h - m_player->m_playfieldWnd->GetHeight());
         m_player->m_playfieldWnd->SetPos(pos.x, pos.y);
      }
      else if (GetOutput(m_wndId).GetMode() == RenderOutput::OM_EMBEDDED)
      {  // Drag ancillary embedded window
         // Floating ancillary window are directly handled in the main event dispatch handler since input events are only broadcasted to ImGui for the main window
         SDL_Point pos;
         GetOutput(m_wndId).GetPos(pos.x, pos.y);
         const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
         ImGui::ResetMouseDragDelta();
         pos.x = clamp(pos.x + static_cast<int>(drag.x), 0, m_player->m_playfieldWnd->GetWidth() - GetOutput(m_wndId).GetWidth());
         pos.y = clamp(pos.y + static_cast<int>(drag.y), 0, m_player->m_playfieldWnd->GetHeight() - GetOutput(m_wndId).GetHeight());
         GetOutput(m_wndId).SetPos(pos.x, pos.y);
      }
   }
}

}
