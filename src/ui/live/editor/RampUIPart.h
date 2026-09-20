#pragma once

#include "EditorUIPart.h"
#include "parts/ramp.h"

namespace VPX::EditorUI
{

class RampUIPart final : public EditableUIPart<Ramp>
{
public:
   explicit RampUIPart(Ramp* ramp);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   DragPointCurve* GetDragPointCurve() override { return &m_part->m_curve; }
   float GetDragPointZ(const DragPoint* point) const override { return point->m_calcHeight; }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   void InsertPointOnCurve(const Vertex2D& pos) override;
   bool HasPointAutoTexture() const override { return true; }
   void UpdatePointZField(PropertyPane& props, DragPoint* point) override;
};

}
