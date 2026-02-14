// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PartGroupVisualsProperty.h"

PartGroupVisualsProperty::PartGroupVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPARTGROUP_VISUALS, pvsel)
{
}

BOOL PartGroupVisualsProperty::OnInitDialog()
{
   AttachItem(IDC_SPACE_REFERENCE, m_referenceSpace);
   m_referenceSpace.SetRedraw(false);
   m_referenceSpace.ResetContent();
   m_referenceSpace.AddString("Playfield");
   m_referenceSpace.AddString("Cabinet");
   m_referenceSpace.AddString("Cabinet Feet");
   m_referenceSpace.AddString("Room");
   m_referenceSpace.AddString("Inherit");
   m_referenceSpace.SetRedraw(true);
   AttachItem(IDC_MASK_DESKTOP, m_visibilityDesktop);
   AttachItem(IDC_MASK_FSS, m_visibilityFSS);
   AttachItem(IDC_MASK_CABINET, m_visibilityCabinet);
   AttachItem(IDC_MASK_MIXED_REALITY, m_visibilityMixedReality);
   AttachItem(IDC_MASK_VIRTUAL_REALITY, m_visibilityVirtualReality);

   UpdateVisuals();
   m_resizer.Initialize(GetHwnd(), CRect(0, 0, 0, 0));
   m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_visibilityDesktop, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_visibilityFSS, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_visibilityCabinet, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_visibilityMixedReality, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_visibilityVirtualReality, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC2).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_referenceSpace, CResizer::topleft, RD_STRETCH_WIDTH);
   return TRUE;
}

INT_PTR PartGroupVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}

void PartGroupVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
   // Only show the first element on multi-select
   PartGroup* const partGroup = static_cast<PartGroup*>(m_pvsel->ElementAt(0));
   if (partGroup == nullptr)
      return;
   if (dispid == IDC_SPACE_REFERENCE || dispid == -1)
      m_referenceSpace.SetCurSel(static_cast<int>(partGroup->m_d.m_spaceReference));

   if (dispid == IDC_MASK_DESKTOP || dispid == -1)
      m_visibilityDesktop.SetCheck((partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PlayerModeVisibilityMask::PMVM_DESKTOP) ? BST_CHECKED : BST_UNCHECKED);
   if (dispid == IDC_MASK_FSS || dispid == -1)
      m_visibilityFSS.SetCheck((partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PlayerModeVisibilityMask::PMVM_FSS) ? BST_CHECKED : BST_UNCHECKED);
   if (dispid == IDC_MASK_CABINET || dispid == -1)
      m_visibilityCabinet.SetCheck((partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PlayerModeVisibilityMask::PMVM_CABINET) ? BST_CHECKED : BST_UNCHECKED);
   if (dispid == IDC_MASK_MIXED_REALITY || dispid == -1)
      m_visibilityMixedReality.SetCheck((partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PlayerModeVisibilityMask::PMVM_MIXED_REALITY) ? BST_CHECKED : BST_UNCHECKED);
   if (dispid == IDC_MASK_VIRTUAL_REALITY || dispid == -1)
      m_visibilityVirtualReality.SetCheck((partGroup->m_d.m_playerModeVisibilityMask & PartGroupData::PlayerModeVisibilityMask::PMVM_VIRTUAL_REALITY) ? BST_CHECKED : BST_UNCHECKED);
}

void PartGroupVisualsProperty::UpdatePlayerModeVisibilityMask(PartGroup* const partGroup, PartGroupData::PlayerModeVisibilityMask mask, bool checked)
{
   const bool wasChecked = (partGroup->m_d.m_playerModeVisibilityMask & mask) != 0;
   if (checked != wasChecked)
   {
      PropertyDialog::StartUndo(partGroup);
      if (checked)
         partGroup->m_d.m_playerModeVisibilityMask |= mask;
      else
         partGroup->m_d.m_playerModeVisibilityMask &= ~mask;
      PropertyDialog::EndUndo(partGroup);
   }
}

void PartGroupVisualsProperty::UpdateProperties(const int dispid)
{
   for (int i = 0; i < m_pvsel->size(); i++)
   {
      if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPartGroup))
         continue;
      PartGroup* const partGroup = static_cast<PartGroup*>(m_pvsel->ElementAt(i));
      switch (dispid)
      {
      case IDC_SPACE_REFERENCE:
         partGroup->m_d.m_spaceReference = static_cast<PartGroupData::SpaceReference>(m_referenceSpace.GetCurSel());
         break;

      case IDC_MASK_DESKTOP:
         UpdatePlayerModeVisibilityMask(partGroup, PartGroupData::PlayerModeVisibilityMask::PMVM_DESKTOP, m_visibilityDesktop.GetCheck() == BST_CHECKED);
         break;

      case IDC_MASK_FSS:
         UpdatePlayerModeVisibilityMask(partGroup, PartGroupData::PlayerModeVisibilityMask::PMVM_FSS, m_visibilityFSS.GetCheck() == BST_CHECKED);
         break;

      case IDC_MASK_CABINET:
         UpdatePlayerModeVisibilityMask(partGroup, PartGroupData::PlayerModeVisibilityMask::PMVM_CABINET, m_visibilityCabinet.GetCheck() == BST_CHECKED);
         break;

      case IDC_MASK_MIXED_REALITY:
         UpdatePlayerModeVisibilityMask(partGroup, PartGroupData::PlayerModeVisibilityMask::PMVM_MIXED_REALITY, m_visibilityMixedReality.GetCheck() == BST_CHECKED);
         break;

      case IDC_MASK_VIRTUAL_REALITY:
         UpdatePlayerModeVisibilityMask(partGroup, PartGroupData::PlayerModeVisibilityMask::PMVM_VIRTUAL_REALITY, m_visibilityVirtualReality.GetCheck() == BST_CHECKED);
         break;

      default: break;
      }
      partGroup->UpdateStatusBarInfo();
   }
   UpdateVisuals(dispid);
}
