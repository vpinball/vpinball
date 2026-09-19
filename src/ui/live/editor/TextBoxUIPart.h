#pragma once

#include "EditorUIPart.h"
#include "parts/textbox.h"

namespace VPX::EditorUI
{

class TextBoxUIPart final : public EditableUIPart<Textbox>
{
public:
   explicit TextBoxUIPart(Textbox* textbox);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
