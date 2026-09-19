#pragma once

#include "EditorUIPart.h"
#include "parts/spinner.h"

namespace VPX::EditorUI
{

class SpinnerUIPart final : public EditableUIPart<Spinner>
{
public:
   explicit SpinnerUIPart(Spinner* spinner);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
