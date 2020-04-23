#include "stdafx.h"
#include "Properties/FlasherVisualsProperty.h"
#include <WindowsX.h>

FlasherVisualsProperty::FlasherVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPFLASHER_VISUALS, pvsel)
{
    m_filterList.push_back("None");
    m_filterList.push_back("Additive");
    m_filterList.push_back("Overlay");
    m_filterList.push_back("Multiply");
    m_filterList.push_back("Screen");

    m_imageAlignList.push_back("World");
    m_imageAlignList.push_back("Wrap");
}

void FlasherVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlasher))
            continue;
        Flasher * const flash = (Flasher *)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hUseDMDCheck, flash->m_d.m_isDMD);
        PropertyDialog::SetCheckboxState(m_hDisplayInEditorCheck, flash->m_d.m_displayTexture);
        PropertyDialog::SetCheckboxState(m_hAdditiveBlendCheck, flash->m_d.m_addBlend);
        PropertyDialog::SetCheckboxState(m_hVisibleCheck, flash->m_d.m_isVisible);
        PropertyDialog::SetFloatTextbox(m_posXEdit, flash->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, flash->m_d.m_vCenter.y);
        PropertyDialog::SetFloatTextbox(m_heightEdit, flash->m_d.m_height);
        PropertyDialog::SetFloatTextbox(m_rotXEdit, flash->m_d.m_rotX);
        PropertyDialog::SetFloatTextbox(m_rotYEdit, flash->m_d.m_rotY);
        PropertyDialog::SetFloatTextbox(m_rotZEdit, flash->m_d.m_rotZ);
        PropertyDialog::SetIntTextbox(m_filterAmountEdit, flash->m_d.m_filterAmount);
        PropertyDialog::SetIntTextbox(m_opacyitAmountEdit, flash->m_d.m_alpha);
        PropertyDialog::SetFloatTextbox(m_depthBiasEdit, flash->m_d.m_depthBias);
        PropertyDialog::SetFloatTextbox(m_modulateEdit, flash->m_d.m_modulate_vs_add);
        PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_imageACombo, flash->m_d.m_szImageA);
        PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_imageBCombo, flash->m_d.m_szImageB);
        PropertyDialog::UpdateComboBox(m_imageAlignList, m_modeCombo, m_imageAlignList[(int)flash->m_d.m_imagealignment-1].c_str());
        PropertyDialog::UpdateComboBox(m_filterList, m_filterCombo, m_filterList[flash->m_d.m_filter].c_str());
        m_colorButton.SetColor(flash->m_d.m_color);
        //only show the first element on multi-select
        break;
    }
}

void FlasherVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlasher))
            continue;
        Flasher * const flash = (Flasher *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_VISIBLE_CHECK:
                CHECK_UPDATE_ITEM(flash->m_d.m_isVisible, PropertyDialog::GetCheckboxState(m_hVisibleCheck), flash);
                break;
            case IDC_DISPLAY_IMAGE_CHECK:
                CHECK_UPDATE_ITEM(flash->m_d.m_displayTexture, PropertyDialog::GetCheckboxState(m_hDisplayInEditorCheck), flash);
                break;
            case IDC_ADDBLEND:
                CHECK_UPDATE_ITEM(flash->m_d.m_addBlend, PropertyDialog::GetCheckboxState(m_hAdditiveBlendCheck), flash);
                break;
            case IDC_DMD:
                CHECK_UPDATE_ITEM(flash->m_d.m_isDMD, PropertyDialog::GetCheckboxState(m_hUseDMDCheck), flash);
                break;
            case DISPID_Image:
                CHECK_UPDATE_COMBO_TEXT(flash->m_d.m_szImageA, m_imageACombo, flash);
                break;
            case DISPID_Image2:
                CHECK_UPDATE_COMBO_TEXT(flash->m_d.m_szImageB, m_imageBCombo, flash);
                break;
            case IDC_FLASHER_MODE_COMBO:
                CHECK_UPDATE_ITEM(flash->m_d.m_imagealignment, (RampImageAlignment)(PropertyDialog::GetComboBoxIndex(m_modeCombo, m_imageAlignList) + 1), flash);
                break;
            case IDC_EFFECT_COMBO:
                CHECK_UPDATE_ITEM(flash->m_d.m_filter, (Filters)PropertyDialog::GetComboBoxIndex(m_filterCombo, m_filterList), flash);
                break;
            case IDC_DEPTH_BIAS:
                CHECK_UPDATE_ITEM(flash->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit), flash);
                break;
            case IDC_FILTERAMOUNT_EDIT:
                CHECK_UPDATE_VALUE_SETTER(flash->SetFilterAmount, flash->GetFilterAmount, PropertyDialog::GetIntTextbox, m_filterAmountEdit, flash);
                break;
            case IDC_ALPHA_EDIT:
                CHECK_UPDATE_VALUE_SETTER(flash->SetAlpha, flash->GetAlpha, PropertyDialog::GetIntTextbox, m_opacyitAmountEdit, flash);
                break;
            case IDC_MODULATE_VS_ADD:
                CHECK_UPDATE_ITEM(flash->m_d.m_modulate_vs_add, PropertyDialog::GetFloatTextbox(m_modulateEdit), flash);
                break;
            case 5:
                CHECK_UPDATE_ITEM(flash->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), flash);
                break;
            case 6:
                CHECK_UPDATE_ITEM(flash->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), flash);
                break;
            case IDC_HEIGHT_EDIT:
                CHECK_UPDATE_ITEM(flash->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), flash);
                break;
            case 9:
                CHECK_UPDATE_ITEM(flash->m_d.m_rotX, PropertyDialog::GetFloatTextbox(m_rotXEdit), flash);
                break;
            case 2:
                CHECK_UPDATE_ITEM(flash->m_d.m_rotY, PropertyDialog::GetFloatTextbox(m_rotYEdit), flash);
                break;
            case 1:
                CHECK_UPDATE_ITEM(flash->m_d.m_rotZ, PropertyDialog::GetFloatTextbox(m_rotZEdit), flash);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(flash->m_d.m_color);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    flash->m_d.m_color = m_colorDialog.GetColor();
                    m_colorButton.SetColor(flash->m_d.m_color);
                }
                break;
            }
            default:
                break;
        }
    }
    UpdateVisuals();
}

BOOL FlasherVisualsProperty::OnInitDialog()
{
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    AttachItem(DISPID_Image, m_imageACombo);
    m_hDisplayInEditorCheck = ::GetDlgItem(GetHwnd(), IDC_DISPLAY_IMAGE_CHECK);
    AttachItem(DISPID_Image2, m_imageBCombo);
    AttachItem(IDC_FLASHER_MODE_COMBO, m_modeCombo);
    AttachItem(IDC_EFFECT_COMBO, m_filterCombo);
    AttachItem(IDC_FILTERAMOUNT_EDIT, m_filterAmountEdit);
    AttachItem(IDC_ALPHA_EDIT, m_opacyitAmountEdit);
    m_hAdditiveBlendCheck = ::GetDlgItem(GetHwnd(), IDC_ADDBLEND);
    m_hUseDMDCheck = ::GetDlgItem(GetHwnd(), IDC_DMD);
    AttachItem(IDC_DEPTH_BIAS, m_depthBiasEdit);
    AttachItem(IDC_MODULATE_VS_ADD, m_modulateEdit);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
    AttachItem(5, m_posXEdit);
    AttachItem(6, m_posYEdit);
    AttachItem(IDC_HEIGHT_EDIT, m_heightEdit);
    AttachItem(9, m_rotXEdit);
    AttachItem(2, m_rotYEdit);
    AttachItem(1, m_rotZEdit);
    UpdateVisuals();
    return TRUE;
}

INT_PTR FlasherVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
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
