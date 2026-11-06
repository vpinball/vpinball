#include "core/stdafx.h"

#include "FlipperUIPart.h"

#include "math/matrix.h"

namespace VPX::EditorUI
{

FlipperUIPart::FlipperUIPart(Flipper* flipper)
   : EditableUIPart(flipper, { &Data::m_visible })
{
}

FlipperUIPart::TransformMask FlipperUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_Center.x, m_part->m_d.m_Center.y);
   transform = Matrix3D::MatrixTranslate(m_part->m_d.m_Center.x, m_part->m_d.m_Center.y, height);
   return TM_TransAny;
}

void FlipperUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_Center.x = pos.x;
   m_part->m_d.m_Center.y = pos.y;
}

void FlipperUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
   //ctx.DrawHitObjects(m_part);
}

void FlipperUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Flipper"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.MaterialVisuals(m_part, "Material"s, &Data::m_szMaterial);
      props.MaterialVisuals(m_part, "Rubber Material"s, &Data::m_szRubberMaterial, "Image"s, &Data::m_szImage);
      props.InputFloat<Flipper>(
         m_part, "Rubber Thickness"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rubberthickness; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rubberthickness = v; }, PropertyPane::Unit::VPLength, 1, "Thickness of the rubber ring around the flipper");
      props.InputFloat<Flipper>(
         m_part, "Rubber Offset"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rubberheight; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rubberheight = v; }, PropertyPane::Unit::VPLength, 1, "Vertical offset of the rubber ring relative to the flipper's center");
      props.InputFloat<Flipper>(
         m_part, "Rubber Width"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rubberwidth; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rubberwidth = v; }, PropertyPane::Unit::VPLength, 1, "Width of the rubber ring around the flipper");
      props.Checkbox<Flipper>(
         m_part, "Visible"s, //
         [this](const Flipper* flipper) { return GetVisibility(flipper); }, //
         [this](Flipper* flipper, bool v) { SetVisibility(flipper, v); });
      props.Checkbox<Flipper>(
         m_part, "Enabled"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_enabled; }, //
         [](Flipper* flipper, bool v) { flipper->m_d.m_enabled = v; });
      props.Checkbox<Flipper>(
         m_part, "Reflection Enabled"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_reflectionEnabled; }, //
         [](Flipper* flipper, bool v) { flipper->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
      props.EndSection();
   }

   if (props.BeginSection("Geometry"s))
   {
      props.InputFloat<Flipper>(
         m_part, "Base Radius"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_BaseRadius; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_BaseRadius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_part, "End Radius"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_EndRadius; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_EndRadius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_part, "Length (min difficulty)"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_FlipperRadiusMax; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_FlipperRadiusMax = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_part, "Length (max difficulty)"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_FlipperRadiusMin; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_FlipperRadiusMin = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_part, "Height"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_height; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_part, "Start Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_StartAngle; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_StartAngle = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Flipper>(
         m_part, "End Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_EndAngle; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_EndAngle = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Flipper>(
         m_part, "Position"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_Center; }, //
         [](Flipper* flipper, const Vertex2D& v) { flipper->Translate(Vertex2D(v.x - flipper->m_d.m_Center.x, v.y - flipper->m_d.m_Center.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Flipper>(
         m_part, "Surface"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_szSurface; }, //
         [](Flipper* flipper, const string& v) { flipper->m_d.m_szSurface = v; }, "Surface this part rests on (defines its base height)");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Flipper>(
         m_part, "Mass"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_mass; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_mass = v; }, PropertyPane::Unit::None, 2, "Mass of the flipper, affecting how strongly it pushes the ball");
      props.InputFloat<Flipper>(
         m_part, "Strength"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_strength; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_strength = v; }, PropertyPane::Unit::None, 2, "Force of the flipper's swing, driving its speed and the kick given to the ball");
      props.InputFloat<Flipper>(
         m_part, "Elasticity"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_elasticity; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_part, "Elasticity Falloff"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_elasticityFalloff; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3, "Reduce elasticity for slower impacts (0 = constant elasticity)");
      props.InputFloat<Flipper>(
         m_part, "Friction"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_friction; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_friction = max(v, 0.f); }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_part, "Return Strength"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_return; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_return = v; }, PropertyPane::Unit::None, 3,
         "How strongly the flipper is pulled back to its rest position (0 = stays up until released)");
      props.InputFloat<Flipper>(
         m_part, "Coil Ramp Up"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rampUp; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rampUp = v; }, PropertyPane::Unit::None, 3,
         "Time for the solenoid to reach full strength, softening the start of the stroke (0 = instant)");
      props.InputFloat<Flipper>(
         m_part, "Scatter Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_scatter; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3, "Random angle added to the ball's bounce direction");
      props.InputFloat<Flipper>(
         m_part, "EOS Torque"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_torqueDamping; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_torqueDamping = v; }, PropertyPane::Unit::None, 3,
         "Force applied once the flipper passes the EOS angle (models the hold coil after the end-of-stroke switch)");
      props.InputFloat<Flipper>(
         m_part, "EOS Torque Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_torqueDampingAngle; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_torqueDampingAngle = v; }, PropertyPane::Unit::Degree, 1,
         "Angle at which the end-of-stroke switch engages and the torque drops to the EOS value");
      props.EndSection();
   }

   if (props.BeginSection("User Override"s))
   {
      props.Combo<Flipper>(
         m_part, "Physics"s, vector<string> { "Disable"s, "Set 1"s, "Set 2"s, "Set 3"s, "Set 4"s, "Set 5"s, "Set 6"s, "Set 7"s, "Set 8"s }, //
         [](const Flipper* flipper) { return flipper->m_d.m_OverridePhysics; }, //
         [](Flipper* flipper, int v) { flipper->m_d.m_OverridePhysics = v; }, "Apply one of the table's global physics sets to this flipper");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
