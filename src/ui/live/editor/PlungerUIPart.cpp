#include "core/stdafx.h"

#include "PlungerUIPart.h"

namespace VPX::EditorUI
{

PlungerUIPart::PlungerUIPart(Plunger* plunger)
   : m_plunger(plunger)
   , m_visible(plunger->m_d.m_visible)
{
}

PlungerUIPart::~PlungerUIPart() { m_plunger->m_d.m_visible = m_visible; }

PlungerUIPart::TransformMask PlungerUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D Smatrix = Matrix3D::MatrixScale(m_plunger->m_d.m_width, m_plunger->m_d.m_height, m_plunger->m_d.m_width);
   const float height = m_plunger->GetPTable()->GetSurfaceHeight(m_plunger->m_d.m_szSurface, m_plunger->m_d.m_v.x, m_plunger->m_d.m_v.y) + m_plunger->m_d.m_zAdjust;
   const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(m_plunger->m_d.m_v.x, m_plunger->m_d.m_v.y, height);
   transform = Smatrix * Tmatrix;
   return static_cast<TransformMask>(static_cast<int>(TM_TransAny) | static_cast<int>(TM_ScaleAll));
}

void PlungerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_plunger->m_d.m_v.x = pos.x;
   m_plunger->m_d.m_v.y = pos.y;
   m_plunger->m_d.m_zAdjust = pos.z - m_plunger->GetPTable()->GetSurfaceHeight(m_plunger->m_d.m_szSurface, m_plunger->m_d.m_v.x, m_plunger->m_d.m_v.y);
   m_plunger->m_d.m_height = scale.y;
   m_plunger->m_d.m_width = (scale.x + scale.z) / 2.f;
}

void PlungerUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_plunger->m_d.m_visible;

   const bool isUIVisible = m_plunger->IsVisible(m_plunger);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_plunger->m_d.m_visible = true;
      ctx.DrawHitObjects(m_plunger);
      // FIXME ctx.DrawWireframe(m_plunger);
   }

   m_plunger->m_d.m_visible = isUIVisible && m_visible;
}

void PlungerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Plunger"s, m_plunger);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Plunger>(
         m_plunger, "Visible"s, //
         [this](const Plunger* plunger) { return plunger == m_plunger ? m_visible : plunger->m_d.m_visible; }, //
         [this](Plunger* plunger, bool v)
         {
            plunger->m_d.m_visible = v;
            (plunger == m_plunger ? m_visible : plunger->m_d.m_visible) = v;
         });
      props.Combo<Plunger>(
         m_plunger, "Type"s, vector<string> { "Modern"s, "Flat"s, "Custom"s }, //
         [](const Plunger* plunger) { return static_cast<int>(plunger->m_d.m_type) - 1; }, //
         [](Plunger* plunger, int v) { plunger->m_d.m_type = static_cast<PlungerType>(v + 1); });
      props.MaterialCombo<Plunger>(
         m_plunger, "Material"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_szMaterial; }, //
         [](Plunger* plunger, const string& v) { plunger->m_d.m_szMaterial = v; });
      props.ImageCombo<Plunger>(
         m_plunger, "Image"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_szImage; }, //
         [](Plunger* plunger, const string& v) { plunger->m_d.m_szImage = v; });
      props.InputInt<Plunger>(
         m_plunger, "Flat Frames"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_animFrames; }, //
         [](Plunger* plunger, int v) { plunger->m_d.m_animFrames = v; });
      props.InputFloat<Plunger>(
         m_plunger, "Width"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_width; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_width = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Height"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_height; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.Checkbox<Plunger>(
         m_plunger, "Reflection Enabled"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_reflectionEnabled; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Custom Settings"s))
   {
      props.InputFloat<Plunger>(
         m_plunger, "Rod Diameter"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_rodDiam; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_rodDiam = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputString<Plunger>(
         m_plunger, "Tip Shape"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_szTipShape; }, //
         [](Plunger* plunger, const string& v) { plunger->m_d.m_szTipShape = v; });
      props.InputFloat<Plunger>(
         m_plunger, "Ring Gap"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_ringGap; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_ringGap = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Ring Diameter"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_ringDiam; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_ringDiam = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Ring Width"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_ringWidth; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_ringWidth = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Spring Diameter"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_springDiam; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_springDiam = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Spring Gauge"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_springGauge; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_springGauge = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Spring Loops"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_springLoops; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_springLoops = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "End Loops"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_springEndLoops; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_springEndLoops = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat3<Plunger>(
         m_plunger, "Position"s, //
         [](const Plunger* plunger) { return vec3(plunger->m_d.m_v.x, plunger->m_d.m_v.y, plunger->m_d.m_zAdjust); }, //
         [](Plunger* plunger, const vec3& v)
         {
            plunger->Translate(Vertex2D(v.x - plunger->m_d.m_v.x, v.y - plunger->m_d.m_v.y));
            plunger->m_d.m_zAdjust = v.z;
         },
         PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Plunger>(
         m_plunger, "Surface"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_szSurface; }, //
         [](Plunger* plunger, const string& v) { plunger->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Plunger>(
         m_plunger, "Pull Speed"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_speedPull; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_speedPull = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Plunger>(
         m_plunger, "Release Speed"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_speedFire; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_speedFire = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Plunger>(
         m_plunger, "Stroke Length"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_stroke; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_stroke = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_plunger, "Scatter Velocity"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_scatterVelocity; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_scatterVelocity = v; }, PropertyPane::Unit::None, 2);
      props.Checkbox<Plunger>(
         m_plunger, "Enable Mechanical Plunger"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_mechPlunger; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_mechPlunger = v; });
      props.Checkbox<Plunger>(
         m_plunger, "Auto Plunger"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_autoPlunger; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_autoPlunger = v; });
      props.InputFloat<Plunger>(
         m_plunger, "Mech Strength"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_mechStrength; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_mechStrength = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Plunger>(
         m_plunger, "Momentum Xfer"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_momentumXfer; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_momentumXfer = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Plunger>(
         m_plunger, "Park Position"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_parkPosition; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_parkPosition = v; }, PropertyPane::Unit::Percent, 3);
      props.EndSection();
   }

   props.TimerSection<Plunger>(m_plunger, [](Plunger* obj) { return &(obj->m_d.m_tdr); });
}

}
