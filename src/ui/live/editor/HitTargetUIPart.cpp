#pragma once

#include "core/stdafx.h"

#include "HitTargetUIPart.h"

namespace VPX::EditorUI
{

HitTargetUIPart::HitTargetUIPart(HitTarget* hitTarget)
   : m_hitTarget(hitTarget)
{
}

HitTargetUIPart::~HitTargetUIPart() { }

HitTargetUIPart::TransformMask HitTargetUIPart::GetTransform(Matrix3D& transform) { return TM_None; }

void HitTargetUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { }

void HitTargetUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.IsSelected())
      ctx.DrawHitObjects(m_hitTarget);
}

void HitTargetUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("HitTarget", m_hitTarget);

   if (props.BeginSection(PropertyPane::Section::Visual))
   {

      props.Checkbox<HitTarget>(
         m_hitTarget, "Reflection Enabled"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_reflectionEnabled; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection(PropertyPane::Section::Position))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection<HitTarget>(m_hitTarget, [](HitTarget* obj) { return &(obj->m_d.m_tdr); });
}

}