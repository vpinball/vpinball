#include "core/stdafx.h"

#include "LightSeqUIPart.h"

namespace VPX::EditorUI
{

LightSeqUIPart::LightSeqUIPart(LightSeq* lightSeq)
   : m_lightSeq(lightSeq)
{
}

LightSeqUIPart::~LightSeqUIPart() { }

LightSeqUIPart::TransformMask LightSeqUIPart::GetTransform(Matrix3D& transform)
{
   transform = Matrix3D::MatrixTranslate(m_lightSeq->m_d.m_vCenter.x, m_lightSeq->m_d.m_vCenter.y, 0.f);
   return TM_TransAny;
}

void LightSeqUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
   m_lightSeq->m_d.m_vCenter.x = pos.x;
   m_lightSeq->m_d.m_vCenter.y = pos.y;
}

void LightSeqUIPart::Render(const EditorRenderContext& ctx)
{
   const bool isUIVisible = m_lightSeq->IsVisible(m_lightSeq);
   if (isUIVisible && ctx.IsShowInvisible())
   {
      const ImU32 color = ctx.GetColor(ctx.IsSelected());
   }
}

void LightSeqUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("LightSeq"s, m_lightSeq);
   
   if (props.BeginSection("Visuals"s))
   {
      props.CollectionCombo<LightSeq>(
         m_lightSeq, "Collection"s, //
         [](const LightSeq* lightSeq) { return MakeString(lightSeq->m_d.m_wzCollection); }, //
         [](LightSeq* lightSeq, const string& v) { lightSeq->m_d.m_wzCollection = MakeWString(v); });
      props.InputFloat2<LightSeq>(
         m_lightSeq, "Animation Center"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_vCenter; }, //
         [](LightSeq* lightSeq, const Vertex2D& v) { lightSeq->m_d.m_vCenter = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputInt<LightSeq>(
         m_lightSeq, "Update Interval (ms)"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_updateinterval; }, //
         [](LightSeq* lightSeq, int v) { lightSeq->m_d.m_updateinterval = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<LightSeq>(
         m_lightSeq, "Position"s, //
         [](const LightSeq* lightSeq) { return lightSeq->m_d.m_v; }, //
         [](LightSeq* lightSeq, const Vertex2D& v) { lightSeq->m_d.m_v = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   props.TimerSection<LightSeq>(m_lightSeq, [](LightSeq* obj) { return &(obj->m_d.m_tdr); });
}

}
