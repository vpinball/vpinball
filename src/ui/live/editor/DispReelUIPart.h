#pragma once

#include "EditorUIPart.h"
#include "parts/dispreel.h"

namespace VPX::EditorUI
{

class DispReelUIPart final : public EditableUIPart<DispReel>
{
public:
   explicit DispReelUIPart(DispReel* dispReel);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
