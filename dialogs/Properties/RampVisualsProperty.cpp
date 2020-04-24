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

    m_depthBiasEdit.SetDialog(this);
    m_topHeightEdit.SetDialog(this);
    m_bottomHeightEdit.SetDialog(this);
    m_topWidthEdit.SetDialog(this);
    m_bottomWdthEdit.SetDialog(this);
    m_leftWallEdit.SetDialog(this);
    m_rightWallEdit.SetDialog(this);
    m_diameterEdit.SetDialog(this);
    m_distanceXEdit.SetDialog(this);
    m_distanceYEdit.SetDialog(this);
}

void RampVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
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
                CHECK_UPDATE_ITEM(ramp->m_d.m_heightbottom, PropertyDialog::GetFloatTextbox(m_bottomHeightEdit), ramp);
                break;
            case 2:
                CHECK_UPDATE_ITEM(ramp->m_d.m_heighttop, PropertyDialog::GetFloatTextbox(m_topHeightEdit), ramp);
                break;
            case 3:
                CHECK_UPDATE_ITEM(ramp->m_d.m_widthbottom, PropertyDialog::GetFloatTextbox(m_bottomWdthEdit), ramp);
                break;
            case 4:
                CHECK_UPDATE_ITEM(ramp->m_d.m_widthtop, PropertyDialog::GetFloatTextbox(m_topWidthEdit), ramp);
                break;
            case 6:
                CHECK_UPDATE_ITEM(ramp->m_d.m_type, (RampType)(PropertyDialog::GetComboBoxIndex(m_typeCombo, m_typeList)), ramp);
                break;
            case 8:
                CHECK_UPDATE_ITEM(ramp->m_d.m_imagealignment, (RampImageAlignment)(PropertyDialog::GetComboBoxIndex(m_modeCombo, m_imageModeList) + 1), ramp);
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
    }
    UpdateVisuals();
}

BOOL RampVisualsProperty::OnInitDialog()
{
    AttachItem(6, m_typeCombo);
    AttachItem(DISPID_Image, m_imageCombo);
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    AttachItem(8, m_modeCombo);
    m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
    m_topHeightEdit.AttachItem(2);
    m_bottomHeightEdit.AttachItem(1);
    m_topWidthEdit.AttachItem(4);
    m_bottomWdthEdit.AttachItem(3);
    m_leftWallEdit.AttachItem(108);
    m_rightWallEdit.AttachItem(109);
    m_diameterEdit.AttachItem(IDC_WIRE_DIAMETER);
    m_distanceXEdit.AttachItem(IDC_WIRE_DISTX);
    m_distanceYEdit.AttachItem(IDC_WIRE_DISTY);

    m_baseImageCombo = &m_imageCombo;
    m_baseMaterialCombo = &m_materialCombo;
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);

    UpdateVisuals();
    return TRUE;
}

