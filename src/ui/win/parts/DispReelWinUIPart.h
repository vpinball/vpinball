#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class DispReel;

class DispReelWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_DISPREEL;
   static inline constexpr int CursorID = IDC_DISPREEL;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Backglass;

   explicit DispReelWinUIPart(PinTableWnd* editor, DispReel* dispreel);

   ItemTypeEnum GetItemType() const override { return eItemDispReel; }
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;

private:
   DispReel* const m_dispreel;
};
