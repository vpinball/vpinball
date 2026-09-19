#pragma once

#include "EditorUIPart.h"
#include "parts/decal.h"

namespace VPX::EditorUI
{

class DecalUIPart final : public EditableUIPart<Decal>
{
public:
   explicit DecalUIPart(Decal* decal);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
