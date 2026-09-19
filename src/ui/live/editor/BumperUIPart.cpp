#include "core/stdafx.h"

#include "BumperUIPart.h"

namespace VPX::EditorUI
{

BumperUIPart::BumperUIPart(Bumper* bumper)
   : EditableUIPart(bumper, { &Data::m_baseVisible, &Data::m_capVisible, &Data::m_skirtVisible, &Data::m_ringVisible })
{
}

BumperUIPart::TransformMask BumperUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void BumperUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_vCenter.x = pos.x;
   m_part->m_d.m_vCenter.y = pos.y;
}

void BumperUIPart::RenderOverlay(const EditorRenderContext& ctx) { ctx.DrawWireframe(m_part); }

void BumperUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Bumper"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.InputFloat<Bumper>(
         m_part, "Radius"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_radius; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Bumper>(
         m_part, "Height Scale"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_heightScale; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_heightScale = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Bumper>(
         m_part, "Orientation"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_orientation; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_orientation = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Bumper>(
         m_part, "Ring Speed"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_ringSpeed; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_ringSpeed = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Bumper>(
         m_part, "Ring Drop"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_ringDropOffset; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_ringDropOffset = v; }, PropertyPane::Unit::None, 1);

      props.Checkbox<Bumper>(
         m_part, "Cap Visible"s, //
         [this](const Bumper* bumper) { return GetVisibility(bumper, &Data::m_capVisible); }, //
         [this](Bumper* bumper, bool v) { SetVisibility(bumper, v, &Data::m_capVisible); });
      props.MaterialCombo<Bumper>(
         m_part, "Cap Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szCapMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szCapMaterial = v; });

      props.Checkbox<Bumper>(
         m_part, "Base Visible"s, //
         [this](const Bumper* bumper) { return GetVisibility(bumper, &Data::m_baseVisible); }, //
         [this](Bumper* bumper, bool v) { SetVisibility(bumper, v, &Data::m_baseVisible); });
      props.MaterialCombo<Bumper>(
         m_part, "Base Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szBaseMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szBaseMaterial = v; });

      props.Checkbox<Bumper>(
         m_part, "Skirt Visible"s, //
         [this](const Bumper* bumper) { return GetVisibility(bumper, &Data::m_skirtVisible); }, //
         [this](Bumper* bumper, bool v) { SetVisibility(bumper, v, &Data::m_skirtVisible); });
      props.MaterialCombo<Bumper>(
         m_part, "Skirt Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szSkirtMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szSkirtMaterial = v; });

      props.Checkbox<Bumper>(
         m_part, "Ring Visible"s, //
         [this](const Bumper* bumper) { return GetVisibility(bumper, &Data::m_ringVisible); }, //
         [this](Bumper* bumper, bool v) { SetVisibility(bumper, v, &Data::m_ringVisible); });
      props.MaterialCombo<Bumper>(
         m_part, "Ring Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szRingMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szRingMaterial = v; });

      props.Checkbox<Bumper>(
         m_part, "Reflection Enabled"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_reflectionEnabled; }, //
         [](Bumper* bumper, bool v) { bumper->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Bumper>(
         m_part, "Position"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_vCenter; }, //
         [](Bumper* bumper, const Vertex2D& v) { bumper->Translate(Vertex2D(v.x - bumper->m_d.m_vCenter.x, v.y - bumper->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Bumper>(
         m_part, "Surface"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szSurface; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Bumper>(
         m_part, "Has Hit Event"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_hitEvent; }, //
         [](Bumper* bumper, bool v) { bumper->m_d.m_hitEvent = v; });
      props.InputFloat<Bumper>(
         m_part, "Force"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_force; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_force = v; }, PropertyPane::Unit::VPLength, 3);
      props.InputFloat<Bumper>(
         m_part, "Hit Threshold"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_threshold; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_threshold = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Bumper>(
         m_part, "Scatter Angle"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_scatter; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Bumper>(
         m_part, "Collidable"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_collidable; }, //
         [](Bumper* bumper, bool v) { bumper->m_d.m_collidable = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
