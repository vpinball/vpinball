#pragma once

#include "EditorUIPart.h"
#include "parts/ball.h"

namespace VPX::EditorUI
{

class BallUIPart final : public EditableUIPart<Ball>
{
public:
   explicit BallUIPart(Ball* ball);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
