#include "core/stdafx.h"

#include "DispReelUIPart.h"

namespace VPX::EditorUI
{

DispReelUIPart::DispReelUIPart(DispReel* dispreel)
   : m_dispreel(dispreel)
{
}

DispReelUIPart::~DispReelUIPart() { }

DispReelUIPart::TransformMask DispReelUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void DispReelUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
}

void DispReelUIPart::Render(const EditorRenderContext& ctx) { }

void DispReelUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("DispReel"s, m_dispreel);
   
   if (props.BeginSection("Visuals"s))
   {
      props.EndSection();
   }

   props.TimerSection<DispReel>(m_dispreel, [](DispReel* obj) { return &(obj->m_d.m_tdr); });
}

}
