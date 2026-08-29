#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class DispReel;

class DispReelWinUIPart final : public IWinUIPart
{
public:
   explicit DispReelWinUIPart(PinTableWnd* editor, DispReel* dispreel);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;

private:
   PinTableWnd* const m_editor;
   DispReel* const m_dispreel;
};
