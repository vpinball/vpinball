#include "stdafx.h"
#include "Properties/GateVisualsProperty.h"
#include <WindowsX.h>

GateVisualsProperty::GateVisualsProperty(VectorProtected<ISelect> *pvsel) : BaseProperty(IDD_PROPGATE_VISUALS, pvsel)
{
    m_typeList.push_back("GateWireW");
    m_typeList.push_back("GateWireRectangle");
    m_typeList.push_back("GatePlate");
    m_typeList.push_back("GateLongPlate");
}

void GateVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType()!=eItemGate))
            continue;
        Gate *gate = (Gate *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)gate->m_d.m_type-1].c_str());
        PropertyDialog::UpdateMaterialComboBox(gate->GetPTable()->GetMaterialList(), m_materialCombo, gate->m_d.m_szMaterial);
        PropertyDialog::UpdateSurfaceComboBox(gate->GetPTable(), m_surfaceCombo, gate->m_d.m_szSurface);
        PropertyDialog::SetFloatTextbox(m_xposEdit, gate->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_yposEdit, gate->m_d.m_vCenter.y);
        PropertyDialog::SetFloatTextbox(m_lengthEdit, gate->m_d.m_length);
        PropertyDialog::SetFloatTextbox(m_heightEdit, gate->m_d.m_height);
        PropertyDialog::SetFloatTextbox(m_lengthEdit, gate->m_d.m_length);
        PropertyDialog::SetFloatTextbox(m_rotationEdit, gate->m_d.m_rotation);
        PropertyDialog::SetFloatTextbox(m_openAngleEdit, gate->m_d.m_angleMax);
        PropertyDialog::SetFloatTextbox(m_closeAngleEdit, gate->m_d.m_angleMin);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 14), gate->m_d.m_visible);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 15), gate->m_d.m_showBracket);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), IDC_GATE_REFLECT_ENABLED_CHECK), gate->m_d.m_reflectionEnabled);
    }
}

void GateVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemGate))
            continue;
        Gate *gate = (Gate *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_type = (GateType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList)+1);
                PropertyDialog::EndUndo(gate);
                break;
            case IDC_MATERIAL_COMBO:
                PropertyDialog::StartUndo(gate);
                PropertyDialog::GetComboBoxText(m_materialCombo, gate->m_d.m_szMaterial);
                PropertyDialog::EndUndo(gate);
                break;
            case 1502:
                PropertyDialog::StartUndo(gate);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, gate->m_d.m_szSurface);
                PropertyDialog::EndUndo(gate);
                break;
            case 5:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_xposEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case 6:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_yposEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case 14:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_visible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 14));
                PropertyDialog::EndUndo(gate);
                break;
            case 15:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_showBracket = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 15));
                PropertyDialog::EndUndo(gate);
                break;
            case IDC_GATE_REFLECT_ENABLED_CHECK:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_reflectionEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), IDC_GATE_REFLECT_ENABLED_CHECK));
                PropertyDialog::EndUndo(gate);
                break;
            case DISPID_Gate_Length:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_length = PropertyDialog::GetFloatTextbox(m_lengthEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case DISPID_Gate_Height1:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_height = PropertyDialog::GetFloatTextbox(m_lengthEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case DISPID_Gate_Rotation:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_rotation = PropertyDialog::GetFloatTextbox(m_rotationEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case 2145:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_angleMax= PropertyDialog::GetFloatTextbox(m_openAngleEdit);
                PropertyDialog::EndUndo(gate);
                break;
            case 2144:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_angleMin = PropertyDialog::GetFloatTextbox(m_closeAngleEdit);
                PropertyDialog::EndUndo(gate);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL GateVisualsProperty::OnInitDialog()
{
    AttachItem(9, m_typeCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    AttachItem(1502, m_surfaceCombo);
    AttachItem(5, m_xposEdit);
    AttachItem(6, m_yposEdit);
    AttachItem(DISPID_Gate_Length, m_lengthEdit);
    AttachItem(DISPID_Gate_Height1, m_heightEdit);
    AttachItem(DISPID_Gate_Rotation, m_rotationEdit);
    AttachItem(2145, m_openAngleEdit);
    AttachItem(2144, m_closeAngleEdit);
    UpdateVisuals();
    return TRUE;
}

BOOL GateVisualsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
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

