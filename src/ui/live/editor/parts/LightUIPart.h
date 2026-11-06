#pragma once

#include "EditorUIPart.h"
#include "math/matrix.h"
#include "parts/light.h"

namespace VPX::EditorUI
{

class LightUIPart final : public EditableUIPart<Light>
{
public:
   explicit LightUIPart(Light* light);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   DragPointCurve* GetDragPointCurve() const override { return &m_part->m_curve; }
   float GetDragPointZ(const DragPoint* point) const override { return point->GetZ() + m_part->m_surfaceHeight; }

   // The bulb center (m_d.m_vCenter) is editable independently of the outline curve
   bool HasEditCenter() const override { return true; }
   Vertex3Ds GetEditCenter() const override { return Vertex3Ds(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, m_part->m_surfaceHeight); }
   void SetEditCenter(const Vertex2D& pos) override { m_part->m_d.m_vCenter = pos; }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   bool IsNewPointSmooth() const override { return true; }
};

}
