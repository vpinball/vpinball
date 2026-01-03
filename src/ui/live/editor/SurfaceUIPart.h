#pragma once

#include "EditableUIPart.h"
#include "parts/surface.h"

namespace VPX::EditorUI
{

class SurfaceUIPart final : public EditableUIPart
{
public:
   explicit SurfaceUIPart(Surface* surface);
   ~SurfaceUIPart() override;

   IEditable* GetEditable() const override { return m_surface; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Surface* const m_surface;
   string m_outlinerPath;
   bool m_topVisible;
   bool m_sideVisible;
};

}
