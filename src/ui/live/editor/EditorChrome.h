// license:GPLv3+

#pragma once

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

private:
   void RenderMenuBar();

   EditorUI &m_editor;
   float m_menuBarHeight = 0.0f;
   float m_statusBarHeight = 0.0f;
};

}
