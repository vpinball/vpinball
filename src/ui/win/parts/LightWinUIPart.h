#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"
#include "ui/win/parts/LightCenterWinUIPart.h"

class Light;

class LightWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_LIGHT;
   static inline constexpr int CursorID = IDC_LIGHT;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::PlayfieldAndBackglass;

   explicit LightWinUIPart(PinTableWnd* editor, Light* light);

   ItemTypeEnum GetItemType() const override { return eItemLight; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void EditMenu(Win32xx::CMenu& menu) override;
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(DragPoint* point) override { return m_pointParts.Get(point); }
   IWinUIPart* GetSubPartByIndex(int index) override { return (index == LightCenterSubPartIndex) ? static_cast<IWinUIPart*>(&m_centerPart) : m_pointParts.GetAt(index); }

private:
   void RenderOutline(Sur* psur);

   Light* const m_light;
   DragPointUIPartList m_pointParts;
   LightCenterWinUIPart m_centerPart;
};
