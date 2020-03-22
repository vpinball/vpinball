#include "stdafx.h"
#include "Properties/TableVisualsProperty.h"
#include <WindowsX.h>

TableVisualsProperty::TableVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_VISUALS, pvsel)
{
}

void TableVisualsProperty::UpdateVisuals()
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();

    if (table == nullptr)
        return;

    PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_imageCombo, table->m_szImage);
    PropertyDialog::UpdateMaterialComboBox(table->GetMaterialList(), m_materialCombo, table->m_szPlayfieldMaterial);
    PropertyDialog::SetCheckboxState(m_hReflectElementsCheck, table->m_reflectElementsOnPlayfield);
    PropertyDialog::SetIntTextbox(m_reflectionStrengthEdit, table->GetPlayfieldReflectionStrength());
    PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_ballImageCombo, table->m_szBallImage);
    PropertyDialog::SetCheckboxState(m_hLogoModeCheck, table->m_BallDecalMode);
    PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_ballDecalCombo, table->m_szBallImageDecal);
    PropertyDialog::SetFloatTextbox(m_ballReflectPlayfieldEdit, table->m_ballPlayfieldReflectionStrength);
    PropertyDialog::SetFloatTextbox(m_ballDefaultBulbIntensScaleEdit, table->m_defaultBulbIntensityScaleOnBall);
}

void TableVisualsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    
    if (table == nullptr)
        return;

    switch (dispid)
    {
        case DISPID_Image:
            CHECK_UPDATE_COMBO_TEXT(table->m_szImage, m_imageCombo, table);
            break;
        case IDC_MATERIAL_COMBO:
            CHECK_UPDATE_COMBO_TEXT(table->m_szPlayfieldMaterial, m_materialCombo, table);
            break;
        case IDC_REFLECT_ELEMENTS_CHECK:
            CHECK_UPDATE_ITEM(table->m_reflectElementsOnPlayfield, PropertyDialog::GetCheckboxState(m_hReflectElementsCheck), table);
            break;
        case IDC_REFLECTION_PLAYFIELD:
            CHECK_UPDATE_VALUE_SETTER(table->SetPlayfieldReflectionStrength, table->GetPlayfieldReflectionStrength, PropertyDialog::GetIntTextbox, m_reflectionStrengthEdit, table);
            break;
        case IDC_BALL_DECAL_MODE:
            CHECK_UPDATE_ITEM(table->m_BallDecalMode, PropertyDialog::GetCheckboxState(m_hLogoModeCheck), table);
            break;
        case 1505:
            CHECK_UPDATE_COMBO_TEXT(table->m_szBallImage, m_ballImageCombo, table);
            break;
        case 1508:
            CHECK_UPDATE_COMBO_TEXT(table->m_szBallImageDecal, m_ballDecalCombo, table);
            break;
        case IDC_BALLPLAYFIELD_REFLECTION:
            CHECK_UPDATE_ITEM(table->m_ballPlayfieldReflectionStrength, PropertyDialog::GetFloatTextbox(m_ballReflectPlayfieldEdit), table);
            break;
        case IDC_BULBINTENSITYSCALE:
            CHECK_UPDATE_ITEM(table->m_defaultBulbIntensityScaleOnBall, PropertyDialog::GetFloatTextbox(m_ballDefaultBulbIntensScaleEdit), table);
            break;
        default:
            break;
    }
    UpdateVisuals();
}

BOOL TableVisualsProperty::OnInitDialog()
{
    AttachItem(DISPID_Image, m_imageCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_hReflectElementsCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ELEMENTS_CHECK);
    AttachItem(IDC_REFLECTION_PLAYFIELD, m_reflectionStrengthEdit);
    AttachItem(1505, m_ballImageCombo);
    m_hLogoModeCheck = ::GetDlgItem(GetHwnd(), IDC_BALL_DECAL_MODE);
    AttachItem(1508, m_ballDecalCombo);
    AttachItem(IDC_BALLPLAYFIELD_REFLECTION, m_ballReflectPlayfieldEdit);
    AttachItem(IDC_BULBINTENSITYSCALE, m_ballDefaultBulbIntensScaleEdit);

    UpdateVisuals();
    return TRUE;
}
