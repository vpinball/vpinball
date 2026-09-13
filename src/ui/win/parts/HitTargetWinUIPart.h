#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class HitTarget;

class HitTargetWinUIPart final : public IWinUIPart
{
public:
   explicit HitTargetWinUIPart(PinTableWnd* editor, HitTarget* hittarget);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;

private:
   HitTarget* const m_hittarget;
};
