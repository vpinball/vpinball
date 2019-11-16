#include "stdafx.h"
#include "Properties/KickerPhysicsProperty.h"
#include <WindowsX.h>

KickerPhysicsProperty::KickerPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPKICKER_STATE, pvsel)
{
}

void KickerPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hEnableCheck, kicker->m_d.m_enabled);
        PropertyDialog::SetCheckboxState(m_hFallThroughCheck, kicker->m_d.m_fallThrough);
        PropertyDialog::SetCheckboxState(m_hLegacyCheck, kicker->m_d.m_legacyMode);
        PropertyDialog::SetFloatTextbox(m_hitHeightEdit, kicker->m_d.m_hit_height);
        PropertyDialog::SetFloatTextbox(m_hitAccuracyEdit, kicker->m_d.m_hitAccuracy);
        UpdateBaseVisuals(kicker, &kicker->m_d);
        //only show the first element on multi-select
        break;
    }
}

void KickerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemKicker))
            continue;
        Kicker * const kicker = (Kicker *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_Enabled:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_enabled = PropertyDialog::GetCheckboxState(m_hEnableCheck);
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_FALL_THROUGH_HOLE:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_fallThrough = PropertyDialog::GetCheckboxState(m_hFallThroughCheck);
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_LEGACYMODE:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_legacyMode = PropertyDialog::GetCheckboxState(m_hLegacyCheck);
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_KICKER_HIT_HEIGHT_EDIT:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_hit_height = PropertyDialog::GetFloatTextbox(m_hitHeightEdit);
                PropertyDialog::EndUndo(kicker);
                break;
            case IDC_HIT_ACC_EDIT:
                PropertyDialog::StartUndo(kicker);
                kicker->m_d.m_hitAccuracy = PropertyDialog::GetFloatTextbox(m_hitAccuracyEdit);
                PropertyDialog::EndUndo(kicker);
                break;
            default:
                UpdateBaseProperties(kicker, &kicker->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL KickerPhysicsProperty::OnInitDialog()
{
    m_hEnableCheck = ::GetDlgItem(GetHwnd(), DISPID_Enabled);
    m_hFallThroughCheck = ::GetDlgItem(GetHwnd(), IDC_FALL_THROUGH_HOLE);
    m_hLegacyCheck = ::GetDlgItem(GetHwnd(), IDC_LEGACYMODE);
    AttachItem(IDC_SCATTER_ANGLE_EDIT, m_scatterAngleEdit);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;
    AttachItem(IDC_HIT_ACC_EDIT, m_hitAccuracyEdit);
    AttachItem(IDC_KICKER_HIT_HEIGHT_EDIT, m_hitHeightEdit);
    UpdateVisuals();
    return TRUE;
}
