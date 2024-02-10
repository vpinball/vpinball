#include "stdafx.h"
#include "ui/dialogs/Properties/FlasherVisualsProperty.h"
#include <WindowsX.h>

FlasherVisualsProperty::FlasherVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPFLASHER_VISUALS, pvsel)
{
    m_filterList.push_back("None"s);
    m_filterList.push_back("Additive"s);
    m_filterList.push_back("Overlay"s);
    m_filterList.push_back("Multiply"s);
    m_filterList.push_back("Screen"s);

    m_imageAlignList.push_back("World"s);
    m_imageAlignList.push_back("Wrap"s);

    m_opacityAmountEdit.SetDialog(this);
    m_filterAmountEdit.SetDialog(this);
    m_intensityEdit.SetDialog(this);
    m_fadeSpeedUpEdit.SetDialog(this);
    m_fadeSpeedDownEdit.SetDialog(this);
    m_heightEdit.SetDialog(this);
    m_rotXEdit.SetDialog(this);
    m_rotYEdit.SetDialog(this);
    m_rotZEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_depthBiasEdit.SetDialog(this);
    m_modulateEdit.SetDialog(this);
    m_imageACombo.SetDialog(this);
    m_imageBCombo.SetDialog(this);
    m_modeCombo.SetDialog(this);
    m_filterCombo.SetDialog(this);

    m_lightmapCombo.SetDialog(this);
}

void FlasherVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlasher))
            continue;
        Flasher * const flash = (Flasher *)m_pvsel->ElementAt(i);

        if (dispid == IDC_DMD || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hUseDMDCheck, flash->m_d.m_isDMD);
        if (dispid == IDC_DISPLAY_IMAGE_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hDisplayInEditorCheck, flash->m_d.m_displayTexture);
        if (dispid == IDC_ADDBLEND || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hAdditiveBlendCheck, flash->m_d.m_addBlend);
        if (dispid == IDC_VISIBLE_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hVisibleCheck, flash->m_d.m_isVisible);
        if (dispid == 5 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, flash->m_d.m_vCenter.x);
        if (dispid == 6 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, flash->m_d.m_vCenter.y);
        if (dispid == IDC_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_heightEdit, flash->m_d.m_height);
        if (dispid == 9 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotXEdit, flash->m_d.m_rotX);
        if (dispid == 2 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotYEdit, flash->m_d.m_rotY);
        if (dispid == 1 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotZEdit, flash->m_d.m_rotZ);
        if (dispid == IDC_FILTERAMOUNT_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_filterAmountEdit, flash->m_d.m_filterAmount);
        if (dispid == IDC_ALPHA_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_opacityAmountEdit, flash->m_d.m_alpha);
        if (dispid == IDC_DEPTH_BIAS || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_depthBiasEdit, flash->m_d.m_depthBias);
        if (dispid == IDC_MODULATE_VS_ADD || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_modulateEdit, flash->m_d.m_modulate_vs_add);
        if (dispid == DISPID_Image || dispid == -1)
            PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_imageACombo, flash->m_d.m_szImageA);
        if (dispid == DISPID_Image2 || dispid == -1)
            PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_imageBCombo, flash->m_d.m_szImageB);
        if (dispid == IDC_FLASHER_MODE_COMBO || dispid == -1)
            PropertyDialog::UpdateComboBox(m_imageAlignList, m_modeCombo, m_imageAlignList[(int)flash->m_d.m_imagealignment]);
        if (dispid == IDC_EFFECT_COMBO || dispid == -1)
            PropertyDialog::UpdateComboBox(m_filterList, m_filterCombo, m_filterList[flash->m_d.m_filter]);
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_colorButton.SetColor(flash->m_d.m_color);
        if (dispid == IDC_LIGHTMAP || dispid == -1)
            UpdateLightmapComboBox(flash->GetPTable(), m_lightmapCombo, flash->m_d.m_szLightmap);
        //only show the first element on multi-select
        break;
    }
}

void FlasherVisualsProperty::UpdateLightmapComboBox(const PinTable *table, const CComboBox &combo, const string &selectName)
{
    std::vector<Light *> lights;
    for (size_t i = 0; i < table->m_vedit.size(); i++)
    {
        IEditable *const pe = table->m_vedit[i];
        if (pe->GetItemType() == ItemTypeEnum::eItemLight)
            lights.push_back((Light *)pe);
    }

    bool need_reset = combo.GetCount() != lights.size() + 1; // Not the same number of items
    need_reset |= combo.FindStringExact(1, selectName.c_str()) == CB_ERR; // Selection is not part of combo
    if (!need_reset)
    {
        bool texelFound = false;
        for (const auto texel : lights)
        {
            if (strncmp(texel->GetName(), selectName.c_str(), MAXTOKEN) == 0) //!! lstrcmpi?
               texelFound = true;
            need_reset |= combo.FindStringExact(1, texel->GetName()) == CB_ERR; // Combo does not contain an image from the image list
        }
        need_reset |= !texelFound; // Selection is not part of image list
    }
    if (need_reset)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < lights.size(); i++)
            combo.AddString(lights[i]->GetName());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void FlasherVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlasher))
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
                CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szImageA, m_imageACombo, flash);
                break;
            case DISPID_Image2:
                CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szImageB, m_imageBCombo, flash);
                break;
            case IDC_FLASHER_MODE_COMBO:
                CHECK_UPDATE_ITEM(flash->m_d.m_imagealignment, (RampImageAlignment)(PropertyDialog::GetComboBoxIndex(m_modeCombo, m_imageAlignList)), flash);
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
                CHECK_UPDATE_VALUE_SETTER(flash->SetAlpha, flash->GetAlpha, PropertyDialog::GetIntTextbox, m_opacityAmountEdit, flash);
                break;
            case IDC_MODULATE_VS_ADD:
                CHECK_UPDATE_ITEM(flash->m_d.m_modulate_vs_add, PropertyDialog::GetFloatTextbox(m_modulateEdit), flash);
                break;
            case 5:
            {
               const float oldX = flash->m_d.m_vCenter.x;
               const float newX = PropertyDialog::GetFloatTextbox(m_posXEdit);
               if (oldX != newX)
               {
                  const float dx = newX-oldX;
                  PropertyDialog::StartUndo(flash);
                  flash->MoveOffset(dx, 0.0f);
                  PropertyDialog::EndUndo(flash);
               }
               break;
            }
            case 6:
            {
               const float oldY = flash->m_d.m_vCenter.y;
               const float newY = PropertyDialog::GetFloatTextbox(m_posYEdit);
               if (oldY != newY)
               {
                  const float dy = newY-oldY;
                  PropertyDialog::StartUndo(flash);
                  flash->MoveOffset(0.0f, dy);
                  PropertyDialog::EndUndo(flash);
               }
               break;
            }
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
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(flash->m_d.m_color);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    flash->m_d.m_color = m_colorDialog.GetColor();
                    m_colorButton.SetColor(flash->m_d.m_color);
                    memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
                }
                break;
            }
            case IDC_LIGHTMAP:
                CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szLightmap, m_lightmapCombo, flash);
                break;
            default:
                break;
        }
        flash->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL FlasherVisualsProperty::OnInitDialog()
{
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_imageACombo.AttachItem(DISPID_Image);
    m_hDisplayInEditorCheck = ::GetDlgItem(GetHwnd(), IDC_DISPLAY_IMAGE_CHECK);
    m_imageBCombo.AttachItem(DISPID_Image2);
    m_modeCombo.AttachItem(IDC_FLASHER_MODE_COMBO);
    m_filterCombo.AttachItem(IDC_EFFECT_COMBO);
    m_hAdditiveBlendCheck = ::GetDlgItem(GetHwnd(), IDC_ADDBLEND);
    m_hUseDMDCheck = ::GetDlgItem(GetHwnd(), IDC_DMD);
    m_filterAmountEdit.AttachItem(IDC_FILTERAMOUNT_EDIT);
    m_opacityAmountEdit.AttachItem(IDC_ALPHA_EDIT);
    m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
    m_modulateEdit.AttachItem(IDC_MODULATE_VS_ADD);
    m_posXEdit.AttachItem(5);
    m_posYEdit.AttachItem(6);
    m_heightEdit.AttachItem(IDC_HEIGHT_EDIT);
    m_rotXEdit.AttachItem(9);
    m_rotYEdit.AttachItem(2);
    m_rotZEdit.AttachItem(1);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
    m_lightmapCombo.AttachItem(IDC_LIGHTMAP);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topright, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC17), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC18), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC19), CResizer::topleft, 0);
    m_resizer.AddChild(m_lightmapCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imageACombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hDisplayInEditorCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imageBCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_modeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_filterCombo, CResizer::topleft, 0);
    m_resizer.AddChild(m_hAdditiveBlendCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hUseDMDCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_filterAmountEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_opacityAmountEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_depthBiasEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_modulateEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_heightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotZEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_colorButton, CResizer::topleft, 0);

    return TRUE;
}

INT_PTR FlasherVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
