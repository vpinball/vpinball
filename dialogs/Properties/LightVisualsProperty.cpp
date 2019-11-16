#include "stdafx.h"
#include "Properties/LightVisualsProperty.h"
#include <WindowsX.h>

LightVisualsProperty::LightVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHT_VISUALS, pvsel)
{
}

void LightVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);

        PropertyDialog::SetFloatTextbox(m_falloffEdit, light->m_d.m_falloff);
        PropertyDialog::SetFloatTextbox(m_falloffPowerEdit, light->m_d.m_falloff_power);
        PropertyDialog::SetFloatTextbox(m_intensityEdit, light->m_d.m_intensity);
        PropertyDialog::SetFloatTextbox(m_fadeSpeedUpEdit, light->m_d.m_fadeSpeedUp);
        PropertyDialog::SetFloatTextbox(m_fadeSpeedDownEdit, light->m_d.m_fadeSpeedDown);
        PropertyDialog::SetCheckboxState(m_hPassThroughCheck, light->m_d.m_imageMode);
        PropertyDialog::SetCheckboxState(m_hEnableCheck, light->m_d.m_BulbLight);
        PropertyDialog::SetCheckboxState(m_hShowMeshCheck, light->m_d.m_showBulbMesh);
        PropertyDialog::SetCheckboxState(m_hStaticMeshCheck, light->m_d.m_staticBulbMesh);
        PropertyDialog::SetCheckboxState(m_hRelectOnBalls, light->m_d.m_showReflectionOnBall);
        PropertyDialog::SetFloatTextbox(m_depthBiasEdit, light->m_d.m_depthBias);
        PropertyDialog::SetFloatTextbox(m_haloHeightEdit, light->m_d.m_bulbHaloHeight);
        PropertyDialog::SetFloatTextbox(m_scaleMeshEdit, light->m_d.m_meshRadius);
        PropertyDialog::SetFloatTextbox(m_modulateEdit, light->m_d.m_modulate_vs_add);
        PropertyDialog::SetFloatTextbox(m_transmitEdit, light->m_d.m_transmissionScale);
        PropertyDialog::SetFloatTextbox(m_posXEdit, light->m_d.m_vCenter.x);
        PropertyDialog::SetFloatTextbox(m_posYEdit, light->m_d.m_vCenter.y);
        PropertyDialog::UpdateSurfaceComboBox(light->GetPTable(), m_surfaceCombo, light->m_d.m_szSurface);
        m_colorButton1.SetColor(light->m_d.m_color);
        m_colorButton2.SetColor(light->m_d.m_color2);
        UpdateBaseVisuals(light, &light->m_d);
        //only show the first element on multi-select
        break;
    }
}

void LightVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case 1:
                PropertyDialog::StartUndo(light);
                light->m_d.m_falloff = PropertyDialog::GetFloatTextbox(m_falloffEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_LIGHT_FALLOFF_POWER:
                PropertyDialog::StartUndo(light);
                light->m_d.m_falloff_power = PropertyDialog::GetFloatTextbox(m_falloffPowerEdit);
                PropertyDialog::EndUndo(light);
                break;
            case 12:
                PropertyDialog::StartUndo(light);
                light->m_d.m_intensity = PropertyDialog::GetFloatTextbox(m_intensityEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_FADE_SPEED_UP:
                PropertyDialog::StartUndo(light);
                light->m_d.m_fadeSpeedUp = PropertyDialog::GetFloatTextbox(m_fadeSpeedUpEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_FADE_SPEED_DOWN:
                PropertyDialog::StartUndo(light);
                light->m_d.m_fadeSpeedDown = PropertyDialog::GetFloatTextbox(m_fadeSpeedDownEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_IMAGE_MODE:
                PropertyDialog::StartUndo(light);
                light->m_d.m_imageMode = PropertyDialog::GetCheckboxState(m_hPassThroughCheck);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_DEPTH_BIAS:
                PropertyDialog::StartUndo(light);
                light->m_d.m_depthBias = PropertyDialog::GetFloatTextbox(m_depthBiasEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_LIGHT_TYPE_CHECKBOX:
                PropertyDialog::StartUndo(light);
                light->m_d.m_BulbLight = PropertyDialog::GetCheckboxState(m_hEnableCheck);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_SHOW_BULB_MESH:
                PropertyDialog::StartUndo(light);
                light->m_d.m_showBulbMesh = PropertyDialog::GetCheckboxState(m_hShowMeshCheck);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_STATIC_BULB_MESH:
                PropertyDialog::StartUndo(light);
                light->m_d.m_staticBulbMesh = PropertyDialog::GetCheckboxState(m_hStaticMeshCheck);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_REFLECT_ON_BALLS:
                PropertyDialog::StartUndo(light);
                light->m_d.m_showReflectionOnBall = PropertyDialog::GetCheckboxState(m_hRelectOnBalls);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_HALO_EDIT:
                PropertyDialog::StartUndo(light);
                light->m_d.m_bulbHaloHeight = PropertyDialog::GetFloatTextbox(m_haloHeightEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_SCALE_BULB_MESH:
                PropertyDialog::StartUndo(light);
                light->m_d.m_meshRadius = PropertyDialog::GetFloatTextbox(m_scaleMeshEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_BULB_MODULATE_VS_ADD:
                PropertyDialog::StartUndo(light);
                light->m_d.m_modulate_vs_add = PropertyDialog::GetFloatTextbox(m_modulateEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_TRANSMISSION_SCALE:
                PropertyDialog::StartUndo(light);
                light->m_d.m_transmissionScale = PropertyDialog::GetFloatTextbox(m_transmitEdit);
                PropertyDialog::EndUndo(light);
                break;
            case 902:
                PropertyDialog::StartUndo(light);
                light->m_d.m_vCenter.x = PropertyDialog::GetFloatTextbox(m_posXEdit);
                PropertyDialog::EndUndo(light);
                break;
            case 903:
                PropertyDialog::StartUndo(light);
                light->m_d.m_vCenter.y = PropertyDialog::GetFloatTextbox(m_posYEdit);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_SURFACE_COMBO:
                PropertyDialog::StartUndo(light);
                PropertyDialog::GetComboBoxText(m_surfaceCombo, light->m_d.m_szSurface);
                PropertyDialog::EndUndo(light);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(light->m_d.m_color);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    light->m_d.m_color = m_colorDialog.GetColor();
                    m_colorButton1.SetColor(light->m_d.m_color);
                }
                break;
            }
            case IDC_COLOR_BUTTON2:
            {
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(light->m_d.m_color2);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    light->m_d.m_color2 = m_colorDialog.GetColor();
                    m_colorButton2.SetColor(light->m_d.m_color2);
                }
                break;
            }
            default:
                UpdateBaseProperties(light, &light->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL LightVisualsProperty::OnInitDialog()
{
    AttachItem(1, m_falloffEdit);
    AttachItem(IDC_LIGHT_FALLOFF_POWER, m_falloffPowerEdit);
    AttachItem(12, m_intensityEdit);
    AttachItem(IDC_FADE_SPEED_UP, m_fadeSpeedUpEdit);
    AttachItem(IDC_FADE_SPEED_DOWN, m_fadeSpeedDownEdit);
    m_hPassThroughCheck = ::GetDlgItem(GetHwnd(), IDC_IMAGE_MODE);
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    AttachItem(IDC_DEPTH_BIAS, m_depthBiasEdit);
    m_hEnableCheck= ::GetDlgItem(GetHwnd(), IDC_LIGHT_TYPE_CHECKBOX);
    m_hShowMeshCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_BULB_MESH);
    m_hStaticMeshCheck = ::GetDlgItem(GetHwnd(), IDC_STATIC_BULB_MESH);
    AttachItem(IDC_HALO_EDIT, m_haloHeightEdit);
    AttachItem(IDC_SCALE_BULB_MESH, m_scaleMeshEdit);
    AttachItem(IDC_BULB_MODULATE_VS_ADD, m_modulateEdit);
    AttachItem(IDC_TRANSMISSION_SCALE, m_transmitEdit);
    m_hRelectOnBalls = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ON_BALLS);
    AttachItem(902, m_posXEdit);
    AttachItem(903, m_posYEdit);
    AttachItem(IDC_SURFACE_COMBO, m_surfaceCombo);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
    AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
    UpdateVisuals();
    return TRUE;
}

INT_PTR LightVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            UINT nID = static_cast<UINT>(wParam);
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
