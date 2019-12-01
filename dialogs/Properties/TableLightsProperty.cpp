#include "stdafx.h"
#include "Properties/TableLightsProperty.h"
#include <WindowsX.h>

TableLightsProperty::TableLightsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_LIGHTSOURCES, pvsel)
{
}

void TableLightsProperty::UpdateVisuals()
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();

    m_colorButton1.SetColor(table->m_lightAmbient);
    m_colorButton2.SetColor(table->m_Light[0].emission);
    PropertyDialog::SetFloatTextbox(m_lightEmissionScaleEdit, table->m_lightEmissionScale);
    PropertyDialog::SetFloatTextbox(m_lightHeightEdit, table->m_lightHeight);
    PropertyDialog::SetFloatTextbox(m_lightRangeEdit, table->m_lightRange);
    PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_envEmissionImageCombo, table->m_szEnvImage);
    PropertyDialog::SetFloatTextbox(m_envEmissionScaleEdit, table->m_envEmissionScale);
    PropertyDialog::SetFloatTextbox(m_ambientOcclusionScaleEdit, table->m_AOScale);
    PropertyDialog::SetFloatTextbox(m_bloomStrengthEdit, table->m_bloom_strength);
    PropertyDialog::SetFloatTextbox(m_screenSpaceReflEdit, table->m_SSRScale);
}

void TableLightsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    switch (dispid)
    {
        case IDC_COLOR_BUTTON1:
        {
            CHOOSECOLOR cc = m_colorDialog.GetParameters();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            m_colorDialog.SetParameters(cc);
            m_colorDialog.SetColor(table->m_lightAmbient);
            if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
            {
                table->m_lightAmbient = m_colorDialog.GetColor();
                m_colorButton1.SetColor(table->m_lightAmbient);
            }
            break;
        }
        case IDC_COLOR_BUTTON2:
        {
            CHOOSECOLOR cc = m_colorDialog.GetParameters();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            m_colorDialog.SetParameters(cc);
            m_colorDialog.SetColor(table->m_Light[0].emission);
            if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
            {
                table->m_Light[0].emission = m_colorDialog.GetColor();
                m_colorButton2.SetColor(table->m_Light[0].emission);
            }
            break;
        }
        case IDC_LIGHTEMISSIONSCALE:
            PropertyDialog::StartUndo(table);
            table->m_lightEmissionScale = PropertyDialog::GetFloatTextbox(m_lightEmissionScaleEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_LIGHTHEIGHT:
            PropertyDialog::StartUndo(table);
            table->m_lightHeight = PropertyDialog::GetFloatTextbox(m_lightHeightEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_LIGHTRANGE:
            PropertyDialog::StartUndo(table);
            table->m_lightRange = PropertyDialog::GetFloatTextbox(m_lightRangeEdit);
            PropertyDialog::EndUndo(table);
            break;
        case DISPID_Image7:
            PropertyDialog::StartUndo(table);
            PropertyDialog::GetComboBoxText(m_envEmissionImageCombo, table->m_szEnvImage);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_ENVEMISSIONSCALE:
            PropertyDialog::StartUndo(table);
            table->m_envEmissionScale = PropertyDialog::GetFloatTextbox(m_envEmissionScaleEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_AOSCALE:
            PropertyDialog::StartUndo(table);
            table->m_AOScale = PropertyDialog::GetFloatTextbox(m_ambientOcclusionScaleEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_BLOOM_STRENGTH:
            PropertyDialog::StartUndo(table);
            table->m_bloom_strength = PropertyDialog::GetFloatTextbox(m_bloomStrengthEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_SSR_STRENGTH:
            PropertyDialog::StartUndo(table);
            table->m_SSRScale = PropertyDialog::GetFloatTextbox(m_screenSpaceReflEdit);
            PropertyDialog::EndUndo(table);
            break;
        default:
            break;
    }
}

BOOL TableLightsProperty::OnInitDialog()
{
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
    AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
    AttachItem(IDC_LIGHTEMISSIONSCALE, m_lightEmissionScaleEdit);
    AttachItem(IDC_LIGHTHEIGHT, m_lightHeightEdit);
    AttachItem(IDC_LIGHTRANGE, m_lightRangeEdit);
    AttachItem(DISPID_Image7, m_envEmissionImageCombo);
    AttachItem(IDC_ENVEMISSIONSCALE, m_envEmissionScaleEdit);
    AttachItem(IDC_AOSCALE, m_ambientOcclusionScaleEdit);
    AttachItem(IDC_BLOOM_STRENGTH, m_bloomStrengthEdit);
    AttachItem(IDC_SSR_STRENGTH, m_screenSpaceReflEdit);

    UpdateVisuals();
    return TRUE;
}

INT_PTR TableLightsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
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
