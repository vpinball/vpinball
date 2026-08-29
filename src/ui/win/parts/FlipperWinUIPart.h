#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Flipper;

class FlipperWinUIPart final : public IWinUIPart
{
public:
   explicit FlipperWinUIPart(PinTableWnd* editor, Flipper* flipper);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;

private:
   PinTableWnd* const m_editor;
   Flipper* const m_flipper;
};
