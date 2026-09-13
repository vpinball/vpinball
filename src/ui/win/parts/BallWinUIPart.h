#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Ball;

class BallWinUIPart final : public IWinUIPart
{
public:
   explicit BallWinUIPart(PinTableWnd* editor, Ball* ball);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   Ball* const m_ball;
};
