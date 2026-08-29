#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Textbox;

class TextboxWinUIPart final : public IWinUIPart
{
public:
   explicit TextboxWinUIPart(PinTableWnd* editor, Textbox* textbox);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;

private:
   PinTableWnd* const m_editor;
   Textbox* const m_textbox;
};
