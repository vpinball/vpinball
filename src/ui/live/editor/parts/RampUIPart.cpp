#include "core/stdafx.h"

#include "RampUIPart.h"

#include "math/matrix.h"

namespace VPX::EditorUI
{

RampUIPart::RampUIPart(Ramp* ramp)
   : EditableUIPart(ramp, { &Data::m_visible })
{
}

RampUIPart::TransformMask RampUIPart::GetTransform(Matrix3D& transform) { return GetCurveTransform(transform, 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop)); }

void RampUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   SetCurveTransform(pos, scale, rot);
   const float pz = 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop);
   m_part->m_d.m_heightbottom += pos.z - pz;
   m_part->m_d.m_heighttop += pos.z - pz;
}

void RampUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void RampUIPart::InsertPointOnCurve(const Vertex2D& pos)
{
   m_part->AddPoint(pos, true); // Ramp points are usually always smooth
}

void RampUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Ramp"s, m_part);

   UpdateCurveSection(props);

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Ramp>(
         m_part, "Type"s, vector<string> { "Flat"s, "4 Wires"s, "2 Wires"s, "3 Wires Left"s, "3 Wires Right"s, "1 Wire"s }, //
         [](const Ramp* ramp) { return static_cast<int>(ramp->m_d.m_type); }, //
         [](Ramp* ramp, int v) { ramp->m_d.m_type = static_cast<RampType>(v); });
      props.MaterialVisuals(m_part, "Material"s, &Data::m_szMaterial, "Image"s, &Data::m_szImage);
      props.Combo<Ramp>(
         m_part, "Image Mode"s, vector { "World"s, "Wrap"s }, //
         [](const Ramp* ramp) { return static_cast<int>(ramp->m_d.m_imagealignment); }, //
         [](Ramp* ramp, int v) { ramp->m_d.m_imagealignment = static_cast<RampImageAlignment>(v); }, "World maps the image in table space; Wrap stretches it along the part's shape");
      props.Checkbox<Ramp>(
         m_part, "Apply Image to Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_imageWalls; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_imageWalls = v; }, "Also stretch the image over the ramp's side walls");
      props.InputFloat<Ramp>(
         m_part, "Depth Bias"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_depthBias; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0, "Depth offset applied at render time to fix flickering between overlapping parts");
      props.Checkbox<Ramp>(
         m_part, "Visible"s, //
         [this](const Ramp* ramp) { return GetVisibility(ramp); }, //
         [this](Ramp* ramp, bool v) { SetVisibility(ramp, v); });
      props.Checkbox<Ramp>(
         m_part, "Reflection Enabled"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_reflectionEnabled; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
      props.EndSection();
   }

   if (props.BeginSection("Geometry"s))
   {
      props.InputFloat<Ramp>(
         m_part, "Top Height"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_heighttop; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_heighttop = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Bottom Height"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_heightbottom; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_heightbottom = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Top Width"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_widthtop; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_widthtop = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Bottom Width"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_widthbottom; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_widthbottom = v; }, PropertyPane::Unit::VPLength, 1);

      props.Separator("Visible Walls"s);
      props.InputFloat<Ramp>(
         m_part, "Left Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_leftwallheightvisible; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_leftwallheightvisible = v; }, PropertyPane::Unit::VPLength, 1, "Height of the wall rendered on the left edge of the ramp (render only)");
      props.InputFloat<Ramp>(
         m_part, "Right Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_rightwallheightvisible; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_rightwallheightvisible = v; }, PropertyPane::Unit::VPLength, 1, "Height of the wall rendered on the right edge of the ramp (render only)");

      props.Separator("Wire Ramp"s);
      props.InputFloat<Ramp>(
         m_part, "Diameter"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDiameter; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDiameter = v; }, PropertyPane::Unit::VPLength, 1, "Thickness of the ramp's wires");
      props.InputFloat<Ramp>(
         m_part, "Distance X"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDistanceX; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDistanceX = v; }, PropertyPane::Unit::VPLength, 1, "Horizontal spacing between the wires");
      props.InputFloat<Ramp>(
         m_part, "Distance Y"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDistanceY; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDistanceY = v; }, PropertyPane::Unit::VPLength, 1, "Vertical spacing between the wires");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Ramp>(
         m_part, "Has Hit Event"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_hitEvent; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_hitEvent = v; }, "Fire a Hit event to the script when the ball hits this part");
      props.InputFloat<Ramp>(
         m_part, "Hit Threshold"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_threshold; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_threshold = v; }, PropertyPane::Unit::None, 3, "Minimum ball speed needed to trigger this part's hit event");
      props.InputFloat<Ramp>(
         m_part, "Left Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_leftwallheight; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_leftwallheight = v; }, PropertyPane::Unit::VPLength, 1, "Height of the physical wall on the left edge (0 = no wall)");
      props.InputFloat<Ramp>(
         m_part, "Right Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_rightwallheight; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_rightwallheight = v; }, PropertyPane::Unit::VPLength, 1, "Height of the physical wall on the right edge (0 = no wall)");
      props.MaterialCombo<Ramp>(
         m_part, "Physics Material"s, //
         [](const Ramp* surf) { return surf->m_d.m_szPhysicsMaterial; }, //
         [](Ramp* surf, const string& v) { surf->m_d.m_szPhysicsMaterial = v; }, "Material preset used for this part's physics values");
      props.Checkbox<Ramp>(
         m_part, "Overwrite Material"s, //
         [](const Ramp* surf) { return surf->m_d.m_overwritePhysics; }, //
         [](Ramp* surf, bool v) { surf->m_d.m_overwritePhysics = v; }, "Use the fields below instead of the physics material's values");
      props.InputFloat<Ramp>(
         m_part, "Elasticity"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_elasticity; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_part, "Friction"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_friction; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_friction = max(v, 0.f); }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_part, "Scatter Angle"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_scatter; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3, "Random angle added to the ball's bounce direction");
      props.Checkbox<Ramp>(
         m_part, "Collidable"s, //
         [](const Ramp* surf) { return surf->m_d.m_collidable; }, //
         [](Ramp* surf, bool v) { surf->m_d.m_collidable = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

void RampUIPart::UpdatePointZField(PropertyPane& props, DragPoint* point)
{
   props.InputFloat<DragPoint>(
      point, "Height Offset"s, //
      [](const DragPoint* p) { return p->GetZ(); }, //
      [](DragPoint* p, float v) { p->SetZ(v); }, PropertyPane::Unit::VPLength, 1, "Height of this point relative to the height interpolated between the ramp's bottom and top");
   ImGui::BeginDisabled();
   props.InputFloat<DragPoint>(
      point, "Real Height"s, //
      [](const DragPoint* p) { return p->GetCalcHeight(); }, //
      [](DragPoint*, float) {}, PropertyPane::Unit::VPLength, 1, "Absolute height of this point after interpolation");
   ImGui::EndDisabled();
}
}
