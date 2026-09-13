#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Kicker;

class KickerWinUIPart final : public IWinUIPart
{
public:
   explicit KickerWinUIPart(PinTableWnd* editor, Kicker* kicker);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   Kicker* const m_kicker;
};
