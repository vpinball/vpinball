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
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_decal->m_d.m_visible = true;
      // ctx.DrawWireframe(m_decal);
   }

   m_decal->m_d.m_visible = isUIVisible && m_visible;
}

void DecalUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Decal"s, m_decal);

   if (props.BeginSection("Visual"s))
   {
      props.Checkbox<Decal>(m_decal, "Reflection Enabled"s, //
         [](const Decal* decal) { return decal->m_d.m_reflectionEnabled; }, //
         [](Decal* decal, bool v) { decal->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection<Decal>(m_decal, [](Decal* obj) { return &(obj->m_d.m_tdr); });
}

}
