// license:GPLv3+

#include "core/stdafx.h"
#include "RendererInspectionModal.h"

#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"

// Title (used as Id) of the modal dialog
#define ID_RENDERER_INSPECTION "Renderer Inspection"

namespace VPX::EditorUI
{

RendererInspectionModal::RendererInspectionModal(EditorUI &editor)
   : m_editor(editor)
   , m_passSelection(IF_FPS)
{
}

void RendererInspectionModal::Close()
{
   if (!m_visible)
      return;
   m_visible = false;
   m_editor.m_player->m_infoMode = IF_FPS;
}

void RendererInspectionModal::Render()
{
   if (!m_visible)
      return;

   // FIXME m_editor.m_renderer->DisableStaticPrePass(false);
   m_editor.m_camMode = ViewMode::PreviewCam;

   ImGui::SetNextWindowSize(ImVec2(350.f * m_editor.m_liveUI.GetDPI(), 0));
   if (ImGui::Begin(ID_RENDERER_INSPECTION, &m_visible))
   {
      ImGui::TextUnformatted("Display single render pass:");
      ImGui::RadioButton("Disabled", &m_passSelection, IF_FPS);
#if defined(ENABLE_DX9) // No GPU profiler for OpenGL or BGFX for the time being
      ImGui::RadioButton("Profiler", &m_passSelection, IF_PROFILING);
#endif
      ImGui::RadioButton("Static prerender pass", &m_passSelection, IF_STATIC_ONLY);
      ImGui::RadioButton("Dynamic render pass", &m_passSelection, IF_DYNAMIC_ONLY);
      ImGui::RadioButton("Transmitted light pass", &m_passSelection, IF_LIGHT_BUFFER_ONLY);
      if (m_editor.m_player->m_renderer->GetAOMode() != 0)
         ImGui::RadioButton("Ambient Occlusion pass", &m_passSelection, IF_AO_ONLY);
      for (size_t i = 0; i < m_editor.m_table->m_vrenderprobe.size(); i++)
      {
         ImGui::RadioButton(m_editor.m_table->m_vrenderprobe[i]->GetName().c_str(), &m_passSelection, 100 + (int)i);
      }
      if (m_passSelection < 100)
         m_editor.m_player->m_infoMode = (InfoMode)m_passSelection;
      else
      {
         m_editor.m_player->m_infoMode = IF_RENDER_PROBES;
         m_editor.m_player->m_infoProbeIndex = m_passSelection - 100;
      }
      ImGui::NewLine();

      // Latency timing table
      if (ImGui::BeginTable("Latencies", 4, ImGuiTableFlags_Borders))
      {
         const uint32_t period = m_editor.m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME);
         ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Avg", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableHeadersRow();
#define PROF_ROW(name, section)                                                                                                                                                              \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::TextUnformatted(name);                                                                                                                                                             \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::Text("%4.1fms", m_editor.m_player->m_logicProfiler.GetSlidingMin(section) * 1e-3);                                                                                                 \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::Text("%4.1fms", m_editor.m_player->m_logicProfiler.GetSlidingMax(section) * 1e-3);                                                                                                 \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::Text("%4.1fms", m_editor.m_player->m_logicProfiler.GetSlidingAvg(section) * 1e-3);
         PROF_ROW("Input to Script lag", FrameProfiler::PROFILE_INPUT_POLL_PERIOD)
         PROF_ROW("Input to Present lag", FrameProfiler::PROFILE_INPUT_TO_PRESENT)
#undef PROF_ROW
         ImGui::EndTable();
         ImGui::NewLine();
      }

      /* ImGui::TextUnformatted("Press F11 to reset min/max/average timings");
      if (ImGui::IsKeyPressed(LiveUI::GetImGuiKeyFromSDLScancode(m_editor.m_player->m_actionToSDLScanCodeMapping[eFrameCount])))
         m_editor.m_player->InitFPS();*/

      // Other detailed information
      ImGui::TextUnformatted(m_editor.m_player->GetPerfInfo().c_str());
   }
   ImGui::End();

   // Restore default rendering when the modal is closed (leaving the render pass override active would stick into gameplay)
   if (!m_visible)
   {
      m_passSelection = IF_FPS;
      m_editor.m_player->m_infoMode = IF_FPS;
   }
}

}
