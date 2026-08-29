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

private:
   PinTableWnd* const m_editor;
   Flasher* const m_flasher;
};
