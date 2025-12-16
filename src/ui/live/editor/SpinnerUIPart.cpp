#pragma once

#include "core/stdafx.h"

#include "SpinnerUIPart.h"

namespace VPX::EditorUI
{

SpinnerUIPart::SpinnerUIPart(Spinner* spinner)
   : m_spinner(spinner)
{
}

SpinnerUIPart::~SpinnerUIPart() { }

SpinnerUIPart::TransformMask SpinnerUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_spinner->GetPTable()->GetSurfaceHeight(m_spinner->m_d.m_szSurface, m_spinner->m_d.m_vCenter.x, m_spinner->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_spinner->m_d.m_vCenter.x, m_spinner->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void SpinnerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
   m_spinner->m_d.m_vCenter.x = pos.x;
   m_spinner->m_d.m_vCenter.y = pos.y;
}

void SpinnerUIPart::Render(const EditorRenderContext& ctx) { }

void SpinnerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Spinner", m_spinner);
   
   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      props.Checkbox<Spinner>(m_spinner, "Reflection Enabled"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_reflectionEnabled; }, //
         [](Spinner* spinner, bool v) { spinner->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }
   
   if (props.BeginSection(PropertyPane::Section::Position))
   {
      // Missing position
      props.EndSection();
   }
   
   props.TimerSection<Spinner>(m_spinner, [](Spinner* obj) { return &(obj->m_d.m_tdr); });
}

}