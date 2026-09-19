#include "core/stdafx.h"
#include "BallUIPart.h"

#include "utils/color.h"

namespace VPX::EditorUI
{

BallUIPart::BallUIPart(Ball* ball)
   : EditableUIPart(ball, { &Data::m_visible })
{
}

BallUIPart::TransformMask BallUIPart::GetTransform(Matrix3D& transform)
{
   transform = Matrix3D::MatrixScale(m_part->m_hitBall.m_d.m_radius) * Matrix3D::MatrixTranslate(m_part->m_hitBall.m_d.m_pos);
   return static_cast<TransformMask>(TM_TransAny | TM_ScaleAll);
}

void BallUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_hitBall.m_d.m_pos = pos;
   m_part->m_hitBall.m_d.m_radius = (scale.x + scale.y + scale.z) / 3.f;
}

void BallUIPart::RenderOverlay(const EditorRenderContext& ctx) { ctx.DrawWireframe(m_part); }

void BallUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Ball"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Ball>(
         m_part, "Visible"s, //
         [this](const Ball* ball) { return GetVisibility(ball); }, //
         [this](Ball* ball, bool v) { SetVisibility(ball, v); });
      props.Checkbox<Ball>(
         m_part, "Reflection enabled"s, //
         [](const Ball* ball) { return ball->m_d.m_reflectionEnabled; }, //
         [](Ball* ball, bool v) { ball->m_d.m_reflectionEnabled = v; });
      props.Checkbox<Ball>(
         m_part, "Reflection forced"s, //
         [](const Ball* ball) { return ball->m_d.m_forceReflection; }, //
         [](Ball* ball, bool v) { ball->m_d.m_forceReflection = v; });

      props.Separator("Rendering"s);
      props.Checkbox<Ball>(
         m_part, "Use Table Settings"s, //
         [](const Ball* ball) { return ball->m_d.m_useTableRenderSettings; }, //
         [](Ball* ball, bool v) { ball->m_d.m_useTableRenderSettings = v; });
      ImGui::BeginDisabled(props.GetEditedPart<Ball>(m_part)->m_d.m_useTableRenderSettings);
      props.ImageCombo<Ball>(
         m_part, "Ball Image"s, //
         [](const Ball* ball) { return ball->m_d.m_szImage; }, //
         [](Ball* ball, const string& v) { ball->m_d.m_szImage = v; });
      props.Checkbox<Ball>(
         m_part, "Spherical Map"s, //
         [](const Ball* ball) { return ball->m_d.m_pinballEnvSphericalMapping; }, //
         [](Ball* ball, bool v) { ball->m_d.m_pinballEnvSphericalMapping = v; });
      props.ImageCombo<Ball>(
         m_part, "Decal"s, //
         [](const Ball* ball) { return ball->m_d.m_imageDecal; }, //
         [](Ball* ball, const string& v) { ball->m_d.m_imageDecal = v; });
      props.Checkbox<Ball>(
         m_part, "Logo mode"s, //
         [](const Ball* ball) { return ball->m_d.m_decalMode; }, //
         [](Ball* ball, bool v) { ball->m_d.m_decalMode = v; });
      props.InputRGB<Ball>(
         m_part, "Tint"s, //
         [](const Ball* ball) { return convertColor(ball->m_d.m_color); }, //
         [](Ball* ball, const vec3& v) { ball->m_d.m_color = convertColorRGB(v); });

      props.Separator("Reflections"s);
      props.InputFloat<Ball>(
         m_part, "Playfield Strength"s, //
         [](const Ball* ball) { return ball->m_d.m_playfieldReflectionStrength; }, //
         [](Ball* ball, float v) { ball->m_d.m_playfieldReflectionStrength = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Ball>(
         m_part, "Bulb Intensity Scale"s, //
         [](const Ball* ball) { return ball->m_d.m_bulb_intensity_scale; }, //
         [](Ball* ball, float v) { ball->m_d.m_bulb_intensity_scale = v; }, PropertyPane::Unit::None, 2);
      ImGui::EndDisabled();

      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat3<Ball>(
         m_part, "Position"s, //
         [](const Ball* ball) { return ball->m_hitBall.m_d.m_pos; }, //
         [](Ball* ball, const vec3& v) { ball->m_hitBall.m_d.m_pos = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ball>(
         m_part, "Radius"s, //
         [](const Ball* ball) { return ball->m_hitBall.m_d.m_radius; }, //
         [](Ball* ball, float v) { ball->m_hitBall.m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 2);
      props.InputFloat<Ball>(
         m_part, "Mass"s, //
         [](const Ball* ball) { return ball->m_hitBall.m_d.m_mass; }, //
         [](Ball* ball, float v) { ball->m_hitBall.m_d.m_mass = v; }, PropertyPane::Unit::VPMass, 2);
      // Only show these when inspecting a played table
      if (m_part->GetPTable()->m_liveBaseTable)
      {
         props.InputFloat3<Ball>(
            m_part, "Velocity"s, //
            [](const Ball* ball) { return ball->m_hitBall.m_d.m_vel; }, //
            [](Ball* ball, const vec3& v) { ball->m_hitBall.m_d.m_vel = v; }, PropertyPane::Unit::VPSpeed, 3);
         props.InputFloat3<Ball>(
            m_part, "Angular Momentum"s, //
            [](const Ball* ball) { return ball->m_hitBall.m_angularmomentum; }, //
            [](Ball* ball, const vec3& v) { ball->m_hitBall.m_angularmomentum = v; }, PropertyPane::Unit::None, 3);
      }
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
