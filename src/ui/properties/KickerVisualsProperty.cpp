// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/KickerVisualsProperty.h"
#include <WindowsX.h>

KickerVisualsProperty::KickerVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPKICKER_VISUALS, pvsel)
{
    m_typeList.push_back("Invisible"s);
    m_typeList.push_back("Hole"s);
    m_typeList.push_back("Cup"s);
    m_typeList.push_back("Simple Hole"s);
    m_typeList.push_back("Williams"s);
    m_typeList.push_back("Gottlieb"s);
    m_typeList.push_back("Cup 2"s);

    m_radiusEdit.SetDialog(this);
    m_orientationEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);

    m_materialCombo.SetDialog(this);
    m_displayCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void KickerVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);

        if (dispid == IDC_KICKER_DISPLAY_COMBO || dispid == -1)
            PropertyDialog::UpdateComboBox(m_typeList, m_displayCombo, m_typeList[kicker->m_d.m_kickertype]);
        if (dispid == IDC_KICKER_RADIUS_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_radiusEdit, kicker->m_d.m_radius);
        if (dispid == IDC_KICKER_ORIENTATION_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_orientationEdit, kicker->m_d.m_orientation);
        if (dispid == 902 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, kicker->m_d.m_vCenter.x);
        if (dispid == 903 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, kicker->m_d.m_vCenter.y);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(kicker->GetPTable(), m_surfaceCombo, kicker->m_d.m_szSurface);
        UpdateBaseVisuals(kicker, &kicker->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void KickerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_KICKER_DISPLAY_COMBO:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_kickertype = (KickerType)(PropertyDialog::GetComboBoxIndex(m_displayCombo, m_typeList));
                PropertyDialog::EndUndo(kicker);
                CHECK_UPDATE_ITEM(kicker->m_d.m_kickertype, (KickerType)(PropertyDialog::GetComboBoxIndex(m_displayCombo, m_typeList)), kicker);
                break;
            case IDC_KICKER_RADIUS_EDIT:
                CHECK_UPDATE_ITEM(kicker->m_d.m_radius, PropertyDialog::GetFloatTextbox(m_radiusEdit), kicker);
                break;
            case IDC_KICKER_ORIENTATION_EDIT:
                CHECK_UPDATE_ITEM(kicker->m_d.m_orientation, PropertyDialog::GetFloatTextbox(m_orientationEdit), kicker);
                break;
            case 902:
                CHECK_UPDATE_ITEM(kicker->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), kicker);
                break;
            case 903:
                CHECK_UPDATE_ITEM(kicker->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), kicker);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT_STRING(kicker->m_d.m_szSurface, m_surfaceCombo, kicker);
                break;
            default:
                UpdateBaseProperties(kicker, &kicker->m_d, dispid);
                break;
        }
        kicker->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL KickerVisualsProperty::OnInitDialog()
{
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_displayCombo.AttachItem(IDC_KICKER_DISPLAY_COMBO);
    m_radiusEdit.AttachItem(IDC_KICKER_RADIUS_EDIT);
    m_orientationEdit.AttachItem(IDC_KICKER_ORIENTATION_EDIT);
    m_posXEdit.AttachItem(902);
    m_posYEdit.AttachItem(903);
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_displayCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_radiusEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_orientationEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_surfaceCombo, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR KickerVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
