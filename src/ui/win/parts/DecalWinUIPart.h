#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Decal;

class DecalWinUIPart final : public IWinUIPart
{
public:
   explicit DecalWinUIPart(PinTableWnd* editor, Decal* decal);
   
   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Decal* const m_decal;
};
