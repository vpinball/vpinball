// license:GPLv3+

#include "core/stdafx.h"
#include "ui/properties/BallPhysicsProperty.h"
#include <WindowsX.h>

BallPhysicsProperty::BallPhysicsProperty(const VectorProtected<ISelect> *pvsel)
   : BasePropertyDialog(IDD_PROPBALL_PHYSICS, pvsel)
{
   m_massEdit.SetDialog(this);
}

void BallPhysicsProperty::UpdateVisuals(const int dispid /*=-1*/)
{
   //only show the first element on multi-select
   Ball *const ball = (Ball *)m_pvsel->ElementAt(0);
   if (ball == nullptr)
      return;
   if (dispid == 14 || dispid == -1)
      PropertyDialog::SetFloatTextbox(m_massEdit, ball->m_hitBall.m_d.m_mass);
   UpdateBaseVisuals(ball, &ball->m_d, dispid);
}

void BallPhysicsProperty::UpdateProperties(const int dispid)
{
   for (int i = 0; i < m_pvsel->size(); i++)
   {
      if ((m_pvsel->ElementAt(i) == nullptr) || (m_pvsel->ElementAt(i)->GetItemType() != eItemBall))
         continue;
      Ball *const ball = (Ball *)m_pvsel->ElementAt(0);
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
   m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 0, 0));
   m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topleft, 0);
   m_resizer.AddChild(m_massEdit.GetHwnd(), CResizer::topleft, RD_STRETCH_WIDTH);

   return TRUE;
}

INT_PTR BallPhysicsProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
