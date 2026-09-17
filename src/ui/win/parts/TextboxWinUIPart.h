#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Textbox;

class TextboxWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_TEXTBOX;
   static inline constexpr int CursorID = IDC_TEXTBOX;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Backglass;

   explicit TextboxWinUIPart(PinTableWnd* editor, Textbox* textbox);

   ItemTypeEnum GetItemType() const override { return eItemTextbox; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;

private:
   Textbox* const m_textbox;
};
