#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Flasher;

class FlasherWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_FLASHER;
   static inline constexpr int CursorID = IDC_FLASHER;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::PlayfieldAndBackglass;

   explicit FlasherWinUIPart(PinTableWnd* editor, Flasher* flasher);

   ItemTypeEnum GetItemType() const override { return eItemFlasher; }
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void UpdateStatusBarObjectPos() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(DragPoint* point) override { return m_pointParts.Get(point); }
   IWinUIPart* GetSubPartByIndex(int index) override { return m_pointParts.GetAt(index); }

private:
   Flasher* const m_flasher;
   DragPointUIPartList m_pointParts;
};
