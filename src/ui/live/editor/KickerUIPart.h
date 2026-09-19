#pragma once

#include "EditorUIPart.h"
#include "parts/kicker.h"

namespace VPX::EditorUI
{

class KickerUIPart final : public EditableUIPart<Kicker>
{
public:
   explicit KickerUIPart(Kicker* kicker);
   ~KickerUIPart() override;

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   bool IsExtraHidden() const override { return m_part->m_d.m_kickertype == KickerType::KickerInvisible; }
};

}
