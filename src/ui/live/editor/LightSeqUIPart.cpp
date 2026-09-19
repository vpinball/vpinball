#include "core/stdafx.h"

#include "LightSeqUIPart.h"

namespace VPX::EditorUI
{

LightSeqUIPart::LightSeqUIPart(LightSeq* lightSeq)
   : EditableUIPart(lightSeq)
{
}

LightSeqUIPart::TransformMask LightSeqUIPart::GetTransform(Matrix3D& transform)
{
   transform = Matrix3D::MatrixTranslate(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, 0.f);
   return TM_TransAny;
}

void LightSeqUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
   m_part->m_d.m_vCenter.x = pos.x;
   m_part->m_d.m_vCenter.y = pos.y;
}

void LightSeqUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   // TODO draw an icon at the animation center (like TimerUIPart does)
}

void LightSeqUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("LightSeq"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.CollectionCombo<LightSeq>(
         m_part, "Collection"s, //
         [](const LightSeq* lightSeq) { return MakeString(lightSeq->m_d.m_wzCollection); }, //
         [](LightSeq* lightSeq, const string& v) { lightSeq->m_d.m_wzCollection = MakeWString(v); });
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
