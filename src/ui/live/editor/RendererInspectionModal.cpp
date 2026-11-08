// license:GPLv3+

#include "core/stdafx.h"
#include "RendererInspectionModal.h"

#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"
#include "ViewportPane.h"

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
   {
      // The inspected view is driven by the user camera while this dialog is open: release the
      // override when it is not, so the primary view goes back to the table camera
      m_editor.m_renderer->GetPrimaryView()->m_customViewProj = false;
      return;
   }

   // FIXME m_editor.m_renderer->DisableStaticPrePass(false);
   // Keep the active camera mode so the view can be moved freely while inspecting

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
      if (m_passSelection >= 100 + static_cast<int>(m_editor.m_table->m_vrenderprobe.size()))
         m_passSelection = IF_FPS;
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

   // The inspected view is driven by the editor camera of the target viewport (the user camera):
   // the usual viewport moves apply over the window, except while over this dialog
   ViewportPane *const vp = m_editor.GetTargetViewport();
   if (vp != nullptr && vp->m_cameraValid)
   {
      const ImGuiIO &io = ImGui::GetIO();
      if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
      {
         if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !io.KeyShift)
            vp->OrbitCamera(io.MouseDelta.x, io.MouseDelta.y);
         else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && io.KeyShift)
         {
            // Pan so that the point under the cursor follows it (same math as the viewports)
            Matrix3D viewInverse(vp->m_camView);
            viewInverse.Invert();
            const vec3 up = viewInverse.GetOrthoNormalUp(), dir = viewInverse.GetOrthoNormalDir(), pos = viewInverse.GetOrthoNormalPos(), right = viewInverse.GetOrthoNormalRight();
            const vec3 camTarget = pos - dir * vp->m_camDistance;
            const float halfH = vp->m_perspectiveCam ? vp->m_camDistance * tanf(0.5f * ANGTORAD(m_editor.m_navFovY)) : vp->m_camDistance;
            const ImVec2 drag = io.MouseDelta;
            const vec3 offset = right * (drag.x * 2.f * halfH / io.DisplaySize.y) + up * (-drag.y * 2.f * halfH / io.DisplaySize.y);
            vp->m_camView = Matrix3D::MatrixLookAtRH(pos + offset, camTarget + offset, up);
         }
         if (io.MouseWheel != 0.f)
         {
            // Zoom about the orbit pivot (same math as the viewports)
            Matrix3D viewInverse(vp->m_camView);
            viewInverse.Invert();
            const vec3 up = viewInverse.GetOrthoNormalUp(), dir = viewInverse.GetOrthoNormalDir(), pos = viewInverse.GetOrthoNormalPos();
            vec3 pivot = pos - dir * vp->m_camDistance;
            vp->GetNavPivot(pivot);
            const float zoomFactor = powf(1.1f, (m_editor.m_navInvertZoom ? 1.f : -1.f) * io.MouseWheel);
            vp->m_camDistance *= zoomFactor;
            const vec3 newEye = pivot + (pos - pivot) * zoomFactor;
            vp->m_camView = Matrix3D::MatrixLookAtRH(newEye, newEye - dir * vp->m_camDistance, up);
         }
      }
      // The inspected render is driven by this camera (SetViewProj survives the per frame layout reset)
      const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
      const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
      const float aspect = io.DisplaySize.x / io.DisplaySize.y;
      Matrix3D proj;
      if (vp->m_perspectiveCam)
         proj = Matrix3D::MatrixPerspectiveFovRH(m_editor.m_navFovY, aspect, clamp(vp->m_camDistance * 0.05f, 5.f, 250.f), 50000.f);
      else
      {
         const float halfW = vp->m_camDistance * aspect, halfH = vp->m_camDistance;
         proj = Matrix3D::MatrixOrthoOffCenterRH(-halfW, halfW, -halfH, halfH, 0.5f, -50000.f);
      }
      m_editor.m_renderer->GetPrimaryView()->SetViewProj(RH2LH * vp->m_camView * YAxis, YAxis * proj);
   }

   // Restore default rendering when the modal is closed (leaving the render pass override active would stick into gameplay)
   if (!m_visible)
   {
      m_passSelection = IF_FPS;
      m_editor.m_player->m_infoMode = IF_FPS;
      m_editor.m_renderer->GetPrimaryView()->m_customViewProj = false;
   }
}

}
