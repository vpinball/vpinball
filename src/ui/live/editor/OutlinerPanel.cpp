// license:GPLv3+

#include "core/stdafx.h"
#include "OutlinerPanel.h"
#include "ui/live/IconsLucide.h"
#include "EditorTheme.h"
#include "EditorChrome.h"

#include "parts/Collection.h"
#include "parts/Material.h"
#include "parts/PartGroup.h"
#include "parts/Sound.h"
#include "PropertiesPanel.h"
#include "renderer/Texture.h"
#include "ui/live/EditorUI.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"
#include "ViewportPane.h"

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace VPX::EditorUI
{

std::shared_ptr<EditorUIPart> OutlinerPanel::m_anchor;
std::shared_ptr<EditorUIPart> OutlinerPanel::m_savedAnchor;


template <class T> static std::vector<T> SortedCaseInsensitive(std::vector<T> &list, const std::function<string(T)> &map)
{
   std::vector<T> sorted(list.begin(), list.end());
   std::ranges::sort(sorted, [&map](const T &a, const T &b) { return StrLessNoCase(map(a), map(b)); });
   return sorted;
}

// Push a dimmed selection color, used to highlight the non active elements of a multi selection
// less than the active one (the element edited in the property pane keeps the normal selection color).
// Pushes 3 colors (Header, HeaderHovered, HeaderActive): pop with PopStyleColor(3)
static void PushInactiveSelectionColor()
{
   const ImVec4 color = EditorTheme::V(EditorTheme::RowSelected);
   const ImVec4 hover = EditorTheme::V(EditorTheme::RowSelectedHover);
   ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(color.x, color.y, color.z, color.w * 0.4f));
   ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(hover.x, hover.y, hover.z, hover.w * 0.5f));
   ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(hover.x, hover.y, hover.z, hover.w * 0.5f));
}

// Name shortened with "..." so that it fits in maxWidth (whole UTF-8 characters are removed)
static string EllipsizeToWidth(const string &text, const float maxWidth)
{
   ImFont *const font = ImGui::GetFont();
   const float fontSize = ImGui::GetFontSize();
   const auto widthOf = [font, fontSize](const char *begin, const char *end) { return font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, begin, end).x; };
   const char *const begin = text.c_str();
   const char *const end = begin + text.size();
   if (widthOf(begin, end) <= maxWidth)
      return text;
   // Cut at the last whole UTF-8 character leaving room for the ellipsis (binary search on character boundaries)
   const float limit = maxWidth - widthOf("...", &"..."[3]);
   const auto isCont = [&text](const size_t i) { return (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80; };
   size_t lo = 0, hi = text.size();
   while (lo < hi)
   {
      size_t mid = lo + (hi - lo + 1) / 2;
      while (mid > lo && isCont(mid))
         --mid; // Do not measure a partial UTF-8 sequence
      if (mid == lo)
      {
         // The character starting at lo spans the probe point: the boundary right after it is the only remaining candidate
         size_t b = lo + 1;
         while (b < text.size() && isCont(b))
            ++b;
         if (b > hi || widthOf(begin, begin + b) > limit)
            break;
         lo = b;
      }
      else if (widthOf(begin, begin + mid) <= limit)
         lo = mid;
      else
         hi = mid - 1;
   }
   return text.substr(0, lo) + "...";
}

// Push the full row selection color of the outliner (active selection)
// (3 colors, pop with PopStyleColor(3)): a hovered selected row turns a lighter blue, not grey
static void PushSelectionColor()
{
   ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::V(EditorTheme::RowSelected));
   ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::V(EditorTheme::RowSelectedHover));
   ImGui::PushStyleColor(ImGuiCol_HeaderActive, EditorTheme::V(EditorTheme::RowSelectedHover));
}

// Row colors of a tree row: selection blue for selected rows only, a neutral grey fill for the hovered rows (3 colors, pop with PopStyleColor(3))
static void PushRowColors(const bool selected, const bool inactive)
{
   if (selected)
   {
      if (inactive)
         PushInactiveSelectionColor();
      else
         PushSelectionColor();
      return;
   }
   ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.f, 0.f, 0.f, 0.f));
   ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::V(EditorTheme::Hover));
   ImGui::PushStyleColor(ImGuiCol_HeaderActive, EditorTheme::V(EditorTheme::Hover));
}

// Tree node with a line chevron instead of ImGui's filled triangle: the node is submitted with an invisible text (same ID, click, keyboard and
// open state behaviour), then the chevron and the label are drawn over it
bool OutlinerPanel::ChevronTreeNode(const char *label, const ImGuiTreeNodeFlags flags, const char *icon, const bool iconBadge)
{
   const ImGuiStyle &style = ImGui::GetStyle();
   const ImVec2 pos = ImGui::GetCursorScreenPos();
   const float textOffsetY = ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset;
   const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
   if (m_setAllOpen >= 0)
      ImGui::GetStateStorage()->SetInt(ImGui::GetID(label), m_setAllOpen); // Expand/collapse all: written before the node reads its open state
   ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.f));
   const bool opened = ImGui::TreeNodeEx(label, flags);
   if (opened)
      ++m_openNodeCount;
   ImGui::PopStyleColor();
   const ImVec2 itemMin = ImGui::GetItemRectMin(), itemMax = ImGui::GetItemRectMax();
   const float fontSize = ImGui::GetFontSize();
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   if ((flags & ImGuiTreeNodeFlags_Leaf) == 0)
      EditorChrome::DrawChevron(drawList, ImVec2(pos.x + style.FramePadding.x + fontSize * 0.5f, 0.5f * (itemMin.y + itemMax.y)),
         opened ? EditorChrome::ChevronDir::Down : EditorChrome::ChevronDir::Right, fontSize * 0.25f, EditorTheme::TextDim);
   const ImVec2 labelPos(pos.x + ImGui::GetTreeNodeToLabelSpacing(), pos.y + textOffsetY);
   const char *const labelEnd = ImGui::FindRenderedTextEnd(label);
   if (icon == nullptr)
   {
      drawList->AddText(labelPos, textColor, label, labelEnd);
      return opened;
   }
   // Leading icon drawn ink-centered on the row: the glyph ink overshoots the line box, sitting too high on the text origin
   const ImVec4 ink = TextInkBounds(icon);
   const ImVec2 inkMin(labelPos.x, 0.5f * (itemMin.y + itemMax.y - (ink.w - ink.y)));
   if (iconBadge)
      // Grey chip behind the icon: the group in which new and imported parts are added
      drawList->AddRectFilled(ImVec2(inkMin.x - 4.f, inkMin.y - 3.f), ImVec2(inkMin.x + (ink.z - ink.x) + 4.f, inkMin.y + (ink.w - ink.y) + 3.f), EditorTheme::Hover, 4.f);
   drawList->AddText(ImVec2(inkMin.x - ink.x, inkMin.y - ink.y), textColor, icon);
   drawList->AddText(ImVec2(labelPos.x + ImGui::CalcTextSize(icon).x, labelPos.y), textColor, label + strlen(icon), labelEnd);
   return opened;
}

namespace
{
   // Type display names, with an alias also accepted by the 't:' filter token
   struct TypeInfo
   {
      ItemTypeEnum type;
      const char *name;
      const char *alias;
   };
   constexpr TypeInfo TypeInfos[] = {
      { eItemSurface, "Surface", "wall" },
      { eItemFlipper, "Flipper", "" },
      { eItemTimer, "Timer", "" },
      { eItemPlunger, "Plunger", "" },
      { eItemTextbox, "Textbox", "" },
      { eItemBumper, "Bumper", "" },
      { eItemTrigger, "Trigger", "" },
      { eItemLight, "Light", "" },
      { eItemKicker, "Kicker", "" },
      { eItemDecal, "Decal", "" },
      { eItemGate, "Gate", "" },
      { eItemSpinner, "Spinner", "" },
      { eItemRamp, "Ramp", "" },
      { eItemDispReel, "Display Reel", "dispreel" },
      { eItemLightSeq, "Light Seq", "lightseq" },
      { eItemPrimitive, "Primitive", "" },
      { eItemFlasher, "Flasher", "" },
      { eItemRubber, "Rubber", "" },
      { eItemHitTarget, "Target", "hittarget" },
      { eItemBall, "Ball", "" },
      { eItemPartGroup, "Part Group", "partgroup" },
   };

   const TypeInfo *FindTypeInfo(ItemTypeEnum type)
   {
      for (const TypeInfo &info : TypeInfos)
         if (info.type == type)
            return &info;
      return nullptr;
   }

   // prefix must already be lowercase
   bool StartsWithNoCase(const char *text, const string &prefix)
   {
      for (const char p : prefix)
      {
         if (*text == '\0' || cLower(*text) != p)
            return false;
         ++text;
      }
      return true;
   }

   // Object type categories of the filter popover (bit index in the type mask)
   enum TypeCategory
   {
      CatPrimitives,
      CatLights,
      CatFlashers,
      CatRamps,
      CatWalls,
      CatRubbers,
      CatFlippers,
      CatBumpers,
      CatTargets,
      CatGatesSpinners,
      CatTriggersKickers,
      CatOther,
      CatCount
   };
   static_assert(OutlinerPanel::AllTypesMask == (1u << CatCount) - 1u);
   constexpr const char *CategoryLabels[CatCount]
      = { "Primitives", "Lights", "Flashers", "Ramps", "Walls", "Rubbers", "Flippers", "Bumpers", "Targets", "Gates / Spinners", "Triggers / Kickers", "Other" };

   TypeCategory CategoryOf(ItemTypeEnum type)
   {
      switch (type)
      {
      case eItemPrimitive: return CatPrimitives;
      case eItemLight: return CatLights;
      case eItemFlasher: return CatFlashers;
      case eItemRamp: return CatRamps;
      case eItemSurface: return CatWalls;
      case eItemRubber: return CatRubbers;
      case eItemFlipper: return CatFlippers;
      case eItemBumper: return CatBumpers;
      case eItemHitTarget: return CatTargets;
      case eItemGate:
      case eItemSpinner: return CatGatesSpinners;
      case eItemTrigger:
      case eItemKicker: return CatTriggersKickers;
      default: return CatOther;
      }
   }

   const char *TypeIcon(ItemTypeEnum type)
   {
      switch (type)
      {
      case eItemPrimitive: return ICON_LC_BOX;
      case eItemLight: return ICON_LC_LIGHTBULB;
      case eItemFlasher: return ICON_LC_ZAP;
      case eItemRamp: return ICON_LC_ROUTE;
      case eItemSurface: return ICON_LC_SQUARE;
      case eItemRubber: return ICON_LC_CIRCLE;
      case eItemFlipper: return ICON_LC_MOVE_3D; // Not a cursor
      case eItemBumper: return ICON_LC_DISC;
      case eItemHitTarget: return ICON_LC_CIRCLE_DOT;
      case eItemGate: return ICON_LC_MOVE_HORIZONTAL;
      case eItemSpinner: return ICON_LC_REFRESH_CW;
      case eItemTrigger: return ICON_LC_MINUS;
      case eItemKicker: return ICON_LC_CIRCLE;
      case eItemPlunger: return ICON_LC_ARROW_UP;
      case eItemTimer: return ICON_LC_TIMER;
      case eItemTextbox: return ICON_LC_TYPE;
      case eItemDecal: return ICON_LC_IMAGE;
      default: return ICON_LC_TAG;
      }
   }
}

void OutlinerPanel::ParseFilter()
{
   if (m_filter == m_parsedFilter)
      return;
   m_parsedFilter = m_filter;
   m_tokens.names.clear();
   m_tokens.types.clear();
   m_tokens.materials.clear();
   const string lowered = lowerCase(m_filter);
   size_t pos = 0;
   while (pos < lowered.size())
   {
      const size_t end = std::min(lowered.find(' ', pos), lowered.size());
      string token = lowered.substr(pos, end - pos);
      pos = end + 1;
      if (token.empty())
         continue;
      vector<string> *target = &m_tokens.names;
      if (token.size() >= 2 && token[1] == ':' && (token[0] == 't' || token[0] == 'm' || token[0] == 'n'))
      {
         target = token[0] == 't' ? &m_tokens.types : token[0] == 'm' ? &m_tokens.materials : &m_tokens.names;
         token.erase(0, 2);
         if (token.empty())
            continue;
      }
      target->push_back(std::move(token));
   }
   m_openMatchesPending = true;
   ++m_filterGeneration;
}

bool OutlinerPanel::MatchesFilter(const string &name) const
{
   if (m_tokens.names.empty())
      return true;
   const string name_lcase = lowerCase(name);
   return std::ranges::all_of(m_tokens.names, [&name_lcase](const string &token) { return name_lcase.find(token) != std::string::npos; });
}

bool OutlinerPanel::MatchesPartCached(size_t index) const
{
   if (m_tokens.empty())
      return true;
   if (!std::ranges::all_of(m_tokens.names, [this, index](const string &token) { return m_lowerNames[index].find(token) != std::string::npos; }))
      return false;
   if (!m_tokens.types.empty())
   {
      const TypeInfo *info = FindTypeInfo(m_editor.m_editables[index]->GetEditable()->GetItemType());
      const char *name = info ? info->name : "Part";
      const char *alias = info ? info->alias : "";
      if (!std::ranges::all_of(m_tokens.types, [name, alias](const string &token) { return StartsWithNoCase(name, token) || StartsWithNoCase(alias, token); }))
         return false;
   }
   return std::ranges::all_of(m_tokens.materials, [this, index](const string &token) { return m_lowerMaterials[index].find(token) != std::string::npos; });
}

void OutlinerPanel::RefreshFilterCache()
{
   const auto &editables = m_editor.m_editables;
   const bool backdropView = m_editor.IsBackdropEditMode();
   const double now = ImGui::GetTime();
   if (editables.size() == m_cacheSize && m_filterGeneration == m_cacheGeneration && backdropView == m_cacheBackdrop && now - m_cacheTime < 1.0)
   {
      // Reordered (drag & drop) or replaced (undo) parts keep the count: compare the content
      bool same = true;
      for (size_t i = 0; i < editables.size() && same; ++i)
         same = editables[i]->GetEditable() == m_cachePtrs[i];
      if (same)
         return;
   }
   m_cacheSize = editables.size();
   m_cacheGeneration = m_filterGeneration;
   m_cacheBackdrop = backdropView;
   m_cacheTime = now;
   m_cachePtrs.resize(editables.size());
   m_lowerNames.resize(editables.size());
   m_lowerMaterials.resize(editables.size());
   for (size_t i = 0; i < editables.size(); ++i)
   {
      const IEditable *const editable = editables[i]->GetEditable();
      m_cachePtrs[i] = editable;
      if (m_tokens.names.empty())
         m_lowerNames[i].clear();
      else
         m_lowerNames[i] = lowerCase(editable->GetName());
      if (m_tokens.materials.empty())
         m_lowerMaterials[i].clear();
      else
         m_lowerMaterials[i] = lowerCase(PropertiesPanel::PartMaterial(editable));
   }
}

nlohmann::json OutlinerPanel::SaveSetup() const { return { { "mode", m_displayMode }, { "types", m_typeMask }, { "sel_only", m_selectedOnly } }; }

void OutlinerPanel::LoadSetup(const nlohmann::json &j)
{
   if (!j.is_object())
      return;
   SetDisplayMode(j.value("mode", 0));
   SetTypeMask(j.value("types", AllTypesMask));
   SetSelectedOnly(j.value("sel_only", false));
}

void OutlinerPanel::RenderPane(const AreaRect &rect, const int leafId)
{
   EditorUI &editor = m_editor;
   if (editor.m_table->IsLocked())
      return;

   char name[32];
   snprintf(name, sizeof(name), "##Pane%d", leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar
      | ImGuiWindowFlags_NoScrollWithMouse; // The tree scrolls in its own child, under the header
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * editor.m_liveUI.GetDPI(), 4.0f * editor.m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin(name, nullptr, window_flags);
   ImGui::PushID(leafId);
   const ImGuiStyle &style = ImGui::GetStyle();

   // Selection made elsewhere (viewport, property pane, search): a synced tree adopts the selection's display
   // mode and reveals it; an unsynced one only scrolls to a selection already listed in its current mode (the
   // mode is never changed under the user's feet)
   if (editor.m_selection != m_lastSeenSelection)
   {
      m_lastSeenSelection = editor.m_selection;
      int kind = -1;
      switch (editor.m_selection.GetType())
      {
      case Selection::S_MATERIAL:
      case Selection::S_IMAGE:
      case Selection::S_SOUND:
      case Selection::S_RENDERPROBE:
      case Selection::S_COLLECTION:
      case Selection::S_CAMERA: kind = 1; break;
      case Selection::S_EDITABLE: kind = 0; break;
      default: break;
      }
      if (kind >= 0 && (m_syncToSelection || kind == m_displayMode))
      {
         m_revealResource = kind == 1;
         if (m_syncToSelection)
            m_displayMode = kind;
      }
   }

   // Width of the right aligned columns (filter button of the header, eye buttons of the tree): the tree's scrollbar space is always
   // reserved, so that they do not shift when the scrollbar appears
   const float columnWidth = ImGui::GetContentRegionAvail().x - style.ScrollbarSize;
   const bool canReparent = editor.m_table->m_liveBaseTable == nullptr;

   // Header, first row: editor type (Blender's editor type switcher), display mode (icon dropdown
   // in the editor-type style), sync to selection, the filter popover and the new-part-group action
   {
      editor.RenderAreaTypeButton(leafId);
      ImGui::SameLine();
      {
         static constexpr const char *modeIcons[] = { ICON_LC_LAYERS, ICON_LC_LIST };
         static constexpr const char *modeNames[] = { "View Layer", "Table Data" };
         const char *const icon = modeIcons[m_displayMode];
         const ImVec4 iconInk = TextInkBounds(icon); // Icon ink exceeds the layout box: size/offset on ink bounds, same as RenderAreaTypeButton
         const ImVec2 iconSize = ImGui::CalcTextSize(icon);
         const float chevronWidth = ImGui::GetFontSize() * 0.4f;
         const ImVec2 buttonSize(style.FramePadding.x * 2.f + iconSize.x + style.ItemInnerSpacing.x + chevronWidth, ImMax(iconSize.y, iconInk.w - iconInk.y) + style.FramePadding.y * 2.f);
         if (ImGui::Button("##OutlinerMode", buttonSize))
            ImGui::OpenPopup("##OutlinerModePopup");
         const ImVec2 itemMin = ImGui::GetItemRectMin(), itemMax = ImGui::GetItemRectMax();
         ImDrawList *const drawList = ImGui::GetWindowDrawList();
         const float centerY = (itemMin.y + itemMax.y) * 0.5f;
         drawList->AddText(ImVec2(itemMin.x + style.FramePadding.x - iconInk.x, itemMin.y + style.FramePadding.y - iconInk.y), EditorTheme::Text, icon);
         const float half = chevronWidth * 0.5f;
         EditorChrome::DrawChevron(drawList, ImVec2(itemMax.x - style.FramePadding.x - half, centerY), EditorChrome::ChevronDir::Down, half, EditorTheme::TextDim);
         ImGui::SetItemTooltip("%s", modeNames[m_displayMode]);
         ImGui::SetNextWindowPos(ImVec2(itemMin.x, itemMax.y), ImGuiCond_Always);
         if (ImGui::BeginPopup("##OutlinerModePopup"))
         {
            for (int mode = 0; mode < 2; ++mode)
               if (ImGui::Selectable((modeIcons[mode] + " "s + modeNames[mode]).c_str(), m_displayMode == mode))
                  m_displayMode = mode;
            ImGui::EndPopup();
         }
      }
      ImGui::SameLine();
      const bool popStyle = m_syncToSelection;
      if (popStyle)
         ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
      if (IconButtonInk(ICON_LC_LINK, "##SyncTree"))
      {
         m_syncToSelection = !m_syncToSelection;
         m_lastSyncedPart = nullptr; // Reveal the current selection on next render
      }
      if (popStyle)
         ImGui::PopStyleColor();
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Sync tree with selection");
      ImGui::SameLine();
      const bool typeFilterActive = m_typeMask != AllTypesMask || m_selectedOnly;
      if (typeFilterActive)
         ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
      if (IconButtonInk(ICON_LC_FILTER, "##OutlinerFilterButton"))
         ImGui::OpenPopup("##OutlinerFilterPopup");
      if (typeFilterActive)
         ImGui::PopStyleColor();
      ImGui::SetItemTooltip("Filters");
      ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y), ImGuiCond_Always, ImVec2(1.f, 0.f));
      if (ImGui::BeginPopup("##OutlinerFilterPopup"))
      {
         bool changed = false;
         if (ImGui::SmallButton("All"))
         {
            m_typeMask = AllTypesMask;
            changed = true;
         }
         ImGui::SameLine();
         if (ImGui::SmallButton("None"))
         {
            m_typeMask = 0;
            changed = true;
         }
         ImGui::Separator();
         for (int cat = 0; cat < CatCount; ++cat)
            changed |= ImGui::CheckboxFlags(CategoryLabels[cat], &m_typeMask, 1u << cat);
         ImGui::Separator();
         changed |= ImGui::Checkbox("Selected only", &m_selectedOnly);
         if (changed)
            m_openMatchesPending = true;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      // Expand/collapse all: collapses every twirl-down node when any is open, expands them all otherwise
      const bool anyOpen = m_openNodeCount > 0;
      if (IconButtonInk(anyOpen ? ICON_LC_CHEVRON_UP : ICON_LC_CHEVRON_DOWN, "##ExpandAll"))
         m_setAllOpen = anyOpen ? 0 : 1;
      ImGui::SetItemTooltip("%s", anyOpen ? "Collapse All" : "Expand All");
      ImGui::SameLine();
      ImGui::BeginDisabled(!canReparent);
      if (IconButtonInk(ICON_LC_FOLDER, "##NewPartGroup"))
         editor.CreatePartGroup();
      ImGui::EndDisabled();
      ImGui::SetItemTooltip("New Part Group");
   }
   // Second row: the search field spanning the full column (clear button inside the field once there is a text)
   {
      ImGui::SetNextItemWidth(columnWidth);
      // The clear button drawn inside the field must be allowed to overlap the input's hover/active claim
      ImGui::SetNextItemAllowOverlap();
      if (m_focusFilter)
      {
         ImGui::SetKeyboardFocusHere();
         m_focusFilter = false;
      }
      ImGui::InputTextWithHint("##OutlinerFilter", "Search", &m_filter);
      ImGui::SetItemTooltip("Name words, t:light, m:<material>, n:<name> (Ctrl+F)");
      if (!m_filter.empty())
      {
         const ImVec2 fieldMin = ImGui::GetItemRectMin();
         const ImVec2 fieldMax = ImGui::GetItemRectMax();
         // Clear button sized on the X glyph's ink bounds (IconButtonInk), centered vertically inside
         // the field's right edge — plain ImGui::Button lets the icon ink overflow past the field
         const ImVec4 xInk = TextInkBounds(ICON_LC_X);
         const ImVec2 xBox = ImGui::CalcTextSize(ICON_LC_X);
         const ImVec2 btn(ImMax(xBox.x, xInk.z - xInk.x) + style.FramePadding.x * 2.f, ImMax(xBox.y, xInk.w - xInk.y) + style.FramePadding.y * 2.f);
         ImGui::SetCursorScreenPos(ImVec2(fieldMax.x - btn.x - style.FramePadding.x * 0.5f, fieldMin.y + (fieldMax.y - fieldMin.y - btn.y) * 0.5f));
         ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
         ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextDim));
         if (IconButtonInk(ICON_LC_X, "##ClearOutlinerFilter"))
         {
            // The active field edits an internal buffer which would be written back over the cleared string
            ImGui::ClearActiveID();
            m_filter.clear();
         }
         ImGui::PopStyleColor(2);
         ImGui::SetItemTooltip("Clear");
      }
   }
   ImGui::Separator();
   ParseFilter();
   RefreshFilterCache();

   // The tree scrolls under the header
   ImGui::BeginChild("##OutlinerTree", ImVec2(0.f, 0.f), ImGuiChildFlags_None);
   m_openNodeCount = 0;

   // Drag & drop of parts and part groups to move them between groups (not supported in inspection
   // mode as the inspected table is a live copy). The drag payload is just a marker: the moved parts
   // are the multi selection (the dragged part is selected first if it was not part of it). The move
   // is applied after the tree iteration since it mutates the ordering of the iterated part list.
   // Part groups sorted by path (see EditorUI::UpdateEditableList) so rows have no user order: no insert-before/after, only into group / to the top level.
   const ImGuiPayload *const dragPayload = ImGui::GetDragDropPayload();
   const bool vpxDragging = canReparent && dragPayload != nullptr && dragPayload->IsDataType("VPX_PART");
   const bool dragBlockedPrev = m_dragBlocked; // Drop refused under the mouse last frame (shown in the drag tooltip)
   m_dragBlocked = false;
   bool hoveredCollapsedGroup = false;
   PartGroup *dropGroup = nullptr;
   bool dropRequested = false;
   ImVec2 scenePartsMin(0.f, 0.f), scenePartsMax(0.f, 0.f); // Row of the root node, highlighted when dropping on the empty space below the tree
   // Number of selected parts that a drop on 'group' (nullptr = top level) would move; blocked is set when a part is refused (cycle, or a non group part to the top level)
   const auto evalDrop = [&editor](const PartGroup *group, bool &blocked, string *firstName) -> int
   {
      int count = 0;
      blocked = false;
      for (const auto &uiPart : editor.m_multiSel)
      {
         const IEditable *const editable = uiPart->GetEditable();
         if (editable == nullptr || editable == group || editable->GetPartGroup() == group)
            continue;
         if (!EditorUI::CanMovePartToGroup(editable, group))
         {
            blocked = true;
            continue;
         }
         if (count == 0 && firstName != nullptr)
            *firstName = editable->GetName();
         ++count;
      }
      return count;
   };
   const auto partDragSource = [&editor, canReparent, dragBlockedPrev](const std::shared_ptr<EditorUIPart> &uiPart)
   {
      if (!canReparent || !ImGui::BeginDragDropSource())
         return;
      if (!editor.IsPartSelected(uiPart))
         editor.SetSelection(Selection(uiPart));
      const IEditable *editable = uiPart->GetEditable();
      ImGui::SetDragDropPayload("VPX_PART", &editable, sizeof(editable));
      const size_t count = editor.m_multiSel.size();
      if (count > 1)
         ImGui::Text("%s  Move %d parts", TypeIcon(editable->GetItemType()), (int)count);
      else
         ImGui::Text("%s  %s", TypeIcon(editable->GetItemType()), editable->GetName().c_str());
      if (dragBlockedPrev)
         ImGui::TextColored(EditorTheme::V(EditorTheme::TextDim), "Not allowed here");
      ImGui::EndDragDropSource();
   };
   // highlight: outline the target row (the last item, or the given rect). Parts rows target their group without highlight.
   const auto partDropTarget = [this, &dropGroup, &dropRequested, &evalDrop, canReparent](PartGroup *group, bool highlight, const ImVec2 *rectMin = nullptr, const ImVec2 *rectMax = nullptr)
   {
      if (!canReparent || !ImGui::BeginDragDropTarget())
         return;
      const ImGuiPayload *const payload = ImGui::AcceptDragDropPayload("VPX_PART", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
      if (payload != nullptr)
      {
         bool blocked = false;
         const int count = evalDrop(group, blocked, nullptr);
         if (count == 0 && blocked)
         {
            m_dragBlocked = true;
            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
         }
         else if (count > 0)
         {
            if (highlight)
            {
               const ImVec2 rMin = rectMin != nullptr ? *rectMin : ImGui::GetItemRectMin();
               const ImVec2 rMax = rectMax != nullptr ? *rectMax : ImGui::GetItemRectMax();
               ImDrawList *const drawList = ImGui::GetWindowDrawList();
               drawList->AddRectFilled(rMin, rMax, (EditorTheme::Fill & ~IM_COL32_A_MASK) | (48u << IM_COL32_A_SHIFT), 2.f);
               drawList->AddRect(rMin, rMax, EditorTheme::Fill, 2.f, 0, 2.f);
            }
            if (payload->IsDelivery())
            {
               dropGroup = group;
               dropRequested = true;
            }
         }
      }
      ImGui::EndDragDropTarget();
   };
   // Part row: type icon, selectable with ctrl/shift multi selection (double click frames the part), drag source, drop target and eye toggle
   std::shared_ptr<EditorUIPart> rangeTarget; // Shift+click: applied once all the rows are known
   const auto renderPartLeaf = [this, &editor, &partDragSource, &partDropTarget, &rangeTarget](
                                  const std::shared_ptr<EditorUIPart> &edit, int &itemCounter, const std::shared_ptr<EditorUIPart> &revealPart, float eyeX, float indentX)
   {
      IEditable *const editable = edit->GetEditable();
      m_listedParts.push_back(edit);
      const bool inactiveSel = editor.IsPartSelected(edit) && edit != editor.m_selection.GetPart();
      PushRowColors(editor.IsPartSelected(edit), inactiveSel);
      const bool clicked
         = ImGui::Selectable(("##Outliner"s + std::to_string(itemCounter++)).c_str(), editor.IsPartSelected(edit), ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick);
      ImGui::PopStyleColor(3);
      {
         // The highlight spans the whole row: the part left of the selectable (the tree indentation) gets the same color
         const bool rowSelected = editor.IsPartSelected(edit);
         const bool rowHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlappedByItem);
         if (rowSelected || rowHovered)
         {
            ImVec4 color = EditorTheme::V(rowSelected ? (rowHovered ? EditorTheme::RowSelectedHover : EditorTheme::RowSelected) : EditorTheme::Hover);
            if (inactiveSel)
               color.w *= 0.4f;
            const float rowMinX = ImGui::GetCurrentWindow()->WorkRect.Min.x;
            const ImVec2 itemMin = ImGui::GetItemRectMin();
            if (itemMin.x > rowMinX)
               ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(rowMinX, itemMin.y), ImVec2(itemMin.x, ImGui::GetItemRectMax().y), ImGui::GetColorU32(color));
         }
      }
      {
         // Type icon (dim) and name (normal text, faint when the part is hidden) drawn over the selectable, indented under the parent group's name
         const char *const icon = TypeIcon(editable->GetItemType());
         const ImVec2 textPos = ImVec2(ImGui::GetItemRectMin().x + indentX, ImGui::GetItemRectMin().y);
         ImDrawList *const drawList = ImGui::GetWindowDrawList();
         // Lucide glyph ink sits higher than the text line box: center the ink vertically rather than aligning on the text origin
         const ImVec4 iconInk = TextInkBounds(icon);
         drawList->AddText(ImVec2(textPos.x - iconInk.x, textPos.y + (ImGui::GetTextLineHeight() - (iconInk.w - iconInk.y)) * 0.5f - iconInk.y), EditorTheme::TextDim, icon);
         const float nameX = textPos.x + ImGui::CalcTextSize(icon).x + 6.f;
         const float eyeScreenX = ImGui::GetWindowPos().x - ImGui::GetScrollX() + eyeX; // Where the eye button is placed by SameLine(eyeX)
         const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlappedByItem);
         const string shown = EllipsizeToWidth(editable->GetName(), eyeScreenX - ImGui::GetStyle().ItemSpacing.x - nameX);
         drawList->AddText(ImVec2(nameX, textPos.y), editable->IsUIVisible(false) ? EditorTheme::Text : (hovered ? EditorTheme::TextDim : EditorTheme::TextFaint), shown.c_str());
         if (hovered && shown != editable->GetName() && ImGui::GetDragDropPayload() == nullptr)
            ImGui::SetTooltip("%s", editable->GetName().c_str()); // Full name of an ellipsized row
      }
      if (clicked)
      {
         const ImGuiIO &selIO = ImGui::GetIO();
         if (selIO.KeyCtrl)
         {
            editor.TogglePartSelection(edit);
            m_anchor = edit;
         }
         else if (selIO.KeyShift)
            rangeTarget = edit;
         else
         {
            editor.SetSelection(Selection(edit));
            m_anchor = edit;
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
               if (ViewportPane *const vp = editor.GetTargetViewport(); vp != nullptr)
                  vp->FrameSelection(); // Blender's View Selected
         }
      }
      partDragSource(edit);
      partDropTarget(editable->GetPartGroup(), false); // Dropping on a part moves to its group (same as the Win32 layer dialog)
      if (edit == revealPart && !ImGui::IsItemVisible())
         ImGui::SetScrollHereY();
      if (editor.m_table->m_liveBaseTable == nullptr)
      {
         ImGui::SameLine(eyeX);
         const char *const eyeIcon = editable->IsUIVisible(false) ? ICON_LC_EYE : ICON_LC_EYE_OFF;
         // The small button is shorter than the row: center it vertically (SameLine restored the row height in CurrLineSize)
         const ImVec4 eyeInk = TextInkBounds(eyeIcon);
         const float eyeOffsetY = (ImGui::GetCurrentWindow()->DC.CurrLineSize.y - ImMax(ImGui::GetTextLineHeight(), eyeInk.w - eyeInk.y)) * 0.5f;
         if (eyeOffsetY > 0.f)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + eyeOffsetY);
         ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(editable->IsUIVisible(false) ? EditorTheme::TextDim : EditorTheme::TextFaint));
         ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
         ImGui::PushID(editable);
         if (SmallIconButtonInk(eyeIcon, "##Eye"))
         {
            editable->SetUIVisible(!editable->IsUIVisible(false));
            editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         }
         ImGui::PopID();
         ImGui::PopStyleColor(2);
      }
   };

   if (m_displayMode == 0)
   {
      // View Layer: the part groups (Blender's collections) tree
      const bool filterActive = !m_tokens.empty() || m_typeMask != AllTypesMask || m_selectedOnly;
      const bool openMatches = m_openMatchesPending && filterActive;
      m_openMatchesPending = false;
      m_listedParts.clear();
      ImGui::PushID("parts");
      // The root node is not part of expand/collapse all: neither forced nor counted
      const int savedSetAllOpen = m_setAllOpen, savedOpenNodeCount = m_openNodeCount;
      m_setAllOpen = -1;
      const bool scenePartsOpened = ChevronTreeNode("Scene Parts", ImGuiTreeNodeFlags_DefaultOpen);
      m_openNodeCount = savedOpenNodeCount;
      m_setAllOpen = savedSetAllOpen;
      scenePartsMin = ImGui::GetItemRectMin();
      scenePartsMax = ImGui::GetItemRectMax();
      partDropTarget(nullptr, true); // Dropping on the scene root ungroups part groups
      if (scenePartsOpened)
      {
         // Table definition parts
         struct Node
         {
            PartGroup *group;
            bool opened;
         };
         // One pass computing what is listed: parts must match the type mask, the selected only option and the search tokens (or be in a group
         // matching them). With any filter, groups without listed part are hidden. Also counts the listed parts of each group (current view mode only)
         const bool backdropView = editor.IsBackdropEditMode();
         const bool hasTokens = !m_tokens.empty();
         m_matchedGroups.clear();
         m_visibleGroups.clear();
         m_groupCounts.clear();
         m_groupStack.clear();
         m_showPart.assign(editor.m_editables.size(), 0);
         for (size_t i = 0; i < editor.m_editables.size(); ++i)
         {
            const std::shared_ptr<EditorUIPart> &uiPart = editor.m_editables[i];
            const IEditable *const editable = uiPart->GetEditable();
            while (!m_groupStack.empty() && (editable->GetPartGroup() == nullptr || !editable->IsChild(m_groupStack.back())))
               m_groupStack.pop_back();
            if (editable->GetItemType() == eItemPartGroup)
            {
               if (hasTokens && MatchesPartCached(i))
               {
                  m_matchedGroups.insert(static_cast<const PartGroup *>(editable));
                  for (const PartGroup *ancestor : m_groupStack)
                     m_visibleGroups.insert(ancestor);
                  m_visibleGroups.insert(static_cast<const PartGroup *>(editable));
               }
               m_groupStack.push_back(static_cast<const PartGroup *>(editable));
               continue;
            }
            const bool listed = editable->m_desktopBackdrop == backdropView && (m_typeMask & (1u << CategoryOf(editable->GetItemType()))) != 0
               && (!m_selectedOnly || editor.IsPartSelected(uiPart))
               && (!hasTokens || MatchesPartCached(i) || std::ranges::any_of(m_groupStack, [this](const PartGroup *group) { return m_matchedGroups.contains(group); }));
            m_showPart[i] = listed;
            if (!listed)
               continue;
            for (const PartGroup *ancestor : m_groupStack)
            {
               m_visibleGroups.insert(ancestor);
               ++m_groupCounts[ancestor];
            }
         }
         vector<Node> stack;
         int outlinerItem = 0;
         const float eyeX = columnWidth - IconButtonInkSize(ICON_LC_EYE).x; // Right edge of the eye buttons = right edge of the filter button
         // Highlight the group in which new and imported parts are added
         PartGroup *const newPartTarget = editor.GetPartGroupForNewPart();
         // When 'sync tree with selection' is enabled, reveal the newly selected part in the tree
         const std::shared_ptr<EditorUIPart> revealPart = (m_syncToSelection && editor.m_selection.GetPart() != m_lastSyncedPart) ? editor.m_selection.GetPart() : nullptr;
         m_lastSyncedPart = editor.m_selection.GetPart();
         for (size_t idx = 0; idx < editor.m_editables.size(); ++idx)
         {
            const auto &edit = editor.m_editables[idx];
            const PartGroup *parent = edit->GetEditable()->GetPartGroup();
            while (!stack.empty()
               && ((parent == nullptr && (edit->GetEditable()->GetItemType() == eItemPartGroup)) // Root partgroup: pop all
                  || (parent == nullptr && (edit->GetEditable()->GetItemType() != eItemPartGroup) && (stack.back().group != nullptr)) // Live object: pop all unless in 'Live Object' group
                  || (parent != nullptr && (!edit->GetEditable()->IsChild(stack.back().group))))) // Child object: pop up to the parent group
            {
               if (stack.back().opened)
                  ImGui::TreePop();
               stack.pop_back();
            }
            const bool isGroup = edit->GetEditable()->GetItemType() == eItemPartGroup;
            // With a filter, only display groups containing a listed part
            if (isGroup && filterActive && !m_visibleGroups.contains(static_cast<PartGroup *>(edit->GetEditable())))
               continue;
            // In desktop backdrop mode, only display backdrop parts, and conversely in the other view modes
            if (!isGroup && (edit->GetEditable()->m_desktopBackdrop != backdropView))
               continue;
            ImGui::AlignTextToFramePadding();
            if (isGroup)
            {
               PartGroup *group = static_cast<PartGroup *>(edit->GetEditable());
               bool opened = false;
               if (stack.empty() || stack.back().opened)
               {
                  if ((revealPart && revealPart->GetEditable()->IsChild(group)) || (openMatches && m_visibleGroups.contains(group)))
                     ImGui::SetNextItemOpen(true);
                  const bool isNewPartTarget = (group == newPartTarget);
                  const bool inactiveSel = editor.IsPartSelected(edit) && edit != editor.m_selection.GetPart();
                  PushRowColors(editor.IsPartSelected(edit), inactiveSel);
                  // Folder icon and count are outside the ID (after ###) so that the node keeps its open state when they change
                  const auto countIt = m_groupCounts.find(group);
                  const string &groupName = edit->GetEditable()->GetName();
                  const string idPart = "###" + groupName;
                  const bool wasOpen = ImGui::GetStateStorage()->GetInt(ImGui::GetID(idPart.c_str()), 0) != 0;
                  // Name shortened so that it ends before the count and the eye button
                  const string countText = std::to_string(countIt != m_groupCounts.end() ? countIt->second : 0);
                  const char *const icon = wasOpen ? ICON_LC_FOLDER_OPEN : ICON_LC_FOLDER;
                  const string iconPart = icon + "  "s;
                  const float labelStartX = ImGui::GetCursorScreenPos().x + ImGui::GetTreeNodeToLabelSpacing();
                  const float labelEndX = ImGui::GetWindowPos().x - ImGui::GetScrollX() + eyeX - style.ItemSpacing.x * 2.f - ImGui::CalcTextSize(countText.c_str()).x;
                  const string groupLabel = iconPart + EllipsizeToWidth(groupName, labelEndX - labelStartX - ImGui::CalcTextSize(iconPart.c_str()).x) + idPart;
                  const bool groupHidden = !group->IsUIVisible(false);
                  if (groupHidden)
                     ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextFaint));
                  opened = ChevronTreeNode(groupLabel.c_str(),
                     ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_SpanFullWidth | (editor.IsPartSelected(edit) ? ImGuiTreeNodeFlags_Selected : 0), icon, isNewPartTarget);
                  if (groupHidden)
                     ImGui::PopStyleColor();
                  ImGui::PopStyleColor(3);
                  const bool rowHovered = ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                  if (ImGui::IsItemHovered() && ImGui::GetDragDropPayload() == nullptr && groupLabel.substr(0, groupLabel.find("###")).find(groupName) == string::npos)
                     ImGui::SetTooltip("%s", groupName.c_str()); // Full name of an ellipsized row
                  partDragSource(edit);
                  partDropTarget(group, true);
                  if (vpxDragging && !opened && rowHovered)
                  {
                     // Blender opens a collapsed collection hovered while dragging
                     hoveredCollapsedGroup = true;
                     const ImGuiID groupId = ImGui::GetItemID();
                     if (m_dragHoverGroup != groupId)
                     {
                        m_dragHoverGroup = groupId;
                        m_dragHoverSince = ImGui::GetTime();
                     }
                     else if (ImGui::GetTime() - m_dragHoverSince > 0.7)
                     {
                        ImGui::GetStateStorage()->SetInt(groupId, 1);
                        m_dragHoverGroup = 0;
                     }
                  }
                  if (ImGui::BeginPopupContextItem())
                  {
                     if (ImGui::MenuItem("Select"))
                        editor.SetSelection(Selection(edit));
                     if (ImGui::MenuItem("Select Contents"))
                        editor.SelectPartsInGroup(group);
                     ImGui::Separator();
                     if (ImGui::MenuItem("Add Reference Image...", nullptr, false, editor.m_referenceImageOp.CanAdd()))
                        editor.m_referenceImageOp.OpenAddDialog();
                     ImGui::EndPopup();
                  }
                  {
                     // Part count (dim, right aligned before the eye column); the disc behind the icon marks the group new and imported parts are added to
                     ImGui::SameLine(eyeX - style.ItemSpacing.x - ImGui::CalcTextSize(countText.c_str()).x);
                     ImGui::TextColored(EditorTheme::V(EditorTheme::TextDim), "%s", countText.c_str());
                     if (isNewPartTarget && rowHovered && ImGui::GetIO().MousePos.x >= labelStartX && ImGui::GetIO().MousePos.x <= labelStartX + TextInkBounds(icon).z)
                        ImGui::SetTooltip("New and imported parts are added to this group");
                  }
                  if (editor.m_table->m_liveBaseTable == nullptr)
                  {
                     ImGui::SameLine(eyeX);
                     const char *const eyeIcon = group->IsUIVisible(false) ? ICON_LC_EYE : ICON_LC_EYE_OFF;
                     // The small button is shorter than the row: center it vertically (SameLine restored the row height in CurrLineSize)
                     const ImVec4 eyeInk = TextInkBounds(eyeIcon);
                     const float eyeOffsetY = (ImGui::GetCurrentWindow()->DC.CurrLineSize.y - ImMax(ImGui::GetTextLineHeight(), eyeInk.w - eyeInk.y)) * 0.5f;
                     if (eyeOffsetY > 0.f)
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + eyeOffsetY);
                     ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(group->IsUIVisible(false) ? EditorTheme::TextDim : EditorTheme::TextFaint));
                     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
                     ImGui::PushID(group);
                     if (SmallIconButtonInk(eyeIcon, "##Eye"))
                     {
                        group->SetUIVisible(!group->IsUIVisible(false));
                        editor.m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
                     }
                     ImGui::PopID();
                     ImGui::PopStyleColor(2);
                  }
                  if (edit == revealPart && !ImGui::IsItemVisible())
                     ImGui::SetScrollHereY();
               }
               stack.emplace_back(group, opened);
            }
            else
            {
               const bool show = m_showPart[idx] != 0;
               if (parent == nullptr && stack.empty() && show)
               {
                  if (revealPart && revealPart->GetEditable()->GetPartGroup() == nullptr)
                     ImGui::SetNextItemOpen(true);
                  stack.push_back({ nullptr, ChevronTreeNode("[Live Objects]", ImGuiTreeNodeFlags_AllowOverlap) });
                  partDropTarget(nullptr, false);
               }
               if (!stack.empty() && stack.back().opened && show)
                  renderPartLeaf(edit, outlinerItem, revealPart, eyeX,
                     stack.back().group != nullptr ? ImGui::GetTreeNodeToLabelSpacing() - style.IndentSpacing + ImGui::CalcTextSize(ICON_LC_FOLDER_OPEN "  ").x : 0.f);
            }
         }
         while (!stack.empty())
         {
            if (stack.back().opened)
               ImGui::TreePop();
            stack.pop_back();
         }
         ImGui::TreePop();
      }
      ImGui::PopID();
      if (!hoveredCollapsedGroup)
         m_dragHoverGroup = 0;
      if (vpxDragging)
      {
         // Empty space below the tree: drop target of the top level
         const float freeHeight = ImGui::GetContentRegionAvail().y - style.ItemSpacing.y - 2.f;
         if (freeHeight > 4.f)
         {
            ImGui::InvisibleButton("##OutlinerDropRoot", ImVec2(-FLT_MIN, freeHeight));
            partDropTarget(nullptr, true, &scenePartsMin, &scenePartsMax);
         }
         // Scroll the tree when the mouse is near its top or bottom edge
         const ImVec2 winPos = ImGui::GetWindowPos();
         const ImVec2 winSize = ImGui::GetWindowSize();
         const ImVec2 mouse = ImGui::GetIO().MousePos;
         const float edge = ImGui::GetFontSize() * 2.f;
         if (mouse.x >= winPos.x && mouse.x <= winPos.x + winSize.x)
         {
            float speed = 0.f; // -1..1
            if (mouse.y >= winPos.y - edge && mouse.y < winPos.y + edge)
               speed = -ImClamp(1.f - (mouse.y - winPos.y) / edge, 0.f, 1.f);
            else if (mouse.y > winPos.y + winSize.y - edge && mouse.y <= winPos.y + winSize.y + edge)
               speed = ImClamp(1.f - (winPos.y + winSize.y - mouse.y) / edge, 0.f, 1.f);
            if (speed != 0.f)
               ImGui::SetScrollY(ImGui::GetScrollY() + speed * 600.f * ImGui::GetIO().DeltaTime);
         }
      }
      if (rangeTarget)
      {
         // Range between the anchor and the clicked row, restricted to the rows listed (type filter, search, open groups)
         const auto anchorIt = std::ranges::find(m_listedParts, m_anchor);
         const auto targetIt = std::ranges::find(m_listedParts, rangeTarget);
         if (m_anchor == nullptr || anchorIt == m_listedParts.end() || targetIt == m_listedParts.end())
            editor.SetSelection(Selection(rangeTarget));
         else
         {
            editor.m_multiSel.assign(std::min(anchorIt, targetIt), std::max(anchorIt, targetIt) + 1);
            editor.m_selection = Selection(rangeTarget);
         }
      }
   }
   else
   {
      // Table Data: the table resources as twirl-down categories (Blender's 'Blender File' mode)
      ImGui::PushID("data");
      // Category label with a section icon and the item count, the count is outside the ID
      char label[96];
      const auto categoryLabel = [&label](const char *icon, const char *name, size_t count) -> const char *
      {
         snprintf(label, sizeof(label), "%s %s (%d)###%s", icon, name, (int)count, name);
         return label;
      };
      const std::function<string(Material *)> materialName = [](Material *material) -> string { return material->m_name; };
      const Selection::SelectionType selType = editor.m_selection.GetType();
      if (m_revealResource && selType == Selection::S_MATERIAL)
         ImGui::SetNextItemOpen(true);
      RenderResourceList(categoryLabel(ICON_LC_DROPLET, "Materials", editor.m_table->m_materials.size()), ICON_LC_DROPLET, editor.m_table->m_materials, editor.m_multiSelMaterials,
         editor.m_outlinerMaterialAnchor, materialName);
      const std::function<string(Texture *)> imageName = [](Texture *image) -> string { return image->m_name; };
      if (m_revealResource && selType == Selection::S_IMAGE)
         ImGui::SetNextItemOpen(true);
      RenderResourceList(
         categoryLabel(ICON_LC_IMAGE, "Images", editor.m_table->m_vimage.size()), ICON_LC_IMAGE, editor.m_table->m_vimage, editor.m_multiSelImages, editor.m_outlinerImageAnchor, imageName);
      const std::function<string(VPX::Sound *)> soundName = [](VPX::Sound *sound) -> string { return sound->GetName(); };
      if (m_revealResource && selType == Selection::S_SOUND)
         ImGui::SetNextItemOpen(true);
      RenderResourceList(
         categoryLabel(ICON_LC_DISC, "Sounds", editor.m_table->m_vsound.size()), ICON_LC_DISC, editor.m_table->m_vsound, editor.m_multiSelSounds, editor.m_outlinerSoundAnchor, soundName);
      if (m_revealResource && selType == Selection::S_RENDERPROBE)
         ImGui::SetNextItemOpen(true);
      if (ChevronTreeNode(categoryLabel(ICON_LC_LENS, "Render Probes", editor.m_table->m_vrenderprobe.size())))
      {
         int pos = 0;
         for (RenderProbe *probe : editor.m_table->m_vrenderprobe)
         {
            if (!MatchesFilter(probe->GetName()))
               continue;
            ImGui::PushID(pos++);
            Selection sel(probe);
            if (ImGui::Selectable((ICON_LC_LENS " "s + probe->GetName()).c_str(), editor.m_selection == sel))
               editor.SetSelection(sel);
            if (m_revealResource && editor.m_selection == sel && !ImGui::IsItemVisible())
               ImGui::SetScrollHereY();
            ImGui::PopID();
         }
         ImGui::TreePop();
      }
      // Collections can only be edited on the base table (the inspected table is a live copy)
      const bool canEditCollections = editor.m_table->m_liveBaseTable == nullptr;
      if (m_revealResource && selType == Selection::S_COLLECTION)
         ImGui::SetNextItemOpen(true);
      const bool collectionsOpened = ChevronTreeNode(categoryLabel(ICON_LC_LAYERS, "Collections", editor.m_table->GetCollections().size()));
      if (ImGui::BeginPopupContextItem())
      {
         ImGui::BeginDisabled(!canEditCollections);
         if (ImGui::MenuItem("New Collection"))
            editor.CreateCollection(false);
         const bool hasScriptableSel = std::ranges::any_of(
            editor.m_multiSel, [](const std::shared_ptr<EditorUIPart> &part) { return part->GetEditable() != nullptr && part->GetEditable()->GetIScriptable() != nullptr; });
         ImGui::BeginDisabled(!hasScriptableSel);
         if (ImGui::MenuItem("New Collection From Selection"))
            editor.CreateCollection(true);
         ImGui::EndDisabled();
         ImGui::EndDisabled();
         ImGui::EndPopup();
      }
      if (collectionsOpened)
      {
         // Collections are listed in their storage order as it is user defined and relevant to scripts
         for (Collection *const collection : editor.m_table->GetCollections())
         {
            if (!MatchesFilter(collection->m_name))
               continue;
            const Selection sel(collection);
            if (ImGui::Selectable((ICON_LC_LAYERS " "s + collection->m_name + "##collection"s).c_str(), editor.m_selection == sel))
               editor.SetSelection(sel);
            if (m_revealResource && editor.m_selection == sel && !ImGui::IsItemVisible())
               ImGui::SetScrollHereY();
         }
         ImGui::TreePop();
      }
      if (m_revealResource && selType == Selection::S_CAMERA)
         ImGui::SetNextItemOpen(true);
      if (ChevronTreeNode(ICON_LC_VIDEO " View Setups"))
      {
         // Selecting a camera setup only marks the selection: the preview panes adopt the mode (the
         // primary view is never switched)
         const auto setPreviewMode = [&editor](const ViewSetupID mode)
         {
            for (const auto &[id, view] : editor.m_areaViews)
               if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->m_viewType == ViewMode::PreviewCam)
                  vp->m_viewMode = mode;
         };
         if (ImGui::Selectable(ICON_LC_VIDEO " Editor Camera", editor.m_selection.GetType() == Selection::S_NONE))
         {
            editor.ClearSelection();
            setPreviewMode(BG_INVALID); // Editor camera: preview panes follow the table's mode
         }
         static constexpr const char *previewNames[] = { ICON_LC_VIDEO " Preview: Desktop", ICON_LC_VIDEO " Preview: Cabinet", ICON_LC_VIDEO " Preview: Full Single Screen" };
         for (int cam = 0; cam < 3; ++cam)
            if (ImGui::Selectable(previewNames[cam], editor.m_selection == Selection::Camera(cam)))
            {
               editor.SetSelection(Selection::Camera(cam));
               setPreviewMode(cam == 0 ? BG_DESKTOP : cam == 1 ? BG_FULLSCREEN : BG_FSS);
            }
         ImGui::TreePop();
      }
      m_revealResource = false;
      ImGui::PopID();
   }
   ImGui::EndChild();
   if (dropRequested)
   {
      // Applied after the tree iteration as the move reorders the iterated part list. One undo record (made by MoveSelectionToPartGroup), relabelled here.
      bool blocked = false;
      string firstName;
      const int count = evalDrop(dropGroup, blocked, &firstName);
      if (count > 0)
      {
         const string what = count == 1 ? firstName : std::to_string(count) + " parts";
         const string target = dropGroup != nullptr ? dropGroup->GetName() : "top level"s;
         // A move waiting for the confirmation of lossy space conversions is labelled and reported by
         // LayerSpaceChangeOp when (and if) it is applied
         if (editor.MoveSelectionToPartGroup(dropGroup))
         {
            editor.m_undo.SetLabel("Move " + what + " to " + target);
            editor.Report("Moved " + what + " to " + target);
         }
      }
   }

   ImGui::PopID();
   m_setAllOpen = -1;
   ImGui::End();
   ImGui::PopStyleVar(3);
}

template <class T>
void OutlinerPanel::RenderResourceList(const char *label, const char *icon, vector<T *> &items, vector<T *> &multiSel, T *&anchor, const std::function<string(T *)> &nameOf)
{
   if (!ChevronTreeNode(label))
      return;
   EditorUI &editor = m_editor;
   const std::vector<T *> sorted = SortedCaseInsensitive(items, nameOf);
   int pos = 0;
   for (T *const item : sorted)
   {
      if (!MatchesFilter(nameOf(item)))
         continue;
      ImGui::PushID(pos++);
      const bool selected = std::ranges::find(multiSel, item) != multiSel.end();
      const bool inactiveSel = selected && editor.m_selection != Selection(item);
      if (inactiveSel)
         PushInactiveSelectionColor();
      const bool clicked = ImGui::Selectable((icon + " "s + nameOf(item)).c_str(), selected);
      if (inactiveSel)
         ImGui::PopStyleColor(3);
      if (m_revealResource && editor.m_selection == Selection(item) && !ImGui::IsItemVisible())
         ImGui::SetScrollHereY();
      ImGui::PopID();
      if (!clicked)
         continue;
      const ImGuiIO &selIO = ImGui::GetIO();
      if (selIO.KeyCtrl)
      {
         const auto it = std::ranges::find(multiSel, item);
         if (it == multiSel.end())
         {
            multiSel.push_back(item);
            editor.m_selection = Selection(item);
         }
         else
         {
            const bool wasActive = editor.m_selection == Selection(item);
            multiSel.erase(it);
            if (multiSel.empty())
               editor.m_selection = Selection();
            else if (wasActive)
               editor.m_selection = Selection(multiSel.back());
         }
         anchor = item;
      }
      else if (selIO.KeyShift && anchor != nullptr)
      {
         // Range selection between the anchor and the clicked item, restricted to the visible items
         vector<T *> visible;
         for (T *const v : sorted)
            if (MatchesFilter(nameOf(v)))
               visible.push_back(v);
         const auto anchorIt = std::ranges::find(visible, anchor);
         const auto itemIt = std::ranges::find(visible, item);
         if (anchorIt == visible.end() || itemIt == visible.end())
         {
            editor.SetSelection(Selection(item));
            anchor = item;
         }
         else
         {
            multiSel.clear();
            for (auto it = std::min(anchorIt, itemIt); it != std::max(anchorIt, itemIt) + 1; ++it)
               multiSel.push_back(*it);
            editor.m_selection = Selection(item);
         }
      }
      else
      {
         editor.SetSelection(Selection(item));
         anchor = item;
      }
   }
   ImGui::TreePop();
}

void OutlinerPanel::AppendStatusHints(vector<StatusHint> &hints) const
{
   const string mod = StatusModKey;
   if (ImGui::GetDragDropPayload() != nullptr)
      hints = { { "Drop", "Move to Group" } };
   else
      hints = { { ICON_LC_MOUSE_LEFT, "Select" }, { mod + "+" ICON_LC_MOUSE_LEFT, "Toggle" }, { "Shift+" ICON_LC_MOUSE_LEFT, "Range" }, { ICON_LC_MOUSE_RIGHT, "Menu" },
         { "Drag", "Move to Group" }, { mod + "+F", "Search" } };
}
}
