#include "core/stdafx.h"

#include "BallUIPart.h"

namespace VPX::EditorUI
{

BallUIPart::BallUIPart(Ball* ball)
   : m_ball(ball)
   , m_visible(ball->m_d.m_visible)
{
}

BallUIPart::~BallUIPart() { m_ball->m_d.m_visible = m_visible; }

BallUIPart::TransformMask BallUIPart::GetTransform(Matrix3D& transform)
{
   transform = Matrix3D::MatrixTranslate(m_ball->m_hitBall.m_d.m_pos);
   return static_cast<TransformMask>(TM_TransAny | TM_ScaleAll);
}

void BallUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { m_ball->m_hitBall.m_d.m_pos = pos; }

void BallUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_ball->m_d.m_visible;

   const bool isUIVisible = m_ball->IsVisible(m_ball);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_ball->m_d.m_visible = true;
      ctx.DrawWireframe(m_ball);
   }

   m_ball->m_d.m_visible = isUIVisible && m_visible;
}

void BallUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Ball"s, m_ball);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Ball>(
         m_ball, "Visible"s, //
         [this](const Ball* ball) { return ball == m_ball ? m_visible : ball->m_d.m_visible; }, //
         [this](Ball* ball, bool v) { (ball == m_ball ? m_visible : ball->m_d.m_visible) = v; });
      props.Checkbox<Ball>(
         m_ball, "Reflection enabled"s, //
         [](const Ball* ball) { return ball->m_d.m_reflectionEnabled; }, //
         [](Ball* ball, bool v) { ball->m_d.m_reflectionEnabled = v; });
      props.Checkbox<Ball>(
         m_ball, "Reflection forced"s, //
         [](const Ball* ball) { return ball->m_d.m_forceReflection; }, //
         [](Ball* ball, bool v) { ball->m_d.m_forceReflection = v; });
      
      props.Separator("Rendering"s);
      props.Checkbox<Ball>(
         m_ball, "Use Table Settings"s, //
         [](const Ball* ball) { return ball->m_d.m_useTableRenderSettings; }, //
         [](Ball* ball, bool v) { ball->m_d.m_useTableRenderSettings = v; });
      ImGui::BeginDisabled(props.GetEditedPart<Ball>(m_ball)->m_d.m_useTableRenderSettings);
      props.ImageCombo<Ball>(
         m_ball, "Ball Image"s, //
         [](const Ball* ball) { return ball->m_d.m_szImage; }, //
         [](Ball* ball, const string& v) { ball->m_d.m_szImage = v; });
      props.Checkbox<Ball>(
         m_ball, "Spherical Map"s, //
         [](const Ball* ball) { return ball->m_d.m_pinballEnvSphericalMapping; }, //
         [](Ball* ball, bool v) { ball->m_d.m_pinballEnvSphericalMapping = v; });
      props.ImageCombo<Ball>(
         m_ball, "Decal"s, //
         [](const Ball* ball) { return ball->m_d.m_imageDecal; }, //
         [](Ball* ball, const string& v) { ball->m_d.m_imageDecal = v; });
      props.Checkbox<Ball>(
         m_ball, "Logo mode"s, //
         [](const Ball* ball) { return ball->m_d.m_decalMode; }, //
         [](Ball* ball, bool v) { ball->m_d.m_decalMode = v; });
      props.InputRGB<Ball>(
         m_ball, "Tint"s, //
         [](const Ball* ball) { return convertColor(ball->m_d.m_color); }, //
         [](Ball* ball, const vec3& v) { ball->m_d.m_color = convertColorRGB(v); });

      props.Separator("Reflections"s);
      props.InputFloat<Ball>(
         m_ball, "Playfield Strength"s, //
         [](const Ball* ball) { return ball->m_d.m_playfieldReflectionStrength; }, //
         [](Ball* ball, float v) { ball->m_d.m_playfieldReflectionStrength = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Ball>(
         m_ball, "Bulb Intensity Scale"s, //
         [](const Ball* ball) { return ball->m_d.m_bulb_intensity_scale; }, //
         [](Ball* ball, float v) { ball->m_d.m_bulb_intensity_scale = v; }, PropertyPane::Unit::None, 2);
      ImGui::EndDisabled();

      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat3<Ball>(
         m_ball, "Position"s, //
         [](const Ball* ball) { return ball->m_hitBall.m_d.m_pos; }, //
         [](Ball* ball, const vec3& v) { ball->m_hitBall.m_d.m_pos = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ball>(
         m_ball, "Radius"s, //
         [](const Ball* ball) { return ball->m_hitBall.m_d.m_radius; }, //
         [](Ball* ball, float v) { ball->m_hitBall.m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 2);
      props.InputFloat<Ball>(
         m_ball, "Mass"s, //
         [](const Ball* ball) { return ball->m_hitBall.m_d.m_mass; }, //
         [](Ball* ball, float v) { ball->m_hitBall.m_d.m_mass = v; }, PropertyPane::Unit::VPMass, 2);
      // Only show these when inspecting a played table
      if (m_ball->GetPTable()->m_liveBaseTable)
      {
         props.InputFloat3<Ball>(
            m_ball, "Velocity"s, //
            [](const Ball* ball) { return ball->m_hitBall.m_d.m_vel; }, //
            [](Ball* ball, const vec3& v) { ball->m_hitBall.m_d.m_vel = v; }, PropertyPane::Unit::VPSpeed, 3);
         props.InputFloat3<Ball>(
            m_ball, "Angular Momentum"s, //
            [](const Ball* ball) { return ball->m_hitBall.m_angularmomentum; }, //
            [](Ball* ball, const vec3& v) { ball->m_hitBall.m_angularmomentum = v; }, PropertyPane::Unit::None, 3);
      }
      props.EndSection();
   }

   props.TimerSection<Ball>(m_ball, [](Ball* obj) { return &(obj->m_d.m_tdr); });
}

}
