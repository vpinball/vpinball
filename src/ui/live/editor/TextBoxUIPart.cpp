#include "core/stdafx.h"

#include "TextBoxUIPart.h"

namespace VPX::EditorUI
{

TextBoxUIPart::TextBoxUIPart(Textbox* textbox)
   : m_textbox(textbox)
   , m_visible(textbox->m_d.m_visible)
{
}

TextBoxUIPart::~TextBoxUIPart() { m_textbox->m_d.m_visible = m_visible; }

TextBoxUIPart::TransformMask TextBoxUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void TextBoxUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
}

void TextBoxUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_textbox->m_d.m_visible;

   const bool isUIVisible = m_textbox->IsVisible(m_textbox);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_textbox->m_d.m_visible = true;
      //ctx.DrawWireframe(m_light);
      //ctx.DrawHitObjects(m_light);
   }

   m_textbox->m_d.m_visible = isUIVisible && m_visible;
}

void TextBoxUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("TextBox"s, m_textbox);
   
   if (props.BeginSection("Visuals"s))
   {
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      // Missing position
      props.EndSection();
   }
}

}
