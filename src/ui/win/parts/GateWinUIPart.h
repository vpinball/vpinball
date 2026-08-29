#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Gate;

class GateWinUIPart final : public IWinUIPart
{
public:
   explicit GateWinUIPart(PinTableWnd* editor, Gate* gate);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Gate* const m_gate;
};
