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
   // Textboxes are flat 2D parts in the backdrop XY plane
   transform = Matrix3D::MatrixTranslate(0.5f * (m_part->m_d.m_v1.x + m_part->m_d.m_v2.x), 0.5f * (m_part->m_d.m_v1.y + m_part->m_d.m_v2.y), 0.f);
   return static_cast<TransformMask>(TM_TransX | TM_TransY);
}

void TextBoxUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center(0.5f * (m_part->m_d.m_v1.x + m_part->m_d.m_v2.x), 0.5f * (m_part->m_d.m_v1.y + m_part->m_d.m_v2.y));
   m_part->Translate(Vertex2D(pos.x - center.x, pos.y - center.y));
}

void TextBoxUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawHitObjects(m_part); // Draw the UI picking quad (textboxes have no wireframe to display)
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
