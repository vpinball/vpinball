// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/SpinnerPhysicsProperty.h"
#include <WindowsX.h>

SpinnerPhysicsProperty::SpinnerPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPSPINNER_PHYSICS, pvsel)
{
    m_dampingEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
}

void SpinnerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
            continue;
        Spinner *const spinner = (Spinner *)m_pvsel->ElementAt(i);

        if (dispid == IDC_DAMPING_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_dampingEdit, spinner->m_d.m_damping);

        UpdateBaseVisuals(spinner, &spinner->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void SpinnerPhysicsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
            continue;
        Spinner *const spinner = (Spinner *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_DAMPING_EDIT:
                CHECK_UPDATE_ITEM(spinner->m_d.m_damping, PropertyDialog::GetFloatTextbox(m_dampingEdit), spinner);
                break;

            default:
                UpdateBaseProperties(spinner, &spinner->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL SpinnerPhysicsProperty::OnInitDialog()
{
    m_elasticityEdit.AttachItem(IDC_ELASTICITY_EDIT);
    m_baseElasticityEdit = &m_elasticityEdit;
    m_dampingEdit.AttachItem(IDC_DAMPING_EDIT);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(m_elasticityEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_dampingEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR SpinnerPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
