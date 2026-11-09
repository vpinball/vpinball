// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

#include "EditorAreaView.h"
#include "Selection.h"
#include "unordered_dense.h"

class PartGroup;
class IEditable;

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Panel modelled on Blender's Outliner: a display mode (scene parts tree or table data), a
// search box and a filter popover (object types, selected only), used to select the edited
// object. Hosted by a leaf of the area layout (several instances may coexist, each with its
// own display mode and filters, persisted in the leaf's payload).
class OutlinerPanel : public EditorAreaView
{
public:
   explicit OutlinerPanel(EditorUI &editor)
      : EditorAreaView(editor)
   {
   }

   AreaType GetType() const override { return AreaType::Outliner; }
   void RenderPane(const AreaRect &rect, int leafId) override;
   void AppendStatusHints(vector<StatusHint> &hints) const override;
   nlohmann::json SaveSetup() const override;
   void LoadSetup(const nlohmann::json &j) override;

   void FocusFilter() { m_focusFilter = true; } // Ctrl+F: type a filter right away

   // Default width of a newly hosted panel in logical pixels (scaled by the UI DPI)
   static constexpr float PaneWidth = 240.f;

   // Anchor part for shift+click range selection (the last part clicked, here or in the 3D view):
   // shared by all outliner instances since the selection it ranges over is shared
   static std::shared_ptr<EditorUIPart> m_anchor;
   static std::shared_ptr<EditorUIPart> m_savedAnchor; // Anchor saved across live-table switches

   // Display mode and filter popover state, saved in the leaf's payload (SaveSetup/LoadSetup).
   // The type mask has one bit per object category (all set: every type is listed), display
   // mode 0 is the scene parts tree, 1 the table data.
   int GetDisplayMode() const { return m_displayMode; }
   void SetDisplayMode(int mode) { m_displayMode = mode == 1 ? 1 : 0; }
   uint32_t GetTypeMask() const { return m_typeMask; }
   void SetTypeMask(uint32_t mask)
   {
      m_typeMask = mask & AllTypesMask;
      m_openMatchesPending = true;
   }
   bool GetSelectedOnly() const { return m_selectedOnly; }
   void SetSelectedOnly(bool selectedOnly)
   {
      m_selectedOnly = selectedOnly;
      m_openMatchesPending = true;
   }
   static constexpr uint32_t AllTypesMask = (1u << 12) - 1u;

private:
   // Filter text split in lowercase tokens: 'n:x' or bare words (name contains), 't:x' (type starts with), 'm:x' (material contains)
   struct FilterTokens
   {
      vector<string> names;
      vector<string> types;
      vector<string> materials;
      bool empty() const { return names.empty() && types.empty() && materials.empty(); }
   };
   // Tree node with a line chevron instead of ImGui's filled triangle; while m_setAllOpen >= 0 every node is forced open/closed (expand/collapse all)
   // icon: the label's leading icon glyph, drawn ink-centered on the row (its glyph ink overshoots the line box); iconBadge: a grey chip behind it
   bool ChevronTreeNode(const char *label, ImGuiTreeNodeFlags flags = 0, const char *icon = nullptr, bool iconBadge = false);
   void ParseFilter();
   bool MatchesFilter(const string &name) const; // Name tokens only (non part lists)
   bool MatchesPartCached(size_t index) const; // All tokens, for m_editables[index] (needs RefreshFilterCache)
   void RefreshFilterCache();

   // Render a resource list (images, sounds, materials) with ctrl/shift multi selection.
   // multiSel holds all the selected items, anchor is the last clicked one for range selection.
   // When asTree is set, the list is wrapped in a twirl-down node labelled label.
   template <class T> void RenderResourceList(const char *label, const char *icon, vector<T *> &items, vector<T *> &multiSel, T *&anchor, const std::function<string(T *)> &nameOf);

   string m_filter;
   bool m_focusFilter = false;
   bool m_syncToSelection = false; // When enabled, the tree is expanded & scrolled to reveal the selected part on selection change
   std::shared_ptr<EditorUIPart> m_lastSyncedPart; // Last selection revealed in the tree while syncing

   string m_parsedFilter; // Filter text the tokens were parsed from
   FilterTokens m_tokens;
   bool m_openMatchesPending = false; // Filter changed: open the groups containing matches once the tree is drawn

   Selection m_lastSeenSelection; // To detect selection changes made elsewhere
   bool m_revealResource = false; // The selection changed to a table data item: open its category and scroll to it
   bool m_dragBlocked = false; // The drop under the mouse is refused (cycle), set while drawing the drop targets
   ImGuiID m_dragHoverGroup = 0; // Collapsed group hovered while dragging parts (opens after a short delay)
   double m_dragHoverSince = 0.0;
   vector<std::shared_ptr<EditorUIPart>> m_listedParts; // Rows drawn in the parts tree this frame, in order (for shift+click range selection)
   int m_setAllOpen = -1; // >= 0 for one frame: force every twirl-down node open (1) or closed (0) — the expand/collapse all button
   int m_openNodeCount = 0; // Twirl-down nodes drawn open this frame (decides the collapse/expand all button's action)

   uint32_t m_typeMask = AllTypesMask;
   bool m_selectedOnly = false;
   int m_displayMode = 0;

   // Per frame scratch data of the parts tree (members to avoid reallocating)
   ankerl::unordered_dense::set<const PartGroup *> m_matchedGroups;
   ankerl::unordered_dense::set<const PartGroup *> m_visibleGroups;
   ankerl::unordered_dense::map<const PartGroup *, int> m_groupCounts;
   vector<const PartGroup *> m_groupStack;
   vector<uint8_t> m_showPart; // Per m_editables entry: part is listed (matches the filter or is in a matching group)

   // Lowercase names/materials of m_editables (only filled when a matching token exists), refreshed when the parts,
   // filter or view mode change, or every second to catch renames
   vector<const IEditable *> m_cachePtrs; // Editable each cached index was built from, a mismatch means the cache is stale
   vector<string> m_lowerNames;
   vector<string> m_lowerMaterials;
   size_t m_cacheSize = 0;
   int m_filterGeneration = 0;
   int m_cacheGeneration = -1;
   bool m_cacheBackdrop = false;
   double m_cacheTime = -10.0;
};

}
