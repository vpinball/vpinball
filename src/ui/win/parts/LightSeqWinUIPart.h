#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class LightSeq;

class LightSeqWinUIPart final : public IWinUIPart
{
public:
   explicit LightSeqWinUIPart(PinTableWnd* editor, LightSeq* lightseq);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   void RenderOutline(Sur* psur);

   PinTableWnd* const m_editor;
   LightSeq* const m_lightseq;
};
