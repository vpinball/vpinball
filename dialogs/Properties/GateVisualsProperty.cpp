#include "stdafx.h"
#include "Properties/GateVisualsProperty.h"
#include <WindowsX.h>

GateVisualsProperty::GateVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPGATE_VISUALS, pvsel)
{
    m_typeList.push_back("Wire W");
    m_typeList.push_back("Wire Rectangle");
    m_typeList.push_back("Plate");
    m_typeList.push_back("Long Plate");
}

void GateVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType()!=eItemGate))
            continue;
        Gate * const gate = (Gate *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)gate->m_d.m_type-1].c_str());
        PropertyDialog::UpdateSurfaceComboBox(gate->GetPTable(), m_surfaceCombo, gate->m_d.m_szSurface);
        PropertyDialog::SetFloatTextbox(m_xposEdit, gate->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_yposEdit, gate->m_d.m_vCenter.y);
        PropertyDialog::SetFloatTextbox(m_lengthEdit, gate->m_d.m_length);
        PropertyDialog::SetFloatTextbox(m_heightEdit, gate->m_d.m_height);
        PropertyDialog::SetFloatTextbox(m_lengthEdit, gate->m_d.m_length);
        PropertyDialog::SetFloatTextbox(m_rotationEdit, gate->m_d.m_rotation);
        PropertyDialog::SetFloatTextbox(m_openAngleEdit, gate->m_d.m_angleMax);
        PropertyDialog::SetFloatTextbox(m_closeAngleEdit, gate->m_d.m_angleMin);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 15), gate->m_d.m_showBracket);
        UpdateBaseVisuals(gate, &gate->m_d);
        //only show the first element on multi-select
        break;
    }
}

void GateVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemGate))
            continue;
        Gate * const gate = (Gate *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_type = (GateType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList)+1);
                PropertyDialog::EndUndo(gate);
                break;
            case IDC_SURFACE_COMBO:
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
            case 15:
                PropertyDialog::StartUndo(gate);
                gate->m_d.m_showBracket = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
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
                UpdateBaseProperties(gate, &gate->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL GateVisualsProperty::OnInitDialog()
{
    AttachItem(9, m_typeCombo);
    AttachItem(IDC_SURFACE_COMBO, m_surfaceCombo);
    AttachItem(5, m_xposEdit);
    AttachItem(6, m_yposEdit);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    AttachItem(DISPID_Gate_Length, m_lengthEdit);
    AttachItem(DISPID_Gate_Height1, m_heightEdit);
    AttachItem(DISPID_Gate_Rotation, m_rotationEdit);
    AttachItem(2145, m_openAngleEdit);
    AttachItem(2144, m_closeAngleEdit);

    m_baseMaterialCombo = &m_materialCombo;
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);

    UpdateVisuals();
    return TRUE;
}

