// license:GPLv3+

#pragma once

// Blender-style sidebar (N panel) of a 3D viewport pane: a floating column of vertical-text
// tab chips on the right edge of the pane's image (Item / Tool / View) plus a tool-shelf style
// overlay that opens next to them with the selected tab's controls. One instance per
// ViewportPane — the pane owns its open state, active tab and width, persisted in the leaf's
// layout data (see ViewportPane::SaveSetup/LoadSetup).

#include <nlohmann/json.hpp>

namespace VPX::EditorUI
{

class EditorUI;
class ViewportPane;

class ViewportSidebar final
{
public:
   ViewportSidebar(EditorUI& editor, ViewportPane& pane)
      : m_editor(editor)
      , m_pane(pane)
   {
   }

   // Draws the tab chips and the open shelf at the right edge of the pane's image (called once
   // per frame from ViewportPane::RenderPane while the leaf is shown)
   void Render();

   bool IsShown() const { return m_shown; }
   void Toggle() { m_shown = !m_shown; }
   // Mouse over the panel this frame: the pane keeps the keyboard (N and friends still target it)
   bool IsHovered() const { return m_hovered; }
   float DrawnWidth() const { return m_drawnWidth; } // Display px, 0 while hidden (the nav gizmo shifts left of it)
   float Width() const { return m_width; } // Logical px

   void CopyFrom(const ViewportSidebar& src); // Split panes clone the region UI (Blender)

   // Per-pane sidebar setup (open state, tab, width), serialized inside the hosting leaf's
   // persisted payload so it is restored with the layout on the next session
   nlohmann::json SaveSetup() const;
   void LoadSetup(const nlohmann::json& j);

private:
   enum class Tab : int
   {
      Item = 0,
      Tool,
      View
   };

   void RenderItemTab();
   void RenderToolTab();
   void RenderViewTab();
   // Length unit of the fields (same mapping as the property pane's length unit)
   float ToDisplayLen(const float vpu) const;
   float ToVpuLen(const float v) const;
   const char* LenUnitLabel() const;
   // Marks the selection for undo once per field gesture (no record while inspecting a live copy)
   void MarkSelectionUndo();

   EditorUI& m_editor;
   ViewportPane& m_pane;

   bool m_shown = false; // View > Sidebar / N: the tab chips are shown (the shelf may still be collapsed)
   bool m_expanded = true; // The tab shelf is open next to the chips (clicking the active tab's chip collapses it)
   Tab m_tab = Tab::Item;
   float m_width = 240.f; // Logical px (scaled by dpi), resizable by dragging the panel's left edge
   float m_drawnWidth = 0.f; // Drawn width last frame, display px (0 while hidden)
   float m_contentHeight = 0.f; // Content height measured last frame, display px (the shelf is sized on it)
   bool m_hovered = false; // Mouse over the panel this frame
   bool m_undoMarked = false; // The in-flight field gesture was marked for undo (re-arms on deactivation)
};

}
