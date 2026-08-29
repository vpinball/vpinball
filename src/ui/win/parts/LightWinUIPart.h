#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Light;

class LightWinUIPart final : public IWinUIPart
{
public:
   explicit LightWinUIPart(PinTableWnd* editor, Light* light);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   void RenderOutline(Sur* psur);

   PinTableWnd* const m_editor;
   Light* const m_light;
};
