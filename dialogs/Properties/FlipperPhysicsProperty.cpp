#include "stdafx.h"
#include "Properties/FlipperPhysicsProperty.h"
#include <WindowsX.h>

FlipperPhysicsProperty::FlipperPhysicsProperty(VectorProtected<ISelect> *pvsel) : BaseProperty(IDD_PROPFLIPPER_PHYSICS, pvsel)
{
    m_physicSetList.push_back("Set1");
    m_physicSetList.push_back("Set2");
    m_physicSetList.push_back("Set3");
    m_physicSetList.push_back("Set4");
    m_physicSetList.push_back("Set5");
    m_physicSetList.push_back("Set6");
    m_physicSetList.push_back("Set7");
    m_physicSetList.push_back("Set8");
}

void FlipperPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlipper))
            continue;
        Flipper *flipper = (Flipper *)m_pvsel->ElementAt(i);
        PropertyDialog::SetFloatTextbox(m_massEdit, flipper->m_d.m_mass);
        PropertyDialog::SetFloatTextbox(m_strengthEdit, flipper->m_d.m_strength);
        PropertyDialog::SetFloatTextbox(m_elasticityEdit, flipper->m_d.m_elasticity);
        PropertyDialog::SetFloatTextbox(m_elasticityFalloffEdit, flipper->m_d.m_elasticityFalloff);
        PropertyDialog::SetFloatTextbox(m_frictionEdit, flipper->m_d.m_friction);
        PropertyDialog::SetFloatTextbox(m_returnStrengthEdit, flipper->m_d.m_return);
        PropertyDialog::SetFloatTextbox(m_coilUpRampEdit, flipper->m_d.m_rampUp);
        PropertyDialog::SetFloatTextbox(m_scatterAngleEdit, flipper->m_d.m_scatter);
        PropertyDialog::SetFloatTextbox(m_eosTorqueEdit, flipper->m_d.m_torqueDamping);
        PropertyDialog::SetFloatTextbox(m_eosTorqueAngleEdit, flipper->m_d.m_torqueDampingAngle);
        PropertyDialog::UpdateComboBox(m_physicSetList, m_overwriteSettingsCombo, m_physicSetList[(int)flipper->m_d.m_OverridePhysics - 1].c_str());
    }
}

void FlipperPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlipper))
            continue;
        Flipper *flipper = (Flipper *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 19:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_strength = PropertyDialog::GetFloatTextbox(m_strengthEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 21:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_elasticity = PropertyDialog::GetFloatTextbox(m_elasticityEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 23:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_return = PropertyDialog::GetFloatTextbox(m_returnStrengthEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 26:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_friction = PropertyDialog::GetFloatTextbox(m_frictionEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 27:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_rampUp = PropertyDialog::GetFloatTextbox(m_coilUpRampEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 28:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_elasticityFalloff = PropertyDialog::GetFloatTextbox(m_elasticityFalloffEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 112:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_scatter = PropertyDialog::GetFloatTextbox(m_scatterAngleEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 113:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_torqueDamping = PropertyDialog::GetFloatTextbox(m_eosTorqueEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 189:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_torqueDampingAngle = PropertyDialog::GetFloatTextbox(m_eosTorqueAngleEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case DISPID_Flipper_Speed:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_mass = PropertyDialog::GetFloatTextbox(m_massEdit);
                PropertyDialog::EndUndo(flipper);
                break;
            case 1044:
                PropertyDialog::StartUndo(flipper);
                flipper->m_d.m_OverridePhysics = (PhysicsSet)(PropertyDialog::GetComboBoxIndex(m_overwriteSettingsCombo, m_physicSetList) + 1);
                PropertyDialog::EndUndo(flipper);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL FlipperPhysicsProperty::OnInitDialog()
{
    AttachItem(DISPID_Flipper_Speed, m_massEdit);
    AttachItem(19, m_strengthEdit);
    AttachItem(21, m_elasticityEdit);
    AttachItem(28, m_elasticityFalloffEdit);
    AttachItem(26, m_frictionEdit);
    AttachItem(23, m_returnStrengthEdit);
    AttachItem(27, m_coilUpRampEdit);
    AttachItem(112, m_scatterAngleEdit);
    AttachItem(113, m_eosTorqueEdit);
    AttachItem(189, m_eosTorqueAngleEdit);
    AttachItem(1044, m_overwriteSettingsCombo);
    UpdateVisuals();
    return TRUE;
}

BOOL FlipperPhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
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

