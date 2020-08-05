#include "stdafx.h"
#include "Properties/GateVisualsProperty.h"
#include <WindowsX.h>

GateVisualsProperty::GateVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPGATE_VISUALS, pvsel)
{
    m_typeList.push_back("Wire W");
    m_typeList.push_back("Wire Rectangle");
    m_typeList.push_back("Plate");
    m_typeList.push_back("Long Plate");

    m_xposEdit.SetDialog(this);
    m_yposEdit.SetDialog(this);
    m_lengthEdit.SetDialog(this);
    m_heightEdit.SetDialog(this);
    m_rotationEdit.SetDialog(this);
    m_openAngleEdit.SetDialog(this);
    m_closeAngleEdit.SetDialog(this);

    m_typeCombo.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void GateVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType()!=eItemGate))
            continue;
        Gate * const gate = (Gate *)m_pvsel->ElementAt(i);
        if (dispid == 9 || dispid == -1)
            PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)gate->m_d.m_type - 1]);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(gate->GetPTable(), m_surfaceCombo, gate->m_d.m_szSurface);
        if (dispid == 5 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_xposEdit, gate->m_d.m_vCenter.x);
        if (dispid == 6 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_yposEdit, gate->m_d.m_vCenter.y);
        if (dispid == DISPID_Gate_Length || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_lengthEdit, gate->m_d.m_length);
        if (dispid == DISPID_Gate_Height1 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_heightEdit, gate->m_d.m_height);
        if (dispid == DISPID_Gate_Rotation || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotationEdit, gate->m_d.m_rotation);
        if (dispid == 2145 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_openAngleEdit, gate->GetOpenAngle());
        if (dispid == 2144 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_closeAngleEdit, gate->GetCloseAngle());
        if (dispid == 15 || dispid == -1)
            PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 15), gate->m_d.m_showBracket);
        UpdateBaseVisuals(gate, &gate->m_d, dispid);
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
                CHECK_UPDATE_ITEM(gate->m_d.m_type, (GateType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList) + 1), gate);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT(gate->m_d.m_szSurface, m_surfaceCombo, gate);
                break;
            case 5:
                CHECK_UPDATE_ITEM(gate->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_xposEdit), gate);
                break;
            case 6:
                CHECK_UPDATE_ITEM(gate->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_yposEdit), gate);
                break;
            case 15:
                CHECK_UPDATE_ITEM(gate->m_d.m_showBracket, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), gate);
                break;
            case DISPID_Gate_Length:
                CHECK_UPDATE_ITEM(gate->m_d.m_length, PropertyDialog::GetFloatTextbox(m_lengthEdit), gate);
                break;
            case DISPID_Gate_Height1:
                CHECK_UPDATE_ITEM(gate->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), gate);
                break;
            case DISPID_Gate_Rotation:
                CHECK_UPDATE_ITEM(gate->m_d.m_rotation, PropertyDialog::GetFloatTextbox(m_rotationEdit), gate);
                break;
            case 2145:
                CHECK_UPDATE_VALUE_SETTER(gate->SetOpenAngle, gate->GetOpenAngle, PropertyDialog::GetFloatTextbox, m_openAngleEdit, gate);
                break;
            case 2144:
                CHECK_UPDATE_VALUE_SETTER(gate->SetCloseAngle, gate->GetCloseAngle, PropertyDialog::GetFloatTextbox, m_closeAngleEdit, gate);
                break;
            default:
                UpdateBaseProperties(gate, &gate->m_d, dispid);
                break;
        }
        gate->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL GateVisualsProperty::OnInitDialog()
{
    m_typeCombo.AttachItem(9);
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
    m_xposEdit.AttachItem(5);
    m_yposEdit.AttachItem(6);
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_lengthEdit.AttachItem(DISPID_Gate_Length);
    m_heightEdit.AttachItem(DISPID_Gate_Height1);
    m_rotationEdit.AttachItem(DISPID_Gate_Rotation);
    m_openAngleEdit.AttachItem(2145);
    m_closeAngleEdit.AttachItem(2144);

    m_baseMaterialCombo = &m_materialCombo;
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);

    UpdateVisuals();
    return TRUE;
}

