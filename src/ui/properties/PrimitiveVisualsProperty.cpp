// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PrimitiveVisualsProperty.h"
#include <WindowsX.h>

PrimitiveVisualsProperty::PrimitiveVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPRIMITIVE_VISUALS, pvsel)
{
    m_disableLightingEdit.SetDialog(this);
    m_disableLightFromBelowEdit.SetDialog(this);
    m_legacySidesEdit.SetDialog(this);
    m_edgeFactorUIEdit.SetDialog(this);
    m_depthBiasEdit.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_normalMapCombo.SetDialog(this);
    m_lightmapCombo.SetDialog(this);
    m_reflectionCombo.SetDialog(this);
    m_reflectionAmountEdit.SetDialog(this);
    m_refractionCombo.SetDialog(this);
    m_refractionThicknessEdit.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_opacityAmountEdit.SetDialog(this);
}

void PrimitiveVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive*)m_pvsel->ElementAt(i);

        if (dispid == IDC_ADDBLEND || dispid == -1)
        {
            PropertyDialog::SetCheckboxState(m_hAdditiveBlendCheck, prim->m_d.m_addBlend);
            // Disable/Enable all ignored properties when using unshaded additive blending
            ::EnableWindow(m_normalMapCombo, !prim->m_d.m_addBlend);
            ::EnableWindow(m_materialCombo, !prim->m_d.m_addBlend);
            ::EnableWindow(m_reflectionCombo, !prim->m_d.m_addBlend);
            ::EnableWindow(m_reflectionAmountEdit, !prim->m_d.m_addBlend);
            ::EnableWindow(m_refractionCombo, !prim->m_d.m_addBlend);
            ::EnableWindow(m_refractionThicknessEdit, !prim->m_d.m_addBlend);
            ::EnableWindow(m_hObjectSpaceCheck, !prim->m_d.m_addBlend);
            ::EnableWindow(m_hRenderBackfacingCheck, !prim->m_d.m_addBlend);
            ::EnableWindow(m_hDepthMaskWriteCheck, !prim->m_d.m_addBlend);
            ::EnableWindow(m_disableLightingEdit, !prim->m_d.m_addBlend);
            ::EnableWindow(m_disableLightFromBelowEdit, !prim->m_d.m_addBlend);
        }
        if (dispid == IDC_DISPLAY_TEXTURE_CHECKBOX || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hDisplayImageCheck, prim->m_d.m_displayTexture);
        if (dispid == IDC_DRAW_TEXTURES_SIDES_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hDrawTexturesInsideCheck, prim->m_d.m_drawTexturesInside);
        if (dispid == IDC_OBJECT_SPACE_NORMALMAP || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hObjectSpaceCheck, prim->m_d.m_objectSpaceNormalMap);
        if (dispid == IDC_PRIMITIVE_ENABLE_BACKFACES || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hRenderBackfacingCheck, prim->m_d.m_backfacesEnabled);
        if (dispid == IDC_PRIMITIVE_ENABLE_DEPTH_MASK || dispid == -1)
        {
            PropertyDialog::SetCheckboxState(m_hDepthMaskWriteCheck, prim->m_d.m_useDepthMask);
            // Render backface is only applied when depth mask is used (no culling otherwise)
            ::EnableWindow(m_hRenderBackfacingCheck, prim->m_d.m_useDepthMask);
            GetDlgItem(IDC_STATIC5).EnableWindow(prim->m_d.m_useDepthMask);
        }
        if (dispid == IDC_STATIC_RENDERING_CHECK || dispid == -1)
        {
            PropertyDialog::SetCheckboxState(m_hStaticRenderingCheck, prim->m_d.m_staticRendering);
            // Disable 'disable from light from below' for static part to avoid confusing the user
            m_disableLightFromBelowEdit.EnableWindow(!prim->m_d.m_staticRendering);
        }
        if (dispid == IDC_ALPHA_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_opacityAmountEdit, prim->m_d.m_alpha);
        if (dispid == IDC_DEPTH_BIAS || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_depthBiasEdit, prim->m_d.m_depthBias);
        if (dispid == IDC_BLEND_DISABLE_LIGHTING || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_disableLightingEdit, prim->m_d.m_disableLightingTop);
        if (dispid == IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, 1.f - prim->m_d.m_disableLightingBelow);
        if (dispid == IDC_PRIMITIVE_LEGACY_SIDES_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_legacySidesEdit, prim->m_d.m_Sides);
        if (dispid == IDC_EDGE_FACTOR_UI || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_edgeFactorUIEdit, prim->m_d.m_edgeFactorUI);
        if (dispid == DISPID_Image2 || dispid == -1)
            PropertyDialog::UpdateTextureComboBox(prim->GetPTable()->GetImageList(), m_normalMapCombo, prim->m_d.m_szNormalMap);
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_colorButton.SetColor(prim->m_d.m_color);
        if (dispid == DISPID_REFLECTION_PROBE || dispid == -1)
           UpdateRenderProbeComboBox(prim->GetPTable()->GetRenderProbeList(RenderProbe::PLANE_REFLECTION), m_reflectionCombo, prim->m_d.m_szReflectionProbe);
        if (dispid == IDC_REFLECTION_AMOUNT || dispid == -1)
           PropertyDialog::SetFloatTextbox(m_reflectionAmountEdit, prim->m_d.m_reflectionStrength);
        if (dispid == DISPID_REFRACTION_PROBE || dispid == -1)
           UpdateRenderProbeComboBox(prim->GetPTable()->GetRenderProbeList(RenderProbe::SCREEN_SPACE_TRANSPARENCY), m_refractionCombo, prim->m_d.m_szRefractionProbe);
        if (dispid == IDC_REFRACTION_THICKNESS || dispid == -1)
           PropertyDialog::SetFloatTextbox(m_refractionThicknessEdit, prim->m_d.m_refractionThickness);
        if (dispid == IDC_LIGHTMAP || dispid == -1)
           UpdateLightmapComboBox(prim->GetPTable(), m_lightmapCombo, prim->m_d.m_szLightmap);

        // Disable playfield settings that are taken from table settings to avoid confusing the user
        if (m_hReflectionEnabledCheck && (dispid == IDC_REFLECT_ENABLED_CHECK || dispid == -1))
           ::EnableWindow(m_hReflectionEnabledCheck, !prim->IsPlayfield());
        if (m_baseImageCombo && (dispid == DISPID_Image || dispid == -1))
           m_baseImageCombo->EnableWindow(!prim->IsPlayfield());
        if (m_baseMaterialCombo && (dispid == IDC_MATERIAL_COMBO || dispid == -1))
           m_baseMaterialCombo->EnableWindow(!prim->IsPlayfield());

        UpdateBaseVisuals(prim, &prim->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void PrimitiveVisualsProperty::UpdateLightmapComboBox(const PinTable *table, const CComboBox &combo, const string &selectName)
{
   std::vector<Light*> lights;
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

void PrimitiveVisualsProperty::UpdateRenderProbeComboBox(const vector<RenderProbe *> &contentList, const CComboBox &combo, const string &selectName)
{
    bool need_reset = combo.GetCount() != contentList.size() + 1; // Not the same number of items
    need_reset |= combo.FindStringExact(1, selectName.c_str()) == CB_ERR; // Selection is not part of combo
    if (!need_reset)
    {
        bool texelFound = false;
        for (const auto texel : contentList)
        {
            if (strncmp(texel->GetName().c_str(), selectName.c_str(), MAXTOKEN) == 0) //!! lstrcmpi?
                texelFound = true;
            need_reset |= combo.FindStringExact(1, texel->GetName().c_str()) == CB_ERR; // Combo does not contain an image from the image list
        }
        need_reset |= !texelFound; // Selection is not part of image list
    }
    if (need_reset)
    {
        combo.ResetContent();
        combo.AddString(_T("<None>"));
        for (size_t i = 0; i < contentList.size(); i++)
            combo.AddString(contentList[i]->GetName().c_str());
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName.c_str()));
}

void PrimitiveVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_ADDBLEND:
                CHECK_UPDATE_ITEM(prim->m_d.m_addBlend, PropertyDialog::GetCheckboxState(m_hAdditiveBlendCheck), prim);
                break;
            case IDC_DISPLAY_TEXTURE_CHECKBOX:
                CHECK_UPDATE_ITEM(prim->m_d.m_displayTexture, PropertyDialog::GetCheckboxState(m_hDisplayImageCheck), prim);
                break;
            case IDC_OBJECT_SPACE_NORMALMAP:
                CHECK_UPDATE_ITEM(prim->m_d.m_objectSpaceNormalMap, PropertyDialog::GetCheckboxState(m_hObjectSpaceCheck), prim);
                break;
            case IDC_PRIMITIVE_ENABLE_BACKFACES:
                CHECK_UPDATE_ITEM(prim->m_d.m_backfacesEnabled, PropertyDialog::GetCheckboxState(m_hRenderBackfacingCheck), prim);
                break;
            case IDC_PRIMITIVE_ENABLE_DEPTH_MASK:
                CHECK_UPDATE_ITEM(prim->m_d.m_useDepthMask, PropertyDialog::GetCheckboxState(m_hDepthMaskWriteCheck), prim);
                break;
            case IDC_STATIC_RENDERING_CHECK:
                CHECK_UPDATE_ITEM(prim->m_d.m_staticRendering, PropertyDialog::GetCheckboxState(m_hStaticRenderingCheck), prim);
                UpdateVisuals(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW);
                break;
            case IDC_DRAW_TEXTURES_SIDES_CHECK:
                CHECK_UPDATE_ITEM(prim->m_d.m_drawTexturesInside, PropertyDialog::GetCheckboxState(m_hDrawTexturesInsideCheck), prim);
                break;
            case IDC_LOAD_MESH_BUTTON:
                PropertyDialog::StartUndo(prim);
                prim->LoadMeshDialog();
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_EXPORT_MESH_BUTTON:
                PropertyDialog::StartUndo(prim);
                prim->ExportMeshDialog();
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_ALPHA_EDIT:
                CHECK_UPDATE_VALUE_SETTER(prim->SetAlpha, prim->GetAlpha, PropertyDialog::GetFloatTextbox, m_opacityAmountEdit, prim);
                break;
            case IDC_DEPTH_BIAS:
                CHECK_UPDATE_ITEM(prim->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit), prim);
                break;
            case IDC_BLEND_DISABLE_LIGHTING:
                CHECK_UPDATE_ITEM(prim->m_d.m_disableLightingTop, PropertyDialog::GetFloatTextbox(m_disableLightingEdit), prim);
                break;
            case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
                CHECK_UPDATE_ITEM(prim->m_d.m_disableLightingBelow, 1.f - PropertyDialog::GetFloatTextbox(m_disableLightFromBelowEdit), prim);
                break;
            case IDC_PRIMITIVE_LEGACY_SIDES_EDIT:
                CHECK_UPDATE_ITEM(prim->m_d.m_Sides, PropertyDialog::GetIntTextbox(m_legacySidesEdit), prim);
                break;
            case IDC_EDGE_FACTOR_UI:
                CHECK_UPDATE_ITEM(prim->m_d.m_edgeFactorUI, PropertyDialog::GetFloatTextbox(m_edgeFactorUIEdit), prim);
                break;
            case DISPID_Image2:
                CHECK_UPDATE_COMBO_TEXT_STRING(prim->m_d.m_szNormalMap, m_normalMapCombo, prim);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(prim->m_d.m_color);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    prim->m_d.m_color = m_colorDialog.GetColor();
                    m_colorButton.SetColor(prim->m_d.m_color);
                    memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
                }
                break;
            }
            case IDC_LIGHTMAP:
                CHECK_UPDATE_COMBO_TEXT_STRING(prim->m_d.m_szLightmap, m_lightmapCombo, prim);
                break;
            case DISPID_REFLECTION_PROBE:
                CHECK_UPDATE_COMBO_TEXT_STRING(prim->m_d.m_szReflectionProbe, m_reflectionCombo, prim);
                break;
            case IDC_REFLECTION_AMOUNT:
                CHECK_UPDATE_ITEM(prim->m_d.m_reflectionStrength, PropertyDialog::GetFloatTextbox(m_reflectionAmountEdit), prim);
                break;
            case DISPID_REFRACTION_PROBE:
                CHECK_UPDATE_COMBO_TEXT_STRING(prim->m_d.m_szRefractionProbe, m_refractionCombo, prim);
                break;
            case IDC_REFRACTION_THICKNESS:
                CHECK_UPDATE_ITEM(prim->m_d.m_refractionThickness, PropertyDialog::GetFloatTextbox(m_refractionThicknessEdit), prim);
                break;
            default:
                UpdateBaseProperties(prim, &prim->m_d, dispid);
                break;
        }
        prim->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

void PrimitiveVisualsProperty::AddToolTip(const char *const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = parentHwnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)controlHwnd;
    toolInfo.lpszText = (char *)text;
    SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

BOOL PrimitiveVisualsProperty::OnInitDialog()
{
    m_hDisplayImageCheck = ::GetDlgItem(GetHwnd(), IDC_DISPLAY_TEXTURE_CHECKBOX);
    m_hObjectSpaceCheck = ::GetDlgItem(GetHwnd(), IDC_OBJECT_SPACE_NORMALMAP);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hRenderBackfacingCheck = ::GetDlgItem(GetHwnd(), IDC_PRIMITIVE_ENABLE_BACKFACES);
    m_hDepthMaskWriteCheck = ::GetDlgItem(GetHwnd(), IDC_PRIMITIVE_ENABLE_DEPTH_MASK);
    m_hStaticRenderingCheck = ::GetDlgItem(GetHwnd(), IDC_STATIC_RENDERING_CHECK);
    m_hDrawTexturesInsideCheck = ::GetDlgItem(GetHwnd(), IDC_DRAW_TEXTURES_SIDES_CHECK);
    AttachItem(IDC_LOAD_MESH_BUTTON, m_importMeshButton);
    AttachItem(IDC_EXPORT_MESH_BUTTON, m_exportMeshButton);
    m_hAdditiveBlendCheck = ::GetDlgItem(GetHwnd(), IDC_ADDBLEND);
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;
    m_normalMapCombo.AttachItem(DISPID_Image2);
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_opacityAmountEdit.AttachItem(IDC_ALPHA_EDIT);
    m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
    m_disableLightingEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING);
    m_disableLightFromBelowEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW);
    m_legacySidesEdit.AttachItem(IDC_PRIMITIVE_LEGACY_SIDES_EDIT);
    m_edgeFactorUIEdit.AttachItem(IDC_EDGE_FACTOR_UI);
    m_lightmapCombo.AttachItem(IDC_LIGHTMAP);
    m_reflectionCombo.AttachItem(DISPID_REFLECTION_PROBE);
    m_reflectionAmountEdit.AttachItem(IDC_REFLECTION_AMOUNT);
    m_refractionCombo.AttachItem(DISPID_REFRACTION_PROBE);
    m_refractionThicknessEdit.AttachItem(IDC_REFRACTION_THICKNESS);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hDrawTexturesInsideCheck, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(m_legacySidesEdit, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(m_edgeFactorUIEdit, CResizer::topright, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hStaticRenderingCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hRenderBackfacingCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hDepthMaskWriteCheck, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(m_depthBiasEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hAdditiveBlendCheck, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC19), CResizer::topleft, 0);
    m_resizer.AddChild(m_lightmapCombo, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hDisplayImageCheck, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(m_normalMapCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hObjectSpaceCheck, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(m_disableLightingEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(m_disableLightFromBelowEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(m_colorButton, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(m_opacityAmountEdit, CResizer::topright, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_reflectionCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(m_reflectionAmountEdit, CResizer::topright, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_refractionCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC17), CResizer::topleft, 0);
    m_resizer.AddChild(m_refractionThicknessEdit, CResizer::topright, RD_STRETCH_WIDTH);

    const HWND hwndDlg = GetHwnd();
    const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_pvp->theInstance, NULL);
    if (toolTipHwnd)
    {
        SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
        AddToolTip("Render backfacing transparent faces (Should be enabled for transparent parts)", hwndDlg, toolTipHwnd, m_hRenderBackfacingCheck);
        AddToolTip("Hide parts behind this one using the depth mask (Should be disabled for transparent parts)", hwndDlg, toolTipHwnd, m_hDepthMaskWriteCheck);
    }
    return TRUE;
}

INT_PTR PrimitiveVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
