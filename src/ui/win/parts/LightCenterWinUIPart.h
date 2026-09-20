#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Light;

// UI part for the light center handle: transforms apply to the light center, not the whole light.
class LightCenterWinUIPart final : public IWinUIPart
{
public:
   explicit LightCenterWinUIPart(PinTableWnd* editor, Light* light);

   ItemTypeEnum GetItemType() const override { return eItemLightCenter; }
   bool IsSubPart() const override { return true; }
   int GetSubPartIndex() const override { return LightCenterSubPartIndex; }

   // The light center handle is drawn by the light's IWinUIPart
   void UIRenderPass1(Sur* psur) override { }
   void UIRenderPass2(Sur* psur) override { }

   Vertex2D GetCenter() const override;
   void Translate(const Vertex2D& offset) override;

private:
   Light* const m_light;
};
