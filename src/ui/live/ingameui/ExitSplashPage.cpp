// license:GPLv3+

#include "core/stdafx.h"

#include "ExitSplashPage.h"
#include "InGameUIItem.h"

namespace VPX::InGameUI
{

ExitSplashPage::ExitSplashPage()
   : InGameUIPage("Visual Pinball X"s, ""s, SaveMode::None)
{
   BuildPage();
}

void ExitSplashPage::BuildPage()
{
#ifdef __STANDALONE__
   constexpr bool isStandalone = true;
#else
   constexpr bool isStandalone = false;
#endif

#if ((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   constexpr bool hasKeyboard = false;
#else
   constexpr bool hasKeyboard = true;
#endif

#if ((defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS)) || defined(__ANDROID__))
   constexpr bool isTouch = true;
#else
   constexpr bool isTouch = false;
#endif

   ClearItems();

   AddItem(std::make_unique<InGameUIItem>("Table Options & Settings"s, ""s, "homepage"s));

   // FIXME remove unsupported Win32 only legacy BAM headtracking
   #ifdef WIN32
   if (m_player->m_headTracking)
      AddItem(std::make_unique<InGameUIItem>("Adjust Headtracking"s, ""s, []() { ImGui::OpenPopup(ID_BAM_SETTINGS); }));
   #endif

   if (hasKeyboard && m_player->m_renderer->m_stereo3D != STEREO_VR)
      AddItem(std::make_unique<InGameUIItem>("Live Editor"s, ""s,
         [this]()
         {
            m_player->m_liveUI->HideUI();
            m_player->m_liveUI->OpenEditorUI();
         }));

   if (g_pvp->m_ptableActive->TournamentModePossible())
      AddItem(std::make_unique<InGameUIItem>("Generate Tournament File"s, ""s,
         [this]()
         {
            m_player->m_liveUI->HideUI();
            g_pvp->GenerateTournamentFile();
         }));

   if (isTouch)
      AddItem(std::make_unique<InGameUIItem>(g_pvp->m_settings.LoadValueBool(Settings::Player, "TouchOverlay"s) ? "Disable Touch Overlay"s
            : "Enable Touch Overlay"s, ""s,
         [this]()
         {
            bool showTouchOverlay = g_pvp->m_settings.LoadValueBool(Settings::Player, "TouchOverlay"s);
            g_pvp->m_settings.SaveValue(Settings::Player, "TouchOverlay"s, showTouchOverlay);
            m_player->m_liveUI->ShowTouchOverlay(showTouchOverlay);
            ImGui::GetIO().MousePos.x = 0;
            ImGui::GetIO().MousePos.y = 0;
            BuildPage();
         }));

   if (!hasKeyboard)
      AddItem(std::make_unique<InGameUIItem>(m_player->m_liveUI->m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED ? "Disable FPS"s : "Enable FPS"s, ""s,
         [this]() {
            m_player->m_liveUI->m_perfUI.SetPerfMode(m_player->m_liveUI->m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED ? PerfUI::PerfMode::PM_FPS : PerfUI::PerfMode::PM_DISABLED);
            ImGui::GetIO().MousePos.x = 0;
            ImGui::GetIO().MousePos.y = 0;
            BuildPage();
         }));

   if (!isStandalone)
      AddItem(std::make_unique<InGameUIItem>("Quit to Editor"s, ""s, [this]() { m_player->m_ptable->QuitPlayer(Player::CS_STOP_PLAY); }));
   else
      AddItem(std::make_unique<InGameUIItem>("Quit"s, ""s, [this]() { m_player->m_ptable->QuitPlayer(Player::CS_CLOSE_APP); }));
}


/*
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
*/

void ExitSplashPage::Render(float elapsedMs)
{
   // Display table name, author, version, blurb and description => Move to a dedicated page ?
   {
      std::ostringstream info;

      // If description does not already contain a title, add one
      if (m_player->m_ptable->m_description.find('#') == std::string::npos)
      {
         if (!m_player->m_ptable->m_tableName.empty())
            info << "# " << m_player->m_ptable->m_tableName << '\n';
         else
            info << "# Table\n";
      }

      const size_t line_length = info.str().size();
      if (!m_player->m_ptable->m_blurb.empty())
         info << m_player->m_ptable->m_blurb << std::string(line_length, '=') << '\n';
      if (!m_player->m_ptable->m_description.empty())
         info << m_player->m_ptable->m_description;

      info << "\n\n  ";
      if (!m_player->m_ptable->m_author.empty())
         info << "By " << m_player->m_ptable->m_author << ", ";
      if (!m_player->m_ptable->m_version.empty())
         info << "Version: " << m_player->m_ptable->m_version;
      info << " (" << (!m_player->m_ptable->m_dateSaved.empty() ? m_player->m_ptable->m_dateSaved : "N.A."s) << " Revision " << m_player->m_ptable->m_numTimesSaved << ")\n";

      constexpr ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f - fabs(GetOpenCloseAnimPos()));
      ImGui::SetNextWindowBgAlpha(0.5f * (1.f - fabs(GetOpenCloseAnimPos())));
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin((std::to_string(reinterpret_cast<uint64_t>(this)) + ".back"s).c_str(), nullptr, window_flags);
      m_player->m_liveUI->SetMarkdownStartId(ImGui::GetItemID());
      ImGui::Markdown(info.str().c_str(), info.str().length(), m_player->m_liveUI->GetMarkdownConfig());
      ImGui::End();
      ImGui::PopStyleVar();
   }
  
   InGameUIPage::Render(elapsedMs);
}

};