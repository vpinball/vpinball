// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/RubberVisualsProperty.h"
#include <WindowsX.h>

RubberVisualsProperty::RubberVisualsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPRUBBER_VISUALS, pvsel)
{
    m_heightEdit.SetDialog(this);
    m_thicknessEdit.SetDialog(this);
    m_rotXEdit.SetDialog(this);
    m_rotYEdit.SetDialog(this);
    m_rotZEdit.SetDialog(this);
    m_imageCombo.SetDialog(this);
    m_materialCombo.SetDialog(this);
}

void RubberVisualsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);

        if (dispid == IDC_STATIC_RENDERING_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hStaticRenderingCheck, rubber->m_d.m_staticRendering);
        if (dispid == IDC_SHOW_IN_EDITOR_CHECK || dispid == -1)
            PropertyDialog::SetCheckboxState(m_hShowInEditorCheck, rubber->m_d.m_showInEditor);
        if (dispid == IDC_RUBBER_HEIGHT_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_heightEdit, rubber->m_d.m_height);
        if (dispid == IDC_RUBBER_THICKNESS_EDIT || dispid == -1)
            PropertyDialog::SetIntTextbox(m_thicknessEdit, rubber->m_d.m_thickness);
        if (dispid == IDC_RUBBER_ROTX_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotXEdit, rubber->m_d.m_rotX);
        if (dispid == IDC_RUBBER_ROTY_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotYEdit, rubber->m_d.m_rotY);
        if (dispid == IDC_RUBBER_ROTZ_EDIT || dispid == -1)
            PropertyDialog::SetFloatTextbox(m_rotZEdit, rubber->m_d.m_rotZ);

        UpdateBaseVisuals(rubber, &rubber->m_d, dispid);
        //only show the first element on multi-select
        break;
    }
}

void RubberVisualsProperty::UpdateProperties(const int dispid)
{
    for (int i = 0; i < m_pvsel->size(); i++)
    {
        if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemRubber))
            continue;
        Rubber *const rubber = (Rubber *)m_pvsel->ElementAt(i);

        switch (dispid)
        {
            case IDC_STATIC_RENDERING_CHECK:
                CHECK_UPDATE_ITEM(rubber->m_d.m_staticRendering, PropertyDialog::GetCheckboxState(m_hStaticRenderingCheck), rubber);
                break;
            case IDC_SHOW_IN_EDITOR_CHECK:
                CHECK_UPDATE_ITEM(rubber->m_d.m_showInEditor, PropertyDialog::GetCheckboxState(m_hShowInEditorCheck), rubber);
                break;
            case IDC_RUBBER_HEIGHT_EDIT:
                CHECK_UPDATE_ITEM(rubber->m_d.m_height, PropertyDialog::GetFloatTextbox(m_heightEdit), rubber);
                break;
            case IDC_RUBBER_THICKNESS_EDIT:
                CHECK_UPDATE_ITEM(rubber->m_d.m_thickness, PropertyDialog::GetIntTextbox(m_thicknessEdit), rubber);
                break;
            case IDC_RUBBER_ROTX_EDIT:
                CHECK_UPDATE_ITEM(rubber->m_d.m_rotX, PropertyDialog::GetFloatTextbox(m_rotXEdit), rubber);
                break;
            case IDC_RUBBER_ROTY_EDIT:
                CHECK_UPDATE_ITEM(rubber->m_d.m_rotY, PropertyDialog::GetFloatTextbox(m_rotYEdit), rubber);
                break;
            case IDC_RUBBER_ROTZ_EDIT:
                CHECK_UPDATE_ITEM(rubber->m_d.m_rotZ, PropertyDialog::GetFloatTextbox(m_rotZEdit), rubber);
                break;
            default:
                UpdateBaseProperties(rubber, &rubber->m_d, dispid);
                break;
        }
        rubber->UpdateStatusBarInfo();
    }
    UpdateVisuals(dispid);
}

BOOL RubberVisualsProperty::OnInitDialog()
{
    m_imageCombo.AttachItem(DISPID_Image);
    m_baseImageCombo = &m_imageCombo;
    m_materialCombo.AttachItem(IDC_MATERIAL_COMBO);
    m_baseMaterialCombo = &m_materialCombo;
    m_hStaticRenderingCheck = ::GetDlgItem(GetHwnd(), IDC_STATIC_RENDERING_CHECK);
    m_hVisibleCheck = ::GetDlgItem(GetHwnd(), IDC_VISIBLE_CHECK);
    m_hReflectionEnabledCheck = ::GetDlgItem(GetHwnd(), IDC_REFLECT_ENABLED_CHECK);
    m_heightEdit.AttachItem(IDC_RUBBER_HEIGHT_EDIT);
    m_thicknessEdit.AttachItem(IDC_RUBBER_THICKNESS_EDIT);
    m_rotXEdit.AttachItem(IDC_RUBBER_ROTX_EDIT);
    m_rotYEdit.AttachItem(IDC_RUBBER_ROTY_EDIT);
    m_rotZEdit.AttachItem(IDC_RUBBER_ROTZ_EDIT);
    m_hShowInEditorCheck = ::GetDlgItem(GetHwnd(), IDC_SHOW_IN_EDITOR_CHECK);
    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_imageCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_materialCombo, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hStaticRenderingCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hVisibleCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_hReflectionEnabledCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_heightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_thicknessEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotXEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotYEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_rotZEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_hShowInEditorCheck, CResizer::topleft, 0);

    return TRUE;
}

INT_PTR RubberVisualsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
