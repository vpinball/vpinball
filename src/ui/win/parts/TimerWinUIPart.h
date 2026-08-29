#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Timer;

class TimerWinUIPart final : public IWinUIPart
{
public:
   explicit TimerWinUIPart(PinTableWnd* editor, Timer* timer);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Timer* const m_timer;
};
