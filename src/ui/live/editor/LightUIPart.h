#pragma once

#include "EditableUIPart.h"
#include "parts/light.h"

namespace VPX::EditorUI
{

class LightUIPart : public EditableUIPart
{
public:
   explicit LightUIPart(Light* light);
   ~LightUIPart() override;

   IEditable* GetEditable() const override { return m_light; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Light* const m_light;
   string m_outlinerPath;
   bool m_visible;
};

}