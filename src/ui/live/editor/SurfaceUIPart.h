#pragma once

#include "EditorUIPart.h"
#include "parts/surface.h"

namespace VPX::EditorUI
{

class SurfaceUIPart final : public EditableUIPart<Surface>
{
public:
   explicit SurfaceUIPart(Surface* surface);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   DragPointCurve* GetDragPointCurve() override { return &m_part->m_curve; }
   float GetDragPointZ(const DragPoint* point) const override { return point->m_v.z + 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop); }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
