#include "core/stdafx.h"

#include "SurfaceUIPart.h"

#include "math/matrix.h"

namespace VPX::EditorUI
{

SurfaceUIPart::SurfaceUIPart(Surface* surface)
   : EditableUIPart(surface, { &Data::m_topBottomVisible, &Data::m_sideVisible })
{
}

SurfaceUIPart::TransformMask SurfaceUIPart::GetTransform(Matrix3D& transform) { return GetCurveTransform(transform, 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop)); }

void SurfaceUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   SetCurveTransform(pos, scale, rot);
   const float pz = 0.5f * (m_part->m_d.m_heightbottom + m_part->m_d.m_heighttop);
   m_part->m_d.m_heightbottom += pos.z - pz;
   m_part->m_d.m_heighttop += pos.z - pz;
}

void SurfaceUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void SurfaceUIPart::UpdatePointZField(PropertyPane& props, DragPoint* point)
{
   // Surface drag points have no Z of their own: show the surface height range instead
   ImGui::BeginDisabled();
   props.InputFloat2<Surface>(
      m_part, "Z Range"s, //
      [](const Surface* surf) { return Vertex2D(surf->m_d.m_heightbottom, surf->m_d.m_heighttop); }, //
      [](Surface*, const Vertex2D&) {}, PropertyPane::Unit::VPLength, 1);
   ImGui::EndDisabled();
}

void SurfaceUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Surface"s, m_part);

   UpdateCurveSection(props);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Surface>(
         m_part, "Top Visible"s, //
         [this](const Surface* surf) { return GetVisibility(surf, &Data::m_topBottomVisible); }, //
         [this](Surface* surf, bool v) { SetVisibility(surf, v, &Data::m_topBottomVisible); });
      props.MaterialVisuals(m_part, "Top Material"s, &Data::m_szTopMaterial, "Top Image"s, &Data::m_szImage, &Data::m_displayTexture);

      props.Checkbox<Surface>(
         m_part, "Side Visible"s, //
         [this](const Surface* surf) { return GetVisibility(surf, &Data::m_sideVisible); }, //
         [this](Surface* surf, bool v) { SetVisibility(surf, v, &Data::m_sideVisible); });
      props.MaterialVisuals(m_part, "Side Material"s, &Data::m_szSideMaterial, "Side Image"s, &Data::m_szSideImage);
      props.MaterialCombo<Surface>(
         m_part, "Slingshot Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szSlingShotMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szSlingShotMaterial = v; }, "Material used for the curve segments flagged as slingshots");

      props.Checkbox<Surface>(
         m_part, "Animate Slingshot"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshotAnimation; }, //
         [](Surface* surf, bool v) { surf->m_d.m_slingshotAnimation = v; }, "Play an animation on slingshot segments when they fire");
      props.Checkbox<Surface>(
         m_part, "Hide when dropped"s, //
         [](const Surface* surf) { return surf->m_d.m_flipbook; }, //
         [](Surface* surf, bool v) { surf->m_d.m_flipbook = v; }, "Drop the surface into the playfield when hit, like a drop target");

      props.InputFloat<Surface>(
         m_part, "Disable Spot Lights"s, //
         [](const Surface* surf) { return surf->m_d.m_disableLightingTop; }, //
         [](Surface* surf, float v) { surf->m_d.m_disableLightingTop = v; }, PropertyPane::Unit::None, 2,
         "Reduce the effect of dynamic light sources on the top of this part (0 = fully lit)");
      props.InputFloat<Surface>(
         m_part, "Translucency"s, //
         [](const Surface* surf) { return surf->m_d.m_disableLightingBelow; }, //
         [](Surface* surf, float v) { surf->m_d.m_disableLightingBelow = v; }, PropertyPane::Unit::None, 1, "Let light pass through the part from below, faking translucency");
      props.Checkbox<Surface>(
         m_part, "Reflection Enabled"s, //
         [](const Surface* surf) { return surf->m_d.m_reflectionEnabled; }, //
         [](Surface* surf, bool v) { surf->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
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
         [](Surface* surf, bool v) { surf->m_d.m_hitEvent = v; }, "Fire a Hit event to the script when the ball hits this part");
      props.InputFloat<Surface>(
         m_part, "Hit Threshold"s, //
         [](const Surface* surf) { return surf->m_d.m_threshold; }, //
         [](Surface* surf, float v) { surf->m_d.m_threshold = v; }, PropertyPane::Unit::None, 2, "Minimum ball speed needed to trigger this part's hit event");
      props.InputFloat<Surface>(
         m_part, "Slingshot Force"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshotforce; }, //
         [](Surface* surf, float v) { surf->m_d.m_slingshotforce = v; }, PropertyPane::Unit::None, 2, "Force of the kick applied to the ball by slingshot segments");
      props.InputFloat<Surface>(
         m_part, "Slingshot Threshold"s, //
         [](const Surface* surf) { return surf->m_d.m_slingshot_threshold; }, //
         [](Surface* surf, float v) { surf->m_d.m_slingshot_threshold = v; }, PropertyPane::Unit::None, 2, "Minimum ball speed needed for a slingshot segment to fire");
      props.MaterialCombo<Surface>(
         m_part, "Physics Material"s, //
         [](const Surface* surf) { return surf->m_d.m_szPhysicsMaterial; }, //
         [](Surface* surf, const string& v) { surf->m_d.m_szPhysicsMaterial = v; }, "Material preset used for this part's physics values");
      props.Checkbox<Surface>(
         m_part, "Overwrite Material"s, //
         [](const Surface* surf) { return surf->m_d.m_overwritePhysics; }, //
         [](Surface* surf, bool v) { surf->m_d.m_overwritePhysics = v; }, "Use the fields below instead of the physics material's values");
      props.InputFloat<Surface>(
         m_part, "Elasticity"s, //
         [](const Surface* surface) { return surface->m_d.m_elasticity; }, //
         [](Surface* surface, float v) { surface->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Surface>(
         m_part, "Elasticity Falloff"s, //
         [](const Surface* surface) { return surface->m_d.m_elasticityFalloff; }, //
         [](Surface* surface, float v) { surface->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3, "Reduce elasticity for slower impacts (0 = constant elasticity)");
      props.InputFloat<Surface>(
         m_part, "Friction"s, //
         [](const Surface* surface) { return surface->m_d.m_friction; }, //
         [](Surface* surface, float v) { surface->m_d.m_friction = max(v, 0.f); }, PropertyPane::Unit::None, 3);
      props.InputFloat<Surface>(
         m_part, "Scatter Angle"s, //
         [](const Surface* surface) { return surface->m_d.m_scatter; }, //
         [](Surface* surface, float v) { surface->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3, "Random angle added to the ball's bounce direction");
      props.Checkbox<Surface>(
         m_part, "Can Drop"s, //
         [](const Surface* surf) { return surf->m_d.m_droppable; }, //
         [](Surface* surf, bool v) { surf->m_d.m_droppable = v; }, "Allow this surface to be dropped (hidden) by the script");
      props.Checkbox<Surface>(
         m_part, "Collidable"s, //
         [](const Surface* surf) { return surf->m_d.m_collidable; }, //
         [](Surface* surf, bool v) { surf->m_d.m_collidable = v; });
      props.Checkbox<Surface>(
         m_part, "Bottom Collidable"s, //
         [](const Surface* surf) { return surf->m_d.m_isBottomSolid; }, //
         [](Surface* surf, bool v) { surf->m_d.m_isBottomSolid = v; }, "Make the surface's bottom face solid as well as its sides");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
