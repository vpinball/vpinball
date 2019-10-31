#include "stdafx.h"
#include "Properties/DragpointVisualsProperty.h"
#include <WindowsX.h>


DragpointVisualsProperty::DragpointVisualsProperty(int id, VectorProtected<ISelect> *pvsel) :BaseProperty(id, pvsel), m_id(id)
{
}

void DragpointVisualsProperty::UpdateVisuals()
{
    DragPoint *prev = NULL;

    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDragPoint))
            continue;
        DragPoint *dpoint = (DragPoint *)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 3), dpoint->m_smooth);
        if (prev!=NULL)
        {
            if(prev->m_v.x!=dpoint->m_v.x)
                m_posXEdit.SetWindowText(NULL);
            if (prev->m_v.y != dpoint->m_v.y)
                m_posYEdit.SetWindowText(NULL);
        }
        else
        {
            PropertyDialog::SetFloatTextbox(m_posXEdit, dpoint->m_v.x);
            PropertyDialog::SetFloatTextbox(m_posYEdit, dpoint->m_v.y);
        }
        if(m_id==IDD_PROPPOINT_VISUALSWHEIGHT)
        {
            PropertyDialog::SetFloatTextbox(m_realHeightEdit, dpoint->m_calcHeight);
            PropertyDialog::SetFloatTextbox(m_heightOffsetEdit, dpoint->m_v.z);
        }
        if (m_id == IDD_PROPPOINT_VISUALSWTEX)
        {
            PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 4), dpoint->m_autoTexture);
            if (prev != NULL && prev->m_texturecoord != dpoint->m_texturecoord)
                m_textureCoordEdit.SetWindowText(NULL);
            else
                PropertyDialog::SetFloatTextbox(m_textureCoordEdit, dpoint->m_texturecoord);

        }
        prev = dpoint;
    }
}

void DragpointVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDragPoint))
            continue;
        DragPoint *dpoint = (DragPoint *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case 1:
                PropertyDialog::StartUndo(dpoint);
                dpoint->m_v.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(dpoint);
                break;
            case 2:
                PropertyDialog::StartUndo(dpoint);
                dpoint->m_v.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(dpoint);
                break;
            case 4:
                PropertyDialog::StartUndo(dpoint);
                dpoint->m_autoTexture = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), 4));
                PropertyDialog::EndUndo(dpoint);
                break;
            case 5:
                PropertyDialog::StartUndo(dpoint);
                dpoint->m_texturecoord = PropertyDialog::GetFloatTextbox(m_textureCoordEdit);
                PropertyDialog::EndUndo(dpoint);
                break;
            case 6:
                PropertyDialog::StartUndo(dpoint);
                dpoint->m_v.z = PropertyDialog::GetFloatTextbox(m_heightOffsetEdit);
                PropertyDialog::EndUndo(dpoint);
                break;
            case IDC_CALC_HEIGHT_EDIT:
                PropertyDialog::StartUndo(dpoint);
                dpoint->m_calcHeight = PropertyDialog::GetFloatTextbox(m_realHeightEdit);
                PropertyDialog::EndUndo(dpoint);
                break;
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL DragpointVisualsProperty::OnInitDialog()
{
    if(m_id == IDD_PROPPOINT_VISUALSWTEX)
        AttachItem(5, m_textureCoordEdit);
    AttachItem(1, m_posXEdit);
    AttachItem(2, m_posYEdit);

    if(m_id==IDD_PROPPOINT_VISUALSWHEIGHT)
    {
        AttachItem(6, m_heightOffsetEdit);
        AttachItem(IDC_CALC_HEIGHT_EDIT, m_realHeightEdit);
    }
    AttachItem(IDC_POINT_COPY_BUTTON, m_copyButton);
    AttachItem(IDC_POINT_PASTE_BUTTON, m_pasteButton);
    UpdateVisuals();
    return TRUE;
}

BOOL DragpointVisualsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (dispID)
    {
        case IDC_POINT_COPY_BUTTON:
        {
            ISelect *const pItem = m_pvsel->ElementAt(0);
            if ((m_pvsel->Size() == 1) && (pItem->GetItemType() == eItemDragPoint))
            {
                DragPoint *pPoint = (DragPoint *)pItem;
                pPoint->Copy();
            }
            return TRUE;
        }
        case IDC_POINT_PASTE_BUTTON:
        {
            ISelect *const pItem = m_pvsel->ElementAt(0);
            if ((m_pvsel->Size() == 1) && (pItem->GetItemType() == eItemDragPoint))
            {
                DragPoint *pPoint = (DragPoint *)pItem;
                pPoint->Paste();
            }
            return TRUE;
        }
        default:
            break;
    }
    switch (HIWORD(wParam))
    {
        case EN_KILLFOCUS:
        case CBN_KILLFOCUS:
        {
            UpdateProperties(dispID);
            return TRUE;
        }
        case BN_CLICKED:
        {
            if (dispID == 3)
            {
                for (int i = 0; i < m_pvsel->Size(); i++)
                {
                    if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDragPoint))
                        continue;
                    DragPoint *dpoint = (DragPoint *)m_pvsel->ElementAt(i);
                    dpoint->DoCommand(ID_POINTMENU_SMOOTH, 0, 0);
                }
            }
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;

}

