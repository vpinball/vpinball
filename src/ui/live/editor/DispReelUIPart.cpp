#include "core/stdafx.h"

#include "DispReelUIPart.h"

namespace VPX::EditorUI
{

DispReelUIPart::DispReelUIPart(DispReel* dispreel)
   : EditableUIPart(dispreel)
{
}

DispReelUIPart::TransformMask DispReelUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void DispReelUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
}

void DispReelUIPart::RenderOverlay(const EditorRenderContext& ctx) { }

void DispReelUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("DispReel"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
