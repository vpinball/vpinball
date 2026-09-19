#pragma once

#include "EditorUIPart.h"
#include "parts/hittarget.h"

namespace VPX::EditorUI
{

class HitTargetUIPart final : public EditableUIPart<HitTarget>
{
public:
   explicit HitTargetUIPart(HitTarget* hitTarget);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
