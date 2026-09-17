#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Primitive;

class PrimitiveWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_PRIMITIVE;
   static inline constexpr int CursorID = IDC_PRIMITIVE;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::Playfield;

   explicit PrimitiveWinUIPart(PinTableWnd* editor, Primitive* primitive);

   ItemTypeEnum GetItemType() const override { return eItemPrimitive; }

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   void UpdateStatusBarInfo() override;

private:
   Primitive* const m_primitive;
};
