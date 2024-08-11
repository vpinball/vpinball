// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/RampVisualsProperty.h"
#include <WindowsX.h>

RampVisualsProperty::RampVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRAMP_VISUALS, pvsel)
{
    m_typeList.push_back("Flat"s);
    m_typeList.push_back("4-Wire"s);
    m_typeList.push_back("2-Wire"s);
    m_typeList.push_back("3-Wire Left"s);
    m_typeList.push_back("3-Wire Right"s);
    m_typeList.push_back("1-Wire"s);

    m_imageModeList.push_back("World"s);
    m_imageModeList.push_back("Wrap"s);

    m_depthBiasEdit.SetDialog(this);
    m_topHeightEdit.SetDialog(this);
    m_bottomHeightEdit.SetDialog(this);
    m_topWidthEdit.SetDialog(this);
    m_bottomWidthEdit.SetDialog(this);
    m_leftWallEdit.SetDialog(this);
    m_rightWallEdit.SetDialog(this);
    m_diameterEdit.SetDialog(this);
    m_distanceXEdit.SetDialog(this);
    m_distanceYEdit.SetDialog(this);

    m_typeCombo.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_modeCombo.SetDialog(this);
}

void RampVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
        if (dispid == 6 || dispid == -1)
            PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)ramp->m_d.m_type]);
        if (dispid == 8 || dispid == -1)
            PropertyDialog::UpdateComboBox(m_imageModeList, m_modeCombo, m_imageModeList[(int)ramp->m_d.m_imagealignment]);
        if (dispid == IDC_DEPTH_BIAS || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_depthBiasEdit, ramp->m_d.m_depthBias);
        if (dispid == 2 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_topHeightEdit, ramp->m_d.m_heighttop);
        if (dispid == 1 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_bottomHeightEdit, ramp->m_d.m_heightbottom);
        if (dispid == 4 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_topWidthEdit, ramp->m_d.m_widthtop);
        if (dispid == 3 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_bottomWidthEdit, ramp->m_d.m_widthbottom);
        if (dispid == 108 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_leftWallEdit, ramp->m_d.m_leftwallheightvisible);
        if (dispid == 109 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rightWallEdit, ramp->m_d.m_rightwallheightvisible);
        if (dispid == IDC_WIRE_DIAMETER || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_diameterEdit, ramp->m_d.m_wireDiameter);
        if (dispid == IDC_WIRE_DISTX || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_distanceXEdit, ramp->m_d.m_wireDistanceX);
        if (dispid == IDC_WIRE_DISTY || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_distanceYEdit, ramp->m_d.m_wireDistanceY);
        if (dispid == 9 || dispid == -1)
            PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 9), ramp->m_d.m_imageWalls);

        UpdateBaseVisuals(ramp, &ramp->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void RampVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 1:
                CHECK_UPDATE_ITEM(ramp->m_d.m_heightbottom, PropertyDialog::GetFloatTextbox(m_bottomHeightEdit), ramp);
                break;
            case 2:
                CHECK_UPDATE_ITEM(ramp->m_d.m_heighttop, PropertyDialog::GetFloatTextbox(m_topHeightEdit), ramp);
                break;
            case 3:
                CHECK_UPDATE_ITEM(ramp->m_d.m_widthbottom, PropertyDialog::GetFloatTextbox(m_bottomWidthEdit), ramp);
                break;
            case 4:
                CHECK_UPDATE_ITEM(ramp->m_d.m_widthtop, PropertyDialog::GetFloatTextbox(m_topWidthEdit), ramp);
                break;
            case 6:
                CHECK_UPDATE_ITEM(ramp->m_d.m_type, (RampType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList)), ramp);
                break;
            case 8:
                CHECK_UPDATE_ITEM(ramp->m_d.m_imagealignment, (RampImageAlignment)(PropertyDialog::GetComboBoxIndex(m_modeCombo, m_imageModeList)), ramp);
                break;
            case 9:
                CHECK_UPDATE_ITEM(ramp->m_d.m_imageWalls, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), ramp);
                break;
            case 108:
                CHECK_UPDATE_ITEM(ramp->m_d.m_leftwallheightvisible, PropertyDialog::GetFloatTextbox(m_leftWallEdit), ramp);
                break;
            case 109:
                CHECK_UPDATE_ITEM(ramp->m_d.m_rightwallheightvisible, PropertyDialog::GetFloatTextbox(m_rightWallEdit), ramp);
                break;
            case 112:
                CHECK_UPDATE_ITEM(ramp->m_d.m_visible, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), ramp);
                break;
            case IDC_REFLECT_ENABLED_CHECK:
                CHECK_UPDATE_ITEM(ramp->m_d.m_reflectionEnabled, PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid)), ramp);
                break;
            case IDC_DEPTH_BIAS:
                CHECK_UPDATE_ITEM(ramp->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit), ramp);
                break;
            case IDC_WIRE_DIAMETER:
                CHECK_UPDATE_ITEM(ramp->m_d.m_wireDiameter, PropertyDialog::GetFloatTextbox(m_diameterEdit), ramp);
                break;
            case IDC_WIRE_DISTX:
                CHECK_UPDATE_ITEM(ramp->m_d.m_wireDistanceX, PropertyDialog::GetFloatTextbox(m_distanceXEdit), ramp);
                break;
            case IDC_WIRE_DISTY:
                CHECK_UPDATE_ITEM(ramp->m_d.m_wireDistanceY, PropertyDialog::GetFloatTextbox(m_distanceYEdit), ramp);
                break;
            default:
                UpdateBaseProperties(ramp, &ramp->m_d, dispid);
                break;
        }
        ramp->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL RampVisualsProperty::OnInitDialog()
{
    m_typeCombo.AttachItem(6);
    m_imageCombo.AttachItem(DISPID_Image);
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_modeCombo.AttachItem(8);
    m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
    m_topHeightEdit.AttachItem(2);
    m_bottomHeightEdit.AttachItem(1);
    m_topWidthEdit.AttachItem(4);
    m_bottomWidthEdit.AttachItem(3);
    m_leftWallEdit.AttachItem(108);
    m_rightWallEdit.AttachItem(109);
    m_diameterEdit.AttachItem(IDC_WIRE_DIAMETER);
    m_distanceXEdit.AttachItem(IDC_WIRE_DISTX);
    m_distanceYEdit.AttachItem(IDC_WIRE_DISTY);

    m_baseImageCombo = &m_imageCombo;
    m_baseMaterialCombo = &m_materialCombo;
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hApplyImageToWallCheck = ::GetDlgItem(GetHwnd(), 9);

    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC17), CResizer::topleft, 0);
    m_resizer.AddChild(m_typeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_modeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_depthBiasEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_topHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_bottomHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_topWidthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_bottomWidthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_leftWallEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rightWallEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_diameterEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_distanceXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_distanceYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hApplyImageToWallCheck, CResizer::topleft, 0);

    return TRUE;
}

INT_PTR RampVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
