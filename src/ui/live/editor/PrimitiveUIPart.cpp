#include "core/stdafx.h"
#include "PrimitiveUIPart.h"

#include "core/extern.h"
#include "core/player.h"
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

   // Apply asynchronous mesh file dialog results
   if (m_pendingMeshImport && !m_pendingMeshImport->empty())
   {
      // A file was selected: reset the options to their defaults and show the import option dialog
      m_meshImportFileName = *m_pendingMeshImport;
      m_pendingMeshImport = nullptr;
      m_meshUnitsMeters = false;
      m_meshImportAbsolutePosition = false;
      m_meshImportCenterMesh = false;
      m_meshImportMaterial = false;
      m_meshImportAnimation = false;
      m_meshImportNoForsyth = false;
      m_meshImportFailed = false;
      ImGui::OpenPopup("Wavefront OBJ Importer");
   }
   if (m_pendingMeshExport && !m_pendingMeshExport->empty())
   {
      // A file was selected: reset the options to their defaults and show the export option dialog
      m_meshExportFileName = *m_pendingMeshExport;
      m_pendingMeshExport = nullptr;
      m_meshUnitsMeters = false;
      ImGui::OpenPopup("Wavefront OBJ Exporter");
   }

   // Import option dialog (same options as the WinUI 'Wavefront OBJ Importer' dialog)
   if (!m_meshImportFileName.empty())
   {
      if (ImGui::BeginPopupModal("Wavefront OBJ Importer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
      {
         ImGui::TextWrapped("%s", m_meshImportFileName.c_str());
         ImGui::Spacing();
         UpdateMeshUnitsUI();
         ImGui::SeparatorText("Options");
         ImGui::Columns(2, nullptr, true);
         ImGui::Checkbox("Center mesh to its midpoint", &m_meshImportCenterMesh);
         int position = m_meshImportAbsolutePosition ? 1 : 0;
         ImGui::RadioButton("Place at primitive's position", &position, 0);
         ImGui::RadioButton("Place at mesh's absolute position (use mesh's midpoint)", &position, 1);
         m_meshImportAbsolutePosition = position != 0;
         ImGui::NextColumn();
         ImGui::Checkbox("Import mesh's material", &m_meshImportMaterial);
         ImGui::Checkbox("Import Animation Sequence", &m_meshImportAnimation);
         ImGui::Checkbox("Do not reorder/optimize data", &m_meshImportNoForsyth);
         ImGui::Columns(1);
         if (m_meshImportFailed)
            ImGui::TextColored(ImVec4(1.f, 0.2f, 0.2f, 1.f), "Failed to import file!");
         ImGui::Separator();
         if (ImGui::Button("Import"))
         {
            m_part->m_d.m_meshFileName = std::filesystem::path(m_meshImportFileName).filename().string();
            m_meshImportFailed = !m_part->LoadMesh(m_meshImportFileName, m_meshUnitsMeters ? MeshUnits::Meters : MeshUnits::VPUnits, m_meshImportAbsolutePosition, m_meshImportCenterMesh,
               m_meshImportMaterial, m_meshImportAnimation, !m_meshImportNoForsyth);
            if (!m_meshImportFailed)
            {
               m_meshImportFileName.clear();
               ImGui::CloseCurrentPopup();
            }
         }
         ImGui::SameLine();
         if (ImGui::Button("Cancel"))
         {
            m_meshImportFileName.clear();
            ImGui::CloseCurrentPopup();
         }
         ImGui::EndPopup();
      }
      else
         m_meshImportFileName.clear();
   }

   // Export option dialog (same options as the WinUI 'Wavefront OBJ Exporter' dialog)
   if (!m_meshExportFileName.empty())
   {
      if (ImGui::BeginPopupModal("Wavefront OBJ Exporter", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
      {
         ImGui::TextWrapped("%s", m_meshExportFileName.c_str());
         ImGui::Spacing();
         UpdateMeshUnitsUI();
         ImGui::Separator();
         if (ImGui::Button("Export"))
         {
            m_part->m_mesh.SaveWavefrontObj(
               m_meshExportFileName, m_part->m_d.m_use3DMesh ? MakeString(m_part->m_wzName) : "Primitive"s, m_meshUnitsMeters ? MeshUnits::Meters : MeshUnits::VPUnits);
            m_meshExportFileName.clear();
            ImGui::CloseCurrentPopup();
         }
         ImGui::SameLine();
         if (ImGui::Button("Cancel"))
         {
            m_meshExportFileName.clear();
            ImGui::CloseCurrentPopup();
         }
         ImGui::EndPopup();
      }
      else
         m_meshExportFileName.clear();
   }

   if (props.BeginSection("Visuals"s))
   {
      props.Separator("Geometry"s);
      if (ImGui::Button("Import Mesh"))
         ImportMesh();
      ImGui::SameLine();
      ImGui::BeginDisabled(!props.GetEditedPart<Primitive>(m_part)->m_d.m_use3DMesh);
      if (ImGui::Button("Export Mesh"))
         ExportMesh();
      ImGui::EndDisabled();
      if (props.GetEditedPart<Primitive>(m_part)->m_d.m_use3DMesh)
      {
         ImGui::BeginDisabled();
         props.InputString<Primitive>(
            m_part, "Mesh File"s, //
            [](const Primitive* primitive) { return primitive->m_d.m_meshFileName; }, //
            [](Primitive*, const string&) {});
         ImGui::EndDisabled();
      }
      else
      {
         props.InputInt<Primitive>(
            m_part, "Sides"s, //
            [](const Primitive* primitive) { return primitive->m_d.m_Sides; }, //
            [](Primitive* primitive, int v) { primitive->m_d.m_Sides = v; });
         props.Checkbox<Primitive>(
            m_part, "Draw Textures Inside"s, //
            [](const Primitive* primitive) { return primitive->m_d.m_drawTexturesInside; }, //
            [](Primitive* primitive, bool v) { primitive->m_d.m_drawTexturesInside = v; });
      }

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
      props.Checkbox<Primitive>(
         m_part, "Show in Editor"s, //
         [](const Primitive* primitive) { return primitive->m_d.m_displayTexture; }, //
         [](Primitive* primitive, bool v) { primitive->m_d.m_displayTexture = v; });
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
         [](Primitive* primitive, const vec3& v) { primitive->m_d.m_vSize = v; }, PropertyPane::Unit::PercentX100, 2);

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

void PrimitiveUIPart::UpdateMeshUnitsUI()
{
   ImGui::SeparatorText("Units and axes");
   int units = m_meshUnitsMeters ? 1 : 0;
   ImGui::RadioButton("VPUnits (Up -Z, Visual Pinball default)", &units, 0);
   ImGui::Indent();
   ImGui::TextDisabled("Recommended Blender Import/Export Transformations:\nScale 0.00054, Forward -Y, Up -Z");
   ImGui::Unindent();
   ImGui::RadioButton("Meters (Up Y, Blender default)", &units, 1);
   ImGui::Indent();
   ImGui::TextDisabled("Matches Blender's default Import/Export Transformations:\nScale 1.0, Forward -Z, Up Y. Primitive scale stays 1.");
   ImGui::Unindent();
   m_meshUnitsMeters = units != 0;
}

void PrimitiveUIPart::ImportMesh()
{
   if (g_pplayer == nullptr || g_pplayer->m_playfieldWnd == nullptr)
      return;
   m_pendingMeshImport = std::make_shared<string>();
   const SDL_DialogFileFilter filters[] = { { "Wavefront obj file", "obj" } };
   SDL_ShowOpenFileDialog(
      [](void* userdata, const char* const* filelist, int filter)
      {
         auto* res = static_cast<std::shared_ptr<string>*>(userdata);
         if (filelist != nullptr && filelist[0] != nullptr)
            **res = filelist[0];
         delete res;
      },
      new std::shared_ptr<string>(m_pendingMeshImport), //
      g_pplayer->m_playfieldWnd->GetCore(), filters, 1, nullptr, false);
}

void PrimitiveUIPart::ExportMesh()
{
   if (g_pplayer == nullptr || g_pplayer->m_playfieldWnd == nullptr)
      return;
   m_pendingMeshExport = std::make_shared<string>();
   const SDL_DialogFileFilter filters[] = { { "Wavefront obj file", "obj" } };
   SDL_ShowSaveFileDialog(
      [](void* userdata, const char* const* filelist, int filter)
      {
         auto* res = static_cast<std::shared_ptr<string>*>(userdata);
         if (filelist != nullptr && filelist[0] != nullptr)
            **res = filelist[0];
         delete res;
      },
      new std::shared_ptr<string>(m_pendingMeshExport), //
      g_pplayer->m_playfieldWnd->GetCore(), filters, 1, nullptr);
}
}
