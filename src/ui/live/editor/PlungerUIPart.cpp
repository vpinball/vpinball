#include "core/stdafx.h"

#include "PlungerUIPart.h"

#include "math/matrix.h"

namespace VPX::EditorUI
{

PlungerUIPart::PlungerUIPart(Plunger* plunger)
   : EditableUIPart(plunger, { &Data::m_visible })
{
}

PlungerUIPart::TransformMask PlungerUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D Smatrix = Matrix3D::MatrixScale(m_part->m_d.m_width, m_part->m_d.m_height, m_part->m_d.m_width);
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_v.x, m_part->m_d.m_v.y) + m_part->m_d.m_zAdjust;
   const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(m_part->m_d.m_v.x, m_part->m_d.m_v.y, height);
   transform = Smatrix * Tmatrix;
   return static_cast<TransformMask>(static_cast<int>(TM_TransAny) | static_cast<int>(TM_ScaleAll));
}

void PlungerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_v.x = pos.x;
   m_part->m_d.m_v.y = pos.y;
   m_part->m_d.m_zAdjust = pos.z - m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_v.x, m_part->m_d.m_v.y);
   m_part->m_d.m_height = scale.y;
   m_part->m_d.m_width = (scale.x + scale.z) / 2.f;
}

void PlungerUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawHitObjects(m_part);
   // FIXME ctx.DrawWireframe(m_part);
}

void PlungerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Plunger"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Plunger>(
         m_part, "Visible"s, //
         [this](const Plunger* plunger) { return GetVisibility(plunger); }, //
         [this](Plunger* plunger, bool v) { SetVisibility(plunger, v); });
      props.Combo<Plunger>(
         m_part, "Type"s, vector<string> { "Modern"s, "Flat"s, "Custom"s }, //
         [](const Plunger* plunger) { return static_cast<int>(plunger->m_d.m_type) - 1; }, //
         [](Plunger* plunger, int v) { plunger->m_d.m_type = static_cast<PlungerType>(v + 1); });
      props.MaterialVisuals(m_part, "Material"s, &Data::m_szMaterial, "Image"s, &Data::m_szImage);
      props.InputFloat<Plunger>(
         m_part, "Half Width"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_width; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_width = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_part, "Bottom Margin"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_height; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1, "Space reserved below the rest position, defining how far the plunger can be pulled");
      props.Checkbox<Plunger>(
         m_part, "Reflection Enabled"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_reflectionEnabled; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");

      if (m_part->m_d.m_type == PlungerType::PlungerTypeFlat)
         props.InputInt<Plunger>(
            m_part, "Flat Frames"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_animFrames; }, //
            [](Plunger* plunger, int v) { plunger->m_d.m_animFrames = v; }, "Number of frames of the flat plunger's image strip animation");
      else if (m_part->m_d.m_type == PlungerType::PlungerTypeCustom)
      {
         props.InputFloat<Plunger>(
            m_part, "Rod Diameter"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_rodDiam; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_rodDiam = v; }, PropertyPane::Unit::VPLength, 1);
         props.InputString<Plunger>(
            m_part, "Tip Shape"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_szTipShape; }, //
            [](Plunger* plunger, const string& v) { plunger->m_d.m_szTipShape = v; }, "Tip cross-section profile as 'offset diameter' pairs separated by semicolons");
         props.InputFloat<Plunger>(
            m_part, "Ring Gap"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_ringGap; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_ringGap = v; }, PropertyPane::Unit::VPLength, 1);
         props.InputFloat<Plunger>(
            m_part, "Ring Diameter"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_ringDiam; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_ringDiam = v; }, PropertyPane::Unit::VPLength, 1);
         props.InputFloat<Plunger>(
            m_part, "Ring Width"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_ringWidth; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_ringWidth = v; }, PropertyPane::Unit::VPLength, 1);
         props.InputFloat<Plunger>(
            m_part, "Spring Diameter"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_springDiam; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_springDiam = v; }, PropertyPane::Unit::VPLength, 1);
         props.InputFloat<Plunger>(
            m_part, "Spring Gauge"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_springGauge; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_springGauge = v; }, PropertyPane::Unit::VPLength, 1, "Diameter of the spring's wire");
         props.InputFloat<Plunger>(
            m_part, "Spring Loops"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_springLoops; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_springLoops = v; }, PropertyPane::Unit::VPLength, 1);
         props.InputFloat<Plunger>(
            m_part, "End Loops"s, //
            [](const Plunger* plunger) { return plunger->m_d.m_springEndLoops; }, //
            [](Plunger* plunger, float v) { plunger->m_d.m_springEndLoops = v; }, PropertyPane::Unit::VPLength, 1);
      }

      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat3<Plunger>(
         m_part, "Position"s, //
         [](const Plunger* plunger) { return vec3(plunger->m_d.m_v.x, plunger->m_d.m_v.y, plunger->m_d.m_zAdjust); }, //
         [](Plunger* plunger, const vec3& v)
         {
            plunger->Translate(Vertex2D(v.x - plunger->m_d.m_v.x, v.y - plunger->m_d.m_v.y));
            plunger->m_d.m_zAdjust = v.z;
         },
         PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Plunger>(
         m_part, "Surface"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_szSurface; }, //
         [](Plunger* plunger, const string& v) { plunger->m_d.m_szSurface = v; }, "Surface this part rests on (defines its base height)");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Plunger>(
         m_part, "Pull Speed"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_speedPull; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_speedPull = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Plunger>(
         m_part, "Release Speed"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_speedFire; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_speedFire = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Plunger>(
         m_part, "Stroke Length"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_stroke; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_stroke = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Plunger>(
         m_part, "Scatter Velocity"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_scatterVelocity; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_scatterVelocity = v; }, PropertyPane::Unit::None, 2, "Random speed variation added when the ball is launched");
      props.Checkbox<Plunger>(
         m_part, "Enable Mechanical Plunger"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_mechPlunger; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_mechPlunger = v; }, "Enable driving the plunger from a mechanical plunger input or the plunger key, beside table script");
      props.Checkbox<Plunger>(
         m_part, "Auto Plunger"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_autoPlunger; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_autoPlunger = v; }, "Fire the plunger by a solenoid instead of a plunger spring");
      props.InputFloat<Plunger>(
         m_part, "Auto Plunger Strength"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_mechStrength; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_mechStrength = v; }, PropertyPane::Unit::None, 2, "Force of the solenoid when the auto plunger fires the ball");
      props.InputFloat<Plunger>(
         m_part, "Momentum Xfer"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_momentumXfer; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_momentumXfer = v; }, PropertyPane::Unit::None, 2, "Ratio of the plunger's momentum transferred to the ball");
      props.InputFloat<Plunger>(
         m_part, "Park Position"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_parkPosition; }, //
         [](Plunger* plunger, float v) { plunger->m_d.m_parkPosition = v; }, PropertyPane::Unit::Percent, 3, "Resting position of the plunger along its stroke");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
