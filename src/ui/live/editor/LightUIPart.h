#pragma once

#include "EditorUIPart.h"
#include "parts/light.h"

namespace VPX::EditorUI
{

class LightUIPart final : public EditableUIPart<Light>
{
public:
   explicit LightUIPart(Light* light);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   DragPointCurve* GetDragPointCurve() override { return &m_part->m_curve; }
   float GetDragPointZ(const DragPoint* point) const override { return point->m_v.z + m_part->m_surfaceHeight; }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
