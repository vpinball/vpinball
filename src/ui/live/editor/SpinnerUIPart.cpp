#include "core/stdafx.h"

#include "SpinnerUIPart.h"

namespace VPX::EditorUI
{

SpinnerUIPart::SpinnerUIPart(Spinner* spinner)
   : m_spinner(spinner)
   , m_visible(spinner->m_d.m_visible)
{
}

SpinnerUIPart::~SpinnerUIPart() { m_spinner->m_d.m_visible = m_visible; }

SpinnerUIPart::TransformMask SpinnerUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_spinner->GetPTable()->GetSurfaceHeight(m_spinner->m_d.m_szSurface, m_spinner->m_d.m_vCenter.x, m_spinner->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_spinner->m_d.m_vCenter.x, m_spinner->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void SpinnerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
   m_spinner->m_d.m_vCenter.x = pos.x;
   m_spinner->m_d.m_vCenter.y = pos.y;
}

void SpinnerUIPart::Render(const EditorRenderContext& ctx)
{ 
   if (ctx.NeedsLiveTableSync())
      m_visible = m_spinner->m_d.m_visible;

   const bool isUIVisible = m_spinner->IsVisible(m_spinner);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_spinner->m_d.m_visible = true;
      ctx.DrawWireframe(m_spinner);
   }

   m_spinner->m_d.m_visible = isUIVisible && m_visible;
}

void SpinnerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Spinner"s, m_spinner);
   
   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Spinner>(
         m_spinner, "Visible"s, //
         [this](const Spinner* spinner) { return spinner == m_spinner ? m_visible : spinner->m_d.m_visible; }, //
         [this](Spinner* spinner, bool v)
         {
            spinner->m_d.m_visible = v;
            (spinner == m_spinner ? m_visible : spinner->m_d.m_visible) = v;
         });
      props.ImageCombo<Spinner>(
         m_spinner, "Image"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_szImage; }, //
         [](Spinner* spinner, const string& v) { spinner->m_d.m_szImage = v; });
      props.MaterialCombo<Spinner>(
         m_spinner, "Material"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_szMaterial; }, //
         [](Spinner* spinner, const string& v) { spinner->m_d.m_szMaterial = v; });
      props.Checkbox<Spinner>(
         m_spinner, "Show Bracket"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_showBracket; }, //
         [](Spinner* spinner, bool v) { spinner->m_d.m_showBracket = v; });
      props.Checkbox<Spinner>(
         m_spinner, "Reflection Enabled"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_reflectionEnabled; }, //
         [](Spinner* spinner, bool v) { spinner->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Spinner>(
         m_spinner, "Position"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_vCenter; }, //
         [](Spinner* spinner, const Vertex2D& v) { spinner->Translate(Vertex2D(v.x - spinner->m_d.m_vCenter.x, v.y - spinner->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Spinner>(
         m_spinner, "Surface"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_szSurface; }, //
         [](Spinner* spinner, const string& v) { spinner->m_d.m_szSurface = v; });
      props.InputFloat<Spinner>(
         m_spinner, "Length"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_length; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_length = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Spinner>(
         m_spinner, "Height"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_height; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Spinner>(
         m_spinner, "Rotation"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_rotation; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Spinner>(
         m_spinner, "Angle Max"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_angleMax; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_angleMax = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Spinner>(
         m_spinner, "Angle Min"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_angleMin; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_angleMin = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.InputFloat<Spinner>(
         m_spinner, "Elasticy"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_elasticity; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Spinner>(
         m_spinner, "Damping"s, //
         [](const Spinner* spinner) { return spinner->m_d.m_damping; }, //
         [](Spinner* spinner, float v) { spinner->m_d.m_damping = v; }, PropertyPane::Unit::None, 2);
      props.EndSection();
   }

   props.TimerSection<Spinner>(m_spinner, [](Spinner* obj) { return &(obj->m_d.m_tdr); });
}

}
