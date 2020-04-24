#include "stdafx.h"
#include "Properties/PlungerVisualsProperty.h"
#include <WindowsX.h>

PlungerVisualsProperty::PlungerVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPLUNGER_VISUALS, pvsel)
{
    m_typeList.push_back("Modern");
    m_typeList.push_back("Flat");
    m_typeList.push_back("Custom");

    m_flatFramesEdit.SetDialog(this);
    m_widthEdit.SetDialog(this);
    m_zAdjustmentEdit.SetDialog(this);
    m_rodDiameterEdit.SetDialog(this);
    m_tipShapeEdit.SetDialog(this);
    m_ringGapEdit.SetDialog(this);
    m_ringDiamEdit.SetDialog(this);
    m_ringWidthEdit.SetDialog(this);
    m_springDiamEdit.SetDialog(this);
    m_springGaugeEdit.SetDialog(this);
    m_springLoopsEdit.SetDialog(this);
    m_endLoopsEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
}

void PlungerVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)plunger->m_d.m_type - 1].c_str());
        PropertyDialog::UpdateSurfaceComboBox(plunger->GetPTable(), m_surfaceCombo, plunger->m_d.m_szSurface);
        PropertyDialog::SetIntTextbox(m_flatFramesEdit, plunger->m_d.m_animFrames);
        PropertyDialog::SetFloatTextbox(m_widthEdit, plunger->m_d.m_width);
        PropertyDialog::SetFloatTextbox(m_zAdjustmentEdit, plunger->m_d.m_zAdjust);
        PropertyDialog::SetFloatTextbox(m_rodDiameterEdit, plunger->m_d.m_rodDiam);
        m_tipShapeEdit.SetWindowText(plunger->m_d.m_szTipShape);
        PropertyDialog::SetFloatTextbox(m_ringGapEdit, plunger->m_d.m_ringGap);
        PropertyDialog::SetFloatTextbox(m_ringDiamEdit, plunger->m_d.m_ringDiam);
        PropertyDialog::SetFloatTextbox(m_ringWidthEdit, plunger->m_d.m_ringWidth);
        PropertyDialog::SetFloatTextbox(m_springDiamEdit, plunger->m_d.m_springDiam);
        PropertyDialog::SetFloatTextbox(m_springGaugeEdit, plunger->m_d.m_springGauge);
        PropertyDialog::SetFloatTextbox(m_springLoopsEdit, plunger->m_d.m_springLoops);
        PropertyDialog::SetFloatTextbox(m_endLoopsEdit, plunger->m_d.m_springEndLoops);
        PropertyDialog::SetFloatTextbox(m_posXEdit, plunger->m_d.m_v.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, plunger->m_d.m_v.y);
        PropertyDialog::SetFloatTextbox(m_endLoopsEdit, plunger->m_d.m_springEndLoops);
        UpdateBaseVisuals(plunger, &plunger->m_d);
        //only show the first element on multi-select
        break;
    }
}

void PlungerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_PLUNGER_TYPE_COMBO:
                CHECK_UPDATE_ITEM(plunger->m_d.m_type, (PlungerType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList) + 1), plunger);
                break;
            case DISPID_PluFrames:
                CHECK_UPDATE_ITEM(plunger->m_d.m_animFrames, PropertyDialog::GetIntTextbox(m_flatFramesEdit), plunger);
                break;
            case DISPID_Width:
                CHECK_UPDATE_ITEM(plunger->m_d.m_width, PropertyDialog::GetFloatTextbox(m_widthEdit), plunger);
                break;
            case DISPID_ZAdjust:
                CHECK_UPDATE_ITEM(plunger->m_d.m_zAdjust, PropertyDialog::GetFloatTextbox(m_zAdjustmentEdit), plunger);
                break;
            case DISPID_RodDiam:
                CHECK_UPDATE_ITEM(plunger->m_d.m_rodDiam, PropertyDialog::GetFloatTextbox(m_rodDiameterEdit), plunger);
                break;
            case DISPID_TipShape:
                PropertyDialog::StartUndo(plunger);
                strncpy_s(plunger->m_d.m_szTipShape, MAXTIPSHAPE-1, m_tipShapeEdit.GetWindowText().c_str(), m_tipShapeEdit.GetWindowText().GetLength());
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_RingGap:
                CHECK_UPDATE_ITEM(plunger->m_d.m_ringGap, PropertyDialog::GetFloatTextbox(m_ringGapEdit), plunger);
                break;
            case DISPID_RingDiam:
                CHECK_UPDATE_ITEM(plunger->m_d.m_ringDiam, PropertyDialog::GetFloatTextbox(m_ringDiamEdit), plunger);
                break;
            case DISPID_RingThickness:
                CHECK_UPDATE_ITEM(plunger->m_d.m_ringWidth, PropertyDialog::GetFloatTextbox(m_ringWidthEdit), plunger);
                break;
            case DISPID_SpringDiam:
                CHECK_UPDATE_ITEM(plunger->m_d.m_springDiam, PropertyDialog::GetFloatTextbox(m_springDiamEdit), plunger);
                break;
            case DISPID_SpringGauge:
                CHECK_UPDATE_ITEM(plunger->m_d.m_springGauge, PropertyDialog::GetFloatTextbox(m_springGaugeEdit), plunger);
                break;
            case DISPID_SpringLoops:
                CHECK_UPDATE_ITEM(plunger->m_d.m_springLoops, PropertyDialog::GetFloatTextbox(m_springLoopsEdit), plunger);
                break;
            case DISPID_SpringEndLoops:
                CHECK_UPDATE_ITEM(plunger->m_d.m_springEndLoops, PropertyDialog::GetFloatTextbox(m_endLoopsEdit), plunger);
                break;
            case 902:
                CHECK_UPDATE_ITEM(plunger->m_d.m_v.x, PropertyDialog::GetFloatTextbox(m_posXEdit), plunger);
                break;
            case 903:
                CHECK_UPDATE_ITEM(plunger->m_d.m_v.y, PropertyDialog::GetFloatTextbox(m_posYEdit), plunger);
                break;
            case 1502:
                CHECK_UPDATE_COMBO_TEXT(plunger->m_d.m_szSurface, m_surfaceCombo, plunger);
                break;
            default:
                UpdateBaseProperties(plunger, &plunger->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL PlungerVisualsProperty::OnInitDialog()
{
    AttachItem(IDC_PLUNGER_TYPE_COMBO, m_typeCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    m_flatFramesEdit.AttachItem(DISPID_PluFrames);
    m_widthEdit.AttachItem(DISPID_Width),
    m_zAdjustmentEdit.AttachItem(DISPID_ZAdjust);
    m_rodDiameterEdit.AttachItem(DISPID_RodDiam);
    m_tipShapeEdit.AttachItem(DISPID_TipShape);
    m_ringGapEdit.AttachItem(DISPID_RingGap);
    m_ringDiamEdit.AttachItem(DISPID_RingDiam);
    m_ringWidthEdit.AttachItem(DISPID_RingThickness);
    m_springDiamEdit.AttachItem(DISPID_SpringDiam);
    m_springGaugeEdit.AttachItem(DISPID_SpringGauge);
    m_springLoopsEdit.AttachItem(DISPID_SpringLoops);
    m_endLoopsEdit.AttachItem(DISPID_SpringEndLoops);
    m_posXEdit.AttachItem(902);
    m_posYEdit.AttachItem(903);
    AttachItem(1502, m_surfaceCombo);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    UpdateVisuals();
    return TRUE;
}
