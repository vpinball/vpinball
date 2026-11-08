// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

struct AreaRect; // AreaLayout.h

namespace VPX::EditorUI
{

class EditorUI;

// Top and bottom chrome of the editor: the main menubar and the status bar. Per-view
// controls and status overlays live in each area (Blender): camera & gizmo status is
// drawn by ViewportPane::RenderStatusOverlay, the context helper hints of the status
// bar by each view's AppendStatusHints.
class EditorChrome
{
public:
   explicit EditorChrome(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(bool showFullUI);
   float GetTopBarHeight() const { return m_menuBarHeight; }
   // Height of the status bar at the bottom of the window (0 while the full UI is hidden): the areas end above it
   float GetStatusBarHeight() const { return m_statusBarHeight; }
   // Draws the status bar (context key hints, last operation report). Called after the areas so it stays on top of them
   void RenderStatusBar(bool showFullUI);
   // Opens the Edit > Preferences window
   void OpenPreferences() { m_preferencesOpen = true; }
   // Tool shelf: the T overlay of the viewport (edit modes, add part, gizmo orientation)
   void RenderToolShelf(const AreaRect &viewportContent);
   bool IsToolShelfShown() const { return m_showToolShelf; }
   void ToggleToolShelf() { m_showToolShelf = !m_showToolShelf; }
   float GetToolShelfWidth() const { return m_toolShelfWidth; } // Drawn shelf width last frame (0 when hidden)
   // Tooltip with a title, an optional shortcut chip and an optional dim description (works on disabled items)
   static void Tip(const char *title, const char *shortcut = nullptr, const char *description = nullptr);

private:
   void RenderMenuBar();
   void RenderPreferences(); // The Edit > Preferences window: a page rail on the left, the page content on the right
   bool ToolShelfVisible() const; // Shelf enabled and allowed in the current state (it may still not fit, see m_toolShelfDrawn)

   EditorUI &m_editor;
   float m_menuBarHeight = 0.0f;
   float m_statusBarHeight = 0.0f;
   bool m_preferencesOpen = false;
   int m_preferencesPage = 0; // Selected page of the Preferences window's rail
   bool m_toolShelfDrawn = false; // The shelf was drawn last frame
   float m_toolShelfWidth = 0.f; // Drawn shelf width last frame, used to keep the viewport info text next to it (0 when hidden)
   bool m_showToolShelf = true; // View > Tool Shelf
};

}
