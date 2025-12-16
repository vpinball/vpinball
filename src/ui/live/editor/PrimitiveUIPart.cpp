#pragma once

#include "core/stdafx.h"

#include "PrimitiveUIPart.h"

namespace VPX::EditorUI
{

PrimitiveUIPart::PrimitiveUIPart(Primitive* primitive)
   : m_primitive(primitive)
   , m_visible(primitive->m_d.m_visible)
{
}

PrimitiveUIPart::~PrimitiveUIPart() { m_primitive->m_d.m_visible = m_visible; }

PrimitiveUIPart::TransformMask PrimitiveUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D Smatrix = Matrix3D::MatrixScale(m_primitive->m_d.m_vSize.x, m_primitive->m_d.m_vSize.y, m_primitive->m_d.m_vSize.z);
   const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(m_primitive->m_d.m_vPosition.x, m_primitive->m_d.m_vPosition.y, m_primitive->m_d.m_vPosition.z);
   const Matrix3D Rmatrix = (Matrix3D::MatrixRotateZ(ANGTORAD(m_primitive->m_d.m_aRotAndTra[2])) //
                               * Matrix3D::MatrixRotateY(ANGTORAD(m_primitive->m_d.m_aRotAndTra[1]))) //
      * Matrix3D::MatrixRotateX(ANGTORAD(m_primitive->m_d.m_aRotAndTra[0]));
   transform = (Smatrix * Rmatrix) * Tmatrix; // fullMatrix = Scale * Rotate * Translate
   return TM_TRSAny;
}

void PrimitiveUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_primitive->m_d.m_vPosition = pos;
   m_primitive->m_d.m_aRotAndTra[0] = rot.x;
   m_primitive->m_d.m_aRotAndTra[1] = rot.y;
   m_primitive->m_d.m_aRotAndTra[2] = rot.z;
   m_primitive->m_d.m_vSize = scale;
}

void PrimitiveUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_primitive->m_d.m_visible;

   const bool isUIVisible = m_primitive->IsVisible(m_primitive);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_primitive->m_d.m_visible = true;
      ctx.DrawWireframe(m_primitive);
   }

   m_primitive->m_d.m_visible = isUIVisible && m_visible;
}

void PrimitiveUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Primitive", m_primitive);

   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      props.Separator("Render Options");
      props.Checkbox<Primitive>(
         m_primitive, "Visible", //
         [this](const Primitive* primitive) { return primitive == m_primitive ? m_visible : primitive->m_d.m_visible; }, //
         [this](Primitive* primitive, bool v) { primitive->m_d.m_visible = v; (primitive == m_primitive ? m_visible : primitive->m_d.m_visible) = v; });
      props.Checkbox<Primitive>(
         m_primitive, "Static Rendering", //
         [](const Primitive* primitive) { return primitive->m_d.m_staticRendering; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_staticRendering = v; });
      props.Checkbox<Primitive>(
         m_primitive, "Reflection Enabled", //
         [](const Primitive* primitive) { return primitive->m_d.m_reflectionEnabled; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_reflectionEnabled = v; });
      props.InputFloat<Primitive>(
         m_primitive, "Depth Bias", //
         [](const Primitive* primitive) { return primitive->m_d.m_depthBias; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);
      props.Checkbox<Primitive>(
         m_primitive, "Depth Mask", //
         [](const Primitive* primitive) { return primitive->m_d.m_useDepthMask; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_useDepthMask = v; });
      props.Checkbox<Primitive>(
         m_primitive, "Render Backfaces", //
         [](const Primitive* primitive) { return primitive->m_d.m_backfacesEnabled; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_backfacesEnabled = v; });
      props.Checkbox<Primitive>(
         m_primitive, "Additive Blend", //
         [](const Primitive* primitive) { return primitive->m_d.m_addBlend; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_addBlend = v; });
      //PropLightmapCombo("Lightmap", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szLightmap) : nullptr, live_obj ? &(live_obj->m_d.m_szLightmap) : nullptr, m_table);

      props.Separator("Material");
      props.MaterialCombo<Primitive>(
         m_primitive, "Material", //
         [](const Primitive* primitive) { return primitive->m_d.m_szMaterial; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szMaterial = v; });
      props.ImageCombo<Primitive>(
         m_primitive, "Image", //
         [](const Primitive* primitive) { return primitive->m_d.m_szImage; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szImage = v; });
      props.ImageCombo<Primitive>(
         m_primitive, "Normal Map", //
         [](const Primitive* primitive) { return primitive->m_d.m_szNormalMap; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szNormalMap = v; });
      props.Checkbox<Primitive>(
         m_primitive, "Object Space NM", //
         [](const Primitive* primitive) { return primitive->m_d.m_objectSpaceNormalMap; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_objectSpaceNormalMap = v; });
      /*
      PropFloat("Disable Spot Lights", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingTop) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingTop) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Translucency", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingBelow) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingBelow) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Modulate Opacity", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_alpha) : nullptr, live_obj ? &(live_obj->m_d.m_alpha) : nullptr, 0.01f, 0.05f, "%.3f");
      PropRGB("Modulate Color", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_color) : nullptr, live_obj ? &(live_obj->m_d.m_color) : nullptr);
      */


      props.Separator("Reflections");
      /*
      PropRenderProbeCombo("Reflection Probe", RenderProbe::PLANE_REFLECTION, startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szReflectionProbe) : nullptr, live_obj ? &(live_obj->m_d.m_szReflectionProbe) : nullptr, m_table);
      PropFloat("Reflection strength", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionStrength) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionStrength) : nullptr, 0.01f, 0.05f, "%.3f");
      */

      props.Separator("Refractions");
      /*
      PropRenderProbeCombo("Refraction Probe", RenderProbe::SCREEN_SPACE_TRANSPARENCY, startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szRefractionProbe) : nullptr, live_obj ? &(live_obj->m_d.m_szRefractionProbe) : nullptr, m_table);
      PropFloat("Refraction thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_refractionThickness) : nullptr, live_obj ? &(live_obj->m_d.m_refractionThickness) : nullptr, 0.01f, 0.05f, "%.3f");
      */

      props.EndSection();
   }

   if (props.BeginSection(PropertyPane::Section::Position))
   {
      props.Separator("Position, Rotation & Size");
      /*
      PropVec3("Position", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vPosition) : nullptr, live_obj ? &(live_obj->m_d.m_vPosition) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Orientation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[0]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[0]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Scale", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vSize) : nullptr, live_obj ? &(live_obj->m_d.m_vSize) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      */

      props.Separator("Additional Transform");
      /*
      PropVec3("Translation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[3]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[3]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[6]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[6]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      */

      props.EndSection();
   }

   props.TimerSection<Primitive>(m_primitive, [](Primitive* obj) { return &(obj->m_d.m_tdr); });
}

}