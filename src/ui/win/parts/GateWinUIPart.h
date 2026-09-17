#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Gate;

class GateWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_GATE;
   static inline constexpr int CursorID = IDC_GATE;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit GateWinUIPart(PinTableWnd* editor, Gate* gate);

   ItemTypeEnum GetItemType() const override { return eItemGate; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   void UpdateStatusBarInfo() override;

private:
   Gate* const m_gate;
};
