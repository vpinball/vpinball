#pragma once

#include "EditorUIPart.h"
#include "parts/trigger.h"

namespace VPX::EditorUI
{

class TriggerUIPart final : public EditableUIPart<Trigger>
{
public:
   explicit TriggerUIPart(Trigger* trigger);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   DragPointCurve* GetDragPointCurve() override { return &m_part->m_curve; }

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   bool IsExtraHidden() const override { return m_part->m_d.m_shape == TriggerShape::TriggerNone; }
};

}
