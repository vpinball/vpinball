#pragma once

#include "EditorUIPart.h"
#include "parts/ramp.h"

namespace VPX::EditorUI
{

class RampUIPart final : public EditableUIPart<Ramp>
{
public:
   explicit RampUIPart(Ramp* ramp);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
