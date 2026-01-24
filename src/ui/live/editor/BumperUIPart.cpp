#include "core/stdafx.h"

#include "BumperUIPart.h"

namespace VPX::EditorUI
{

BumperUIPart::BumperUIPart(Bumper* bumper)
   : m_bumper(bumper)
   , m_baseVisible(bumper->m_d.m_baseVisible)
   , m_capVisible(bumper->m_d.m_capVisible)
   , m_skirtVisible(bumper->m_d.m_skirtVisible)
   , m_ringVisible(bumper->m_d.m_ringVisible)
{
}

BumperUIPart::~BumperUIPart()
{
   m_bumper->m_d.m_baseVisible = m_baseVisible;
   m_bumper->m_d.m_capVisible = m_capVisible;
   m_bumper->m_d.m_skirtVisible = m_skirtVisible;
   m_bumper->m_d.m_ringVisible = m_ringVisible;
}

BumperUIPart::TransformMask BumperUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_bumper->GetPTable()->GetSurfaceHeight(m_bumper->m_d.m_szSurface, m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void BumperUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_bumper->m_d.m_vCenter.x = pos.x;
   m_bumper->m_d.m_vCenter.y = pos.y;
}

void BumperUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
   {
      m_baseVisible = m_bumper->m_d.m_baseVisible;
      m_capVisible = m_bumper->m_d.m_capVisible;
      m_skirtVisible = m_bumper->m_d.m_skirtVisible;
      m_ringVisible = m_bumper->m_d.m_ringVisible;
   }

   const bool isUIVisible = m_bumper->IsVisible(m_bumper);
   if (isUIVisible && (ctx.IsSelected() || (!m_baseVisible && !m_capVisible && !m_skirtVisible && !m_ringVisible && ctx.IsShowInvisible())))
   {
      m_bumper->m_d.m_baseVisible = true;
      m_bumper->m_d.m_capVisible = true;
      m_bumper->m_d.m_skirtVisible = true;
      m_bumper->m_d.m_ringVisible = true;
      ctx.DrawWireframe(m_bumper);
   }

   m_bumper->m_d.m_baseVisible = isUIVisible && m_baseVisible;
   m_bumper->m_d.m_capVisible = isUIVisible && m_capVisible;
   m_bumper->m_d.m_skirtVisible = isUIVisible && m_skirtVisible;
   m_bumper->m_d.m_ringVisible = isUIVisible && m_ringVisible;
}

void BumperUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Bumper"s, m_bumper);

   if (props.BeginSection("Visuals"s))
   {
      props.InputFloat<Bumper>(
         m_bumper, "Radius"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_radius; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Bumper>(
         m_bumper, "Height Scale"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_heightScale; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_heightScale = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Bumper>(
         m_bumper, "Orientation"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_orientation; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_orientation = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Bumper>(
         m_bumper, "Ring Speed"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_ringSpeed; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_ringSpeed = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Bumper>(
         m_bumper, "Ring Drop"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_ringDropOffset; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_ringDropOffset = v; }, PropertyPane::Unit::None, 1);

      props.Checkbox<Bumper>(
         m_bumper, "Cap Visible"s, //
         [this](const Bumper* bumper) { return m_bumper == bumper ? m_capVisible : bumper->m_d.m_capVisible; }, //
         [this](Bumper* bumper, bool v) { bumper->m_d.m_capVisible = v; (m_bumper == bumper ? m_capVisible : bumper->m_d.m_capVisible) = v; });
      props.MaterialCombo<Bumper>(
         m_bumper, "Cap Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szCapMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szCapMaterial = v; });

      props.Checkbox<Bumper>(
         m_bumper, "Base Visible"s, //
         [this](const Bumper* bumper) { return m_bumper == bumper ? m_baseVisible : bumper->m_d.m_baseVisible; }, //
         [this](Bumper* bumper, bool v) { bumper->m_d.m_baseVisible = v; (m_bumper == bumper ? m_baseVisible : bumper->m_d.m_baseVisible) = v; });
      props.MaterialCombo<Bumper>(
         m_bumper, "Base Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szBaseMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szBaseMaterial = v; });

      props.Checkbox<Bumper>(
         m_bumper, "Skirt Visible"s, //
         [this](const Bumper* bumper) { return m_bumper == bumper ? m_skirtVisible : bumper->m_d.m_skirtVisible; }, //
         [this](Bumper* bumper, bool v) { bumper->m_d.m_skirtVisible = v; (m_bumper == bumper ? m_skirtVisible : bumper->m_d.m_skirtVisible) = v; });
      props.MaterialCombo<Bumper>(
         m_bumper, "Skirt Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szSkirtMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szSkirtMaterial = v; });

      props.Checkbox<Bumper>(
         m_bumper, "Ring Visible"s, //
         [this](const Bumper* bumper) { return m_bumper == bumper ? m_ringVisible : bumper->m_d.m_ringVisible; }, //
         [this](Bumper* bumper, bool v) { bumper->m_d.m_ringVisible = v; (m_bumper == bumper ? m_ringVisible : bumper->m_d.m_ringVisible) = v; });
      props.MaterialCombo<Bumper>(
         m_bumper, "Ring Material"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szRingMaterial; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szRingMaterial = v; });

      props.Checkbox<Bumper>(
         m_bumper, "Reflection Enabled"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_reflectionEnabled; }, //
         [](Bumper* bumper, bool v) { bumper->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Bumper>(
         m_bumper, "Position"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_vCenter; }, //
         [](Bumper* bumper, const Vertex2D& v) { bumper->Translate(Vertex2D(v.x - bumper->m_d.m_vCenter.x, v.y - bumper->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Bumper>(
         m_bumper, "Surface"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_szSurface; }, //
         [](Bumper* bumper, const string& v) { bumper->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Bumper>(
         m_bumper, "Has Hit Event"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_hitEvent; }, //
         [](Bumper* bumper, bool v) { bumper->m_d.m_hitEvent = v; });
      props.InputFloat<Bumper>(
         m_bumper, "Force"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_force; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_force = v; }, PropertyPane::Unit::VPLength, 3);
      props.InputFloat<Bumper>(
         m_bumper, "Hit Threshold"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_heightScale; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_heightScale = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Bumper>(
         m_bumper, "Scatter Angle"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_scatter; }, //
         [](Bumper* bumper, float v) { bumper->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Bumper>(
         m_bumper, "Collidable"s, //
         [](const Bumper* bumper) { return bumper->m_d.m_collidable; }, //
         [](Bumper* bumper, bool v) { bumper->m_d.m_collidable = v; });
      props.EndSection();
   }

   props.TimerSection<Bumper>(m_bumper, [](Bumper* obj) { return &(obj->m_d.m_tdr); });
}

}
