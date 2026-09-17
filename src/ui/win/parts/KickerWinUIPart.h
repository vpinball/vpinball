#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Kicker;

class KickerWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_KICKER;
   static inline constexpr int CursorID = IDC_KICKER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit KickerWinUIPart(PinTableWnd* editor, Kicker* kicker);

   ItemTypeEnum GetItemType() const override { return eItemKicker; }
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   void UpdateStatusBarInfo() override;

private:
   Kicker* const m_kicker;
};
