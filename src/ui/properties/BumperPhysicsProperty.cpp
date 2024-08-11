// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/BumperPhysicsProperty.h"
#include <WindowsX.h>

BumperPhysicsProperty::BumperPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPBUMPER_PHYSICS, pvsel)
{
    m_forceEdit.SetDialog(this);
    m_hitThresholdEdit.SetDialog(this);
    m_scatterAngleEdit.SetDialog(this);
}

void BumperPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBumper))
            continue;
        Bumper *const bumper = (Bumper *)m_pvsel->ElementAt(i);
        if( dispid == IDC_FORCE_EDIT || dispid==-1)
            PropertyDialog::SetFloatTextbox(m_forceEdit, bumper->m_d.m_force);
        if (dispid == IDC_COLLIDABLE_CHECK || dispid == -1)
            m_forceEdit.EnableWindow(bumper->m_d.m_collidable);
        UpdateBaseVisuals(bumper, &bumper->m_d);
        //only show the first element on multi-select
        break;
    }
}

void BumperPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBumper))
            continue;
        Bumper *const bumper = (Bumper *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_FORCE_EDIT:
                CHECK_UPDATE_ITEM(bumper->m_d.m_force, PropertyDialog::GetFloatTextbox(m_forceEdit), bumper);
                break;
            default:
                UpdateBaseProperties(bumper, &bumper->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL BumperPhysicsProperty::OnInitDialog()
{
    m_hHitEventCheck = ::GetDlgItem(GetHwnd(), IDC_HAS_HITEVENT_CHECK);
    m_hCollidableCheck = ::GetDlgItem(GetHwnd(), IDC_COLLIDABLE_CHECK);
    m_hitThresholdEdit.AttachItem(IDC_HIT_THRESHOLD_EDIT);
    m_baseHitThresholdEdit = &m_hitThresholdEdit;
    m_forceEdit.AttachItem(IDC_FORCE_EDIT);
    m_scatterAngleEdit.AttachItem(IDC_SCATTER_ANGLE_EDIT);
    m_baseScatterAngleEdit = &m_scatterAngleEdit;

    UpdateVisuals();
    m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(m_hHitEventCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hCollidableCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hitThresholdEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_forceEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_scatterAngleEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR BumperPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
