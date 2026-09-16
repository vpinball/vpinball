#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Spinner;

class SpinnerWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_SPINNER;
   static inline constexpr int CursorID = IDC_SPINNER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit SpinnerWinUIPart(PinTableWnd* editor, Spinner* spinner);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;
   void UpdateStatusBarInfo() override;

private:
   Spinner* const m_spinner;
};
