#pragma once

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

RubberUIPart::TransformMask RubberUIPart::GetTransform(Matrix3D& transform) { return TM_None; }

void RubberUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { }

void RubberUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_rubber->m_d.m_visible;

   const bool isUIVisible = m_rubber->IsVisible(m_rubber);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_rubber->m_d.m_visible = true;
      ctx.DrawWireframe(m_rubber);
   }

   m_rubber->m_d.m_visible = isUIVisible && m_visible;
}

void RubberUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Rubber", m_rubber);

   if (props.BeginSection(PropertyPane::Section::Visual))
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
         [this](Rubber* rubber, bool v) { rubber->m_d.m_visible = v; (rubber == m_rubber ? m_visible : rubber->m_d.m_visible) = v; });
      props.Checkbox<Rubber>(
         m_rubber, "Reflection Enabled"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_reflectionEnabled; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_reflectionEnabled = v; });
      props.InputFloat<Rubber>(
         m_rubber, "Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_height; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputInt<Rubber>(
         m_rubber, "Thickness"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_thickness; }, //
         [](Rubber* rubber, int v) { rubber->m_d.m_thickness = v; });
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

   props.TimerSection<Rubber>(m_rubber, [](Rubber* obj) { return &(obj->m_d.m_tdr); });
}

}