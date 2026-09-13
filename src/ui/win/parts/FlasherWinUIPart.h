#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Flasher;

class FlasherWinUIPart final : public IWinUIPart
{
public:
   explicit FlasherWinUIPart(PinTableWnd* editor, Flasher* flasher);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(ISelect* select) override { return m_pointParts.Get(select); }

private:
   Flasher* const m_flasher;
   DragPointUIPartList m_pointParts;
};
