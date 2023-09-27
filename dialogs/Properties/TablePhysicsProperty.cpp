#include "stdafx.h"
#include "Properties/TablePhysicsProperty.h"
#include <WindowsX.h>

TablePhysicsProperty::TablePhysicsProperty(const VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_PHYSICS, pvsel)
{
    m_gravityConstantEdit.SetDialog(this);
    m_playfieldFrictionEdit.SetDialog(this);
    m_playfieldElasticityEdit.SetDialog(this);
    m_playfieldElasticityFalloffEdit.SetDialog(this);
    m_playfieldScatterEdit.SetDialog(this);
    m_defaultScatterEdit.SetDialog(this);
    m_nudgeTimeEdit.SetDialog(this);
    m_physicsLoopEdit.SetDialog(this);
    m_mechPlungerAdjEdit.SetDialog(this);
    m_tableWidthEdit.SetDialog(this);
    m_tableHeightEdit.SetDialog(this);
    m_bottomGlassHeightEdit.SetDialog(this);
    m_topGlassHeightEdit.SetDialog(this);
    m_tableFieldHeightEdit.SetDialog(this);
    m_minSlopeEdit.SetDialog(this);
    m_maxSlopeEdit.SetDialog(this);
}

void TablePhysicsProperty::UpdateVisuals(const int dispid/*=-1*/)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    if (dispid == IDC_GRAVITY_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_gravityConstantEdit, table->GetGravity());
    if (dispid == IDC_PLAYFIELD_FRICTION_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_playfieldFrictionEdit, table->m_friction);
    if (dispid == IDC_PLAYFIELD_ELASTICITY_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_playfieldElasticityEdit, table->m_elasticity);
    if (dispid == IDC_PLAYFIELD_FALLOFF_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_playfieldElasticityFalloffEdit, table->m_elasticityFalloff);
    if (dispid == IDC_PLAYFIELD_SCATTER_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_playfieldScatterEdit, table->m_scatter);
    if (dispid == IDC_DEFAULT_SCATTER_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_defaultScatterEdit, table->m_defaultScatter);
    if (dispid == IDC_NUDGE_TIME_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_nudgeTimeEdit, table->m_nudgeTime);
    if (dispid == IDC_PHYSICS_MAX_LOOPS_EDIT || dispid == -1)
        PropertyDialog::SetIntTextbox(m_physicsLoopEdit, table->m_PhysicsMaxLoops);
    if (dispid == IDC_MECH_PLUNGER_ADJ_EDIT || dispid == -1)
        PropertyDialog::SetIntTextbox(m_mechPlungerAdjEdit, table->m_plungerNormalize);
    if (dispid == IDC_FILTER_MECH_PLUNGER_CHECK || dispid == -1)
        PropertyDialog::SetCheckboxState(m_hFilterMechanicalPlungerCheck, table->m_plungerFilter);
    if (dispid == IDC_TABLE_WIDTH_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_tableWidthEdit, table->GetTableWidth());
    if (dispid == IDC_TABLE_HEIGHT_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_tableHeightEdit, table->GetHeight());
    if (dispid == IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_bottomGlassHeightEdit, table->m_glassBottomHeight);
    if (dispid == IDC_TABLE_GLASS_TOP_HEIGHT_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_topGlassHeightEdit, table->m_glassTopHeight);
    if (dispid == IDC_TABLE_FIELD_HEIGHT_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_tableFieldHeightEdit, table->m_tableheight);
    if (dispid == IDC_MIN_DIFFICULTY_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_minSlopeEdit, table->m_angletiltMin);
    if (dispid == IDC_MAX_DIFFICULTY_EDIT || dispid == -1)
        PropertyDialog::SetFloatTextbox(m_maxSlopeEdit, table->m_angletiltMax);
}

void TablePhysicsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    if (table == nullptr)
        return;

    switch (dispid)
    {
        case IDC_GRAVITY_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetGravity, table->GetGravity, PropertyDialog::GetFloatTextbox, m_gravityConstantEdit, table);
            break;
        case IDC_PLAYFIELD_FRICTION_EDIT:
            CHECK_UPDATE_ITEM(table->m_friction, PropertyDialog::GetFloatTextbox(m_playfieldFrictionEdit), table);
            break;
        case IDC_PLAYFIELD_ELASTICITY_EDIT:
            CHECK_UPDATE_ITEM(table->m_elasticity, PropertyDialog::GetFloatTextbox(m_playfieldElasticityEdit), table);
            break;
        case IDC_PLAYFIELD_FALLOFF_EDIT:
            CHECK_UPDATE_ITEM(table->m_elasticityFalloff, PropertyDialog::GetFloatTextbox(m_playfieldElasticityFalloffEdit), table);
            break;
        case IDC_PLAYFIELD_SCATTER_EDIT:
            CHECK_UPDATE_ITEM(table->m_scatter, PropertyDialog::GetFloatTextbox(m_playfieldScatterEdit), table);
            break;
        case IDC_DEFAULT_SCATTER_EDIT:
            CHECK_UPDATE_ITEM(table->m_defaultScatter, PropertyDialog::GetFloatTextbox(m_defaultScatterEdit), table);
            break;
        case IDC_NUDGE_TIME_EDIT:
            CHECK_UPDATE_ITEM(table->m_nudgeTime, PropertyDialog::GetFloatTextbox(m_nudgeTimeEdit), table);
            break;
        case IDC_PHYSICS_MAX_LOOPS_EDIT:
            CHECK_UPDATE_ITEM(table->m_PhysicsMaxLoops, PropertyDialog::GetIntTextbox(m_physicsLoopEdit), table);
            break;
        case IDC_MECH_PLUNGER_ADJ_EDIT:
            CHECK_UPDATE_ITEM(table->m_plungerNormalize, PropertyDialog::GetIntTextbox(m_mechPlungerAdjEdit), table);
            break;
        case IDC_FILTER_MECH_PLUNGER_CHECK:
            CHECK_UPDATE_ITEM(table->m_plungerFilter, PropertyDialog::GetCheckboxState(m_hFilterMechanicalPlungerCheck), table);
            break;
        case IDC_TABLE_WIDTH_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetTableWidth, table->GetTableWidth, PropertyDialog::GetFloatTextbox, m_tableWidthEdit, table);
            break;
        case IDC_TABLE_HEIGHT_EDIT:
            CHECK_UPDATE_VALUE_SETTER(table->SetHeight, table->GetHeight, PropertyDialog::GetFloatTextbox, m_tableHeightEdit, table);
            break;
        case IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT:
            CHECK_UPDATE_ITEM(table->m_glassBottomHeight, PropertyDialog::GetFloatTextbox(m_bottomGlassHeightEdit), table);
            break;
        case IDC_TABLE_GLASS_TOP_HEIGHT_EDIT:
            CHECK_UPDATE_ITEM(table->m_glassTopHeight, PropertyDialog::GetFloatTextbox(m_topGlassHeightEdit), table);
            break;
        case IDC_TABLE_FIELD_HEIGHT_EDIT:
            CHECK_UPDATE_ITEM(table->m_tableheight, PropertyDialog::GetFloatTextbox(m_tableFieldHeightEdit), table);
            break;
        case IDC_MIN_DIFFICULTY_EDIT:
            CHECK_UPDATE_ITEM(table->m_angletiltMin, PropertyDialog::GetFloatTextbox(m_minSlopeEdit), table);
            break;
        case IDC_MAX_DIFFICULTY_EDIT:
            CHECK_UPDATE_ITEM(table->m_angletiltMax, PropertyDialog::GetFloatTextbox(m_maxSlopeEdit), table);
            break;
        default:
        break;
    }
    UpdateVisuals(dispid);
}

BOOL TablePhysicsProperty::OnInitDialog()
{
    m_hFilterMechanicalPlungerCheck = ::GetDlgItem(GetHwnd(), IDC_FILTER_MECH_PLUNGER_CHECK);
    AttachItem(IDC_IMPORT_PHYSICS_BUTTON, m_importSetButton);
    AttachItem(IDC_EXPORT_PHYSICS_BUTTON, m_exportSetButton);

    m_gravityConstantEdit.AttachItem(IDC_GRAVITY_EDIT);
    m_playfieldFrictionEdit.AttachItem(IDC_PLAYFIELD_FRICTION_EDIT);
    m_playfieldElasticityEdit.AttachItem(IDC_PLAYFIELD_ELASTICITY_EDIT);
    m_playfieldElasticityFalloffEdit.AttachItem(IDC_PLAYFIELD_FALLOFF_EDIT);
    m_playfieldScatterEdit.AttachItem(IDC_PLAYFIELD_SCATTER_EDIT);
    m_defaultScatterEdit.AttachItem(IDC_DEFAULT_SCATTER_EDIT);
    m_nudgeTimeEdit.AttachItem(IDC_NUDGE_TIME_EDIT);
    m_physicsLoopEdit.AttachItem(IDC_PHYSICS_MAX_LOOPS_EDIT);
    m_mechPlungerAdjEdit.AttachItem(IDC_MECH_PLUNGER_ADJ_EDIT);
    m_tableWidthEdit.AttachItem(IDC_TABLE_WIDTH_EDIT);
    m_tableHeightEdit.AttachItem(IDC_TABLE_HEIGHT_EDIT);
    m_bottomGlassHeightEdit.AttachItem(IDC_TABLE_GLASS_BOTTOM_HEIGHT_EDIT);
    m_topGlassHeightEdit.AttachItem(IDC_TABLE_GLASS_TOP_HEIGHT_EDIT);
    m_tableFieldHeightEdit.AttachItem(IDC_TABLE_FIELD_HEIGHT_EDIT);
    m_minSlopeEdit.AttachItem(IDC_MIN_DIFFICULTY_EDIT);
    m_maxSlopeEdit.AttachItem(IDC_MAX_DIFFICULTY_EDIT);

    UpdateVisuals();

    m_resizer.Initialize(*this, CRect(0, 0, 0, 0));
    m_resizer.AddChild(GetDlgItem(IDC_STATIC1), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC2), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC3), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC4), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC5), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC6), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC7), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC8), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC9), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC10), CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC11), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC12), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC13), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC16), CResizer::topleft, 0);
    m_resizer.AddChild(m_hFilterMechanicalPlungerCheck, CResizer::topleft, 0);
    m_resizer.AddChild(m_importSetButton, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_exportSetButton, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_gravityConstantEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_playfieldFrictionEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_playfieldElasticityFalloffEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_playfieldScatterEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_defaultScatterEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_nudgeTimeEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_physicsLoopEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_mechPlungerAdjEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tableWidthEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tableHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_bottomGlassHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_topGlassHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tableFieldHeightEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_minSlopeEdit, CResizer::topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_maxSlopeEdit, CResizer::topleft, RD_STRETCH_WIDTH);

    return TRUE;
}

BOOL TablePhysicsProperty::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int dispID = LOWORD(wParam);

    switch (dispID)
    {
        case IDC_IMPORT_PHYSICS_BUTTON:
        {
            CComObject<PinTable> * const table = g_pvp->GetActiveTable();
            table->ImportPhysics();
            return TRUE;
        }
        case IDC_EXPORT_PHYSICS_BUTTON:
        {
            CComObject<PinTable> * const table = g_pvp->GetActiveTable();
            table->ExportPhysics();
            return TRUE;
        }
        default:
            break;
    }
    return BasePropertyDialog::OnCommand(wParam,lParam);
}

INT_PTR TablePhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
