// license:GPLv3+

#pragma once

#include "core/ieditable.h"
#include "imgui/imgui.h"

#include <memory>
#include <vector>

class Sampler;

namespace VPX::EditorUI
{

class EditorUI;

// Top chrome of the editor: main menubar, toolbar (with the add part picker popup)
// and the status overlay text. Owns the heights of the top bars used to position
// the side panels.
class EditorChrome
{
public:
   explicit EditorChrome(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(bool showFullUI);
   float GetTopBarHeight() const { return m_menuBarHeight + m_toolBarHeight; }

   // Requests the part type picker popup to be opened at the given position (Shift+A shortcut).
   // The popup is opened from the toolbar window scope, as OpenPopup must be called in the same
   // window scope as the matching BeginPopup.
   void RequestAddPartPopup(const ImVec2 &pos)
   {
      m_addPartPopupPos = pos;
      m_openAddPartPopup = true;
   }

private:
   void RenderMenuBar();
   void RenderToolbar();
   void RenderStatusOverlay();

   EditorUI &m_editor;
   float m_menuBarHeight = 0.0f;
   float m_toolBarHeight = 0.0f;
   struct AddPartButton
   {
      ItemTypeEnum type;
      const char *name;
      std::shared_ptr<Sampler> icon;
   };
   std::vector<AddPartButton> m_addPartButtons; // Lazily initialized add part toolbar buttons
   ImVec2 m_addPartPopupPos {}; // Position at which the Shift+A part type picker popup was opened
   bool m_openAddPartPopup = false; // Request to open the part type picker popup (consumed in the toolbar window scope)
};

}
