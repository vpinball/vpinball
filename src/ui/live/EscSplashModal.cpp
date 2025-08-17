// license:GPLv3+

#include "core/stdafx.h"

#include "EscSplashModal.h"

void EscSplashModal::Open()
{
   if (IsOpened())
      return;
   ImGui::OpenPopup(ID_MODAL_SPLASH);
   m_openTimestamp = msec();
   m_disable_esc = g_pplayer->m_ptable->m_settings.LoadValueBool(Settings::Player, "DisableESC"s);
   g_pplayer->SetPlayState(false);
}

void EscSplashModal::Update()
{
   if (!IsOpened())
      return;

   Player *m_player = g_pplayer;
   Renderer *m_renderer = m_player->m_renderer;
   PinTable *const m_table = m_player->m_ptable;

   const bool enableKeyboardShortcuts = (msec() - m_openTimestamp) > 250;

   ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

   // Display table name,author,version and blurb and description
   {
      std::ostringstream info;

      if (!m_table->m_tableName.empty())
         info << "# " << m_table->m_tableName << '\n';
      else
         info << "# Table\n";

      const size_t line_length = info.str().size();
      if (!m_table->m_blurb.empty())
         info << m_table->m_blurb << std::string(line_length, '=') << '\n';
      if (!m_table->m_description.empty())
         info << m_table->m_description;

      info << "\n\n  ";
      if (!m_table->m_author.empty())
         info << "By " << m_table->m_author << ", ";
      if (!m_table->m_version.empty())
         info << "Version: " << m_table->m_version;
      info << " (" << (!m_table->m_dateSaved.empty() ? m_table->m_dateSaved : "N.A."s) << " Revision " << m_table->m_numTimesSaved << ")\n";

      constexpr ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.5f);
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin("Table Info", nullptr, window_flags);
      m_liveUI.SetMarkdownStartId(ImGui::GetItemID());
      ImGui::Markdown(info.str().c_str(), info.str().length(), m_liveUI.GetMarkdownConfig());
      ImGui::End();
   }

   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
   if (!ImGui::BeginPopupModal(ID_MODAL_SPLASH, nullptr, window_flags))
   {
      ImGui::PopStyleColor();
      return;
   }
   
   #ifndef __STANDALONE__
      const ImVec2 size(m_liveUI.GetDPI() * (m_player->m_headTracking ? 120.f : 100.f), 0);
   #else
      const ImVec2 size(m_liveUI.GetDPI() * 170.f, 0);
   #endif

   // Key shortcut: click on the button, or press escape key (react on key released, otherwise it would immediately reopen the UI,...)
   int keyShortcut = 0;
   if (enableKeyboardShortcuts && (ImGui::IsKeyReleased(ImGuiKey_Escape) || ((ImGui::IsKeyReleased(m_liveUI.GetImGuiKeysFromDIkeycode(m_player->m_rgKeys[eEscape])) && !m_disable_esc))))
      keyShortcut = 1;

   // Map action to ImgUI navigation
   if (m_player)
   {
      const PinInput::InputState& state = m_player->m_pininput.GetInputState();
      if (state.IsKeyPressed(eLeftFlipperKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_UpArrow, true);
      else if (state.IsKeyReleased(eLeftFlipperKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_UpArrow, false);
      if (state.IsKeyPressed(eRightFlipperKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_DownArrow, true);
      else if (state.IsKeyReleased(eRightFlipperKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_DownArrow, false);
      if (state.IsKeyPressed(eStartGameKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, true);
      else if (state.IsKeyReleased(eStartGameKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, false);
      if (state.IsKeyPressed(ePlungerKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, true);
      else if (state.IsKeyReleased(ePlungerKey, m_prevInputState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, false);
      m_prevInputState = state;
   }

   if (ImGui::Button("Resume Game", size) || (keyShortcut == 1))
   {
      ImGui::CloseCurrentPopup();
      m_liveUI.HideUI();
   }
   ImGui::SetItemDefaultFocus();
   if (ImGui::Button("Table Options", size) || (keyShortcut == 2))
   {
      ImGui::CloseCurrentPopup();
      m_liveUI.OpenTweakMode();
   }
   #if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
      if (m_player->m_headTracking && ImGui::Button("Adjust Headtracking", size))
      {
         ImGui::CloseCurrentPopup();
         ImGui::OpenPopup(ID_BAM_SETTINGS);
      }
      if (m_renderer->m_stereo3D != STEREO_VR && (ImGui::Button("Live Editor", size) || (keyShortcut == 3)))
      {
         ImGui::CloseCurrentPopup();
         m_liveUI.OpenEditorUI();
      }
   #endif
   if (g_pvp->m_ptableActive->TournamentModePossible() && ImGui::Button("Generate Tournament File", size))
   {
      g_pvp->GenerateTournamentFile();
   }
   #ifndef __STANDALONE__
      // Quit: click on the button, or press exit button
      if (ImGui::Button("Quit to Editor", size) || (enableKeyboardShortcuts && ImGui::IsKeyPressed(m_liveUI.GetImGuiKeysFromDIkeycode(m_player->m_rgKeys[eExitGame]))))
      {
         m_table->QuitPlayer(Player::CS_STOP_PLAY);
      }
   #else
      #if ((defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS)) || defined(__ANDROID__))
         bool showTouchOverlay = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TouchOverlay"s, false);
         if (ImGui::Button(showTouchOverlay ? "Disable Touch Overlay" : "Enable Touch Overlay", size))
         {
            showTouchOverlay = !showTouchOverlay;
            g_pvp->m_settings.SaveValue(Settings::Player, "TouchOverlay"s, showTouchOverlay);

            ImGui::GetIO().MousePos.x = 0;
            ImGui::GetIO().MousePos.y = 0;
         }
      #endif
      #if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
         if (ImGui::Button(m_liveUI.m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED ? "Disable FPS" : "Enable FPS", size))
         {
            m_liveUI.m_perfUI.SetPerfMode(m_liveUI.m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED ? PerfUI::PerfMode::PM_FPS : PerfUI::PerfMode::PM_DISABLED);
            ImGui::GetIO().MousePos.x = 0;
            ImGui::GetIO().MousePos.y = 0;
         }
      #endif
      if (ImGui::Button("Quit", size) || (enableKeyboardShortcuts && ImGui::IsKeyPressed(m_liveUI.GetImGuiKeysFromDIkeycode(m_player->m_rgKeys[eExitGame]))))
      {
         ImGui::CloseCurrentPopup();
         m_table->QuitPlayer(Player::CS_CLOSE_APP);
      }
   #endif
   const ImVec2 pos = ImGui::GetWindowPos();
   ImVec2 max = pos + ImGui::GetWindowSize();
   const bool hovered = ImGui::IsWindowHovered();
   ImGui::EndPopup();

   // Handle dragging mouse to allow dragging windows
   if (m_player && !m_player->m_playfieldWnd->IsFullScreen())
   {
      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      {
         if (!hovered && !(pos.x <= m_initialDragPos.x && m_initialDragPos.x <= max.x && pos.y <= m_initialDragPos.y && m_initialDragPos.y <= max.y)) // Don't drag if mouse is over UI components
         {
            const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            int x, y;
            m_player->m_playfieldWnd->GetPos(x, y);
            switch (m_liveUI.GetUIOrientation())
            {
            case 0: m_player->m_playfieldWnd->SetPos(x + (int)drag.x, y + (int)drag.y); break;
            case 1: m_player->m_playfieldWnd->SetPos(x - (int)drag.y, y + (int)drag.x); break;
            case 2: m_player->m_playfieldWnd->SetPos(x + (int)drag.x, y - (int)drag.y); break;
            case 3: m_player->m_playfieldWnd->SetPos(x + (int)drag.y, y - (int)drag.x); break;
            default: assert(false);
            }
         }
      }
      else
      {
         m_initialDragPos = ImGui::GetMousePos();
      }
   }
   ImGui::PopStyleColor();
}
