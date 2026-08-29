#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Ramp;

class RampWinUIPart final : public IWinUIPart
{
public:
   explicit RampWinUIPart(PinTableWnd* editor, Ramp* ramp);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Ramp* const m_ramp;
};
