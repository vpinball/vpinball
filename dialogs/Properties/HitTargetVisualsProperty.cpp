#include "stdafx.h"
#include "Properties/HitTargetVisualsProperty.h"
#include <WindowsX.h>

HitTargetVisualsProperty::HitTargetVisualsProperty(VectorProtected<ISelect> *pvsel):BasePropertyDialog(IDD_PROPHITTARGET_VISUALS, pvsel)
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
}

void HitTargetVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
            continue;
        HitTarget * const target = (HitTarget*)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[target->m_d.m_targetType - 1].c_str());
        PropertyDialog::SetFloatTextbox(m_dropSpeedEdit, target->m_d.m_dropSpeed);
        PropertyDialog::SetIntTextbox(m_raiseDelayEdit, target->m_d.m_raiseDelay);
        PropertyDialog::SetFloatTextbox(m_depthBiasEdit, target->m_d.m_depthBias);
        PropertyDialog::SetFloatTextbox(m_disableLightingEdit, target->m_d.m_disableLightingTop);
        PropertyDialog::SetFloatTextbox(m_disableLightBelowEdit, target->m_d.m_disableLightingBelow);
        PropertyDialog::SetFloatTextbox(m_posXEdit, target->m_d.m_vPosition.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, target->m_d.m_vPosition.y);
        PropertyDialog::SetFloatTextbox(m_posZEdit, target->m_d.m_vPosition.z);
        PropertyDialog::SetFloatTextbox(m_scaleXEdit, target->m_d.m_vSize.x);
        PropertyDialog::SetFloatTextbox(m_scaleYEdit, target->m_d.m_vSize.y);
        PropertyDialog::SetFloatTextbox(m_scaleZEdit, target->m_d.m_vSize.z);
        PropertyDialog::SetFloatTextbox(m_orientationEdit, target->m_d.m_rotZ);
        
        UpdateBaseVisuals(target, &target->m_d);
        //only show the first element on multi-select
        break;
    }
}

void HitTargetVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemHitTarget))
            continue;
        HitTarget * const target = (HitTarget*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_HIT_TARGET_TYPE:
                PropertyDialog::StartUndo(target);
                target->m_d.m_targetType = (TargetType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList) + 1);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case IDC_TARGET_MOVE_SPEED_EDIT:
                PropertyDialog::StartUndo(target);
                target->m_d.m_dropSpeed = PropertyDialog::GetFloatTextbox(m_dropSpeedEdit);
                PropertyDialog::EndUndo(target);
                break;
            case IDC_TARGET_RAISE_DELAY_EDIT:
                PropertyDialog::StartUndo(target);
                target->m_d.m_raiseDelay = PropertyDialog::GetIntTextbox(m_raiseDelayEdit);
                PropertyDialog::EndUndo(target);
                break;
            case IDC_DEPTH_BIAS:
                PropertyDialog::StartUndo(target);
                target->m_d.m_depthBias = PropertyDialog::GetFloatTextbox(m_depthBiasEdit);
                PropertyDialog::EndUndo(target);
                break;
            case IDC_BLEND_DISABLE_LIGHTING:
                PropertyDialog::StartUndo(target);
                target->m_d.m_disableLightingTop = PropertyDialog::GetFloatTextbox(m_disableLightingEdit);
                PropertyDialog::EndUndo(target);
                break;
            case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
                PropertyDialog::StartUndo(target);
                target->m_d.m_disableLightingBelow = PropertyDialog::GetFloatTextbox(m_disableLightBelowEdit);
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_POSITION_X:
                PropertyDialog::StartUndo(target);
                target->m_d.m_vPosition.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_POSITION_Y:
                PropertyDialog::StartUndo(target);
                target->m_d.m_vPosition.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_POSITION_Z:
                PropertyDialog::StartUndo(target);
                target->m_d.m_vPosition.z = PropertyDialog::GetFloatTextbox(m_posZEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_SIZE_X:
                PropertyDialog::StartUndo(target);
                target->m_d.m_vSize.x = PropertyDialog::GetFloatTextbox(m_scaleXEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_SIZE_Y:
                PropertyDialog::StartUndo(target);
                target->m_d.m_vSize.y = PropertyDialog::GetFloatTextbox(m_scaleYEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_SIZE_Z:
                PropertyDialog::StartUndo(target);
                target->m_d.m_vSize.z = PropertyDialog::GetFloatTextbox(m_scaleZEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            case DISPID_ROT_Z:
                PropertyDialog::StartUndo(target);
                target->m_d.m_rotZ = PropertyDialog::GetFloatTextbox(m_orientationEdit);
                target->UpdateEditorView();
                PropertyDialog::EndUndo(target);
                break;
            default:
                UpdateBaseProperties(target, &target->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL HitTargetVisualsProperty::OnInitDialog()
{
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    m_hVisibleCheck= ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    AttachItem(IDC_HIT_TARGET_TYPE, m_typeCombo);
    AttachItem(IDC_TARGET_MOVE_SPEED_EDIT, m_dropSpeedEdit);
    AttachItem(IDC_TARGET_RAISE_DELAY_EDIT, m_raiseDelayEdit);
    AttachItem(IDC_DEPTH_BIAS, m_depthBiasEdit);
    AttachItem(IDC_BLEND_DISABLE_LIGHTING, m_disableLightingEdit);
    AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW, m_disableLightBelowEdit);
    AttachItem(DISPID_POSITION_X, m_posXEdit);
    AttachItem(DISPID_POSITION_Y, m_posYEdit);
    AttachItem(DISPID_POSITION_Z, m_posZEdit);
    AttachItem(DISPID_SIZE_X, m_scaleXEdit);
    AttachItem(DISPID_SIZE_Y, m_scaleYEdit);
    AttachItem(DISPID_SIZE_Z, m_scaleZEdit);
    AttachItem(DISPID_ROT_Z, m_orientationEdit);
    UpdateVisuals();
    return TRUE;
}
