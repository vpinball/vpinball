// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/TableVisualsProperty.h"
#include <WindowsX.h>

TableVisualsProperty::TableVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_VISUALS, pvsel)
{
   m_reflectionStrengthEdit.SetDialog(this);
   m_ballReflectPlayfieldEdit.SetDialog(this);
   m_ballDefaultBulbIntensScaleEdit.SetDialog(this);
   m_imageCombo.SetDialog(this);
   m_materialCombo.SetDialog(this);
   m_ballImageCombo.SetDialog(this);
   m_ballDecalCombo.SetDialog(this);
   m_toneMapperCombo.SetDialog(this);
   m_ambientOcclusionScaleEdit.SetDialog(this);
   m_screenSpaceReflEdit.SetDialog(this);
   m_bloomStrengthEdit.SetDialog(this);
   m_exposureEdit.SetDialog(this);
}

void TableVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    if (dispid == DISPID_Image || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_imageCombo, table->m_image);
    if (dispid == IDC_MATERIAL_COMBO || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(table->GetMaterialList(), m_materialCombo, table->m_playfieldMaterial);
    if (dispid == IDC_REFLECTION_PLAYFIELD || dispid == -1)
        PropertyDialog::SetIntTextbox(m_reflectionStrengthEdit, table->GetPlayfieldReflectionStrength());
    if (dispid == IDC_BALL_SPHERICAL_MAP || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hSphericalMapCheck, table->m_ballSphericalMapping);
    if (dispid == DISPID_Image3 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_ballImageCombo, table->m_ballImage);
    if (dispid == IDC_BALL_DECAL_MODE || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hLogoModeCheck, table->m_BallDecalMode);
    if (dispid == DISPID_Image4 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_ballDecalCombo, table->m_ballImageDecal);
    if (dispid == IDC_BALLPLAYFIELD_REFLECTION || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_ballReflectPlayfieldEdit, table->m_ballPlayfieldReflectionStrength);
    if (dispid == IDC_BULBINTENSITYSCALE || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_ballDefaultBulbIntensScaleEdit, table->m_defaultBulbIntensityScaleOnBall);
    if (dispid == IDC_ENABLE_AO || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hEnableAOCheck, table->m_enableAO);
    if (dispid == IDC_AOSCALE || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_ambientOcclusionScaleEdit, table->m_AOScale);
    if (dispid == IDC_ENABLE_SSR || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hEnableSSRCheck, table->m_enableSSR);
    if (dispid == IDC_SSR_STRENGTH || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_screenSpaceReflEdit, table->m_SSRScale);
    if (dispid == IDC_BLOOM_STRENGTH || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_bloomStrengthEdit, table->m_bloom_strength);
    if (dispid == IDC_EXPOSURE || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_exposureEdit, table->GetExposure());
    if (dispid == IDC_TONEMAPPER || dispid == -1)
    {
       if (m_toneMapperCombo.GetCount() < 2)
       {
          m_toneMapperCombo.ResetContent();
          m_toneMapperCombo.AddString(_T("Reinhard"));
          m_toneMapperCombo.AddString(_T("AgX"));
          m_toneMapperCombo.AddString(_T("Filmic"));
          m_toneMapperCombo.AddString(_T("Neutral"));
          #ifdef ENABLE_BGFX
          m_toneMapperCombo.AddString(_T("AgX Punchy"));
          #endif
       }
       m_toneMapperCombo.SetCurSel((int) table->GetToneMapper());
    }
}

void TableVisualsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    switch (dispid)
    {
        case DISPID_Image:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_image, m_imageCombo, table);
            break;
        case IDC_MATERIAL_COMBO:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_playfieldMaterial, m_materialCombo, table);
            break;
        case IDC_REFLECTION_PLAYFIELD:
            CHECK_UPDATE_VALUE_SETTER(table->SetPlayfieldReflectionStrength, table->GetPlayfieldReflectionStrength, PropertyDialog::GetIntTextbox, m_reflectionStrengthEdit, table);
            break;
        case IDC_BALL_SPHERICAL_MAP:
            CHECK_UPDATE_ITEM(table->m_ballSphericalMapping, PropertyDialog::GetCheckboxState(m_hSphericalMapCheck), table);
            break;
        case IDC_BALL_DECAL_MODE:
            CHECK_UPDATE_ITEM(table->m_BallDecalMode, PropertyDialog::GetCheckboxState(m_hLogoModeCheck), table);
            break;
        case DISPID_Image3:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_ballImage, m_ballImageCombo, table);
            break;
        case DISPID_Image4:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_ballImageDecal, m_ballDecalCombo, table);
            break;
        case IDC_BALLPLAYFIELD_REFLECTION:
            CHECK_UPDATE_ITEM(table->m_ballPlayfieldReflectionStrength, PropertyDialog::GetFloatTextbox(m_ballReflectPlayfieldEdit), table);
            break;
        case IDC_BULBINTENSITYSCALE:
            CHECK_UPDATE_ITEM(table->m_defaultBulbIntensityScaleOnBall, PropertyDialog::GetFloatTextbox(m_ballDefaultBulbIntensScaleEdit), table);
            break;
        case IDC_ENABLE_AO:
            CHECK_UPDATE_ITEM(table->m_enableAO, PropertyDialog::GetCheckboxState(m_hEnableAOCheck), table);
            break;
        case IDC_AOSCALE:
            CHECK_UPDATE_ITEM(table->m_AOScale, PropertyDialog::GetFloatTextbox(m_ambientOcclusionScaleEdit), table);
            break;
        case IDC_ENABLE_SSR:
            CHECK_UPDATE_ITEM(table->m_enableSSR, PropertyDialog::GetCheckboxState(m_hEnableSSRCheck), table);
            break;
        case IDC_SSR_STRENGTH:
            CHECK_UPDATE_ITEM(table->m_SSRScale, PropertyDialog::GetFloatTextbox(m_screenSpaceReflEdit), table);
            break;
        case IDC_BLOOM_STRENGTH:
            CHECK_UPDATE_ITEM(table->m_bloom_strength, PropertyDialog::GetFloatTextbox(m_bloomStrengthEdit), table);
            break;
        case IDC_EXPOSURE:
            CHECK_UPDATE_VALUE_SETTER(table->SetExposure, table->GetExposure, PropertyDialog::GetFloatTextbox, m_exposureEdit, table);
            break;
        case IDC_TONEMAPPER:
            if (m_toneMapperCombo.GetCurSel() != CB_ERR && m_toneMapperCombo.GetCurSel() != table->GetToneMapper())
            {
               PropertyDialog::StartUndo(table);
               table->SetToneMapper((ToneMapper) m_toneMapperCombo.GetCurSel());
               PropertyDialog::EndUndo(table);
            }
            break;
        default:
            break;
    }
    UpdateVisuals(dispid);
}

BOOL TableVisualsProperty::OnInitDialog()
{
    m_imageCombo.AttachItem(DISPID_Image);
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_ballImageCombo.AttachItem(DISPID_Image3);
    m_ballDecalCombo.AttachItem(DISPID_Image4);
    m_reflectionStrengthEdit.AttachItem(IDC_REFLECTION_PLAYFIELD);
    m_hLogoModeCheck = GetDlgItem(IDC_BALL_DECAL_MODE).GetHwnd();
    m_hSphericalMapCheck = GetDlgItem(IDC_BALL_SPHERICAL_MAP).GetHwnd();
    m_ballReflectPlayfieldEdit.AttachItem(IDC_BALLPLAYFIELD_REFLECTION);
    m_ballDefaultBulbIntensScaleEdit.AttachItem(IDC_BULBINTENSITYSCALE);
    m_hEnableAOCheck = GetDlgItem(IDC_ENABLE_AO).GetHwnd();
    m_ambientOcclusionScaleEdit.AttachItem(IDC_AOSCALE);
    m_hEnableSSRCheck = GetDlgItem(IDC_ENABLE_SSR).GetHwnd();
    m_screenSpaceReflEdit.AttachItem(IDC_SSR_STRENGTH);
    m_bloomStrengthEdit.AttachItem(IDC_BLOOM_STRENGTH);
    m_exposureEdit.AttachItem(IDC_EXPOSURE);
    m_toneMapperCombo.AttachItem(IDC_TONEMAPPER);

    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ballDecalCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ballImageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_reflectionStrengthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hLogoModeCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hSphericalMapCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_ballReflectPlayfieldEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ballDefaultBulbIntensScaleEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hEnableAOCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(m_ambientOcclusionScaleEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hEnableSSRCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(m_screenSpaceReflEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(m_bloomStrengthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_toneMapperCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(m_exposureEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR TableVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
