#include "stdafx.h"
#include "Properties/PrimitiveVisualsProperty.h"
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
            PropertyDialog::SetCheckboxState(m_hAdditiveBlendCheck, prim->m_d.m_addBlend);
        if (dispid == IDC_DISPLAY_TEXTURE_CHECKBOX || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hDisplayImageCheck, prim->m_d.m_displayTexture);
        if (dispid == IDC_DRAW_TEXTURES_SIDES_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hDrawTexturesInsideCheck, prim->m_d.m_drawTexturesInside);
        if (dispid == IDC_OBJECT_SPACE_NORMALMAP || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hObjectSpaceCheck, prim->m_d.m_objectSpaceNormalMap);
        if (dispid == IDC_PRIMITIVE_ENABLE_BACKFACES || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hRenderBackfacingCheck, prim->m_d.m_backfacesEnabled);
        if (dispid == IDC_STATIC_RENDERING_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hStaticRenderingCheck, prim->m_d.m_staticRendering);
        if (dispid == IDC_ALPHA_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_opacityAmountEdit, prim->m_d.m_alpha);
        if (dispid == IDC_DEPTH_BIAS || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_depthBiasEdit, prim->m_d.m_depthBias);
        if (dispid == IDC_BLEND_DISABLE_LIGHTING || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_disableLightingEdit, prim->m_d.m_disableLightingTop);
        if (dispid == IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, prim->m_d.m_disableLightingBelow);
        if (dispid == IDC_PRIMITIVE_LEGACY_SIDES_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_legacySidesEdit, prim->m_d.m_Sides);
        if (dispid == IDC_EDGE_FACTOR_UI || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_edgeFactorUIEdit, prim->m_d.m_edgeFactorUI);
        if (dispid == DISPID_Image2 || dispid == -1)
            PropertyDialog::UpdateTextureComboBox(prim->GetPTable()->GetImageList(), m_normalMapCombo, prim->m_d.m_szNormalMap);
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_colorButton.SetColor(prim->m_d.m_color);

        UpdateBaseVisuals(prim, &prim->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
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
            case IDC_STATIC_RENDERING_CHECK:
                CHECK_UPDATE_ITEM(prim->m_d.m_staticRendering, PropertyDialog::GetCheckboxState(m_hStaticRenderingCheck), prim);
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
                CHECK_UPDATE_ITEM(prim->m_d.m_disableLightingBelow, PropertyDialog::GetFloatTextbox(m_disableLightFromBelowEdit), prim);
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
            default:
                UpdateBaseProperties(prim, &prim->m_d, dispid);
                break;
        }
        prim->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL PrimitiveVisualsProperty::OnInitDialog()
{
    m_hDisplayImageCheck = ::GetDlgItem(GetHwnd(), IDC_DISPLAY_TEXTURE_CHECKBOX);
    m_hObjectSpaceCheck = ::GetDlgItem(GetHwnd(), IDC_OBJECT_SPACE_NORMALMAP);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hRenderBackfacingCheck = ::GetDlgItem(GetHwnd(), IDC_PRIMITIVE_ENABLE_BACKFACES);
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
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), topleft, 0);
    m_resizer.AddChild(m_hDisplayImageCheck, topleft, 0);
    m_resizer.AddChild(m_hObjectSpaceCheck, topleft, 0);
    m_resizer.AddChild(m_hVisibleCheck, topleft, 0);
    m_resizer.AddChild(m_hReflectionEnabledCheck, topleft, 0);
    m_resizer.AddChild(m_hRenderBackfacingCheck, topleft, 0);
    m_resizer.AddChild(m_hStaticRenderingCheck, topleft, 0);
    m_resizer.AddChild(m_hDrawTexturesInsideCheck, topleft, 0);
    m_resizer.AddChild(m_imageCombo, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_normalMapCombo, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_materialCombo, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hAdditiveBlendCheck, topleft, 0);
    m_resizer.AddChild(m_opacityAmountEdit, topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_depthBiasEdit, topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_disableLightingEdit, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_disableLightFromBelowEdit, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_legacySidesEdit, topleft, 0);
    m_resizer.AddChild(m_edgeFactorUIEdit, topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_colorButton, topleft, 0);

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
