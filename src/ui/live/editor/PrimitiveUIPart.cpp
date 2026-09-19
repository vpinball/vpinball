#include "core/stdafx.h"
#include "PrimitiveUIPart.h"

#include "utils/color.h"


namespace VPX::EditorUI
{

PrimitiveUIPart::PrimitiveUIPart(Primitive* primitive)
   : EditableUIPart(primitive, { &Data::m_visible })
{
}

PrimitiveUIPart::TransformMask PrimitiveUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D Smatrix = Matrix3D::MatrixScale(m_part->m_d.m_vSize.x, m_part->m_d.m_vSize.y, m_part->m_d.m_vSize.z);
   const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(m_part->m_d.m_vPosition);
   const Matrix3D Rmatrix = (Matrix3D::MatrixRotateZ(ANGTORAD(m_part->m_d.m_aRotAndTra[2])) //
                               * Matrix3D::MatrixRotateY(ANGTORAD(m_part->m_d.m_aRotAndTra[1]))) //
      * Matrix3D::MatrixRotateX(ANGTORAD(m_part->m_d.m_aRotAndTra[0]));
   transform = (Smatrix * Rmatrix) * Tmatrix; // fullMatrix = Scale * Rotate * Translate
   return TM_TRSAny;
}

void PrimitiveUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_vPosition = pos;
   m_part->m_d.m_aRotAndTra[0] = rot.x;
   m_part->m_d.m_aRotAndTra[1] = rot.y;
   m_part->m_d.m_aRotAndTra[2] = rot.z;
   m_part->m_d.m_vSize = scale;
}

void PrimitiveUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void PrimitiveUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Primitive"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Separator("Render Options"s);
      props.Checkbox<Primitive>(
         m_part, "Visible"s, //
         [this](const Primitive* primitive) { return GetVisibility(primitive); }, //
         [this](Primitive* primitive, bool v) { SetVisibility(primitive, v); });
      props.Checkbox<Primitive>(
         m_part, "Static Rendering"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_staticRendering; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_staticRendering = v; });
      props.Checkbox<Primitive>(
         m_part, "Reflection Enabled"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_reflectionEnabled; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_reflectionEnabled = v; });
      props.InputFloat<Primitive>(
         m_part, "Depth Bias"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_depthBias; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);
      props.Checkbox<Primitive>(
         m_part, "Depth Mask"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_useDepthMask; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_useDepthMask = v; });
      props.Checkbox<Primitive>(
         m_part, "Render Backfaces"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_backfacesEnabled; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_backfacesEnabled = v; });
      props.Checkbox<Primitive>(
         m_part, "Additive Blend"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_addBlend; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_addBlend = v; });
      props.LightmapCombo<Primitive>(
         m_part, "Lightmap"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szLightmap; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szLightmap = v; });

      props.Separator("Material"s);
      props.MaterialCombo<Primitive>(
         m_part, "Material"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szMaterial; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szMaterial = v; });
      props.ImageCombo<Primitive>(
         m_part, "Image"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szImage; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szImage = v; });
      props.ImageCombo<Primitive>(
         m_part, "Normal Map"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szNormalMap; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szNormalMap = v; });
      props.Checkbox<Primitive>(
         m_part, "Object Space NM"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_objectSpaceNormalMap; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_objectSpaceNormalMap = v; });
      props.InputFloat<Primitive>(
         m_part, "Disable Spot Lights"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_disableLightingTop; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_disableLightingTop = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Primitive>(
         m_part, "Translucency"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_disableLightingBelow; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_disableLightingBelow = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Primitive>(
         m_part, "Modulate Opacity"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_alpha; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_alpha = v; }, PropertyPane::Unit::None, 3);
      props.InputRGB<Primitive>(
         m_part, "Modulate Color"s, //
         [](const Primitive* primitive) { return convertColor(primitive->m_d.m_color); }, //
         [](Primitive* primitive, const vec3& v) { primitive->m_d.m_color = convertColorRGB(v); });

      props.Separator("Reflections");
      props.RenderProbeCombo<Primitive>(
         m_part, "Reflection Probe"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szReflectionProbe; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szReflectionProbe = v; });
      props.InputFloat<Primitive>(
         m_part, "Reflection Strength"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_reflectionStrength; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_reflectionStrength = v; }, PropertyPane::Unit::None, 3);

      props.Separator("Refractions"s);
      props.RenderProbeCombo<Primitive>(
         m_part, "Refraction Probe"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szRefractionProbe; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szRefractionProbe = v; });
      props.InputFloat<Primitive>(
         m_part, "Refraction Thickness"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_refractionThickness; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_refractionThickness = v; }, PropertyPane::Unit::None, 3);

      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.Separator("Position, Rotation & Size"s);
      props.InputFloat3<Primitive>(
         m_part, "Position"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_vPosition; }, //
         [](Primitive* primitive, const vec3& v) { primitive->m_d.m_vPosition = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Primitive>(
         m_part, "Orientation"s, //
         [](const Primitive* primitive) { return vec3(primitive->m_d.m_aRotAndTra[0], primitive->m_d.m_aRotAndTra[1], primitive->m_d.m_aRotAndTra[2]); }, //
         [](Primitive* primitive, const vec3& v) { 
            primitive->m_d.m_aRotAndTra[0] = v.x; 
            primitive->m_d.m_aRotAndTra[1] = v.y;
            primitive->m_d.m_aRotAndTra[2] = v.z;
         },
         PropertyPane::Unit::Degree, 2);
      props.InputFloat3<Primitive>(
         m_part, "Scale"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_vSize; }, //
         [](Primitive* primitive, const vec3& v) { primitive->m_d.m_vSize = v; }, PropertyPane::Unit::Percent, 2);

      props.Separator("Additional Transform"s);
      props.InputFloat3<Primitive>(
         m_part, "Translation"s, //
         [](const Primitive* primitive) { return vec3(primitive->m_d.m_aRotAndTra[3], primitive->m_d.m_aRotAndTra[4], primitive->m_d.m_aRotAndTra[5]); }, //
         [](Primitive* primitive, const vec3& v)
         {
            primitive->m_d.m_aRotAndTra[3] = v.x;
            primitive->m_d.m_aRotAndTra[4] = v.y;
            primitive->m_d.m_aRotAndTra[5] = v.z;
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Primitive>(
         m_part, "Rotation"s, //
         [](const Primitive* primitive) { return vec3(primitive->m_d.m_aRotAndTra[6], primitive->m_d.m_aRotAndTra[7], primitive->m_d.m_aRotAndTra[8]); }, //
         [](Primitive* primitive, const vec3& v)
         {
            primitive->m_d.m_aRotAndTra[6] = v.x;
            primitive->m_d.m_aRotAndTra[7] = v.y;
            primitive->m_d.m_aRotAndTra[8] = v.z;
         },
         PropertyPane::Unit::Degree, 2);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Primitive>(
         m_part, "Has Hit Event"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_hitEvent; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_hitEvent = v; });
      props.InputFloat<Primitive>(
         m_part, "Hit Threshold"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_threshold; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_threshold = v; }, PropertyPane::Unit::None, 3);
      props.MaterialCombo<Primitive>(
         m_part, "Physics Material"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_szPhysicsMaterial; }, //
         [](Primitive* primitive, const string& v) { primitive->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<Primitive>(
         m_part, "Overwrite Material"s, //
         [](const Primitive* surf) { return surf->m_d.m_overwritePhysics; }, //
         [](Primitive* surf, bool v) { surf->m_d.m_overwritePhysics = v; });
      props.InputFloat<Primitive>(
         m_part, "Elasticity"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_elasticity; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Primitive>(
         m_part, "Elasticity Falloff"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_elasticityFalloff; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Primitive>(
         m_part, "Friction"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_friction; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<Primitive>(
         m_part, "Scatter Angle"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_scatter; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<Primitive>(
         m_part, "Collidable"s, //
         [](const Primitive* surf) { return surf->m_d.m_collidable; }, //
         [](Primitive* surf, bool v) { surf->m_d.m_collidable = v; });
      props.Checkbox<Primitive>(
         m_part, "Toy (never collidable)"s, //
         [](const Primitive* surf) { return surf->m_d.m_toy; }, //
         [](Primitive* surf, bool v) { surf->m_d.m_toy = v; });
      props.InputFloat<Primitive>(
         m_part, "Reduction Factor"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_collision_reductionFactor; }, //
         [](Primitive* primitive, float v) { primitive->m_d.m_collision_reductionFactor = v; }, PropertyPane::Unit::None, 3);
      props.EndSection();
   }

   //props.TimerSection(m_part);
}

}
