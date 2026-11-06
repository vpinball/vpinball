// license:GPLv3+

#pragma once

namespace VPX::EditorUI
{

class EditorUI;

// Top chrome of the editor: the main menubar. Per-view controls and status overlays live
// in each area (Blender): camera & gizmo status is drawn by ViewportPane::RenderStatusOverlay.
class EditorChrome
{
public:
   explicit EditorChrome(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(bool showFullUI);
   float GetTopBarHeight() const { return m_menuBarHeight; }

private:
   void RenderMenuBar();

   EditorUI &m_editor;
   float m_menuBarHeight = 0.0f;
};

}
