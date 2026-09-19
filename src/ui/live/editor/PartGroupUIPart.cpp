#include "core/stdafx.h"

#include "PartGroupUIPart.h"

namespace VPX::EditorUI
{

PartGroupUIPart::PartGroupUIPart(PartGroup* partGroup)
   : EditorUIPart(partGroup)
   , m_part(partGroup)
{
}

PartGroupUIPart::TransformMask PartGroupUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void PartGroupUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
}

void PartGroupUIPart::Render(const EditorRenderContext& ctx) {
}

void PartGroupUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("PartGroup"s, m_part);

   if (props.BeginSection("Visuals"s))
   {

      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
