// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/DispreelVisualsProperty.h"
#include <WindowsX.h>

DispreelVisualsProperty::DispreelVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPDISPREEL_VISUALS, pvsel)
{
    m_singleDigitRangeEdit.SetDialog(this);
    m_imagePerRowEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_reelsEdit.SetDialog(this);
    m_reelWidthEdit.SetDialog(this);
    m_reelHeightEdit.SetDialog(this);
    m_reelSpacingEdit.SetDialog(this);
    m_imageCombo.SetDialog(this);
}

void DispreelVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);

        if (dispid == IDC_BACK_TRANSP_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hBackgroundTransparentCheck, reel->m_d.m_transparent);
        if (dispid == IDC_USE_IMAGE_GRID_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hUseImageGridCheck, reel->m_d.m_useImageGrid);
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_colorButton.SetColor(reel->m_d.m_backcolor);
        if (dispid == IDC_SINGLE_DIGIT_RANGE_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_singleDigitRangeEdit, reel->GetRange());
        if (dispid == IDC_IMAGES_PER_ROW_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_imagePerRowEdit, reel->GetImagesPerGridRow());
        if (dispid == 9 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, reel->GetX());
        if (dispid == 10 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, reel->GetY());
        if (dispid == IDC_REELS_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_reelsEdit, reel->GetReels());
        if (dispid == IDC_REEL_WIDTH_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_reelWidthEdit, reel->GetWidth());
        if (dispid == IDC_REEL_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_reelHeightEdit, reel->GetHeight());
        if (dispid == IDC_REEL_SPACING_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_reelSpacingEdit, reel->GetSpacing());
        UpdateBaseVisuals(reel, &reel->m_d);
        //only show the first element on multi-select
        break;
    }
}

void DispreelVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel * const reel = (DispReel *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_BACK_TRANSP_CHECK:
                CHECK_UPDATE_ITEM(reel->m_d.m_transparent, PropertyDialog::GetCheckboxState(m_hBackgroundTransparentCheck), reel);
                break;
            case IDC_USE_IMAGE_GRID_CHECK:
                CHECK_UPDATE_ITEM(reel->m_d.m_useImageGrid, PropertyDialog::GetCheckboxState(m_hUseImageGridCheck), reel);
                break;
            case IDC_SINGLE_DIGIT_RANGE_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetRange, reel->GetRange, PropertyDialog::GetIntTextbox, m_singleDigitRangeEdit, reel);
                break;
            case IDC_IMAGES_PER_ROW_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetImagesPerGridRow, reel->GetImagesPerGridRow, PropertyDialog::GetIntTextbox, m_imagePerRowEdit, reel);
                break;
            case 9:
                CHECK_UPDATE_VALUE_SETTER(reel->SetX, reel->GetX, PropertyDialog::GetFloatTextbox, m_posXEdit, reel);
                break;
            case 10:
                CHECK_UPDATE_VALUE_SETTER(reel->SetY, reel->GetY, PropertyDialog::GetFloatTextbox, m_posYEdit, reel);
                break;
            case IDC_REELS_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetReels, reel->GetReels, PropertyDialog::GetIntTextbox, m_reelsEdit, reel);
                break;
            case IDC_REEL_WIDTH_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetWidth, reel->GetWidth, PropertyDialog::GetFloatTextbox, m_reelWidthEdit, reel);
                break;
            case IDC_REEL_HEIGHT_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetHeight, reel->GetHeight, PropertyDialog::GetFloatTextbox, m_reelHeightEdit, reel);
                break;
            case IDC_REEL_SPACING_EDIT:
                CHECK_UPDATE_VALUE_SETTER(reel->SetSpacing, reel->GetSpacing, PropertyDialog::GetFloatTextbox, m_reelSpacingEdit, reel);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(reel->m_d.m_backcolor);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    reel->m_d.m_backcolor= m_colorDialog.GetColor();
                    m_colorButton.SetColor(reel->m_d.m_backcolor);
                    memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
                }
                break;
            }
            default:
                UpdateBaseProperties(reel, &reel->m_d, dispid);
                break;
        }
        reel->UpdateStatusBarInfo();
    }
    UpdateVisuals();
}

BOOL DispreelVisualsProperty::OnInitDialog()
{
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hBackgroundTransparentCheck = ::GetDlgItem(GetHwnd(), IDC_BACK_TRANSP_CHECK);
    m_hUseImageGridCheck = ::GetDlgItem(GetHwnd(), IDC_USE_IMAGE_GRID_CHECK);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;
    m_singleDigitRangeEdit.AttachItem(IDC_SINGLE_DIGIT_RANGE_EDIT);
    m_imagePerRowEdit.AttachItem(IDC_IMAGES_PER_ROW_EDIT);
    m_posXEdit.AttachItem(9);
    m_posYEdit.AttachItem(10);
    m_reelsEdit.AttachItem(IDC_REELS_EDIT);
    m_reelWidthEdit.AttachItem(IDC_REEL_WIDTH_EDIT);
    m_reelHeightEdit.AttachItem(IDC_REEL_HEIGHT_EDIT);
    m_reelSpacingEdit.AttachItem(IDC_REEL_SPACING_EDIT);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hBackgroundTransparentCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hUseImageGridCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_colorButton, CResizer::topleft, 0);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_singleDigitRangeEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imagePerRowEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_reelsEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_reelWidthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_reelHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_reelSpacingEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR DispreelVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            const LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            const UINT nID = static_cast<UINT>(wParam);
            if (nID == IDC_COLOR_BUTTON1)
            {
                m_colorButton.DrawItem(lpDrawItemStruct);
            }
            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}
