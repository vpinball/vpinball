#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Trigger;

class TriggerWinUIPart final : public IWinUIPart
{
public:
   explicit TriggerWinUIPart(PinTableWnd* editor, Trigger* trigger);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Trigger* const m_trigger;
};
