#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class LightSeq;

class LightSeqWinUIPart final : public IWinUIPart
{
public:
   static inline constexpr int ToolID = ID_INSERT_LIGHTSEQ;
   static inline constexpr int CursorID = IDC_LIGHTSEQ;
   static inline constexpr IWinUIPart::AllowedViews AllowedViews = IWinUIPart::AllowedViews::PlayfieldAndBackglass;

   explicit LightSeqWinUIPart(PinTableWnd* editor, LightSeq* lightseq);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;

private:
   void RenderOutline(Sur* psur);

   LightSeq* const m_lightseq;
};
