#pragma once

#include "EditorUIPart.h"
#include "parts/flasher.h"

namespace VPX::EditorUI
{

class FlasherUIPart final : public EditableUIPart<Flasher>
{
public:
   explicit FlasherUIPart(Flasher* flasher);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   DragPointCurve* GetDragPointCurve() override { return &m_part->m_curve; }
   float GetDragPointZ(const DragPoint* point) const override { return point->m_v.z + m_part->m_d.m_height; }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   bool HasPointAutoTexture() const override { return true; }
   bool HasPointTextureCoord() const override { return true; }
};

}
