#pragma once

#include "EditorUIPart.h"
#include "parts/light.h"

namespace VPX::EditorUI
{

class LightUIPart final : public EditableUIPart<Light>
{
public:
   explicit LightUIPart(Light* light);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
