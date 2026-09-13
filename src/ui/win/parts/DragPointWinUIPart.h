#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class DragPoint;

class DragPointWinUIPart final : public IWinUIPart
{
public:
   explicit DragPointWinUIPart(PinTableWnd* editor, DragPoint* dragPoint);

   // Dragpoints are drawn by their parent's IWinUIPart
   void UIRenderPass1(Sur* psur) override { }
   void UIRenderPass2(Sur* psur) override { }

   void OnLButtonDown(int x, int y) override;
   void OnLButtonUp(int x, int y) override;
   void UpdateStatusBarObjectPos() override;

   int GetMenuId() const override;
   void EditMenu(Win32xx::CMenu& menu) override;
   void DoCommand(int icmd, int x, int y) override;

private:
   DragPoint* const m_dragPoint;
};
