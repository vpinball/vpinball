#pragma once

#include "EditorUIPart.h"
#include "parts/rubber.h"

namespace VPX::EditorUI
{

class RubberUIPart final : public EditableUIPart<Rubber>
{
public:
   explicit RubberUIPart(Rubber* rubber);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
