// license:GPLv3+

#pragma once

class BallPhysicsProperty final : public BasePropertyDialog
{
public:
   BallPhysicsProperty(const VectorProtected<ISelect> *pvsel);
   ~BallPhysicsProperty() override { }

   void UpdateVisuals(const int dispid=-1) override;
   void UpdateProperties(const int dispid) override;

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   EditBox   m_massEdit;
};
