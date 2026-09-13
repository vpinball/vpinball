#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Rubber;

class RubberWinUIPart final : public IWinUIPart
{
public:
   explicit RubberWinUIPart(PinTableWnd* editor, Rubber* rubber);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   void DoCommand(int icmd, int x, int y) override;

private:
   Rubber* const m_rubber;
};
