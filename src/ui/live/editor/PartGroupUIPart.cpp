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
      props.Separator("Play Mode Visibility Mask"s);
      props.Checkbox<PartGroup>(
         m_part, "Desktop"s, //
         [](const PartGroup* partGroup) { return (partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PMVM_DESKTOP) != 0; }, //
         [](PartGroup* partGroup, bool v)
         {
            if (v)
               partGroup->m_d.m_playerModeVisibilityMask |= PartGroupData::PMVM_DESKTOP;
            else
               partGroup->m_d.m_playerModeVisibilityMask &= ~PartGroupData::PMVM_DESKTOP;
         });
      props.Checkbox<PartGroup>(
         m_part, "Full Single Screen"s, //
         [](const PartGroup* partGroup) { return (partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PMVM_FSS) != 0; }, //
         [](PartGroup* partGroup, bool v)
         {
            if (v)
               partGroup->m_d.m_playerModeVisibilityMask |= PartGroupData::PMVM_FSS;
            else
               partGroup->m_d.m_playerModeVisibilityMask &= ~PartGroupData::PMVM_FSS;
         });
      props.Checkbox<PartGroup>(
         m_part, "Cabinet"s, //
         [](const PartGroup* partGroup) { return (partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PMVM_CABINET) != 0; }, //
         [](PartGroup* partGroup, bool v)
         {
            if (v)
               partGroup->m_d.m_playerModeVisibilityMask |= PartGroupData::PMVM_CABINET;
            else
               partGroup->m_d.m_playerModeVisibilityMask &= ~PartGroupData::PMVM_CABINET;
         });
      props.Checkbox<PartGroup>(
         m_part, "Mixed Reality"s, //
         [](const PartGroup* partGroup) { return (partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PMVM_MIXED_REALITY) != 0; }, //
         [](PartGroup* partGroup, bool v)
         {
            if (v)
               partGroup->m_d.m_playerModeVisibilityMask |= PartGroupData::PMVM_MIXED_REALITY;
            else
               partGroup->m_d.m_playerModeVisibilityMask &= ~PartGroupData::PMVM_MIXED_REALITY;
         });
      props.Checkbox<PartGroup>(
         m_part, "Virtual Reality"s, //
         [](const PartGroup* partGroup) { return (partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PMVM_VIRTUAL_REALITY) != 0; }, //
         [](PartGroup* partGroup, bool v)
         {
            if (v)
               partGroup->m_d.m_playerModeVisibilityMask |= PartGroupData::PMVM_VIRTUAL_REALITY;
            else
               partGroup->m_d.m_playerModeVisibilityMask &= ~PartGroupData::PMVM_VIRTUAL_REALITY;
         });

      props.Separator("Space Reference"s);
      props.Combo<PartGroup>(
         m_part, "Space Reference"s, vector<string> { "Playfield"s, "Cabinet"s, "Cabinet Feet"s, "Room"s, "Inherit"s }, //
         [](const PartGroup* partGroup) { return static_cast<int>(partGroup->m_d.m_spaceReference); }, //
         [](PartGroup* partGroup, int v) { partGroup->m_d.m_spaceReference = static_cast<PartGroupData::SpaceReference>(v); });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
