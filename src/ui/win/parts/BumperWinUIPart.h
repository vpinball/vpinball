#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Bumper;

class BumperWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_BUMPER;
   static inline constexpr int CursorID = IDC_BUMPER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit BumperWinUIPart(PinTableWnd* editor, Bumper* bumper);

   ItemTypeEnum GetItemType() const override { return eItemBumper; }
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   Bumper* const m_bumper;
};
