#pragma once

#include "EditorUIPart.h"
#include "math/matrix.h"
#include "parts/flasher.h"

namespace VPX::EditorUI
{

class FlasherUIPart final : public EditableUIPart<Flasher>
{
public:
   explicit FlasherUIPart(Flasher* flasher);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   // The points of a reference image are not edited one by one (an ongoing point edit keeps its curve)
   DragPointCurve* GetDragPointCurve() const override;
   float GetDragPointZ(const DragPoint* point) const override { return point->GetZ() + m_part->m_d.m_height; }

   void RenderOverlay(const EditorRenderContext& ctx) override;
   void RenderBlueprint(const BlueprintRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Vertex2D m_lastScaleRequest { 1.f, 1.f }; // Reference images: last X/Y scale asked by a transform tool (see ReferenceImageMath::UniformScale)
   bool HasPointAutoTexture() const override { return true; }
   bool HasPointTextureCoord() const override { return true; }
};

}
