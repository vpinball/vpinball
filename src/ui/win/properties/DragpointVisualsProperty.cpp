// license:GPLv3+

#include "core/stdafx.h"
#include "DragpointVisualsProperty.h"

#include "math/dragpoint.h"
#include "ui/EditorClipboard.h"
#include "ui/win/resource.h"


DragpointVisualsProperty::DragpointVisualsProperty(int id, const vector<IWinUIPart *> *pvsel) : BasePropertyDialog(id, pvsel), m_id(id)
{
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_heightOffsetEdit.SetDialog(this);
    m_realHeightEdit.SetDialog(this);
    m_textureCoordEdit.SetDialog(this);
}

void DragpointVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    const DragPoint *prev = nullptr;

    for (int i = 0; i < SelCount(); i++)
    {
        if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemDragPoint))
            continue;
        const DragPoint * const dpoint = SelAt(i)->GetDragPoint();

        PropertyDialog::SetCheckboxState(GetDlgItem(3), dpoint->IsSmooth());
        if (prev!=nullptr)
        {
            if(prev->GetX()!=dpoint->GetX() && (dispid==1 || dispid==-1))
                m_posXEdit.SetWindowText(nullptr);
            if (prev->GetY() != dpoint->GetY() && (dispid == 2 || dispid == -1))
                m_posYEdit.SetWindowText(nullptr);
        }
        else
        {
            if(dispid == 1 || dispid == -1)
                PropertyDialog::SetFloatTextbox(m_posXEdit, dpoint->GetX());
            if (dispid == 2 || dispid == -1)
                PropertyDialog::SetFloatTextbox(m_posYEdit, dpoint->GetY());
        }
        if(m_id==IDD_PROPPOINT_VISUALSWHEIGHT)
        {
            if (dispid == IDC_CALC_HEIGHT_EDIT || dispid == -1)
                PropertyDialog::SetFloatTextbox(m_realHeightEdit, dpoint->GetCalcHeight());
            if (dispid == 6 || dispid == -1)
                PropertyDialog::SetFloatTextbox(m_heightOffsetEdit, dpoint->GetZ());
        }
        if (m_id == IDD_PROPPOINT_VISUALSWTEX)
        {
            if (dispid == 4 || dispid == -1)
                PropertyDialog::SetCheckboxState(GetDlgItem(4), dpoint->IsAutoTextureCoordinate());
            if (dispid == 5 || dispid == -1)
            {
                if (prev != nullptr && prev->GetTextureCoordinateU() != dpoint->GetTextureCoordinateU())
                    m_textureCoordEdit.SetWindowText(nullptr);
                else
                    PropertyDialog::SetFloatTextbox(m_textureCoordEdit, dpoint->GetTextureCoordinateU());
            }

        }
        prev = dpoint;
    }
}

void DragpointVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < SelCount(); i++)
    {
        if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemDragPoint))
            continue;
        DragPoint * const dpoint = SelAt(i)->GetDragPoint();

        switch (dispid)
        {
            case 1:
                if (m_posXEdit.IsWindow() && !m_posXEdit.GetWindowText().IsEmpty())
                    CHECK_UPDATE_VALUE_SETTER(dpoint->SetX, dpoint->GetX, PropertyDialog::GetFloatTextbox, m_posXEdit, dpoint->GetIEditable());
                break;
            case 2:
                if (m_posYEdit.IsWindow() && !m_posYEdit.GetWindowText().IsEmpty())
                    CHECK_UPDATE_VALUE_SETTER(dpoint->SetY, dpoint->GetY, PropertyDialog::GetFloatTextbox, m_posYEdit, dpoint->GetIEditable());
                break;
            case 4:
                CHECK_UPDATE_VALUE_SETTER(dpoint->SetAutoTextureCoordinate, dpoint->IsAutoTextureCoordinate, PropertyDialog::GetCheckboxState, GetDlgItem(4), dpoint->GetIEditable());
                break;
            case 5:
                if (m_textureCoordEdit.IsWindow() && !m_textureCoordEdit.GetWindowText().IsEmpty())
                    CHECK_UPDATE_VALUE_SETTER(dpoint->SetTextureCoordinateU, dpoint->GetTextureCoordinateU, PropertyDialog::GetFloatTextbox, m_textureCoordEdit, dpoint->GetIEditable());
                break;
            case 6:
                if (m_heightOffsetEdit.IsWindow() && !m_heightOffsetEdit.GetWindowText().IsEmpty())
                    CHECK_UPDATE_VALUE_SETTER(dpoint->SetZ, dpoint->GetZ, PropertyDialog::GetFloatTextbox, m_heightOffsetEdit, dpoint->GetIEditable());
                break;
            case IDC_CALC_HEIGHT_EDIT:
                if (m_realHeightEdit.IsWindow() && !m_realHeightEdit.GetWindowText().IsEmpty())
                    CHECK_UPDATE_VALUE_SETTER(dpoint->SetCalcHeight, dpoint->GetCalcHeight, PropertyDialog::GetFloatTextbox, m_realHeightEdit, dpoint->GetIEditable());
                break;
            default:
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL DragpointVisualsProperty::OnInitDialog()
{
    if(m_id == IDD_PROPPOINT_VISUALSWTEX)
        m_textureCoordEdit.AttachItem(5);
    m_posXEdit.AttachItem(1);
    m_posYEdit.AttachItem(2);

    if(m_id==IDD_PROPPOINT_VISUALSWHEIGHT)
    {
        m_heightOffsetEdit.AttachItem(6);
        m_realHeightEdit.AttachItem(IDC_CALC_HEIGHT_EDIT);
    }
    AttachItem(IDC_POINT_COPY_BUTTON, m_copyButton);
    AttachItem(IDC_POINT_PASTE_BUTTON, m_pasteButton);
    UpdateVisuals();

    m_resizer.Initialize(GetHwnd(), CRect(0, 0, 0, 0));
    if (m_id == IDD_PROPPOINT_VISUALSWTEX)
    {
       m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, RD_STRETCH_WIDTH);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
       m_resizer.AddChild(m_textureCoordEdit, CResizer::topleft, RD_STRETCH_WIDTH);
       m_resizer.AddChild(GetDlgItem(4), CResizer::topleft, 0);
    }
    else if(m_id==IDD_PROPPOINT_VISUALSWHEIGHT)
    {
       m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, RD_STRETCH_WIDTH);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
       m_resizer.AddChild(m_heightOffsetEdit, CResizer::topleft, RD_STRETCH_WIDTH);
       m_resizer.AddChild(m_realHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
       m_resizer.AddChild(GetDlgItem(4), CResizer::topleft, 0);
    }
    else
    {
       m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, RD_STRETCH_WIDTH);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
       m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    }
    m_resizer.AddChild(GetDlgItem(3), CResizer::topleft, 0);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, 0);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, 0);
    m_resizer.AddChild(m_copyButton, CResizer::topleft, 0);
    m_resizer.AddChild(m_pasteButton, CResizer::topleft, 0);

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
            IWinUIPart *const pItem = SelAt(0);
            if ((SelCount() == 1) && (pItem->GetItemType() == eItemDragPoint))
            {
                DragPoint * const pPoint = pItem->GetDragPoint();
                VPX::EditorClipboard::CopyPoint(pPoint->GetVertex());
            }
            return TRUE;
        }
        case IDC_POINT_PASTE_BUTTON:
        {
            IWinUIPart *const pItem = SelAt(0);
            if ((SelCount() == 1) && (pItem->GetItemType() == eItemDragPoint))
            {
               Vertex3Ds pos;
               if (VPX::EditorClipboard::GetPoint(pos))
               {
                  DragPoint *const pPoint = pItem->GetDragPoint();
                  pPoint->SetX(pos.x);
                  pPoint->SetY(pos.y);
                  pPoint->SetZ(pos.z);
               }
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
                for (int i = 0; i < SelCount(); i++)
                {
                    if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemDragPoint))
                        continue;
                    DragPoint * const dpoint = SelAt(i)->GetDragPoint();
                    PropertyDialog::StartUndo(dpoint->GetIEditable());
                    dpoint->ToggleSmooth();
                    PropertyDialog::EndUndo(dpoint->GetIEditable());
                }
            }
            UpdateProperties(dispID);
            return TRUE;
        }
    }
    return FALSE;
}

INT_PTR DragpointVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
