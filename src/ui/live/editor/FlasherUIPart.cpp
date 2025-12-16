#pragma once

#include "core/stdafx.h"

#include "FlasherUIPart.h"

namespace VPX::EditorUI
{

FlasherUIPart::FlasherUIPart(Flasher* flasher)
   : m_flasher(flasher)
   , m_visible(flasher->m_d.m_isVisible)
{
}

FlasherUIPart::~FlasherUIPart() { m_flasher->m_d.m_isVisible = m_visible; }

FlasherUIPart::TransformMask FlasherUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D trans = Matrix3D::MatrixTranslate(m_flasher->m_d.m_vCenter.x, m_flasher->m_d.m_vCenter.y, m_flasher->m_d.m_height);
   const Matrix3D rotx = Matrix3D::MatrixRotateX(ANGTORAD(m_flasher->m_d.m_rotX));
   const Matrix3D roty = Matrix3D::MatrixRotateY(ANGTORAD(m_flasher->m_d.m_rotY));
   const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(m_flasher->m_d.m_rotZ));
   transform = rotz * roty * rotx * trans;
   return static_cast<TransformMask>(TM_TransAny | TM_RotAny);
}

void FlasherUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const float px = m_flasher->m_d.m_vCenter.x, py = m_flasher->m_d.m_vCenter.y;
   m_flasher->TranslatePoints(Vertex2D { pos.x - px, pos.y - py });
   m_flasher->put_Height(pos.z);
   m_flasher->put_RotX(rot.x);
   m_flasher->put_RotY(rot.y);
   m_flasher->put_RotZ(rot.z);
}

void FlasherUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_flasher->m_d.m_isVisible;

   const bool isUIVisible = m_flasher->IsVisible(m_flasher);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_flasher->m_d.m_isVisible = true;
      ctx.DrawWireframe(m_flasher);
   }

   m_flasher->m_d.m_isVisible = isUIVisible && m_visible;
}

void FlasherUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Flasher", m_flasher);

   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      props.Checkbox<Flasher>(
         m_flasher, "Visible"s, //
         [this](const Flasher* flasher) { return flasher == m_flasher ? m_visible : flasher->m_d.m_isVisible; }, //
         [this](Flasher* flasher, bool v) { flasher->m_d.m_isVisible = v; (flasher == m_flasher ? m_visible : flasher->m_d.m_isVisible) = v; });
      props.Combo<Flasher>(
         m_flasher, "Render Mode"s, vector { "Flasher"s, "DMD"s, "Display"s, "Alpha.Seg."s }, //
         [](const Flasher* flasher) { return flasher->m_d.m_renderMode; }, //
         [](Flasher* flasher, int v) { flasher->m_d.m_renderMode = static_cast<FlasherData::RenderMode>(v); });
      props.InputRGB<Flasher>(
         m_flasher, "Color", //
         [](const Flasher* flasher) { return convertColor(flasher->m_d.m_color); }, //
         [](Flasher* flasher, const vec3& v) { flasher->m_d.m_color = convertColorRGB(v); });
      // Missing Tex coord mode
      props.InputFloat<Flasher>(
         m_flasher, "Depth bias"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_depthBias; }, //
         [](Flasher* flasher, float v) { flasher->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);

      switch (props.GetEditedPart<Flasher>(m_flasher)->m_d.m_renderMode)
      {
      case FlasherData::FLASHER:
         props.ImageCombo<Flasher>(
            m_flasher, "Image A"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageA; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageA = v; });
         props.ImageCombo<Flasher>(
            m_flasher, "Image B"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageB; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageB = v; });
         // Missing Mode
         // Missing Filter Image B
         // Missing Amount
         break;

      case FlasherData::DMD:
         props.Combo<Flasher>(
            m_flasher, "Render Mode"s, vector { "Legacy VPX"s, "Neon Plasma"s, "Red LED"s, "Green LED"s, "Yellow LED"s, "Generic Plasma"s, "Generic LED"s }, //
            [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         // Missing source
         props.ImageCombo<Flasher>(
            m_flasher, "Glass"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageA; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageA = v; });
         props.InputFloat<Flasher>(
            m_flasher, "Glass Roughness"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassRoughness; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassRoughness = v; }, PropertyPane::Unit::None, 0);
         props.InputRGB<Flasher>(
            m_flasher, "Glass Ambient", //
            [](const Flasher* flasher) { return convertColor(flasher->m_d.m_glassAmbient); }, //
            [](Flasher* flasher, const vec3& v) { flasher->m_d.m_glassAmbient = convertColorRGB(v); });
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Left"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadLeft; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadLeft = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Right"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadRight; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadRight = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Top"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadTop; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadTop = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Bottom"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadBottom; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadBottom = v; }, PropertyPane::Unit::Percent, 1);
         break;

      case FlasherData::DISPLAY:
         props.Combo<Flasher>(
            m_flasher, "Render Mode"s, vector { "Pixelated"s, "Smoothed"s, "CRT"s }, //
            [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         // Missing source
         break;

      case FlasherData::ALPHASEG:
         // FIXME missing everything
         break;
      }
      props.EndSection();
   }

   if (props.BeginSection(PropertyPane::Section::Transparency))
   {
      // PropInt("Opacity", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_alpha) : nullptr, live_obj ? &(live_obj->m_d.m_alpha) : nullptr);
      props.LightmapCombo<Flasher>(
         m_flasher, "Image A"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_szLightmap; }, //
         [](Flasher* flasher, const string& v) { flasher->m_d.m_szLightmap = v; });
      props.Checkbox<Flasher>(
         m_flasher, "Additive Blend"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_addBlend; }, //
         [](Flasher* flasher, bool v) { flasher->m_d.m_addBlend = v; });
      props.InputFloat<Flasher>(
         m_flasher, "Modulate"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_modulate_vs_add; }, //
         [](Flasher* flasher, float v) { flasher->m_d.m_modulate_vs_add = v; }, PropertyPane::Unit::Percent, 1);
      props.EndSection();
   }

   if (props.BeginSection(PropertyPane::Section::Position))
   {
      props.InputFloat3<Flasher>(
         m_flasher, "Position", //
         [](const Flasher* flasher) { return vec3(flasher->m_d.m_vCenter.x, flasher->m_d.m_vCenter.y, flasher->m_d.m_height); }, //
         [](Flasher* flasher, const vec3& v)
         {
            const float px = flasher->m_d.m_vCenter.x, py = flasher->m_d.m_vCenter.y;
            flasher->TranslatePoints(Vertex2D { v.x - px, v.y - py });
            flasher->put_Height(v.z);
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Flasher>(
         m_flasher, "Rotation", //
         [](const Flasher* flasher) { return vec3(flasher->m_d.m_rotX, flasher->m_d.m_rotY, flasher->m_d.m_rotZ); }, //
         [](Flasher* flasher, const vec3& v)
         {
            flasher->m_d.m_rotX = v.x;
            flasher->m_d.m_rotY = v.y;
            flasher->m_d.m_rotZ = v.z;
         },
         PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   props.TimerSection<Flasher>(m_flasher, [](Flasher* obj) { return &(obj->m_d.m_tdr); });
}

}