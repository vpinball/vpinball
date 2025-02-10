// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/FlasherVisualsProperty.h"
#include <WindowsX.h>

FlasherVisualsProperty::FlasherVisualsProperty(const VectorProtected<ISelect> *pvsel)
   : BasePropertyDialog(IDD_PROPFLASHER_VISUALS, pvsel)
{
   m_modeCombo.SetDialog(this);
   m_imageAlignList.push_back("World"s);
   m_imageAlignList.push_back("Wrap"s);
   m_depthBiasEdit.SetDialog(this);

   m_styleCombo.SetDialog(this);
   m_linkEdit.SetDialog(this);

   m_glassImageCombo.SetDialog(this);
   m_glassRoughnessEdit.SetDialog(this);
   m_glassPadTopEdit.SetDialog(this);
   m_glassPadBottomEdit.SetDialog(this);
   m_glassPadLeftEdit.SetDialog(this);
   m_glassPadRightEdit.SetDialog(this);

   m_imageACombo.SetDialog(this);
   m_imageBCombo.SetDialog(this);
   m_texModeCombo.SetDialog(this);
   m_filterCombo.SetDialog(this);
   m_filterList.push_back("None"s);
   m_filterList.push_back("Additive"s);
   m_filterList.push_back("Overlay"s);
   m_filterList.push_back("Multiply"s);
   m_filterList.push_back("Screen"s);
   m_filterAmountEdit.SetDialog(this);

   m_opacityAmountEdit.SetDialog(this);
   m_lightmapCombo.SetDialog(this);
   m_modulateEdit.SetDialog(this);

   m_heightEdit.SetDialog(this);
   m_rotXEdit.SetDialog(this);
   m_rotYEdit.SetDialog(this);
   m_rotZEdit.SetDialog(this);
   m_posXEdit.SetDialog(this);
   m_posYEdit.SetDialog(this);
}

void FlasherVisualsProperty::UpdateVisuals(const int dispid /*=-1*/)
{
   for (int i = 0; i < m_pvsel->size(); i++)
   {
      if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlasher))
         continue;
      Flasher *const flash = (Flasher *)m_pvsel->ElementAt(i);
      FlasherData::RenderMode mode = clamp(flash->m_d.m_renderMode, FlasherData::FLASHER, FlasherData::ALPHASEG);

      if (dispid == IDC_STYLE_COMBO || dispid == -1)
      {
         switch (mode)
         {
         case FlasherData::FLASHER: m_modeCombo.SetCurSel(0); break;
         case FlasherData::DMD:
            m_modeCombo.SetCurSel(1);
            m_styleCombo.ResetContent();
            m_styleCombo.AddString("Legacy VPX");
            m_styleCombo.AddString("Neon Plasma");
            m_styleCombo.AddString("Red LED");
            m_styleCombo.AddString("Green LED");
            m_styleCombo.AddString("Yellow LED");
            m_styleCombo.AddString("Generic Plasma");
            m_styleCombo.AddString("Generic LED");
            UpdateVisuals(IDC_DMD);
            break;
         case FlasherData::DISPLAY:
            m_modeCombo.SetCurSel(2);
            m_styleCombo.ResetContent();
            m_styleCombo.AddString("Pixelated");
            m_styleCombo.AddString("Smoothed");
            //m_styleCombo.AddString("ScaleFX");
            //m_styleCombo.AddString("Horizontal CRT");
            //m_styleCombo.AddString("Vertical CRT");
            break;
         case FlasherData::ALPHASEG:
            m_modeCombo.SetCurSel(3);
            m_styleCombo.ResetContent();
            // TODO
            break;
         }

         //::ShowWindow(GetDlgItem(IDC_STATIC21), mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         GetDlgItem(IDC_STATIC21)
            .SetWindowText(mode == FlasherData::FLASHER ? "Images"
                  : mode == FlasherData::DMD            ? "DMD Style"
                  : mode == FlasherData::DISPLAY        ? "Display Style"
                                                        : "Alpha Seg. Style");

         int isDisplay = mode != FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE;
         ::ShowWindow(GetDlgItem(IDC_STATIC24), isDisplay);
         m_styleCombo.ShowWindow(isDisplay);
         ::ShowWindow(GetDlgItem(IDC_STATIC1), isDisplay);
         m_linkEdit.ShowWindow(isDisplay);

         int isDmdOrAlpha = ((mode == FlasherData::DMD) || (mode == FlasherData::ALPHASEG)) ? SW_SHOWNORMAL : SW_HIDE;
         ::ShowWindow(GetDlgItem(IDC_STATIC25), isDmdOrAlpha);
         m_glassImageCombo.ShowWindow(isDmdOrAlpha);
         ::ShowWindow(GetDlgItem(IDC_STATIC26), isDmdOrAlpha);
         m_glassRoughnessEdit.ShowWindow(isDmdOrAlpha);
         ::ShowWindow(GetDlgItem(IDC_STATIC27), isDmdOrAlpha);
         m_glassAmbientButton.ShowWindow(isDmdOrAlpha);
         ::ShowWindow(GetDlgItem(IDC_STATIC28), isDmdOrAlpha);
         m_glassPadTopEdit.ShowWindow(isDmdOrAlpha);
         m_glassPadBottomEdit.ShowWindow(isDmdOrAlpha);
         ::ShowWindow(GetDlgItem(IDC_STATIC29), isDmdOrAlpha);
         m_glassPadLeftEdit.ShowWindow(isDmdOrAlpha);
         m_glassPadRightEdit.ShowWindow(isDmdOrAlpha);

         int isFlasher = mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE;
         ::ShowWindow(GetDlgItem(IDC_STATIC4), isFlasher);
         m_texModeCombo.ShowWindow(isFlasher);
         ::ShowWindow(GetDlgItem(IDC_STATIC20), isFlasher);
         m_imageACombo.ShowWindow(isFlasher);
         ::ShowWindow(GetDlgItem(IDC_STATIC2), isFlasher);
         m_imageBCombo.ShowWindow(isFlasher);
         ::ShowWindow(GetDlgItem(IDC_STATIC3), isFlasher);
         m_filterCombo.ShowWindow(isFlasher);
         ::ShowWindow(GetDlgItem(IDC_STATIC5), isFlasher);
         m_filterAmountEdit.ShowWindow(isFlasher);
         ::ShowWindow(GetDlgItem(IDC_STATIC6), isFlasher);
         ::ShowWindow(m_hDisplayInEditorCheck, isFlasher);

         //::ShowWindow(GetDlgItem(IDC_STATIC22), mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         //::ShowWindow(GetDlgItem(IDC_STATIC7), mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         GetDlgItem(IDC_STATIC7).SetWindowText(mode == FlasherData::FLASHER ? "Opacity" : "Brightness");
         //m_opacityAmountEdit.EnableWindow(mode == FlasherData::FLASHER ? 1 : 0);
         //::ShowWindow(GetDlgItem(IDC_STATIC8), mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         //::ShowWindow(GetDlgItem(IDC_STATIC19), mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         m_lightmapCombo.EnableWindow(mode == FlasherData::FLASHER ? 1 : 0);
         //::ShowWindow(m_hAdditiveBlendCheck, mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         //::ShowWindow(GetDlgItem(IDC_STATIC11), mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
         //m_modulateEdit.ShowWindow(mode == FlasherData::FLASHER ? SW_SHOWNORMAL : SW_HIDE);
      }
      if (dispid == IDC_VISIBLE_CHECK || dispid == -1)
         PropertyDialog::SetCheckboxState(m_hVisibleCheck, flash->m_d.m_isVisible);
      if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
         m_colorButton.SetColor(flash->m_d.m_color);
      if (dispid == IDC_DEPTH_BIAS || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_depthBiasEdit, flash->m_d.m_depthBias);

      if (dispid == IDC_DMD || dispid == -1)
         m_styleCombo.SetCurSel(clamp(flash->m_d.m_renderStyle, 0, flash->m_d.m_renderMode == FlasherData::DMD ? 6 : flash->m_d.m_renderMode == FlasherData::DISPLAY ? 1 : 0));
      if (dispid == IDC_IMAGE_LINK_EDIT || dispid == -1)
         m_linkEdit.SetWindowText(flash->m_d.m_imageSrcLink.c_str());

      if ((mode != FlasherData::FLASHER) && (dispid == IDC_GLASS_IMAGE || dispid == -1))
         PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_glassImageCombo, flash->m_d.m_szImageA);
      if (dispid == IDC_GLASS_DOT_LIGHT || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_glassRoughnessEdit, flash->m_d.m_glassRoughness);
      if (dispid == IDC_GLASS_AMBIENT || dispid == -1)
         m_glassAmbientButton.SetColor(flash->m_d.m_glassAmbient);
      if (dispid == IDC_GLASS_PAD_TOP || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_glassPadTopEdit, flash->m_d.m_glassPadTop);
      if (dispid == IDC_GLASS_PAD_BOTTOM || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_glassPadBottomEdit, flash->m_d.m_glassPadBottom);
      if (dispid == IDC_GLASS_PAD_LEFT || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_glassPadLeftEdit, flash->m_d.m_glassPadLeft);
      if (dispid == IDC_GLASS_PAD_RIGHT || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_glassPadRightEdit, flash->m_d.m_glassPadRight);

      if (dispid == IDC_FLASHER_MODE_COMBO || dispid == -1)
         PropertyDialog::UpdateComboBox(m_imageAlignList, m_texModeCombo, m_imageAlignList[(int)flash->m_d.m_imagealignment]);
      if ((mode == FlasherData::FLASHER) && (dispid == DISPID_Image || dispid == -1))
         PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_imageACombo, flash->m_d.m_szImageA);
      if (dispid == DISPID_Image2 || dispid == -1)
      {
         PropertyDialog::UpdateTextureComboBox(flash->GetPTable()->GetImageList(), m_imageBCombo, flash->m_d.m_szImageB);
         BOOL hasB = flash->GetPTable()->GetImage(flash->m_d.m_szImageB) ? 1 : 0;
         m_filterCombo.EnableWindow(hasB);
         m_filterAmountEdit.EnableWindow(hasB);
      }
      if (dispid == IDC_EFFECT_COMBO || dispid == -1)
         PropertyDialog::UpdateComboBox(m_filterList, m_filterCombo, m_filterList[flash->m_d.m_filter]);
      if (dispid == IDC_FILTERAMOUNT_EDIT || dispid == -1)
         PropertyDialog::SetIntTextbox(m_filterAmountEdit, flash->m_d.m_filterAmount);
      if (dispid == IDC_DISPLAY_IMAGE_CHECK || dispid == -1)
         PropertyDialog::SetCheckboxState(m_hDisplayInEditorCheck, flash->m_d.m_displayTexture);

      if (dispid == IDC_ALPHA_EDIT || dispid == -1)
         PropertyDialog::SetIntTextbox(m_opacityAmountEdit, flash->m_d.m_alpha);
      if (dispid == IDC_LIGHTMAP || dispid == -1)
         UpdateLightmapComboBox(flash->GetPTable(), m_lightmapCombo, flash->m_d.m_szLightmap);
      if (dispid == IDC_ADDBLEND || dispid == -1)
         PropertyDialog::SetCheckboxState(m_hAdditiveBlendCheck, flash->m_d.m_addBlend);
      if (dispid == IDC_MODULATE_VS_ADD || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_modulateEdit, flash->m_d.m_modulate_vs_add);

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
   FlasherData::RenderMode mode = static_cast<FlasherData::RenderMode>(clamp(m_modeCombo.GetCurSel(), FlasherData::FLASHER, FlasherData::ALPHASEG));
   bool isDmd = mode == FlasherData::DMD;
   bool isFlasher = mode == FlasherData::FLASHER;
   for (int i = 0; i < m_pvsel->size(); i++)
   {
      if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemFlasher))
         continue;
      Flasher *const flash = (Flasher *)m_pvsel->ElementAt(i);
      switch (dispid)
      {
      case IDC_STYLE_COMBO:
         PropertyDialog::StartUndo(flash);
         flash->m_d.m_renderMode = mode;
         PropertyDialog::EndUndo(flash);
         UpdateVisuals(-1);
         break;
      case IDC_DMD:
         PropertyDialog::StartUndo(flash);
         flash->m_d.m_renderStyle = clamp(m_styleCombo.GetCurSel(), 0, flash->m_d.m_renderMode == FlasherData::DMD ? 6 : flash->m_d.m_renderMode == FlasherData::DISPLAY ? 1 : 0);
         PropertyDialog::EndUndo(flash);
         break;
      case IDC_IMAGE_LINK_EDIT:
         PropertyDialog::StartUndo(flash);
         flash->m_d.m_imageSrcLink = m_linkEdit.GetWindowText();
         PropertyDialog::EndUndo(flash);
         break;
      case IDC_GLASS_IMAGE: if (isDmd) CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szImageA, m_glassImageCombo, flash); break;
      case IDC_GLASS_DOT_LIGHT: CHECK_UPDATE_ITEM(flash->m_d.m_glassRoughness, PropertyDialog::GetFloatTextbox(m_glassRoughnessEdit), flash); break;
      case IDC_GLASS_AMBIENT:
      {
         CComObject<PinTable> *const ptable = g_pvp->GetActiveTable();
         if (ptable == nullptr)
            break;
         CHOOSECOLOR cc = m_colorDialog.GetParameters();
         cc.Flags = CC_FULLOPEN | CC_RGBINIT;
         m_colorDialog.SetParameters(cc);
         m_colorDialog.SetColor(flash->m_d.m_glassAmbient);
         m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
         if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
         {
            flash->m_d.m_glassAmbient = m_colorDialog.GetColor();
            m_glassAmbientButton.SetColor(flash->m_d.m_glassAmbient);
            memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
         }
         break;
      }
      case IDC_GLASS_PAD_TOP: CHECK_UPDATE_ITEM(flash->m_d.m_glassPadTop, PropertyDialog::GetFloatTextbox(m_glassPadTopEdit), flash); break;
      case IDC_GLASS_PAD_BOTTOM: CHECK_UPDATE_ITEM(flash->m_d.m_glassPadBottom, PropertyDialog::GetFloatTextbox(m_glassPadBottomEdit), flash); break;
      case IDC_GLASS_PAD_LEFT: CHECK_UPDATE_ITEM(flash->m_d.m_glassPadLeft, PropertyDialog::GetFloatTextbox(m_glassPadLeftEdit), flash); break;
      case IDC_GLASS_PAD_RIGHT: CHECK_UPDATE_ITEM(flash->m_d.m_glassPadRight, PropertyDialog::GetFloatTextbox(m_glassPadRightEdit), flash); break;
      case IDC_VISIBLE_CHECK: CHECK_UPDATE_ITEM(flash->m_d.m_isVisible, PropertyDialog::GetCheckboxState(m_hVisibleCheck), flash); break;
      case IDC_DISPLAY_IMAGE_CHECK: CHECK_UPDATE_ITEM(flash->m_d.m_displayTexture, PropertyDialog::GetCheckboxState(m_hDisplayInEditorCheck), flash); break;
      case IDC_ADDBLEND: CHECK_UPDATE_ITEM(flash->m_d.m_addBlend, PropertyDialog::GetCheckboxState(m_hAdditiveBlendCheck), flash); break;
      case DISPID_Image: if (isFlasher) CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szImageA, m_imageACombo, flash); break;
      case DISPID_Image2: CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szImageB, m_imageBCombo, flash); break;
      case IDC_FLASHER_MODE_COMBO: CHECK_UPDATE_ITEM(flash->m_d.m_imagealignment, (RampImageAlignment)(PropertyDialog::GetComboBoxIndex(m_texModeCombo, m_imageAlignList)), flash); break;
      case IDC_EFFECT_COMBO: CHECK_UPDATE_ITEM(flash->m_d.m_filter, (Filters)PropertyDialog::GetComboBoxIndex(m_filterCombo, m_filterList), flash); break;
      case IDC_DEPTH_BIAS: CHECK_UPDATE_ITEM(flash->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit), flash); break;
      case IDC_FILTERAMOUNT_EDIT: CHECK_UPDATE_VALUE_SETTER(flash->SetFilterAmount, flash->GetFilterAmount, PropertyDialog::GetIntTextbox, m_filterAmountEdit, flash); break;
      case IDC_ALPHA_EDIT: CHECK_UPDATE_VALUE_SETTER(flash->SetAlpha, flash->GetAlpha, PropertyDialog::GetIntTextbox, m_opacityAmountEdit, flash); break;
      case IDC_MODULATE_VS_ADD: CHECK_UPDATE_ITEM(flash->m_d.m_modulate_vs_add, PropertyDialog::GetFloatTextbox(m_modulateEdit), flash); break;
      case 5:
      {
         const float oldX = flash->m_d.m_vCenter.x;
         const float newX = PropertyDialog::GetFloatTextbox(m_posXEdit);
         if (oldX != newX)
         {
            const float dx = newX - oldX;
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
            const float dy = newY - oldY;
            PropertyDialog::StartUndo(flash);
            flash->MoveOffset(0.0f, dy);
            PropertyDialog::EndUndo(flash);
         }
         break;
      }
      case IDC_HEIGHT_EDIT: CHECK_UPDATE_ITEM(flash->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), flash); break;
      case 9: CHECK_UPDATE_ITEM(flash->m_d.m_rotX, PropertyDialog::GetFloatTextbox(m_rotXEdit), flash); break;
      case 2: CHECK_UPDATE_ITEM(flash->m_d.m_rotY, PropertyDialog::GetFloatTextbox(m_rotYEdit), flash); break;
      case 1: CHECK_UPDATE_ITEM(flash->m_d.m_rotZ, PropertyDialog::GetFloatTextbox(m_rotZEdit), flash); break;
      case IDC_COLOR_BUTTON1:
      {
         CComObject<PinTable> *const ptable = g_pvp->GetActiveTable();
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
      case IDC_LIGHTMAP: CHECK_UPDATE_COMBO_TEXT_STRING(flash->m_d.m_szLightmap, m_lightmapCombo, flash); break;
      default: break;
      }
      flash->UpdateStatusBarInfo();
   }
   UpdateVisuals(dispid);
}

BOOL FlasherVisualsProperty::OnInitDialog()
{
   m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
   m_modeCombo.AttachItem(IDC_STYLE_COMBO);
   m_modeCombo.AddString("Flasher");
   m_modeCombo.AddString("DMD");
   m_modeCombo.AddString("Display");
   m_modeCombo.AddString("Alpha.Seg.");
   AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
   m_texModeCombo.AttachItem(IDC_FLASHER_MODE_COMBO);
   m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);

   m_styleCombo.AttachItem(IDC_DMD);
   m_linkEdit.AttachItem(IDC_IMAGE_LINK_EDIT);

   m_glassImageCombo.AttachItem(IDC_GLASS_IMAGE);
   m_glassRoughnessEdit.AttachItem(IDC_GLASS_DOT_LIGHT);
   AttachItem(IDC_GLASS_AMBIENT, m_glassAmbientButton);
   m_glassPadTopEdit.AttachItem(IDC_GLASS_PAD_TOP);
   m_glassPadBottomEdit.AttachItem(IDC_GLASS_PAD_BOTTOM);
   m_glassPadLeftEdit.AttachItem(IDC_GLASS_PAD_LEFT);
   m_glassPadRightEdit.AttachItem(IDC_GLASS_PAD_RIGHT);

   m_imageACombo.AttachItem(DISPID_Image);
   m_imageBCombo.AttachItem(DISPID_Image2);
   m_filterCombo.AttachItem(IDC_EFFECT_COMBO);
   m_filterAmountEdit.AttachItem(IDC_FILTERAMOUNT_EDIT);
   m_hDisplayInEditorCheck = ::GetDlgItem(GetHwnd(), IDC_DISPLAY_IMAGE_CHECK);

   m_opacityAmountEdit.AttachItem(IDC_ALPHA_EDIT);
   m_modulateEdit.AttachItem(IDC_MODULATE_VS_ADD);
   m_lightmapCombo.AttachItem(IDC_LIGHTMAP);
   m_hAdditiveBlendCheck = ::GetDlgItem(GetHwnd(), IDC_ADDBLEND);

   m_posXEdit.AttachItem(5);
   m_posYEdit.AttachItem(6);
   m_heightEdit.AttachItem(IDC_HEIGHT_EDIT);
   m_rotXEdit.AttachItem(9);
   m_rotYEdit.AttachItem(2);
   m_rotZEdit.AttachItem(1);

   UpdateVisuals();

   m_resizer.Initialize(*this, CRect(0, 0, 0, 0));

   m_resizer.AddChild(m_modeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_colorButton, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_depthBiasEdit, CResizer::topright, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_texModeCombo, CResizer::topleft, RD_STRETCH_WIDTH);

   m_resizer.AddChild(m_styleCombo, CResizer::topleft, RD_STRETCH_WIDTH); // Display Group
   m_resizer.AddChild(m_linkEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_glassImageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_glassRoughnessEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_glassAmbientButton, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_glassPadBottomEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_glassPadRightEdit, CResizer::topleft, RD_STRETCH_WIDTH);

   m_resizer.AddChild(GetDlgItem(IDC_STATIC21), CResizer::topleft, RD_STRETCH_WIDTH); // Flasher Group
   m_resizer.AddChild(m_imageACombo, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_imageBCombo, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_filterCombo, CResizer::topleft, 0);
   m_resizer.AddChild(m_filterAmountEdit, CResizer::topright, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_hDisplayInEditorCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topright, 0);

   m_resizer.AddChild(GetDlgItem(IDC_STATIC22), CResizer::topleft, RD_STRETCH_WIDTH); // Transparency Group
   m_resizer.AddChild(m_opacityAmountEdit, CResizer::topright, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topright, 0);
   m_resizer.AddChild(m_lightmapCombo, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_hAdditiveBlendCheck, CResizer::topleft, 0);
   m_resizer.AddChild(m_modulateEdit, CResizer::topleft, RD_STRETCH_WIDTH);

   m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, RD_STRETCH_WIDTH); // Position Group
   m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_heightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_rotXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_rotYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_rotZEdit, CResizer::topleft, RD_STRETCH_WIDTH);

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
         m_colorButton.DrawItem(lpDrawItemStruct);
      else if (nID == IDC_GLASS_AMBIENT)
         m_glassAmbientButton.DrawItem(lpDrawItemStruct);
      return TRUE;
   }
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}
