#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Trigger;

class TriggerWinUIPart final : public IWinUIPart
{
public:
   explicit TriggerWinUIPart(PinTableWnd* editor, Trigger* trigger);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void EditMenu(Win32xx::CMenu& menu) override;
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(ISelect* select) override { return m_pointParts.Get(select); }

private:
   Trigger* const m_trigger;
   DragPointUIPartList m_pointParts;
};
