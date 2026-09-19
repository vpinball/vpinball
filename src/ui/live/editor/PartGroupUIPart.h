#pragma once

#include "EditorUIPart.h"
#include "parts/PartGroup.h"

namespace VPX::EditorUI
{

class PartGroupUIPart final : public EditorUIPart
{
public:
   explicit PartGroupUIPart(PartGroup* partGroup);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   PartGroup* const m_part;
};

}
