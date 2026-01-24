#include "core/stdafx.h"

#include "TriggerUIPart.h"

namespace VPX::EditorUI
{

TriggerUIPart::TriggerUIPart(Trigger* trigger)
   : m_trigger(trigger)
   , m_visible(trigger->m_d.m_visible)
{
}

TriggerUIPart::~TriggerUIPart() { m_trigger->m_d.m_visible = m_visible; }

TriggerUIPart::TransformMask TriggerUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_trigger->GetPTable()->GetSurfaceHeight(m_trigger->m_d.m_szSurface, m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void TriggerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D offset(pos.x - m_trigger->m_d.m_vCenter.x, pos.y - m_trigger->m_d.m_vCenter.y);
   m_trigger->Translate(offset);
}

void TriggerUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_trigger->m_d.m_visible;

   const bool isUIVisible = m_trigger->IsVisible(m_trigger);
   const bool visible = m_visible && m_trigger->m_d.m_shape != TriggerShape::TriggerNone;
   if (isUIVisible && (ctx.IsSelected() || (!visible && ctx.IsShowInvisible())))
   {
      m_trigger->m_d.m_visible = true;
      ctx.DrawHitObjects(m_trigger);
   }

   m_trigger->m_d.m_visible = isUIVisible && visible;
}

void TriggerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Trigger"s, m_trigger);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Trigger>(
         m_trigger, "Visible"s, //
         [this](const Trigger* trigger) { return trigger == m_trigger ? m_visible : trigger->m_d.m_visible; }, //
         [this](Trigger* trigger, bool v)
         {
            trigger->m_d.m_visible = v;
            (trigger == m_trigger ? m_visible : trigger->m_d.m_visible) = v;
         });
      props.Checkbox<Trigger>(
         m_trigger, "Reflection Enabled"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_reflectionEnabled; }, //
         [](Trigger* trigger, bool v) { trigger->m_d.m_reflectionEnabled = v; });
      props.Combo<Trigger>(
         m_trigger, "Shape"s, vector<string> { "None"s, "Wire A"s, "Star"s, "Wire B"s, "Button"s, "Wire C"s, "Wire D"s, "Inder"s },
         [](const Trigger* trigger) { return trigger->m_d.m_shape; }, //
         [](Trigger* trigger, int v) { trigger->m_d.m_shape = static_cast<TriggerShape>(v); });
      props.InputFloat<Trigger>(
         m_trigger, "Wire Thickness"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_wireThickness; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_wireThickness = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Trigger>(
         m_trigger, "Star Radius"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_radius; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Trigger>(
         m_trigger, "Rotation"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_rotation; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Trigger>(
         m_trigger, "Anim Speed"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_animSpeed; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_animSpeed = v; }, PropertyPane::Unit::None, 1);
      props.MaterialCombo<Trigger>(
         m_trigger, "Material"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_szMaterial; }, //
         [](Trigger* trigger, const string& v) { trigger->m_d.m_szMaterial = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Trigger>(
         m_trigger, "Enabled"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_enabled; }, //
         [](Trigger* trigger, bool v) { trigger->m_d.m_enabled = v; });
      props.InputFloat<Trigger>(
         m_trigger, "Hit Height"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_hit_height; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_hit_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Trigger>(
         m_trigger, "Position"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_vCenter; }, //
         [](Trigger* trigger, const Vertex2D& v) { trigger->Translate(Vertex2D(v.x - trigger->m_d.m_vCenter.x, v.y - trigger->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Trigger>(
         m_trigger, "Surface"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_szSurface; }, //
         [](Trigger* trigger, const string& v) { trigger->m_d.m_szSurface = v; });
      props.EndSection();
   }

   props.TimerSection<Trigger>(m_trigger, [](Trigger* obj) { return &(obj->m_d.m_tdr); });
}

}
