#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Plunger;

class PlungerWinUIPart final : public IWinUIPart
{
public:
   explicit PlungerWinUIPart(PinTableWnd* editor, Plunger* plunger);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;

private:
   Plunger* const m_plunger;
};
