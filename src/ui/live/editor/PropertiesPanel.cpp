// license:GPLv3+

#include "core/stdafx.h"
#include "PropertiesPanel.h"

#include "parts/Material.h"
#include "parts/primitive.h"
#include "renderer/Texture.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"
#include "utils/color.h"

#include "PropertyPane.h"

namespace VPX::EditorUI
{

void PropertiesPanel::Render(float topBarHeight)
{
   EditorUI &editor = m_editor;
   if (editor.m_table->IsLocked())
      return;

   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = 280.f * editor.m_liveUI.GetDPI();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - pane_width, viewport->Pos.y + topBarHeight));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - topBarHeight));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * editor.m_liveUI.GetDPI(), 4.0f * editor.m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("PROPERTIES", nullptr,
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
         | ImGuiWindowFlags_NoNavFocus);

   if (editor.m_multiSel.size() > 1)
      ImGui::TextDisabled("%d parts selected (editing the active one)", (int)editor.m_multiSel.size());

   PropertyPane props(editor.m_table);
   switch (editor.m_units)
   {
   case EditorUI::Units::VPX: props.SetLengthUnit(PropertyPane::Unit::VPLength); break;
   case EditorUI::Units::Metric: props.SetLengthUnit(PropertyPane::Unit::Millimeters); break;
   case EditorUI::Units::Imperial: props.SetLengthUnit(PropertyPane::Unit::Inches); break;
   }
   if (editor.IsInspectMode() && editor.m_selection.GetType() != Selection::S_IMAGE) // Images are shared between live and startup instance, so they do not have 2 states
   {
      if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
      {
         for (int tab = 0; tab < 2; tab++)
         {
            const bool is_live = (tab == 1);
            if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr, (is_live && m_selectLiveTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
            {
               if (is_live)
                  m_selectLiveTab = false;
               props.SetShowStartup(!is_live);
               switch (editor.m_selection.GetType())
               {
               case Selection::S_NONE: TableProperties(props); break;
               case Selection::S_EDITABLE:
                  editor.m_selection.GetPart()->UpdatePropertyPane(props);
                  if (props.GetModifiedField() > 0)
                  {
                     editor.m_renderer->ReinitRenderable(editor.m_selection.GetPart()->GetEditable()->GetIRenderable());
                     editor.m_player->m_physics->Update(editor.m_selection.GetPart()->GetEditable());
                  }
                  break;
               case Selection::S_IMAGE: ImageProperties(props, editor.m_selection.GetImage()); break;
               case Selection::S_CAMERA: CameraProperties(props, editor.m_selection.GetCamera()); break;
               case Selection::S_MATERIAL: MaterialProperties(props, editor.m_selection.GetMaterial()); break;
               case Selection::S_RENDERPROBE: RenderProbeProperties(props, editor.m_selection.GetProbe()); break;
               }
               ImGui::EndTabItem();
            }
         }
         ImGui::EndTabBar();
      }
   }
   else
   {
      switch (editor.m_selection.GetType())
      {
      case Selection::S_NONE: TableProperties(props); break;
      case Selection::S_EDITABLE:
         editor.m_undo.BeginUndo();
         editor.m_undo.MarkForUndo(editor.m_selection.GetPart()->GetEditable());
         editor.m_undo.EndUndo();
         editor.m_selection.GetPart()->UpdatePropertyPane(props);
         if (props.GetModifiedField() > 0 && (editor.m_lastUndoPart != editor.m_selection.GetPart()->GetEditable() || editor.m_lastUndoId != (0x2000 | props.GetModifiedField())))
         {
            editor.m_lastUndoPart = editor.m_selection.GetPart()->GetEditable();
            editor.m_lastUndoId = 0x2000 | props.GetModifiedField();
         }
         else
         {
            editor.m_undo.Discard();
         }
         if (props.GetModifiedField() > 0)
         {
            editor.m_renderer->ReinitRenderable(editor.m_selection.GetPart()->GetEditable()->GetIRenderable());
            editor.m_player->m_physics->Update(editor.m_selection.GetPart()->GetEditable());
         }
         break;
      case Selection::S_IMAGE: ImageProperties(props, editor.m_selection.GetImage()); break;
      case Selection::S_CAMERA: CameraProperties(props, editor.m_selection.GetCamera()); break;
      case Selection::S_MATERIAL: MaterialProperties(props, editor.m_selection.GetMaterial()); break;
      case Selection::S_RENDERPROBE: RenderProbeProperties(props, editor.m_selection.GetProbe()); break;
      }
   }

   ImGui::End();
   ImGui::PopStyleVar(3);
}

void PropertiesPanel::TableProperties(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *table = props.GetEditedPart<PinTable>(editor.m_table);
   props.Header("Table"s, [table]() { return table->GetWName(); }, [table](const wstring &v) { table->SetName(v); });

   if (props.BeginSection("Visuals"s))
   {
      props.Separator("Playfield"s);
      props.ImageCombo<PinTable>(
         table, "Image"s, //
         [](const PinTable *table) { return table->m_image; }, //
         [](PinTable *table, const string &v) { table->m_image = v; });
      props.MaterialCombo<PinTable>(
         table, "Material"s, //
         [](const PinTable *table) { return table->m_playfieldMaterial; }, //
         [](PinTable *table, const string &v) { table->m_playfieldMaterial = v; });
      props.InputInt<PinTable>(
         table, "Reflection Strength"s, //
         [](const PinTable *table) { return table->GetPlayfieldReflectionStrength(); }, //
         [](PinTable *table, int v) { table->SetPlayfieldReflectionStrength(v); });
      props.Checkbox<PinTable>(
         table, "Display Backdrop"s, //
         [](const PinTable *table) { return table->m_winEditorBackdrop; }, //
         [](PinTable *table, bool v) { table->m_winEditorBackdrop = v; });

      props.Separator("Ball"s);
      props.ImageCombo<PinTable>(
         table, "Ball Image"s, //
         [](const PinTable *table) { return table->m_ballImage; }, //
         [](PinTable *table, const string &v) { table->m_ballImage = v; });
      props.Checkbox<PinTable>(
         table, "Spherical Map"s, //
         [](const PinTable *table) { return table->m_ballSphericalMapping; }, //
         [](PinTable *table, bool v) { table->m_ballSphericalMapping = v; });
      props.ImageCombo<PinTable>(
         table, "Ball Decal"s, //
         [](const PinTable *table) { return table->m_ballImageDecal; }, //
         [](PinTable *table, const string &v) { table->m_ballImageDecal = v; });
      props.Checkbox<PinTable>(
         table, "Logo Mode"s, //
         [](const PinTable *table) { return table->m_BallDecalMode; }, //
         [](PinTable *table, bool v) { table->m_BallDecalMode = v; });
      props.InputFloat<PinTable>(
         table, "Reflection of Playfield"s, //
         [](const PinTable *table) { return table->m_ballPlayfieldReflectionStrength; }, //
         [](PinTable *table, float v) { table->m_ballPlayfieldReflectionStrength = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<PinTable>(
         table, "Default Bulb Intensity Scale"s, //
         [](const PinTable *table) { return table->m_defaultBulbIntensityScaleOnBall; }, //
         [](PinTable *table, float v) { table->m_defaultBulbIntensityScaleOnBall = v; }, PropertyPane::Unit::None, 2);

      props.Separator("Backdrop"s);
      props.InputRGB<PinTable>(
         table, "Background Color"s, //
         [](const PinTable *table) { return convertColor(table->m_colorbackdrop); }, //
         [](PinTable *table, const vec3 &v) { table->m_colorbackdrop = convertColorRGB(v); });
      props.Checkbox<PinTable>(
         table, "Apply Night->Day cycle"s, //
         [](const PinTable *table) { return table->m_ImageBackdropNightDay; }, //
         [](PinTable *table, bool v) { table->m_ImageBackdropNightDay = v; });
      props.ImageCombo<PinTable>(
         table, "DT Image"s, //
         [](const PinTable *table) { return table->m_BG_image[0]; }, //
         [](PinTable *table, const string &v) { table->m_BG_image[0] = v; });
      props.ImageCombo<PinTable>(
         table, "FS Image"s, //
         [](const PinTable *table) { return table->m_BG_image[1]; }, //
         [](PinTable *table, const string &v) { table->m_BG_image[1] = v; });
      props.ImageCombo<PinTable>(
         table, "FSS Image"s, //
         [](const PinTable *table) { return table->m_BG_image[2]; }, //
         [](PinTable *table, const string &v) { table->m_BG_image[2] = v; });
      props.ImageCombo<PinTable>(
         table, "Color Grading LUT"s, //
         [](const PinTable *table) { return table->m_imageColorGrade; }, //
         [](PinTable *table, const string &v) { table->m_imageColorGrade = v; });
      props.Checkbox<PinTable>(
         table, "Enable EMReels"s, //
         [](const PinTable *table) { return table->m_renderEMReels; }, //
         [](PinTable *table, bool v) { table->m_renderEMReels = v; });
      props.Checkbox<PinTable>(
         table, "Enable Decals"s, //
         [](const PinTable *table) { return table->m_renderDecals; }, //
         [](PinTable *table, bool v) { table->m_renderDecals = v; });
      props.Checkbox<PinTable>(
         table, "Enable FSS mode"s, //
         [](const PinTable *table) { return table->IsFSSEnabled(); }, //
         [](PinTable *table, bool v) { table->EnableFSS(v); });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Separator("Physics Constants"s);
      props.InputFloat<PinTable>(
         table, "Gravity Constant"s, //
         [](const PinTable *table) { return table->GetGravity(); }, //
         [](PinTable *table, float v) { table->SetGravity(v); }, PropertyPane::Unit::None, 2);
      props.InputFloat<PinTable>(
         table, "Playfield Elasticity"s, //
         [](const PinTable *table) { return table->m_elasticity; }, //
         [](PinTable *table, float v) { table->m_elasticity = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<PinTable>(
         table, "Playfield Elasticity Falloff"s, //
         [](const PinTable *table) { return table->m_elasticityFalloff; }, //
         [](PinTable *table, float v) { table->m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<PinTable>(
         table, "Playfield Friction"s, //
         [](const PinTable *table) { return table->m_friction; }, //
         [](PinTable *table, float v) { table->m_friction = v; }, PropertyPane::Unit::None, 3);
      props.InputFloat<PinTable>(
         table, "Playfield Scatter Angle"s, //
         [](const PinTable *table) { return table->m_scatter; }, //
         [](PinTable *table, float v) { table->m_scatter = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<PinTable>(
         table, "Default Elements Scatter Angle"s, //
         [](const PinTable *table) { return table->m_defaultScatter; }, //
         [](PinTable *table, float v) { table->m_defaultScatter = v; }, PropertyPane::Unit::Degree, 1);
      props.Combo<PinTable>(
         table, "Overwrite Physics by Global Set"s, vector<string> { "Disable"s, "Set1"s, "Set2"s, "Set3"s, "Set4"s, "Set5"s, "Set6"s, "Set7"s, "Set8"s }, //
         [](const PinTable *table) { return table->m_overridePhysics; }, //
         [](PinTable *table, int v) { table->m_overridePhysics = v; });
      props.Checkbox<PinTable>(
         table, "including Flipper Physics"s, //
         [](const PinTable *table) { return table->m_overridePhysicsFlipper; }, //
         [](PinTable *table, bool v) { table->m_overridePhysicsFlipper = v; });
      props.InputFloat<PinTable>(
         table, "Nudge Time"s, //
         [](const PinTable *table) { return table->m_nudgeTime; }, //
         [](PinTable *table, float v) { table->m_nudgeTime = v; }, PropertyPane::Unit::None, 2);
      props.InputInt<PinTable>(
         table, "Physics Max. Loops (-1=def.)"s, //
         [](const PinTable *table) { return static_cast<int>(table->m_PhysicsMaxLoops); }, //
         [](PinTable *table, int v) { table->m_PhysicsMaxLoops = static_cast<unsigned int>(v); });

      props.Separator("Dimensions"s);
      props.InputFloat<PinTable>(
         table, "Playfield Width"s, //
         [](const PinTable *table) { return table->GetTableWidth(); }, //
         [](PinTable *table, float v) { table->SetTableWidth(v); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Playfield Length"s, //
         [](const PinTable *table) { return table->GetHeight(); }, //
         [](PinTable *table, float v) { table->SetHeight(v); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Top Glass Height"s, //
         [](const PinTable *table) { return table->m_glassTopHeight; }, //
         [](PinTable *table, float v) { table->m_glassTopHeight = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Bottom Glass Height"s, //
         [](const PinTable *table) { return table->m_glassBottomHeight; }, //
         [](PinTable *table, float v) { table->m_glassBottomHeight = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Lockbar Height"s, //
         [](const PinTable *table) { return table->m_groundToLockbarHeight; }, //
         [](PinTable *table, float v) { table->m_groundToLockbarHeight = v; }, PropertyPane::Unit::VPLength, 1);

      props.Separator("Difficulty"s);
      props.InputFloat<PinTable>(
         table, "Slope for Min. Difficulty"s, //
         [](const PinTable *table) { return table->m_angletiltMin; }, //
         [](PinTable *table, float v) { table->m_angletiltMin = v; }, PropertyPane::Unit::Degree, 2);
      props.InputFloat<PinTable>(
         table, "Slope for Max. Difficulty"s, //
         [](const PinTable *table) { return table->m_angletiltMax; }, //
         [](PinTable *table, float v) { table->m_angletiltMax = v; }, PropertyPane::Unit::Degree, 2);
      props.InputFloat<PinTable>(
         table, "Game Difficulty"s, //
         [](const PinTable *table) { return table->m_difficulty; }, //
         [](PinTable *table, float v) { table->m_difficulty = v; }, PropertyPane::Unit::Percent, 0);
      props.EndSection();
   }

   if (props.BeginSection("Sound"s))
   {
      props.InputInt<PinTable>(
         table, "Sound Effect Volume"s, //
         [](const PinTable *table) { return table->GetTableSoundVolume(); }, //
         [](PinTable *table, int v) { table->SetTableSoundVolume(v); });
      props.InputInt<PinTable>(
         table, "Music Volume"s, //
         [](const PinTable *table) { return table->GetTableMusicVolume(); }, //
         [](PinTable *table, int v) { table->SetTableMusicVolume(v); });
      props.EndSection();
   }

   if (props.BeginSection("Lighting"s))
   {
      props.InputRGB<PinTable>(
         table, "Ambient Color", //
         [](const PinTable *table) { return convertColor(table->m_lightAmbient); }, //
         [&editor](PinTable *table, const vec3 &v)
         {
            table->m_lightAmbient = convertColorRGB(v);
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });

      props.InputRGB<PinTable>(
         table, "Light Em. Color", //
         [](const PinTable *table) { return convertColor(table->m_Light[0].emission); }, //
         [&editor](PinTable *table, const vec3 &v)
         {
            table->m_Light[0].emission = convertColorRGB(v);
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });
      props.InputFloat<PinTable>(
         table, "Light Em. Scale"s, //
         [](const PinTable *table) { return table->m_lightEmissionScale; }, //
         [&editor](PinTable *table, float v)
         {
            table->m_lightEmissionScale = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::None, 0);
      props.InputFloat<PinTable>(
         table, "Light Height"s, //
         [](const PinTable *table) { return table->m_lightHeight; }, //
         [&editor](PinTable *table, float v)
         {
            table->m_lightHeight = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Light Range"s, //
         [](const PinTable *table) { return table->m_lightRange; }, //
         [&editor](PinTable *table, float v)
         {
            table->m_lightRange = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Scene Lighting Scale"s, //
         [](const PinTable *table) { return table->m_globalEmissionScale; }, //
         [&editor](PinTable *table, float v)
         {
            table->m_globalEmissionScale = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::Percent, 1);

      props.Separator("Environment Lighting"s);
      props.ImageCombo<PinTable>(
         table, "Environment Image"s, //
         [](const PinTable *table) { return table->m_envImage; }, //
         [&editor](PinTable *table, const string &v)
         {
            table->m_envImage = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });
      props.InputFloat<PinTable>(
         table, "Environment Em. Scale"s, //
         [](const PinTable *table) { return table->m_envEmissionScale; }, //
         [](PinTable *table, float v) { table->m_envEmissionScale = v; }, PropertyPane::Unit::Percent, 3);

      props.Separator("Render Options"s);
      props.Checkbox<PinTable>(
         table, "Enable Ambient Occlusion"s, //
         [](const PinTable *table) { return table->m_enableAO; }, //
         [&editor](PinTable *table, bool v)
         {
            table->m_enableAO = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });
      props.InputFloat<PinTable>(
         table, "Ambient Occlusion Scale"s, //
         [](const PinTable *table) { return table->m_AOScale; }, //
         [](PinTable *table, float v) { table->m_AOScale = v; }, PropertyPane::Unit::Percent, 1);
      props.Checkbox<PinTable>(
         table, "Enable Sc. Sp. Reflections"s, //
         [](const PinTable *table) { return table->m_enableSSR; }, //
         [&editor](PinTable *table, bool v)
         {
            table->m_enableSSR = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });
      props.InputFloat<PinTable>(
         table, "Screen Space Reflection Scale"s, //
         [](const PinTable *table) { return table->m_SSRScale; }, //
         [](PinTable *table, float v) { table->m_SSRScale = v; }, PropertyPane::Unit::Percent, 1);
      props.InputFloat<PinTable>(
         table, "Bloom Strength"s, //
         [](const PinTable *table) { return table->m_bloom_strength; }, //
         [](PinTable *table, float v) { table->m_bloom_strength = v; }, PropertyPane::Unit::Percent, 1);
      props.Combo<PinTable>(
         table, "Tonemapping"s, vector<string> { "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s, "AgX Punchy"s }, //
         [](const PinTable *table) { return static_cast<int>(table->GetToneMapper()); }, //
         [&editor](PinTable *table, int v)
         {
            table->SetToneMapper(static_cast<ToneMapper>(v));
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });
      props.InputFloat<PinTable>(
         table, "Exposure"s, //
         [](const PinTable *table) { return table->GetExposure(); }, //
         [&editor](PinTable *table, float v)
         {
            table->SetExposure(v);
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::None, 2);

      props.EndSection();
   }
}

void PropertiesPanel::CameraProperties(PropertyPane &props, int bgSet)
{
   EditorUI &editor = m_editor;
   ImGui::BeginDisabled(true);
   props.Header("Camera"s, [bgSet]() { return bgSet == 0 ? L"Desktop"s : bgSet == 1 ? L"Cabinet"s : L"Full Single Screen"s; }, [](const wstring &) {});
   ImGui::EndDisabled();

   {
      if (ImGui::Button("Import"))
      {
         editor.m_table->ImportBackdropPOV(editor.m_table->GetSettingsFileName(), false);
         editor.m_renderer->MarkShaderDirty();
      }
      ImGui::SameLine();
      if (ImGui::Button("Export"))
         editor.m_table->ExportBackdropPOV(editor.m_table->GetSettingsFileName());
      ImGui::NewLine();
   }

   if (props.BeginSection("Visuals"s))
   {
      ViewSetup *const vs = &props.GetEditedPart<PinTable>(editor.m_table)->mViewSetups[bgSet];
      props.Combo<ViewSetup>(
         vs, "View Mode"s, vector { "Legacy"s, "Camera"s, "Window"s }, //
         [](const ViewSetup *viewSetup) { return static_cast<int>(viewSetup->mMode); }, //
         [](ViewSetup *viewSetup, int v) { viewSetup->mMode = static_cast<ViewLayoutMode>(v); });
      props.InputFloat<ViewSetup>(
         vs, "Field of View"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mFOV; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mFOV = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<ViewSetup>(
         vs, "Layback"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mLayback; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mLayback = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<ViewSetup>(
         vs, "Look At"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mLookAt; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mLookAt = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<ViewSetup>(
         vs, "X Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewX; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewX = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<ViewSetup>(
         vs, "Y Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewY; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewY = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<ViewSetup>(
         vs, "Z Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewZ; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewZ = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<ViewSetup>(
         vs, "Rotation"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewportRotation; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewportRotation = v; }, PropertyPane::Unit::Degree, 0);
      props.InputFloat<ViewSetup>(
         vs, "View X Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewHOfs; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewHOfs = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<ViewSetup>(
         vs, "View Y Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewVOfs; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewVOfs = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<ViewSetup>(
         vs, "X Scale"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mSceneScaleX; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleX = v; }, PropertyPane::Unit::Percent, 3);
      props.InputFloat<ViewSetup>(
         vs, "Y Scale"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mSceneScaleY; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleY = v; }, PropertyPane::Unit::Percent, 3);
      props.InputFloat<ViewSetup>(
         vs, "Z Scale"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mSceneScaleZ; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleZ = v; }, PropertyPane::Unit::Percent, 3);
      props.EndSection();
   }
}

void PropertiesPanel::ImageProperties(PropertyPane &props, Texture *texture)
{
   EditorUI &editor = m_editor;
   ImGui::BeginDisabled(editor.m_table->m_liveBaseTable != nullptr); // Disable edition in inspection mode as images are shared between startup & inspected table

   props.Header("Image"s, [texture]() { return MakeWString(texture->m_name); }, [texture](const wstring &v) { texture->m_name = MakeString(v); });

   ImTextureID image = editor.m_renderer->m_renderDevice->m_texMan.LoadTexture(texture, false);
   if (image)
   {
      if (props.BeginSection("Visuals"s))
      {
         std::shared_ptr<const BaseTexture> tex = texture->GetRawBitmap(false, 0);

         ImGui::BeginDisabled(tex == nullptr || !tex->HasAlpha());
         props.InputFloat<Texture>(
            texture, "Alpha Mask", //
            [](const Texture *image) { return image->m_alphaTestValue; }, //
            [](Texture *image, float v) { image->m_alphaTestValue = v; }, PropertyPane::Unit::None, 2);
         ImGui::EndDisabled();

         const string info
            = std::to_string(image->GetWidth()) + 'x' + std::to_string(image->GetHeight()) + ' ' + ((tex != nullptr && tex->m_format) ? BaseTexture::GetFormatString(tex->m_format) : ""s);
         props.Separator(info);

         props.EndSection();

         const float w = ImGui::GetWindowWidth();
         ImGui::Image(image, ImVec2(w, static_cast<float>(image->GetHeight()) * w / static_cast<float>(image->GetWidth())));
      }
   }
   else
   {
      ImGui::Text("Failed to load image");
   }

   ImGui::EndDisabled();
}

void PropertiesPanel::RenderProbeProperties(PropertyPane &props, RenderProbe *probe)
{
   EditorUI &editor = m_editor;
   RenderProbe *editedProbe = props.GetEditedPart<RenderProbe>(probe);
   props.Header("Render Probe"s, [editedProbe]() { return MakeWString(editedProbe->GetName()); }, [editedProbe](const wstring &v) { editedProbe->SetName(MakeString(v)); });

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<RenderProbe>(
         probe, "Type"s, vector { "Reflection"s, "Refraction"s }, //
         [](const RenderProbe *probe) { return static_cast<int>(probe->GetType()); }, //
         [](RenderProbe *probe, int v) { probe->SetType(static_cast<RenderProbe::ProbeType>(v)); });
      props.InputFloat3<RenderProbe>(
         probe, "Normal"s, //
         [](const RenderProbe *probe) { return probe->GetReflectionPlaneNormal(); }, //
         [](RenderProbe *probe, const vec3 &v) { probe->SetReflectionPlaneNormal(v); }, PropertyPane::Unit::None, 2);
      props.InputFloat<RenderProbe>(
         probe, "Distance"s, //
         [](const RenderProbe *probe) { return probe->GetReflectionPlaneDistance(); }, //
         [](RenderProbe *probe, float v) { probe->SetReflectionPlaneDistance(v); }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (ImGui::CollapsingHeader("Users", ImGuiTreeNodeFlags_DefaultOpen))
   {
      // Add a white line above
      const ImVec2 headerMin = ImGui::GetItemRectMin();
      const ImVec2 headerMax = ImGui::GetItemRectMax();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      const ImVec2 lineStart(headerMin.x, headerMin.y);
      const ImVec2 lineEnd(headerMax.x, headerMin.y);
      drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

      for (const IEditable *editable : editor.m_table->GetParts())
      {
         if (editable->GetItemType() != eItemPrimitive)
            continue;
         const Primitive *const primitive = static_cast<const Primitive *>(editable);
         if ((probe->GetType() == RenderProbe::PLANE_REFLECTION) && (primitive->m_d.m_szReflectionProbe != probe->GetName()))
            continue;
         if ((probe->GetType() == RenderProbe::SCREEN_SPACE_TRANSPARENCY) && (primitive->m_d.m_szRefractionProbe != probe->GetName()))
            continue;
         const auto it = editor.m_editableMap.find(editable);
         if (it == editor.m_editableMap.end())
            continue;
         if (ImGui::Selectable(primitive->GetName().c_str()))
            editor.SetSelection(Selection(it->second));
      }
   }
}

void PropertiesPanel::MaterialProperties(PropertyPane &props, Material *material)
{
   Material *editedMaterial = props.GetEditedPart<Material>(material);
   props.Header("Material"s, [editedMaterial]() { return MakeWString(editedMaterial->m_name); }, [editedMaterial](const wstring &v) { editedMaterial->m_name = MakeString(v); });

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Material>(
         material, "Type"s, vector { "Default"s, "Metal"s }, //
         [](const Material *material) { return material->m_type; }, //
         [](Material *material, int v) { material->m_type = static_cast<Material::MaterialType>(v); });
      props.InputRGB<Material>(
         material, "Color", //
         [](const Material *material) { return convertColor(material->m_cBase); }, //
         [](Material *material, const vec3 &v) { material->m_cBase = convertColorRGB(v); });
      props.InputFloat<Material>(
         material, "Wrap Lighting"s, //
         [](const Material *material) { return material->m_fWrapLighting; }, //
         [](Material *material, float v) { material->m_fWrapLighting = v; }, PropertyPane::Unit::None, 2);
      if (material->m_type != Material::METAL)
      {
         props.InputRGB<Material>(
            material, "Glossy Color", //
            [](const Material *material) { return convertColor(material->m_cGlossy); }, //
            [](Material *material, const vec3 &v) { material->m_cGlossy = convertColorRGB(v); });
         props.InputFloat<Material>(
            material, "Glossy Image Lerp"s, //
            [](const Material *material) { return material->m_fGlossyImageLerp; }, //
            [](Material *material, float v) { material->m_fGlossyImageLerp = v; }, PropertyPane::Unit::None, 2);
      }
      props.InputFloat<Material>(
         material, "Shininess"s, //
         [](const Material *material) { return material->m_fRoughness; }, //
         [](Material *material, float v) { material->m_fRoughness = v; }, PropertyPane::Unit::None, 2);
      props.InputRGB<Material>(
         material, "Clearcoat Color", //
         [](const Material *material) { return convertColor(material->m_cClearcoat); }, //
         [](Material *material, const vec3 &v) { material->m_cClearcoat = convertColorRGB(v); });
      props.InputFloat<Material>(
         material, "Edge Brightness"s, //
         [](const Material *material) { return material->m_fEdge; }, //
         [](Material *material, float v) { material->m_fEdge = v; }, PropertyPane::Unit::None, 2);
      props.EndSection();
   }

   if (props.BeginSection("Transparency"s))
   {
      props.Checkbox<Material>(
         material, "Enable Transparency"s, //
         [](const Material *material) { return material->m_bOpacityActive; }, //
         [](Material *material, bool v) { material->m_bOpacityActive = v; });
      props.InputFloat<Material>(
         material, "Opacity"s, //
         [](const Material *material) { return material->m_fOpacity; }, //
         [](Material *material, float v) { material->m_fOpacity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Material>(
         material, "Edge Opacity"s, //
         [](const Material *material) { return material->m_fEdgeAlpha; }, //
         [](Material *material, float v) { material->m_fEdgeAlpha = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Material>(
         material, "Thickness"s, //
         [](const Material *material) { return material->m_fThickness; }, //
         [](Material *material, float v) { material->m_fThickness = v; }, PropertyPane::Unit::None, 2);
      props.InputRGB<Material>(
         material, "Refraction Tint", //
         [](const Material *material) { return convertColor(material->m_cRefractionTint); }, //
         [](Material *material, const vec3 &v) { material->m_cRefractionTint = convertColorRGB(v); });
      props.EndSection();
   }
}

}
