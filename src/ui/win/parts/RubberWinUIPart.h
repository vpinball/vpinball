#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Rubber;

class RubberWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_RUBBER;
   static inline constexpr int CursorID = IDC_RUBBER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit RubberWinUIPart(PinTableWnd* editor, Rubber* rubber);

   ItemTypeEnum GetItemType() const override { return eItemRubber; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   void UpdateStatusBarInfo() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(DragPoint* point) override { return m_pointParts.Get(point); }
   IWinUIPart* GetSubPartByIndex(int index) override { return m_pointParts.GetAt(index); }

private:
   Rubber* const m_rubber;
   DragPointUIPartList m_pointParts;
};
