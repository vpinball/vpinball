#pragma once

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
      partGroup->m_isVisible = true;
   }
   else
   {
      partGroup->m_isVisible = false;
      for (const auto edit : partGroup->GetPTable()->m_vedit)
      {
         if (edit->GetISelect() && edit->GetISelect()->m_isVisible && edit->IsChild(partGroup))
         {
            partGroup->m_isVisible = true;
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
   props.EditableHeader("PartGroup", m_partGroup);
   
   if (props.BeginSection("Visual"s))
   {

      props.EndSection();
   }
   
   props.TimerSection<PartGroup>(m_partGroup, [](PartGroup* obj) { return &(obj->m_d.m_tdr); });
}

}