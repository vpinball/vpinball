#include "stdafx.h"
#include "Properties/SpinnerVisualsProperty.h"
#include <WindowsX.h>

SpinnerVisualsProperty::SpinnerVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPSPINNER_VISUALS, pvsel)
{
}

void SpinnerVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
            continue;
        Spinner *const spinner = (Spinner *)m_pvsel->ElementAt(i);

        PropertyDialog::UpdateSurfaceComboBox(spinner->GetPTable(), m_surfaceCombo, spinner->m_d.m_szSurface);
        PropertyDialog::SetCheckboxState(m_hShowBracketCheck, spinner->m_d.m_showBracket);
        PropertyDialog::SetFloatTextbox(m_posXEdit, spinner->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, spinner->m_d.m_vCenter.y);
        PropertyDialog::SetFloatTextbox(m_lengthEdit, spinner->m_d.m_length);
        PropertyDialog::SetFloatTextbox(m_heightEdit, spinner->m_d.m_height);
        PropertyDialog::SetFloatTextbox(m_rotationEdit, spinner->m_d.m_rotation);
        PropertyDialog::SetFloatTextbox(m_angleMaxEdit, spinner->m_d.m_angleMax);
        PropertyDialog::SetFloatTextbox(m_angleMinEdit, spinner->m_d.m_angleMin);

        UpdateBaseVisuals(spinner, &spinner->m_d);
        spinner->UpdateUnitsInfo();
        //only show the first element on multi-select
        break;
    }
}

void SpinnerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
            continue;
        Spinner *const spinner = (Spinner *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_SHOW_BRACKET_CHECK:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_showBracket = PropertyDialog::GetCheckboxState(m_hShowBracketCheck);
                PropertyDialog::EndUndo(spinner);
                break;
            case IDC_SURFACE_COMBO:
                PropertyDialog::StartUndo(spinner);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, spinner->m_d.m_szSurface);
                PropertyDialog::EndUndo(spinner);
                break;
            case 11:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            case 12:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            case DISPID_Spinner_Length:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_length = PropertyDialog::GetFloatTextbox(m_lengthEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            case 5:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_height = PropertyDialog::GetFloatTextbox(m_heightEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            case 4:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_rotation = PropertyDialog::GetFloatTextbox(m_rotationEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            case 13:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_angleMax = PropertyDialog::GetFloatTextbox(m_angleMaxEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            case 14:
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_angleMin = PropertyDialog::GetFloatTextbox(m_angleMinEdit);
                PropertyDialog::EndUndo(spinner);
                break;
            default:
                UpdateBaseProperties(spinner, &spinner->m_d, dispid);
                break;
        }
    }
}

BOOL SpinnerVisualsProperty::OnInitDialog()
{
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hShowBracketCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_BRACKET_CHECK);
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    AttachItem(IDC_SURFACE_COMBO, m_surfaceCombo);
    AttachItem(11, m_posXEdit);
    AttachItem(12, m_posYEdit);
    AttachItem(DISPID_Spinner_Length, m_lengthEdit);
    AttachItem(5, m_heightEdit);
    AttachItem(4, m_rotationEdit);
    AttachItem(13, m_angleMaxEdit);
    AttachItem(14, m_angleMinEdit);
    UpdateVisuals();
    return TRUE;
}
