// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/BallVisualsProperty.h"
#include <WindowsX.h>

BallVisualsProperty::BallVisualsProperty(const VectorProtected<ISelect> *pvsel)
   : BasePropertyDialog(IDD_PROPBALL_VISUALS, pvsel)
{
   m_imageCombo.SetDialog(this);
   m_decalImageCombo.SetDialog(this);
   m_bulbIntensityScaleEdit.SetDialog(this);
   m_pfReflectionStrengthEdit.SetDialog(this);
   m_posXEdit.SetDialog(this);
   m_posYEdit.SetDialog(this);
   m_posZEdit.SetDialog(this);
   m_radiusEdit.SetDialog(this);
}

void BallVisualsProperty::UpdateVisuals(const int dispid /*=-1*/)
{
   for (int i = 0; i < m_pvsel->size(); i++)
   {
      if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBall))
         continue;
      Ball *const ball = (Ball *)m_pvsel->ElementAt(i);
      if (dispid == IDC_USE_TABLE_SETTINGS || dispid == -1)
      {
         PropertyDialog::SetCheckboxState(m_hUseTableSettings, ball->m_d.m_useTableRenderSettings);
         m_imageCombo.EnableWindow(!ball->m_d.m_useTableRenderSettings);
         m_decalImageCombo.EnableWindow(!ball->m_d.m_useTableRenderSettings);
         m_bulbIntensityScaleEdit.EnableWindow(!ball->m_d.m_useTableRenderSettings);
         m_pfReflectionStrengthEdit.EnableWindow(!ball->m_d.m_useTableRenderSettings);
         ::EnableWindow(GetDlgItem(IDC_COLOR_BUTTON1), !ball->m_d.m_useTableRenderSettings);
         ::EnableWindow(m_hSphericalCheck, !ball->m_d.m_useTableRenderSettings);
         ::EnableWindow(m_hDecalModeCheck, !ball->m_d.m_useTableRenderSettings);
      }
      if (dispid == DISPID_Image2 || dispid == -1)
         PropertyDialog::UpdateTextureComboBox(ball->GetPTable()->GetImageList(), m_decalImageCombo, ball->m_d.m_imageDecal);
      if (dispid == IDC_BALL_SPHERICAL_MAP || dispid == -1)
         PropertyDialog::SetCheckboxState(m_hSphericalCheck, ball->m_d.m_pinballEnvSphericalMapping);
      if (dispid == IDC_BALL_DECAL_MODE || dispid == -1)
         PropertyDialog::SetCheckboxState(m_hDecalModeCheck, ball->m_d.m_decalMode);
      if (dispid == IDC_BALL_REFLECTION || dispid == -1)
         PropertyDialog::SetCheckboxState(m_hForceReflectionCheck, ball->m_d.m_forceReflection);
      if (dispid == IDC_BALLPLAYFIELD_REFLECTION || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_pfReflectionStrengthEdit, ball->m_d.m_playfieldReflectionStrength);
      if (dispid == IDC_BULBINTENSITYSCALE || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_bulbIntensityScaleEdit, ball->m_d.m_bulb_intensity_scale);
      if (dispid == IDC_POS_X || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_posXEdit, ball->m_hitBall.m_d.m_pos.x);
      if (dispid == IDC_POS_Y || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_posYEdit, ball->m_hitBall.m_d.m_pos.y);
      if (dispid == IDC_POS_Z || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_posZEdit, ball->m_hitBall.m_d.m_pos.z);
      if (dispid == IDC_RADIUS_EDIT || dispid == -1)
         PropertyDialog::SetFloatTextbox(m_radiusEdit, ball->m_hitBall.m_d.m_radius);
      if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
         m_colorButton.SetColor(ball->m_d.m_color);

      UpdateBaseVisuals(ball, &ball->m_d, dispid);
      //only show the first element on multi-select
      break;
   }
}

void BallVisualsProperty::UpdateProperties(const int dispid)
{
   for (int i = 0; i < m_pvsel->size(); i++)
   {
      if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBall))
         continue;
      Ball *const ball = (Ball *)m_pvsel->ElementAt(i);
      switch (dispid)
      {
      case IDC_USE_TABLE_SETTINGS: CHECK_UPDATE_ITEM(ball->m_d.m_useTableRenderSettings, PropertyDialog::GetCheckboxState(m_hUseTableSettings), ball); break;
      case DISPID_Image2: CHECK_UPDATE_COMBO_TEXT_STRING(ball->m_d.m_imageDecal, m_decalImageCombo, ball); break;
      case IDC_BALL_SPHERICAL_MAP: CHECK_UPDATE_ITEM(ball->m_d.m_pinballEnvSphericalMapping, PropertyDialog::GetCheckboxState(m_hSphericalCheck), ball); break;
      case IDC_BALL_DECAL_MODE: CHECK_UPDATE_ITEM(ball->m_d.m_decalMode, PropertyDialog::GetCheckboxState(m_hDecalModeCheck), ball); break;
      case IDC_BALL_REFLECTION: CHECK_UPDATE_ITEM(ball->m_d.m_forceReflection, PropertyDialog::GetCheckboxState(m_hForceReflectionCheck), ball); break;
      case IDC_BALLPLAYFIELD_REFLECTION: CHECK_UPDATE_ITEM(ball->m_d.m_playfieldReflectionStrength, PropertyDialog::GetFloatTextbox(m_pfReflectionStrengthEdit), ball); break;
      case IDC_BULBINTENSITYSCALE: CHECK_UPDATE_ITEM(ball->m_d.m_bulb_intensity_scale, PropertyDialog::GetFloatTextbox(m_bulbIntensityScaleEdit), ball); break;
      case IDC_POS_X: CHECK_UPDATE_ITEM(ball->m_hitBall.m_d.m_pos.x, PropertyDialog::GetFloatTextbox(m_posXEdit), ball); break;
      case IDC_POS_Y: CHECK_UPDATE_ITEM(ball->m_hitBall.m_d.m_pos.y, PropertyDialog::GetFloatTextbox(m_posYEdit), ball); break;
      case IDC_POS_Z: CHECK_UPDATE_ITEM(ball->m_hitBall.m_d.m_pos.z, PropertyDialog::GetFloatTextbox(m_posZEdit), ball); break;
      case IDC_RADIUS_EDIT: CHECK_UPDATE_ITEM(ball->m_hitBall.m_d.m_radius, PropertyDialog::GetFloatTextbox(m_radiusEdit), ball); break;
      case IDC_COLOR_BUTTON1:
      {
         CComObject<PinTable> *const ptable = g_pvp->GetActiveTable();
         if (ptable == nullptr)
            break;
         CHOOSECOLOR cc = m_colorDialog.GetParameters();
         cc.Flags = CC_FULLOPEN | CC_RGBINIT;
         m_colorDialog.SetParameters(cc);
         m_colorDialog.SetColor(ball->m_d.m_color);
         m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
         if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
         {
            ball->m_d.m_color = m_colorDialog.GetColor();
            m_colorButton.SetColor(ball->m_d.m_color);
            memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
         }
         break;
      }
      default: UpdateBaseProperties(ball, &ball->m_d, dispid); break;
      }
   }
   UpdateVisuals(dispid);
}

BOOL BallVisualsProperty::OnInitDialog()
{
   m_hVisibleCheck = GetDlgItem(IDC_VISIBLE_CHECK);
   m_hReflectionEnabledCheck = GetDlgItem(IDC_REFLECT_ENABLED_CHECK);
   m_hUseTableSettings = GetDlgItem(IDC_USE_TABLE_SETTINGS);
   m_imageCombo.AttachItem(DISPID_Image);
   m_baseImageCombo = &m_imageCombo;
   m_hSphericalCheck = GetDlgItem(IDC_BALL_SPHERICAL_MAP);
   m_decalImageCombo.AttachItem(DISPID_Image2);
   m_hDecalModeCheck = GetDlgItem(IDC_BALL_DECAL_MODE);
   m_pfReflectionStrengthEdit.AttachItem(IDC_BALLPLAYFIELD_REFLECTION);
   m_bulbIntensityScaleEdit.AttachItem(IDC_BULBINTENSITYSCALE);
   m_hForceReflectionCheck = GetDlgItem(IDC_BALL_REFLECTION);
   m_posXEdit.AttachItem(IDC_POS_X);
   m_posYEdit.AttachItem(IDC_POS_Y);
   m_posZEdit.AttachItem(IDC_POS_Z);
   m_radiusEdit.AttachItem(IDC_RADIUS_EDIT);
   AttachItem(IDC_COLOR_BUTTON1, m_colorButton);

   UpdateVisuals();

   m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 0, 0));
   m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_hForceReflectionCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   
   m_resizer.AddChild(GetDlgItem(IDC_STATIC16).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_hUseTableSettings, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC5).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_hSphericalCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_imageCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC6).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_hDecalModeCheck, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_decalImageCombo.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC9).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_colorButton.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

   m_resizer.AddChild(GetDlgItem(IDC_STATIC15).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC7).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_pfReflectionStrengthEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC8).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_bulbIntensityScaleEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

   m_resizer.AddChild(GetDlgItem(IDC_STATIC10).GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC11).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_posXEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC12).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_posYEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC13).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_posZEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC14).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_radiusEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

   return TRUE;
}

INT_PTR BallVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
