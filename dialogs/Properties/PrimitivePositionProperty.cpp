#include "stdafx.h"
#include "Properties/PrimitivePositionProperty.h"
#include <WindowsX.h>

PrimitivePositionProperty::PrimitivePositionProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPRIMITIVE_POSITION, pvsel)
{
}

void PrimitivePositionProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive*)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_posXEdit, prim->m_d.m_vPosition.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, prim->m_d.m_vPosition.y);
        PropertyDialog::SetFloatTextbox(m_posZEdit, prim->m_d.m_vPosition.z);

        PropertyDialog::SetFloatTextbox(m_scaleXEdit, prim->m_d.m_vSize.x);
        PropertyDialog::SetFloatTextbox(m_scaleYEdit, prim->m_d.m_vSize.y);
        PropertyDialog::SetFloatTextbox(m_scaleZEdit, prim->m_d.m_vSize.z);

        PropertyDialog::SetFloatTextbox(m_rotXEdit, prim->m_d.m_aRotAndTra[0]);
        PropertyDialog::SetFloatTextbox(m_rotYEdit, prim->m_d.m_aRotAndTra[1]);
        PropertyDialog::SetFloatTextbox(m_rotZEdit, prim->m_d.m_aRotAndTra[2]);

        PropertyDialog::SetFloatTextbox(m_transXEdit, prim->m_d.m_aRotAndTra[3]);
        PropertyDialog::SetFloatTextbox(m_transYEdit, prim->m_d.m_aRotAndTra[4]);
        PropertyDialog::SetFloatTextbox(m_transZEdit, prim->m_d.m_aRotAndTra[5]);

        PropertyDialog::SetFloatTextbox(m_objRotXEdit, prim->m_d.m_aRotAndTra[6]);
        PropertyDialog::SetFloatTextbox(m_objRotYEdit, prim->m_d.m_aRotAndTra[7]);
        PropertyDialog::SetFloatTextbox(m_objRotZEdit, prim->m_d.m_aRotAndTra[8]);
        prim->UpdateEditorView();
        //only show the first element on multi-select
        break;
    }
}

void PrimitivePositionProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_POSITION_X:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_vPosition.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_POSITION_Y:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_vPosition.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_POSITION_Z:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_vPosition.z = PropertyDialog::GetFloatTextbox(m_posZEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_SIZE_X:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_vSize.x = PropertyDialog::GetFloatTextbox(m_scaleXEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_SIZE_Y:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_vSize.y = PropertyDialog::GetFloatTextbox(m_scaleYEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_SIZE_Z:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_vSize.z = PropertyDialog::GetFloatTextbox(m_scaleZEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA1:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[0] = PropertyDialog::GetFloatTextbox(m_rotXEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA2:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[1] = PropertyDialog::GetFloatTextbox(m_rotYEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA3:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[2] = PropertyDialog::GetFloatTextbox(m_rotZEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA4:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[3] = PropertyDialog::GetFloatTextbox(m_scaleXEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA5:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[4] = PropertyDialog::GetFloatTextbox(m_transYEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA6:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[5] = PropertyDialog::GetFloatTextbox(m_transZEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA7:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[6] = PropertyDialog::GetFloatTextbox(m_objRotXEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA8:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[7] = PropertyDialog::GetFloatTextbox(m_objRotYEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_ROTRA9:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_aRotAndTra[8] = PropertyDialog::GetFloatTextbox(m_objRotZEdit);
                PropertyDialog::EndUndo(prim);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL PrimitivePositionProperty::OnInitDialog() 
{
    AttachItem(DISPID_POSITION_X, m_posXEdit);
    AttachItem(DISPID_POSITION_Y, m_posYEdit);
    AttachItem(DISPID_POSITION_Z, m_posZEdit);
    AttachItem(DISPID_SIZE_X, m_scaleXEdit);
    AttachItem(DISPID_SIZE_Y, m_scaleYEdit);
    AttachItem(DISPID_SIZE_Z, m_scaleZEdit);
    AttachItem(DISPID_ROTRA1, m_rotXEdit);
    AttachItem(DISPID_ROTRA2, m_rotYEdit);
    AttachItem(DISPID_ROTRA3, m_rotZEdit);
    AttachItem(DISPID_ROTRA4, m_transXEdit);
    AttachItem(DISPID_ROTRA5, m_transYEdit);
    AttachItem(DISPID_ROTRA6, m_transZEdit);
    AttachItem(DISPID_ROTRA7, m_objRotXEdit);
    AttachItem(DISPID_ROTRA8, m_objRotYEdit);
    AttachItem(DISPID_ROTRA9, m_objRotZEdit);
    UpdateVisuals();
    return TRUE;
}
