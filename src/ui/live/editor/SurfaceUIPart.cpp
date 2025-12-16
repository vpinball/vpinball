#pragma once

#include "core/stdafx.h"

#include "SurfaceUIPart.h"

namespace VPX::EditorUI
{

SurfaceUIPart::SurfaceUIPart(Surface* surface)
   : m_surface(surface)
   , m_topVisible(surface->m_d.m_topBottomVisible)
   , m_sideVisible(surface->m_d.m_sideVisible)
{
}

SurfaceUIPart::~SurfaceUIPart()
{
   m_surface->m_d.m_topBottomVisible = m_topVisible;
   m_surface->m_d.m_sideVisible = m_sideVisible;
}

SurfaceUIPart::TransformMask SurfaceUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D center = m_surface->GetPointCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (m_surface->m_d.m_heightbottom + m_surface->m_d.m_heighttop));
   return TM_TransAny;
}

void SurfaceUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center = m_surface->GetPointCenter();
   m_surface->TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
   const float pz = 0.5f * (m_surface->m_d.m_heightbottom + m_surface->m_d.m_heighttop);
   m_surface->m_d.m_heightbottom += pos.z - pz;
   m_surface->m_d.m_heighttop += pos.z - pz;
}

void SurfaceUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
   {
      m_topVisible = m_surface->m_d.m_topBottomVisible;
      m_sideVisible = m_surface->m_d.m_sideVisible;
   }

   const bool isUIVisible = m_surface->IsVisible(m_surface);
   if (isUIVisible && (ctx.IsSelected() || (!m_sideVisible && !m_topVisible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_surface->m_d.m_topBottomVisible = true;
      m_surface->m_d.m_sideVisible = true;
      ctx.DrawWireframe(m_surface);
   }

   m_surface->m_d.m_topBottomVisible = isUIVisible && m_topVisible;
   m_surface->m_d.m_sideVisible = isUIVisible && m_sideVisible;
}

void SurfaceUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Surface", m_surface);

   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      props.Checkbox<Surface>(
         m_surface, "Top Visible"s, //
         [this](const Surface* surf) { return surf == m_surface ? m_topVisible : surf->m_d.m_topBottomVisible; }, //
         [this](Surface* surf, bool v) { surf->m_d.m_topBottomVisible = v; (surf == m_surface ? m_topVisible : surf->m_d.m_topBottomVisible) = v; });
      props.ImageCombo<Surface>(
         m_surface, "Top Image"s, //
         [](const Surface* surf) { return surf->m_d.m_szImage; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szImage = v; });
      props.MaterialCombo<Surface>(
         m_surface, "Top Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szTopMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szTopMaterial = v; });

      props.Checkbox<Surface>(
         m_surface, "Side Visible"s, //
         [this](const Surface* surf) { return surf == m_surface ? m_sideVisible : surf->m_d.m_sideVisible; }, //
         [this](Surface* surf, bool v) { surf->m_d.m_sideVisible = v; (surf == m_surface ? m_sideVisible : surf->m_d.m_sideVisible) = v; });
      props.ImageCombo<Surface>(
         m_surface, "Side Image"s, //
         [](const Surface* surf) { return surf->m_d.m_szSideImage; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSideImage = v; });
      props.MaterialCombo<Surface>(
         m_surface, "Side Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szSideMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSideMaterial = v; });
      props.MaterialCombo<Surface>(
         m_surface, "Slingshot Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szSlingShotMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSlingShotMaterial = v; });

      props.Checkbox<Surface>(
         m_surface, "Animate Slingshot"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshotAnimation; }, //
         [](Surface* surf, bool v) { surf->m_d.m_slingshotAnimation = v; });
      props.Checkbox<Surface>(
         m_surface, "Hide when dropped"s, //
         [](const Surface* surf) { return surf->m_d.m_flipbook; }, //
         [](Surface* surf, bool v) { surf->m_d.m_flipbook = v; });

      props.InputFloat<Surface>(
         m_surface, "Disable Spot Lights"s, //
         [](const Surface* surf) { return surf->m_d.m_disableLightingTop; }, //
         [](Surface* surf, float v) { surf->m_d.m_disableLightingTop = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Surface>(
         m_surface, "Translucency"s, //
         [](const Surface* surf) { return surf->m_d.m_disableLightingBelow; }, //
         [](Surface* surf, float v) { surf->m_d.m_disableLightingBelow = v; }, PropertyPane::Unit::None, 1);
      props.Checkbox<Surface>(
         m_surface, "Reflection Enabled"s, //
         [](const Surface* surf) { return surf->m_d.m_reflectionEnabled; }, //
         [](Surface* surf, bool v) { surf->m_d.m_reflectionEnabled = v; });
      props.InputFloat<Surface>(
         m_surface, "Top Height"s, //
         [](const Surface* surf) { return surf->m_d.m_heightbottom; }, //
         [](Surface* surf, float v) { surf->m_d.m_heightbottom = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Surface>(
         m_surface, "Bottom Height"s, //
         [](const Surface* surf) { return surf->m_d.m_heightbottom; }, //
         [](Surface* surf, float v) { surf->m_d.m_heightbottom = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection(PropertyPane::Section::Physics))
   {
      props.EndSection();
   }

   props.TimerSection<Surface>(m_surface, [](Surface* obj) { return &(obj->m_d.m_tdr); });
}

}