#pragma once

class Sur;
class IEditable;
namespace Win32xx
{
class CMenu;
}

class IWinUIPart
{
public:
   virtual ~IWinUIPart() = default;

   // This function draws the shape of the object with a solid fill, called before the grid lines are drawn on the map
   virtual void UIRenderPass1(Sur* psur) = 0;
   // This function draws the shape of the object with a black outline (no solid fill), called after the grid lines have been drawn on the map.
   virtual void UIRenderPass2(Sur* psur) = 0;

   virtual void RenderBlueprint(Sur* psur, bool solid) { UIRenderPass2(psur); }

   // Allows the part to customize the editor context menu before it is displayed
   virtual void EditMenu(Win32xx::CMenu& menu) { }
};
