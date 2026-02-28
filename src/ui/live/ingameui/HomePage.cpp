// license:GPLv3+

#include "core/stdafx.h"

#include "HomePage.h"
#include "core/TournamentFile.h"

namespace VPX::InGameUI
{

HomePage::HomePage()
   : InGameUIPage("Options & Settings"s, ""s, SaveMode::None)
{
   BuildPage();
}

void HomePage::BuildPage()
{
   constexpr bool hasKeyboard = !(g_isAndroid || g_isIOS);
   constexpr bool isTouch = g_isAndroid || g_isIOS;
   ClearItems();

   ////////////////////////////////////////////////////////////////////////////////////////////////
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Table options"s));

   if (!m_player->m_ptable->m_rules.empty())
      AddItem(std::make_unique<InGameUIItem>("Table Rules"s, ""s, "table/rules"s));

   if (!m_player->m_ptable->GetOptions().empty())
      AddItem(std::make_unique<InGameUIItem>("Table Options"s, ""s, "table/options"s));

   if (m_player->m_vrDevice)
   {
      #ifdef ENABLE_XR
         // Legacy OpenVR does not support dynamic repositioning through LiveUI (especially overall scale, this would need to be rewritten but not done as this is planned for deprecation)
         AddItem(std::make_unique<InGameUIItem>("VR Settings"s, ""s, "settings/vr"s));
      #endif
   }
   else
      AddItem(std::make_unique<InGameUIItem>("Point Of View"s, ""s, "settings/pov"s));

   if (m_player->m_ptable->TournamentModePossible())
      AddItem(std::make_unique<InGameUIItem>("Generate Tournament File"s, ""s,
         [this]()
         {
            m_player->m_liveUI->HideUI();
            VPX::TournamentFile::GenerateTournamentFile();
         }));

   ////////////////////////////////////////////////////////////////////////////////////////////////
   const bool allowLiveEditor = hasKeyboard && m_player->m_renderer->m_stereo3D != STEREO_VR && m_player->m_ptable->m_liveBaseTable;
   if (allowLiveEditor || isTouch || !hasKeyboard)
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Misc. Actions"s));

   if (allowLiveEditor)
      AddItem(std::make_unique<InGameUIItem>("Live Editor"s, ""s,
         [this]()
         {
            m_player->m_liveUI->HideUI();
            m_player->m_liveUI->OpenEditorUI();
         }));

   if (isTouch)
      AddItem(std::make_unique<InGameUIItem>(g_app->m_settings.GetPlayer_TouchOverlay() ? "Disable Touch Overlay"s : "Enable Touch Overlay"s, ""s,
         [this]()
         {
            bool showTouchOverlay = !g_app->m_settings.GetPlayer_TouchOverlay();
            g_app->m_settings.ResetPlayer_TouchOverlay();
            g_app->m_settings.SetPlayer_TouchOverlay(showTouchOverlay, false);
            m_player->m_liveUI->ShowTouchOverlay(showTouchOverlay);
            ImGui::GetIO().MousePos.x = 0;
            ImGui::GetIO().MousePos.y = 0;
            BuildPage();
         }));

   if (!hasKeyboard)
      AddItem(std::make_unique<InGameUIItem>(m_player->m_liveUI->m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED ? "Disable FPS"s : "Enable FPS"s, ""s,
         [this]()
         {
            bool wasDisabled = m_player->m_liveUI->m_perfUI.GetPerfMode() == PerfUI::PerfMode::PM_DISABLED;
            m_player->m_liveUI->m_perfUI.SetPerfMode(wasDisabled ? PerfUI::PerfMode::PM_FPS : PerfUI::PerfMode::PM_DISABLED);
            if (wasDisabled)
            {
               m_player->InitFPS();
               m_player->m_logicProfiler.EnableWorstFrameLogging(true);
            }
            ImGui::GetIO().MousePos.x = 0;
            ImGui::GetIO().MousePos.y = 0;
            BuildPage();
         }));

   if (g_isMobile)
      AddItem(std::make_unique<InGameUIItem>("Quit"s, ""s, [this]() {
         #ifdef __LIBVPINBALL__
            m_player->m_ptable->QuitPlayer(Player::CS_CLOSE_CAPTURE_SCREENSHOT);
         #endif
      }));

   ////////////////////////////////////////////////////////////////////////////////////////////////
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Settings"s));

   AddItem(std::make_unique<InGameUIItem>("Sound Settings"s, ""s, "settings/audio"s));

   AddItem(std::make_unique<InGameUIItem>("Graphic Settings"s, ""s, "settings/graphic"s));

   #ifndef ENABLE_DX9
   if (m_player->m_renderer->m_stereo3D != STEREO_VR)
      AddItem(std::make_unique<InGameUIItem>("Stereo Settings"s, ""s, "settings/stereo"s));
   #endif

   AddItem(std::make_unique<InGameUIItem>("Display Settings"s, ""s, "settings/displays"s));

   AddItem(std::make_unique<InGameUIItem>("Input Settings"s, ""s, "settings/input"s));

   AddItem(std::make_unique<InGameUIItem>("Plunger Settings"s, ""s, "settings/plunger"s));

   AddItem(std::make_unique<InGameUIItem>("Nudge & Tilt Settings"s, ""s, "settings/nudge"s));

   AddItem(std::make_unique<InGameUIItem>("Cabinet Settings"s, ""s, "settings/cabinet"s));

   AddItem(std::make_unique<InGameUIItem>("Alpha/DMD Profile Settings"s, ""s, "settings/display_profiles"s));

   AddItem(std::make_unique<InGameUIItem>("Miscellaneous Settings"s, ""s, "settings/misc"s));

   AddItem(std::make_unique<InGameUIItem>("Plugin Settings"s, ""s, "plugins/homepage"s));

   // FIXME remove unsupported Win32 only legacy BAM headtracking
#ifdef WIN32
   if (m_player->m_headTracking)
      AddItem(std::make_unique<InGameUIItem>("BAM Headtracking Settings"s, ""s, []() { ImGui::OpenPopup(ID_BAM_SETTINGS); }));
#endif
}

void HomePage::Render(float elapsedMs)
{
   // Display table name, author, version, blurb and description => Move to a dedicated page ?
   if (m_player->m_vrDevice == nullptr)
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
      info << " (" << (!m_player->m_ptable->m_dateSaved.empty() ? m_player->m_ptable->m_dateSaved : "N/A"s) << " Revision " << m_player->m_ptable->m_numTimesSaved << ")\n";

      constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus // Prevent focus issues
         | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
      ImGui::SetNextWindowBgAlpha(0.666f * (1.f - fabs(GetOpenCloseAnimPos())));
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.666f * (1.f - fabs(GetOpenCloseAnimPos())));
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin((std::to_string(reinterpret_cast<uint64_t>(this)) + ".back").c_str(), nullptr, window_flags);
      m_player->m_liveUI->SetMarkdownStartId(ImGui::GetItemID());
      ImGui::Markdown(info.str().c_str(), info.str().length(), m_player->m_liveUI->GetMarkdownConfig());
      ImGui::End();
      ImGui::PopStyleVar();
   }

   InGameUIPage::Render(elapsedMs);
}

}
