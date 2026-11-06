// license:GPLv3+

#pragma once

namespace VPX::EditorUI
{

class EditorUI;

// Top chrome of the editor: the main menubar and the status overlay text drawn over the
// edit viewport. Per-view controls live in the header of each area (Blender), not here.
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
   void RenderStatusOverlay();

   EditorUI &m_editor;
   float m_menuBarHeight = 0.0f;
};

}
