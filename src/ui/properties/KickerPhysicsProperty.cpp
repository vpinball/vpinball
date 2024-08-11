// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/KickerPhysicsProperty.h"
#include <WindowsX.h>

KickerPhysicsProperty::KickerPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPKICKER_STATE, pvsel)
{
    m_scatterAngleEdit.SetDialog(this);
    m_hitAccuracyEdit.SetDialog(this);
    m_hitHeightEdit.SetDialog(this);
}

void KickerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);

        if (dispid == DISPID_Enabled || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hEnableCheck, kicker->m_d.m_enabled);
        if (dispid == IDC_FALL_THROUGH_HOLE || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hFallThroughCheck, kicker->m_d.m_fallThrough);
        if (dispid == IDC_LEGACYMODE || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hLegacyCheck, kicker->m_d.m_legacyMode);
        if (dispid == IDC_KICKER_HIT_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_hitHeightEdit, kicker->m_d.m_hit_height);
        if (dispid == IDC_HIT_ACC_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_hitAccuracyEdit, kicker->m_d.m_hitAccuracy);
        UpdateBaseVisuals(kicker, &kicker->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void KickerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_Enabled:
                CHECK_UPDATE_ITEM(kicker->m_d.m_enabled, PropertyDialog::GetCheckboxState(m_hEnableCheck), kicker);
                break;
            case IDC_FALL_THROUGH_HOLE:
                CHECK_UPDATE_ITEM(kicker->m_d.m_fallThrough, PropertyDialog::GetCheckboxState(m_hFallThroughCheck), kicker);
                break;
            case IDC_LEGACYMODE:
                CHECK_UPDATE_ITEM(kicker->m_d.m_legacyMode, PropertyDialog::GetCheckboxState(m_hLegacyCheck), kicker);
                break;
            case IDC_KICKER_HIT_HEIGHT_EDIT:
                CHECK_UPDATE_ITEM(kicker->m_d.m_hit_height, PropertyDialog::GetFloatTextbox(m_hitHeightEdit), kicker);
                break;
            case IDC_HIT_ACC_EDIT:
                CHECK_UPDATE_ITEM(kicker->m_d.m_hitAccuracy, PropertyDialog::GetFloatTextbox(m_hitAccuracyEdit), kicker);
                break;
            default:
                UpdateBaseProperties(kicker, &kicker->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL KickerPhysicsProperty::OnInitDialog()
{
    m_hEnableCheck = ::GetDlgItem(GetHwnd(), DISPID_Enabled);
    m_hFallThroughCheck = ::GetDlgItem(GetHwnd(), IDC_FALL_THROUGH_HOLE);
    m_hLegacyCheck = ::GetDlgItem(GetHwnd(), IDC_LEGACYMODE);
    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;

    m_hitAccuracyEdit.AttachItem(IDC_HIT_ACC_EDIT);
    m_hitHeightEdit.AttachItem(IDC_KICKER_HIT_HEIGHT_EDIT);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(m_hEnableCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hFallThroughCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hLegacyCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_scatterAngleEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hitAccuracyEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hitHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR KickerPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
