#pragma once

#include "core/stdafx.h"

#include "HitTargetUIPart.h"

namespace VPX::EditorUI
{

HitTargetUIPart::HitTargetUIPart(HitTarget* hitTarget)
   : m_hitTarget(hitTarget)
   , m_visible(hitTarget->m_d.m_visible)
{
}

HitTargetUIPart::~HitTargetUIPart() { m_hitTarget->m_d.m_visible = m_visible; }

HitTargetUIPart::TransformMask HitTargetUIPart::GetTransform(Matrix3D& transform) { return TM_None; }

void HitTargetUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { }

void HitTargetUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_hitTarget->m_d.m_visible;

   const bool isUIVisible = m_hitTarget->IsVisible(m_hitTarget);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_hitTarget->m_d.m_visible = true;
      ctx.DrawWireframe(m_hitTarget);
   }

   m_hitTarget->m_d.m_visible = isUIVisible && m_visible;
}

void HitTargetUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("HitTarget", m_hitTarget);

   if (props.BeginSection("Visual"s))
   {

      props.Checkbox<HitTarget>(
         m_hitTarget, "Reflection Enabled"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_reflectionEnabled; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection<HitTarget>(m_hitTarget, [](HitTarget* obj) { return &(obj->m_d.m_tdr); });
}

}