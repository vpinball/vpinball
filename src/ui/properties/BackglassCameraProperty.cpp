// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/BackglassCameraProperty.h"
#include <WindowsX.h>

BackglassCameraProperty::BackglassCameraProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPBACKGLASS_CAMERA, pvsel)
{
    m_viewList.push_back("Desktop (DT)"s);
    m_viewList.push_back("Fullscreen (FS)"s);
    m_viewList.push_back("Full Single Screen (FSS)"s);
    m_viewCombo.SetDialog(this);
    m_modeList.push_back("Legacy"s);
    m_modeList.push_back("Camera"s);
    m_modeList.push_back("Window"s);
    m_modeCombo.SetDialog(this);
    m_testOverrideCombo.SetDialog(this);
    m_overrideList.push_back("Test Desktop"s);
    m_overrideList.push_back("Test Cabinet"s);
    m_overrideList.push_back("Test FSS"s);
    m_overrideList.push_back("Disabled"s);
    m_inclinationEdit.SetDialog(this);
    m_fovEdit.SetDialog(this);
    m_laybackEdit.SetDialog(this);
    m_xyRotationEdit.SetDialog(this);
    m_xScaleEdit.SetDialog(this);
    m_yScaleEdit.SetDialog(this);
    m_zScaleEdit.SetDialog(this);
    m_xViewOffsetEdit.SetDialog(this);
    m_yViewOffsetEdit.SetDialog(this);
    m_xOffsetEdit.SetDialog(this);
    m_yOffsetEdit.SetDialog(this);
    m_zOffsetEdit.SetDialog(this);
}

void BackglassCameraProperty::UpdateVisuals(const int dispid/*=-1*/)
{
   CComObject<PinTable> *const table = g_pvp->GetActiveTable();
   if (table == nullptr)
      return;

   const ViewSetup &viewSetup = table->mViewSetups[table->m_currentBackglassMode];
   if (dispid == IDC_BG_FSS || dispid == -1)
      PropertyDialog::SetCheckboxState(m_hFssModeCheck, table->IsFSSEnabled());
   if (dispid == IDC_BG_TEST_DESKTOP_CHECK || dispid == -1)
   {
      PropertyDialog::UpdateComboBox(m_overrideList, m_testOverrideCombo, m_overrideList[table->m_BG_override]);
   }
   if (dispid == IDC_CAMERA_LAYOUT_MODE || dispid == -1)
   {
      PropertyDialog::UpdateComboBox(m_modeList, m_modeCombo, m_modeList[viewSetup.mMode]);
      bool isLegacy = viewSetup.mMode == VLM_LEGACY;
      bool isWindow = viewSetup.mMode == VLM_WINDOW;
      GetDlgItem(IDC_STATIC2 ).SetWindowText(isLegacy ? "Inclination" : "Look At");
      GetDlgItem(IDC_STATIC9 ).SetWindowText(isLegacy ? "X Offset" : "X Position");
      GetDlgItem(IDC_STATIC10).SetWindowText(isLegacy ? "Y Offset" : "Y Position");
      GetDlgItem(IDC_STATIC11).SetWindowText(isLegacy ? "Z Offset" : "Z Position");
      GetDlgItem(IDC_STATIC5 ).SetWindowText(isWindow ? "Rot. Offset" : "Rotation");
      GetDlgItem(IDC_STATIC7 ).SetWindowText(isWindow ? "&Y/Z Scale" : "&Y Scale");
      GetDlgItem(IDC_STATIC2).ShowWindow(isWindow ? 0 : 1); // LookAt label
      GetDlgItem(IDC_INCLINATION_EDIT).ShowWindow(isWindow ? 0 : 1); // LookAt edit
      GetDlgItem(IDC_STATIC3).ShowWindow(isWindow ? 0 : 1); // FOV label
      GetDlgItem(IDC_FOV_EDIT).ShowWindow(isWindow ? 0 : 1); // FOV edit
      GetDlgItem(IDC_STATIC9).ShowWindow(isWindow ? 0 : 1); // Player X label
      GetDlgItem(IDC_X_OFFSET_EDIT).ShowWindow(isWindow ? 0 : 1); // Player X edit
      GetDlgItem(IDC_STATIC10).ShowWindow(isWindow ? 0 : 1); // Player Y label
      GetDlgItem(IDC_Y_OFFSET_EDIT).ShowWindow(isWindow ? 0 : 1); // Player Y edit
      GetDlgItem(IDC_STATIC11).ShowWindow(isWindow ? 0 : 1); // Player Z label
      GetDlgItem(IDC_Z_OFFSET_EDIT).ShowWindow(isWindow ? 0 : 1); // Player Z edit
      GetDlgItem(IDC_STATIC8).ShowWindow(isWindow ? 0 : 1); // Z Scale label
      GetDlgItem(IDC_TABLE_SCALEZ).ShowWindow(isWindow ? 0 : 1); // Z Scale edit
      GetDlgItem(IDC_STATIC4).ShowWindow(isLegacy ? 1 : 0); // Layback label
      GetDlgItem(IDC_LAYBACK_EDIT).ShowWindow(isLegacy ? 1 : 0); // Layback edit
   }
   if (dispid == IDC_BG_COMBOBOX || dispid == -1)
      PropertyDialog::UpdateComboBox(m_viewList, m_viewCombo, m_viewList[table->m_currentBackglassMode]);
   if (dispid == IDC_INCLINATION_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_inclinationEdit, viewSetup.mLookAt);
   if (dispid == IDC_FOV_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_fovEdit, viewSetup.mFOV);
   if (dispid == IDC_LAYBACK_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_laybackEdit, viewSetup.mLayback);
   if (dispid == IDC_XY_ROTATION_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_xyRotationEdit, viewSetup.mViewportRotation);
   if (dispid == IDC_X_SCALE_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_xScaleEdit, viewSetup.mSceneScaleX);
   if (dispid == IDC_Y_SCALE_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_yScaleEdit, viewSetup.mSceneScaleY);
   if (dispid == IDC_TABLE_SCALEZ || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_zScaleEdit, viewSetup.mSceneScaleZ);
   if (dispid == IDC_X_OFFSET_EDIT2 || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_xViewOffsetEdit, viewSetup.mViewHOfs);
   if (dispid == IDC_Y_OFFSET_EDIT2 || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_yViewOffsetEdit, viewSetup.mViewVOfs);
   if (dispid == IDC_X_OFFSET_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_xOffsetEdit, viewSetup.mViewX);
   if (dispid == IDC_Y_OFFSET_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_yOffsetEdit, viewSetup.mViewY);
   if (dispid == IDC_Z_OFFSET_EDIT || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_zOffsetEdit, viewSetup.mViewZ);
}

void BackglassCameraProperty::UpdateProperties(const int dispid)
{
   CComObject<PinTable> *const table = g_pvp->GetActiveTable();
   if (table == nullptr)
      return;

   ViewSetup &viewSetup = table->mViewSetups[table->m_currentBackglassMode];
   switch (dispid)
   {
      case IDC_BG_FSS: CHECK_UPDATE_VALUE_SETTER(table->EnableFSS, table->IsFSSEnabled, PropertyDialog::GetCheckboxState, m_hFssModeCheck, table); table->UpdateCurrentBGSet(); break;
      case IDC_BG_OVERRIDE_COMBO: CHECK_UPDATE_ITEM(table->m_BG_override, (ViewSetupID)PropertyDialog::GetComboBoxIndex(m_testOverrideCombo, m_overrideList), table); table->UpdateCurrentBGSet(); break;
      case IDC_BG_COMBOBOX: CHECK_UPDATE_ITEM(table->m_currentBackglassMode, (ViewSetupID)PropertyDialog::GetComboBoxIndex(m_viewCombo, m_viewList), table); break;
      case IDC_CAMERA_LAYOUT_MODE: CHECK_UPDATE_ITEM(viewSetup.mMode, (ViewLayoutMode)PropertyDialog::GetComboBoxIndex(m_modeCombo, m_modeList), table); break;
      case IDC_INCLINATION_EDIT: CHECK_UPDATE_ITEM(viewSetup.mLookAt, PropertyDialog::GetFloatTextbox(m_inclinationEdit), table); break;
      case IDC_FOV_EDIT: CHECK_UPDATE_ITEM(viewSetup.mFOV, PropertyDialog::GetFloatTextbox(m_fovEdit), table); break;
      case IDC_LAYBACK_EDIT:
         if (viewSetup.mMode == VLM_LEGACY)
            { CHECK_UPDATE_ITEM(viewSetup.mLayback, PropertyDialog::GetFloatTextbox(m_laybackEdit), table); }
         else
            CHECK_UPDATE_ITEM(viewSetup.mLookAt, PropertyDialog::GetFloatTextbox(m_laybackEdit), table);
         break;
      case IDC_XY_ROTATION_EDIT: CHECK_UPDATE_ITEM(viewSetup.mViewportRotation, PropertyDialog::GetFloatTextbox(m_xyRotationEdit), table); break;
      case IDC_X_SCALE_EDIT: CHECK_UPDATE_ITEM(viewSetup.mSceneScaleX, PropertyDialog::GetFloatTextbox(m_xScaleEdit), table); break;
      case IDC_Y_SCALE_EDIT: CHECK_UPDATE_ITEM(viewSetup.mSceneScaleY, PropertyDialog::GetFloatTextbox(m_yScaleEdit), table); break;
      case IDC_TABLE_SCALEZ: CHECK_UPDATE_ITEM(viewSetup.mSceneScaleZ, PropertyDialog::GetFloatTextbox(m_zScaleEdit), table); break;
      case IDC_X_OFFSET_EDIT: CHECK_UPDATE_ITEM(viewSetup.mViewX, PropertyDialog::GetFloatTextbox(m_xOffsetEdit), table); break;
      case IDC_Y_OFFSET_EDIT: CHECK_UPDATE_ITEM(viewSetup.mViewY, PropertyDialog::GetFloatTextbox(m_yOffsetEdit), table); break;
      case IDC_Z_OFFSET_EDIT: CHECK_UPDATE_ITEM(viewSetup.mViewZ, PropertyDialog::GetFloatTextbox(m_zOffsetEdit), table); break;
      case IDC_X_OFFSET_EDIT2: CHECK_UPDATE_ITEM(viewSetup.mViewHOfs, PropertyDialog::GetFloatTextbox(m_xViewOffsetEdit), table); break;
      case IDC_Y_OFFSET_EDIT2: CHECK_UPDATE_ITEM(viewSetup.mViewVOfs, PropertyDialog::GetFloatTextbox(m_yViewOffsetEdit), table); break;

      default: break;
   }

   UpdateVisuals((dispid == IDC_BG_COMBOBOX) ? -1 : dispid);
}

BOOL BackglassCameraProperty::OnInitDialog()
{
    m_hFssModeCheck = ::GetDlgItem(GetHwnd(), IDC_BG_FSS);
    m_testOverrideCombo.AttachItem(IDC_BG_OVERRIDE_COMBO);
    m_viewCombo.AttachItem(IDC_BG_COMBOBOX);
    m_modeCombo.AttachItem(IDC_CAMERA_LAYOUT_MODE);
    m_inclinationEdit.AttachItem(IDC_INCLINATION_EDIT);
    m_fovEdit.AttachItem(IDC_FOV_EDIT);
    m_laybackEdit.AttachItem(IDC_LAYBACK_EDIT);
    m_xyRotationEdit.AttachItem(IDC_XY_ROTATION_EDIT);
    m_xScaleEdit.AttachItem(IDC_X_SCALE_EDIT);
    m_yScaleEdit.AttachItem(IDC_Y_SCALE_EDIT);
    m_zScaleEdit.AttachItem(IDC_TABLE_SCALEZ);
    m_xViewOffsetEdit.AttachItem(IDC_X_OFFSET_EDIT2);
    m_yViewOffsetEdit.AttachItem(IDC_Y_OFFSET_EDIT2);
    m_xOffsetEdit.AttachItem(IDC_X_OFFSET_EDIT);
    m_yOffsetEdit.AttachItem(IDC_Y_OFFSET_EDIT);
    m_zOffsetEdit.AttachItem(IDC_Z_OFFSET_EDIT);

    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    // Labels
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12).GetHwnd(), CResizer::topleft, 0);
    // Group boxes
    m_resizer.AddChild(GetDlgItem(IDC_STATIC21).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC22).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC23).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC24).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC25).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    // Controls
    m_resizer.AddChild(m_hFssModeCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_testOverrideCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_viewCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_modeCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_inclinationEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_fovEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_laybackEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_xyRotationEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_xScaleEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_yScaleEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_zScaleEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_xViewOffsetEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_yViewOffsetEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_xOffsetEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_yOffsetEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_zOffsetEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR BackglassCameraProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
