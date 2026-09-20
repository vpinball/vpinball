#include "core/stdafx.h"

#include "RubberUIPart.h"

namespace VPX::EditorUI
{

RubberUIPart::RubberUIPart(Rubber* rubber)
   : EditableUIPart(rubber, { &Data::m_visible })
{
}

RubberUIPart::TransformMask RubberUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D& center = m_part->m_curve.GetCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, m_part->m_d.m_height);
   return TM_TransAny;
}

void RubberUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D& center = m_part->m_curve.GetCenter();
   m_part->m_curve.TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
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
      props.ImageCombo<Rubber>(
         m_part, "Image"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szImage; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szImage = v; });
      props.MaterialCombo<Rubber>(
         m_part, "Material"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szMaterial; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szMaterial = v; });
      props.Checkbox<Rubber>(
         m_part, "Static Rendering"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_staticRendering; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_staticRendering = v; });
      props.Checkbox<Rubber>(
         m_part, "Visible"s, //
         [this](const Rubber* rubber) { return GetVisibility(rubber); }, //
         [this](Rubber* rubber, bool v) { SetVisibility(rubber, v); });
      props.Checkbox<Rubber>(
         m_part, "Reflection Enabled"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_reflectionEnabled; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_reflectionEnabled = v; });
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
         [](Rubber* rubber, bool v) { rubber->m_d.m_showInEditor = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.MaterialCombo<Rubber>(
         m_part, "Physics Material"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szPhysicsMaterial; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<Rubber>(
         m_part, "Overwrite Physics"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_overwritePhysics; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_overwritePhysics = v; });
      props.InputFloat<Rubber>(
         m_part, "Elasticy"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticity; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Elasticy Falloff"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticityFalloff; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Friction"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_friction; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_friction = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Scatter Angle"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_scatter; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Hit Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitHeight; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_hitHeight = v; }, PropertyPane::Unit::VPLength, 2);
      props.Checkbox<Rubber>(
         m_part, "Collidable"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_collidable; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_collidable = v; });
      props.Checkbox<Rubber>(
         m_part, "Has Hit Event"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitEvent; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_hitEvent = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
