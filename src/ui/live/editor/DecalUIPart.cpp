#include "core/stdafx.h"

#include "DecalUIPart.h"

namespace VPX::EditorUI
{

DecalUIPart::DecalUIPart(Decal* decal)
   : EditableUIPart(decal, { &Data::m_visible })
{
}

DecalUIPart::TransformMask DecalUIPart::GetTransform(Matrix3D& transform)
{
   // Desktop backdrop decals are flat, in the 2D backdrop XY plane
   const float height = m_part->m_desktopBackdrop ? 0.f : m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void DecalUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
   m_part->m_d.m_vCenter.x = pos.x;
   m_part->m_d.m_vCenter.y = pos.y;
}

void DecalUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawHitObjects(m_part); // Draw the UI picking quad (decals have no wireframe to display)
}

void DecalUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Decal"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.MaterialCombo<Decal>(
         m_part, "Material"s, //
         [](const Decal* decal) { return decal->m_d.m_szMaterial; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_szMaterial = v; });
      // Missing type
      props.InputString<Decal>(
         m_part, "Text"s, //
         [](const Decal* decal) { return decal->m_d.m_text; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_text = v; });
      props.Checkbox<Decal>(
         m_part, "Vertical Text"s, //
         [](const Decal* decal) { return decal->m_d.m_verticalText; }, //
         [](Decal* decal, bool v) { decal->m_d.m_verticalText = v; });
      // Missing Font Color and Font selection
      props.ImageCombo<Decal>(
         m_part, "Image"s, //
         [](const Decal* decal) { return decal->m_d.m_szImage; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_szImage = v; });
      // Missing sizing
      props.InputFloat<Decal>(
         m_part, "Width"s, //
         [](const Decal* decal) { return decal->m_d.m_width; }, //
         [](Decal* decal, float v) { decal->m_d.m_width = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Decal>(
         m_part, "Height"s, //
         [](const Decal* decal) { return decal->m_d.m_height; }, //
         [](Decal* decal, float v) { decal->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Decal>(
         m_part, "Position"s, //
         [](const Decal* decal) { return decal->m_d.m_vCenter; }, //
         [](Decal* decal, const Vertex2D& v) { decal->Translate(Vertex2D(v.x - decal->m_d.m_vCenter.x, v.y - decal->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Decal>(
         m_part, "Surface"s, //
         [](const Decal* decal) { return decal->m_d.m_szSurface; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_szSurface = v; });
      props.InputFloat<Decal>(
         m_part, "Rotation"s, //
         [](const Decal* decal) { return decal->m_d.m_rotation; }, //
         [](Decal* decal, float v) { decal->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   //props.TimerSection(m_part);
}

}
