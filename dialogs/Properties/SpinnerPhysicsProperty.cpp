#include "stdafx.h"
#include "Properties/SpinnerPhysicsProperty.h"
#include <WindowsX.h>

SpinnerPhysicsProperty::SpinnerPhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPSPINNER_PHYSICS, pvsel)
{
}

void SpinnerPhysicsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSpinner))
            continue;
        Spinner *const spinner = (Spinner *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_dampingEdit, spinner->m_d.m_damping);
        UpdateBaseVisuals(spinner, &spinner->m_d);
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
                PropertyDialog::StartUndo(spinner);
                spinner->m_d.m_damping = PropertyDialog::GetFloatTextbox(m_dampingEdit);
                PropertyDialog::EndUndo(spinner);
                break;

            default:
                UpdateBaseProperties(spinner, &spinner->m_d, dispid);
                break;
        }
    }
}

BOOL SpinnerPhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_ELASTICITY_EDIT, m_elasticityEdit);
    m_baseElasticityEdit = &m_elasticityEdit;
    AttachItem(IDC_DAMPING_EDIT, m_dampingEdit);
    UpdateVisuals();
    return TRUE;
}
