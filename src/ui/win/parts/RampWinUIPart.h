#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/parts/DragPointUIPartList.h"

class Ramp;

class RampWinUIPart final : public IWinUIPart
{
public:
   explicit RampWinUIPart(PinTableWnd* editor, Ramp* ramp);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;
   void UpdateStatusBarObjectPos() override;
   int GetMenuId() const override { return IDR_SURFACEMENU; }
   void DoCommand(int icmd, int x, int y) override;

   IWinUIPart* GetSubPart(ISelect* select) override { return m_pointParts.Get(select); }

private:
   Ramp* const m_ramp;
   DragPointUIPartList m_pointParts;
};
