#include "core/stdafx.h"

#include "SurfaceUIPart.h"

namespace VPX::EditorUI
{

SurfaceUIPart::SurfaceUIPart(Surface* surface)
   : EditableUIPart(surface, { &Data::m_topBottomVisible, &Data::m_sideVisible })
{
}

SurfaceUIPart::TransformMask SurfaceUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D& center = m_part->m_curve.GetCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop));
   return TM_TransAny;
}

void SurfaceUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D& center = m_part->m_curve.GetCenter();
   m_part->m_curve.TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
   const float pz = 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop);
   m_part->m_d.m_heightbottom += pos.z - pz;
   m_part->m_d.m_heighttop += pos.z - pz;
}

void SurfaceUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void SurfaceUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Surface"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Surface>(
         m_part, "Top Visible"s, //
         [this](const Surface* surf) { return GetVisibility(surf, &Data::m_topBottomVisible); }, //
         [this](Surface* surf, bool v) { SetVisibility(surf, v, &Data::m_topBottomVisible); });
      props.ImageCombo<Surface>(
         m_part, "Top Image"s, //
         [](const Surface* surf) { return surf->m_d.m_szImage; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szImage = v; });
      props.MaterialCombo<Surface>(
         m_part, "Top Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szTopMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szTopMaterial = v; });

      props.Checkbox<Surface>(
         m_part, "Side Visible"s, //
         [this](const Surface* surf) { return GetVisibility(surf, &Data::m_sideVisible); }, //
         [this](Surface* surf, bool v) { SetVisibility(surf, v, &Data::m_sideVisible); });
      props.ImageCombo<Surface>(
         m_part, "Side Image"s, //
         [](const Surface* surf) { return surf->m_d.m_szSideImage; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSideImage = v; });
      props.MaterialCombo<Surface>(
         m_part, "Side Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szSideMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSideMaterial = v; });
      props.MaterialCombo<Surface>(
         m_part, "Slingshot Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szSlingShotMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSlingShotMaterial = v; });

      props.Checkbox<Surface>(
         m_part, "Animate Slingshot"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshotAnimation; }, //
         [](Surface* surf, bool v) { surf->m_d.m_slingshotAnimation = v; });
      props.Checkbox<Surface>(
         m_part, "Hide when dropped"s, //
         [](const Surface* surf) { return surf->m_d.m_flipbook; }, //
         [](Surface* surf, bool v) { surf->m_d.m_flipbook = v; });

      props.InputFloat<Surface>(
         m_part, "Disable Spot Lights"s, //
         [](const Surface* surf) { return surf->m_d.m_disableLightingTop; }, //
         [](Surface* surf, float v) { surf->m_d.m_disableLightingTop = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Surface>(
         m_part, "Translucency"s, //
         [](const Surface* surf) { return surf->m_d.m_disableLightingBelow; }, //
         [](Surface* surf, float v) { surf->m_d.m_disableLightingBelow = v; }, PropertyPane::Unit::None, 1);
      props.Checkbox<Surface>(
         m_part, "Reflection Enabled"s, //
         [](const Surface* surf) { return surf->m_d.m_reflectionEnabled; }, //
         [](Surface* surf, bool v) { surf->m_d.m_reflectionEnabled = v; });
      props.InputFloat<Surface>(
         m_part, "Top Height"s, //
         [](const Surface* surf) { return surf->m_d.m_heighttop; }, //
         [](Surface* surf, float v) { surf->m_d.m_heighttop = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Surface>(
         m_part, "Bottom Height"s, //
         [](const Surface* surf) { return surf->m_d.m_heightbottom; }, //
         [](Surface* surf, float v) { surf->m_d.m_heightbottom = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Surface>(
         m_part, "Has Hit Event"s, //
         [](const Surface* surf) { return surf->m_d.m_hitEvent; }, //
         [](Surface* surf, bool v) { surf->m_d.m_hitEvent = v; });
      props.InputFloat<Surface>(
         m_part, "Hit Threshold"s, //
         [](const Surface* surf) { return surf->m_d.m_threshold; }, //
         [](Surface* surf, float v) { surf->m_d.m_threshold = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Surface>(
         m_part, "Slingshot Force"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshotforce; }, //
         [](Surface* surf, float v) { surf->m_d.m_slingshotforce = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Surface>(
         m_part, "Slingshot Threshold"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshot_threshold; }, //
         [](Surface* surf, float v) { surf->m_d.m_slingshot_threshold = v; }, PropertyPane::Unit::None, 2);
      props.MaterialCombo<Surface>(
         m_part, "Physics Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szPhysicsMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<Surface>(
         m_part, "Overwrite Material"s, //
         [](const Surface* surf) { return surf->m_d.m_overwritePhysics; }, //
         [](Surface* surf, bool v) { surf->m_d.m_overwritePhysics = v; });
      props.InputFloat<Surface>(
         m_part, "Elasticity"s, //
         [](const Surface* surface) { return surface->m_d.m_elasticity; }, //
         [](Surface* surface, float v) { surface->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Surface>(
         m_part, "Elasticity Falloff"s, //
         [](const Surface* surface) { return surface->m_d.m_elasticityFalloff; }, //
         [](Surface* surface, float v) { surface->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Surface>(
         m_part, "Friction"s, //
         [](const Surface* surface) { return surface->m_d.m_friction; }, //
         [](Surface* surface, float v) { surface->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Surface>(
         m_part, "Scatter Angle"s, //
         [](const Surface* surface) { return surface->m_d.m_scatter; }, //
         [](Surface* surface, float v) { surface->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Surface>(
         m_part, "Can Drop"s, //
         [](const Surface* surf) { return surf->m_d.m_droppable; }, //
         [](Surface* surf, bool v) { surf->m_d.m_droppable = v; });
      props.Checkbox<Surface>(
         m_part, "Collidable"s, //
         [](const Surface* surf) { return surf->m_d.m_collidable; }, //
         [](Surface* surf, bool v) { surf->m_d.m_collidable = v; });
      props.Checkbox<Surface>(
         m_part, "Bottom Collidable"s, //
         [](const Surface* surf) { return surf->m_d.m_isBottomSolid; }, //
         [](Surface* surf, bool v) { surf->m_d.m_isBottomSolid = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
