#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Ramp;

class RampWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_RAMP;
   static inline constexpr int CursorID = IDC_RAMP;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit RampWinUIPart(PinTableWnd* editor, Ramp* ramp);

   ItemTypeEnum GetItemType() const override { return eItemRamp; }

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
   Ramp* const m_ramp;
   DragPointUIPartList m_pointParts;
};
