#include "stdafx.h"
#include "Properties/TableLightsProperty.h"
#include <WindowsX.h>

TableLightsProperty::TableLightsProperty(const VectorProtected<ISelect>* pvsel)
  : BasePropertyDialog(IDD_PROPTABLE_LIGHTSOURCES, pvsel)
{
  m_lightEmissionScaleEdit.SetDialog(this);
  m_lightHeightEdit.SetDialog(this);
  m_lightRangeEdit.SetDialog(this);
  m_envEmissionScaleEdit.SetDialog(this);
  m_ambientOcclusionScaleEdit.SetDialog(this);
  m_bloomStrengthEdit.SetDialog(this);
  m_screenSpaceReflEdit.SetDialog(this);
  m_envEmissionImageCombo.SetDialog(this);
}

void TableLightsProperty::UpdateVisuals(const int dispid /*=-1*/)
{
  CComObject<PinTable>* const table = g_pvp->GetActiveTable();
  if (table == nullptr)
    return;

  if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
    m_colorButton1.SetColor(table->m_lightAmbient);
  if (dispid == IDC_COLOR_BUTTON2 || dispid == -1)
    m_colorButton2.SetColor(table->m_Light[0].emission);
  if (dispid == IDC_LIGHTEMISSIONSCALE || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_lightEmissionScaleEdit, table->m_lightEmissionScale);
  if (dispid == IDC_LIGHTHEIGHT || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_lightHeightEdit, table->m_lightHeight);
  if (dispid == IDC_LIGHTRANGE || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_lightRangeEdit, table->m_lightRange);
  if (dispid == DISPID_Image7 || dispid == -1)
    PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_envEmissionImageCombo,
                                          table->m_envImage);
  if (dispid == IDC_ENVEMISSIONSCALE || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_envEmissionScaleEdit, table->m_envEmissionScale);
  if (dispid == IDC_AOSCALE || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_ambientOcclusionScaleEdit, table->m_AOScale);
  if (dispid == IDC_BLOOM_STRENGTH || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_bloomStrengthEdit, table->m_bloom_strength);
  if (dispid == IDC_SSR_STRENGTH || dispid == -1)
    PropertyDialog::SetFloatTextbox(m_screenSpaceReflEdit, table->m_SSRScale);
}

void TableLightsProperty::UpdateProperties(const int dispid)
{
  CComObject<PinTable>* const table = g_pvp->GetActiveTable();
  if (table == nullptr)
    return;

  switch (dispid)
  {
    case IDC_COLOR_BUTTON1:
    {
      CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
      if (ptable == nullptr)
        break;
      CHOOSECOLOR cc = m_colorDialog.GetParameters();
      cc.Flags = CC_FULLOPEN | CC_RGBINIT;
      m_colorDialog.SetParameters(cc);
      m_colorDialog.SetColor(table->m_lightAmbient);
      m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
      if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
      {
        table->m_lightAmbient = m_colorDialog.GetColor();
        m_colorButton1.SetColor(table->m_lightAmbient);
        memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(),
               sizeof(ptable->m_rgcolorcustom));
      }
      break;
    }
    case IDC_COLOR_BUTTON2:
    {
      CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
      if (ptable == nullptr)
        break;
      CHOOSECOLOR cc = m_colorDialog.GetParameters();
      cc.Flags = CC_FULLOPEN | CC_RGBINIT;
      m_colorDialog.SetParameters(cc);
      m_colorDialog.SetColor(table->m_Light[0].emission);
      m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
      if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
      {
        table->m_Light[0].emission = m_colorDialog.GetColor();
        m_colorButton2.SetColor(table->m_Light[0].emission);
        memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(),
               sizeof(ptable->m_rgcolorcustom));
      }
      break;
    }
    case IDC_LIGHTEMISSIONSCALE:
      CHECK_UPDATE_ITEM(table->m_lightEmissionScale,
                        PropertyDialog::GetFloatTextbox(m_lightEmissionScaleEdit), table);
      break;
    case IDC_LIGHTHEIGHT:
      CHECK_UPDATE_ITEM(table->m_lightHeight, PropertyDialog::GetFloatTextbox(m_lightHeightEdit),
                        table);
      break;
    case IDC_LIGHTRANGE:
      CHECK_UPDATE_ITEM(table->m_lightRange, PropertyDialog::GetFloatTextbox(m_lightRangeEdit),
                        table);
      break;
    case DISPID_Image7:
      CHECK_UPDATE_COMBO_TEXT_STRING(table->m_envImage, m_envEmissionImageCombo, table);
      break;
    case IDC_ENVEMISSIONSCALE:
      CHECK_UPDATE_ITEM(table->m_envEmissionScale,
                        PropertyDialog::GetFloatTextbox(m_envEmissionScaleEdit), table);
      break;
    case IDC_AOSCALE:
      CHECK_UPDATE_ITEM(table->m_AOScale,
                        PropertyDialog::GetFloatTextbox(m_ambientOcclusionScaleEdit), table);
      break;
    case IDC_BLOOM_STRENGTH:
      CHECK_UPDATE_ITEM(table->m_bloom_strength,
                        PropertyDialog::GetFloatTextbox(m_bloomStrengthEdit), table);
      break;
    case IDC_SSR_STRENGTH:
      CHECK_UPDATE_ITEM(table->m_SSRScale, PropertyDialog::GetFloatTextbox(m_screenSpaceReflEdit),
                        table);
      break;
    default:
      break;
  }
  UpdateVisuals(dispid);
}

BOOL TableLightsProperty::OnInitDialog()
{
  AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
  AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
  m_lightEmissionScaleEdit.AttachItem(IDC_LIGHTEMISSIONSCALE);
  m_lightHeightEdit.AttachItem(IDC_LIGHTHEIGHT);
  m_lightRangeEdit.AttachItem(IDC_LIGHTRANGE);
  m_envEmissionImageCombo.AttachItem(DISPID_Image7);
  m_envEmissionScaleEdit.AttachItem(IDC_ENVEMISSIONSCALE);
  m_ambientOcclusionScaleEdit.AttachItem(IDC_AOSCALE);
  m_bloomStrengthEdit.AttachItem(IDC_BLOOM_STRENGTH);
  m_screenSpaceReflEdit.AttachItem(IDC_SSR_STRENGTH);

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
  m_resizer.AddChild(m_colorButton1, topleft, 0);
  m_resizer.AddChild(m_colorButton2, topleft, 0);
  m_resizer.AddChild(m_lightEmissionScaleEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_lightHeightEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_lightRangeEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_envEmissionImageCombo, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_envEmissionScaleEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_ambientOcclusionScaleEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_bloomStrengthEdit, topleft, RD_STRETCH_WIDTH);
  m_resizer.AddChild(m_screenSpaceReflEdit, topleft, RD_STRETCH_WIDTH);
  return TRUE;
}

INT_PTR TableLightsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        m_colorButton1.DrawItem(lpDrawItemStruct);
      }
      else if (nID == IDC_COLOR_BUTTON2)
      {
        m_colorButton2.DrawItem(lpDrawItemStruct);
      }
      return TRUE;
    }
  }
  return DialogProcDefault(uMsg, wParam, lParam);
}
