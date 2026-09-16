#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Timer;

class TimerWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_TIMER;
   static inline constexpr int CursorID = IDC_TIMER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::PlayfieldAndBackglass;

   explicit TimerWinUIPart(PinTableWnd* editor, Timer* timer);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   Timer* const m_timer;
};
