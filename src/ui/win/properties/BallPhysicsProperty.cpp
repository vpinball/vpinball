// license:GPLv3+

#include "core/stdafx.h"
#include "BallPhysicsProperty.h"

#include "parts/ball.h"
#include "parts/Collection.h"
#include "parts/pintable.h"
#include "ui/win/resource.h"


BallPhysicsProperty::BallPhysicsProperty(const vector<IWinUIPart *> *pvsel)
   : BasePropertyDialog(IDD_PROPBALL_PHYSICS, pvsel)
{
   m_massEdit.SetDialog(this);
}

void BallPhysicsProperty::UpdateVisuals(const int dispid /*=-1*/)
{
   //only show the first element on multi-select
   Ball *const ball = (Ball *)SelAt(0)->GetEditable();
   if (ball == nullptr)
      return;
   if (dispid == 14 || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_massEdit, ball->m_hitBall.m_d.m_mass);
   UpdateBaseVisuals(ball, &ball->m_d, dispid);
}

void BallPhysicsProperty::UpdateProperties(const int dispid)
{
   for (int i = 0; i < SelCount(); i++)
   {
      if ((SelAt(i) == nullptr) || (SelAt(i)->GetItemType() != eItemBall))
         continue;
      Ball *const ball = (Ball *)SelAt(0)->GetEditable();
      switch (dispid)
      {
      case IDC_MASS: CHECK_UPDATE_ITEM(ball->m_hitBall.m_d.m_mass, PropertyDialog::GetFloatTextbox(m_massEdit), ball); break;
      default: UpdateBaseProperties(ball, &ball->m_d, dispid); break;
      }
   }
   UpdateVisuals(dispid);
}

BOOL BallPhysicsProperty::OnInitDialog()
{
   m_massEdit.AttachItem(IDC_MASS);

   UpdateVisuals();
   m_resizer.Initialize(GetHwnd(), CRect(0, 0, 0, 0));
   m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_massEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

   return TRUE;
}

INT_PTR BallPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
