#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Flasher;

class FlasherWinUIPart final : public IWinUIPart
{
public:
   explicit FlasherWinUIPart(PinTableWnd* editor, Flasher* flasher);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;
   void DoCommand(int icmd, int x, int y) override;

private:
   Flasher* const m_flasher;
};
