#pragma once

#include "EditorUIPart.h"
#include "parts/flipper.h"

namespace VPX::EditorUI
{

class FlipperUIPart final : public EditableUIPart<Flipper>
{
public:
   explicit FlipperUIPart(Flipper* flipper);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
