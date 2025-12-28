#include "core/stdafx.h"

#include "FlipperUIPart.h"

namespace VPX::EditorUI
{

FlipperUIPart::FlipperUIPart(Flipper* flipper)
   : m_flipper(flipper)
   , m_visible(flipper->m_d.m_visible)
{
}

FlipperUIPart::~FlipperUIPart() { m_flipper->m_d.m_visible = m_visible; }

FlipperUIPart::TransformMask FlipperUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_flipper->GetPTable()->GetSurfaceHeight(m_flipper->m_d.m_szSurface, m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y);
   transform = Matrix3D::MatrixTranslate(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, height);
   return static_cast<TransformMask>(TM_TransAny | TM_RotAny);
}

void FlipperUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_flipper->m_d.m_Center.x = pos.x;
   m_flipper->m_d.m_Center.y = pos.y;
}

void FlipperUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_flipper->m_d.m_visible;

   const bool isUIVisible = m_flipper->IsVisible(m_flipper);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_flipper->m_d.m_visible = true;
      ctx.DrawWireframe(m_flipper);
      //ctx.DrawHitObjects(m_flipper);
   }

   m_flipper->m_d.m_visible = isUIVisible && m_visible;
}

void FlipperUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Flipper", m_flipper);

   if (props.BeginSection("Visual"s))
   {

      props.EndSection();
   }

   props.TimerSection<Flipper>(m_flipper, [](Flipper* obj) { return &(obj->m_d.m_tdr); });
}

}