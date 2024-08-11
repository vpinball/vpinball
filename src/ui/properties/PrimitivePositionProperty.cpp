// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PrimitivePositionProperty.h"
#include <WindowsX.h>

PrimitivePositionProperty::PrimitivePositionProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPRIMITIVE_POSITION, pvsel)
{
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_posZEdit.SetDialog(this);
    m_scaleXEdit.SetDialog(this);
    m_scaleYEdit.SetDialog(this);
    m_scaleZEdit.SetDialog(this);
    m_rotXEdit.SetDialog(this);
    m_rotYEdit.SetDialog(this);
    m_rotZEdit.SetDialog(this);
    m_transXEdit.SetDialog(this);
    m_transYEdit.SetDialog(this);
    m_transZEdit.SetDialog(this);
    m_objRotXEdit.SetDialog(this);
    m_objRotYEdit.SetDialog(this);
    m_objRotZEdit.SetDialog(this);
}

void PrimitivePositionProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        const Primitive *const prim = (Primitive*)m_pvsel->ElementAt(i);

        if (dispid == DISPID_POSITION_X || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, prim->m_d.m_vPosition.x);
        if (dispid == DISPID_POSITION_Y || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, prim->m_d.m_vPosition.y);
        if (dispid == DISPID_POSITION_Z || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posZEdit, prim->m_d.m_vPosition.z);

        if (dispid == DISPID_SIZE_X || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_scaleXEdit, prim->m_d.m_vSize.x);
        if (dispid == DISPID_SIZE_Y || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_scaleYEdit, prim->m_d.m_vSize.y);
        if (dispid == DISPID_SIZE_Z || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_scaleZEdit, prim->m_d.m_vSize.z);

        if (dispid == DISPID_ROTRA1 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotXEdit, prim->m_d.m_aRotAndTra[0]);
        if (dispid == DISPID_ROTRA2 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotYEdit, prim->m_d.m_aRotAndTra[1]);
        if (dispid == DISPID_ROTRA3 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotZEdit, prim->m_d.m_aRotAndTra[2]);

        if (dispid == DISPID_ROTRA4 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_transXEdit, prim->m_d.m_aRotAndTra[3]);
        if (dispid == DISPID_ROTRA5 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_transYEdit, prim->m_d.m_aRotAndTra[4]);
        if (dispid == DISPID_ROTRA6 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_transZEdit, prim->m_d.m_aRotAndTra[5]);

        if (dispid == DISPID_ROTRA7 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_objRotXEdit, prim->m_d.m_aRotAndTra[6]);
        if (dispid == DISPID_ROTRA8 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_objRotYEdit, prim->m_d.m_aRotAndTra[7]);
        if (dispid == DISPID_ROTRA9 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_objRotZEdit, prim->m_d.m_aRotAndTra[8]);

        //only show the first element on multi-select
        break;
    }
}

void PrimitivePositionProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_POSITION_X:
                CHECK_UPDATE_ITEM(prim->m_d.m_vPosition.x, PropertyDialog::GetFloatTextbox(m_posXEdit), prim);
                break;
            case DISPID_POSITION_Y:
                CHECK_UPDATE_ITEM(prim->m_d.m_vPosition.y, PropertyDialog::GetFloatTextbox(m_posYEdit), prim);
                break;
            case DISPID_POSITION_Z:
                CHECK_UPDATE_ITEM(prim->m_d.m_vPosition.z, PropertyDialog::GetFloatTextbox(m_posZEdit), prim);
                break;
            case DISPID_SIZE_X:
                CHECK_UPDATE_ITEM(prim->m_d.m_vSize.x, PropertyDialog::GetFloatTextbox(m_scaleXEdit), prim);
                break;
            case DISPID_SIZE_Y:
                CHECK_UPDATE_ITEM(prim->m_d.m_vSize.y, PropertyDialog::GetFloatTextbox(m_scaleYEdit), prim);
                break;
            case DISPID_SIZE_Z:
                CHECK_UPDATE_ITEM(prim->m_d.m_vSize.z, PropertyDialog::GetFloatTextbox(m_scaleZEdit), prim);
                break;
            case DISPID_ROTRA1:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[0], PropertyDialog::GetFloatTextbox(m_rotXEdit), prim);
                break;
            case DISPID_ROTRA2:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[1], PropertyDialog::GetFloatTextbox(m_rotYEdit), prim);
                break;
            case DISPID_ROTRA3:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[2], PropertyDialog::GetFloatTextbox(m_rotZEdit), prim);
                break;
            case DISPID_ROTRA4:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[3], PropertyDialog::GetFloatTextbox(m_transXEdit), prim);
                break;
            case DISPID_ROTRA5:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[4], PropertyDialog::GetFloatTextbox(m_transYEdit), prim);
                break;
            case DISPID_ROTRA6:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[5], PropertyDialog::GetFloatTextbox(m_transZEdit), prim);
                break;
            case DISPID_ROTRA7:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[6], PropertyDialog::GetFloatTextbox(m_objRotXEdit), prim);
                break;
            case DISPID_ROTRA8:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[7], PropertyDialog::GetFloatTextbox(m_objRotYEdit), prim);
                break;
            case DISPID_ROTRA9:
                CHECK_UPDATE_ITEM(prim->m_d.m_aRotAndTra[8], PropertyDialog::GetFloatTextbox(m_objRotZEdit), prim);
                break;
            default:
                break;
        }
        prim->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL PrimitivePositionProperty::OnInitDialog() 
{
    m_posXEdit.AttachItem(DISPID_POSITION_X);
    m_posYEdit.AttachItem(DISPID_POSITION_Y);
    m_posZEdit.AttachItem(DISPID_POSITION_Z);
    m_scaleXEdit.AttachItem(DISPID_SIZE_X);
    m_scaleYEdit.AttachItem(DISPID_SIZE_Y);
    m_scaleZEdit.AttachItem(DISPID_SIZE_Z);
    m_rotXEdit.AttachItem(DISPID_ROTRA1);
    m_rotYEdit.AttachItem(DISPID_ROTRA2);
    m_rotZEdit.AttachItem(DISPID_ROTRA3);
    m_transXEdit.AttachItem(DISPID_ROTRA4);
    m_transYEdit.AttachItem(DISPID_ROTRA5);
    m_transZEdit.AttachItem(DISPID_ROTRA6);
    m_objRotXEdit.AttachItem(DISPID_ROTRA7);
    m_objRotYEdit.AttachItem(DISPID_ROTRA8);
    m_objRotZEdit.AttachItem(DISPID_ROTRA9);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC17), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC18), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC19), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC20), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC21), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC22), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC23), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC24), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC25), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC26), CResizer::topright, 0);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, 0);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, 0);
    m_resizer.AddChild(m_posZEdit, CResizer::topleft, 0);
    m_resizer.AddChild(m_scaleXEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_scaleYEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_scaleZEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotZEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_transXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_transYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_transZEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_objRotXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_objRotYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_objRotZEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR PrimitivePositionProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
