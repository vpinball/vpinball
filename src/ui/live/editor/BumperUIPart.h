#pragma once

#include "EditorUIPart.h"
#include "parts/bumper.h"

namespace VPX::EditorUI
{

class BumperUIPart final : public EditableUIPart<Bumper>
{
public:
   explicit BumperUIPart(Bumper* bumper);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
