#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class PartGroup;

class PartGroupWinUIPart final : public IWinUIPart
{
public:
   explicit PartGroupWinUIPart(PinTableWnd* editor, PartGroup* partgroup);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   PartGroup* const m_partgroup;
};
