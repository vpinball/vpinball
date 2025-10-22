// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUI.h"
#include "ui/live/LiveUI.h"

#include "ExitSplashPage.h"
#include "HomePage.h"
#include "TableOptionsPage.h"
#include "TableRulesPage.h"
#include "AudioSettingsPage.h"
#include "GraphicSettingsPage.h"
#include "InputSettingsPage.h"
#include "MiscSettingsPage.h"
#include "NudgeSettingsPage.h"
#include "PlungerSettingsPage.h"
#include "PointOfViewSettingsPage.h"
#include "VRSettingsPage.h"
#include "BackglassPage.h"
#include "ScoreViewPage.h"


namespace VPX::InGameUI
{

InGameUI::InGameUI(LiveUI &liveUI)
   : m_player(g_pplayer)
{
   AddPage("exit"s, []() { return std::make_unique<ExitSplashPage>(); });
   AddPage("homepage"s, []() { return std::make_unique<HomePage>(); });
   AddPage("settings/audio"s, []() { return std::make_unique<AudioSettingsPage>(); });
   AddPage("settings/graphic"s, []() { return std::make_unique<GraphicSettingsPage>(); });
   AddPage("settings/input"s, []() { return std::make_unique<InputSettingsPage>(); });
   AddPage("settings/misc"s, []() { return std::make_unique<MiscSettingsPage>(); });
   AddPage("settings/nudge"s, []() { return std::make_unique<NudgeSettingsPage>(); });
   AddPage("settings/plunger"s, []() { return std::make_unique<PlungerSettingsPage>(); });
   AddPage("settings/pov"s, []() { return std::make_unique<PointOfViewSettingsPage>(); });
   AddPage("table/options"s, []() { return std::make_unique<TableOptionsPage>(); });
   AddPage("table/rules"s, []() { return std::make_unique<TableRulesPage>(); });
   AddPage("settings/vr"s, []() { return std::make_unique<VRSettingsPage>(); });
   AddPage("settings/backglass"s, []() { return std::make_unique<BackglassPage>(); });
   AddPage("settings/scoreview"s, []() { return std::make_unique<ScoreViewPage>(); });
}

void InGameUI::AddPage(const string &path, std::function<std::unique_ptr<InGameUIPage>()> pageFactory)
{
   m_pages[path] = pageFactory;
}

void InGameUI::Navigate(const string &path, bool isBack)
{
   if (!m_activePages.empty() && m_activePages.back()->IsActive())
   {
      m_activePages.back()->Close(isBack);
   }
   auto it = m_pages.find(path);
   if (it == m_pages.end())
   {
      PLOGE << "InGameUI: unknown page '" << path << "'";
      Close();
      return;
   }
   m_activePages.push_back(std::move(it->second()));
   if (m_activePages.back())
   {
      m_navigationHistory.push_back(path);
      m_activePages.back()->Open(isBack);
   }
   else
   {
      PLOGE << "InGameUI: Failed to create page '" << path << "'";
      m_activePages.pop_back();
      Close();
   }
}

void InGameUI::NavigateBack()
{
   assert(IsOpened());
   assert(!m_navigationHistory.empty());

   m_navigationHistory.pop_back();
   if (m_navigationHistory.empty())
   {
      Close();
   }
   else
   {
      const string path = m_navigationHistory.back();
      m_navigationHistory.pop_back();
      Navigate(path, true);
   }
}

void InGameUI::Open(const string& page)
{
   assert(!IsOpened());
   Navigate(page);
   m_useFlipperNav = m_player->m_vrDevice || m_player->m_ptable->m_BG_current_set == ViewSetupID::BG_FULLSCREEN;
   m_prevActionState = m_player->m_pininput.GetActionState();
}

void InGameUI::Close()
{
   if (GetActivePage())
      GetActivePage()->Close(false);
   if (!m_player->IsPlaying(false))
      m_player->SetPlayState(true);
   m_player->m_ptable->FireOptionEvent(3); // Tweak mode closed event
}

void InGameUI::Update()
{
   const uint32_t now = msec();
   const float elapsed = static_cast<float>(now - m_lastRenderMs) / 1000.f;
   m_lastRenderMs = now;
   if (m_activePages.empty())
      return;

   // Remove closed pages (after closing animation)
   for (auto it = m_activePages.begin(); it != m_activePages.end();)
   {
      if ((*it)->IsClosed())
         it = m_activePages.erase(it);
      else
         ++it;
   }

   if (const InGameUIPage *const activePage = GetActivePage(); activePage && activePage->IsActive())
   {
      // Only pause player if balls are moving to keep attract mode if possible
      if (m_player->IsPlaying(false))
      {
         if (activePage->IsPlayerPauseAllowed())
         {
            bool ballMoving = false;
            for (const auto &ball : m_player->m_vball)
            {
               if (ball->m_d.m_vel.LengthSquared() > 0.25f)
               {
                  ballMoving = true;
                  break;
               }
            }
            if (ballMoving)
            {
               m_player->SetPlayState(false);
            }
         }
      }
      else if (!activePage->IsPlayerPauseAllowed())
      {
         m_player->SetPlayState(true);
      }

      const InputManager::ActionState state = m_player->m_pininput.GetActionState();
      HandlePageInput(state);
      HandleLegacyFlyOver(state);
      m_prevActionState = state;
   }

   for (const auto& page : m_activePages)
      page->Render(elapsed);
}

void InGameUI::HandlePageInput(const InputManager::ActionState &state)
{
   // Disable keyboard shortcut if no control editing is in progress
   if (ImGui::IsAnyItemActive())
      return;

   // If user has moved the mouse, disable flipper navigation
   if (ImGui::GetMousePos() != m_prevMousePos)
   {
      m_prevMousePos = ImGui::GetMousePos();
      m_useFlipperNav = false;
   }

   if (state.IsKeyPressed(m_player->m_pininput.GetLeftMagnaActionId(), m_prevActionState))
   {
      const bool wasFlipperNav = m_useFlipperNav;
      m_useFlipperNav = true;
      GetActivePage()->SelectPrevItem();
      if (!wasFlipperNav)
         GetActivePage()->SelectNextItem();
   }

   if (state.IsKeyPressed(m_player->m_pininput.GetRightMagnaActionId(), m_prevActionState))
   {
      const bool wasFlipperNav = m_useFlipperNav;
      m_useFlipperNav = true;
      GetActivePage()->SelectNextItem();
      if (!wasFlipperNav)
         GetActivePage()->SelectPrevItem();
   }

   if (m_useFlipperNav && state.IsKeyPressed(m_player->m_pininput.GetLeftFlipperActionId(), m_prevActionState))
      GetActivePage()->AdjustItem(-1, true);
   else if (m_useFlipperNav && state.IsKeyDown(m_player->m_pininput.GetLeftFlipperActionId()))
      GetActivePage()->AdjustItem(-1, false);

   if (m_useFlipperNav && state.IsKeyPressed(m_player->m_pininput.GetRightFlipperActionId(), m_prevActionState))
      GetActivePage()->AdjustItem(1, true);
   else if (m_useFlipperNav && state.IsKeyDown(m_player->m_pininput.GetRightFlipperActionId()))
      GetActivePage()->AdjustItem(1, false);

   if (state.IsKeyPressed(m_player->m_pininput.GetLaunchBallActionId(), m_prevActionState))
      GetActivePage()->ResetToDefaults();

   if (state.IsKeyPressed(m_player->m_pininput.GetAddCreditActionId(0), m_prevActionState))
   {
      if (g_pvp->m_povEdit)
         g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
      else
         GetActivePage()->ResetToInitialValues();
   }

   if (state.IsKeyPressed(m_player->m_pininput.GetStartActionId(), m_prevActionState))
      GetActivePage()->Save();

   if (state.IsKeyPressed(m_player->m_pininput.GetExitInteractiveActionId(), m_prevActionState))
      NavigateBack();
}

// Legacy keyboard fly camera when in ingame option. Remove ?
void InGameUI::HandleLegacyFlyOver(const InputManager::ActionState &state)
{
   if (!m_player->m_ptable->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s))
      return;

   if (!ImGui::IsKeyDown(ImGuiKey_LeftAlt) && !ImGui::IsKeyDown(ImGuiKey_RightAlt))
   {
      if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
         m_player->m_renderer->m_cam.x += 10.0f;
      if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
         m_player->m_renderer->m_cam.x -= 10.0f;
      if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
         m_player->m_renderer->m_cam.y += 10.0f;
      if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
         m_player->m_renderer->m_cam.y -= 10.0f;
   }
   else
   {
      if (ImGui::IsKeyDown(ImGuiKey_UpArrow))
         m_player->m_renderer->m_cam.z += 10.0f;
      if (ImGui::IsKeyDown(ImGuiKey_DownArrow))
         m_player->m_renderer->m_cam.z -= 10.0f;
      if (ImGui::IsKeyDown(ImGuiKey_LeftArrow))
         m_player->m_renderer->m_inc += 0.01f;
      if (ImGui::IsKeyDown(ImGuiKey_RightArrow))
         m_player->m_renderer->m_inc -= 0.01f;
   }

   if (state.IsKeyDown(m_player->m_pininput.GetLeftNudgeActionId()))
      m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].mViewportRotation -= 1.0f;

   if (state.IsKeyDown(m_player->m_pininput.GetRightNudgeActionId()))
      m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].mViewportRotation += 1.0f;
}

}
