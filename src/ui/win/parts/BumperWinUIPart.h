#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Bumper;

class BumperWinUIPart final : public IWinUIPart
{
public:
   explicit BumperWinUIPart(PinTableWnd* editor, Bumper* bumper);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   Bumper* const m_bumper;
};
