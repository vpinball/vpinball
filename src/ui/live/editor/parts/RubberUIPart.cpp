#include "core/stdafx.h"

#include "RubberUIPart.h"

#include "math/matrix.h"

namespace VPX::EditorUI
{

RubberUIPart::RubberUIPart(Rubber* rubber)
   : EditableUIPart(rubber, { &Data::m_visible })
{
}

RubberUIPart::TransformMask RubberUIPart::GetTransform(Matrix3D& transform) { return GetCurveTransform(transform, m_part->m_d.m_height); }

void RubberUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   SetCurveTransform(pos, scale, rot);
   m_part->m_d.m_height += pos.z - m_part->m_d.m_height;
}

void RubberUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void RubberUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Rubber"s, m_part);

   UpdateCurveSection(props);

   if (props.BeginSection("Visuals"s))
   {
      props.MaterialVisuals(m_part, "Material"s, &Data::m_szMaterial, "Image"s, &Data::m_szImage);
      props.Checkbox<Rubber>(
         m_part, "Static Rendering"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_staticRendering; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_staticRendering = v; }, "Bake this part into the static prerender pass (faster, but it cannot move or change at runtime)");
      props.Checkbox<Rubber>(
         m_part, "Visible"s, //
         [this](const Rubber* rubber) { return GetVisibility(rubber); }, //
         [this](Rubber* rubber, bool v) { SetVisibility(rubber, v); });
      props.Checkbox<Rubber>(
         m_part, "Reflection Enabled"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_reflectionEnabled; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
      props.InputInt<Rubber>(
         m_part, "Thickness"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_thickness; }, //
         [](Rubber* rubber, int v) { rubber->m_d.m_thickness = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat<Rubber>(
         m_part, "Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_height; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Rubber>(
         m_part, "Rotation"s, //
         [](const Rubber* rubber) { return vec3(rubber->m_d.m_rotX, rubber->m_d.m_rotY, rubber->m_d.m_rotZ); }, //
         [](Rubber* rubber, const vec3& v)
         {
            rubber->m_d.m_rotX = v.x;
            rubber->m_d.m_rotY = v.y;
            rubber->m_d.m_rotZ = v.z;
         },
         PropertyPane::Unit::Degree, 1);
      props.Checkbox<Rubber>(
         m_part, "Show in Editor"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_showInEditor; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_showInEditor = v; }, "Draw this rubber in the editor view (does not affect gameplay)");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.MaterialCombo<Rubber>(
         m_part, "Physics Material"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szPhysicsMaterial; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szPhysicsMaterial = v; }, "Material preset used for this part's physics values");
      props.Checkbox<Rubber>(
         m_part, "Overwrite Physics"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_overwritePhysics; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_overwritePhysics = v; }, "Use the fields below instead of the physics material's values");
      props.InputFloat<Rubber>(
         m_part, "Elasticy"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticity; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Elasticy Falloff"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticityFalloff; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 2, "Reduce elasticity for slower impacts (0 = constant elasticity)");
      props.InputFloat<Rubber>(
         m_part, "Friction"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_friction; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_friction = max(v, 0.f); }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Scatter Angle"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_scatter; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2, "Random angle added to the ball's bounce direction");
      props.InputFloat<Rubber>(
         m_part, "Hit Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitHeight; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_hitHeight = v; }, PropertyPane::Unit::VPLength, 2, "Height of the collision volume above the table surface");
      props.Checkbox<Rubber>(
         m_part, "Collidable"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_collidable; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_collidable = v; });
      props.Checkbox<Rubber>(
         m_part, "Has Hit Event"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitEvent; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_hitEvent = v; }, "Fire a Hit event to the script when the ball hits this part");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
