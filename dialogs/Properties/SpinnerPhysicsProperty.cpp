#include "stdafx.h"
#include "Properties/SpinnerPhysicsProperty.h"
#include <WindowsX.h>

SpinnerPhysicsProperty::SpinnerPhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPSPINNER_PHYSICS, pvsel)
{
    m_dampingEdit.SetDialog(this);
    m_elasticityEdit.SetDialog(this);
}

void SpinnerPhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
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
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
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
    return TRUE;
}
