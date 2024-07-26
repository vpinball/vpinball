#pragma once

class BallPhysicsProperty: public BasePropertyDialog
{
public:
   BallPhysicsProperty(const VectorProtected<ISelect> *pvsel);
   virtual ~BallPhysicsProperty() { }

   void UpdateVisuals(const int dispid=-1);
   void UpdateProperties(const int dispid);

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
   EditBox   m_massEdit;
};
