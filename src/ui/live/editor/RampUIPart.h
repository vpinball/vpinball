#pragma once

#include "EditableUIPart.h"
#include "parts/ramp.h"

namespace VPX::EditorUI
{

class RampUIPart : public EditableUIPart
{
public:
   explicit RampUIPart(Ramp* ramp);
   ~RampUIPart() override;

   IEditable* GetEditable() const override { return m_ramp; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Ramp* const m_ramp;
   string m_outlinerPath;
   bool m_visible;
};

}
