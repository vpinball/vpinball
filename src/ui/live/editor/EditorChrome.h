// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

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
   enum class ChevronDir
   {
      Down,
      Right
   };
   // Line chevron (Lucide style, replaces ImGui's filled triangles), centered at c, half width 'size'
   static void DrawChevron(ImDrawList *drawList, const ImVec2 &center, ChevronDir dir, float size, ImU32 color);

private:
   void RenderMenuBar();
   void RenderPreferences(); // The Edit > Preferences window: a page rail on the left, the page content on the right

   EditorUI &m_editor;
   float m_menuBarHeight = 0.0f;
   float m_statusBarHeight = 0.0f;
   bool m_preferencesOpen = false;
   int m_preferencesPage = 0; // Selected page of the Preferences window's rail
};

}
