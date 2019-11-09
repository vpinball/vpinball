#include "stdafx.h"
#include "Properties/PlungerVisualsProperty.h"
#include <WindowsX.h>

PlungerVisualsProperty::PlungerVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPLUNGER_VISUALS, pvsel)
{
    m_typeList.push_back("PlungerTypeModern");
    m_typeList.push_back("PlungerTypeFlat");
    m_typeList.push_back("PlungerTypeCustom");
}

void PlungerVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger *plunger = (Plunger *)m_pvsel->ElementAt(i);
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
        Plunger *plunger = (Plunger *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_PLUNGER_TYPE_COMBO:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_type = (PlungerType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList) + 1);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_PluFrames:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_animFrames = PropertyDialog::GetIntTextbox(m_flatFramesEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_Width:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_width = PropertyDialog::GetFloatTextbox(m_widthEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_ZAdjust:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_zAdjust = PropertyDialog::GetFloatTextbox(m_zAdjustmentEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_RodDiam:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_rodDiam = PropertyDialog::GetFloatTextbox(m_rodDiameterEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_TipShape:
                PropertyDialog::StartUndo(plunger);
                strncpy_s(plunger->m_d.m_szTipShape, MAXTIPSHAPE-1, m_tipShapeEdit.GetWindowText().c_str(), m_tipShapeEdit.GetWindowText().GetLength());
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_RingGap:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_ringGap = PropertyDialog::GetFloatTextbox(m_ringGapEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_RingDiam:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_ringDiam = PropertyDialog::GetFloatTextbox(m_ringDiamEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_RingThickness:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_ringWidth = PropertyDialog::GetFloatTextbox(m_ringWidthEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_SpringDiam:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_springDiam = PropertyDialog::GetFloatTextbox(m_springDiamEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_SpringGauge:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_springGauge = PropertyDialog::GetFloatTextbox(m_springGaugeEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_SpringLoops:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_springLoops = PropertyDialog::GetFloatTextbox(m_springLoopsEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case DISPID_SpringEndLoops:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_springEndLoops = PropertyDialog::GetFloatTextbox(m_endLoopsEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case 902:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_v.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case 903:
                PropertyDialog::StartUndo(plunger);
                plunger->m_d.m_v.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(plunger);
                break;
            case 1502:
                PropertyDialog::StartUndo(plunger);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, plunger->m_d.m_szSurface);
                PropertyDialog::EndUndo(plunger);
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
    AttachItem(DISPID_PluFrames, m_flatFramesEdit);
    AttachItem(DISPID_Width, m_widthEdit);
    AttachItem(DISPID_ZAdjust, m_zAdjustmentEdit);
    AttachItem(DISPID_RodDiam, m_rodDiameterEdit);
    AttachItem(DISPID_TipShape, m_tipShapeEdit);
    AttachItem(DISPID_RingGap, m_ringGapEdit);
    AttachItem(DISPID_RingDiam, m_ringDiamEdit);
    AttachItem(DISPID_RingThickness, m_ringWidthEdit);
    AttachItem(DISPID_SpringDiam, m_springDiamEdit);
    AttachItem(DISPID_SpringGauge, m_springGaugeEdit);
    AttachItem(DISPID_SpringLoops, m_springLoopsEdit);
    AttachItem(DISPID_SpringEndLoops, m_endLoopsEdit);
    AttachItem(902, m_posXEdit);
    AttachItem(903, m_posYEdit);
    AttachItem(1502, m_surfaceCombo);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    UpdateVisuals();
    return TRUE;
}


