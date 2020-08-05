#include "stdafx.h"
#include "Properties/SpinnerVisualsProperty.h"
#include <WindowsX.h>

SpinnerVisualsProperty::SpinnerVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPSPINNER_VISUALS, pvsel)
{
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_lengthEdit.SetDialog(this);
    m_heightEdit.SetDialog(this);
    m_rotationEdit.SetDialog(this);
    m_angleMaxEdit.SetDialog(this);
    m_angleMinEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void SpinnerVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
            continue;
        Spinner *const spinner = (Spinner *)m_pvsel->ElementAt(i);

        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(spinner->GetPTable(), m_surfaceCombo, spinner->m_d.m_szSurface);
        if (dispid == IDC_SHOW_BRACKET_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hShowBracketCheck, spinner->m_d.m_showBracket);
        if (dispid == 11 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, spinner->m_d.m_vCenter.x);
        if (dispid == 12 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, spinner->m_d.m_vCenter.y);
        if (dispid == DISPID_Spinner_Length || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_lengthEdit, spinner->m_d.m_length);
        if (dispid == 5 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_heightEdit, spinner->m_d.m_height);
        if (dispid == 4 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotationEdit, spinner->m_d.m_rotation);
        if (dispid == 13 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_angleMaxEdit, spinner->GetAngleMax());
        if (dispid == 14 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_angleMinEdit, spinner->GetAngleMin());

        UpdateBaseVisuals(spinner, &spinner->m_d, dispid);
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
                CHECK_UPDATE_ITEM(spinner->m_d.m_showBracket, PropertyDialog::GetCheckboxState(m_hShowBracketCheck), spinner);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT(spinner->m_d.m_szSurface, m_surfaceCombo, spinner);
                break;
            case 11:
                CHECK_UPDATE_ITEM(spinner->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), spinner);
                break;
            case 12:
                CHECK_UPDATE_ITEM(spinner->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), spinner);
                break;
            case DISPID_Spinner_Length:
                CHECK_UPDATE_ITEM(spinner->m_d.m_length, PropertyDialog::GetFloatTextbox(m_lengthEdit), spinner);
                break;
            case 5:
                CHECK_UPDATE_ITEM(spinner->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), spinner);
                break;
            case 4:
                CHECK_UPDATE_ITEM(spinner->m_d.m_rotation, PropertyDialog::GetFloatTextbox(m_rotationEdit), spinner);
                break;
            case 13:
                CHECK_UPDATE_VALUE_SETTER(spinner->SetAngleMax, spinner->GetAngleMax, PropertyDialog::GetFloatTextbox, m_angleMaxEdit, spinner);
                break;
            case 14:
                CHECK_UPDATE_VALUE_SETTER(spinner->SetAngleMin, spinner->GetAngleMin, PropertyDialog::GetFloatTextbox, m_angleMinEdit, spinner);
                break;
            default:
                UpdateBaseProperties(spinner, &spinner->m_d, dispid);
                break;
        }
        spinner->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL SpinnerVisualsProperty::OnInitDialog()
{
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hShowBracketCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_BRACKET_CHECK);
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
    m_posXEdit.AttachItem(11);
    m_posYEdit.AttachItem(12);
    m_lengthEdit.AttachItem(DISPID_Spinner_Length);
    m_heightEdit.AttachItem(5);
    m_rotationEdit.AttachItem(4);
    m_angleMaxEdit.AttachItem(13);
    m_angleMinEdit.AttachItem(14);
    UpdateVisuals();
    return TRUE;
}
