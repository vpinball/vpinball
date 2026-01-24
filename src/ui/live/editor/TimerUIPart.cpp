#include "core/stdafx.h"

#include "TimerUIPart.h"

namespace VPX::EditorUI
{

TimerUIPart::TimerUIPart(Timer* timer)
   : m_timer(timer)
{
}

TimerUIPart::~TimerUIPart() { }

TimerUIPart::TransformMask TimerUIPart::GetTransform(Matrix3D& transform) { return TM_None; }

void TimerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { }

void TimerUIPart::Render(const EditorRenderContext& ctx)
{
   const bool isUIVisible = m_timer->IsVisible(m_timer);
   if (isUIVisible && ctx.IsShowInvisible())
   {
      const ImU32 color = ctx.GetColor(ctx.IsSelected());
      ctx.DrawCircle(Vertex3Ds(m_timer->m_d.m_v.x, m_timer->m_d.m_v.y, 0.f), Vertex3Ds(1.f, 0.f, 0.f), Vertex3Ds(0.f, 1.f, 0.f), 18.f, color);
      ctx.DrawCircle(Vertex3Ds(m_timer->m_d.m_v.x, m_timer->m_d.m_v.y, 0.f), Vertex3Ds(1.f, 0.f, 0.f), Vertex3Ds(0.f, 1.f, 0.f), 15.f, color);
      for (int i = 0; i < 12; i++)
      {
         const float angle = (float)(M_PI * 2.0 / 12.0) * (float)i;
         const float sn = sinf(angle);
         const float cs = cosf(angle);
         ctx.DrawLine(
            Vertex3Ds(m_timer->m_d.m_v.x + sn * 9.0f, m_timer->m_d.m_v.y - cs * 9.0f, 0.f), Vertex3Ds(m_timer->m_d.m_v.x + sn * 15.0f, m_timer->m_d.m_v.y - cs * 15.0f, 0.f), color);
      }

      //angle = ((PI*2)/24) * 3;
      ctx.DrawLine(Vertex3Ds(m_timer->m_d.m_v.x, m_timer->m_d.m_v.y, 0.f), Vertex3Ds(m_timer->m_d.m_v.x + 10.5f, m_timer->m_d.m_v.y - 7.5f, 0.f), color);
   }
}

void TimerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Timer"s, m_timer);

   props.TimerSection<Timer>(m_timer, [](Timer* obj) { return &(obj->m_d.m_tdr); });
}

}
