// license:GPLv3+

#include "core/stdafx.h"
#include "PropertiesPanel.h"

#include "audio/AudioPlayer.h"
#include "core/editablereg.h"
#include "core/player.h"
#include "core/SettingsService.h"
#include "parts/Collection.h"
#include "parts/Material.h"
#include "parts/Sound.h"
#include "parts/ball.h"
#include "parts/bumper.h"
#include "parts/decal.h"
#include "parts/dispreel.h"
#include "parts/flasher.h"
#include "parts/flipper.h"
#include "parts/gate.h"
#include "parts/hittarget.h"
#include "parts/kicker.h"
#include "parts/light.h"
#include "parts/pinbinary.h"
#include "parts/plunger.h"
#include "parts/primitive.h"
#include "parts/ramp.h"
#include "parts/rubber.h"
#include "parts/spinner.h"
#include "parts/surface.h"
#include "parts/textbox.h"
#include "parts/trigger.h"
#include "physics/PhysicsEngine.h"
#include "renderer/Texture.h"
#include "ui/live/EditorUI.h"
#include "ui/live/FileDialog.h"
#include "ui/live/LiveUI.h"
#include "utils/color.h"

#include "ui/live/IconsLucide.h"
#include "EditorTheme.h"
#include "PropertyPane.h"
#include "ui/live/ImGuiHelpers.h"

namespace VPX::EditorUI
{

PropertiesPanel::PropertiesPanel(EditorUI &editor)
   : EditorAreaView(editor)
{
}

PropertiesPanel::~PropertiesPanel() = default;

string PropertiesPanel::PartMaterial(const IEditable *part)
{
   switch (part->GetItemType())
   {
   case eItemPrimitive: return static_cast<const Primitive *>(part)->m_d.m_szMaterial;
   case eItemRamp: return static_cast<const Ramp *>(part)->m_d.m_szMaterial;
   case eItemSurface: return static_cast<const Surface *>(part)->m_d.m_szTopMaterial;
   case eItemDecal: return static_cast<const Decal *>(part)->m_d.m_szMaterial;
   case eItemFlipper: return static_cast<const Flipper *>(part)->m_d.m_szMaterial;
   case eItemHitTarget: return static_cast<const HitTarget *>(part)->m_d.m_szMaterial;
   case eItemPlunger: return static_cast<const Plunger *>(part)->m_d.m_szMaterial;
   case eItemRubber: return static_cast<const Rubber *>(part)->m_d.m_szMaterial;
   case eItemSpinner: return static_cast<const Spinner *>(part)->m_d.m_szMaterial;
   case eItemKicker: return static_cast<const Kicker *>(part)->m_d.m_szMaterial;
   case eItemTrigger: return static_cast<const Trigger *>(part)->m_d.m_szMaterial;
   case eItemGate: return static_cast<const Gate *>(part)->m_d.m_szMaterial;
   case eItemBumper: return static_cast<const Bumper *>(part)->m_d.m_szCapMaterial;
   default: return string();
   }
}

void PropertiesPanel::RenderPane(const AreaRect &rect, const int leafId)
{
   EditorUI &editor = m_editor;
   char name[32];
   snprintf(name, sizeof(name), "##Pane%d", leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * editor.m_liveUI.GetDPI(), 4.0f * editor.m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
   ImGui::PushID(leafId);
   editor.RenderAreaTypeButton(leafId);
   ImGui::Separator();

   // Results of the resource actions' asynchronous file dialogs, applied every frame whichever tab
   // is shown: the pane that requested the action may not be the one rendered when it answers
   ApplyPendingImageActions();
   ApplyPendingSoundActions();
   ApplyPendingMaterialActions();

   // Multi-edit only applies to uniform selections: all selected parts of the same type as the active one
   const bool uniformPartSel = editor.m_selection.GetType() == Selection::S_EDITABLE && editor.m_multiSel.size() > 1
      && std::ranges::all_of(editor.m_multiSel,
         [&editor](const std::shared_ptr<EditorUIPart> &part) { return part->GetEditable()->GetItemType() == editor.m_selection.GetPart()->GetEditable()->GetItemType(); });

   PropertyPane props(editor.m_table);
   switch (editor.m_units)
   {
   case EditorUI::Units::VPX: props.SetLengthUnit(PropertyPane::Unit::VPLength); break;
   case EditorUI::Units::Metric: props.SetLengthUnit(PropertyPane::Unit::Millimeters); break;
   case EditorUI::Units::Imperial: props.SetLengthUnit(PropertyPane::Unit::Inches); break;
   }
   if (uniformPartSel)
   {
      vector<IEditable *> coParts;
      for (const std::shared_ptr<EditorUIPart> &part : editor.m_multiSel)
         if (part != editor.m_selection.GetPart())
            coParts.push_back(part->GetEditable());
      props.SetCoEditedParts(coParts);
   }
   // The rail lists the resources of the displayed side of the selection (its startup copy while inspecting it)
   if (editor.IsInspectMode())
      props.SetShowStartup(m_inspectShowStartup);
   IEditable *const displayed
      = (editor.m_selection.GetType() == Selection::S_EDITABLE && !editor.m_table->IsLocked()) ? props.GetEditedPart<IEditable>(editor.m_selection.GetPart()->GetEditable()) : nullptr;
   const vector<UsedResource> tabs = BuildTabs(displayed);
   // The active tab may vanish with the selection or a field change: fall back to the item tab
   const auto matchActive = [this](const UsedResource &tab) { return tab.kind == m_tab && (tab.kind == PaneTab::Item || tab.kind == PaneTab::Collections || tab.name == m_tabResource); };
   const UsedResource *activeTab = &tabs[0];
   for (const UsedResource &tab : tabs)
      if (matchActive(tab))
         activeTab = &tab;
   m_tab = activeTab->kind;
   m_tabResource = activeTab->name;

   // Vertical rail of icon tabs, docked on the pane's left edge (the same look as the viewport sidebar's chips)
   const float dpi = editor.m_liveUI.GetDPI();
   float railW = 0.f;
   for (const UsedResource &tab : tabs)
      railW = max(railW, IconButtonInkSize(TabIcon(tab.kind)).x);
   ImGui::PushStyleColor(ImGuiCol_ChildBg, EditorTheme::V(EditorTheme::Strip));
   ImGui::BeginChild("##TabRail", ImVec2(railW + 4.f * dpi, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
   ImGui::PopStyleColor();
   int tabIndex = 0;
   for (const UsedResource &tab : tabs)
   {
      const bool active = activeTab == &tab;
      ImGui::PushStyleColor(ImGuiCol_Button, EditorTheme::V(active ? EditorTheme::TabActive : (EditorTheme::Strip & 0x00FFFFFFu) | (217u << 24)));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::V(EditorTheme::TabHover));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorTheme::V(EditorTheme::TabHover));
      ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(active ? EditorTheme::Accent : EditorTheme::Text));
      if (IconButtonInk(TabIcon(tab.kind), ("##tab" + std::to_string(tabIndex++)).c_str()))
      {
         m_tab = tab.kind;
         m_tabResource = tab.name;
      }
      if (tab.kind == PaneTab::Item)
         ImGui::SetItemTooltip("Item");
      else if (tab.kind == PaneTab::Collections)
         ImGui::SetItemTooltip("Collections");
      else
         ImGui::SetItemTooltip("%s: %s (%s)", TabKindName(tab.kind), tab.name.c_str(), tab.role.c_str());
      ImGui::PopStyleColor(4);
   }
   ImGui::EndChild();
   for (const UsedResource &tab : tabs) // A rail click above switched the active tab: re-resolve it
      if (matchActive(tab))
         activeTab = &tab;
   ImGui::SameLine(0.f, 0.f);

   // Scrolled content of the active tab (the scrollbar lives here so the rail's width stays stable)
   ImGui::BeginChild("##PaneContent", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);
   switch (activeTab->kind)
   {
   case PaneTab::Item: RenderItemTab(props, uniformPartSel); break;
   case PaneTab::Collections:
      if (editor.IsInspectMode())
         WithStartupLiveTabs(props, [this, &props](bool) { RenderCollectionsTab(props); });
      else
         RenderCollectionsTab(props);
      break;
   case PaneTab::Font:
   {
      // The font descriptor is a field of the part: it goes through the same undo wrapper as the
      // item pane (the part's lock scope is opened here since the item header's one is not rendered)
      const auto renderFont = [this, &props]()
      {
         ImGui::BeginDisabled(props.GetEditedPart<IEditable>(m_editor.m_selection.GetPart()->GetEditable())->IsUILocked());
         RenderFontTab(props);
         ImGui::EndDisabled();
      };
      if (editor.IsInspectMode())
         WithStartupLiveTabs(props, [&renderFont](bool) { renderFont(); });
      else
         RenderPartEditWithUndo(props, renderFont);
      break;
   }
   default: // A resource used by the part
   {
      // Materials and render probes exist on both sides of an inspected table; images and sounds are shared
      const bool dualState = activeTab->kind == PaneTab::Material || activeTab->kind == PaneTab::RenderProbe;
      m_inResourceTab = true;
      if (editor.IsInspectMode() && dualState)
      {
         WithStartupLiveTabs(props,
            [this, &props, activeTab](bool isLive)
            {
               RenderResourceTab(props, *activeTab);
               if (isLive && activeTab->kind == PaneTab::Material && props.GetModifiedField() > 0)
                  if (Material *const material = m_editor.m_table->GetMaterial(activeTab->name))
                     m_editor.m_liveEditedMaterials.insert(material);
            });
      }
      else
      {
         RenderResourceTab(props, *activeTab);
         if (props.GetModifiedField() > 0)
            editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      }
      m_inResourceTab = false;
      break;
   }
   }
   ImGui::EndChild();

   RenderPopups();

   ImGui::PopID();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

const char *PropertiesPanel::TabIcon(const PaneTab tab)
{
   switch (tab)
   {
   case PaneTab::Item: return ICON_LC_TAG;
   case PaneTab::Collections: return ICON_LC_LAYERS;
   case PaneTab::Material: return ICON_LC_GLOBE;
   case PaneTab::Image: return ICON_LC_IMAGE;
   case PaneTab::RenderProbe: return ICON_LC_VIDEO;
   case PaneTab::Sound: return ICON_LC_DISC;
   case PaneTab::Font: return ICON_LC_TYPE;
   }
   return ICON_LC_TAG;
}

const char *PropertiesPanel::TabKindName(const PaneTab tab)
{
   switch (tab)
   {
   case PaneTab::Material: return "Material";
   case PaneTab::Image: return "Image";
   case PaneTab::RenderProbe: return "Render Probe";
   case PaneTab::Sound: return "Sound";
   case PaneTab::Font: return "Font";
   default: return "Item";
   }
}

vector<PropertiesPanel::UsedResource> PropertiesPanel::BuildTabs(IEditable *displayed) const
{
   vector<UsedResource> tabs { { PaneTab::Item, ""s, ""s } };
   // Only scriptable parts may belong to a collection (membership is persisted by name)
   if (displayed == nullptr || displayed->GetIScriptable() == nullptr)
      return tabs;
   tabs.push_back({ PaneTab::Collections, ""s, ""s });
   CollectPartResources(displayed, tabs);
   return tabs;
}

// The table resources a part references, in field order. Deduplicated by (kind, name): a part
// referencing the same resource in several fields gets a single tab (its tooltip names the first field)
void PropertiesPanel::CollectPartResources(const IEditable *part, vector<UsedResource> &out)
{
   const auto add = [&out](const PaneTab kind, const string &name, const string &role)
   {
      if (!name.empty() && std::ranges::none_of(out, [&kind, &name](const UsedResource &tab) { return tab.kind == kind && tab.name == name; }))
         out.push_back({ kind, name, role });
   };
   switch (part->GetItemType())
   {
   case eItemPrimitive:
   {
      const Primitive *const primitive = static_cast<const Primitive *>(part);
      if (primitive->IsPlayfield())
      {
         // The playfield mesh's material and image are overridden by the table's playfield definition
         add(PaneTab::Material, primitive->m_ptable->m_playfieldMaterial, "Material"s);
         add(PaneTab::Image, primitive->m_ptable->m_image, "Image"s);
      }
      else
      {
         add(PaneTab::Material, primitive->m_d.m_szMaterial, "Material"s);
         add(PaneTab::Image, primitive->m_d.m_szImage, "Image"s);
      }
      // The implicit playfield mesh does not support normal mapping (the pane disables the field)
      if (part != g_pplayer->m_implicitPlayfieldMesh)
         add(PaneTab::Image, primitive->m_d.m_szNormalMap, "Normal Map"s);
      add(PaneTab::Material, primitive->m_d.m_szPhysicsMaterial, "Physics Material"s);
      add(PaneTab::RenderProbe, primitive->m_d.m_szReflectionProbe, "Reflection Probe"s);
      add(PaneTab::RenderProbe, primitive->m_d.m_szRefractionProbe, "Refraction Probe"s);
      break;
   }
   case eItemSurface:
   {
      const Surface *const surf = static_cast<const Surface *>(part);
      add(PaneTab::Material, surf->m_d.m_szTopMaterial, "Top Material"s);
      add(PaneTab::Material, surf->m_d.m_szSideMaterial, "Side Material"s);
      add(PaneTab::Material, surf->m_d.m_szSlingShotMaterial, "Slingshot Material"s);
      add(PaneTab::Material, surf->m_d.m_szPhysicsMaterial, "Physics Material"s);
      add(PaneTab::Image, surf->m_d.m_szImage, "Top Image"s);
      add(PaneTab::Image, surf->m_d.m_szSideImage, "Side Image"s);
      break;
   }
   case eItemBumper:
   {
      const Bumper *const bumper = static_cast<const Bumper *>(part);
      add(PaneTab::Material, bumper->m_d.m_szCapMaterial, "Cap Material"s);
      add(PaneTab::Material, bumper->m_d.m_szBaseMaterial, "Base Material"s);
      add(PaneTab::Material, bumper->m_d.m_szSkirtMaterial, "Skirt Material"s);
      add(PaneTab::Material, bumper->m_d.m_szRingMaterial, "Ring Material"s);
      break;
   }
   case eItemDecal:
   {
      const Decal *const decal = static_cast<const Decal *>(part);
      add(PaneTab::Material, decal->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Image, decal->m_d.m_szImage, "Image"s);
      add(PaneTab::Font, decal->m_d.m_font.name, "Font"s);
      break;
   }
   case eItemDispReel:
   {
      const DispReel *const dispReel = static_cast<const DispReel *>(part);
      add(PaneTab::Image, dispReel->m_d.m_szImage, "Image"s);
      add(PaneTab::Sound, dispReel->m_d.m_szSound, "Sound"s);
      break;
   }
   case eItemFlasher:
   {
      const Flasher *const flasher = static_cast<const Flasher *>(part);
      add(PaneTab::Image, flasher->m_d.m_szImageA, "Image A"s);
      add(PaneTab::Image, flasher->m_d.m_szImageB, "Image B"s);
      break;
   }
   case eItemFlipper:
   {
      const Flipper *const flipper = static_cast<const Flipper *>(part);
      add(PaneTab::Material, flipper->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Material, flipper->m_d.m_szRubberMaterial, "Rubber Material"s);
      add(PaneTab::Image, flipper->m_d.m_szImage, "Image"s);
      break;
   }
   case eItemGate: add(PaneTab::Material, static_cast<const Gate *>(part)->m_d.m_szMaterial, "Material"s); break;
   case eItemHitTarget:
   {
      const HitTarget *const hitTarget = static_cast<const HitTarget *>(part);
      add(PaneTab::Material, hitTarget->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Material, hitTarget->m_d.m_szPhysicsMaterial, "Physics Material"s);
      add(PaneTab::Image, hitTarget->m_d.m_szImage, "Image"s);
      break;
   }
   case eItemKicker: add(PaneTab::Material, static_cast<const Kicker *>(part)->m_d.m_szMaterial, "Material"s); break;
   case eItemLight: add(PaneTab::Image, static_cast<const Light *>(part)->m_d.m_szImage, "Image"s); break;
   case eItemPlunger:
   {
      const Plunger *const plunger = static_cast<const Plunger *>(part);
      add(PaneTab::Material, plunger->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Image, plunger->m_d.m_szImage, "Image"s);
      break;
   }
   case eItemRamp:
   {
      const Ramp *const ramp = static_cast<const Ramp *>(part);
      add(PaneTab::Material, ramp->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Material, ramp->m_d.m_szPhysicsMaterial, "Physics Material"s);
      add(PaneTab::Image, ramp->m_d.m_szImage, "Image"s);
      break;
   }
   case eItemRubber:
   {
      const Rubber *const rubber = static_cast<const Rubber *>(part);
      add(PaneTab::Material, rubber->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Material, rubber->m_d.m_szPhysicsMaterial, "Physics Material"s);
      add(PaneTab::Image, rubber->m_d.m_szImage, "Image"s);
      break;
   }
   case eItemSpinner:
   {
      const Spinner *const spinner = static_cast<const Spinner *>(part);
      add(PaneTab::Material, spinner->m_d.m_szMaterial, "Material"s);
      add(PaneTab::Image, spinner->m_d.m_szImage, "Image"s);
      break;
   }
   case eItemTrigger: add(PaneTab::Material, static_cast<const Trigger *>(part)->m_d.m_szMaterial, "Material"s); break;
   case eItemTextbox: add(PaneTab::Font, static_cast<const Textbox *>(part)->m_d.m_font.name, "Font"s); break;
   case eItemBall:
   {
      const Ball *const ball = static_cast<const Ball *>(part);
      add(PaneTab::Image, ball->m_d.m_szImage, "Ball Image"s);
      add(PaneTab::Image, ball->m_d.m_imageDecal, "Decal"s);
      break;
   }
   default: break;
   }
}

void PropertiesPanel::WithStartupLiveTabs(PropertyPane &props, const std::function<void(bool isLive)> &content)
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
            m_inspectShowStartup = !is_live;
            props.SetShowStartup(!is_live);
            content(is_live);
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
}

void PropertiesPanel::RenderItemTab(PropertyPane &props, const bool uniformPartSel)
{
   EditorUI &editor = m_editor;

   if (editor.m_multiSel.size() > 1)
      ImGui::TextDisabled(uniformPartSel ? "%d parts selected" : "%d parts selected (editing the active one)", (int)editor.m_multiSel.size());
   if (editor.m_multiSelImages.size() > 1)
      ImGui::TextDisabled("%d images selected (editing the active one)", (int)editor.m_multiSelImages.size());
   if (editor.m_multiSelSounds.size() > 1)
      ImGui::TextDisabled("%d sounds selected (editing the active one)", (int)editor.m_multiSelSounds.size());
   if (editor.m_multiSelMaterials.size() > 1)
      ImGui::TextDisabled("%d materials selected (editing the active one)", (int)editor.m_multiSelMaterials.size());

   if (editor.m_table->IsLocked())
   {
      // A locked table is read-only: the pane still shows its (disabled) properties so that the header's lock toggle can unlock it
      TableProperties(props);
      props.EndEditScope();
      if (props.GetModifiedField() > 0)
         editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      return;
   }
   if (editor.IsInspectMode() && editor.m_selection.GetType() != Selection::S_IMAGE
      && editor.m_selection.GetType() != Selection::S_SOUND) // Images & sounds are shared between live and startup instance, so they do not have 2 states
   {
      WithStartupLiveTabs(props,
         [this, &props](bool is_live)
         {
            EditorUI &editor = m_editor;
            switch (editor.m_selection.GetType())
            {
            case Selection::S_NONE:
               TableProperties(props);
               props.EndEditScope();
               break;
            case Selection::S_EDITABLE:
               editor.m_selection.GetPart()->UpdatePropertyPane(props);
               props.EndEditScope();
               if (props.GetModifiedField() > 0)
                  RefreshEditedParts(props);
               break;
            case Selection::S_IMAGE: ImageProperties(props, editor.m_selection.GetImage()); break;
            case Selection::S_CAMERA: CameraProperties(props, editor.m_selection.GetCamera()); break;
            case Selection::S_MATERIAL: MaterialProperties(props, editor.m_selection.GetMaterial()); break;
            case Selection::S_RENDERPROBE: RenderProbeProperties(props, editor.m_selection.GetProbe()); break;
            case Selection::S_SOUND: SoundProperties(props, editor.m_selection.GetSound()); break;
            case Selection::S_COLLECTION: CollectionProperties(props, editor.m_selection.GetCollection()); break;
            }
            // Remember what was edited on the live copy: 'Save' copies it back to the startup table that is written to disk
            if (is_live && props.GetModifiedField() > 0)
            {
               if (editor.m_selection.GetType() == Selection::S_MATERIAL)
                  editor.m_liveEditedMaterials.insert(editor.m_selection.GetMaterial());
               else if (editor.m_selection.GetType() == Selection::S_EDITABLE)
                  editor.m_liveEditedParts.insert(editor.m_selection.GetPart()->GetEditable());
            }
         });
   }
   else
   {
      switch (editor.m_selection.GetType())
      {
      case Selection::S_NONE:
         TableProperties(props);
         props.EndEditScope();
         break;
      case Selection::S_EDITABLE: RenderPartEditWithUndo(props, [&editor, &props]() { editor.m_selection.GetPart()->UpdatePropertyPane(props); }); break;
      case Selection::S_IMAGE: ImageProperties(props, editor.m_selection.GetImage()); break;
      case Selection::S_CAMERA: CameraProperties(props, editor.m_selection.GetCamera()); break;
      case Selection::S_MATERIAL: MaterialProperties(props, editor.m_selection.GetMaterial()); break;
      case Selection::S_RENDERPROBE: RenderProbeProperties(props, editor.m_selection.GetProbe()); break;
      case Selection::S_SOUND: SoundProperties(props, editor.m_selection.GetSound()); break;
      case Selection::S_COLLECTION: CollectionProperties(props, editor.m_selection.GetCollection()); break;
      }
      if (editor.m_selection.GetType() != Selection::S_EDITABLE && props.GetModifiedField() > 0)
         editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
   }
}

// The part field edits of the item and font tabs, wrapped in an undo record that collapses the
// whole edit gesture (a drag, a typed value) into a single undo step
void PropertiesPanel::RenderPartEditWithUndo(PropertyPane &props, const std::function<void()> &renderFields)
{
   EditorUI &editor = m_editor;
   IEditable *const editable = editor.m_selection.GetPart()->GetEditable();
   const string nameBefore = editable->GetName();
   editor.m_undo.BeginUndo();
   if (props.IsMultiEdit()) // Multi-edit applies the field changes to every selected part
   {
      for (const std::shared_ptr<EditorUIPart> &part : editor.m_multiSel)
         editor.m_undo.MarkForUndo(part->GetEditable());
   }
   else
      editor.m_undo.MarkForUndo(editable);
   // The playfield mesh pane edits table fields, which the primitive's undo state does not hold
   if (editable->GetItemType() == eItemPrimitive && static_cast<const Primitive *>(editable)->IsPlayfield())
   {
      PinTable *const table = static_cast<const Primitive *>(editable)->m_ptable;
      editor.m_undo.MarkForAction(
         [table, material = table->m_playfieldMaterial, image = table->m_image, showInEditor = table->m_winEditorBackdrop, reflectionStrength = table->m_playfieldReflectionStrength]()
         {
            table->m_playfieldMaterial = material;
            table->m_image = image;
            table->m_winEditorBackdrop = showInEditor;
            table->m_playfieldReflectionStrength = reflectionStrength;
         });
   }
   renderFields();
   props.EndEditScope();
   // A renamed surface or ramp retargets the parts placed on it, recorded in the same undo record
   editor.m_table->UpdateSurfaceReferences(editable, nameBefore, [&editor](IEditable *part) { editor.m_undo.MarkForUndo(part); });
   editor.m_undo.EndUndo();
   {
      // The edit of one field (a drag, a typed value) is one step: its first frame keeps the record, the next ones discard theirs.
      // The chain ends when the interaction does (mouse released, no item active) or when another item takes it
      // (Tab between components, a click on another field), so the next edit is a step of its own
      const bool modified = props.GetModifiedField() > 0;
      const unsigned int editId = 0x2000u | static_cast<unsigned int>(props.GetModifiedField());
      const bool continues = modified && editor.m_lastUndoPart == editable && editor.m_lastUndoId == editId;
      if (modified && !continues)
      {
         editor.m_lastUndoPart = editable;
         editor.m_lastUndoId = editId;
         m_editChainRecord = editor.m_undo.UndoCount();
      }
      else
      {
         editor.m_undo.Discard();
         // Esc cancelled the drag / typed value (restored): the step it had started is dropped too, if it is still the latest record
         if (!modified && props.ConsumeEditCancel() && editor.m_lastUndoPart == editable && editor.m_undo.HasUndo() && editor.m_undo.UndoCount() == m_editChainRecord)
         {
            editor.m_undo.Discard();
            editor.m_lastUndoPart = nullptr;
            editor.m_lastUndoId = 0;
            m_editChainRecord = 0;
         }
      }
      const bool ended = !modified && !ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsAnyItemActive();
      const ImGuiID activeId = ImGui::GetActiveID();
      if (editor.m_lastUndoId != 0x1000 // The gizmo's own marker is not ours to clear
         && (ended || (m_prevActiveId != 0 && activeId != m_prevActiveId && activeId != 0)))
      {
         editor.m_lastUndoPart = nullptr;
         editor.m_lastUndoId = 0;
         m_editChainRecord = 0;
      }
      m_prevActiveId = activeId;
   }
   if (props.GetModifiedField() > 0)
      RefreshEditedParts(props);
}

void PropertiesPanel::RenderCollectionsTab(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   IEditable *const displayed = props.GetEditedPart<IEditable>(editor.m_selection.GetPart()->GetEditable());
   PinTable *const displayedTable = displayed->GetPTable();

   // The membership is toggled for the whole selection: the displayed part plus every selected
   // scriptable part (the collections tab does not require a uniform selection like field edits do)
   vector<IEditable *> targets { displayed };
   for (const std::shared_ptr<EditorUIPart> &part : editor.m_multiSel)
   {
      if (part == editor.m_selection.GetPart())
         continue;
      IEditable *const co = props.GetEditedPart<IEditable>(part->GetEditable());
      if (co != nullptr && co->GetIScriptable() != nullptr && !co->IsUILocked())
         targets.push_back(co);
   }

   if (props.BeginSection("Collections"s))
   {
      if (displayedTable->GetCollections().empty())
         ImGui::TextDisabled("No collections");
      for (CComObject<Collection> *const col : displayedTable->GetCollections())
      {
         ImGui::PushID(col);
         int memberCount = 0;
         for (IEditable *const target : targets)
            if (std::ranges::find(col->GetParts(), target) != col->GetParts().end())
               memberCount++;
         bool member = memberCount == static_cast<int>(targets.size());
         ImGui::BeginDisabled(displayed->IsUILocked());
         ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, memberCount != 0 && !member);
         if (ImGui::Checkbox("##member", &member))
         {
            for (IEditable *const target : targets)
               if (member && std::ranges::find(col->GetParts(), target) == col->GetParts().end())
                  displayedTable->AddPartToCollection(col, target);
               else if (!member && std::ranges::find(col->GetParts(), target) != col->GetParts().end())
                  displayedTable->RemovePartFromCollection(col, target);
            displayedTable->SetNonUndoableDirty(SaveDirtyState::Dirty);
         }
         ImGui::PopItemFlag();
         ImGui::EndDisabled();
         ImGui::SameLine();
         // The collection itself is selectable like in the outliner (the startup one resolves to its live copy)
         if (ImGui::Selectable(col->m_name.c_str()))
         {
            Collection *const liveCol = (displayedTable == editor.m_table) ? col : editor.m_table->GetLiveFromStartup<Collection>(col);
            if (liveCol != nullptr)
               editor.SetSelection(Selection(liveCol));
         }
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%d member(s)", static_cast<int>(col->GetParts().size()));
         ImGui::PopID();
      }
      props.EndSection();
   }
   // Creating a collection always applies to the live table: disabled while inspecting a copy
   ImGui::BeginDisabled(editor.m_table->m_liveBaseTable != nullptr);
   if (ImGui::Button("New Collection From Selection"))
      editor.CreateCollection(true);
   ImGui::EndDisabled();
}

void PropertiesPanel::RenderResourceTab(PropertyPane &props, const UsedResource &res)
{
   EditorUI &editor = m_editor;
   IEditable *const displayed = props.GetEditedPart<IEditable>(editor.m_selection.GetPart()->GetEditable());
   PinTable *const displayedTable = displayed->GetPTable();
   PinTable *const liveTable = editor.m_table;

   // The resource is resolved by name on the displayed side (the field only holds its name), then
   // mapped to the live object so the pane's startup/live sync machinery applies on both sides
   Material *const material = (res.kind == PaneTab::Material) ? displayedTable->GetMaterial(res.name) : nullptr;
   Texture *const image = (res.kind == PaneTab::Image) ? displayedTable->GetImage(res.name) : nullptr;
   RenderProbe *const probe = (res.kind == PaneTab::RenderProbe) ? displayedTable->GetRenderProbe(res.name) : nullptr;
   VPX::Sound *sound = nullptr;
   if (res.kind == PaneTab::Sound)
      for (VPX::Sound *const snd : displayedTable->m_vsound)
         if (snd->GetName() == res.name)
            sound = snd;
   Material *const liveMaterial = (material == nullptr) ? nullptr : (displayedTable == liveTable ? material : liveTable->GetLiveFromStartup<Material>(material));
   RenderProbe *const liveProbe = (probe == nullptr) ? nullptr : (displayedTable == liveTable ? probe : liveTable->GetLiveFromStartup<RenderProbe>(probe));

   if (material == nullptr && image == nullptr && probe == nullptr && sound == nullptr)
   {
      ImGui::TextDisabled("Missing %s '%s'", TabKindName(res.kind), res.name.c_str());
      return;
   }

   // The field referencing the resource, and a shortcut selecting the resource itself (its own
   // pane carries the resource actions, which act on the editor selection)
   if (SmallIconButtonInk(ICON_LC_CHEVRON_RIGHT, "##select"))
   {
      switch (res.kind)
      {
      case PaneTab::Material:
         if (liveMaterial != nullptr)
            editor.SetSelection(Selection(liveMaterial));
         break;
      case PaneTab::Image: editor.SetSelection(Selection(image)); break;
      case PaneTab::RenderProbe:
         if (liveProbe != nullptr)
            editor.SetSelection(Selection(liveProbe));
         break;
      case PaneTab::Sound: editor.SetSelection(Selection(sound)); break;
      default: break;
      }
   }
   ImGui::SetItemTooltip("Select %s", res.name.c_str());
   ImGui::SameLine();
   ImGui::AlignTextToFramePadding();
   ImGui::TextDisabled("%s", res.role.c_str());

   switch (res.kind)
   {
   // A startup-side resource without a live counterpart (a failed mapping) still shows its own fields
   case PaneTab::Material: MaterialProperties(props, liveMaterial != nullptr ? liveMaterial : material); break;
   case PaneTab::Image: ImageProperties(props, image); break;
   case PaneTab::RenderProbe: RenderProbeProperties(props, liveProbe != nullptr ? liveProbe : probe); break;
   case PaneTab::Sound: SoundProperties(props, sound); break;
   default: break;
   }
}

void PropertiesPanel::RenderFontTab(PropertyPane &props)
{
   IEditable *const editable = m_editor.m_selection.GetPart()->GetEditable();
   IEditable *const displayed = props.GetEditedPart<IEditable>(editable);

   // The part's embedded font descriptor (the same fields as the item pane's font widgets)
   if (props.BeginSection("Font"s))
   {
      switch (editable->GetItemType())
      {
      case eItemTextbox:
         props.Font<Textbox>(
            static_cast<Textbox *>(editable), //
            [](const Textbox *textbox) { return textbox->m_d.m_font; }, //
            [](Textbox *textbox, const FontDesc &v) { textbox->m_d.m_font = v; });
         break;
      case eItemDecal:
         props.Font<Decal>(
            static_cast<Decal *>(editable), //
            [](const Decal *decal) { return decal->m_d.m_font; }, //
            [](Decal *decal, const FontDesc &v)
            {
               decal->m_d.m_font = v;
               decal->EnsureSize();
            });
         break;
      default: break;
      }
      props.EndSection();
   }

   // The bundled font binary the descriptor resolves to, when the table embeds one of the same name
   if (props.BeginSection("Bundled Font"s))
   {
      const FontDesc &desc = (editable->GetItemType() == eItemTextbox) ? static_cast<Textbox *>(displayed)->m_d.m_font : static_cast<Decal *>(displayed)->m_d.m_font;
      PinFont *bundled = nullptr;
      for (PinFont *const font : displayed->GetPTable()->GetFontList())
         if (StrCompareNoCase(font->m_name, desc.name))
            bundled = font;
      if (bundled != nullptr)
      {
         props.PropertyLabel("Name"s);
         ImGui::TextUnformatted(bundled->m_name.c_str());
         if (!bundled->m_path.empty())
         {
            props.PropertyLabel("File"s);
            ImGui::TextWrapped("%s", PathToUTF8(bundled->m_path).c_str());
         }
         props.PropertyLabel("Size"s);
         ImGui::Text("%d bytes", static_cast<int>(bundled->m_buffer.size()));
      }
      else
         ImGui::TextDisabled("No bundled font matches '%s'", desc.name.c_str());
      props.EndSection();
   }
}

nlohmann::json PropertiesPanel::SaveSetup() const { return { { "tab", static_cast<int>(m_tab) }, { "res", m_tabResource } }; }

void PropertiesPanel::LoadSetup(const nlohmann::json &j)
{
   if (!j.is_object())
      return;
   if (const auto it = j.find("tab"); it != j.end() && it->is_number_integer())
      m_tab = static_cast<PaneTab>(clamp(it->get<int>(), 0, static_cast<int>(PaneTab::Font)));
   if (const auto it = j.find("res"); it != j.end() && it->is_string())
      m_tabResource = it->get<string>();
}

void PropertiesPanel::RefreshEditedParts(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   const auto refresh = [&editor](const std::shared_ptr<EditorUIPart> &part)
   {
      editor.m_renderer->ReinitRenderable(part->GetEditable()->GetIRenderable());
      editor.m_player->m_physics->Update(part->GetEditable());
      // Multi-edit writes the co-edited parts' visibility fields directly: resync their editor overrides
      // (the active part's overrides are updated by the pane's own setters, so it must be excluded)
      if (part != editor.m_selection.GetPart())
         part->SyncVisibilityOverrides();
   };
   if (props.IsMultiEdit())
      for (const std::shared_ptr<EditorUIPart> &part : editor.m_multiSel)
         refresh(part);
   else
      refresh(editor.m_selection.GetPart());
}

void PropertiesPanel::TableProperties(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *table = props.GetEditedPart<PinTable>(editor.m_table);
   props.TableHeader(table);

   if (props.BeginSection("Visuals"s))
   {
      props.Separator("Playfield"s);
      props.MaterialCombo<PinTable>(
         table, "Material"s, //
         [](const PinTable *table) { return table->m_playfieldMaterial; }, //
         [](PinTable *table, const string &v) { table->m_playfieldMaterial = v; });
      props.ImageCombo<PinTable>(
         table, "Image"s, //
         [](const PinTable *table) { return table->m_image; }, //
         [](PinTable *table, const string &v) { table->m_image = v; });
      ImGui::BeginDisabled(table->m_image.empty());
      props.Checkbox<PinTable>(
         table, "Show in Editor"s, //
         [](const PinTable *table) { return table->m_winEditorBackdrop; }, //
         [](PinTable *table, bool v)
         {
            table->m_winEditorBackdrop = v;
            if (g_pplayer->m_implicitPlayfieldMesh)
               g_pplayer->m_implicitPlayfieldMesh->m_d.m_displayTexture = v;
         },
         "Draw the image on the playfield in the editor (does not change how it renders in game)");
      ImGui::EndDisabled();
      props.InputInt<PinTable>(
         table, "Reflection Strength"s, //
         [](const PinTable *table) { return table->GetPlayfieldReflectionStrength(); }, //
         [](PinTable *table, int v) { table->SetPlayfieldReflectionStrength(v); });

      props.Separator("Ball"s);
      props.ImageCombo<PinTable>(
         table, "Ball Image"s, //
         [](const PinTable *table) { return table->m_ballImage; }, //
         [](PinTable *table, const string &v) { table->m_ballImage = v; });
      props.Checkbox<PinTable>(
         table, "Spherical Map"s, //
         [](const PinTable *table) { return table->m_ballSphericalMapping; }, //
         [](PinTable *table, bool v) { table->m_ballSphericalMapping = v; }, "Wrap the image around the ball instead of projecting it flat");
      props.ImageCombo<PinTable>(
         table, "Ball Decal"s, //
         [](const PinTable *table) { return table->m_ballImageDecal; }, //
         [](PinTable *table, const string &v) { table->m_ballImageDecal = v; });
      props.Checkbox<PinTable>(
         table, "Logo Mode"s, //
         [](const PinTable *table) { return table->m_BallDecalMode; }, //
         [](PinTable *table, bool v) { table->m_BallDecalMode = v; }, "Draw the decal as a flat logo keeping its orientation instead of wrapping it on the ball");
      props.InputFloat<PinTable>(
         table, "Reflection of Playfield"s, //
         [](const PinTable *table) { return table->m_ballPlayfieldReflectionStrength; }, //
         [](PinTable *table, float v) { table->m_ballPlayfieldReflectionStrength = v; }, PropertyPane::Unit::None, 2, "Strength of the playfield's reflection on the ball");
      props.InputFloat<PinTable>(
         table, "Default Bulb Intensity Scale"s, //
         [](const PinTable *table) { return table->m_defaultBulbIntensityScaleOnBall; }, //
         [](PinTable *table, float v) { table->m_defaultBulbIntensityScaleOnBall = v; }, PropertyPane::Unit::None, 2, "Default scale of bulb light contribution on the ball");

      props.Separator("Backdrop"s);
      props.InputRGB<PinTable>(
         table, "Background Color"s, //
         [](const PinTable *table) { return convertColor(table->m_colorbackdrop); }, //
         [](PinTable *table, const vec3 &v) { table->m_colorbackdrop = convertColorRGB(v); });
      props.Checkbox<PinTable>(
         table, "Apply Night->Day cycle"s, //
         [](const PinTable *table) { return table->m_ImageBackdropNightDay; }, //
         [](PinTable *table, bool v) { table->m_ImageBackdropNightDay = v; }, "Tint the backdrop image according to the night/day setting");
      props.ImageCombo<PinTable>(
         table, "DT Image"s, //
         [](const PinTable *table) { return table->m_BG_image[0]; }, //
         [](PinTable *table, const string &v) { table->m_BG_image[0] = v; }, "Backdrop image in desktop view");
      props.ImageCombo<PinTable>(
         table, "FS Image"s, //
         [](const PinTable *table) { return table->m_BG_image[1]; }, //
         [](PinTable *table, const string &v) { table->m_BG_image[1] = v; }, "Backdrop image in fullscreen (cabinet) view");
      props.ImageCombo<PinTable>(
         table, "FSS Image"s, //
         [](const PinTable *table) { return table->m_BG_image[2]; }, //
         [](PinTable *table, const string &v) { table->m_BG_image[2] = v; }, "Backdrop image in full single screen view");
      props.ImageCombo<PinTable>(
         table, "Color Grading LUT"s, //
         [](const PinTable *table) { return table->m_imageColorGrade; }, //
         [](PinTable *table, const string &v) { table->m_imageColorGrade = v; }, "Lookup-table image applied to color grade the final render");
      props.Checkbox<PinTable>(
         table, "Enable EMReels"s, //
         [](const PinTable *table) { return table->m_renderEMReels; }, //
         [](PinTable *table, bool v) { table->m_renderEMReels = v; }, "Render the table's electromechanical reel displays");
      props.Checkbox<PinTable>(
         table, "Enable Decals"s, //
         [](const PinTable *table) { return table->m_renderDecals; }, //
         [](PinTable *table, bool v) { table->m_renderDecals = v; });
      props.Checkbox<PinTable>(
         table, "Enable FSS mode"s, //
         [](const PinTable *table) { return table->IsFSSEnabled(); }, //
         [](PinTable *table, bool v) { table->EnableFSS(v); }, "Full Single Screen mode: render the table and its backglass on a single display");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Combo<PinTable>(
         table, "Physics Profile"s, vector<string> { "10.8"s, "10.9"s }, //
         [](const PinTable *table) { return (int)table->m_physicProfile; }, //
         [](PinTable *table, int v) { table->m_physicProfile = (PhysicProfile)v; },
         "Physics engine behavior: 10.8 reproduces VPX 10.8 physics, 10.9 enables the corrected physics (see physconst.h). A change applies the next time the table is played");
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
         [](PinTable *table, float v) { table->m_elasticityFalloff = v; }, PropertyPane::Unit::None, 3, "Reduce elasticity for slower impacts (0 = constant elasticity)");
      props.InputFloat<PinTable>(
         table, "Playfield Friction"s, //
         [](const PinTable *table) { return table->m_friction; }, //
         [](PinTable *table, float v) { table->m_friction = max(v, 0.f); }, PropertyPane::Unit::None, 3);
      props.InputFloat<PinTable>(
         table, "Playfield Scatter Angle"s, //
         [](const PinTable *table) { return table->m_scatter; }, //
         [](PinTable *table, float v) { table->m_scatter = v; }, PropertyPane::Unit::Degree, 1, "Random angle added to the ball's bounce on the playfield");
      props.InputFloat<PinTable>(
         table, "Default Elements Scatter Angle"s, //
         [](const PinTable *table) { return table->m_defaultScatter; }, //
         [](PinTable *table, float v) { table->m_defaultScatter = v; }, PropertyPane::Unit::Degree, 1, "Scatter angle used by elements that don't define their own");
      props.Combo<PinTable>(
         table, "Overwrite Physics by Global Set"s, vector<string> { "Disable"s, "Set1"s, "Set2"s, "Set3"s, "Set4"s, "Set5"s, "Set6"s, "Set7"s, "Set8"s }, //
         [](const PinTable *table) { return table->m_overridePhysics; }, //
         [](PinTable *table, int v) { table->m_overridePhysics = v; }, "Apply a predefined global physics set to the whole table");
      props.Checkbox<PinTable>(
         table, "including Flipper Physics"s, //
         [](const PinTable *table) { return table->m_overridePhysicsFlipper; }, //
         [](PinTable *table, bool v) { table->m_overridePhysicsFlipper = v; }, "Also apply the physics set to flippers, which use their own physics values");
      props.InputFloat<PinTable>(
         table, "Nudge Time"s, //
         [](const PinTable *table) { return table->m_nudgeTime; }, //
         [](PinTable *table, float v) { table->m_nudgeTime = v; }, PropertyPane::Unit::None, 2, "Duration of the impulse applied to the table by a nudge");
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
         [](PinTable *table, float v) { table->m_angletiltMin = v; }, PropertyPane::Unit::Degree, 2, "Table incline at minimum game difficulty");
      props.InputFloat<PinTable>(
         table, "Slope for Max. Difficulty"s, //
         [](const PinTable *table) { return table->m_angletiltMax; }, //
         [](PinTable *table, float v) { table->m_angletiltMax = v; }, PropertyPane::Unit::Degree, 2, "Table incline at maximum game difficulty");
      props.InputFloat<PinTable>(
         table, "Game Difficulty"s, //
         [](const PinTable *table) { return table->m_difficulty; }, //
         [](PinTable *table, float v) { table->m_difficulty = v; }, PropertyPane::Unit::Percent, 0, "Interpolate the table's difficulty settings between the min & max slopes");
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
         PropertyPane::Unit::None, 0, "Intensity of the table's main light emission");
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
         PropertyPane::Unit::Percent, 1, "Global scale applied to all scene lighting");

      props.Separator("Environment Lighting"s);
      props.ImageCombo<PinTable>(
         table, "Environment Image"s, //
         [](const PinTable *table) { return table->m_envImage; }, //
         [&editor](PinTable *table, const string &v)
         {
            table->m_envImage = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         "Image used for ambient environment lighting");
      props.InputFloat<PinTable>(
         table, "Environment Em. Scale"s, //
         [](const PinTable *table) { return table->m_envEmissionScale; }, //
         [&editor](PinTable *table, float v)
         {
            table->m_envEmissionScale = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::Percent, 3, "Intensity of the environment lighting");

      props.Separator("Render Options"s);
      props.Checkbox<PinTable>(
         table, "Enable Ambient Occlusion"s, //
         [](const PinTable *table) { return table->m_enableAO; }, //
         [&editor](PinTable *table, bool v)
         {
            table->m_enableAO = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         "Darken creases and contact areas with ambient occlusion");
      props.InputFloat<PinTable>(
         table, "Ambient Occlusion Scale"s, //
         [](const PinTable *table) { return table->m_AOScale; }, //
         [](PinTable *table, float v) { table->m_AOScale = v; }, PropertyPane::Unit::Percent, 1, "Strength of the ambient occlusion");
      props.Checkbox<PinTable>(
         table, "Enable Sc. Sp. Reflections"s, //
         [](const PinTable *table) { return table->m_enableSSR; }, //
         [&editor](PinTable *table, bool v)
         {
            table->m_enableSSR = v;
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         "Reflect on-screen content on glossy surfaces (screen-space reflections)");
      props.InputFloat<PinTable>(
         table, "Screen Space Reflection Scale"s, //
         [](const PinTable *table) { return table->m_SSRScale; }, //
         [](PinTable *table, float v) { table->m_SSRScale = v; }, PropertyPane::Unit::Percent, 1, "Strength of the screen-space reflections");
      props.InputFloat<PinTable>(
         table, "Bloom Strength"s, //
         [](const PinTable *table) { return table->m_bloom_strength; }, //
         [](PinTable *table, float v) { table->m_bloom_strength = v; }, PropertyPane::Unit::Percent, 1, "Intensity of the glow around bright areas");
      props.Combo<PinTable>(
         table, "Tonemapping"s, vector<string> { "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s }, //
         [](const PinTable *table) { return static_cast<int>(table->GetToneMapper()); }, //
         [&editor](PinTable *table, int v)
         {
            table->SetToneMapper(static_cast<ToneMapper>(v));
            editor.m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         "How high dynamic range colors are mapped to the display");
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
   props.Header("Camera"s, [bgSet]() { return bgSet == 0 ? "Desktop"s : bgSet == 1 ? "Cabinet"s : "Full Single Screen"s; }, [](const string &) {});
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
         [](ViewSetup *viewSetup, int v) { viewSetup->mMode = static_cast<ViewLayoutMode>(v); }, "How the view is computed: legacy fixed projection, camera parameters, or window mapping");
      props.InputFloat<ViewSetup>(
         vs, "Field of View"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mFOV; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mFOV = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<ViewSetup>(
         vs, "Layback"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mLayback; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mLayback = v; }, PropertyPane::Unit::Degree, 1, "Fake tilt of the table toward the viewer (legacy view only)");
      props.InputFloat<ViewSetup>(
         vs, "Look At"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mLookAt; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mLookAt = v; }, PropertyPane::Unit::None, 2, "Height along the table the camera is aimed at");
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
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleX = v; }, PropertyPane::Unit::Percent, 3, "Stretch the rendered scene (compensates display distortion)");
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

   props.Header("Image"s, [texture]() { return texture->m_name; }, [texture](const string &v) { texture->m_name = v; });

   if (!m_inResourceTab) // The actions act on the editor's image selection, not on the pane's image
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
            [](Texture *image, float v) { image->m_alphaTestValue = v; }, PropertyPane::Unit::None, 2, "Pixels with alpha below this value are discarded (cutout transparency)");
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
   props.Header("Render Probe"s, [editedProbe]() { return editedProbe->GetName(); }, [editedProbe](const string &v) { editedProbe->SetName(v); });

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<RenderProbe>(
         probe, "Type"s, vector { "Reflection"s, "Refraction"s }, //
         [](const RenderProbe *probe) { return static_cast<int>(probe->GetType()); }, //
         [](RenderProbe *probe, int v) { probe->SetType(static_cast<RenderProbe::ProbeType>(v)); },
         "Reflection renders the scene mirrored on a plane; Refraction renders what is behind the surface");
      props.InputFloat3<RenderProbe>(
         probe, "Normal"s, //
         [](const RenderProbe *probe) { return probe->GetReflectionPlaneNormal(); }, //
         [](RenderProbe *probe, const vec3 &v) { probe->SetReflectionPlaneNormal(v); }, PropertyPane::Unit::None, 2, "Normal of the plane the scene is reflected on");
      props.InputFloat<RenderProbe>(
         probe, "Distance"s, //
         [](const RenderProbe *probe) { return probe->GetReflectionPlaneDistance(); }, //
         [](RenderProbe *probe, float v) { probe->SetReflectionPlaneDistance(v); }, PropertyPane::Unit::VPLength, 1, "Distance of the reflection plane along its normal");
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
   props.Header("Material"s, [editedMaterial]() { return editedMaterial->m_name; }, [editedMaterial](const string &v) { editedMaterial->m_name = v; });

   ImGui::BeginDisabled(m_editor.m_table->m_liveBaseTable != nullptr); // Material list actions are not supported in inspection mode
   if (!m_inResourceTab) // The actions act on the editor's material selection, not on the pane's material
      MaterialActions(props);
   ImGui::EndDisabled();

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Material>(
         material, "Type"s, vector { "Default"s, "Metal"s }, //
         [](const Material *material) { return material->m_type; }, //
         [](Material *material, int v) { material->m_type = static_cast<Material::MaterialType>(v); }, "How the material reflects light: dielectric (default) or metal");
      props.InputRGB<Material>(
         material, "Color", //
         [](const Material *material) { return convertColor(material->m_cBase); }, //
         [](Material *material, const vec3 &v) { material->m_cBase = convertColorRGB(v); });
      props.InputFloat<Material>(
         material, "Wrap Lighting"s, //
         [](const Material *material) { return material->m_fWrapLighting; }, //
         [](Material *material, float v) { material->m_fWrapLighting = v; }, PropertyPane::Unit::None, 2, "Wrap diffuse light around the part's edges, softening the shading");
      if (editedMaterial->m_type != Material::METAL)
      {
         props.InputRGB<Material>(
            material, "Glossy Color", //
            [](const Material *material) { return convertColor(material->m_cGlossy); }, //
            [](Material *material, const vec3 &v) { material->m_cGlossy = convertColorRGB(v); });
         props.InputFloat<Material>(
            material, "Glossy Image Lerp"s, //
            [](const Material *material) { return material->m_fGlossyImageLerp; }, //
            [](Material *material, float v) { material->m_fGlossyImageLerp = v; }, PropertyPane::Unit::None, 2,
            "How much specular highlights take the image's color instead of the glossy color");
      }
      props.InputFloat<Material>(
         material, "Shininess"s, //
         [](const Material *material) { return material->m_fRoughness; }, //
         [](Material *material, float v) { material->m_fRoughness = v; }, PropertyPane::Unit::None, 2, "Sharpness of the specular highlights (higher = smaller, brighter highlights)");
      props.InputRGB<Material>(
         material, "Clearcoat Color", //
         [](const Material *material) { return convertColor(material->m_cClearcoat); }, //
         [](Material *material, const vec3 &v) { material->m_cClearcoat = convertColorRGB(v); });
      props.InputFloat<Material>(
         material, "Edge Brightness"s, //
         [](const Material *material) { return material->m_fEdge; }, //
         [](Material *material, float v) { material->m_fEdge = v; }, PropertyPane::Unit::None, 2, "Extra brightness at grazing angles");
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
         [](Material *material, float v) { material->m_fEdgeAlpha = v; }, PropertyPane::Unit::None, 2, "Opacity at grazing angles");
      props.InputFloat<Material>(
         material, "Thickness"s, //
         [](const Material *material) { return material->m_fThickness; }, //
         [](Material *material, float v) { material->m_fThickness = v; }, PropertyPane::Unit::None, 2, "Simulated thickness used for refraction through transparent parts");
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
      "Sound"s, [sound]() { return sound->GetName(); },
      [&editor, sound](const string &v)
      {
         sound->SetName(v);
         editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      });

   ImGui::BeginDisabled(editor.m_table->m_liveBaseTable != nullptr); // Sound list actions are not supported in inspection mode as sounds are shared
   if (!m_inResourceTab) // The actions act on the editor's sound selection, not on the pane's sound
      SoundActions(props);
   ImGui::EndDisabled();

   ImGui::BeginDisabled(audioPlayer == nullptr);
   const string playStopLabel = isPlaying ? ICON_LC_SQUARE " Stop"s : ICON_LC_PLAY " Play"s;
   if (IconButtonInk(playStopLabel.c_str(), "##PlayStop"))
   {
      if (isPlaying)
      {
         audioPlayer->StopSound(sound);
         m_playingSound = nullptr;
         m_playingSoundObserved = false;
      }
      else
      {
         audioPlayer->PlaySound(sound, 1.0f, 0.f, 0, 0.f, 0.f, 0, false, true);
         m_playingSound = sound;
         m_playingSoundObserved = false;
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
            editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         },
         "Play this sound on the table's or the backglass' audio output");
      props.InputFloat<VPX::Sound>(
         sound, "Volume"s, //
         [](const VPX::Sound *s) { return dequantizeSignedPercent(s->GetVolume()); }, //
         [&editor](VPX::Sound *s, float v)
         {
            s->SetVolume(quantizeSignedPercent(v));
            editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         },
         PropertyPane::Unit::None, 2);
      props.InputFloat<VPX::Sound>(
         sound, "Pan"s, //
         [](const VPX::Sound *s) { return dequantizeSignedPercent(s->GetPan()); }, //
         [&editor](VPX::Sound *s, float v)
         {
            s->SetPan(quantizeSignedPercent(v));
            editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         },
         PropertyPane::Unit::None, 2);
      props.InputFloat<VPX::Sound>(
         sound, "Front/Rear Fade"s, //
         [](const VPX::Sound *s) { return dequantizeSignedPercent(s->GetFrontRearFade()); }, //
         [&editor](VPX::Sound *s, float v)
         {
            s->SetFrontRearFade(quantizeSignedPercent(v));
            editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         },
         PropertyPane::Unit::None, 2, "Balance between front and rear speakers (surround setups)");
      props.EndSection();
   }

   if (props.BeginSection("File"s))
   {
      if (m_soundInfoFor != sound)
      {
         m_soundInfoFor = sound;
         m_soundInfo = audioPlayer ? audioPlayer->GetSoundInformations(sound) : std::optional<VPX::SoundSpec>();
      }
      ImGui::TextWrapped("%s", PathToUTF8(sound->GetImportPath()).c_str());
      ImGui::TextDisabled("Size: %s", SizeToReadable(sound->GetFileSize()).c_str()); //
      if (m_soundInfo)
         ImGui::TextDisabled("%.2f s, %u Hz, %u channel%s", m_soundInfo->lengthInSeconds, m_soundInfo->sampleFrequency, m_soundInfo->nChannels, m_soundInfo->nChannels > 1 ? "s" : "");
      props.EndSection();
   }

   ImGui::EndDisabled();
}

void PropertiesPanel::CollectionProperties(PropertyPane &props, Collection *collection)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   // The displayed collection: when inspecting a live table, this is the live collection on the
   // 'Live' tab and its startup version on the 'Startup' tab (they are distinct objects)
   Collection *const edited = props.GetEditedPart<Collection>(collection);
   PinTable *const editedTable = (edited == collection) ? table : table->m_liveBaseTable;

   props.Header(
      "Collection"s, [edited]() { return edited->m_name; },
      [editedTable, edited](const string &v)
      {
         // Same uniqueness handling as the Win32 collection manager (names are script identifiers)
         string name = TruncateToUTF16Length(v, MAXNAMEBUFFER - 1);
         if (name.empty() || editedTable == nullptr)
            return;
         if (lowerCase(name) != lowerCase(edited->m_name) && !editedTable->IsNameUnique(name))
            name = editedTable->GetUniqueName(name);
         editedTable->RenameCollection(edited, name);
         editedTable->SetNonUndoableDirty(SaveDirtyState::Dirty);
      });

   // Actions on the table's collection list are only available when editing the base table
   // (the inspected table is a live copy whose changes would be lost when the play session ends)
   ImGui::BeginDisabled(table->m_liveBaseTable != nullptr);
   if (props.BeginSection("Actions"s))
   {
      if (ImGui::Button("New"))
         editor.CreateCollection(false);
      ImGui::SameLine();
      const auto &collections = table->GetCollections();
      int colIndex = -1;
      for (size_t i = 0; i < collections.size(); i++)
         if (collections[i] == collection)
            colIndex = static_cast<int>(i);
      ImGui::BeginDisabled(colIndex <= 0);
      if (ImGui::Button("Move Up"))
      {
         table->MoveCollectionUp(collections[colIndex]);
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      }
      ImGui::EndDisabled();
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Move the collection up in the table's collection list");
      ImGui::SameLine();
      ImGui::BeginDisabled(colIndex < 0 || colIndex >= static_cast<int>(collections.size()) - 1);
      if (ImGui::Button("Move Down"))
      {
         table->MoveCollectionDown(collections[colIndex]);
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      }
      ImGui::EndDisabled();
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Move the collection down in the table's collection list");
      ImGui::SameLine();
      if (ImGui::Button("Delete"))
      {
         RequestConfirm("Delete the selected collection?"s,
            [this, collection]()
            {
               EditorUI &editor = m_editor;
               for (CComObject<Collection> *const col : editor.m_table->GetCollections())
                  if (col == collection)
                  {
                     editor.m_table->RemoveCollection(col);
                     break;
                  }
               editor.m_selection = Selection();
               editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
            });
      }
      props.EndSection();
   }
   ImGui::EndDisabled();

   if (props.BeginSection("Options"s))
   {
      // Fields are synchronizable between the live and startup versions of the collection: the
      // setter may receive either one, so the table to flag as dirty is resolved from the object
      const auto markDirty = [&editor](Collection *col)
      {
         PinTable *dirtyTable = editor.m_table;
         if (dirtyTable->m_liveBaseTable && dirtyTable->GetLiveFromStartup<Collection>(col) != nullptr)
            dirtyTable = dirtyTable->m_liveBaseTable;
         dirtyTable->SetNonUndoableDirty(SaveDirtyState::Dirty);
      };
      props.Checkbox<Collection>(
         collection, "Fire events for this collection"s, //
         [](const Collection *col) { return col->m_fireEvents; }, //
         [&markDirty](Collection *col, bool v)
         {
            col->m_fireEvents = v;
            markDirty(col);
         },
         "Fire a Collection event when a member is hit, on top of the member's own event");
      props.Checkbox<Collection>(
         collection, "Suppress individual events for each member"s, //
         [](const Collection *col) { return col->m_stopSingleEvents; }, //
         [&markDirty](Collection *col, bool v)
         {
            col->m_stopSingleEvents = v;
            markDirty(col);
         },
         "Only fire the collection's event, not each member's own event");
      props.Checkbox<Collection>(
         collection, "Group elements together"s, //
         [](const Collection *col) { return col->m_groupElements; }, //
         [&markDirty](Collection *col, bool v)
         {
            col->m_groupElements = v;
            markDirty(col);
         },
         "Show this collection's members as a group in the editor");
      props.EndSection();
   }

   if (props.BeginSection("Content"s))
   {
      const vector<IEditable *> &members = edited->GetParts();

      // Member picker listing the table's scriptable parts that are not members yet
      props.PropertyLabel("Add part"s);
      ImGui::SetNextItemWidth(-FLT_MIN);
      if (ImGui::BeginCombo("##AddPart", "Select a part to add"))
      {
         vector<IEditable *> candidates;
         for (IEditable *const part : editedTable->GetParts())
            if (part->GetIScriptable() != nullptr && std::ranges::find(members, part) == members.end())
               candidates.push_back(part);
         std::ranges::sort(candidates, [](const IEditable *a, const IEditable *b) { return StrLessNoCase(a->GetName(), b->GetName()); });
         for (IEditable *const part : candidates)
            if (ImGui::Selectable(part->GetName().c_str()))
            {
               vector<IEditable *> newContent(members);
               newContent.push_back(part);
               editedTable->SetCollectionContent(edited, newContent);
               editedTable->SetNonUndoableDirty(SaveDirtyState::Dirty);
            }
         ImGui::EndCombo();
      }

      // Members, in the collection's order which is the order exposed to scripts
      if (members.empty())
         ImGui::TextDisabled("Empty collection");
      const float buttonWidth = IconButtonInkSize(ICON_LC_ARROW_UP).x;
      const float buttonsX = ImGui::GetContentRegionAvail().x - 3.f * buttonWidth - 2.f * ImGui::GetStyle().ItemSpacing.x;
      int moveIndex = -1, removeIndex = -1;
      for (int i = 0; i < static_cast<int>(members.size()); i++)
      {
         IEditable *const member = members[i];
         ImGui::PushID(i);
         if (ImGui::Selectable(member->GetName().c_str()))
         {
            // Select the part in the editor (a startup part resolves to its live counterpart)
            IEditable *const target = (edited == collection) ? member : table->GetLiveFromStartup<IEditable>(member);
            if (target != nullptr)
               if (const auto it = editor.m_editableMap.find(target); it != editor.m_editableMap.end())
                  editor.SetSelection(Selection(it->second));
         }
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", LocalString(EditableRegistry::GetTypeNameStringID(member->GetItemType())).m_szbuffer);
         ImGui::SameLine(buttonsX);
         ImGui::BeginDisabled(i == 0);
         if (SmallIconButtonInk(ICON_LC_ARROW_UP, "##MoveUp"))
            moveIndex = i - 1;
         ImGui::EndDisabled();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Move up");
         ImGui::SameLine();
         ImGui::BeginDisabled(i == static_cast<int>(members.size()) - 1);
         if (SmallIconButtonInk(ICON_LC_ARROW_DOWN, "##MoveDown"))
            moveIndex = i;
         ImGui::EndDisabled();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Move down");
         ImGui::SameLine();
         if (SmallIconButtonInk(ICON_LC_X, "##RemoveMember"))
            removeIndex = i;
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Remove from collection");
         ImGui::PopID();
      }
      if (moveIndex != -1 || removeIndex != -1)
      {
         vector<IEditable *> newContent(members);
         if (moveIndex != -1)
            std::swap(newContent[moveIndex], newContent[moveIndex + 1]);
         else
            newContent.erase(newContent.begin() + removeIndex);
         editedTable->SetCollectionContent(edited, newContent);
         editedTable->SetNonUndoableDirty(SaveDirtyState::Dirty);
      }
      props.EndSection();
   }
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
         if (editable == nullptr && where.whereUsedObjectname == m_editor.m_table->m_name)
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

// Replaces 'old' by the image loaded from 'path', keeping 'old's name. When the file holds the
// very same content, 'old' is kept and returned; when it fails to load, nullptr is returned.
// 'dup' is set to another table image with the same content, when there is one.
static Texture *ReimportImageFile(PinTable *table, Texture *const old, const std::filesystem::path &path, Texture **dup = nullptr)
{
   if (dup != nullptr)
      *dup = nullptr;
   Texture *const image = Texture::CreateFromFile(path);
   if (image == nullptr)
      return nullptr;
   if (dup != nullptr && !image->HasSameContent(*old))
      *dup = table->FindImageByContent(image);
   image->m_name = old->m_name;
   return table->ReImportImage(old, image);
}

// Update the selection when a reimport replaced an image (the old object is deleted)
void PropertiesPanel::ReplaceImageInSelection(Texture *oldImage, Texture *newImage)
{
   EditorUI &editor = m_editor;
   std::replace(editor.m_multiSelImages.begin(), editor.m_multiSelImages.end(), oldImage, newImage);
   if (editor.m_selection == Selection(oldImage))
      editor.m_selection = (newImage != nullptr) ? Selection(newImage) : Selection();
   if (editor.m_outlinerImageAnchor == oldImage)
      editor.m_outlinerImageAnchor = newImage;
}

void PropertiesPanel::ApplyPendingImageActions()
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<Texture *> &sel = editor.m_multiSelImages;

   if (m_pendingImageImport && !m_pendingImageImport->empty())
   {
      vector<Texture *> imported;
      int reused = 0;
      for (const string &file : *m_pendingImageImport)
         if (Texture *const loaded = Texture::CreateFromFile(PathFromUTF8(file)))
         {
            Texture *const tex = table->ImportImage(loaded);
            reused += (tex != loaded) ? 1 : 0; // A file identical to an existing image is not imported again: the existing image is used
            imported.push_back(tex);
         }
      g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(PathFromUTF8(m_pendingImageImport->front()).parent_path()), false);
      m_pendingImageImport = nullptr;
      if (reused > 0)
         m_editor.m_liveUI.PushNotification(std::to_string(reused) + " file(s) were already in the table: the existing image(s) were used"s, 8000);
      if (!imported.empty())
      {
         sel = imported;
         editor.m_selection = Selection(imported.back());
         editor.m_outlinerImageAnchor = imported.back();
         if (reused < static_cast<int>(imported.size()))
            table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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
         // ReimportImageFile replaces and deletes the old image, unless the file content is unchanged
         Texture *dup = nullptr;
         Texture *const tex = ReimportImageFile(table, old, PathFromUTF8(file), &dup);
         if (tex == nullptr)
            m_actionStatus = "Failed to reimport '"s + file + '\'';
         else
         {
            if (tex != old)
            {
               ReplaceImageInSelection(old, tex);
               table->SetNonUndoableDirty(SaveDirtyState::Dirty);
            }
            else
               m_editor.m_liveUI.PushNotification("'"s + old->m_name + "' is already up to date"s, 8000);
            if (dup != nullptr)
               m_editor.m_liveUI.PushNotification("'"s + tex->m_name + "' is now a duplicate of image '"s + dup->m_name + "'"s, 8000);
         }
         g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(PathFromUTF8(file).parent_path()), false);
      }
   }
   if (m_pendingImageExport && !m_pendingImageExport->empty())
   {
      const std::filesystem::path path = PathFromUTF8(*m_pendingImageExport);
      m_pendingImageExport = nullptr;
      // Like the Win32 dialog: with multiple selected images, only the folder of the picked file is used
      int failed = 0;
      for (Texture *tex : m_pendingImageExportSel)
      {
         const std::filesystem::path file = (m_pendingImageExportSel.size() > 1)
            ? path.parent_path() / (m_exportUseNames ? (PathFromString(tex->m_name) += tex->GetFilePath().extension()) : tex->GetFilePath().filename())
            : path;
         if (!tex->SaveFile(file))
            failed++;
      }
      m_pendingImageExportSel.clear();
      if (failed > 0)
         m_actionStatus = "Failed to export "s + std::to_string(failed) + " image(s)"s;
      g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(path.parent_path()), false);
   }
}

void PropertiesPanel::ImageActions(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<Texture *> &sel = editor.m_multiSelImages;

   if (props.BeginSection("Actions"s))
   {
      const bool canImport = editor.m_player->m_playfieldWnd != nullptr;
      ImGui::BeginDisabled(!canImport);
      if (ImGui::Button("Import"))
      {
         m_pendingImageImport = std::make_shared<vector<string>>();
         const SDL_DialogFileFilter filters[] = { { "Image Files", "bmp;jpg;jpeg;png;tga;webp;exr;hdr" } };
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_ImageDir())); // SDL expects UTF-8
         ShowOpenFileDialog(
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
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_ImageDir())); // SDL expects UTF-8
         ShowSaveFileDialog(
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
               editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
            });
      }
      ImGui::EndDisabled();

      // --

      ImGui::BeginDisabled(sel.empty());

      if (ImGui::Button("Reimport"))
      {
         RequestConfirm("Reimport the selected image(s) using their existing file path(s)?"s,
            [this]()
            {
               EditorUI &editor = m_editor;
               const vector<Texture *> sel = editor.m_multiSelImages;
               int missing = 0, unchanged = 0, replaced = 0, duplicated = 0;
               for (Texture *const old : sel)
                  if (!std::filesystem::exists(old->GetFilePath()))
                     missing++;
                  else
                  {
                     Texture *dup = nullptr;
                     if (Texture *const tex = ReimportImageFile(m_editor.m_table, old, old->GetFilePath(), &dup)) // On failure, the old image is kept
                     {
                        if (tex != old)
                        {
                           ReplaceImageInSelection(old, tex);
                           replaced++;
                           if (dup != nullptr)
                              duplicated++;
                        }
                        else
                           unchanged++;
                     }
                     else
                        missing++;
                  }
               if (missing > 0)
                  m_actionStatus = std::to_string(missing) + " file(s) not found or failed to load";
               else if (unchanged > 0)
                  m_actionStatus = std::to_string(unchanged) + " image(s) already up to date"s;
               if (duplicated > 0)
                  m_editor.m_liveUI.PushNotification(std::to_string(duplicated) + " image(s) are now duplicates of existing images"s, 8000);
               if (replaced > 0)
                  m_editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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
               const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_ImageDir())); // SDL expects UTF-8
               ShowOpenFileDialog(
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
         int missing = 0, unchanged = 0, replaced = 0, duplicated = 0;
         for (Texture *const old : all)
            if (!std::filesystem::exists(old->GetFilePath()))
               missing++;
            else
            {
               Texture *dup = nullptr;
               if (Texture *const tex = ReimportImageFile(table, old, old->GetFilePath(), &dup)) // On failure, the old image is kept
               {
                  if (tex != old)
                  {
                     ReplaceImageInSelection(old, tex);
                     replaced++;
                     if (dup != nullptr)
                        duplicated++;
                  }
                  else
                     unchanged++;
               }
               else
                  missing++;
            }
         if (missing > 0)
            m_actionStatus = std::to_string(missing) + " file(s) not found or failed to load";
         else if (unchanged > 0)
            m_actionStatus = std::to_string(unchanged) + " image(s) already up to date"s;
         if (duplicated > 0)
            m_editor.m_liveUI.PushNotification(std::to_string(duplicated) + " image(s) are now duplicates of existing images"s, 8000);
         if (replaced > 0)
            table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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

void PropertiesPanel::ApplyPendingSoundActions()
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<VPX::Sound *> &sel = editor.m_multiSelSounds;

   if (m_pendingSoundImport && !m_pendingSoundImport->empty())
   {
      vector<VPX::Sound *> imported;
      for (const string &file : *m_pendingSoundImport)
         if (VPX::Sound *const sound = table->ImportSound(PathFromUTF8(file)))
            imported.push_back(sound);
      g_settingsService.GetAppSettings().SetRecentDir_SoundDir(PathToString(PathFromUTF8(m_pendingSoundImport->front()).parent_path()), false);
      m_pendingSoundImport = nullptr;
      if (!imported.empty())
      {
         sel = imported;
         editor.m_selection = Selection(imported.back());
         editor.m_outlinerSoundAnchor = imported.back();
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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
         table->ReImportSound(m_pendingSoundReimportTarget, PathFromUTF8(file));
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      }
      m_pendingSoundReimportTarget = nullptr;
      g_settingsService.GetAppSettings().SetRecentDir_SoundDir(PathToString(PathFromUTF8(file).parent_path()), false);
   }
   if (m_pendingSoundExport && !m_pendingSoundExport->empty())
   {
      const std::filesystem::path path = PathFromUTF8(*m_pendingSoundExport);
      m_pendingSoundExport = nullptr;
      // Like the Win32 dialog: with multiple selected sounds, only the folder of the picked file is used
      int failed = 0;
      for (VPX::Sound *sound : m_pendingSoundExportSel)
      {
         const std::filesystem::path file = (m_pendingSoundExportSel.size() > 1)
            ? path.parent_path() / (m_exportUseNames ? (PathFromString(sound->GetName()) += sound->GetImportPath().extension()) : sound->GetImportPath().filename())
            : path;
         if (!table->ExportSound(sound, file))
            failed++;
      }
      m_pendingSoundExportSel.clear();
      if (failed > 0)
         m_actionStatus = "Failed to export "s + std::to_string(failed) + " sound(s)"s;
      g_settingsService.GetAppSettings().SetRecentDir_SoundDir(PathToString(path.parent_path()), false);
   }
}

void PropertiesPanel::SoundActions(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<VPX::Sound *> &sel = editor.m_multiSelSounds;

   if (props.BeginSection("Actions"s))
   {
      const bool canImport = editor.m_player->m_playfieldWnd != nullptr;
      ImGui::BeginDisabled(!canImport);
      if (ImGui::Button("Import"))
      {
         m_pendingSoundImport = std::make_shared<vector<string>>();
         const SDL_DialogFileFilter filters[] = { { "Sound Files", "wav;ogg;opus;mp3;flac" } };
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_SoundDir())); // SDL expects UTF-8
         ShowOpenFileDialog(
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
               m_editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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
               const SDL_DialogFileFilter filters[] = { { "Sound Files", "wav;ogg;opus;mp3;flac" } };
               const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_SoundDir())); // SDL expects UTF-8
               ShowOpenFileDialog(
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
         const SDL_DialogFileFilter filters[] = { { "Sound Files", "wav;ogg;opus;mp3;flac" } };
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_SoundDir())); // SDL expects UTF-8
         ShowSaveFileDialog(
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
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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
               editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
            });
      }
      ImGui::EndDisabled();

      ImGui::Checkbox("Use names on export", &m_exportUseNames);
      if (!m_actionStatus.empty())
         ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_actionStatus.c_str());
      props.EndSection();
   }
}

void PropertiesPanel::ApplyPendingMaterialActions()
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<Material *> &sel = editor.m_multiSelMaterials;

   if (m_pendingMaterialImport && !m_pendingMaterialImport->empty())
   {
      const string file = *m_pendingMaterialImport;
      m_pendingMaterialImport = nullptr;
      std::ifstream f(PathFromUTF8(file), std::ios::binary);
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
            pmat->m_name = string_from_utf8_or_cp1252(mat.szName, strnlen(mat.szName, std::size(mat.szName))); // May not be null terminated in a corrupted file
            table->AddMaterial(pmat);
            sel.push_back(pmat);
            editor.m_selection = Selection(pmat);
            editor.m_outlinerMaterialAnchor = pmat;
         }
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         g_settingsService.GetAppSettings().SetRecentDir_MaterialDir(PathToString(PathFromUTF8(file).parent_path()), false);
      }
   }
   if (m_pendingMaterialExport && !m_pendingMaterialExport->empty())
   {
      const string file = *m_pendingMaterialExport;
      m_pendingMaterialExport = nullptr;
      std::ofstream f(PathFromUTF8(file), std::ios::binary | std::ios::trunc);
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
            strncpy_s(mat.szName, std::size(mat.szName), TruncateToUTF8Length(pmat->m_name, std::size(mat.szName) - 1).c_str()); // Do not split a UTF-8 character
            f.write(reinterpret_cast<const char *>(&mat), sizeof(SaveMaterial));
            f.write(reinterpret_cast<const char *>(&pmat->m_fElasticity), sizeof(float));
            f.write(reinterpret_cast<const char *>(&pmat->m_fElasticityFalloff), sizeof(float));
            f.write(reinterpret_cast<const char *>(&pmat->m_fFriction), sizeof(float));
            f.write(reinterpret_cast<const char *>(&pmat->m_fScatterAngle), sizeof(float));
         }
         m_pendingMaterialExportSel.clear();
         g_settingsService.GetAppSettings().SetRecentDir_MaterialDir(PathToString(PathFromUTF8(file).parent_path()), false);
      }
   }
}

void PropertiesPanel::MaterialActions(PropertyPane &props)
{
   EditorUI &editor = m_editor;
   PinTable *const table = editor.m_table;
   vector<Material *> &sel = editor.m_multiSelMaterials;

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
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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
         table->SetNonUndoableDirty(SaveDirtyState::Dirty);
      }
      ImGui::EndDisabled();

      ImGui::SameLine();
      ImGui::BeginDisabled(editor.m_player->m_playfieldWnd == nullptr);
      if (ImGui::Button("Import"))
      {
         m_pendingMaterialImport = std::make_shared<string>();
         const SDL_DialogFileFilter filters[] = { { "Material Files", "mat" } };
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_MaterialDir())); // SDL expects UTF-8
         ShowOpenFileDialog(
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
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_MaterialDir())); // SDL expects UTF-8
         ShowSaveFileDialog(
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
               editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
            });
      }
      ImGui::EndDisabled();

      if (!m_actionStatus.empty())
         ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_actionStatus.c_str());
      props.EndSection();
   }
}

void PropertiesPanel::AppendStatusHints(vector<StatusHint> &hints) const { hints = { { "Wheel", "Scroll" } }; }
}
