#pragma once

#include "EditableUIPart.h"
#include "parts/timer.h"

namespace VPX::EditorUI
{

class TimerUIPart : public EditableUIPart
{
public:
   TimerUIPart(Timer* timer);
   ~TimerUIPart() override;

   IEditable* GetEditable() const override { return m_timer; }

   const string& GetOutlinerPath() const { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Timer* const m_timer;
   string m_outlinerPath;
};

}
