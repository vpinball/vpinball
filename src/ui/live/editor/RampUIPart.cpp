#include "core/stdafx.h"

#include "RampUIPart.h"

namespace VPX::EditorUI
{

RampUIPart::RampUIPart(Ramp* ramp)
   : m_ramp(ramp)
   , m_visible(ramp->m_d.m_visible)
{
}

RampUIPart::~RampUIPart() { m_ramp->m_d.m_visible = m_visible; }

RampUIPart::TransformMask RampUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D center = m_ramp->GetPointCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (m_ramp->m_d.m_heightbottom + m_ramp->m_d.m_heighttop));
   return TM_TransAny;
}

void RampUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center = m_ramp->GetPointCenter();
   m_ramp->TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
   m_ramp->GetPTable()->m_undo.Undo(true);
   const float pz = 0.5f * (m_ramp->m_d.m_heightbottom + m_ramp->m_d.m_heighttop);
   m_ramp->m_d.m_heightbottom += pos.z - pz;
   m_ramp->m_d.m_heighttop += pos.z - pz;
}

void RampUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_ramp->m_d.m_visible;

   const bool isUIVisible = m_ramp->IsVisible(m_ramp);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_ramp->m_d.m_visible = true;
      ctx.DrawWireframe(m_ramp);
   }

   m_ramp->m_d.m_visible = isUIVisible && m_visible;
}

void RampUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Ramp"s, m_ramp);

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Ramp>(
         m_ramp, "Type"s, vector<string> { "Flat"s, "4 Wires"s, "2 Wires"s, "3 Wires Left"s, "3 Wires Right"s, "1 Wire"s }, //
         [](const Ramp* ramp) { return static_cast<int>(ramp->m_d.m_type); }, //
         [](Ramp* ramp, int v) { ramp->m_d.m_szImage = static_cast<RampType>(v); });
      props.ImageCombo<Ramp>(
         m_ramp, "Image"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_szImage; }, //
         [](Ramp* ramp, const string& v) { ramp->m_d.m_szImage = v; });
      props.MaterialCombo<Ramp>(
         m_ramp, "Material"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_szMaterial; }, //
         [](Ramp* ramp, const string& v) { ramp->m_d.m_szMaterial = v; });
      props.Combo<Ramp>(
         m_ramp, "Image Mode"s, vector { "World"s, "Wrap"s }, //
         [](const Ramp* ramp) { return static_cast<int>(ramp->m_d.m_imagealignment); }, //
         [](Ramp* ramp, int v) { ramp->m_d.m_imagealignment = static_cast<RampImageAlignment>(v); });
      props.Checkbox<Ramp>(
         m_ramp, "Apply Image to Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_imageWalls; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_imageWalls = v; });
      props.InputFloat<Ramp>(
         m_ramp, "Depth Bias"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_depthBias; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);
      props.Checkbox<Ramp>(
         m_ramp, "Visible"s, //
         [this](const Ramp* ramp) { return ramp == m_ramp ? m_visible : ramp->m_d.m_visible; }, //
         [this](Ramp* ramp, bool v)
         {
            ramp->m_d.m_visible = v;
            (ramp == m_ramp ? m_visible : ramp->m_d.m_visible) = v;
         });
      props.Checkbox<Ramp>(
         m_ramp, "Reflection Enabled"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_reflectionEnabled; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Geometry"s))
   {
      props.InputFloat<Ramp>(
         m_ramp, "Top Height"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_heighttop; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_heighttop = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Bottom Height"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_heightbottom; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_heightbottom = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Top Width"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_widthtop; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_widthtop = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Bottom Width"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_widthbottom; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_widthbottom = v; }, PropertyPane::Unit::VPLength, 1);

      props.Separator("Visible Walls"s);
      props.InputFloat<Ramp>(
         m_ramp, "Left Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_leftwallheightvisible; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_leftwallheightvisible = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Right Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_rightwallheightvisible; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_rightwallheightvisible = v; }, PropertyPane::Unit::VPLength, 1);

      props.Separator("Wire Ramp"s);
      props.InputFloat<Ramp>(
         m_ramp, "Diameter"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDiameter; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDiameter = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Distance X"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDistanceX; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDistanceX = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Distance Y"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_wireDistanceY; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_wireDistanceY = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Ramp>(
         m_ramp, "Has Hit Event"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_hitEvent; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_hitEvent = v; });
      props.InputFloat<Ramp>(
         m_ramp, "Hit Threshold"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_threshold; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_threshold = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_ramp, "Left Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_leftwallheight; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_leftwallheight = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Ramp>(
         m_ramp, "Right Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_rightwallheight; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_rightwallheight = v; }, PropertyPane::Unit::VPLength, 1);
      props.MaterialCombo<Ramp>(
         m_ramp, "Physics Material"s, //
         [](const Ramp* surf) { return surf->m_d.m_szPhysicsMaterial; }, //
         [](Ramp* surf, const string& v) { surf->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<Ramp>(
         m_ramp, "Overwrite Material"s, //
         [](const Ramp* surf) { return surf->m_d.m_overwritePhysics; }, //
         [](Ramp* surf, bool v) { surf->m_d.m_overwritePhysics = v; });
      props.InputFloat<Ramp>(
         m_ramp, "Elasticity"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_elasticity; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_ramp, "Friction"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_friction; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Ramp>(
         m_ramp, "Scatter Angle"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_scatter; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Ramp>(
         m_ramp, "Collidable"s, //
         [](const Ramp* surf) { return surf->m_d.m_collidable; }, //
         [](Ramp* surf, bool v) { surf->m_d.m_collidable = v; });
      props.EndSection();
   }

   props.TimerSection<Ramp>(m_ramp, [](Ramp* obj) { return &(obj->m_d.m_tdr); });
}

}
