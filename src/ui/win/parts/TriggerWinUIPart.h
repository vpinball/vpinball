#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Trigger;

class TriggerWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_TRIGGER;
   static inline constexpr int CursorID = IDC_TRIGGER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit TriggerWinUIPart(PinTableWnd* editor, Trigger* trigger);

   ItemTypeEnum GetItemType() const override { return eItemTrigger; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void EditMenu(Win32xx::CMenu& menu) override;
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(DragPoint* point) override { return m_pointParts.Get(point); }

private:
   Trigger* const m_trigger;
   DragPointUIPartList m_pointParts;
};
