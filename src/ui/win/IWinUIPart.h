#pragma once

#include "core/iselect.h"

class Sur;
class IEditable;
class ISelect;
class PinTableWnd;
namespace Win32xx
{
class CMenu;
}

class IWinUIPart
{
public:
   IWinUIPart(PinTableWnd* editor, ISelect* select)
      : m_editor(editor)
      , m_select(select)
   {
   }
   virtual ~IWinUIPart() = default;

   ISelect* GetSelect() const { return m_select; }
   IEditable* GetEditable() const { return m_select->GetIEditable(); }

   enum class SelectState
   {
      NotSelected,
      Selected,
      MultiSelected
   };
   SelectState m_selectstate = SelectState::NotSelected;

   bool m_dragging = false;

   bool m_markedForUndo = false; // Flag set when dragged to enable undo

   // This function draws the shape of the object with a solid fill, called before the grid lines are drawn on the map
   virtual void UIRenderPass1(Sur* psur) = 0;
   // This function draws the shape of the object with a black outline (no solid fill), called after the grid lines have been drawn on the map.
   virtual void UIRenderPass2(Sur* psur) = 0;

   virtual void RenderBlueprint(Sur* psur, bool solid) { UIRenderPass2(psur); }

   // Win32 editor picking
   virtual void OnLButtonDown(int x, int y);
   virtual void OnLButtonUp(int x, int y);

   // Updates the editor status bar with the part's position (in table units)
   virtual void UpdateStatusBarObjectPos();

   // Returns the context menu resource id used when right-clicking this part (-1 for none)
   virtual int GetMenuId() const { return -1; }

   // Allows the part to customize the editor context menu before it is displayed
   virtual void EditMenu(Win32xx::CMenu& menu) { }

   // Executes a command picked from the context menu
   virtual void DoCommand(int icmd, int x, int y);

   // Returns the UI part for a sub select owned by this part's editable (e.g. a drag point, a light center), nullptr if none
   virtual IWinUIPart* GetSubPart(ISelect* select) { return nullptr; }

protected:
   void SetStatusBarObjectPos(const float x, const float y);

   PinTableWnd* const m_editor;
   ISelect* const m_select;
};
