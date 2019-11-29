#include "stdafx.h"
#include "Properties/TablePhysicsProperty.h"
#include <WindowsX.h>

TablePhysicsProperty::TablePhysicsProperty(VectorProtected<ISelect> *pvsel) : BasePropertyDialog(IDD_PROPTABLE_PHYSICS, pvsel)
{
}

void TablePhysicsProperty::UpdateVisuals()
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    PropertyDialog::SetFloatTextbox(m_gravityConstantEdit, table->GetGravity());
    PropertyDialog::SetFloatTextbox(m_playfieldFrictionEdit, table->m_friction);
    PropertyDialog::SetFloatTextbox(m_playfieldElasticityEdit, table->m_elasticity);
    PropertyDialog::SetFloatTextbox(m_playfieldElasticityFalloffEdit, table->m_elasticityFalloff);
    PropertyDialog::SetFloatTextbox(m_playfieldScatterEdit, table->m_scatter);
    PropertyDialog::SetFloatTextbox(m_defaultScatterEdit, table->m_defaultScatter);
    PropertyDialog::SetFloatTextbox(m_nudgeTimeEdit, table->m_nudgeTime);
    PropertyDialog::SetIntTextbox(m_physicsLoopEdit, table->m_PhysicsMaxLoops);
    PropertyDialog::SetIntTextbox(m_mechPlungerAdjEdit, table->m_plungerNormalize);
    PropertyDialog::SetCheckboxState(m_hFilterMechanicalPlungerCheck, table->m_plungerFilter);
    PropertyDialog::SetFloatTextbox(m_tableWidthEdit, table->GetTableWidth());
    PropertyDialog::SetFloatTextbox(m_tableHeightEdit, table->GetHeight());
    PropertyDialog::SetFloatTextbox(m_topGlassHeightEdit, table->m_glassheight);
    PropertyDialog::SetFloatTextbox(m_tableFieldHeightEdit, table->m_tableheight);
    PropertyDialog::SetFloatTextbox(m_minSlopeEdit, table->m_angletiltMin);
    PropertyDialog::SetFloatTextbox(m_maxSlopeEdit, table->m_angletiltMax);
}

void TablePhysicsProperty::UpdateProperties(const int dispid)
{
    CComObject<PinTable> * const table = g_pvp->GetActiveTable();
    switch (dispid)
    {
        case IDC_GRAVITY_EDIT:
            PropertyDialog::StartUndo(table);
            table->SetGravity(PropertyDialog::GetFloatTextbox(m_gravityConstantEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_PLAYFIELD_FRICTION_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_friction = PropertyDialog::GetFloatTextbox(m_playfieldFrictionEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_PLAYFIELD_ELASTICITY_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_elasticity = PropertyDialog::GetFloatTextbox(m_playfieldElasticityEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_PLAYFIELD_FALLOFF_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_elasticityFalloff = PropertyDialog::GetFloatTextbox(m_playfieldElasticityFalloffEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_PLAYFIELD_SCATTER_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_scatter = PropertyDialog::GetFloatTextbox(m_playfieldScatterEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_DEFAULT_SCATTER_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_scatter = PropertyDialog::GetFloatTextbox(m_defaultScatterEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_NUDGE_TIME_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_nudgeTime = PropertyDialog::GetFloatTextbox(m_nudgeTimeEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_PHYSICS_MAC_LOOPS_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_PhysicsMaxLoops = PropertyDialog::GetIntTextbox(m_physicsLoopEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_MECH_PLUNGER_ADJ_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_plungerNormalize = PropertyDialog::GetIntTextbox(m_mechPlungerAdjEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_FILTER_MECH_PLUNGER_CHECK:
            PropertyDialog::StartUndo(table);
            table->m_plungerFilter = PropertyDialog::GetCheckboxState(m_hFilterMechanicalPlungerCheck);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLE_WIDTH_EDIT:
            PropertyDialog::StartUndo(table);
            table->SetTableWidth(PropertyDialog::GetFloatTextbox(m_tableWidthEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLE_HEIGHT_EDIT:
            PropertyDialog::StartUndo(table);
            table->SetHeight(PropertyDialog::GetFloatTextbox(m_tableHeightEdit));
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLE_GLASS_HEIGHT_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_glassheight = PropertyDialog::GetFloatTextbox(m_topGlassHeightEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_TABLE_FIELD_HEIGHT_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_tableheight = PropertyDialog::GetFloatTextbox(m_tableFieldHeightEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_MIN_DIFFICULTY_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_angletiltMin = PropertyDialog::GetFloatTextbox(m_minSlopeEdit);
            PropertyDialog::EndUndo(table);
            break;
        case IDC_MAX_DIFFICULTY_EDIT:
            PropertyDialog::StartUndo(table);
            table->m_angletiltMax = PropertyDialog::GetFloatTextbox(m_maxSlopeEdit);
            PropertyDialog::EndUndo(table);
            break;

        default:
    	break;
    }
}

BOOL TablePhysicsProperty::OnInitDialog()
{
    AttachItem(IDC_GRAVITY_EDIT, m_gravityConstantEdit);
    AttachItem(IDC_PLAYFIELD_FRICTION_EDIT, m_playfieldFrictionEdit);
    AttachItem(IDC_PLAYFIELD_ELASTICITY_EDIT, m_playfieldElasticityEdit);
    AttachItem(IDC_PLAYFIELD_FALLOFF_EDIT, m_playfieldElasticityFalloffEdit);
    AttachItem(IDC_PLAYFIELD_SCATTER_EDIT, m_playfieldScatterEdit);
    AttachItem(IDC_DEFAULT_SCATTER_EDIT, m_defaultScatterEdit);
    AttachItem(IDC_IMPORT_PHYSICS_BUTTON, m_importSetButton);
    AttachItem(IDC_EXPORT_PHYSICS_BUTTON, m_exportSetButton);
    AttachItem(IDC_NUDGE_TIME_EDIT, m_nudgeTimeEdit);
    AttachItem(IDC_PHYSICS_MAC_LOOPS_EDIT, m_physicsLoopEdit);
    AttachItem(IDC_MECH_PLUNGER_ADJ_EDIT, m_mechPlungerAdjEdit);
    m_hFilterMechanicalPlungerCheck = ::GetDlgItem(GetHwnd(), IDC_FILTER_MECH_PLUNGER_CHECK);
    AttachItem(IDC_TABLE_WIDTH_EDIT, m_tableWidthEdit);
    AttachItem(IDC_TABLE_HEIGHT_EDIT, m_tableHeightEdit);
    AttachItem(IDC_TABLE_GLASS_HEIGHT_EDIT, m_topGlassHeightEdit);
    AttachItem(IDC_TABLE_FIELD_HEIGHT_EDIT, m_tableFieldHeightEdit);
    AttachItem(IDC_MIN_DIFFICULTY_EDIT, m_minSlopeEdit);
    AttachItem(IDC_MAX_DIFFICULTY_EDIT, m_maxSlopeEdit);
    UpdateVisuals();
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
    return FALSE;
}
