#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Decal;

class DecalWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_DECAL;
   static inline constexpr int CursorID = IDC_DECAL;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::PlayfieldAndBackglass;

   explicit DecalWinUIPart(PinTableWnd* editor, Decal* decal);

   ItemTypeEnum GetItemType() const override { return eItemDecal; }
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   Decal* const m_decal;
};
