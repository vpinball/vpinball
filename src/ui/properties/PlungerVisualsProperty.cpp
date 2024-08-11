// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/PlungerVisualsProperty.h"
#include <WindowsX.h>

PlungerVisualsProperty::PlungerVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPPLUNGER_VISUALS, pvsel)
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

    m_typeCombo.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void PlungerVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);
        if (dispid == IDC_PLUNGER_TYPE_COMBO || dispid == -1)
            PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)plunger->m_d.m_type-1]);
        if (dispid == 1502 || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(plunger->GetPTable(), m_surfaceCombo, plunger->m_d.m_szSurface);
        if (dispid == DISPID_PluFrames || dispid == -1)
            PropertyDialog::SetIntTextbox(m_flatFramesEdit, plunger->m_d.m_animFrames);
        if (dispid == DISPID_Width || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_widthEdit, plunger->m_d.m_width);
        if (dispid == DISPID_ZAdjust || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_zAdjustmentEdit, plunger->m_d.m_zAdjust);
        if (dispid == DISPID_RodDiam || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rodDiameterEdit, plunger->m_d.m_rodDiam);
        if (dispid == DISPID_TipShape || dispid == -1)
            m_tipShapeEdit.SetWindowText(plunger->m_d.m_szTipShape);
        if (dispid == DISPID_RingGap || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_ringGapEdit, plunger->m_d.m_ringGap);
        if (dispid == DISPID_RingDiam || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_ringDiamEdit, plunger->m_d.m_ringDiam);
        if (dispid == DISPID_RingThickness || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_ringWidthEdit, plunger->m_d.m_ringWidth);
        if (dispid == DISPID_SpringDiam || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_springDiamEdit, plunger->m_d.m_springDiam);
        if (dispid == DISPID_SpringGauge || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_springGaugeEdit, plunger->m_d.m_springGauge);
        if (dispid == DISPID_SpringLoops || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_springLoopsEdit, plunger->m_d.m_springLoops);
        if (dispid == 902 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, plunger->m_d.m_v.x);
        if (dispid == 903 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, plunger->m_d.m_v.y);
        if (dispid == DISPID_SpringEndLoops || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_endLoopsEdit, plunger->m_d.m_springEndLoops);
        UpdateBaseVisuals(plunger, &plunger->m_d);
        //only show the first element on multi-select
        break;
    }
}

void PlungerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemPlunger))
            continue;
        Plunger * const plunger = (Plunger *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_PLUNGER_TYPE_COMBO:
                CHECK_UPDATE_ITEM(plunger->m_d.m_type, (PlungerType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList)+1), plunger);
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
                strncpy_s(plunger->m_d.m_szTipShape, m_tipShapeEdit.GetWindowText().c_str(), sizeof(plunger->m_d.m_szTipShape)-1);
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
                CHECK_UPDATE_COMBO_TEXT_STRING(plunger->m_d.m_szSurface, m_surfaceCombo, plunger);
                break;
            default:
                UpdateBaseProperties(plunger, &plunger->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL PlungerVisualsProperty::OnInitDialog()
{
    m_typeCombo.AttachItem(IDC_PLUNGER_TYPE_COMBO);
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;
    m_flatFramesEdit.AttachItem(DISPID_PluFrames);
    m_widthEdit.AttachItem(DISPID_Width);
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
    m_surfaceCombo.AttachItem(1502);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC17), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC18), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC19), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC20), CResizer::topleft, 0);
    m_resizer.AddChild(m_typeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_flatFramesEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_widthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_zAdjustmentEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rodDiameterEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tipShapeEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ringGapEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ringDiamEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_ringWidthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_springDiamEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_springGaugeEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_springLoopsEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_endLoopsEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_surfaceCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, 0);

    return TRUE;
}

INT_PTR PlungerVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
