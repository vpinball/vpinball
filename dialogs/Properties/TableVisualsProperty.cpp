#include "stdafx.h"
#include "Properties/TableVisualsProperty.h"
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
}

void TableVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();

    if (table == nullptr)
        return;

    if (dispid == DISPID_Image || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_imageCombo, table->m_szImage);
    if (dispid == IDC_MATERIAL_COMBO || dispid == -1)
        PropertyDialog::UpdateMaterialComboBox(table->GetMaterialList(), m_materialCombo, table->m_szPlayfieldMaterial);
    if (dispid == IDC_REFLECT_ELEMENTS_CHECK || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hReflectElementsCheck, table->m_reflectElementsOnPlayfield);
    if (dispid == IDC_REFLECTION_PLAYFIELD || dispid == -1)
        PropertyDialog::SetIntTextbox(m_reflectionStrengthEdit, table->GetPlayfieldReflectionStrength());
    if (dispid == 1505 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_ballImageCombo, table->m_szBallImage);
    if (dispid == IDC_BALL_DECAL_MODE || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hLogoModeCheck, table->m_BallDecalMode);
    if (dispid == 1508 || dispid == -1)
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_ballDecalCombo, table->m_szBallImageDecal);
    if (dispid == IDC_BALLPLAYFIELD_REFLECTION || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_ballReflectPlayfieldEdit, table->m_ballPlayfieldReflectionStrength);
    if (dispid == IDC_BULBINTENSITYSCALE || dispid == -1)
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
    UpdateVisuals(dispid);
}

BOOL TableVisualsProperty::OnInitDialog()
{
    m_imageCombo.AttachItem(DISPID_Image);
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_ballImageCombo.AttachItem(1505);
    m_ballDecalCombo.AttachItem(1508);
    m_hReflectElementsCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ELEMENTS_CHECK);
    m_reflectionStrengthEdit.AttachItem(IDC_REFLECTION_PLAYFIELD);
    m_hLogoModeCheck = ::GetDlgItem(GetHwnd(), IDC_BALL_DECAL_MODE);
    m_ballReflectPlayfieldEdit.AttachItem(IDC_BALLPLAYFIELD_REFLECTION);
    m_ballDefaultBulbIntensScaleEdit.AttachItem(IDC_BULBINTENSITYSCALE);

    UpdateVisuals();
    return TRUE;
}
