// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/TriggerVisualsProperty.h"
#include <WindowsX.h>

TriggerVisualsProperty::TriggerVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTRIGGER_VISUALS, pvsel)
{
    m_shapeList.push_back("None"s);
    m_shapeList.push_back("Wire A"s);
    m_shapeList.push_back("Star"s);
    m_shapeList.push_back("Wire B"s);
    m_shapeList.push_back("Button"s);
    m_shapeList.push_back("Wire C"s);
    m_shapeList.push_back("Wire D"s);
    m_shapeList.push_back("Inder"s);

    m_wireThicknessEdit.SetDialog(this);
    m_starRadiusEdit.SetDialog(this);
    m_rotationEdit.SetDialog(this);
    m_animationSpeedEdit.SetDialog(this);
    m_posXEdit.SetDialog(this);
    m_posYEdit.SetDialog(this);
    m_shapeCombo.SetDialog(this);
    m_materialCombo.SetDialog(this);
    m_surfaceCombo.SetDialog(this);
}

void TriggerVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger * const trigger = (Trigger *)m_pvsel->ElementAt(i);

        if (dispid == DISPID_Shape || dispid == -1)
            PropertyDialog::UpdateComboBox(m_shapeList, m_shapeCombo, m_shapeList[(int)trigger->m_d.m_shape]);
        if (dispid == 902 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posXEdit, trigger->m_d.m_vCenter.x);
        if (dispid == 903 || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_posYEdit, trigger->m_d.m_vCenter.y);
        if (dispid == IDC_VISIBLE_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hVisibleCheck, trigger->m_d.m_visible);
        if (dispid == IDC_REFLECT_ENABLED_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hReflectionEnabledCheck, trigger->m_d.m_reflectionEnabled);
        if (dispid == IDC_STAR_THICKNESS_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_wireThicknessEdit, trigger->m_d.m_wireThickness);
        if (dispid == IDC_STAR_RADIUS_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_starRadiusEdit, trigger->m_d.m_radius);
        if (dispid == IDC_ROTATION_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotationEdit, trigger->m_d.m_rotation);
        if (dispid == IDC_RINGSPEED_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_animationSpeedEdit, trigger->m_d.m_animSpeed);
        if (dispid == IDC_SURFACE_COMBO || dispid == -1)
            PropertyDialog::UpdateSurfaceComboBox(trigger->GetPTable(), m_surfaceCombo, trigger->m_d.m_szSurface);

        UpdateBaseVisuals(trigger, &trigger->m_d, dispid);
        trigger->UpdateStatusBarInfo();
        //only show the first element on multi-select
        break;
    }
}

void TriggerVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemTrigger))
            continue;
        Trigger * const trigger = (Trigger *)m_pvsel->ElementAt(i);
        switch (dispid)
        {
            case DISPID_Shape:
                CHECK_UPDATE_ITEM(trigger->m_d.m_shape, (TriggerShape)(PropertyDialog::GetComboBoxIndex(m_shapeCombo, m_shapeList)), trigger);
                break;
            case 902:
                CHECK_UPDATE_ITEM(trigger->m_d.m_vCenter.x, PropertyDialog::GetFloatTextbox(m_posXEdit), trigger);
                break;
            case 903:
                CHECK_UPDATE_ITEM(trigger->m_d.m_vCenter.y, PropertyDialog::GetFloatTextbox(m_posYEdit), trigger);
                break;
            case IDC_SURFACE_COMBO:
                CHECK_UPDATE_COMBO_TEXT_STRING(trigger->m_d.m_szSurface, m_surfaceCombo, trigger);
                break;
            case IDC_STAR_THICKNESS_EDIT:
                CHECK_UPDATE_ITEM(trigger->m_d.m_wireThickness, PropertyDialog::GetFloatTextbox(m_wireThicknessEdit), trigger);
                break;
            case IDC_STAR_RADIUS_EDIT:
                CHECK_UPDATE_ITEM(trigger->m_d.m_radius, PropertyDialog::GetFloatTextbox(m_starRadiusEdit), trigger);
                break;
            case IDC_ROTATION_EDIT:
                CHECK_UPDATE_ITEM(trigger->m_d.m_rotation, PropertyDialog::GetFloatTextbox(m_rotationEdit), trigger);
                break;
            case IDC_RINGSPEED_EDIT:
                CHECK_UPDATE_ITEM(trigger->m_d.m_animSpeed, PropertyDialog::GetFloatTextbox(m_animationSpeedEdit), trigger);
                break;
            default:
                UpdateBaseProperties(trigger, &trigger->m_d, dispid);
                break;
        }
        trigger->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL TriggerVisualsProperty::OnInitDialog()
{
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_surfaceCombo.AttachItem(IDC_SURFACE_COMBO);
    m_shapeCombo.AttachItem(DISPID_Shape);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_posXEdit.AttachItem(902);
    m_posYEdit.AttachItem(903);
    m_wireThicknessEdit.AttachItem(IDC_STAR_THICKNESS_EDIT);
    m_starRadiusEdit.AttachItem(IDC_STAR_RADIUS_EDIT);
    m_rotationEdit.AttachItem(IDC_ROTATION_EDIT);
    m_animationSpeedEdit.AttachItem(IDC_RINGSPEED_EDIT);

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
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_surfaceCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_shapeCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_posXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_posYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_wireThicknessEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_starRadiusEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotationEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_animationSpeedEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

INT_PTR TriggerVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
