#include "core/stdafx.h"

#include "LightSeqUIPart.h"

#include "math/matrix.h"

namespace VPX::EditorUI
{

LightSeqUIPart::LightSeqUIPart(LightSeq* lightSeq)
   : EditableUIPart(lightSeq)
{
}

LightSeqUIPart::TransformMask LightSeqUIPart::GetTransform(Matrix3D& transform)
{
   transform = Matrix3D::MatrixTranslate(m_part->m_d.m_v.x, m_part->m_d.m_v.y, 0.f);
   return TM_TransAny;
}

void LightSeqUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_v.x = pos.x;
   m_part->m_d.m_v.y = pos.y;
}

void LightSeqUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   const ImU32 color = ctx.GetColor(ctx.IsSelected());
   const Vertex3Ds xAxis(1.f, 0.f, 0.f);
   const Vertex3Ds yAxis(0.f, 1.f, 0.f);

   // Part position marker (same as the win32 editor): a ring of 8 dots inside a circle
   const Vertex2D& v = m_part->m_d.m_v;
   ctx.DrawCircle(Vertex3Ds(v.x, v.y, 0.f), xAxis, yAxis, 18.f, color);
   for (int i = 0; i < 8; i++)
   {
      const float angle = (float)(M_PI * 2.0 / 8.0) * (float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      ctx.DrawCircle(Vertex3Ds(v.x + sn * 12.0f, v.y - cs * 12.0f, 0.f), xAxis, yAxis, 4.f, color);
   }
   ctx.DrawCircle(Vertex3Ds(v.x, v.y - 3.f, 0.f), xAxis, yAxis, 4.f, color);

   // Animation center marker (same as the win32 editor): a crosshair with a ring of 8 dots
   const Vertex2D& c = m_part->m_d.m_vCenter;
   ctx.DrawLine(Vertex3Ds(c.x - 10.f, c.y, 0.f), Vertex3Ds(c.x + 10.f, c.y, 0.f), color);
   ctx.DrawLine(Vertex3Ds(c.x, c.y - 10.f, 0.f), Vertex3Ds(c.x, c.y + 10.f, 0.f), color);
   for (int i = 0; i < 8; i++)
   {
      const float angle = (float)(M_PI * 2.0 / 8.0) * (float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      ctx.DrawCircle(Vertex3Ds(c.x + sn * 7.0f, c.y - cs * 7.0f, 0.f), xAxis, yAxis, 2.f, color);
   }
   ctx.DrawCircle(Vertex3Ds(c.x, c.y - 2.5f, 0.f), xAxis, yAxis, 2.f, color);
}

void LightSeqUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("LightSeq"s, m_part);

   UpdateCenterSection(props, &Data::m_vCenter);

   if (props.BeginSection("Visuals"s))
   {
      props.CollectionCombo<LightSeq>(
         m_part, "Collection"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_collection; }, //
         [](LightSeq* lightSeq, const string& v) { lightSeq->m_d.m_collection = v; });
      props.InputFloat2<LightSeq>(
         m_part, "Animation Center"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_vCenter; }, //
         [](LightSeq* lightSeq, const Vertex2D& v) { lightSeq->m_d.m_vCenter = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputInt<LightSeq>(
         m_part, "Update Interval (ms)"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_updateinterval; }, //
         [](LightSeq* lightSeq, int v) { lightSeq->m_d.m_updateinterval = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<LightSeq>(
         m_part, "Position"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_v; }, //
         [](LightSeq* lightSeq, const Vertex2D& v) { lightSeq->m_d.m_v = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
