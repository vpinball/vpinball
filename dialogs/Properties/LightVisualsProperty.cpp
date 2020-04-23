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
                CHECK_UPDATE_ITEM(light->m_d.m_falloff, PropertyDialog::GetFloatTextbox(m_falloffEdit), light);
                break;
            case IDC_LIGHT_FALLOFF_POWER:
                CHECK_UPDATE_ITEM(light->m_d.m_falloff_power, PropertyDialog::GetFloatTextbox(m_falloffPowerEdit), light);
                break;
            case 12:
                CHECK_UPDATE_ITEM(light->m_d.m_intensity, PropertyDialog::GetFloatTextbox(m_intensityEdit), light);
                break;
            case IDC_FADE_SPEED_UP:
                CHECK_UPDATE_ITEM(light->m_d.m_fadeSpeedUp, PropertyDialog::GetFloatTextbox(m_fadeSpeedUpEdit), light);
                break;
            case IDC_FADE_SPEED_DOWN:
                CHECK_UPDATE_ITEM(light->m_d.m_fadeSpeedDown, PropertyDialog::GetFloatTextbox(m_fadeSpeedDownEdit), light);
                break;
            case IDC_IMAGE_MODE:
                CHECK_UPDATE_ITEM(light->m_d.m_imageMode, PropertyDialog::GetCheckboxState(m_hPassThroughCheck), light);
                break;
            case IDC_DEPTH_BIAS:
                CHECK_UPDATE_ITEM(light->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit), light);
                break;
            case IDC_LIGHT_TYPE_CHECKBOX:
                CHECK_UPDATE_ITEM(light->m_d.m_BulbLight, PropertyDialog::GetCheckboxState(m_hEnableCheck), light);
                break;
            case IDC_SHOW_BULB_MESH:
                CHECK_UPDATE_ITEM(light->m_d.m_showBulbMesh, PropertyDialog::GetCheckboxState(m_hShowMeshCheck), light);
                break;
            case IDC_STATIC_BULB_MESH:
                CHECK_UPDATE_ITEM(light->m_d.m_staticBulbMesh, PropertyDialog::GetCheckboxState(m_hStaticMeshCheck), light);
                break;
            case IDC_REFLECT_ON_BALLS:
                CHECK_UPDATE_ITEM(light->m_d.m_showReflectionOnBall, PropertyDialog::GetCheckboxState(m_hRelectOnBalls), light);
                break;
            case IDC_HALO_EDIT:
                CHECK_UPDATE_ITEM(light->m_d.m_bulbHaloHeight, PropertyDialog::GetFloatTextbox(m_haloHeightEdit), light);
                break;
            case IDC_SCALE_BULB_MESH:
                CHECK_UPDATE_ITEM(light->m_d.m_meshRadius, PropertyDialog::GetFloatTextbox(m_scaleMeshEdit), light);
                break;
            case IDC_BULB_MODULATE_VS_ADD:
                CHECK_UPDATE_ITEM(light->m_d.m_modulate_vs_add, PropertyDialog::GetFloatTextbox(m_modulateEdit), light);
                break;
            case IDC_TRANSMISSION_SCALE:
                CHECK_UPDATE_ITEM(light->m_d.m_transmissionScale, PropertyDialog::GetFloatTextbox(m_transmitEdit), light);
                break;
            case 902:
                CHECK_UPDATE_ITEM(light->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), light);
                break;
            case 903:
                CHECK_UPDATE_ITEM(light->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), light);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT(light->m_d.m_szSurface, m_surfaceCombo, light);
                break;
            case IDC_COLOR_BUTTON1:
            {
                CComObject<PinTable> *ptable=g_pvp->GetActiveTable();
                if(ptable==nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(light->m_d.m_color);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    light->m_d.m_color = m_colorDialog.GetColor();
                    m_colorButton1.SetColor(light->m_d.m_color);
                    memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
                }
                break;
            }
            case IDC_COLOR_BUTTON2:
            {
                CComObject<PinTable>* ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                CHOOSECOLOR cc = m_colorDialog.GetParameters();
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                m_colorDialog.SetParameters(cc);
                m_colorDialog.SetColor(light->m_d.m_color2);
                m_colorDialog.SetCustomColors(ptable->m_rgcolorcustom);
                if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
                {
                    light->m_d.m_color2 = m_colorDialog.GetColor();
                    m_colorButton2.SetColor(light->m_d.m_color2);
                    memcpy(ptable->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(ptable->m_rgcolorcustom));
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
