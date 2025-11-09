// license:GPLv3+

#include "core/stdafx.h"

#include "PlumbOverlay.h"

void PlumbOverlay::Update()
{
   const Player *m_player = g_pplayer;

   if (!m_player->m_physics->IsPlumbSimulated())
      return;

   const bool isInNudgeSettings = m_player->m_liveUI->m_inGameUI.IsOpened("settings/nudge"s);

   if (m_lastTiltIndex != m_player->m_physics->GetPlumbTiltIndex())
   {
      m_lastTiltIndex = m_player->m_physics->GetPlumbTiltIndex();
      m_tiltFade = 1.f;
      m_plumbFadeCounter = 0;
   }
   else
   {
      m_tiltFade = std::max(m_tiltFade - 0.01f, 0.f);
   }

   const float vel = m_player->m_physics->GetPlumbVel().Length();
   if (isInNudgeSettings || vel > 0.025f)
      m_plumbFadeCounter = 0;
   else
      m_plumbFadeCounter++;
   if (m_plumbFadeCounter > 120)
      return;

   float plumbFade;
   if (m_plumbFadeCounter < 20)
      plumbFade = 1.f;
   else 
      plumbFade = static_cast<float>(120 - m_plumbFadeCounter) / 100.f;

   const ImVec2 fullSize = ImVec2(200.f * m_uiScale, 200.f * m_uiScale);
   const ImVec2 halfSize = fullSize * 0.5f;
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground| ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
      | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(0.f, ImGui::GetIO().DisplaySize.y - fullSize.y));
   ImGui::SetNextWindowSize(fullSize);
   ImGui::Begin("PlumbOverlay", nullptr, window_flags);
   const ImVec2 &pos = ImGui::GetWindowPos();
   const ImVec2 radius = fullSize * (float)(0.5 * 0.8);
   const ImVec2 scale = radius / sinf(m_player->m_physics->GetPlumbTiltThreshold());
   // Background
   const ImU32 backCol = IM_COL32(isInNudgeSettings ? 0.f : (m_tiltFade * 255.f), 0, 0, plumbFade * 64.f);
   ImGui::GetWindowDrawList()->AddEllipseFilled(pos + halfSize, radius * 1.1f, backCol);
   // Tilt circle
   const ImU32 alphaCol = IM_COL32(255, 0, 0, plumbFade * 255.f);
   ImGui::GetWindowDrawList()->AddEllipse(pos + halfSize, radius, alphaCol, 0.0f, 0, 2.f * m_uiScale);
   // Plumb position
   const Vertex3Ds &plumb = m_player->m_physics->GetPlumbPos();
   const ImVec2 plumbPos = pos + halfSize + scale * ImVec2(plumb.x, plumb.y) / m_player->m_physics->GetPlumbPoleLength() + ImVec2(0.5f, 0.5f);
   ImGui::GetWindowDrawList()->AddLine(pos + halfSize, plumbPos, alphaCol, 2.f * m_uiScale);
   ImGui::GetWindowDrawList()->AddCircleFilled(plumbPos, 5.f * m_uiScale, alphaCol);
   ImGui::End();
}
