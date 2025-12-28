#pragma once

#include "EditableUIPart.h"
#include "parts/kicker.h"

namespace VPX::EditorUI
{

class KickerUIPart : public EditableUIPart
{
public:
   explicit KickerUIPart(Kicker* kicker);
   ~KickerUIPart() override;

   IEditable* GetEditable() const override { return m_kicker; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Kicker* const m_kicker;
   string m_outlinerPath;
   KickerType m_kickerType;
};

}
