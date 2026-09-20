// license:GPLv3+

#include "core/stdafx.h"
#include "LiveRenderContext.h"

#include "renderer/Renderer.h"
#include "utils/color.h"

namespace VPX::EditorUI
{

LiveRenderContext::LiveRenderContext(Player *player, ImDrawList *drawlist, ViewMode viewMode, Renderer::ShadeMode shadeMode, bool needsLiveTableSync)
   : m_player(player)
   , m_drawlist(drawlist)
   , m_viewMode(viewMode)
   , m_shadeMode(shadeMode)
   , m_needsLiveTableSync(needsLiveTableSync)
{
}

bool LiveRenderContext::IsShowInvisible() const
{
   // Don't show invisible part in Live edit mode as there is no editor visibility management in this mode
   return (m_player->m_ptable->m_liveBaseTable == nullptr) && (GetViewMode() != ViewMode::PreviewCam);
}

ImVec2 LiveRenderContext::Project(const Vertex3Ds &v) const
{
   const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
   const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
   const Matrix3D mvp = m_player->m_renderer->GetMVP().GetModelViewProj(0);
   const float xp = mvp._11 * v.x + mvp._21 * v.y + mvp._31 * v.z + mvp._41;
   const float yp = mvp._12 * v.x + mvp._22 * v.y + mvp._32 * v.z + mvp._42;
   //const float zp = mvp._13 * v.x + mvp._23 * v.y + mvp._33 * v.z + mvp._43;
   const float wp = mvp._14 * v.x + mvp._24 * v.y + mvp._34 * v.z + mvp._44;
   if (wp <= 1e-10f) // behind camera (or degenerated)
      return ImVec2 { FLT_MAX, FLT_MAX };
   const float inv_wp = 1.0f / wp;
   return ImVec2 { (wp + xp) * rClipWidth * inv_wp, (wp - yp) * rClipHeight * inv_wp };
}

void LiveRenderContext::DrawLine(const Vertex3Ds &a, const Vertex3Ds &b, ImU32 color) const
{
   if (m_drawlist)
   {
      const ImVec2 p1 = Project(a);
      const ImVec2 p2 = Project(b);
      m_drawlist->AddLine(p1, p2, color);
   }
   // TODO also render when running in 3D (for logic parts like timers,...)
}

void LiveRenderContext::DrawCircle(const Vertex3Ds &center, const Vertex3Ds &x, const Vertex3Ds &y, float radius, ImU32 color) const
{
   if (m_drawlist)
   {
      ImVec2 prev;
      constexpr int n = 32;
      for (int i = 0; i <= n; i++)
      {
         const float c = radius * cos((float)i * (float)(2. * M_PI / n));
         const float s = radius * sin((float)i * (float)(2. * M_PI / n));
         const ImVec2 p = Project(Vertex3Ds(center.x + c * x.x + s * y.x, center.y + c * x.y + s * y.y, center.z + c * x.z + s * y.z));
         if (i > 0 && p.x != FLT_MAX && prev.x != FLT_MAX) // Skip segments ending behind the camera
            GetDrawList()->AddLine(prev, p, color, 1.f);
         prev = p;
      }
   }
   // TODO also render when running in 3D (for logic parts like timers,...)
}

void LiveRenderContext::DrawHitObjects(IEditable *editable) const
{
   if (m_drawlist)
   {
      auto project = [this](Vertex3Ds v)
      {
         const ImVec2 pt = Project(v);
         return Vertex2D(pt.x, pt.y);
      };
      const ImU32 color = GetColor(m_isSelected);
      const ImU32 alpha = (color & 0x00FFFFFFu) | 0x20000000u;
      ImGui::PushStyleColor(ImGuiCol_PlotLines, color);
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, alpha);
      for (auto pho : m_player->m_physics->GetUIHitObjects(editable))
         pho->DrawUI(project, m_drawlist, true);
      ImGui::PopStyleColor(2);
   }
}

void LiveRenderContext::DrawWireframe(IEditable *editable) const
{
   if (IsSelected())
   {
      // Selection overlay
      g_pplayer->m_renderer->DrawWireframe(editable, convertColor(GetColor(true), 32.f / 255.f), convertColor(GetColor(true), 1.f), false);
   }
   else
   {
      // "Invisible" part, but UI visible
      const vec4 fillColor = m_shadeMode == Renderer::ShadeMode::NoDepthWireframe ? vec4(0.f, 0.f, 0.f, 32.f / 255.f) : vec4(32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.f);
      const vec4 edgeColor(0.f, 0.f, 0.f, 1.f);
      g_pplayer->m_renderer->DrawWireframe(editable, fillColor, edgeColor, m_shadeMode != Renderer::ShadeMode::NoDepthWireframe);
   }
}

}
