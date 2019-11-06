#include "stdafx.h"
#include "Properties/TriggerVisualsProperty.h"
#include <WindowsX.h>

TriggerVisualsProperty::TriggerVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTRIGGER_VISUALS, pvsel)
{
    m_shapeList.push_back("TriggerWireA");
    m_shapeList.push_back("TriggerStar");
    m_shapeList.push_back("TriggerWireB");
    m_shapeList.push_back("TriggerButton");
    m_shapeList.push_back("TriggerWireC");
    m_shapeList.push_back("TriggerWireD");

}

void TriggerVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger *trigger = (Trigger *)m_pvsel->ElementAt(i);

        PropertyDialog::UpdateComboBox(m_shapeList, m_shapeCombo, m_shapeList[(int)trigger->m_d.m_shape - 1].c_str());
        PropertyDialog::SetFloatTextbox(m_posXEdit, trigger->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, trigger->m_d.m_vCenter.y);
        PropertyDialog::SetCheckboxState(m_hVisibleCheck, trigger->m_d.m_visible);
        PropertyDialog::SetCheckboxState(m_hReflectionEnabledCheck, trigger->m_d.m_reflectionEnabled);
        PropertyDialog::SetFloatTextbox(m_wireThicknessEdit, trigger->m_d.m_wireThickness);
        PropertyDialog::SetFloatTextbox(m_starRadiusEdit, trigger->m_d.m_radius);
        PropertyDialog::SetFloatTextbox(m_rotationEdit, trigger->m_d.m_rotation);
        PropertyDialog::SetFloatTextbox(m_animationSpeedEdit, trigger->m_d.m_animSpeed);
        PropertyDialog::UpdateSurfaceComboBox(trigger->GetPTable(), m_surfaceCombo, trigger->m_d.m_szSurface);
        UpdateBaseVisuals(trigger, &trigger->m_d);
    }
}

void TriggerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger *trigger = (Trigger *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 1503:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_shape = (TriggerShape)(PropertyDialog::GetComboBoxIndex(m_shapeCombo, m_shapeList) + 1);
                PropertyDialog::EndUndo(trigger);
                break;
            case 902:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(trigger);
                break;
            case 903:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(trigger);
                break;
            case IDC_SURFACE_COMBO:
                PropertyDialog::StartUndo(trigger);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, trigger->m_d.m_szSurface);
                PropertyDialog::EndUndo(trigger);
                break;
            case IDC_STAR_THICKNESS_EDIT:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_wireThickness = PropertyDialog::GetFloatTextbox(m_wireThicknessEdit);
                PropertyDialog::EndUndo(trigger);
                break;
            case IDC_STAR_RADIUS_EDIT:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_radius = PropertyDialog::GetFloatTextbox(m_starRadiusEdit);
                PropertyDialog::EndUndo(trigger);
                break;
            case IDC_ROTATION_EDIT:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_rotation = PropertyDialog::GetFloatTextbox(m_rotationEdit);
                PropertyDialog::EndUndo(trigger);
                break;
            case IDC_RINGSPEED_EDIT:
                PropertyDialog::StartUndo(trigger);
                trigger->m_d.m_animSpeed = PropertyDialog::GetFloatTextbox(m_animationSpeedEdit);
                PropertyDialog::EndUndo(trigger);
                break;

            default:
                UpdateBaseProperties(trigger, &trigger->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL TriggerVisualsProperty::OnInitDialog()
{
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    AttachItem(IDC_SURFACE_COMBO, m_surfaceCombo);
    AttachItem(1503, m_shapeCombo);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    AttachItem(902, m_posXEdit);
    AttachItem(903, m_posYEdit);
    AttachItem(IDC_STAR_THICKNESS_EDIT, m_wireThicknessEdit);
    AttachItem(IDC_STAR_RADIUS_EDIT, m_starRadiusEdit);
    AttachItem(IDC_ROTATION_EDIT, m_rotationEdit);
    AttachItem(IDC_RINGSPEED_EDIT, m_animationSpeedEdit);

    UpdateVisuals();
    return TRUE;
}

BOOL TriggerVisualsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        case BN_CLICKED:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

