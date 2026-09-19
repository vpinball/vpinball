#pragma once

#include "EditorUIPart.h"
#include "parts/lightseq.h"

namespace VPX::EditorUI
{

class LightSeqUIPart final : public EditableUIPart<LightSeq>
{
public:
   explicit LightSeqUIPart(LightSeq* lightSeq);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;
};

}
