// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/WallVisualsProperty.h"
#include <WindowsX.h>

WallVisualsProperty::WallVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPWALL_VISUALS, pvsel)
{
    m_disableLightingEdit.SetDialog(this);
    m_disableLightFromBelowEdit.SetDialog(this);
    m_topHeightEdit.SetDialog(this);
    m_bottomHeightEdit.SetDialog(this);
    m_topImageCombo.SetDialog(this);
    m_sideImageCombo.SetDialog(this);
    m_topMaterialCombo.SetDialog(this);
    m_sideMaterialCombo.SetDialog(this);
    m_slingshotMaterialCombo.SetDialog(this);
}

void WallVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    //only show the first element on multi-select
    Surface* const wall = (Surface*)m_pvsel->ElementAt(0);
    if (wall == nullptr)
        return;

    if (dispid == DISPID_Image2 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(wall->GetPTable()->GetImageList(), m_sideImageCombo, wall->m_d.m_szSideImage);
    if (dispid == IDC_MATERIAL_COMBO || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_topMaterialCombo, wall->m_d.m_szTopMaterial);
    if (dispid == IDC_MATERIAL_COMBO2 || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_sideMaterialCombo, wall->m_d.m_szSideMaterial);
    if (dispid == IDC_MATERIAL_COMBO3 || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(wall->GetPTable()->GetMaterialList(), m_slingshotMaterialCombo, wall->m_d.m_szSlingShotMaterial);
    if (dispid == 16 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 16), wall->m_d.m_topBottomVisible);
    if (dispid == 13 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 13), wall->m_d.m_displayTexture);
    if (dispid == 109 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 109), wall->m_d.m_sideVisible);
    if (dispid == 112 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 112), wall->m_d.m_slingshotAnimation);
    if (dispid == 113 || dispid == -1)
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 113), wall->m_d.m_flipbook);
    if (dispid == IDC_BLEND_DISABLE_LIGHTING || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_disableLightingEdit, wall->m_d.m_disableLightingTop);
    if (dispid == IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_disableLightFromBelowEdit, 1.f - wall->m_d.m_disableLightingBelow);
    if (dispid == 9 || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_topHeightEdit, wall->m_d.m_heighttop);
    if (dispid == 8 || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_bottomHeightEdit, wall->m_d.m_heightbottom);

    UpdateBaseVisuals(wall, &wall->m_d, dispid);
}

void WallVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemSurface))
            continue;
        Surface * const wall = (Surface*)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 9:
                CHECK_UPDATE_ITEM(wall->m_d.m_heighttop, PropertyDialog::GetFloatTextbox(m_topHeightEdit), wall);
                break;
            case 8:
                CHECK_UPDATE_ITEM(wall->m_d.m_heightbottom, PropertyDialog::GetFloatTextbox(m_bottomHeightEdit), wall);
                break;
            case DISPID_Image2:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szSideImage, m_sideImageCombo, wall);
                break;
            case IDC_MATERIAL_COMBO:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szTopMaterial, m_topMaterialCombo, wall);
                break;
            case IDC_MATERIAL_COMBO2:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szSideMaterial, m_sideMaterialCombo, wall);
                break;
            case IDC_MATERIAL_COMBO3:
                CHECK_UPDATE_COMBO_TEXT_STRING(wall->m_d.m_szSlingShotMaterial, m_slingshotMaterialCombo, wall);
                break;
            case IDC_BLEND_DISABLE_LIGHTING:
                CHECK_UPDATE_ITEM(wall->m_d.m_disableLightingTop, PropertyDialog::GetFloatTextbox(m_disableLightingEdit), wall);
                break;
            case IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW:
                CHECK_UPDATE_ITEM(wall->m_d.m_disableLightingBelow, 1.f - PropertyDialog::GetFloatTextbox(m_disableLightFromBelowEdit), wall);
                break;
            case 16:
                CHECK_UPDATE_ITEM(wall->m_d.m_topBottomVisible, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 13:
                CHECK_UPDATE_ITEM(wall->m_d.m_displayTexture, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 109:
                CHECK_UPDATE_ITEM(wall->m_d.m_sideVisible, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 112:
                CHECK_UPDATE_ITEM(wall->m_d.m_slingshotAnimation, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            case 113:
                CHECK_UPDATE_ITEM(wall->m_d.m_flipbook, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), wall);
                break;
            default:
                UpdateBaseProperties(wall, &wall->m_d, dispid);
                break;
        }
        wall->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL WallVisualsProperty::OnInitDialog()
{
    m_topImageCombo.AttachItem(DISPID_Image);
    m_sideImageCombo.AttachItem(DISPID_Image2);
    m_topMaterialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_sideMaterialCombo.AttachItem(IDC_MATERIAL_COMBO2);
    m_slingshotMaterialCombo.AttachItem(IDC_MATERIAL_COMBO3);
    m_disableLightingEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING);
    m_disableLightFromBelowEdit.AttachItem(IDC_BLEND_DISABLE_LIGHTING_FROM_BELOW);
    m_topHeightEdit.AttachItem(9);
    m_bottomHeightEdit.AttachItem(8);
    m_hDisplayInEditor = ::GetDlgItem(GetHwnd(), 13);
    m_hTopImageVisible = ::GetDlgItem(GetHwnd(), 16);
    m_hSideImageVisible = ::GetDlgItem(GetHwnd(), 109);
    m_hAnimateSlingshot= ::GetDlgItem(GetHwnd(), 112);
    m_hFlipbook = ::GetDlgItem(GetHwnd(), 113);
    m_baseImageCombo = &m_topImageCombo;
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);

    UpdateVisuals();
    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(m_topImageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_sideImageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_topMaterialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_sideMaterialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_slingshotMaterialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_disableLightingEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_disableLightFromBelowEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_topHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_bottomHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hDisplayInEditor, CResizer::topleft, 0);
    m_resizer.AddChild(m_hTopImageVisible, CResizer::topleft, 0);
    m_resizer.AddChild(m_hSideImageVisible, CResizer::topleft, 0);
    m_resizer.AddChild(m_hAnimateSlingshot, CResizer::topleft, 0);
    m_resizer.AddChild(m_hFlipbook, CResizer::topleft, 0);
    m_resizer.AddChild(m_bottomHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR WallVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
