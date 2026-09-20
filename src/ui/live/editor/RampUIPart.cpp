#include "core/stdafx.h"

#include "RampUIPart.h"

namespace VPX::EditorUI
{

RampUIPart::RampUIPart(Ramp* ramp)
   : EditableUIPart(ramp, { &Data::m_visible })
{
}

RampUIPart::TransformMask RampUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D& center = m_part->m_curve.GetCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop));
   return TM_TransAny;
}

void RampUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D& center = m_part->m_curve.GetCenter();
   m_part->m_curve.TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
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
      props.ImageCombo<Ramp>(
         m_part, "Image"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_szImage; }, //
         [](Ramp* ramp, const string& v) { ramp->m_d.m_szImage = v; });
      props.MaterialCombo<Ramp>(
         m_part, "Material"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_szMaterial; }, //
         [](Ramp* ramp, const string& v) { ramp->m_d.m_szMaterial = v; });
      props.Combo<Ramp>(
         m_part, "Image Mode"s, vector { "World"s, "Wrap"s }, //
         [](const Ramp* ramp) { return static_cast<int>(ramp->m_d.m_imagealignment); }, //
         [](Ramp* ramp, int v) { ramp->m_d.m_imagealignment = static_cast<RampImageAlignment>(v); });
      props.Checkbox<Ramp>(
         m_part, "Apply Image to Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_imageWalls; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_imageWalls = v; });
      props.InputFloat<Ramp>(
         m_part, "Depth Bias"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_depthBias; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);
      props.Checkbox<Ramp>(
         m_part, "Visible"s, //
         [this](const Ramp* ramp) { return GetVisibility(ramp); }, //
         [this](Ramp* ramp, bool v) { SetVisibility(ramp, v); });
      props.Checkbox<Ramp>(
         m_part, "Reflection Enabled"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_reflectionEnabled; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_reflectionEnabled = v; });
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
         [](Ramp* ramp, float v) { ramp->m_d.m_leftwallheightvisible = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Right Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_rightwallheightvisible; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_rightwallheightvisible = v; }, PropertyPane::Unit::VPLength, 1);

      props.Separator("Wire Ramp"s);
      props.InputFloat<Ramp>(
         m_part, "Diameter"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDiameter; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDiameter = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Distance X"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDistanceX; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDistanceX = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Distance Y"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDistanceY; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDistanceY = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Ramp>(
         m_part, "Has Hit Event"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_hitEvent; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_hitEvent = v; });
      props.InputFloat<Ramp>(
         m_part, "Hit Threshold"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_threshold; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_threshold = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_part, "Left Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_leftwallheight; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_leftwallheight = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_part, "Right Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_rightwallheight; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_rightwallheight = v; }, PropertyPane::Unit::VPLength, 1);
      props.MaterialCombo<Ramp>(
         m_part, "Physics Material"s, //
         [](const Ramp* surf) { return surf->m_d.m_szPhysicsMaterial; }, //
         [](Ramp* surf, const string& v) { surf->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<Ramp>(
         m_part, "Overwrite Material"s, //
         [](const Ramp* surf) { return surf->m_d.m_overwritePhysics; }, //
         [](Ramp* surf, bool v) { surf->m_d.m_overwritePhysics = v; });
      props.InputFloat<Ramp>(
         m_part, "Elasticity"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_elasticity; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_part, "Friction"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_friction; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_part, "Scatter Angle"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_scatter; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
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
      [](const DragPoint* p) { return p->m_v.z; }, //
      [](DragPoint* p, float v) { p->m_v.z = v; }, PropertyPane::Unit::VPLength, 1);
   ImGui::BeginDisabled();
   props.InputFloat<DragPoint>(
      point, "Real Height"s, //
      [](const DragPoint* p) { return p->m_calcHeight; }, //
      [](DragPoint*, float) {}, PropertyPane::Unit::VPLength, 1);
   ImGui::EndDisabled();
}
}
