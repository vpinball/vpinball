#include "stdafx.h"
#include "Properties/PrimitiveVisualsProperty.h"
#include <WindowsX.h>

PrimitiveVisualsProperty::PrimitiveVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPRIMITIVE_VISUALS, pvsel)
{
}

void PrimitiveVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive*)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hDisplayImageCheck, prim->m_d.m_displayTexture);
        PropertyDialog::SetCheckboxState(m_hDrawTexturesInsideCheck, prim->m_d.m_drawTexturesInside);
        PropertyDialog::SetCheckboxState(m_hObjectSpaceCheck, prim->m_d.m_objectSpaceNormalMap);
        PropertyDialog::SetCheckboxState(m_hRenderBackfacingCheck, prim->m_d.m_backfacesEnabled);
        PropertyDialog::SetCheckboxState(m_hStaticRenderingCheck, prim->m_d.m_staticRendering);
        PropertyDialog::SetFloatTextbox(m_depthBiasEdit, prim->m_d.m_depthBias);
        PropertyDialog::SetFloatTextbox(m_disableLightingEdit, prim->m_d.m_disableLightingTop);
        PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, prim->m_d.m_disableLightingBelow);
        PropertyDialog::SetIntTextbox(m_legacySidesEdit, prim->m_d.m_Sides);
        PropertyDialog::SetFloatTextbox(m_editorEdit, prim->m_d.m_edgeFactorUI);
        PropertyDialog::UpdateTextureComboBox(prim->GetPTable()->GetImageList(), m_normalMapCombo, prim->m_d.m_szNormalMap);
        UpdateBaseVisuals(prim, &prim->m_d);
        //only show the first element on multi-select
        break;
    }
}

void PrimitiveVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPrimitive))
            continue;
        Primitive *const prim = (Primitive *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case IDC_DISPLAY_TEXTURE_CHECKBOX:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_displayTexture = PropertyDialog::GetCheckboxState(m_hDisplayImageCheck);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_OBJECT_SPACE_NORMALMAP:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_objectSpaceNormalMap = PropertyDialog::GetCheckboxState(m_hObjectSpaceCheck);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_PRIMITIVE_ENABLE_BACKFACES:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_backfacesEnabled = PropertyDialog::GetCheckboxState(m_hRenderBackfacingCheck);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_STATIC_RENDERING_CHECK:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_staticRendering = PropertyDialog::GetCheckboxState(m_hStaticRenderingCheck);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_DRAW_TEXTURES_SIDES_CHECK:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_drawTexturesInside = PropertyDialog::GetCheckboxState(m_hDrawTexturesInsideCheck);
                PropertyDialog::EndUndo(prim);
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
            case IDC_DEPTH_BIAS:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_depthBias = PropertyDialog::GetFloatTextbox(m_depthBiasEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_BLEND_DISABLE_LIGHTING:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_disableLightingTop = PropertyDialog::GetFloatTextbox(m_disableLightingEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_disableLightingBelow = PropertyDialog::GetFloatTextbox(m_disableLightFromBelowEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_PRIMITIVE_LEGACY_SIDES_EDIT:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_Sides = PropertyDialog::GetIntTextbox(m_legacySidesEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case IDC_EDGE_FACTOR_UI:
                PropertyDialog::StartUndo(prim);
                prim->m_d.m_edgeFactorUI = PropertyDialog::GetFloatTextbox(m_editorEdit);
                PropertyDialog::EndUndo(prim);
                break;
            case DISPID_Image2:
                PropertyDialog::StartUndo(prim);
                PropertyDialog::GetComboBoxText(m_normalMapCombo, prim->m_d.m_szNormalMap);
                PropertyDialog::EndUndo(prim);
                break;
            default:
                UpdateBaseProperties(prim, &prim->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
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
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    AttachItem(DISPID_Image2, m_normalMapCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    AttachItem(IDC_DEPTH_BIAS, m_depthBiasEdit);
    AttachItem(IDC_BLEND_DISABLE_LIGHTING, m_disableLightingEdit);
    AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW, m_disableLightFromBelowEdit);
    AttachItem(IDC_PRIMITIVE_LEGACY_SIDES_EDIT, m_legacySidesEdit);
    AttachItem(IDC_EDGE_FACTOR_UI, m_editorEdit);
    UpdateVisuals();
    return TRUE;
}
