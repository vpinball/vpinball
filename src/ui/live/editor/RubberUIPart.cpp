#include "core/stdafx.h"

#include "RubberUIPart.h"

namespace VPX::EditorUI
{

RubberUIPart::RubberUIPart(Rubber* rubber)
   : m_rubber(rubber)
   , m_visible(rubber->m_d.m_visible)
{
}

RubberUIPart::~RubberUIPart() { m_rubber->m_d.m_visible = m_visible; }

RubberUIPart::TransformMask RubberUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D center = m_rubber->GetPointCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, m_rubber->m_d.m_height);
   return TM_TransAny;
}

void RubberUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center = m_rubber->GetPointCenter();
   m_rubber->TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
   m_rubber->GetPTable()->m_undo.Undo(true);
   m_rubber->m_d.m_height += pos.z - m_rubber->m_d.m_height;
}

void RubberUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_rubber->m_d.m_visible;

   const bool isUIVisible = m_rubber->IsVisible(m_rubber);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_rubber->m_d.m_visible = true;
      ctx.DrawWireframe(m_rubber);
   }

   m_rubber->m_d.m_visible = isUIVisible && m_visible;
}

void RubberUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Rubber"s, m_rubber);

   if (props.BeginSection("Visuals"s))
   {
      props.ImageCombo<Rubber>(
         m_rubber, "Image"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szImage; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szImage = v; });
      props.MaterialCombo<Rubber>(
         m_rubber, "Material"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szMaterial; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szMaterial = v; });
      props.Checkbox<Rubber>(
         m_rubber, "Static Rendering"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_staticRendering; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_staticRendering = v; });
      props.Checkbox<Rubber>(
         m_rubber, "Visible"s, //
         [this](const Rubber* rubber) { return rubber == m_rubber ? m_visible : rubber->m_d.m_visible; }, //
         [this](Rubber* rubber, bool v)
         {
            rubber->m_d.m_visible = v;
            (rubber == m_rubber ? m_visible : rubber->m_d.m_visible) = v;
         });
      props.Checkbox<Rubber>(
         m_rubber, "Reflection Enabled"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_reflectionEnabled; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_reflectionEnabled = v; });
      props.InputInt<Rubber>(
         m_rubber, "Thickness"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_thickness; }, //
         [](Rubber* rubber, int v) { rubber->m_d.m_thickness = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat<Rubber>(
         m_rubber, "Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_height; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Rubber>(
         m_rubber, "Rotation"s, //
         [](const Rubber* rubber) { return vec3(rubber->m_d.m_rotX, rubber->m_d.m_rotY, rubber->m_d.m_rotZ); }, //
         [](Rubber* rubber, const vec3& v)
         {
            rubber->m_d.m_rotX = v.x;
            rubber->m_d.m_rotY = v.y;
            rubber->m_d.m_rotZ = v.z;
         },
         PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.MaterialCombo<Rubber>(
         m_rubber, "Physics Material"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szPhysicsMaterial; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<Rubber>(
         m_rubber, "Overwrite Physics"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_overwritePhysics; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_overwritePhysics = v; });
      props.InputFloat<Rubber>(
         m_rubber, "Elasticy"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticity; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_rubber, "Elasticy Falloff"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticityFalloff; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_rubber, "Friction"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_friction; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_friction = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_rubber, "Scatter Angle"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_scatter; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_rubber, "Hit Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitHeight; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_hitHeight = v; }, PropertyPane::Unit::VPLength, 2);
      props.Checkbox<Rubber>(
         m_rubber, "Collidable"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_collidable; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_collidable = v; });
      props.Checkbox<Rubber>(
         m_rubber, "Has Hit Event"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitEvent; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_hitEvent = v; });
      props.EndSection();
   }

   props.TimerSection<Rubber>(m_rubber, [](Rubber* obj) { return &(obj->m_d.m_tdr); });
}

}
