// license:GPLv3+

#pragma once

class DrawingOrderDialog : public CDialog
{
public:
   DrawingOrderDialog(bool select);
   virtual ~DrawingOrderDialog();

protected:
   virtual void OnClose();
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   void         UpdateDrawingOrder(IEditable *ptr, bool up);

private:
   HWND hOrderList;
   bool m_drawingOrderSelect;
};
