#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Spinner;

class SpinnerWinUIPart final : public IWinUIPart
{
public:
   explicit SpinnerWinUIPart(PinTableWnd* editor, Spinner* spinner);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;

private:
   PinTableWnd* const m_editor;
   Spinner* const m_spinner;
};
