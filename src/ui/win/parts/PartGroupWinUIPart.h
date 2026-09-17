#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class PartGroup;

class PartGroupWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_PARTGROUP;
   static inline constexpr int CursorID = IDC_PARTGROUP;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::PlayfieldAndBackglass;

   explicit PartGroupWinUIPart(PinTableWnd* editor, PartGroup* partgroup);

   ItemTypeEnum GetItemType() const override { return eItemPartGroup; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   PartGroup* const m_partgroup;
};
