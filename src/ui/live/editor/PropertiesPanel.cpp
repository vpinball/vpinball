// license:GPLv3+

#include "core/stdafx.h"
#include "PropertiesPanel.h"

#include "audio/AudioPlayer.h"
#include "core/editablereg.h"
#include "core/player.h"
#include "core/SettingsService.h"
#include "parts/Material.h"
#include "parts/Sound.h"
#include "parts/primitive.h"
#include "renderer/Texture.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"
#include "utils/color.h"

#include "PropertyPane.h"

namespace VPX::EditorUI
{

PropertiesPanel::PropertiesPanel(EditorUI &editor)
   : m_editor(editor)
{
}

PropertiesPanel::~PropertiesPanel() = default;

void PropertiesPanel::Render(float topBarHeight)
{
   EditorUI &editor = m_editor;
   if (editor.m_table->IsLocked())
      return;

   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = PaneWidth * editor.m_liveUI.GetDPI();
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
   if (editor.m_multiSelImages.size() > 1)
      ImGui::TextDisabled("%d images selected (editing the active one)", (int)editor.m_multiSelImages.size());
   if (editor.m_multiSelSounds.size() > 1)
      ImGui::TextDisabled("%d sounds selected (editing the active one)", (int)editor.m_multiSelSounds.size());
   if (editor.m_multiSelMaterials.size() > 1)
      ImGui::TextDisabled("%d materials selected (editing the active one)", (int)editor.m_multiSelMaterials.size());

   PropertyPane props(editor.m_table);
   switch (editor.m_units)
   {
   case EditorUI::Units::VPX: props.SetLengthUnit(PropertyPane::Unit::VPLength); break;
   case EditorUI::Units::Metric: props.SetLengthUnit(PropertyPane::Unit::Millimeters); break;
   case EditorUI::Units::Imperial: props.SetLengthUnit(PropertyPane::Unit::Inches); break;
   }
   if (editor.IsInspectMode() && editor.m_selection.GetType() != Selection::S_IMAGE
      && editor.m_selection.GetType() != Selection::S_SOUND) // Images & sounds are shared between live and startup instance, so they do not have 2 states
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
               case Selection::S_SOUND: SoundProperties(props, editor.m_selection.GetSound()); break;
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
      case Selection::S_SOUND: SoundProperties(props, editor.m_selection.GetSound()); break;
      }
   }

   RenderPopups();

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

   ImageActions(props);

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
         const float scale = w / static_cast<float>(std::max(image->GetWidth(), image->GetHeight()));
         const float imgW = static_cast<float>(image->GetWidth()) * scale;
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (w - imgW) * 0.5f);
         ImGui::Image(image, ImVec2(imgW, static_cast<float>(image->GetHeight()) * scale));
      }
   }
   else
   {
      ImGui::Text("Failed to load image");
   }

   ImGui::EndDisabled();

   vector<WhereUsedInfo> whereUsed;
   editor.m_table->ShowWhereImageUsed(whereUsed, texture);
   UsersSection(props, whereUsed);
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

   ImGui::BeginDisabled(m_editor.m_table->m_liveBaseTable != nullptr); // Material list actions are not supported in inspection mode
   MaterialActions(props);
   ImGui::EndDisabled();

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

   vector<WhereUsedInfo> whereUsed;
   m_editor.m_table->ShowWhereMaterialUsed(whereUsed, material);
   UsersSection(props, whereUsed);
}

void PropertiesPanel::SoundProperties(PropertyPane &props, VPX::Sound *sound)
{
   EditorUI &editor = m_editor;
   VPX::AudioPlayer *const audioPlayer = editor.m_player->m_audioPlayer.get();

   // Track the playback state of the selected sound: playback commands are dispatched asynchronously
   // so the button keeps the "stop" state until playback is actually observed, and reverts to "play"
   // when the sound reaches its end.
   if (m_playingSound != sound)
   {
      m_playingSound = nullptr;
      m_playingSoundObserved = false;
   }
   const bool playing = audioPlayer && audioPlayer->IsSoundPlaying(sound);
   m_playingSoundObserved |= playing;
   if (m_playingSoundObserved && !playing)
   {
      m_playingSound = nullptr;
      m_playingSoundObserved = false;
   }
   const bool isPlaying = playing || (m_playingSound == sound);

   props.Header(
      "Sound"s, [sound]() { return MakeWString(sound->GetName()); },
      [&editor, sound](const wstring &v)
      {
         sound->SetName(MakeString(v));
         editor.m_table->SetNonUndoableDirty(eSaveDirty);
      });

   ImGui::BeginDisabled(editor.m_table->m_liveBaseTable != nullptr); // Sound list actions are not supported in inspection mode as sounds are shared
   SoundActions(props);
   ImGui::EndDisabled();

   ImGui::BeginDisabled(audioPlayer == nullptr);
   if (ImGui::Button(isPlaying ? (ICON_FK_STOP " Stop"s).c_str() : (ICON_FK_PLAY " Play"s).c_str()))
   {
      if (isPlaying)
      {
         audioPlayer->StopSound(sound);
         m_playingSound = nullptr;
         m_playingSoundObserved = false;
      }
      else
      {
         float volume = (float)sound->GetVolume();
         sound->SetVolume(100);
         audioPlayer->PlaySound(sound, volume, 0.f, 0, 0.f, 0.f, 0, false, true);
         m_playingSound = sound;
         m_playingSoundObserved = false;
         sound->SetVolume(volume);
      }
   }
   ImGui::EndDisabled();

   ImGui::BeginDisabled(editor.m_table->m_liveBaseTable != nullptr); // Disable edition in inspection mode as sounds are shared between startup & inspected table

   if (props.BeginSection("Audio Positioning"s))
   {
      props.Combo<VPX::Sound>(
         sound, "Output Target"s, vector { "Table"s, "Backglass"s }, //
         [](const VPX::Sound *s) { return static_cast<int>(s->GetOutputTarget()); }, //
         [&editor](VPX::Sound *s, int v)
         {
            s->SetOutputTarget(static_cast<VPX::SoundOutTypes>(v));
            editor.m_table->SetNonUndoableDirty(eSaveDirty);
         });
      props.InputFloat<VPX::Sound>(
         sound, "Volume"s, //
         [](const VPX::Sound *s) { return dequantizeSignedPercent(s->GetVolume()); }, //
         [&editor](VPX::Sound *s, float v)
         {
            s->SetVolume(quantizeSignedPercent(v));
            editor.m_table->SetNonUndoableDirty(eSaveDirty);
         },
         PropertyPane::Unit::None, 2);
      props.InputFloat<VPX::Sound>(
         sound, "Pan"s, //
         [](const VPX::Sound *s) { return dequantizeSignedPercent(s->GetPan()); }, //
         [&editor](VPX::Sound *s, float v)
         {
            s->SetPan(quantizeSignedPercent(v));
            editor.m_table->SetNonUndoableDirty(eSaveDirty);
         },
         PropertyPane::Unit::None, 2);
      props.InputFloat<VPX::Sound>(
         sound, "Front/Rear Fade"s, //
         [](const VPX::Sound *s) { return dequantizeSignedPercent(s->GetFrontRearFade()); }, //
         [&editor](VPX::Sound *s, float v)
         {
            s->SetFrontRearFade(quantizeSignedPercent(v));
            editor.m_table->SetNonUndoableDirty(eSaveDirty);
         },
         PropertyPane::Unit::None, 2);
      props.EndSection();
   }

   if (props.BeginSection("File"s))
   {
      if (m_soundInfoFor != sound)
      {
         m_soundInfoFor = sound;
         m_soundInfo = audioPlayer ? audioPlayer->GetSoundInformations(sound) : std::optional<VPX::SoundSpec>();
      }
      ImGui::TextWrapped("%s", sound->GetImportPath().string().c_str());
      ImGui::TextDisabled("Size: %s", SizeToReadable(sound->GetFileSize()).c_str()); //
      if (m_soundInfo)
         ImGui::TextDisabled("%.2f s, %u Hz, %u channel%s", m_soundInfo->lengthInSeconds, m_soundInfo->sampleFrequency, m_soundInfo->nChannels, m_soundInfo->nChannels > 1 ? "s" : "");
      props.EndSection();
   }

   ImGui::EndDisabled();
}

void PropertiesPanel::RequestConfirm(const string &message, const std::function<void()> &action)
{
   m_confirmMessage = message;
   m_confirmAction = action;
   ImGui::OpenPopup("Confirm Action");
}

void PropertiesPanel::UsersSection(PropertyPane &props, const vector<WhereUsedInfo> &whereUsed)
{
   EditorUI &editor = m_editor;
   if (props.BeginSection("Users"s))
   {
      if (whereUsed.empty())
         ImGui::TextDisabled("Not used by any part");
      for (const WhereUsedInfo &where : whereUsed)
      {
         IEditable * editable = editor.m_table->GetElementByName(where.whereUsedObjectname.c_str());
         if (editable == nullptr && where.whereUsedObjectname == MakeString(m_editor.m_table->m_wzName))
            editable = m_editor.m_table;
         ImGui::PushID(&where);
         // Clicking an entry selects the using part in the editor, like the Win32 dialog's 'Edit Object' button
         if (ImGui::Selectable(where.whereUsedObjectname.c_str()))
            if (editable != nullptr)
               if (const auto it = editor.m_editableMap.find(editable); it != editor.m_editableMap.end())
                  editor.SetSelection(Selection(it->second));
         ImGui::PopID();
         ImGui::SameLine();
         string type = "";
         if (editable != nullptr && editable->GetItemType() == eItemTable)
            type = "Table";
         else if (editable != nullptr)
            type = LocalString(EditableRegistry::GetTypeNameStringID(editable->GetItemType())).m_szbuffer;
         ImGui::TextDisabled("[%s - %s]", type.c_str(), where.whereUsedPropertyName.c_str());
      }
      props.EndSection();
   }
}

void PropertiesPanel::RenderPopups()
{
   // Confirmation popup for destructive resource actions (OpenPopup is called from this window's scope)
   if (ImGui::BeginPopupModal("Confirm Action", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
   {
      ImGui::TextWrapped("%s", m_confirmMessage.c_str());
      ImGui::Separator();
      if (ImGui::Button("OK"))
      {
         if (m_confirmAction)
            m_confirmAction();
         m_confirmAction = nullptr;
         ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel"))
      {
         m_confirmAction = nullptr;
         ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
   }
}

void PropertiesPanel::ImageActions(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<Texture *> &sel = editor.m_multiSelImages;

   // Update the selection when a reimport replaced an image (the old object is deleted)
   const auto replaceImage = [&editor](Texture *oldImage, Texture *newImage)
   {
      std::replace(editor.m_multiSelImages.begin(), editor.m_multiSelImages.end(), oldImage, newImage);
      if (editor.m_selection == Selection(oldImage))
         editor.m_selection = (newImage != nullptr) ? Selection(newImage) : Selection();
      if (editor.m_outlinerImageAnchor == oldImage)
         editor.m_outlinerImageAnchor = newImage;
   };

   // Apply asynchronous file dialog results
   if (m_pendingImageImport && !m_pendingImageImport->empty())
   {
      vector<Texture *> imported;
      for (const string &file : *m_pendingImageImport)
         if (Texture *const tex = table->ImportImage(file, ""s))
            imported.push_back(tex);
      g_settingsService.GetAppSettings().SetRecentDir_ImageDir(std::filesystem::path(m_pendingImageImport->front()).parent_path().string(), false);
      m_pendingImageImport = nullptr;
      if (!imported.empty())
      {
         sel = imported;
         editor.m_selection = Selection(imported.back());
         editor.m_outlinerImageAnchor = imported.back();
         table->SetNonUndoableDirty(eSaveDirty);
      }
      else
         m_actionStatus = "Failed to import the selected file(s)"s;
   }
   if (m_pendingImageReimport && !m_pendingImageReimport->empty())
   {
      const string file = *m_pendingImageReimport;
      m_pendingImageReimport = nullptr;
      Texture *const old = m_pendingImageReimportTarget;
      m_pendingImageReimportTarget = nullptr;
      if (old != nullptr)
      {
         // ImportImage replaces and deletes the image that already uses this name
         if (Texture *const tex = table->ImportImage(file, old->m_name))
         {
            replaceImage(old, tex);
            table->SetNonUndoableDirty(eSaveDirty);
         }
         else
            m_actionStatus = "Failed to reimport '"s + file + '\'';
         g_settingsService.GetAppSettings().SetRecentDir_ImageDir(std::filesystem::path(file).parent_path().string(), false);
      }
   }
   if (m_pendingImageExport && !m_pendingImageExport->empty())
   {
      const std::filesystem::path path = *m_pendingImageExport;
      m_pendingImageExport = nullptr;
      // Like the Win32 dialog: with multiple selected images, only the folder of the picked file is used
      int failed = 0;
      for (Texture *tex : m_pendingImageExportSel)
      {
         const std::filesystem::path file = (m_pendingImageExportSel.size() > 1)
            ? path.parent_path() / (m_exportUseNames ? tex->m_name + tex->GetFilePath().extension().string() : tex->GetFilePath().filename().string())
            : path;
         if (!tex->SaveFile(file.string()))
            failed++;
      }
      m_pendingImageExportSel.clear();
      if (failed > 0)
         m_actionStatus = "Failed to export "s + std::to_string(failed) + " image(s)"s;
      g_settingsService.GetAppSettings().SetRecentDir_ImageDir(path.parent_path().string(), false);
   }

   if (props.BeginSection("Actions"s))
   {
      const bool canImport = editor.m_player->m_playfieldWnd != nullptr;
      ImGui::BeginDisabled(!canImport);
      if (ImGui::Button("Import"))
      {
         m_pendingImageImport = std::make_shared<vector<string>>();
         const SDL_DialogFileFilter filters[] = { { "Image Files", "bmp;jpg;jpeg;png;tga;webp;exr;hdr" } };
         const string dir = g_settingsService.GetAppSettings().GetRecentDir_ImageDir();
         SDL_ShowOpenFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<vector<string>> *>(userdata);
               if (filelist != nullptr)
                  for (int i = 0; filelist[i] != nullptr; i++)
                     (**res).push_back(filelist[i]);
               delete res;
            },
            new std::shared_ptr<vector<string>>(m_pendingImageImport), //
            editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), true);
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty() || !canImport);
      if (ImGui::Button("Export"))
      {
         m_pendingImageExportSel = sel;
         m_pendingImageExport = std::make_shared<string>();
         const SDL_DialogFileFilter filters[] = { { "Image Files", "bmp;jpg;jpeg;png;tga;webp;exr;hdr" } };
         const string dir = g_settingsService.GetAppSettings().GetRecentDir_ImageDir();
         SDL_ShowSaveFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<string> *>(userdata);
               if (filelist != nullptr && filelist[0] != nullptr)
                  **res = filelist[0];
               delete res;
            },
            new std::shared_ptr<string>(m_pendingImageExport), //
            editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str());
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty());
      if (ImGui::Button("Delete"))
      {
         RequestConfirm("Delete the selected image(s)?"s,
            [this]()
            {
               EditorUI &editor = m_editor;
               for (Texture *const tex : editor.m_multiSelImages)
                  editor.m_table->RemoveImage(tex);
               editor.m_multiSelImages.clear();
               editor.m_outlinerImageAnchor = nullptr;
               editor.m_selection = Selection();
               editor.m_table->SetNonUndoableDirty(eSaveDirty);
            });
      }
      ImGui::EndDisabled();

      // --

      ImGui::BeginDisabled(sel.empty());

      if (ImGui::Button("Reimport"))
      {
         RequestConfirm("Reimport the selected image(s) using their existing file path(s)?"s,
            [this, replaceImage]()
            {
               EditorUI &editor = m_editor;
               const vector<Texture *> sel = editor.m_multiSelImages;
               int missing = 0;
               for (Texture *const old : sel)
                  if (std::filesystem::exists(old->GetFilePath()))
                     replaceImage(old, m_editor.m_table->ImportImage(old->GetFilePath(), old->m_name));
                  else
                     missing++;
               if (missing > 0)
                  m_actionStatus = std::to_string(missing) + " file(s) not found"s;
               m_editor.m_table->SetNonUndoableDirty(eSaveDirty);
            });
      }

      ImGui::SameLine();
      ImGui::BeginDisabled(sel.size() != 1);
      if (ImGui::Button("Reimport From"))
      {
         RequestConfirm("Reimport the selected image using a different file path?"s,
            [this]()
            {
               if (m_editor.m_player->m_playfieldWnd == nullptr)
                  return;
               m_pendingImageReimportTarget = m_editor.m_multiSelImages.empty() ? nullptr : m_editor.m_multiSelImages.back();
               m_pendingImageReimport = std::make_shared<string>();
               const SDL_DialogFileFilter filters[] = { { "Image Files", "bmp;jpg;jpeg;png;tga;webp;exr;hdr" } };
               const string dir = g_settingsService.GetAppSettings().GetRecentDir_ImageDir();
               SDL_ShowOpenFileDialog(
                  [](void *userdata, const char *const *filelist, int filter)
                  {
                     auto *res = static_cast<std::shared_ptr<string> *>(userdata);
                     if (filelist != nullptr && filelist[0] != nullptr)
                        **res = filelist[0];
                     delete res;
                  },
                  new std::shared_ptr<string>(m_pendingImageReimport), //
                  m_editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), false);
            });
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(table->m_vimage.empty());
      if (ImGui::Button("Reimport All"))
      {
         const vector<Texture *> all = table->m_vimage;
         int missing = 0;
         for (Texture *const old : all)
            if (std::filesystem::exists(old->GetFilePath()))
               replaceImage(old, table->ImportImage(old->GetFilePath(), old->m_name));
            else
               missing++;
         if (missing > 0)
            m_actionStatus = std::to_string(missing) + " file(s) not found"s;
         table->SetNonUndoableDirty(eSaveDirty);
      }
      ImGui::EndDisabled();

      ImGui::EndDisabled();

      // --
      ImGui::Checkbox("Use names on export", &m_exportUseNames);
      if (!m_actionStatus.empty())
         ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_actionStatus.c_str());
      props.EndSection();
   }
}

void PropertiesPanel::SoundActions(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<VPX::Sound *> &sel = editor.m_multiSelSounds;

   // Apply asynchronous file dialog results
   if (m_pendingSoundImport && !m_pendingSoundImport->empty())
   {
      vector<VPX::Sound *> imported;
      for (const string &file : *m_pendingSoundImport)
         if (VPX::Sound *const sound = table->ImportSound(file))
            imported.push_back(sound);
      g_settingsService.GetAppSettings().SetRecentDir_SoundDir(std::filesystem::path(m_pendingSoundImport->front()).parent_path().string(), false);
      m_pendingSoundImport = nullptr;
      if (!imported.empty())
      {
         sel = imported;
         editor.m_selection = Selection(imported.back());
         editor.m_outlinerSoundAnchor = imported.back();
         table->SetNonUndoableDirty(eSaveDirty);
      }
      else
         m_actionStatus = "Failed to import the selected file(s)"s;
   }
   if (m_pendingSoundReimport && !m_pendingSoundReimport->empty())
   {
      const string file = *m_pendingSoundReimport;
      m_pendingSoundReimport = nullptr;
      if (m_pendingSoundReimportTarget != nullptr)
      {
         table->ReImportSound(m_pendingSoundReimportTarget, file);
         table->SetNonUndoableDirty(eSaveDirty);
      }
      m_pendingSoundReimportTarget = nullptr;
      g_settingsService.GetAppSettings().SetRecentDir_SoundDir(std::filesystem::path(file).parent_path().string(), false);
   }
   if (m_pendingSoundExport && !m_pendingSoundExport->empty())
   {
      const std::filesystem::path path = *m_pendingSoundExport;
      m_pendingSoundExport = nullptr;
      // Like the Win32 dialog: with multiple selected sounds, only the folder of the picked file is used
      int failed = 0;
      for (VPX::Sound *sound : m_pendingSoundExportSel)
      {
         const std::filesystem::path file = (m_pendingSoundExportSel.size() > 1)
            ? path.parent_path() / (m_exportUseNames ? sound->GetName() + sound->GetImportPath().extension().string() : sound->GetImportPath().filename().string())
            : path;
         if (!table->ExportSound(sound, file))
            failed++;
      }
      m_pendingSoundExportSel.clear();
      if (failed > 0)
         m_actionStatus = "Failed to export "s + std::to_string(failed) + " sound(s)"s;
      g_settingsService.GetAppSettings().SetRecentDir_SoundDir(path.parent_path().string(), false);
   }

   if (props.BeginSection("Actions"s))
   {
      const bool canImport = editor.m_player->m_playfieldWnd != nullptr;
      ImGui::BeginDisabled(!canImport);
      if (ImGui::Button("Import"))
      {
         m_pendingSoundImport = std::make_shared<vector<string>>();
         const SDL_DialogFileFilter filters[] = { { "Sound Files", "wav;ogg;mp3" } };
         const string dir = g_settingsService.GetAppSettings().GetRecentDir_SoundDir();
         SDL_ShowOpenFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<vector<string>> *>(userdata);
               if (filelist != nullptr)
                  for (int i = 0; filelist[i] != nullptr; i++)
                     (**res).push_back(filelist[i]);
               delete res;
            },
            new std::shared_ptr<vector<string>>(m_pendingSoundImport), //
            editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), true);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty());
      if (ImGui::Button("Reimport"))
      {
         RequestConfirm("Reimport the selected sound(s) using their existing file path(s)?"s,
            [this]()
            {
               int missing = 0;
               for (VPX::Sound *const sound : m_editor.m_multiSelSounds)
                  if (std::filesystem::exists(sound->GetImportPath()))
                     m_editor.m_table->ReImportSound(sound, sound->GetImportPath());
                  else
                     missing++;
               if (missing > 0)
                  m_actionStatus = std::to_string(missing) + " file(s) not found"s;
               m_editor.m_table->SetNonUndoableDirty(eSaveDirty);
            });
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::BeginDisabled(sel.size() != 1);
      if (ImGui::Button("Reimport From"))
      {
         RequestConfirm("Reimport the selected sound using a different file path?"s,
            [this]()
            {
               if (m_editor.m_player->m_playfieldWnd == nullptr)
                  return;
               m_pendingSoundReimportTarget = m_editor.m_multiSelSounds.empty() ? nullptr : m_editor.m_multiSelSounds.back();
               m_pendingSoundReimport = std::make_shared<string>();
               const SDL_DialogFileFilter filters[] = { { "Sound Files", "wav;ogg;mp3" } };
               const string dir = g_settingsService.GetAppSettings().GetRecentDir_SoundDir();
               SDL_ShowOpenFileDialog(
                  [](void *userdata, const char *const *filelist, int filter)
                  {
                     auto *res = static_cast<std::shared_ptr<string> *>(userdata);
                     if (filelist != nullptr && filelist[0] != nullptr)
                        **res = filelist[0];
                     delete res;
                  },
                  new std::shared_ptr<string>(m_pendingSoundReimport), //
                  m_editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), false);
            });
      }
      ImGui::EndDisabled();

      ImGui::BeginDisabled(sel.empty() || !canImport);
      if (ImGui::Button("Export"))
      {
         m_pendingSoundExportSel = sel;
         m_pendingSoundExport = std::make_shared<string>();
         const SDL_DialogFileFilter filters[] = { { "Sound Files", "wav;ogg;mp3" } };
         const string dir = g_settingsService.GetAppSettings().GetRecentDir_SoundDir();
         SDL_ShowSaveFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<string> *>(userdata);
               if (filelist != nullptr && filelist[0] != nullptr)
                  **res = filelist[0];
               delete res;
            },
            new std::shared_ptr<string>(m_pendingSoundExport), //
            editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str());
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty());
      if (ImGui::Button("Toggle Table/Backglass"))
      {
         for (VPX::Sound *const sound : sel)
            sound->SetOutputTarget((sound->GetOutputTarget() != VPX::SNDOUT_BACKGLASS) ? VPX::SNDOUT_BACKGLASS : VPX::SNDOUT_TABLE);
         table->SetNonUndoableDirty(eSaveDirty);
      }
      ImGui::SameLine();
      if (ImGui::Button("Delete"))
      {
         RequestConfirm("Delete the selected sound(s)?"s,
            [this]()
            {
               EditorUI &editor = m_editor;
               for (VPX::Sound *const sound : editor.m_multiSelSounds)
                  editor.m_table->RemoveSound(sound);
               editor.m_multiSelSounds.clear();
               editor.m_outlinerSoundAnchor = nullptr;
               editor.m_selection = Selection();
               editor.m_table->SetNonUndoableDirty(eSaveDirty);
            });
      }
      ImGui::EndDisabled();

      ImGui::Checkbox("Use names on export", &m_exportUseNames);
      if (!m_actionStatus.empty())
         ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_actionStatus.c_str());
      props.EndSection();
   }
}

void PropertiesPanel::MaterialActions(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<Material *> &sel = editor.m_multiSelMaterials;

   // Apply asynchronous file dialog results
   if (m_pendingMaterialImport && !m_pendingMaterialImport->empty())
   {
      const string file = *m_pendingMaterialImport;
      m_pendingMaterialImport = nullptr;
      std::ifstream f(file, std::ios::binary);
      int version = 0, count = 0;
      f.read(reinterpret_cast<char *>(&version), sizeof(version));
      if (!f || version != MATERIAL_VERSION)
         m_actionStatus = "Materials are not compatible with this version!"s;
      else
      {
         f.read(reinterpret_cast<char *>(&count), sizeof(count));
         for (int i = 0; i < count && f; i++)
         {
            SaveMaterial mat;
            float elasticity = 0.f, elasticityFalloff = 0.f, friction = 0.f, scatterAngle = 0.f;
            f.read(reinterpret_cast<char *>(&mat), sizeof(SaveMaterial));
            f.read(reinterpret_cast<char *>(&elasticity), sizeof(float));
            f.read(reinterpret_cast<char *>(&elasticityFalloff), sizeof(float));
            f.read(reinterpret_cast<char *>(&friction), sizeof(float));
            f.read(reinterpret_cast<char *>(&scatterAngle), sizeof(float));
            if (!f)
               break;
            Material *const pmat = new Material(mat.bIsMetal ? Material::METAL : Material::BASIC, mat.fWrapLighting, mat.fRoughness, dequantizeUnsigned<8>(mat.fGlossyImageLerp),
               dequantizeUnsigned<8>(mat.fThickness), mat.fEdge, dequantizeUnsigned<7>(mat.bOpacityActive_fEdgeAlpha >> 1), mat.fOpacity, mat.cBase, mat.cGlossy, mat.cClearcoat,
               !!(mat.bOpacityActive_fEdgeAlpha & 1), elasticity, elasticityFalloff, friction, scatterAngle, 0xFFFFFFFF);
            pmat->m_name = mat.szName;
            table->AddMaterial(pmat);
            sel.push_back(pmat);
            editor.m_selection = Selection(pmat);
            editor.m_outlinerMaterialAnchor = pmat;
         }
         table->SetNonUndoableDirty(eSaveDirty);
         g_settingsService.GetAppSettings().SetRecentDir_MaterialDir(std::filesystem::path(file).parent_path().string(), false);
      }
   }
   if (m_pendingMaterialExport && !m_pendingMaterialExport->empty())
   {
      const string file = *m_pendingMaterialExport;
      m_pendingMaterialExport = nullptr;
      std::ofstream f(file, std::ios::binary | std::ios::trunc);
      if (!f)
         m_actionStatus = "Failed to export materials"s;
      else
      {
         constexpr int mv = MATERIAL_VERSION;
         const int count = static_cast<int>(m_pendingMaterialExportSel.size());
         f.write(reinterpret_cast<const char *>(&mv), sizeof(mv));
         f.write(reinterpret_cast<const char *>(&count), sizeof(count));
         for (const Material *const pmat : m_pendingMaterialExportSel)
         {
            SaveMaterial mat;
            mat.cBase = pmat->m_cBase;
            mat.cGlossy = pmat->m_cGlossy;
            mat.cClearcoat = pmat->m_cClearcoat;
            mat.fRoughness = pmat->m_fRoughness;
            mat.fGlossyImageLerp = quantizeUnsigned<8>(clamp(pmat->m_fGlossyImageLerp, 0.f, 1.f));
            mat.fThickness = quantizeUnsigned<8>(clamp(pmat->m_fThickness, 0.f, 1.f));
            mat.fEdge = pmat->m_fEdge;
            mat.fWrapLighting = pmat->m_fWrapLighting;
            mat.bIsMetal = pmat->m_type == Material::METAL;
            mat.fOpacity = pmat->m_fOpacity;
            mat.bOpacityActive_fEdgeAlpha = pmat->m_bOpacityActive ? 1 : 0;
            mat.bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(clamp(pmat->m_fEdgeAlpha, 0.f, 1.f)) << 1;
            strncpy_s(mat.szName, std::size(mat.szName), pmat->m_name.c_str());
            f.write(reinterpret_cast<const char *>(&mat), sizeof(SaveMaterial));
            f.write(reinterpret_cast<const char *>(&pmat->m_fElasticity), sizeof(float));
            f.write(reinterpret_cast<const char *>(&pmat->m_fElasticityFalloff), sizeof(float));
            f.write(reinterpret_cast<const char *>(&pmat->m_fFriction), sizeof(float));
            f.write(reinterpret_cast<const char *>(&pmat->m_fScatterAngle), sizeof(float));
         }
         m_pendingMaterialExportSel.clear();
         g_settingsService.GetAppSettings().SetRecentDir_MaterialDir(std::filesystem::path(file).parent_path().string(), false);
      }
   }

   if (props.BeginSection("Actions"s))
   {
      if (ImGui::Button("Add"))
      {
         Material *const pmat = new Material();
         table->AddMaterial(pmat);
         sel.clear();
         sel.push_back(pmat);
         editor.m_selection = Selection(pmat);
         editor.m_outlinerMaterialAnchor = pmat;
         table->SetNonUndoableDirty(eSaveDirty);
      }

      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty());
      if (ImGui::Button("Clone"))
      {
         vector<Material *> clones;
         for (const Material *const mat : sel)
         {
            Material *const pmat = new Material(mat);
            table->AddMaterial(pmat);
            clones.push_back(pmat);
         }
         sel = clones;
         editor.m_selection = Selection(clones.back());
         editor.m_outlinerMaterialAnchor = clones.back();
         table->SetNonUndoableDirty(eSaveDirty);
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(editor.m_player->m_playfieldWnd == nullptr);
      if (ImGui::Button("Import"))
      {
         m_pendingMaterialImport = std::make_shared<string>();
         const SDL_DialogFileFilter filters[] = { { "Material Files", "mat" } };
         const string dir = g_settingsService.GetAppSettings().GetRecentDir_MaterialDir();
         SDL_ShowOpenFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<string> *>(userdata);
               if (filelist != nullptr && filelist[0] != nullptr)
                  **res = filelist[0];
               delete res;
            },
            new std::shared_ptr<string>(m_pendingMaterialImport), //
            editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), false);
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty() || editor.m_player->m_playfieldWnd == nullptr);
      if (ImGui::Button("Export"))
      {
         m_pendingMaterialExportSel = sel;
         m_pendingMaterialExport = std::make_shared<string>();
         const SDL_DialogFileFilter filters[] = { { "Material Files", "mat" } };
         const string dir = g_settingsService.GetAppSettings().GetRecentDir_MaterialDir();
         SDL_ShowSaveFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<string> *>(userdata);
               if (filelist != nullptr && filelist[0] != nullptr)
                  **res = filelist[0];
               delete res;
            },
            new std::shared_ptr<string>(m_pendingMaterialExport), //
            editor.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str());
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(sel.empty());
      if (ImGui::Button("Delete"))
      {
         RequestConfirm("Delete the selected material(s)?"s,
            [this]()
            {
               EditorUI &editor = m_editor;
               for (Material *const mat : editor.m_multiSelMaterials)
                  editor.m_table->RemoveMaterial(mat);
               editor.m_multiSelMaterials.clear();
               editor.m_outlinerMaterialAnchor = nullptr;
               editor.m_selection = Selection();
               editor.m_table->SetNonUndoableDirty(eSaveDirty);
            });
      }
      ImGui::EndDisabled();

      if (!m_actionStatus.empty())
         ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_actionStatus.c_str());
      props.EndSection();
   }
}
}
