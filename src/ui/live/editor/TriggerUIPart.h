#pragma once

#include "EditableUIPart.h"
#include "parts/trigger.h"

namespace VPX::EditorUI
{

class TriggerUIPart final : public EditableUIPart
{
public:
   explicit TriggerUIPart(Trigger* surface);
   ~TriggerUIPart() override;

   IEditable* GetEditable() const override { return m_trigger; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Trigger* const m_trigger;
   string m_outlinerPath;
   bool m_visible;
};

}
