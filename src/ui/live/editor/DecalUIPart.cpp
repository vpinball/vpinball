#include "core/stdafx.h"

#include "DecalUIPart.h"

namespace VPX::EditorUI
{

DecalUIPart::DecalUIPart(Decal* decal)
   : m_decal(decal)
   , m_visible(decal->m_d.m_visible)
{
}

DecalUIPart::~DecalUIPart() { m_decal->m_d.m_visible = m_visible; }

DecalUIPart::TransformMask DecalUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_decal->GetPTable()->GetSurfaceHeight(m_decal->m_d.m_szSurface, m_decal->m_d.m_vCenter.x, m_decal->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_decal->m_d.m_vCenter.x, m_decal->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void DecalUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
   m_decal->m_d.m_vCenter.x = pos.x;
   m_decal->m_d.m_vCenter.y = pos.y;
}

void DecalUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_decal->m_d.m_visible;

   const bool isUIVisible = m_decal->IsVisible(m_decal);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_decal->m_d.m_visible = true;
      // ctx.DrawWireframe(m_decal);
   }

   m_decal->m_d.m_visible = isUIVisible && m_visible;
}

void DecalUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Decal"s, m_decal);
   
   if (props.BeginSection("Visuals"s))
   {
      props.MaterialCombo<Decal>(
         m_decal, "Material"s, //
         [](const Decal* decal) { return decal->m_d.m_szMaterial; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_szMaterial = v; });
      // Missing type
      props.InputString<Decal>(
         m_decal, "Text"s, //
         [](const Decal* decal) { return decal->m_d.m_text; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_text = v; });
      props.Checkbox<Decal>(
         m_decal, "Vertical Text"s, //
         [](const Decal* decal) { return decal->m_d.m_verticalText; }, //
         [](Decal* decal, bool v) { decal->m_d.m_verticalText = v; });
      // Missing Font Color and Font selection
      props.ImageCombo<Decal>(
         m_decal, "Image"s, //
         [](const Decal* decal) { return decal->m_d.m_szImage; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_szImage = v; });
      // Missing sizing
      props.InputFloat<Decal>(
         m_decal, "Width"s, //
         [](const Decal* decal) { return decal->m_d.m_width; }, //
         [](Decal* decal, float v) { decal->m_d.m_width = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Decal>(
         m_decal, "Height"s, //
         [](const Decal* decal) { return decal->m_d.m_height; }, //
         [](Decal* decal, float v) { decal->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Decal>(
         m_decal, "Position"s, //
         [](const Decal* decal) { return decal->m_d.m_vCenter; }, //
         [](Decal* decal, const Vertex2D& v) { decal->Translate(Vertex2D(v.x - decal->m_d.m_vCenter.x, v.y - decal->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Decal>(
         m_decal, "Surface"s, //
         [](const Decal* decal) { return decal->m_d.m_szSurface; }, //
         [](Decal* decal, const string& v) { decal->m_d.m_szSurface = v; });
      props.InputFloat<Decal>(
         m_decal, "Rotation"s, //
         [](const Decal* decal) { return decal->m_d.m_rotation; }, //
         [](Decal* decal, float v) { decal->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   props.TimerSection<Decal>(m_decal, [](Decal* obj) { return &(obj->m_d.m_tdr); });
}

}
