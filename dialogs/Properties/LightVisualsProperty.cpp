#include "stdafx.h"
#include "Properties/LightVisualsProperty.h"
#include <WindowsX.h>

LightVisualsProperty::LightVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPLIGHT_VISUALS, pvsel)
{
    m_falloffEdit.SetDialog(this);
    m_falloffPowerEdit.SetDialog(this);
    m_intensityEdit.SetDialog(this);
    m_fadeSpeedUpEdit.SetDialog(this);
    m_fadeSpeedDownEdit.SetDialog(this);
    m_depthBiasEdit.SetDialog(this);
    m_haloHeightEdit.SetDialog(this);
    m_scaleMeshEdit.SetDialog(this);
    m_modulateEdit.SetDialog(this);
    m_transmitEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void LightVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);

        if (dispid == 1 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_falloffEdit, light->m_d.m_falloff);
        if (dispid == IDC_LIGHT_FALLOFF_POWER || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_falloffPowerEdit, light->m_d.m_falloff_power);
        if (dispid == 12 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_intensityEdit, light->m_d.m_intensity);
        if (dispid == IDC_FADE_SPEED_UP || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_fadeSpeedUpEdit, light->m_d.m_fadeSpeedUp);
        if (dispid == IDC_FADE_SPEED_DOWN || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_fadeSpeedDownEdit, light->m_d.m_fadeSpeedDown);
        if (dispid == IDC_IMAGE_MODE || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hPassThroughCheck, light->m_d.m_imageMode);
        if (dispid == IDC_LIGHT_TYPE_CHECKBOX || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hEnableCheck, light->m_d.m_BulbLight);
        if (dispid == IDC_SHOW_BULB_MESH || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hShowMeshCheck, light->m_d.m_showBulbMesh);
        if (dispid == IDC_STATIC_BULB_MESH || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hStaticMeshCheck, light->m_d.m_staticBulbMesh);
        if (dispid == IDC_REFLECT_ON_BALLS || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hRelectOnBalls, light->m_d.m_showReflectionOnBall);
        if (dispid == IDC_DEPTH_BIAS || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_depthBiasEdit, light->m_d.m_depthBias);
        if (dispid == IDC_HALO_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_haloHeightEdit, light->m_d.m_bulbHaloHeight);
        if (dispid == IDC_SCALE_BULB_MESH || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_scaleMeshEdit, light->m_d.m_meshRadius);
        if (dispid == IDC_BULB_MODULATE_VS_ADD || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_modulateEdit, light->m_d.m_modulate_vs_add);
        if (dispid == IDC_TRANSMISSION_SCALE || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_transmitEdit, light->m_d.m_transmissionScale);
        if (dispid == 902 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, light->m_d.m_vCenter.x);
        if (dispid == 903 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, light->m_d.m_vCenter.y);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(light->GetPTable(), m_surfaceCombo, light->m_d.m_szSurface);
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_colorButton1.SetColor(light->m_d.m_color);
        if (dispid == IDC_COLOR_BUTTON2 || dispid == -1)
            m_colorButton2.SetColor(light->m_d.m_color2);

        UpdateBaseVisuals(light, &light->m_d, dispid);
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
                CComObject<PinTable> * const ptable = g_pvp->GetActiveTable();
                if(ptable==nullptr)
                    break;
                if(i==0)
                {
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
                }
                else
                {
                   Light* const firstLight = (Light*)m_pvsel->ElementAt(0);
                   light->m_d.m_color = firstLight->m_d.m_color;
                   m_colorButton1.SetColor(light->m_d.m_color);
                   ptable->SetDirtyDraw();
                }
                break;
            }
            case IDC_COLOR_BUTTON2:
            {
                CComObject<PinTable>* const ptable = g_pvp->GetActiveTable();
                if (ptable == nullptr)
                    break;
                if(i==0)
                {
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
                }
                else
                {
                   Light* const firstLight = (Light*)m_pvsel->ElementAt(0);
                   light->m_d.m_color2 = firstLight->m_d.m_color2;
                   m_colorButton2.SetColor(light->m_d.m_color2);
                }
                break;
            }
            default:
                UpdateBaseProperties(light, &light->m_d, dispid);
                break;
        }
    }
    UpdateVisuals(dispid);
}

BOOL LightVisualsProperty::OnInitDialog()
{
    m_falloffEdit.AttachItem(1);
    m_falloffPowerEdit.AttachItem(IDC_LIGHT_FALLOFF_POWER);
    m_intensityEdit.AttachItem(12);
    m_fadeSpeedUpEdit.AttachItem(IDC_FADE_SPEED_UP);
    m_fadeSpeedDownEdit.AttachItem(IDC_FADE_SPEED_DOWN);
    m_hPassThroughCheck = ::GetDlgItem(GetHwnd(), IDC_IMAGE_MODE);
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;
    m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
    m_hEnableCheck= ::GetDlgItem(GetHwnd(), IDC_LIGHT_TYPE_CHECKBOX);
    m_hShowMeshCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_BULB_MESH);
    m_hStaticMeshCheck = ::GetDlgItem(GetHwnd(), IDC_STATIC_BULB_MESH);
    m_haloHeightEdit.AttachItem(IDC_HALO_EDIT);
    m_scaleMeshEdit.AttachItem(IDC_SCALE_BULB_MESH);
    m_modulateEdit.AttachItem(IDC_BULB_MODULATE_VS_ADD);
    m_transmitEdit.AttachItem(IDC_TRANSMISSION_SCALE);
    m_hRelectOnBalls = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ON_BALLS);
    m_posXEdit.AttachItem(902);
    m_posYEdit.AttachItem(903);
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
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
