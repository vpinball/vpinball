#pragma once

#include "EditableUIPart.h"
#include "parts/decal.h"

namespace VPX::EditorUI
{

class DecalUIPart : public EditableUIPart
{
public:
   explicit DecalUIPart(Decal* becal);
   ~DecalUIPart() override;

   IEditable* GetEditable() const override { return m_decal; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Decal* const m_decal;
   string m_outlinerPath;
   bool m_visible;
};

}
