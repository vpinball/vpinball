#include "stdafx.h"
#include "Properties/BackglassCameraProperty.h"
#include <WindowsX.h>

BackglassCameraProperty::BackglassCameraProperty(const VectorProtected<ISelect>* pvsel)
  : BasePropertyDialog(IDD_PROPBACKGLASS_CAMERA, pvsel)
{
  m_modeList.push_back("Desktop (DT)");
  m_modeList.push_back("Fullscreen (FS)");
  m_modeList.push_back("Full Single Screen (FSS)");
  m_inclinationEdit.SetDialog(this);
  m_fovEdit.SetDialog(this);
  m_laybackEdit.SetDialog(this);
  m_xyRotationEdit.SetDialog(this);
  m_xScaleEdit.SetDialog(this);
  m_yScaleEdit.SetDialog(this);
  m_zScaleEdit.SetDialog(this);
  m_xOffsetEdit.SetDialog(this);
  m_yOffsetEdit.SetDialog(this);
  m_zOffsetEdit.SetDialog(this);
  m_modeCombo.SetDialog(this);
}

void BackglassCameraProperty::UpdateVisuals(const int dispid /*=-1*/)
{
  CComObject<PinTable>* const table = g_pvp->GetActiveTable();
  if (table == nullptr)
  {
    return;
  }

  if (dispid == IDC_BG_FSS || dispid == -1)
  {
    PropertyDialog::SetCheckboxState(m_hFssModeCheck, table->GetShowFSS());
  }
  if (dispid == IDC_BG_TEST_DESKTOP_CHECK || dispid == -1)
  {
    PropertyDialog::SetCheckboxState(m_hTestDesktopCheck, table->GetShowDT());
  }
  if (dispid == IDC_BG_COMBOBOX || dispid == -1)
  {
    PropertyDialog::UpdateComboBox(m_modeList, m_modeCombo,
                                   m_modeList[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_INCLINATION_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_inclinationEdit,
                                    table->m_BG_inclination[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_FOV_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_fovEdit, table->m_BG_FOV[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_LAYBACK_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_laybackEdit,
                                    table->m_BG_layback[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_XY_ROTATION_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_xyRotationEdit,
                                    table->m_BG_rotation[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_X_SCALE_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_xScaleEdit,
                                    table->m_BG_scalex[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_Y_SCALE_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_yScaleEdit,
                                    table->m_BG_scaley[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_TABLE_SCALEZ || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_zScaleEdit,
                                    table->m_BG_scalez[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_X_OFFSET_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_xOffsetEdit,
                                    table->m_BG_xlatex[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_Y_OFFSET_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_yOffsetEdit,
                                    table->m_BG_xlatey[table->m_currentBackglassMode]);
  }
  if (dispid == IDC_Z_OFFSET_EDIT || dispid == -1)
  {
    PropertyDialog::SetFloatTextbox(m_zOffsetEdit,
                                    table->m_BG_xlatez[table->m_currentBackglassMode]);
  }
}

void BackglassCameraProperty::UpdateProperties(const int dispid)
{
  CComObject<PinTable>* const table = g_pvp->GetActiveTable();
  if (table == nullptr)
  {
    return;
  }

  switch (dispid)
  {
    case IDC_BG_FSS:
      CHECK_UPDATE_VALUE_SETTER(table->SetShowFSS, table->GetShowFSS,
                                PropertyDialog::GetCheckboxState, m_hFssModeCheck, table);
      break;
    case IDC_BG_TEST_DESKTOP_CHECK:
      CHECK_UPDATE_VALUE_SETTER(table->SetShowDT, table->GetShowDT,
                                PropertyDialog::GetCheckboxState, m_hTestDesktopCheck, table);
      break;
    case IDC_BG_COMBOBOX:
      CHECK_UPDATE_ITEM(table->m_currentBackglassMode,
                        PropertyDialog::GetComboBoxIndex(m_modeCombo, m_modeList), table);
      break;
    case IDC_INCLINATION_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_inclination[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_inclinationEdit), table);
      break;
    case IDC_FOV_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_FOV[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_fovEdit), table);
      break;
    case IDC_LAYBACK_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_layback[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_laybackEdit), table);
      break;
    case IDC_XY_ROTATION_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_rotation[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_xyRotationEdit), table);
      break;
    case IDC_X_SCALE_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_scalex[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_xScaleEdit), table);
      break;
    case IDC_Y_SCALE_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_scaley[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_yScaleEdit), table);
      break;
    case IDC_TABLE_SCALEZ:
      CHECK_UPDATE_ITEM(table->m_BG_scalez[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_zScaleEdit), table);
      break;
    case IDC_X_OFFSET_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_xlatex[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_xOffsetEdit), table);
      break;
    case IDC_Y_OFFSET_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_xlatey[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_yOffsetEdit), table);
      break;
    case IDC_Z_OFFSET_EDIT:
      CHECK_UPDATE_ITEM(table->m_BG_xlatez[table->m_currentBackglassMode],
                        PropertyDialog::GetFloatTextbox(m_zOffsetEdit), table);
      break;
    default:
      break;
  }

  UpdateVisuals((dispid == IDC_BG_COMBOBOX) ? -1 : dispid);
}

BOOL BackglassCameraProperty::OnInitDialog()
{
  m_hFssModeCheck = ::GetDlgItem(GetHwnd(), IDC_BG_FSS);
  m_hTestDesktopCheck = ::GetDlgItem(GetHwnd(), IDC_BG_TEST_DESKTOP_CHECK);
  m_modeCombo.AttachItem(IDC_BG_COMBOBOX);
  m_inclinationEdit.AttachItem(IDC_INCLINATION_EDIT);
  m_fovEdit.AttachItem(IDC_FOV_EDIT);
  m_laybackEdit.AttachItem(IDC_LAYBACK_EDIT);
  m_xyRotationEdit.AttachItem(IDC_XY_ROTATION_EDIT);
  m_xScaleEdit.AttachItem(IDC_X_SCALE_EDIT);
  m_yScaleEdit.AttachItem(IDC_Y_SCALE_EDIT);
  m_zScaleEdit.AttachItem(IDC_TABLE_SCALEZ);
  m_xOffsetEdit.AttachItem(IDC_X_OFFSET_EDIT);
  m_yOffsetEdit.AttachItem(IDC_Y_OFFSET_EDIT);
  m_zOffsetEdit.AttachItem(IDC_Z_OFFSET_EDIT);

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
  m_resizer.AddChild(GetDlgItem(IDC_STATIC11), topleft, 0);
  m_resizer.AddChild(m_hFssModeCheck, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_hTestDesktopCheck, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_modeCombo, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_inclinationEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_fovEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_laybackEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_xyRotationEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_xScaleEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_yScaleEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_zScaleEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_xOffsetEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_yOffsetEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_zOffsetEdit, topleft, RD_STRETCH_WIDTH);
  return TRUE;
}

INT_PTR BackglassCameraProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  m_resizer.HandleMessage(uMsg, wParam, lParam);
  return DialogProcDefault(uMsg, wParam, lParam);
}
