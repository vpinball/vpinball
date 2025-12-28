#include "core/stdafx.h"

#include "GateUIPart.h"

namespace VPX::EditorUI
{

GateUIPart::GateUIPart(Gate* gate)
   : m_gate(gate)
   , m_visible(gate->m_d.m_visible)
{
}

GateUIPart::~GateUIPart() { m_gate->m_d.m_visible = m_visible; }

GateUIPart::TransformMask GateUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_gate->GetPTable()->GetSurfaceHeight(m_gate->m_d.m_szSurface, m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void GateUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_gate->m_d.m_vCenter.x = pos.x;
   m_gate->m_d.m_vCenter.y = pos.y;
}

void GateUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_gate->m_d.m_visible;

   const bool isUIVisible = m_gate->IsVisible(m_gate);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_gate->m_d.m_visible = true;
      ctx.DrawWireframe(m_gate);
   }

   m_gate->m_d.m_visible = isUIVisible && m_visible;
}

void GateUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Gate", m_gate);

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Gate>(
         m_gate, "Shape", vector<string> { "Wire W"s, "Wire Rectangle"s, "Gate Plate"s, "Long Plate"s }, //
         [](const Gate* gate) { return static_cast<int>(gate->m_d.m_type) - 1; }, //
         [](Gate* gate, int v) { gate->m_d.m_type = static_cast<GateType>(v + 1); });
      props.Checkbox<Gate>(
         m_gate, "Visible"s, //
         [this](const Gate* gate) { return gate == m_gate ? m_visible : gate->m_d.m_visible; }, //
         [this](Gate* gate, bool v)
         {
            gate->m_d.m_visible = v;
            (gate == m_gate ? m_visible : gate->m_d.m_visible) = v;
         });
      props.Checkbox<Gate>(
         m_gate, "Show Bracket"s, //
         [](const Gate* gate) { return gate->m_d.m_showBracket; }, //
         [](Gate* gate, bool v) { gate->m_d.m_showBracket = v; });
      props.Checkbox<Gate>(
         m_gate, "Reflection Enabled"s, //
         [](const Gate* gate) { return gate->m_d.m_reflectionEnabled; }, //
         [](Gate* gate, bool v) { gate->m_d.m_reflectionEnabled = v; });
      props.MaterialCombo<Gate>(
         m_gate, "Material"s, //
         [](const Gate* gate) { return gate->m_d.m_szMaterial; }, //
         [](Gate* gate, const string& v) { gate->m_d.m_szMaterial = v; });
      props.EndSection();
   }

   if (props.BeginSection("Geometry"s))
   {
      props.InputFloat<Gate>(
         m_gate, "Length", //
         [](const Gate* gate) { return gate->m_d.m_length; }, //
         [](Gate* gate, float v) { gate->m_d.m_length = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Gate>(
         m_gate, "Height", //
         [](const Gate* gate) { return gate->m_d.m_height; }, //
         [](Gate* gate, float v) { gate->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Gate>(
         m_gate, "Open Angle", //
         [](const Gate* gate) { return gate->m_d.m_angleMax; }, //
         [](Gate* gate, float v) { gate->m_d.m_angleMax = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Gate>(
         m_gate, "Close Angle", //
         [](const Gate* gate) { return gate->m_d.m_angleMin; }, //
         [](Gate* gate, float v) { gate->m_d.m_angleMin = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat<Gate>(
         m_gate, "X", //
         [](const Gate* gate) { return gate->m_d.m_vCenter.x; }, //
         [](Gate* gate, float v) { gate->Translate(Vertex2D(v - gate->m_d.m_vCenter.x, 0.f)); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Gate>(
         m_gate, "Y", //
         [](const Gate* gate) { return gate->m_d.m_vCenter.y; }, //
         [](Gate* gate, float v) { gate->Translate(Vertex2D(0.f, v - gate->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Gate>(
         m_gate, "Surface", //
         [](const Gate* gate) { return gate->m_d.m_szSurface; }, //
         [](Gate* gate, const string& v) { gate->m_d.m_szSurface = v; });
      props.InputFloat<Gate>(
         m_gate, "Rotation", //
         [](const Gate* gate) { return gate->m_d.m_rotation; }, //
         [](Gate* gate, float v) { gate->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Gate>(
         m_gate, "Elasticity", //
         [](const Gate* gate) { return gate->m_d.m_elasticity; }, //
         [](Gate* gate, float v) { gate->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Gate>(
         m_gate, "Friction", //
         [](const Gate* gate) { return gate->m_d.m_friction; }, //
         [](Gate* gate, float v) { gate->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Gate>(
         m_gate, "Damping", //
         [](const Gate* gate) { return gate->m_d.m_damping; }, //
         [](Gate* gate, float v) { gate->m_d.m_damping = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Gate>(
         m_gate, "Gravity Factor", //
         [](const Gate* gate) { return gate->m_d.m_gravityfactor; }, //
         [](Gate* gate, float v) { gate->m_d.m_gravityfactor = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Gate>(
         m_gate, "Collidable"s, //
         [](const Gate* gate) { return gate->m_d.m_collidable; }, //
         [](Gate* gate, bool v) { gate->m_d.m_collidable = v; });
      props.Checkbox<Gate>(
         m_gate, "Two Way"s, //
         [](const Gate* gate) { return gate->m_d.m_twoWay; }, //
         [](Gate* gate, bool v) { gate->m_d.m_twoWay = v; });
      props.EndSection();
   }

   props.TimerSection<Gate>(m_gate, [](Gate* obj) { return &(obj->m_d.m_tdr); });
}

}