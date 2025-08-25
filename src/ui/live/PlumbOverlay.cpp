// license:GPLv3+

#include "core/stdafx.h"

#include "PlumbOverlay.h"

void PlumbOverlay::Update()
{
   const Player *m_player = g_pplayer;

   if (!m_player->m_physics->IsPlumbSimulated())
      return;

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
   float plumbFade = 0.f;
   if (vel > 0.025f)
      m_plumbFadeCounter = 0;
   else
      m_plumbFadeCounter++;
   if (m_plumbFadeCounter > 120)
      return;
   else if (m_plumbFadeCounter < 20)
      plumbFade = 1.f;
   else 
      plumbFade = static_cast<float>(120 - m_plumbFadeCounter) / 100.f;

   const ImVec2 fullSize = ImVec2(200.f * m_dpi, 200.f * m_dpi);
   const ImVec2 halfSize = fullSize * 0.5f;
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(0.f, ImGui::GetIO().DisplaySize.y - fullSize.y));
   ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(m_tiltFade * 255.f, 0, 0, plumbFade * 64.f));
   ImGui::SetNextWindowSize(fullSize);
   ImGui::Begin("PlumbOverlay", nullptr, window_flags);
   const ImVec2 &pos = ImGui::GetWindowPos();
   ImU32 alphaCol = IM_COL32(255, 0, 0, plumbFade * 255.f);
   // Tilt circle
   const ImVec2 radius = fullSize * (0.5f * 0.8f);
   const ImVec2 scale = radius / sin(m_player->m_physics->GetPlumbTiltThreshold() * (float)(M_PI * 0.25));
   ImGui::GetWindowDrawList()->AddEllipse(pos + halfSize, radius, alphaCol);
   // Plumb position
   const Vertex3Ds &plumb = m_player->m_physics->GetPlumbPos();
   const ImVec2 plumbPos = pos + halfSize + scale * ImVec2(plumb.x, plumb.y) / m_player->m_physics->GetPlumbPoleLength() + ImVec2(0.5f, 0.5f);
   ImGui::GetWindowDrawList()->AddLine(pos + halfSize, plumbPos, alphaCol);
   ImGui::GetWindowDrawList()->AddCircleFilled(plumbPos, 5.f * m_dpi, alphaCol);
   ImGui::End();
   ImGui::PopStyleColor();
}
