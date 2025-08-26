// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUI.h"
#include "ui/live/LiveUI.h"

#include "HomePage.h"
#include "TableOptionsPage.h"
#include "TableRulesPage.h"
#include "AudioSettingsPage.h"
#include "MiscSettingsPage.h"
#include "NudgeSettingsPage.h"
#include "PointOfViewSettingsPage.h"
#include "VRSettingsPage.h"


namespace VPX::InGameUI
{

InGameUI::InGameUI(LiveUI &liveUI)
   : m_player(g_pplayer)
{
   AddPage(std::make_unique<HomePage>());
   AddPage(std::make_unique<TableOptionsPage>());
   AddPage(std::make_unique<TableRulesPage>());
   AddPage(std::make_unique<AudioSettingsPage>());
   AddPage(std::make_unique<MiscSettingsPage>());
   AddPage(std::make_unique<NudgeSettingsPage>());
   AddPage(std::make_unique<PointOfViewSettingsPage>());
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
      string path = m_navigationHistory.back();
      Navigate(path);
      m_navigationHistory.pop_back();
      m_navigationHistory.pop_back();
   }
}

void InGameUI::Open()
{
   assert(!IsOpened());
   m_isOpened = true;
   Navigate("homepage");
}

void InGameUI::Close()
{
   assert(IsOpened());
   m_isOpened = false;
   if (m_playerPaused)
      m_player->SetPlayState(true);
   m_playerPaused = false;
   m_player->m_ptable->FireOptionEvent(3); // Tweak mode closed event
   if (m_activePage)
      m_activePage->Close();
}

void InGameUI::Update()
{
   if (!m_isOpened)
      return;

   // Only pause player if balls are moving to keep attract mode if possible
   if (!m_playerPaused)
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
         m_playerPaused = true;
      }
   }

   PinInput::InputState state = m_player->m_pininput.GetInputState();
   HandlePageInput(state);
   HandleLegacyFlyOver(state);
   m_prevInputState = state;

   m_activePage->Render();
}

void InGameUI::HandlePageInput(const PinInput::InputState &state)
{
   // Disable keyboard shortcut if no control editing is in progress
   // TODO: we should only continue to process gamepad & VR controller
   if (ImGui::IsAnyItemActive())
      return;

   if (state.IsKeyPressed(eLeftMagnaSave, m_prevInputState))
   {
      const bool wasFlipperNav = m_useFlipperNav;
      m_useFlipperNav = true;
      m_activePage->SelectPrevItem();
      if (!wasFlipperNav)
         m_activePage->SelectNextItem();
   }

   if (state.IsKeyPressed(eRightMagnaSave, m_prevInputState))
   {
      const bool wasFlipperNav = m_useFlipperNav;
      m_useFlipperNav = true;
      m_activePage->SelectNextItem();
      if (!wasFlipperNav)
         m_activePage->SelectPrevItem();
   }

   if (m_useFlipperNav && state.IsKeyPressed(eLeftFlipperKey, m_prevInputState))
      m_activePage->AdjustItem(-1, true);
   else if (m_useFlipperNav && state.IsKeyDown(eLeftFlipperKey))
      m_activePage->AdjustItem(-1, false);

   if (m_useFlipperNav && state.IsKeyPressed(eRightFlipperKey, m_prevInputState))
      m_activePage->AdjustItem(1, true);
   else if (m_useFlipperNav && state.IsKeyDown(eRightFlipperKey))
      m_activePage->AdjustItem(1, false);

   if (state.IsKeyPressed(ePlungerKey, m_prevInputState))
      m_activePage->ResetToDefaults();

   if (state.IsKeyPressed(eAddCreditKey, m_prevInputState))
   {
      if (g_pvp->m_povEdit)
         g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
      else
         m_activePage->ResetToInitialValues();
   }

   if (state.IsKeyPressed(eStartGameKey, m_prevInputState))
      m_activePage->Save();

   if (state.IsKeyReleased(eEscape, m_prevInputState))
      Close(); // FIXME should a navigate back, up to InGameUI close, applied on key release as this is the way it is handle for the main splash (to be changed ?)
}

// Legacy leyboard fly camera when in ingame option. Remove ?
void InGameUI::HandleLegacyFlyOver(const PinInput::InputState &state)
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

   if (state.IsKeyDown(eLeftTiltKey))
      m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].mViewportRotation -= 1.0f;

   if (state.IsKeyDown(eRightTiltKey))
      m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].mViewportRotation += 1.0f;
}


};