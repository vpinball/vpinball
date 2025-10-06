// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUI.h"
#include "ui/live/LiveUI.h"

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


namespace VPX::InGameUI
{

InGameUI::InGameUI(LiveUI &liveUI)
   : m_player(g_pplayer)
{
   AddPage(std::make_unique<HomePage>());
   AddPage(std::make_unique<AudioSettingsPage>());
   AddPage(std::make_unique<GraphicSettingsPage>());
   AddPage(std::make_unique<InputSettingsPage>());
   AddPage(std::make_unique<MiscSettingsPage>());
   AddPage(std::make_unique<NudgeSettingsPage>());
   AddPage(std::make_unique<PlungerSettingsPage>());
   AddPage(std::make_unique<PointOfViewSettingsPage>());
   AddPage(std::make_unique<TableOptionsPage>());
   AddPage(std::make_unique<TableRulesPage>());
   if (m_player->m_vrDevice)
      AddPage(std::make_unique<VRSettingsPage>());
}

void InGameUI::AddPage(std::unique_ptr<InGameUIPage> page) { m_pages[page->GetPath()] = std::move(page); }

void InGameUI::Navigate(const string &path)
{
   assert(IsOpened());
   if (m_activePage)
   {
      m_navigationHistory.push_back(m_activePage->GetPath());
      m_activePage->Close();
   }
   m_activePage = m_pages[path].get();
   if (m_activePage)
      m_activePage->Open();
   else
      Close();
}

void InGameUI::NavigateBack()
{
   assert(IsOpened());
   if (m_navigationHistory.empty())
      Close();
   else
   {
      const string path = m_navigationHistory.back();
      Navigate(path);
      m_navigationHistory.pop_back();
      m_navigationHistory.pop_back();
   }
}

void InGameUI::Open()
{
   assert(!IsOpened());
   m_isOpened = true;
   Navigate("homepage"s);
   m_useFlipperNav = m_player->m_vrDevice || m_player->m_ptable->m_BG_current_set == ViewSetupID::BG_FULLSCREEN;
}

void InGameUI::Close()
{
   assert(IsOpened());
   m_isOpened = false;
   if (!m_player->IsPlaying(false))
      m_player->SetPlayState(true);
   m_player->m_ptable->FireOptionEvent(3); // Tweak mode closed event
   if (m_activePage)
      m_activePage->Close();
}

void InGameUI::Update()
{
   if (!m_isOpened)
      return;

   // Only pause player if balls are moving to keep attract mode if possible
   if (m_player->IsPlaying(false))
   {
      if (m_activePage->IsPlayerPauseAllowed())
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
   else if (!m_activePage->IsPlayerPauseAllowed())
   {
      m_player->SetPlayState(true);
   }

   const InputManager::ActionState state = m_player->m_pininput.GetActionState();
   HandlePageInput(state);
   HandleLegacyFlyOver(state);
   m_prevActionState = state;

   m_activePage->Render();
}

void InGameUI::HandlePageInput(const InputManager::ActionState &state)
{
   // Disable keyboard shortcut if no control editing is in progress
   // TODO: we should only continue to process gamepad & VR controller
   if (ImGui::IsAnyItemActive())
      return;

   // If user has moved the mouse, disable flipper navigation
   if (ImGui::GetMousePos() != m_prevMousePos)
   {
      m_prevMousePos = ImGui::GetMousePos();
      m_useFlipperNav = false;
   }

   // For popups, we use ImGui navigation by forwarding events
   if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId))
   {
      if (state.IsKeyPressed(m_player->m_pininput.GetLeftMagnaActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_UpArrow, true);
      else if (state.IsKeyReleased(m_player->m_pininput.GetLeftMagnaActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_UpArrow, false);
      if (state.IsKeyPressed(m_player->m_pininput.GetRightMagnaActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_DownArrow, true);
      else if (state.IsKeyReleased(m_player->m_pininput.GetRightMagnaActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_DownArrow, false);
      if (state.IsKeyPressed(m_player->m_pininput.GetLeftFlipperActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, true);
      else if (state.IsKeyReleased(m_player->m_pininput.GetLeftFlipperActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, false);
      if (state.IsKeyPressed(m_player->m_pininput.GetRightFlipperActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, true);
      else if (state.IsKeyReleased(m_player->m_pininput.GetRightFlipperActionId(), m_prevActionState))
         ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, false);
      return;
   }

   if (state.IsKeyPressed(m_player->m_pininput.GetLeftMagnaActionId(), m_prevActionState))
   {
      const bool wasFlipperNav = m_useFlipperNav;
      m_useFlipperNav = true;
      m_activePage->SelectPrevItem();
      if (!wasFlipperNav)
         m_activePage->SelectNextItem();
   }

   if (state.IsKeyPressed(m_player->m_pininput.GetRightMagnaActionId(), m_prevActionState))
   {
      const bool wasFlipperNav = m_useFlipperNav;
      m_useFlipperNav = true;
      m_activePage->SelectNextItem();
      if (!wasFlipperNav)
         m_activePage->SelectPrevItem();
   }

   if (m_useFlipperNav && state.IsKeyPressed(m_player->m_pininput.GetLeftFlipperActionId(), m_prevActionState))
      m_activePage->AdjustItem(-1, true);
   else if (m_useFlipperNav && state.IsKeyDown(m_player->m_pininput.GetLeftFlipperActionId()))
      m_activePage->AdjustItem(-1, false);

   if (m_useFlipperNav && state.IsKeyPressed(m_player->m_pininput.GetRightFlipperActionId(), m_prevActionState))
      m_activePage->AdjustItem(1, true);
   else if (m_useFlipperNav && state.IsKeyDown(m_player->m_pininput.GetRightFlipperActionId()))
      m_activePage->AdjustItem(1, false);

   if (state.IsKeyPressed(m_player->m_pininput.GetLaunchBallActionId(), m_prevActionState))
      m_activePage->ResetToDefaults();

   if (state.IsKeyPressed(m_player->m_pininput.GetAddCreditActionId(0), m_prevActionState))
   {
      if (g_pvp->m_povEdit)
         g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
      else
         m_activePage->ResetToInitialValues();
   }

   if (state.IsKeyPressed(m_player->m_pininput.GetStartActionId(), m_prevActionState))
      m_activePage->Save();

   if (state.IsKeyReleased(m_player->m_pininput.GetExitInteractiveActionId(), m_prevActionState))
      Close(); // FIXME should a navigate back, up to InGameUI close, applied on key release as this is the way it is handled for the main splash (to be changed ?)
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
