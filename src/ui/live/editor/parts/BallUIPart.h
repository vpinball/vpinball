#pragma once

#include "EditorUIPart.h"
#include "math/matrix.h"
#include "parts/ball.h"

namespace VPX::EditorUI
{

class BallUIPart final : public EditableUIPart<Ball>
{
public:
   explicit BallUIPart(Ball* ball);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;
   std::function<void()> CaptureTransformState() override
   {
      // The position and radius live in the physics ball state, not in m_d
      return [restoreBase = EditableUIPart<Ball>::CaptureTransformState(), ball = m_part, state = m_part->m_hitBall.m_d]()
      {
         restoreBase();
         ball->m_hitBall.m_d = state;
      };
   }

   void RenderOverlay(const EditorRenderContext& ctx) override;
   void RenderBlueprint(const BlueprintRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
