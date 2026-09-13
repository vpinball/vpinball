#pragma once

#include "ui/win/IWinUIPart.h"

class PinTable;

class TableWinUIPart final : public IWinUIPart
{
public:
   explicit TableWinUIPart(PinTableWnd* editor, PinTable* table);

   // The table itself is rendered by PinTableWnd
   void UIRenderPass1(Sur* psur) override { }
   void UIRenderPass2(Sur* psur) override { }

   void OnLButtonDown(int x, int y) override;
   // Band select is ended by PinTableWnd::OnLeftButtonUp
   void OnLButtonUp(int x, int y) override { }

   void DoCommand(int icmd, int x, int y) override;

private:
   PinTable* const m_table;
};
