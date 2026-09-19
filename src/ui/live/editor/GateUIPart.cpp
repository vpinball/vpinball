#include "core/stdafx.h"

#include "GateUIPart.h"

namespace VPX::EditorUI
{

GateUIPart::GateUIPart(Gate* gate)
   : EditableUIPart(gate, { &Data::m_visible })
{
}

GateUIPart::TransformMask GateUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D Smatrix = Matrix3D::MatrixScale(m_part->m_d.m_length);
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y) + m_part->m_d.m_height;
   const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, height);
   const Matrix3D Rmatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_part->m_d.m_rotation));
   transform = (Smatrix * Rmatrix) * Tmatrix;
   return static_cast<TransformMask>(static_cast<int>(TM_TransAny) | static_cast<int>(TM_ScaleAll) | static_cast<int>(TM_RotZ));
}

void GateUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_vCenter.x = pos.x;
   m_part->m_d.m_vCenter.y = pos.y;
   m_part->m_d.m_height = pos.z - m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
   m_part->m_d.m_length = (scale.x + scale.y + scale.z) / 3.f;
   m_part->m_d.m_rotation = rot.z;
}

void GateUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
   ctx.DrawHitObjects(m_part);
}

void GateUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Gate"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Gate>(
         m_part, "Shape"s, vector<string> { "Wire W"s, "Wire Rectangle"s, "Gate Plate"s, "Long Plate"s }, //
         [](const Gate* gate) { return static_cast<int>(gate->m_d.m_type) - 1; }, //
         [](Gate* gate, int v) { gate->m_d.m_type = static_cast<GateType>(v + 1); });
      props.Checkbox<Gate>(
         m_part, "Visible"s, //
         [this](const Gate* gate) { return GetVisibility(gate); }, //
         [this](Gate* gate, bool v) { SetVisibility(gate, v); });
      props.Checkbox<Gate>(
         m_part, "Show Bracket"s, //
         [](const Gate* gate) { return gate->m_d.m_showBracket; }, //
         [](Gate* gate, bool v) { gate->m_d.m_showBracket = v; });
      props.Checkbox<Gate>(
         m_part, "Reflection Enabled"s, //
         [](const Gate* gate) { return gate->m_d.m_reflectionEnabled; }, //
         [](Gate* gate, bool v) { gate->m_d.m_reflectionEnabled = v; });
      props.MaterialCombo<Gate>(
         m_part, "Material"s, //
         [](const Gate* gate) { return gate->m_d.m_szMaterial; }, //
         [](Gate* gate, const string& v) { gate->m_d.m_szMaterial = v; });
      props.EndSection();
   }

   if (props.BeginSection("Geometry"s))
   {
      props.InputFloat<Gate>(
         m_part, "Length"s, //
         [](const Gate* gate) { return gate->m_d.m_length; }, //
         [](Gate* gate, float v) { gate->m_d.m_length = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Gate>(
         m_part, "Height"s, //
         [](const Gate* gate) { return gate->m_d.m_height; }, //
         [](Gate* gate, float v) { gate->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Gate>(
         m_part, "Open Angle"s, //
         [](const Gate* gate) { return gate->m_d.m_angleMax; }, //
         [](Gate* gate, float v) { gate->m_d.m_angleMax = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Gate>(
         m_part, "Close Angle"s, //
         [](const Gate* gate) { return gate->m_d.m_angleMin; }, //
         [](Gate* gate, float v) { gate->m_d.m_angleMin = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Gate>(
         m_part, "Position"s, //
         [](const Gate* gate) { return gate->m_d.m_vCenter; }, //
         [](Gate* gate, const Vertex2D& v) { gate->Translate(Vertex2D(v.x - gate->m_d.m_vCenter.x, v.y - gate->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Gate>(
         m_part, "Surface"s, //
         [](const Gate* gate) { return gate->m_d.m_szSurface; }, //
         [](Gate* gate, const string& v) { gate->m_d.m_szSurface = v; });
      props.InputFloat<Gate>(
         m_part, "Rotation"s, //
         [](const Gate* gate) { return gate->m_d.m_rotation; }, //
         [](Gate* gate, float v) { gate->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Gate>(
         m_part, "Elasticity"s, //
         [](const Gate* gate) { return gate->m_d.m_elasticity; }, //
         [](Gate* gate, float v) { gate->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Gate>(
         m_part, "Friction"s, //
         [](const Gate* gate) { return gate->m_d.m_friction; }, //
         [](Gate* gate, float v) { gate->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Gate>(
         m_part, "Damping"s, //
         [](const Gate* gate) { return gate->m_d.m_damping; }, //
         [](Gate* gate, float v) { gate->m_d.m_damping = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Gate>(
         m_part, "Gravity Factor"s, //
         [](const Gate* gate) { return gate->m_d.m_gravityfactor; }, //
         [](Gate* gate, float v) { gate->m_d.m_gravityfactor = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Gate>(
         m_part, "Collidable"s, //
         [](const Gate* gate) { return gate->m_d.m_collidable; }, //
         [](Gate* gate, bool v) { gate->m_d.m_collidable = v; });
      props.Checkbox<Gate>(
         m_part, "Two Way"s, //
         [](const Gate* gate) { return gate->m_d.m_twoWay; }, //
         [](Gate* gate, bool v) { gate->m_d.m_twoWay = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
