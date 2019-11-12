#include "stdafx.h"
#include "Properties/DispreelVisualsProperty.h"
#include <WindowsX.h>

DispreelVisualsProperty::DispreelVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPDISPREEL_VISUALS, pvsel)
{
}

void DispreelVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel *reel = (DispReel *)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hBackgroundTransparentCheck, reel->m_d.m_transparent);
        PropertyDialog::SetCheckboxState(m_hUseImageGridCheck, reel->m_d.m_useImageGrid);
        m_colorButton.SetColor(reel->m_d.m_backcolor);
        PropertyDialog::SetIntTextbox(m_singleDigitRangeEdit, reel->GetRange());
        PropertyDialog::SetIntTextbox(m_imagePerRowEdit, reel->GetImagesPerGridRow());
        PropertyDialog::SetFloatTextbox(m_posXEdit, reel->GetX());
        PropertyDialog::SetFloatTextbox(m_posYEdit, reel->GetY());
        PropertyDialog::SetIntTextbox(m_reelsEdit, reel->GetReels());
        PropertyDialog::SetFloatTextbox(m_reelWidthEdit, reel->GetWidth());
        PropertyDialog::SetFloatTextbox(m_reelHeightEdit, reel->GetHeight());
        PropertyDialog::SetFloatTextbox(m_reelSpacingEdit, reel->GetSpacing());
        UpdateBaseVisuals(reel, &reel->m_d);
        //only show the first element on multi-select
        break;

    }
}

void DispreelVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemDispReel))
            continue;
        DispReel *reel = (DispReel *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_BACK_TRANSP_CHECK:
                PropertyDialog::StartUndo(reel);
                reel->m_d.m_transparent = PropertyDialog::GetCheckboxState(m_hBackgroundTransparentCheck);
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_USE_IMAGE_GRID_CHECK:
                PropertyDialog::StartUndo(reel);
                reel->m_d.m_useImageGrid = PropertyDialog::GetCheckboxState(m_hUseImageGridCheck);
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_SINGLE_DIGIT_RANGE_EDIT:
                PropertyDialog::StartUndo(reel);
                reel->SetRange(PropertyDialog::GetIntTextbox(m_singleDigitRangeEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_IMAGES_PER_ROW_EDIT:
                PropertyDialog::StartUndo(reel);
                reel->SetImagesPerGridRow(PropertyDialog::GetIntTextbox(m_imagePerRowEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case 9:
                PropertyDialog::StartUndo(reel);
                reel->SetX(PropertyDialog::GetFloatTextbox(m_posXEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case 10:
                PropertyDialog::StartUndo(reel);
                reel->SetY(PropertyDialog::GetFloatTextbox(m_posYEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_REELS_EDIT:
                PropertyDialog::StartUndo(reel);
                reel->SetReels(PropertyDialog::GetIntTextbox(m_reelsEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_REEL_WIDTH_EDIT:
                PropertyDialog::StartUndo(reel);
                reel->SetWidth(PropertyDialog::GetFloatTextbox(m_reelWidthEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_REEL_HEIGHT_EDIT:
                PropertyDialog::StartUndo(reel);
                reel->SetHeight(PropertyDialog::GetFloatTextbox(m_reelHeightEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_REEL_SPACING_EDIT:
                PropertyDialog::StartUndo(reel);
                reel->SetSpacing(PropertyDialog::GetFloatTextbox(m_reelSpacingEdit));
                PropertyDialog::EndUndo(reel);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(reel->m_d.m_backcolor);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    reel->m_d.m_backcolor= m_colorDialog.GetColor();
                    m_colorButton.SetColor(reel->m_d.m_backcolor);
                }
                break;
            }
            default:
                UpdateBaseProperties(reel, &reel->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL DispreelVisualsProperty::OnInitDialog()
{
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hBackgroundTransparentCheck = ::GetDlgItem(GetHwnd(), IDC_BACK_TRANSP_CHECK);
    m_hUseImageGridCheck = ::GetDlgItem(GetHwnd(), IDC_USE_IMAGE_GRID_CHECK);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
    AttachItem(IDC_SINGLE_DIGIT_RANGE_EDIT, m_singleDigitRangeEdit);
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    AttachItem(IDC_IMAGES_PER_ROW_EDIT, m_imagePerRowEdit);
    AttachItem(9, m_posXEdit);
    AttachItem(10, m_posYEdit);
    AttachItem(IDC_REELS_EDIT, m_reelsEdit);
    AttachItem(IDC_REEL_WIDTH_EDIT, m_reelWidthEdit);
    AttachItem(IDC_REEL_HEIGHT_EDIT, m_reelHeightEdit);
    AttachItem(IDC_REEL_SPACING_EDIT, m_reelSpacingEdit);
    UpdateVisuals();
    return TRUE;
}

INT_PTR DispreelVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            UINT nID = static_cast<UINT>(wParam);
            if (nID == IDC_COLOR_BUTTON1)
            {
                m_colorButton.DrawItem(lpDrawItemStruct);
            }
            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}

