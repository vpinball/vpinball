#pragma once

#include "core/ieditable.h"
#include "ui/win/resource.h"

class Sur;
class DragPoint;
class PinTableWnd;
namespace Win32xx
{
class CMenu;
}

class IWinUIPart
{
public:
   // Views in which a part can be edited in the Win32 editor (playfield and/or backglass)
   enum class AllowedViews : unsigned
   {
      None = 0,
      Playfield = 1,
      Backglass = 2,
      PlayfieldAndBackglass = 3
   };

   // Returns true if a part with the given allowed views may be used in the given view
   static constexpr bool IsViewAllowed(const AllowedViews allowedViews, const AllowedViews view) { return (static_cast<unsigned>(allowedViews) & static_cast<unsigned>(view)) != 0; }

   IWinUIPart(PinTableWnd* editor, IEditable* editable)
      : m_editor(editor)
      , m_editable(editable)
   {
   }
   virtual ~IWinUIPart() = default;

   IEditable* GetEditable() const { return m_editable; }

   // Type of the table part this UI part edits
   virtual ItemTypeEnum GetItemType() const = 0;

   // True for UI parts of sub selects (drag points, light centers) owned by another part
   virtual bool IsSubPart() const { return false; }

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

   // Updates the editor status bar info field with part specific information (dimensions, mesh stats, ...)
   virtual void UpdateStatusBarInfo();

   // Returns the context menu resource id used when right-clicking this part (-1 for none)
   virtual int GetMenuId() const { return -1; }

   // Allows the part to customize the editor context menu before it is displayed
   virtual void EditMenu(Win32xx::CMenu& menu) { }

   // Executes a command picked from the context menu
   virtual void DoCommand(int icmd, int x, int y);

   // Returns the drag point edited by this UI part, nullptr if this UI part does not edit a drag point
   virtual DragPoint* GetDragPoint() const { return nullptr; }

   // Returns the UI part for a sub element owned by this part's editable (a drag point), nullptr if none
   virtual IWinUIPart* GetSubPart(DragPoint* point) { return nullptr; }

   // Sub part index of a light's center handle (its single non drag point sub part)
   static constexpr int LightCenterSubPartIndex = -2;

   // Index identifying this part as a sub part of its owner (index of a drag point in the owner's DragPointCurve,
   // or a predefined negative id for unique sub parts like the light center). -1 when this is not a sub part.
   // Unlike pointers, the index remains valid when the part's drag points are deleted and recreated (undo, ...).
   virtual int GetSubPartIndex() const { return -1; }

   // Returns the UI part of this part's sub part with the given index (as returned by GetSubPartIndex), nullptr if none
   virtual IWinUIPart* GetSubPartByIndex(int index) { return nullptr; }

   // Geometric transforms of the selectable element (applied to the sub element itself for sub selects like drag points)
   // Higher level transforms (scale, rotate, flip) are performed on the IEditable, or on the sub element through GetCenter/Translate
   virtual Vertex2D GetCenter() const;
   virtual void Translate(const Vertex2D& offset);

protected:
   void SetStatusBarObjectPos(const float x, const float y);

   const COLORREF m_blueprintSolidColor = RGB(0, 0, 0);

   PinTableWnd* const m_editor;
   IEditable* const m_editable;
};
