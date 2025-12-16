#pragma once

#include "core/stdafx.h"

#include "GateUIPart.h"

namespace VPX::EditorUI
{

GateUIPart::GateUIPart(Gate* gate)
   : m_gate(gate)
   , m_visible(gate->m_d.m_visible)
{
}

GateUIPart::~GateUIPart() { m_gate->m_d.m_visible = m_visible; }

GateUIPart::TransformMask GateUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_gate->GetPTable()->GetSurfaceHeight(m_gate->m_d.m_szSurface, m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void GateUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_gate->m_d.m_vCenter.x = pos.x;
   m_gate->m_d.m_vCenter.y = pos.y;
}

void GateUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_gate->m_d.m_visible;

   const bool isUIVisible = m_gate->IsVisible(m_gate);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_gate->m_d.m_visible = true;
      ctx.DrawWireframe(m_gate);
   }

   m_gate->m_d.m_visible = isUIVisible && m_visible;
}

void GateUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Gate", m_gate);

   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      props.Checkbox<Gate>(
         m_gate, "Visible"s, //
         [this](const Gate* gate) { return gate == m_gate ? m_visible : gate->m_d.m_visible; }, //
         [this](Gate* gate, bool v) { gate->m_d.m_visible = v; (gate == m_gate ? m_visible : gate->m_d.m_visible) = v; });
      props.Checkbox<Gate>(
         m_gate, "Show Bracket"s, //
         [](const Gate* gate) { return gate->m_d.m_showBracket; }, //
         [](Gate* gate, bool v) { gate->m_d.m_showBracket = v; });
      props.Checkbox<Gate>(
         m_gate, "Reflection Enabled"s, //
         [](const Gate* gate) { return gate->m_d.m_reflectionEnabled; }, //
         [](Gate* gate, bool v) { gate->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection(PropertyPane::Section::Position))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection<Gate>(m_gate, [](Gate* obj) { return &(obj->m_d.m_tdr); });
}

}