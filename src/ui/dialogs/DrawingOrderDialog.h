// license:GPLv3+

#pragma once

class DrawingOrderDialog final : public CDialog
{
public:
   DrawingOrderDialog(bool select);
   ~DrawingOrderDialog() override;

protected:
   void OnClose() override;
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

   void UpdateDrawingOrder(IEditable *ptr, bool up);

private:
   HWND hOrderList;
   bool m_drawingOrderSelect;
};
