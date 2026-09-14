#include "core/stdafx.h"

#include "PartGroupUIPart.h"

namespace VPX::EditorUI
{

PartGroupUIPart::PartGroupUIPart(PartGroup* partGroup)
   : m_partGroup(partGroup)
{
   // Win32 UI does not manage PartGroup UI hidden/shown state, so we default to visible for inspection mode or if at least one child is visible
   if (partGroup->GetPTable()->m_liveBaseTable)
   {
      partGroup->SetUIVisible(true);
   }
   else
   {
      partGroup->SetUIVisible(false);
      for (const auto edit : partGroup->GetPTable()->GetParts())
      {
         if (edit->GetISelect() && edit->IsUIVisible(false) && edit->IsChild(partGroup))
         {
            partGroup->SetUIVisible(true);
            break;
         }
      }
   }
}

PartGroupUIPart::~PartGroupUIPart() { }

PartGroupUIPart::TransformMask PartGroupUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void PartGroupUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
}

void PartGroupUIPart::Render(const EditorRenderContext& ctx) { }

void PartGroupUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("PartGroup"s, m_partGroup);

   if (props.BeginSection("Visuals"s))
   {

      props.EndSection();
   }

   props.TimerSection(m_partGroup);
}

}
