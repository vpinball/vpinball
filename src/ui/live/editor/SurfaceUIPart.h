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

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
