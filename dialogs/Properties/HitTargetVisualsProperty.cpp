#include "stdafx.h"
#include "Properties/HitTargetVisualsProperty.h"
#include <WindowsX.h>

HitTargetVisualsProperty::HitTargetVisualsProperty(const VectorProtected<ISelect>* pvsel)
  : BasePropertyDialog(IDD_PROPHITTARGET_VISUALS, pvsel)
{
  m_typeList.push_back("DropTarget Beveled");
  m_typeList.push_back("DropTarget Simple");
  m_typeList.push_back("HitTarget Round");
  m_typeList.push_back("HitTarget Rectangle");
  m_typeList.push_back("HitTarget Rectangle Fat");
  m_typeList.push_back("HitTarget Square Fat");
  m_typeList.push_back("DropTarget Simple Flat");
  m_typeList.push_back("HitTarget Slim Fat");
  m_typeList.push_back("HitTarget Slim");

  m_dropSpeedEdit.SetDialog(this);
  m_raiseDelayEdit.SetDialog(this);
  m_depthBiasEdit.SetDialog(this);
  m_disableLightingEdit.SetDialog(this);
  m_disableLightBelowEdit.SetDialog(this);
  m_posXEdit.SetDialog(this);
  m_posYEdit.SetDialog(this);
  m_posZEdit.SetDialog(this);
  m_scaleXEdit.SetDialog(this);
  m_scaleYEdit.SetDialog(this);
  m_scaleZEdit.SetDialog(this);
  m_orientationEdit.SetDialog(this);

  m_imageCombo.SetDialog(this);
  m_materialCombo.SetDialog(this);
  m_typeCombo.SetDialog(this);
}

void HitTargetVisualsProperty::UpdateVisuals(const int dispid /*=-1*/)
{
  for (int i = 0; i < m_pvsel->size(); i++)
  {
    if ((m_pvsel->ElementAt(i) == nullptr) ||
        (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
    {
      continue;
    }
    HitTarget* const target = (HitTarget*)m_pvsel->ElementAt(i);
    if (dispid == IDC_HIT_TARGET_TYPE || dispid == -1)
    {
      PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo,
                                     m_typeList[target->m_d.m_targetType - 1]);
    }
    if (dispid == IDC_TARGET_MOVE_SPEED_EDIT || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_dropSpeedEdit, target->m_d.m_dropSpeed);
    }
    if (dispid == IDC_TARGET_RAISE_DELAY_EDIT || dispid == -1)
    {
      PropertyDialog::SetIntTextbox(m_raiseDelayEdit, target->m_d.m_raiseDelay);
    }
    if (dispid == IDC_DEPTH_BIAS || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_depthBiasEdit, target->m_d.m_depthBias);
    }
    if (dispid == IDC_BLEND_DISABLE_LIGHTING || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_disableLightingEdit, target->m_d.m_disableLightingTop);
    }
    if (dispid == IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_disableLightBelowEdit, target->m_d.m_disableLightingBelow);
    }
    if (dispid == DISPID_POSITION_X || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_posXEdit, target->m_d.m_vPosition.x);
    }
    if (dispid == DISPID_POSITION_Y || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_posYEdit, target->m_d.m_vPosition.y);
    }
    if (dispid == DISPID_POSITION_Z || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_posZEdit, target->m_d.m_vPosition.z);
    }
    if (dispid == DISPID_SIZE_X || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_scaleXEdit, target->m_d.m_vSize.x);
    }
    if (dispid == DISPID_SIZE_Y || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_scaleYEdit, target->m_d.m_vSize.y);
    }
    if (dispid == DISPID_SIZE_Z || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_scaleZEdit, target->m_d.m_vSize.z);
    }
    if (dispid == DISPID_ROT_Z || dispid == -1)
    {
      PropertyDialog::SetFloatTextbox(m_orientationEdit, target->m_d.m_rotZ);
    }

    UpdateBaseVisuals(target, &target->m_d, dispid);
    //only show the first element on multi-select
    break;
  }
}

void HitTargetVisualsProperty::UpdateProperties(const int dispid)
{
  for (int i = 0; i < m_pvsel->size(); i++)
  {
    if ((m_pvsel->ElementAt(i) == nullptr) ||
        (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
    {
      continue;
    }
    HitTarget* const target = (HitTarget*)m_pvsel->ElementAt(i);
    switch (dispid)
    {
      case IDC_HIT_TARGET_TYPE:
        CHECK_UPDATE_ITEM(
            target->m_d.m_targetType,
            (TargetType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList) + 1), target);
        target->UpdateStatusBarInfo();
        break;
      case IDC_TARGET_MOVE_SPEED_EDIT:
        CHECK_UPDATE_ITEM(target->m_d.m_dropSpeed, PropertyDialog::GetFloatTextbox(m_dropSpeedEdit),
                          target);
        break;
      case IDC_TARGET_RAISE_DELAY_EDIT:
        CHECK_UPDATE_ITEM(target->m_d.m_raiseDelay, PropertyDialog::GetIntTextbox(m_raiseDelayEdit),
                          target);
        break;
      case IDC_DEPTH_BIAS:
        CHECK_UPDATE_ITEM(target->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit),
                          target);
        break;
      case IDC_BLEND_DISABLE_LIGHTING:
        CHECK_UPDATE_ITEM(target->m_d.m_disableLightingTop,
                          PropertyDialog::GetFloatTextbox(m_disableLightingEdit), target);
        break;
      case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
        CHECK_UPDATE_ITEM(target->m_d.m_disableLightingBelow,
                          PropertyDialog::GetFloatTextbox(m_disableLightBelowEdit), target);
        break;
      case DISPID_POSITION_X:
        CHECK_UPDATE_ITEM(target->m_d.m_vPosition.x, PropertyDialog::GetFloatTextbox(m_posXEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      case DISPID_POSITION_Y:
        CHECK_UPDATE_ITEM(target->m_d.m_vPosition.y, PropertyDialog::GetFloatTextbox(m_posYEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      case DISPID_POSITION_Z:
        CHECK_UPDATE_ITEM(target->m_d.m_vPosition.z, PropertyDialog::GetFloatTextbox(m_posZEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      case DISPID_SIZE_X:
        CHECK_UPDATE_ITEM(target->m_d.m_vSize.x, PropertyDialog::GetFloatTextbox(m_scaleXEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      case DISPID_SIZE_Y:
        CHECK_UPDATE_ITEM(target->m_d.m_vSize.y, PropertyDialog::GetFloatTextbox(m_scaleYEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      case DISPID_SIZE_Z:
        CHECK_UPDATE_ITEM(target->m_d.m_vSize.z, PropertyDialog::GetFloatTextbox(m_scaleZEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      case DISPID_ROT_Z:
        CHECK_UPDATE_ITEM(target->m_d.m_rotZ, PropertyDialog::GetFloatTextbox(m_orientationEdit),
                          target);
        target->UpdateStatusBarInfo();
        break;
      default:
        UpdateBaseProperties(target, &target->m_d, dispid);
        break;
    }
    target->UpdateStatusBarInfo();
  }
  UpdateVisuals(dispid);
}

BOOL HitTargetVisualsProperty::OnInitDialog()
{
  m_imageCombo.AttachItem(DISPID_Image);
  m_baseImageCombo = &m_imageCombo;
  m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
  m_baseMaterialCombo = &m_materialCombo;
  m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
  m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
  m_typeCombo.AttachItem(IDC_HIT_TARGET_TYPE);
  m_dropSpeedEdit.AttachItem(IDC_TARGET_MOVE_SPEED_EDIT);
  m_raiseDelayEdit.AttachItem(IDC_TARGET_RAISE_DELAY_EDIT);
  m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
  m_disableLightingEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING);
  m_disableLightBelowEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW);
  m_posXEdit.AttachItem(DISPID_POSITION_X);
  m_posYEdit.AttachItem(DISPID_POSITION_Y);
  m_posZEdit.AttachItem(DISPID_POSITION_Z);
  m_scaleXEdit.AttachItem(DISPID_SIZE_X);
  m_scaleYEdit.AttachItem(DISPID_SIZE_Y);
  m_scaleZEdit.AttachItem(DISPID_SIZE_Z);
  m_orientationEdit.AttachItem(DISPID_ROT_Z);
  UpdateVisuals();

  m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
  m_resizer.AddChild(GetDlgItem(IDC_STATIC1), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC2), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC3), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC4), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC5), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC6), topright, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC7), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC8), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC9), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC10), topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC11), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC12), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC13), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC14), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC15), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC16), topleft, 0);
  m_resizer.AddChild(GetDlgItem(IDC_STATIC17), topleft, 0);
  m_resizer.AddChild(m_imageCombo, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_materialCombo, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_hVisibleCheck, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_hReflectionEnabledCheck, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_typeCombo, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_dropSpeedEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_raiseDelayEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_depthBiasEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_disableLightingEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_disableLightBelowEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_posXEdit, topleft, 0);
  m_resizer.AddChild(m_posYEdit, topleft, 0);
  m_resizer.AddChild(m_posZEdit, topleft, 0);
  m_resizer.AddChild(m_scaleXEdit, topright, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_scaleYEdit, topright, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_scaleZEdit, topright, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_orientationEdit, topleft, RD_STRETCH_WIDTH);

  return TRUE;
}

INT_PTR HitTargetVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  m_resizer.HandleMessage(uMsg, wParam, lParam);
  return DialogProcDefault(uMsg, wParam, lParam);
}
