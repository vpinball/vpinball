#pragma once

#include "EditorUIPart.h"
#include "parts/plunger.h"

namespace VPX::EditorUI
{

class PlungerUIPart final : public EditableUIPart<Plunger>
{
public:
   explicit PlungerUIPart(Plunger* plunger);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
