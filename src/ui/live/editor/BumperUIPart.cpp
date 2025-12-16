#pragma once

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
   if (isUIVisible && (ctx.IsSelected() || (!m_baseVisible && !m_capVisible && !m_skirtVisible && !m_ringVisible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_bumper->m_d.m_baseVisible = true;
      m_bumper->m_d.m_capVisible = true;
      m_bumper->m_d.m_skirtVisible = true;
      m_bumper->m_d.m_ringVisible = true;
      // FIXME implement wireframe rendering
      ctx.DrawHitObjects(m_bumper);
   }

   m_bumper->m_d.m_baseVisible = isUIVisible && m_baseVisible;
   m_bumper->m_d.m_capVisible = isUIVisible && m_capVisible;
   m_bumper->m_d.m_skirtVisible = isUIVisible && m_skirtVisible;
   m_bumper->m_d.m_ringVisible = isUIVisible && m_ringVisible;
}

void BumperUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Bumper", m_bumper);

   if (props.BeginSection(PropertyPane::Section::Visual))
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

   if (props.BeginSection(PropertyPane::Section::Position))
   {
      // Missing position
      props.EndSection();
   }

   props.TimerSection<Bumper>(m_bumper, [](Bumper* obj) { return &(obj->m_d.m_tdr); });
}

}