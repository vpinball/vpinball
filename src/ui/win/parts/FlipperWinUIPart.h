#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Flipper;

class FlipperWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_FLIPPER;
   static inline constexpr int CursorID = IDC_FLIPPER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit FlipperWinUIPart(PinTableWnd* editor, Flipper* flipper);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;

private:
   Flipper* const m_flipper;
};
