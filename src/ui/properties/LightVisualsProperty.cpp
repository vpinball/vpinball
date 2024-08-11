// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/LightVisualsProperty.h"
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
    m_posZEdit.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_typeCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
    m_faderCombo.SetDialog(this);
}

void LightVisualsProperty::UpdateLightType(const int mode)
{
    GetDlgItem(IDC_IMAGE_LABEL).ShowWindow(SW_HIDE);
    m_imageCombo.ShowWindow(SW_HIDE);
    GetDlgItem(IDC_IMAGE_MODE).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_HALO_LABEL).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_HALO_EDIT).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_MODULATE_LABEL).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BULB_MODULATE_VS_ADD).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_TRANSMIT_LABEL).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_TRANSMISSION_SCALE).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_REFLECT_ENABLED_CHECK).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_DEPTHBIAS_LABEL).ShowWindow(SW_HIDE);
    GetDlgItem(IDC_DEPTH_BIAS).ShowWindow(SW_HIDE);
    if (mode == 0)
    {
    }
    else if (mode == 1)
    {
       GetDlgItem(IDC_IMAGE_LABEL).ShowWindow(SW_SHOWNORMAL);
       m_imageCombo.ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_IMAGE_MODE).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_DEPTHBIAS_LABEL).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_DEPTH_BIAS).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_REFLECT_ENABLED_CHECK).ShowWindow(SW_SHOWNORMAL);
    }
    else
    {
       GetDlgItem(IDC_HALO_LABEL).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_HALO_EDIT).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_MODULATE_LABEL).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_BULB_MODULATE_VS_ADD).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_TRANSMIT_LABEL).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_TRANSMISSION_SCALE).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_DEPTHBIAS_LABEL).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_DEPTH_BIAS).ShowWindow(SW_SHOWNORMAL);
       GetDlgItem(IDC_REFLECT_ENABLED_CHECK).ShowWindow(SW_SHOWNORMAL);
    }
}

void LightVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);

        if (dispid == IDC_INTENSITY || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_intensityEdit, light->m_d.m_intensity);
        if (dispid == IDC_FADE_SPEED_UP || dispid == -1)
            PropertyDialog::SetIntTextbox(m_fadeSpeedUpEdit, min(24*60*60*1000, (int)(0.5 + light->m_d.m_intensity / light->m_d.m_fadeSpeedUp)));
        if (dispid == IDC_FADE_SPEED_DOWN || dispid == -1)
            PropertyDialog::SetIntTextbox(m_fadeSpeedDownEdit, min(24*60*60*1000, (int)(0.5 + light->m_d.m_intensity / light->m_d.m_fadeSpeedDown)));
        if (dispid == IDC_COLOR_BUTTON1 || dispid == -1)
            m_colorButton1.SetColor(light->m_d.m_color);
        if (dispid == IDC_COLOR_BUTTON2 || dispid == -1)
            m_colorButton2.SetColor(light->m_d.m_color2);
        if (dispid == IDC_FALLOFF || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_falloffEdit, light->m_d.m_falloff);
        if (dispid == IDC_LIGHT_FALLOFF_POWER || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_falloffPowerEdit, light->m_d.m_falloff_power);

        if (dispid == IDC_LIGHT_FADER_COMBO || dispid == -1)
            m_faderCombo.SetCurSel(light->m_d.m_fader);

        if (dispid == IDC_LIGHT_TYPE_COMBO || dispid == -1)
        {
            if (!light->m_d.m_visible)
            {
               m_typeCombo.SetCurSel(0);
               UpdateLightType(0);
            }
            else if (!light->m_d.m_BulbLight)
            {
               m_typeCombo.SetCurSel(1);
               UpdateLightType(1);
            }
            else
            {
               m_typeCombo.SetCurSel(2);
               UpdateLightType(2);
            }
        }
        if (dispid == IDC_DEPTH_BIAS || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_depthBiasEdit, light->m_d.m_depthBias);

        if (dispid == IDC_HALO_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_haloHeightEdit, light->m_d.m_bulbHaloHeight);
        if (dispid == IDC_BULB_MODULATE_VS_ADD || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_modulateEdit, light->m_d.m_modulate_vs_add);
        if (dispid == IDC_TRANSMISSION_SCALE || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_transmitEdit, light->m_d.m_transmissionScale);

        if (dispid == IDC_IMAGE_MODE || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hPassThroughCheck, light->m_d.m_imageMode);

        if (dispid == IDC_SHOW_BULB_MESH || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hShowMeshCheck, light->m_d.m_showBulbMesh);
        if (dispid == IDC_STATIC_BULB_MESH || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hStaticMeshCheck, light->m_d.m_staticBulbMesh);
        if (dispid == IDC_SCALE_BULB_MESH || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_scaleMeshEdit, light->m_d.m_meshRadius);

        if (dispid == IDC_REFLECT_ON_BALLS || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hRelectOnBalls, light->m_d.m_showReflectionOnBall);
        if (dispid == IDC_SHADOWS || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hRaytracedBallShadows, light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS);

        if (dispid == IDC_POS_X || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, light->m_d.m_vCenter.x);
        if (dispid == IDC_POS_Y || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, light->m_d.m_vCenter.y);
        if (dispid == IDC_POS_Z || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posZEdit, light->m_d.m_height);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(light->GetPTable(), m_surfaceCombo, light->m_d.m_szSurface);

        UpdateBaseVisuals(light, &light->m_d, dispid);

        //only show the first element on multi-select
        break;
    }
}

void LightVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemLight))
            continue;
        Light * const light = (Light *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_FALLOFF:
                CHECK_UPDATE_ITEM(light->m_d.m_falloff, PropertyDialog::GetFloatTextbox(m_falloffEdit), light);
                break;
            case IDC_LIGHT_FALLOFF_POWER:
                CHECK_UPDATE_ITEM(light->m_d.m_falloff_power, PropertyDialog::GetFloatTextbox(m_falloffPowerEdit), light);
                break;
            case IDC_INTENSITY:
                 {
                     if (light->m_d.m_intensity != PropertyDialog::GetFloatTextbox(m_intensityEdit))
                     {
                         PropertyDialog::StartUndo(light);
                         light->m_d.m_intensity = PropertyDialog::GetFloatTextbox(m_intensityEdit);
                         int speedUpMs = PropertyDialog::GetIntTextbox(m_fadeSpeedUpEdit);
                         light->m_d.m_fadeSpeedUp = speedUpMs <= 0 ? (light->m_d.m_intensity * 1000.0f) : (light->m_d.m_intensity / (float)speedUpMs);
                         int speedDownMs = PropertyDialog::GetIntTextbox(m_fadeSpeedDownEdit);
                         light->m_d.m_fadeSpeedDown = speedDownMs <= 0 ? (light->m_d.m_intensity * 1000.0f) : (light->m_d.m_intensity / (float)speedDownMs);
                         PropertyDialog::EndUndo(light);
                     }
                 }
                break;
            case IDC_LIGHT_FADER_COMBO:
                CHECK_UPDATE_ITEM(light->m_d.m_fader, (Fader)m_faderCombo.GetCurSel(), light);
                break;
            case IDC_FADE_SPEED_UP:
            {
                int speedUpMs = PropertyDialog::GetIntTextbox(m_fadeSpeedUpEdit);
                CHECK_UPDATE_ITEM(light->m_d.m_fadeSpeedUp, speedUpMs <= 0 ? (light->m_d.m_intensity * 1000.0f) : (light->m_d.m_intensity / (float)speedUpMs), light);
                break;
            }
            case IDC_FADE_SPEED_DOWN:
            {
                int speedDownMs = PropertyDialog::GetIntTextbox(m_fadeSpeedDownEdit);
                CHECK_UPDATE_ITEM(light->m_d.m_fadeSpeedDown, speedDownMs <= 0 ? (light->m_d.m_intensity * 1000.0f) : (light->m_d.m_intensity / (float)speedDownMs), light);
                break;
            }

            case IDC_LIGHT_TYPE_COMBO:
                PropertyDialog::StartUndo(light);
                if (m_typeCombo.GetCurSel() == 0)
                {
                   light->m_d.m_visible = false;
                   UpdateLightType(0);
                }
                else if (m_typeCombo.GetCurSel() == 1)
                {
                   light->m_d.m_visible = true;
                   light->m_d.m_BulbLight = false;
                   UpdateLightType(1);
                }
                else
                {
                   light->m_d.m_visible = true;
                   light->m_d.m_BulbLight = true;
                   UpdateLightType(2);
                }
                PropertyDialog::EndUndo(light);
                break;
            case IDC_DEPTH_BIAS:
                CHECK_UPDATE_ITEM(light->m_d.m_depthBias, PropertyDialog::GetFloatTextbox(m_depthBiasEdit), light);
                break;

            case IDC_IMAGE_MODE:
                CHECK_UPDATE_ITEM(light->m_d.m_imageMode, PropertyDialog::GetCheckboxState(m_hPassThroughCheck), light);
                break;
            
            case IDC_HALO_EDIT:
                CHECK_UPDATE_ITEM(light->m_d.m_bulbHaloHeight, PropertyDialog::GetFloatTextbox(m_haloHeightEdit), light);
                break;
            case IDC_BULB_MODULATE_VS_ADD:
                CHECK_UPDATE_ITEM(light->m_d.m_modulate_vs_add, PropertyDialog::GetFloatTextbox(m_modulateEdit), light);
                break;
            case IDC_TRANSMISSION_SCALE:
                CHECK_UPDATE_ITEM(light->m_d.m_transmissionScale, PropertyDialog::GetFloatTextbox(m_transmitEdit), light);
                break;

            case IDC_SHOW_BULB_MESH:
                CHECK_UPDATE_ITEM(light->m_d.m_showBulbMesh, PropertyDialog::GetCheckboxState(m_hShowMeshCheck), light);
                break;
            case IDC_STATIC_BULB_MESH:
                CHECK_UPDATE_ITEM(light->m_d.m_staticBulbMesh, PropertyDialog::GetCheckboxState(m_hStaticMeshCheck), light);
                break;
            case IDC_SCALE_BULB_MESH:
                CHECK_UPDATE_ITEM(light->m_d.m_meshRadius, PropertyDialog::GetFloatTextbox(m_scaleMeshEdit), light);
                break;

            case IDC_REFLECT_ON_BALLS:
                CHECK_UPDATE_ITEM(light->m_d.m_showReflectionOnBall, PropertyDialog::GetCheckboxState(m_hRelectOnBalls), light);
                break;
            case IDC_SHADOWS:
                CHECK_UPDATE_ITEM(light->m_d.m_shadows, PropertyDialog::GetCheckboxState(m_hRaytracedBallShadows) ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE, light);
                break;

            case IDC_POS_X:
                CHECK_UPDATE_ITEM(light->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), light);
                break;
            case IDC_POS_Y:
                CHECK_UPDATE_ITEM(light->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), light);
                break;
            case IDC_POS_Z:
                CHECK_UPDATE_ITEM(light->m_d.m_height, PropertyDialog::GetFloatTextbox(m_posZEdit), light);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT_STRING(light->m_d.m_szSurface, m_surfaceCombo, light);
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
                   const Light* const firstLight = (Light*)m_pvsel->ElementAt(0);
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
                   const Light* const firstLight = (Light*)m_pvsel->ElementAt(0);
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
    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, RD_STRETCH_WIDTH); // Light setting group box
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_intensityEdit.AttachItem(IDC_INTENSITY);
    m_resizer.AddChild(m_intensityEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC24), CResizer::topleft, 0);
    m_faderCombo.AttachItem(IDC_LIGHT_FADER_COMBO);
    m_faderCombo.AddString("LED (None)");
    m_faderCombo.AddString("Linear");
    m_faderCombo.AddString("Incandescent");
    m_resizer.AddChild(m_faderCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_fadeSpeedUpEdit.AttachItem(IDC_FADE_SPEED_UP);
    m_resizer.AddChild(m_fadeSpeedUpEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_fadeSpeedDownEdit.AttachItem(IDC_FADE_SPEED_DOWN);
    m_resizer.AddChild(m_fadeSpeedDownEdit, CResizer::topright, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
    m_resizer.AddChild(m_colorButton1, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
    m_resizer.AddChild(m_colorButton2, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_falloffEdit.AttachItem(IDC_FALLOFF);
    m_resizer.AddChild(m_falloffEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_falloffPowerEdit.AttachItem(IDC_LIGHT_FALLOFF_POWER);
    m_resizer.AddChild(m_falloffPowerEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, RD_STRETCH_WIDTH); // Render Mode group box
    m_resizer.AddChild(GetDlgItem(IDC_STATIC23), CResizer::topleft, 0);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_typeCombo.AttachItem(IDC_LIGHT_TYPE_COMBO);
    m_typeCombo.AddString("Hidden");
    m_typeCombo.AddString("Classic");
    m_typeCombo.AddString("Halo");
    m_resizer.AddChild(m_typeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_HALO_LABEL), CResizer::topleft, 0);
    m_haloHeightEdit.AttachItem(IDC_HALO_EDIT);
    m_resizer.AddChild(m_haloHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_MODULATE_LABEL), CResizer::topleft, 0);
    m_modulateEdit.AttachItem(IDC_BULB_MODULATE_VS_ADD);
    m_resizer.AddChild(m_modulateEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_TRANSMIT_LABEL), CResizer::topleft, 0);
    m_transmitEdit.AttachItem(IDC_TRANSMISSION_SCALE);
    m_resizer.AddChild(m_transmitEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_IMAGE_LABEL), CResizer::topleft, 0);
    m_baseImageCombo = &m_imageCombo;
    m_imageCombo.AttachItem(DISPID_Image);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_hPassThroughCheck = ::GetDlgItem(GetHwnd(), IDC_IMAGE_MODE);
    m_resizer.AddChild(m_hPassThroughCheck, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_DEPTHBIAS_LABEL), CResizer::topleft, 0);
    m_depthBiasEdit.AttachItem(IDC_DEPTH_BIAS);
    m_resizer.AddChild(m_depthBiasEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC21), CResizer::topleft, RD_STRETCH_WIDTH); // Ball & reflections group box
    m_hRelectOnBalls = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ON_BALLS);
    m_resizer.AddChild(m_hRelectOnBalls, CResizer::topleft, RD_STRETCH_WIDTH);
    m_hRaytracedBallShadows = ::GetDlgItem(GetHwnd(), IDC_SHADOWS);
    m_resizer.AddChild(m_hRaytracedBallShadows, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC22), CResizer::topleft, RD_STRETCH_WIDTH); // Bulb group box
    m_hShowMeshCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_BULB_MESH);
    m_resizer.AddChild(m_hShowMeshCheck, CResizer::topleft, 0);
    m_hStaticMeshCheck = ::GetDlgItem(GetHwnd(), IDC_STATIC_BULB_MESH);
    m_resizer.AddChild(m_hStaticMeshCheck, CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, 0);
    m_scaleMeshEdit.AttachItem(IDC_SCALE_BULB_MESH);
    m_resizer.AddChild(m_scaleMeshEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, RD_STRETCH_WIDTH); // Position group box
    m_resizer.AddChild(GetDlgItem(IDC_STATIC17), CResizer::topleft, 0);
    m_posXEdit.AttachItem(IDC_POS_X);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC18), CResizer::topleft, 0);
    m_posYEdit.AttachItem(IDC_POS_Y);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC20), CResizer::topleft, 0);
    m_posZEdit.AttachItem(IDC_POS_Z);
    m_resizer.AddChild(m_posZEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC19), CResizer::topleft, 0);
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
    m_resizer.AddChild(m_surfaceCombo, CResizer::topleft, RD_STRETCH_WIDTH);

    UpdateVisuals();

    return TRUE;
}

INT_PTR LightVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
