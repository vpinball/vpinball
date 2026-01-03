#pragma once

#include "EditableUIPart.h"
#include "parts/flipper.h"

namespace VPX::EditorUI
{

class FlipperUIPart final : public EditableUIPart
{
public:
   explicit FlipperUIPart(Flipper* flipper);
   ~FlipperUIPart() override;

   IEditable* GetEditable() const override { return m_flipper; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Flipper* const m_flipper;
   string m_outlinerPath;
   bool m_visible;
};

}
