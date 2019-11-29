#include "stdafx.h"
#include "Properties/BackglassCameraProperty.h"
#include <WindowsX.h>


BackglassCameraProperty::BackglassCameraProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPBACKGLASS_CAMERA, pvsel)
{
    m_modeList.push_back("Desktop (DT)");
    m_modeList.push_back("Fullscreen (FS)");
    m_modeList.push_back("Full Single Screen (FSS)");
}

void BackglassCameraProperty::UpdateVisuals()
{
    CComObject<PinTable> *const table = g_pvp->GetActiveTable();
    PropertyDialog::SetCheckboxState(m_hFssModeCheck, table->GetShowFSS());
    PropertyDialog::SetCheckboxState(m_hTestDesktopCheck, table->GetShowDT());
    PropertyDialog::UpdateComboBox(m_modeList, m_modeCombo, m_modeList[table->m_currentBackglassMode].c_str());
    PropertyDialog::SetFloatTextbox(m_inclinationEdit, table->m_BG_inclination[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_fovEdit, table->m_BG_FOV[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_laybackEdit, table->m_BG_layback[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_xyRotationEdit, table->m_BG_rotation[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_xScaleEdit, table->m_BG_scalex[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_yScaleEdit, table->m_BG_scaley[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_zScaleEdit, table->m_BG_scalez[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_xOffsetEdit, table->m_BG_xlatex[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_yOffsetEdit, table->m_BG_xlatey[table->m_currentBackglassMode]);
    PropertyDialog::SetFloatTextbox(m_zOffsetEdit, table->m_BG_xlatez[table->m_currentBackglassMode]);
}

void BackglassCameraProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> *const table = g_pvp->GetActiveTable();
    switch (dispid)
    {
        case IDC_BG_FSS:
            PropertyDialog::StartUndo(table);
            table->SetShowFSS(PropertyDialog::GetCheckboxState(m_hFssModeCheck));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_BG_TEST_DESKTOP_CHECK:
            table->SetShowDT(PropertyDialog::GetCheckboxState(m_hTestDesktopCheck));
            break;
        case IDC_BG_COMBOBOX:
            PropertyDialog::StartUndo(table);
            table->m_currentBackglassMode = PropertyDialog::GetComboBoxIndex(m_modeCombo, m_modeList);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_INCLINATION_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_inclination[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_inclinationEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_FOV_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_FOV[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_fovEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_LAYBACK_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_layback[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_laybackEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_XY_ROTATION_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_rotation[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_xyRotationEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_X_SCALE_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_scalex[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_xScaleEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_Y_SCALE_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_scaley[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_yScaleEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLE_SCALEZ:
            PropertyDialog::StartUndo(table);
            table->m_BG_scalez[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_zScaleEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_X_OFFSET_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_xlatex[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_xOffsetEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_Y_OFFSET_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_xlatey[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_yOffsetEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_Z_OFFSET_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_BG_xlatez[table->m_currentBackglassMode] = PropertyDialog::GetFloatTextbox(m_zOffsetEdit);
            PropertyDialog::EndUndo(table);
            break;
        default:
            break;
    }
    UpdateVisuals();
}

BOOL BackglassCameraProperty::OnInitDialog()
{
    m_hFssModeCheck = ::GetDlgItem(GetHwnd(), IDC_BG_FSS);
    m_hTestDesktopCheck = ::GetDlgItem(GetHwnd(), IDC_BG_TEST_DESKTOP_CHECK);
    AttachItem(IDC_BG_COMBOBOX, m_modeCombo);
    AttachItem(IDC_INCLINATION_EDIT, m_inclinationEdit);
    AttachItem(IDC_FOV_EDIT, m_fovEdit);
    AttachItem(IDC_LAYBACK_EDIT, m_laybackEdit);
    AttachItem(IDC_XY_ROTATION_EDIT, m_xyRotationEdit);
    AttachItem(IDC_X_SCALE_EDIT, m_xScaleEdit);
    AttachItem(IDC_Y_SCALE_EDIT, m_yScaleEdit);
    AttachItem(IDC_TABLE_SCALEZ, m_zScaleEdit);
    AttachItem(IDC_X_OFFSET_EDIT, m_xOffsetEdit);
    AttachItem(IDC_Y_OFFSET_EDIT, m_yOffsetEdit);
    AttachItem(IDC_Z_OFFSET_EDIT, m_zOffsetEdit);

    UpdateVisuals();

    return TRUE;
}
