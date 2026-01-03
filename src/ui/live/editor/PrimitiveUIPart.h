#pragma once

#include "EditableUIPart.h"
#include "parts/primitive.h"

namespace VPX::EditorUI
{

class PrimitiveUIPart final : public EditableUIPart
{
public:
   explicit PrimitiveUIPart(Primitive* primitive);
   ~PrimitiveUIPart() override;

   IEditable* GetEditable() const override { return m_primitive; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Primitive* const m_primitive;
   string m_outlinerPath;
   bool m_visible;
};

}
