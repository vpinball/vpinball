#pragma once

#include "EditableUIPart.h"
#include "parts/flasher.h"

namespace VPX::EditorUI
{

class FlasherUIPart : public EditableUIPart
{
public:
   explicit FlasherUIPart(Flasher* flasher);
   ~FlasherUIPart() override;

   IEditable* GetEditable() const override { return m_flasher; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Flasher* const m_flasher;
   string m_outlinerPath;
   bool m_visible;
};

}