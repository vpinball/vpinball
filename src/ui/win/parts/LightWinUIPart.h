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
   void EditMenu(Win32xx::CMenu& menu) override;
   void DoCommand(int icmd, int x, int y) override;

private:
   void RenderOutline(Sur* psur);

   Light* const m_light;
};
