#include "core/stdafx.h"

#include "TextBoxUIPart.h"

#include "utils/color.h"

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
      props.Checkbox<Textbox>(
         m_part, "Transparent"s, //
         [](const Textbox* textbox) { return textbox->m_d.m_transparent; }, //
         [](Textbox* textbox, bool v) { textbox->m_d.m_transparent = v; });
      props.InputRGB<Textbox>(
         m_part, "Back Color"s, //
         [](const Textbox* textbox) { return convertColor(textbox->m_d.m_backcolor); }, //
         [](Textbox* textbox, const vec3& v) { textbox->m_d.m_backcolor = convertColorRGB(v); });
      props.InputRGB<Textbox>(
         m_part, "Text Color"s, //
         [](const Textbox* textbox) { return convertColor(textbox->m_d.m_fontcolor); }, //
         [](Textbox* textbox, const vec3& v) { textbox->m_d.m_fontcolor = convertColorRGB(v); });
      props.Font<Textbox>(
         m_part, //
         [](const Textbox* textbox) { return textbox->m_d.m_font; }, //
         [](Textbox* textbox, const FontDesc& v) { textbox->m_d.m_font = v; });
      props.Combo<Textbox>(
         m_part, "Alignment"s, vector<string> { "Left"s, "Center"s, "Right"s }, //
         [](const Textbox* textbox) { return static_cast<int>(textbox->m_d.m_talign); }, //
         [](Textbox* textbox, int v) { textbox->m_d.m_talign = static_cast<TextAlignment>(v); });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Textbox>(
         m_part, "Position"s, //
         [](const Textbox* textbox) { return textbox->m_d.m_v1; }, //
         [](Textbox* textbox, const Vertex2D& v) { textbox->Translate(Vertex2D(v.x - textbox->m_d.m_v1.x, v.y - textbox->m_d.m_v1.y)); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat2<Textbox>(
         m_part, "Size"s, //
         [](const Textbox* textbox) { return Vertex2D(textbox->m_d.m_v2.x - textbox->m_d.m_v1.x, textbox->m_d.m_v2.y - textbox->m_d.m_v1.y); }, //
         [](Textbox* textbox, const Vertex2D& v)
         {
            textbox->m_d.m_v2.x = textbox->m_d.m_v1.x + v.x;
            textbox->m_d.m_v2.y = textbox->m_d.m_v1.y + v.y;
         },
         PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("State"s))
   {
      props.Checkbox<Textbox>(
         m_part, "Use Script DMD"s, //
         [](const Textbox* textbox) { return textbox->m_d.m_isDMD; }, //
         [](Textbox* textbox, bool v) { textbox->m_d.m_isDMD = v; });
      props.InputFloat<Textbox>(
         m_part, "Text Intensity"s, //
         [](const Textbox* textbox) { return textbox->m_d.m_intensity_scale; }, //
         [](Textbox* textbox, float v) { textbox->m_d.m_intensity_scale = v; }, PropertyPane::Unit::None, 1);
      props.InputString<Textbox>(
         m_part, "Text"s, //
         [](const Textbox* textbox) { return textbox->m_d.m_text; }, //
         [](Textbox* textbox, const string& v) { textbox->m_d.m_text = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
