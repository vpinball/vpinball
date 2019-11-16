#include "stdafx.h"
#include "Properties/KickerVisualsProperty.h"
#include <WindowsX.h>

KickerVisualsProperty::KickerVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPKICKER_VISUALS, pvsel)
{
    m_typeList.push_back("KickerInvisible");
    m_typeList.push_back("KickerHole");
    m_typeList.push_back("KickerCup");
    m_typeList.push_back("KickerHoleSimple");
    m_typeList.push_back("KickerWilliams");
    m_typeList.push_back("KickerGottlieb");
    m_typeList.push_back("KickerCup2");
}

void KickerVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);

        PropertyDialog::UpdateComboBox(m_typeList, m_displayCombo, m_typeList[kicker->m_d.m_kickertype].c_str());
        PropertyDialog::SetFloatTextbox(m_radiusEdit, kicker->m_d.m_radius);
        PropertyDialog::SetFloatTextbox(m_orientationEdit, kicker->m_d.m_orientation);
        PropertyDialog::SetFloatTextbox(m_posXEdit, kicker->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, kicker->m_d.m_vCenter.y);
        PropertyDialog::UpdateSurfaceComboBox(kicker->GetPTable(), m_surfaceCombo, kicker->m_d.m_szSurface);
        UpdateBaseVisuals(kicker, &kicker->m_d);
        //only show the first element on multi-select
        break;
    }
}

void KickerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_KICKER_DISPLAY_COMBO:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_kickertype = (KickerType)(PropertyDialog::GetComboBoxIndex(m_displayCombo, m_typeList));
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_KICKER_RADIUS_EDIT:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_radius = PropertyDialog::GetFloatTextbox(m_radiusEdit);
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_KICKER_ORIENTATION_EDIT:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_orientation = PropertyDialog::GetFloatTextbox(m_orientationEdit);
                PropertyDialog::EndUndo(kicker);
                break;
            case 902:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(kicker);
                break;
            case 903:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_SURFACE_COMBO:
                PropertyDialog::StartUndo(kicker);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, kicker->m_d.m_szSurface);
                PropertyDialog::EndUndo(kicker);
                break;
            default:
                UpdateBaseProperties(kicker, &kicker->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL KickerVisualsProperty::OnInitDialog()
{
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    AttachItem(IDC_KICKER_DISPLAY_COMBO, m_displayCombo);
    AttachItem(IDC_KICKER_RADIUS_EDIT, m_radiusEdit);
    AttachItem(IDC_KICKER_ORIENTATION_EDIT, m_orientationEdit);
    AttachItem(902, m_posXEdit);
    AttachItem(903, m_posYEdit);
    AttachItem(IDC_SURFACE_COMBO, m_surfaceCombo);
    UpdateVisuals();
    return TRUE;
}
