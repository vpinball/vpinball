#pragma once

#include "EditorUIPart.h"
#include "parts/gate.h"

namespace VPX::EditorUI
{

class GateUIPart final : public EditableUIPart<Gate>
{
public:
   explicit GateUIPart(Gate* gate);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
