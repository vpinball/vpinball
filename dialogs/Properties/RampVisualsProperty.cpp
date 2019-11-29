#include "stdafx.h"
#include "Properties/RampVisualsProperty.h"
#include <WindowsX.h>

RampVisualsProperty::RampVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRAMP_VISUALS, pvsel)
{
    m_typeList.push_back("Flat");
    m_typeList.push_back("4-Wire");
    m_typeList.push_back("2-Wire");
    m_typeList.push_back("3-Wire Left");
    m_typeList.push_back("3-Wire Right");
    m_typeList.push_back("1-Wire");

    m_imageModeList.push_back("World");
    m_imageModeList.push_back("Wrap");
}

void RampVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
        PropertyDialog::UpdateComboBox(m_typeList, m_typeCombo, m_typeList[(int)ramp->m_d.m_type].c_str());
        PropertyDialog::UpdateComboBox(m_imageModeList, m_modeCombo, m_imageModeList[(int)ramp->m_d.m_imagealignment - 1].c_str());
        PropertyDialog::SetFloatTextbox(m_depthBiasEdit, ramp->m_d.m_depthBias);
        PropertyDialog::SetFloatTextbox(m_topHeightEdit, ramp->m_d.m_heighttop);
        PropertyDialog::SetFloatTextbox(m_bottomHeightEdit, ramp->m_d.m_heightbottom);
        PropertyDialog::SetFloatTextbox(m_topWidthEdit, ramp->m_d.m_widthtop);
        PropertyDialog::SetFloatTextbox(m_bottomWdthEdit, ramp->m_d.m_widthbottom);
        PropertyDialog::SetFloatTextbox(m_leftWallEdit, ramp->m_d.m_leftwallheightvisible);
        PropertyDialog::SetFloatTextbox(m_rightWallEdit, ramp->m_d.m_rightwallheightvisible);
        PropertyDialog::SetFloatTextbox(m_diameterEdit, ramp->m_d.m_wireDiameter);
        PropertyDialog::SetFloatTextbox(m_distanceXEdit, ramp->m_d.m_wireDistanceX);
        PropertyDialog::SetFloatTextbox(m_distanceYEdit, ramp->m_d.m_wireDistanceY);
        PropertyDialog::SetCheckboxState(::GetDlgItem(GetHwnd(), 9), ramp->m_d.m_imageWalls);
        UpdateBaseVisuals(ramp, &ramp->m_d);
        //only show the first element on multi-select
        break;
    }
}

void RampVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRamp))
            continue;
        Ramp * const ramp = (Ramp *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case 1:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_heightbottom = PropertyDialog::GetFloatTextbox(m_bottomHeightEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 2:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_heighttop = PropertyDialog::GetFloatTextbox(m_topHeightEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 3:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_widthbottom = PropertyDialog::GetFloatTextbox(m_bottomWdthEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 4:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_widthtop = PropertyDialog::GetFloatTextbox(m_topWidthEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 6:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_type = (RampType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList));
                PropertyDialog::EndUndo(ramp);
                break;
            case 8:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_imagealignment = (RampImageAlignment)(PropertyDialog::GetComboBoxIndex(m_modeCombo, m_imageModeList) + 1);
                PropertyDialog::EndUndo(ramp);
                break;
            case 9:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_imageWalls = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(ramp);
                break;
            case 108:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_leftwallheightvisible = PropertyDialog::GetFloatTextbox(m_leftWallEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 109:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_rightwallheightvisible = PropertyDialog::GetFloatTextbox(m_rightWallEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case 112:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_visible = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_REFLECT_ENABLED_CHECK:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_reflectionEnabled = PropertyDialog::GetCheckboxState(::GetDlgItem(GetHwnd(), dispid));
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_DEPTH_BIAS:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_depthBias = PropertyDialog::GetFloatTextbox(m_depthBiasEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_WIRE_DIAMETER:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_wireDiameter = PropertyDialog::GetFloatTextbox(m_diameterEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_WIRE_DISTX:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_wireDistanceX = PropertyDialog::GetFloatTextbox(m_distanceXEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            case IDC_WIRE_DISTY:
                PropertyDialog::StartUndo(ramp);
                ramp->m_d.m_wireDistanceY = PropertyDialog::GetFloatTextbox(m_distanceYEdit);
                PropertyDialog::EndUndo(ramp);
                break;
            default:
                UpdateBaseProperties(ramp, &ramp->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL RampVisualsProperty::OnInitDialog()
{
    AttachItem(6, m_typeCombo);
    AttachItem(DISPID_Image, m_imageCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    AttachItem(8, m_modeCombo);
    AttachItem(IDC_DEPTH_BIAS, m_depthBiasEdit);
    AttachItem(2, m_topHeightEdit);
    AttachItem(1, m_bottomHeightEdit);
    AttachItem(4, m_topWidthEdit);
    AttachItem(3, m_bottomWdthEdit);
    AttachItem(108, m_leftWallEdit);
    AttachItem(109, m_rightWallEdit);
    AttachItem(IDC_WIRE_DIAMETER, m_diameterEdit);
    AttachItem(IDC_WIRE_DISTX, m_distanceXEdit);
    AttachItem(IDC_WIRE_DISTY, m_distanceYEdit);

    m_baseImageCombo = &m_imageCombo;
    m_baseMaterialCombo = &m_materialCombo;
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);

    UpdateVisuals();
    return TRUE;
}

