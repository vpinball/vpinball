#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Primitive;

class PrimitiveWinUIPart final : public IWinUIPart
{
public:
   explicit PrimitiveWinUIPart(PinTableWnd* editor, Primitive* primitive);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Primitive* const m_primitive;
};
