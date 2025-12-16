#pragma once

#include "core/stdafx.h"

#include "RampUIPart.h"

namespace VPX::EditorUI
{

RampUIPart::RampUIPart(Ramp* ramp)
   : m_ramp(ramp)
   , m_visible(ramp->m_d.m_visible)
{
}

RampUIPart::~RampUIPart() { m_ramp->m_d.m_visible = m_visible; }

RampUIPart::TransformMask RampUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D center = m_ramp->GetPointCenter();
   transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (m_ramp->m_d.m_heightbottom + m_ramp->m_d.m_heighttop));
   return TM_TransAny;
}

void RampUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center = m_ramp->GetPointCenter();
   m_ramp->TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
   const float pz = 0.5f * (m_ramp->m_d.m_heightbottom + m_ramp->m_d.m_heighttop);
   m_ramp->m_d.m_heightbottom += pos.z - pz;
   m_ramp->m_d.m_heighttop += pos.z - pz;
}

void RampUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_ramp->m_d.m_visible;

   const bool isUIVisible = m_ramp->IsVisible(m_ramp);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_ramp->m_d.m_visible = true;
      ctx.DrawWireframe(m_ramp);
   }

   m_ramp->m_d.m_visible = isUIVisible && m_visible;
}

void RampUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Ramp", m_ramp);

   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      // Missing type
      props.ImageCombo<Ramp>(
         m_ramp, "Image"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_szImage; }, //
         [](Ramp* ramp, const string& v) { ramp->m_d.m_szImage = v; });
      props.MaterialCombo<Ramp>(
         m_ramp, "Material"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_szMaterial; }, //
         [](Ramp* ramp, const string& v) { ramp->m_d.m_szMaterial = v; });
      // Missing World
      props.Checkbox<Ramp>(
         m_ramp, "Apply Image to Wall"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_imageWalls; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_imageWalls = v; });
      props.InputFloat<Ramp>(
         m_ramp, "Depth Bias"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_depthBias; }, //
         [](Ramp* ramp, float v) { ramp->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);
      props.Checkbox<Ramp>(
         m_ramp, "Visible"s, //
         [this](const Ramp* ramp) { return ramp == m_ramp ? m_visible : ramp->m_d.m_visible; }, //
         [this](Ramp* ramp, bool v) { ramp->m_d.m_visible = v; (ramp == m_ramp ? m_visible : ramp->m_d.m_visible) = v; });
      props.Checkbox<Ramp>(
         m_ramp, "Reflection Enabled"s, //
         [](const Ramp* ramp) { return ramp->m_d.m_reflectionEnabled; }, //
         [](Ramp* ramp, bool v) { ramp->m_d.m_reflectionEnabled = v; });
      // Missing all dimensions
      props.EndSection();
   }

   props.TimerSection<Ramp>(m_ramp, [](Ramp* obj) { return &(obj->m_d.m_tdr); });
}

}