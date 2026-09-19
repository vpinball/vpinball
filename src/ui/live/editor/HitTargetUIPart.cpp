#include "core/stdafx.h"

#include "HitTargetUIPart.h"

namespace VPX::EditorUI
{

HitTargetUIPart::HitTargetUIPart(HitTarget* hitTarget)
   : EditableUIPart(hitTarget, { &Data::m_visible })
{
}

HitTargetUIPart::TransformMask HitTargetUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D Smatrix = Matrix3D::MatrixScale(m_part->m_d.m_vSize.x, m_part->m_d.m_vSize.y, m_part->m_d.m_vSize.z);
   const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(m_part->m_d.m_vPosition);
   const Matrix3D Rmatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_part->m_d.m_rotZ));
   transform = (Smatrix * Rmatrix) * Tmatrix;
   return static_cast<TransformMask>(static_cast<int>(TM_TransAny) | static_cast<int>(TM_ScaleAny) | static_cast<int>(TM_RotZ));
}

void HitTargetUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   m_part->m_d.m_vPosition = pos;
   m_part->m_d.m_vSize = scale;
   m_part->m_d.m_rotZ = rot.z;
}

void HitTargetUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void HitTargetUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("HitTarget"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<HitTarget>(
         m_part, "Type"s,
         vector<string> { "Drop Target Beveled"s, "Drop Target Simple"s, "Hit Target Round"s, "Hit Target Rectangle"s, "Hit Fat Target Rectangle"s, "Hit Fat Target Square"s,
            "Drop Target Flat Simple"s, "Hit Fat Target Slim"s, "Hit Target Slim"s }, //
         [](const HitTarget* hitTarget) { return static_cast<int>(hitTarget->m_d.m_targetType) - 1; }, //
         [](HitTarget* hitTarget, int v) { hitTarget->m_d.m_targetType = static_cast<TargetType>(v + 1); });
      props.ImageCombo<HitTarget>(
         m_part, "Image"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_szImage; }, //
         [](HitTarget* hitTarget, const string& v) { hitTarget->m_d.m_szImage = v; });
      props.MaterialCombo<HitTarget>(
         m_part, "Material"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_szMaterial; }, //
         [](HitTarget* hitTarget, const string& v) { hitTarget->m_d.m_szMaterial = v; });
      props.InputFloat<HitTarget>(
         m_part, "Drop Speed"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_dropSpeed; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_dropSpeed = v; }, PropertyPane::Unit::None, 1);
      props.InputInt<HitTarget>(
         m_part, "Raise Delay (ms)"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_raiseDelay; }, //
         [](HitTarget* hitTarget, int v) { hitTarget->m_d.m_raiseDelay = v; });
      props.InputFloat<HitTarget>(
         m_part, "Depth Bias"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_depthBias; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<HitTarget>(
         m_part, "Disable Spot Lights"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_disableLightingTop; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_disableLightingTop = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<HitTarget>(
         m_part, "Translucency"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_disableLightingBelow; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_disableLightingBelow = v; }, PropertyPane::Unit::None, 1);
      props.Checkbox<HitTarget>(
         m_part, "Visible"s, //
         [this](const HitTarget* hitTarget) { return GetVisibility(hitTarget); }, //
         [this](HitTarget* hitTarget, bool v) { SetVisibility(hitTarget, v); });
      props.Checkbox<HitTarget>(
         m_part, "Reflection Enabled"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_reflectionEnabled; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_reflectionEnabled = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat3<HitTarget>(
         m_part, "Position"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_vPosition; }, //
         [](HitTarget* hitTarget, const vec3& v) { hitTarget->m_d.m_vPosition = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<HitTarget>(
         m_part, "Size"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_vSize; }, //
         [](HitTarget* hitTarget, const vec3& v) { hitTarget->m_d.m_vSize = v; }, PropertyPane::Unit::Percent, 3);
      props.InputFloat<HitTarget>(
         m_part, "Orientation"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_rotZ; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_rotZ = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<HitTarget>(
         m_part, "Has Hit Event"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_hitEvent; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_hitEvent = v; });
      props.InputFloat<HitTarget>(
         m_part, "Hit Threshold"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_threshold; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_threshold = v; }, PropertyPane::Unit::None, 2);
      props.MaterialCombo<HitTarget>(
         m_part, "Physics Material"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_szPhysicsMaterial; }, //
         [](HitTarget* hitTarget, const string& v) { hitTarget->m_d.m_szPhysicsMaterial = v; });
      props.Checkbox<HitTarget>(
         m_part, "Overwrite Material"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_overwritePhysics; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_overwritePhysics = v; });
      props.InputFloat<HitTarget>(
         m_part, "Elasticity"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_elasticity; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<HitTarget>(
         m_part, "Elasticity Falloff"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_elasticityFalloff; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<HitTarget>(
         m_part, "Friction"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_friction; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<HitTarget>(
         m_part, "Scatter Angle"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_scatter; }, //
         [](HitTarget* hitTarget, float v) { hitTarget->m_d.m_scatter = v; }, PropertyPane::Unit::None, 3);
      props.Checkbox<HitTarget>(
         m_part, "Legacy"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_legacy; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_legacy = v; });
      props.Checkbox<HitTarget>(
         m_part, "Collidable"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_collidable; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_collidable = v; });
      props.Checkbox<HitTarget>(
         m_part, "Dropped"s, //
         [](const HitTarget* hitTarget) { return hitTarget->m_d.m_isDropped; }, //
         [](HitTarget* hitTarget, bool v) { hitTarget->m_d.m_isDropped = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
