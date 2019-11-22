#include "stdafx.h"
#include "Properties/RubberVisualsProperty.h"
#include <WindowsX.h>

RubberVisualsProperty::RubberVisualsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRUBBER_VISUALS, pvsel)
{
}

void RubberVisualsProperty::UpdateVisuals()
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);

        PropertyDialog::SetCheckboxState(m_hStaticRenderingCheck, rubber->m_d.m_staticRendering);
        PropertyDialog::SetCheckboxState(m_hShowInEditorCheck, rubber->m_d.m_showInEditor);
        PropertyDialog::SetFloatTextbox(m_heightEdit, rubber->m_d.m_height);
        PropertyDialog::SetIntTextbox(m_thicknessEdit, rubber->m_d.m_thickness);
        PropertyDialog::SetFloatTextbox(m_rotXEdit, rubber->m_d.m_rotX);
        PropertyDialog::SetFloatTextbox(m_rotYEdit, rubber->m_d.m_rotY);
        PropertyDialog::SetFloatTextbox(m_rotZEdit, rubber->m_d.m_rotZ);
        UpdateBaseVisuals(rubber, &rubber->m_d);
        //only show the first element on multi-select
        break;
    }
}

void RubberVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->Size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == NULL) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_STATIC_RENDERING_CHECK:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_staticRendering = PropertyDialog::GetCheckboxState(m_hStaticRenderingCheck);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_SHOW_IN_EDITOR_CHECK:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_showInEditor = PropertyDialog::GetCheckboxState(m_hShowInEditorCheck);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_RUBBER_HEIGHT_EDIT:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_height= PropertyDialog::GetFloatTextbox(m_heightEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_RUBBER_THICKNESS_EDIT:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_thickness = PropertyDialog::GetIntTextbox(m_thicknessEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_RUBBER_ROTX_EDIT:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_rotX = PropertyDialog::GetFloatTextbox(m_rotXEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_RUBBER_ROTY_EDIT:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_rotY = PropertyDialog::GetFloatTextbox(m_rotYEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            case IDC_RUBBER_ROTZ_EDIT:
                PropertyDialog::StartUndo(rubber);
                rubber->m_d.m_rotZ = PropertyDialog::GetFloatTextbox(m_rotZEdit);
                PropertyDialog::EndUndo(rubber);
                break;
            default:
                UpdateBaseProperties(rubber, &rubber->m_d, dispid);
                break;
        }
    }
    UpdateVisuals();
}

BOOL RubberVisualsProperty::OnInitDialog()
{
    AttachItem(DISPID_Image, m_imageCombo);
    m_baseImageCombo = &m_imageCombo;
    AttachItem(IDC_MATERIAL_COMBO, m_materialCombo);
    m_baseMaterialCombo = &m_materialCombo;
    m_hStaticRenderingCheck = ::GetDlgItem(GetHwnd(), IDC_STATIC_RENDERING_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    AttachItem(IDC_RUBBER_HEIGHT_EDIT, m_heightEdit);
    AttachItem(IDC_RUBBER_THICKNESS_EDIT, m_thicknessEdit);
    AttachItem(IDC_RUBBER_ROTX_EDIT, m_rotXEdit);
    AttachItem(IDC_RUBBER_ROTY_EDIT, m_rotYEdit);
    AttachItem(IDC_RUBBER_ROTZ_EDIT, m_rotZEdit);
    m_hShowInEditorCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_IN_EDITOR_CHECK);
    UpdateVisuals();
    return TRUE;
}
