#include "core/stdafx.h"

#include "TextBoxUIPart.h"

namespace VPX::EditorUI
{

TextBoxUIPart::TextBoxUIPart(Textbox* textbox)
   : EditableUIPart(textbox, { &Data::m_visible })
{
}

TextBoxUIPart::TransformMask TextBoxUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void TextBoxUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
}

void TextBoxUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   // TODO draw a selection overlay (textboxes have no wireframe/hit objects to display)
}

void TextBoxUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("TextBox"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
