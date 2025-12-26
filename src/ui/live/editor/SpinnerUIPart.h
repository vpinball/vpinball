#pragma once

#include "EditableUIPart.h"
#include "parts/spinner.h"

namespace VPX::EditorUI
{

class SpinnerUIPart : public EditableUIPart
{
public:
   explicit SpinnerUIPart(Spinner* spinner);
   ~SpinnerUIPart() override;

   IEditable* GetEditable() const override { return m_spinner; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Spinner* const m_spinner;
   string m_outlinerPath;
   bool m_visible;
};

}