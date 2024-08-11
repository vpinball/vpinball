// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/TableLightsProperty.h"
#include <WindowsX.h>

TableLightsProperty::TableLightsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_LIGHTSOURCES, pvsel)
{
    m_lightEmissionScaleEdit.SetDialog(this);
    m_lightHeightEdit.SetDialog(this);
    m_lightRangeEdit.SetDialog(this);
    m_envEmissionScaleEdit.SetDialog(this);
    m_envEmissionImageCombo.SetDialog(this);
    m_sceneLightScaleEdit.SetDialog(this);
}

void TableLightsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
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
        PropertyDialog::UpdateTextureComboBox(table->GetImageList(), m_envEmissionImageCombo, table->m_envImage);
    if (dispid == IDC_ENVEMISSIONSCALE || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_envEmissionScaleEdit, table->m_envEmissionScale);
    if (dispid == IDC_SCENELIGHTSCALE || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_sceneLightScaleEdit, table->m_globalEmissionScale);
}

void TableLightsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
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
                memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
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
                memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
            }
            break;
        }
        case IDC_LIGHTEMISSIONSCALE:
            CHECK_UPDATE_ITEM(table->m_lightEmissionScale, PropertyDialog::GetFloatTextbox(m_lightEmissionScaleEdit), table);
            break;
        case IDC_LIGHTHEIGHT:
            CHECK_UPDATE_ITEM(table->m_lightHeight, PropertyDialog::GetFloatTextbox(m_lightHeightEdit), table);
            break;
        case IDC_LIGHTRANGE:
            CHECK_UPDATE_ITEM(table->m_lightRange, PropertyDialog::GetFloatTextbox(m_lightRangeEdit), table);
            break;
        case DISPID_Image7:
            CHECK_UPDATE_COMBO_TEXT_STRING(table->m_envImage, m_envEmissionImageCombo, table);
            break;
        case IDC_ENVEMISSIONSCALE:
            CHECK_UPDATE_ITEM(table->m_envEmissionScale, PropertyDialog::GetFloatTextbox(m_envEmissionScaleEdit), table);
            break;
        case IDC_SCENELIGHTSCALE:
            CHECK_UPDATE_ITEM(table->m_globalEmissionScale, PropertyDialog::GetFloatTextbox(m_sceneLightScaleEdit), table);
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
    m_sceneLightScaleEdit.AttachItem(IDC_SCENELIGHTSCALE);

    UpdateVisuals();
    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));

    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_envEmissionImageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(m_envEmissionScaleEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(m_colorButton1, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(m_lightEmissionScaleEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(m_lightHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(m_lightRangeEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(m_colorButton2, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_sceneLightScaleEdit, CResizer::topleft, RD_STRETCH_WIDTH);

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
