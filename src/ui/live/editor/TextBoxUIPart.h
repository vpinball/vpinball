#pragma once

#include "EditableUIPart.h"
#include "parts/textbox.h"

namespace VPX::EditorUI
{

class TextBoxUIPart final : public EditableUIPart
{
public:
   explicit TextBoxUIPart(Textbox* textbox);
   ~TextBoxUIPart() override;

   IEditable* GetEditable() const override { return m_textbox; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Textbox* const m_textbox;
   string m_outlinerPath;
   bool m_visible;
};

}
