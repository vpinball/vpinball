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

PlungerUIPart::TransformMask PlungerUIPart::GetTransform(Matrix3D& transform) { return TM_None; }

void PlungerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { }

void PlungerUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_plunger->m_d.m_visible;

   const bool isUIVisible = m_plunger->IsVisible(m_plunger);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_plunger->m_d.m_visible = true;
      // FIXME ctx.DrawHitObjects(m_plunger);
   }

   m_plunger->m_d.m_visible = isUIVisible && m_visible;
}

void PlungerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Plunger", m_plunger);

   if (props.BeginSection("Visual"s))
   {

      props.Checkbox<Plunger>(
         m_plunger, "Reflection Enabled"s, //
         [](const Plunger* plunger) { return plunger->m_d.m_reflectionEnabled; }, //
         [](Plunger* plunger, bool v) { plunger->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection<Plunger>(m_plunger, [](Plunger* obj) { return &(obj->m_d.m_tdr); });
}

}