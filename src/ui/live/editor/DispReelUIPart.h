#pragma once

#include "EditableUIPart.h"
#include "parts/dispreel.h"

namespace VPX::EditorUI
{

class DispReelUIPart : public EditableUIPart
{
public:
   explicit DispReelUIPart(DispReel* dispreel);
   ~DispReelUIPart() override;

   IEditable* GetEditable() const override { return m_dispreel; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   DispReel* const m_dispreel;
   string m_outlinerPath;
};

}
