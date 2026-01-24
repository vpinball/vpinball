#include "core/stdafx.h"

#include "FlipperUIPart.h"

namespace VPX::EditorUI
{

FlipperUIPart::FlipperUIPart(Flipper* flipper)
   : m_flipper(flipper)
   , m_visible(flipper->m_d.m_visible)
{
}

FlipperUIPart::~FlipperUIPart() { m_flipper->m_d.m_visible = m_visible; }

FlipperUIPart::TransformMask FlipperUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_flipper->GetPTable()->GetSurfaceHeight(m_flipper->m_d.m_szSurface, m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y);
   transform = Matrix3D::MatrixTranslate(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, height);
   return static_cast<TransformMask>(TM_TransAny | TM_RotAny);
}

void FlipperUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_flipper->m_d.m_Center.x = pos.x;
   m_flipper->m_d.m_Center.y = pos.y;
}

void FlipperUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_flipper->m_d.m_visible;

   const bool isUIVisible = m_flipper->IsVisible(m_flipper);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_flipper->m_d.m_visible = true;
      ctx.DrawWireframe(m_flipper);
      //ctx.DrawHitObjects(m_flipper);
   }

   m_flipper->m_d.m_visible = isUIVisible && m_visible;
}

void FlipperUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Flipper"s, m_flipper);

   if (props.BeginSection("Visuals"s))
   {
      props.ImageCombo<Flipper>(
         m_flipper, "Image"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_szImage; }, //
         [](Flipper* flipper, const string& v) { flipper->m_d.m_szImage = v; });
      props.MaterialCombo<Flipper>(
         m_flipper, "Material"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_szMaterial; }, //
         [](Flipper* flipper, const string& v) { flipper->m_d.m_szMaterial = v; });
      props.MaterialCombo<Flipper>(
         m_flipper, "Rubber Material"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_szRubberMaterial; }, //
         [](Flipper* flipper, const string& v) { flipper->m_d.m_szRubberMaterial = v; });
      props.InputFloat<Flipper>(
         m_flipper, "Rubber Thickness"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rubberthickness; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rubberthickness = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "Rubber Offset"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rubberheight; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rubberheight = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "Rubber Width"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rubberwidth; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rubberwidth = v; }, PropertyPane::Unit::VPLength, 1);
      props.Checkbox<Flipper>(
         m_flipper, "Visible"s, //
         [this](const Flipper* flipper) { return flipper == m_flipper ? m_visible : flipper->m_d.m_visible; }, //
         [this](Flipper* flipper, bool v)
         {
            flipper->m_d.m_visible = v;
            (flipper == m_flipper ? m_visible : flipper->m_d.m_visible) = v;
         });
      props.Checkbox<Flipper>(
         m_flipper, "Enabled"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_enabled; }, //
         [](Flipper* flipper, bool v) { flipper->m_d.m_enabled = v; });
      props.Checkbox<Flipper>(
         m_flipper, "Reflection Enabled"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_reflectionEnabled; }, //
         [](Flipper* flipper, bool v) { flipper->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Geometry"s))
   {
      props.InputFloat<Flipper>(
         m_flipper, "Base Radius"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_BaseRadius; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_BaseRadius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "End Radius"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_EndRadius; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_EndRadius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "Length (min difficulty)"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_FlipperRadiusMax; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_FlipperRadiusMax = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "Length (max difficulty)"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_FlipperRadiusMin; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_FlipperRadiusMin = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "Height"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_height; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Flipper>(
         m_flipper, "Start Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_StartAngle; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_StartAngle = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Flipper>(
         m_flipper, "End Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_EndAngle; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_EndAngle = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Flipper>(
         m_flipper, "Position"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_Center; }, //
         [](Flipper* flipper, const Vertex2D& v) { flipper->Translate(Vertex2D(v.x - flipper->m_d.m_Center.x, v.y - flipper->m_d.m_Center.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Flipper>(
         m_flipper, "Surface"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_szSurface; }, //
         [](Flipper* flipper, const string& v) { flipper->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Flipper>(
         m_flipper, "Mass"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_mass; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_mass = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Flipper>(
         m_flipper, "Strength"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_strength; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_strength = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Flipper>(
         m_flipper, "Elasticity"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_elasticity; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "Elasticity Falloff"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_elasticityFalloff; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "Friction"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_friction; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "Return Strength"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_return; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_return = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "Coil Ramp Up"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_rampUp; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_rampUp = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "Scatter Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_scatter; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "EOS Torque"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_torqueDamping; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_torqueDamping = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Flipper>(
         m_flipper, "EOS Torque Angle"s, //
         [](const Flipper* flipper) { return flipper->m_d.m_torqueDampingAngle; }, //
         [](Flipper* flipper, float v) { flipper->m_d.m_torqueDampingAngle = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("User Override"s))
   {
      props.Combo<Flipper>(
         m_flipper, "Image"s, vector<string> { "Disable"s, "Set 1"s, "Set 2"s, "Set 3"s, "Set 4"s, "Set 5"s, "Set 6"s, "Set 7"s, "Set 8"s }, //
         [](const Flipper* flipper) { return flipper->m_d.m_OverridePhysics; }, //
         [](Flipper* flipper, int v) { flipper->m_d.m_OverridePhysics = v; });
      props.EndSection();
   }

   props.TimerSection<Flipper>(m_flipper, [](Flipper* obj) { return &(obj->m_d.m_tdr); });
}

}
