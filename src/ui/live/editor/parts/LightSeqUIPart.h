#pragma once

#include "EditorUIPart.h"
#include "math/matrix.h"
#include "parts/lightseq.h"

namespace VPX::EditorUI
{

class LightSeqUIPart final : public EditableUIPart<LightSeq>
{
public:
   explicit LightSeqUIPart(LightSeq* lightSeq);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   // The animation center (m_d.m_vCenter) is editable through the point edit mode
   bool HasEditCenter() const override { return true; }
   Vertex3Ds GetEditCenter() const override { return Vertex3Ds(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, 0.f); }
   void SetEditCenter(const Vertex2D& pos) override { m_part->m_d.m_vCenter = pos; }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
