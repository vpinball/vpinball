#pragma once

#include "EditorUIPart.h"
#include "math/matrix.h"
#include "parts/timer.h"

namespace VPX::EditorUI
{

class TimerUIPart final : public EditorUIPart
{
public:
   explicit TimerUIPart(Timer* timer);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Timer* m_part;
};

}
