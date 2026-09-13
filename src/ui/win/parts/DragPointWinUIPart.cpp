// license:GPLv3+

#include "core/stdafx.h"

#include "parts/dragpoint.h"
#include "parts/pintable.h"
#include "ui/win/parts/DragPointWinUIPart.h"

DragPointWinUIPart::DragPointWinUIPart(PinTableWnd* editor, DragPoint* dragPoint)
   : IWinUIPart(editor, dragPoint)
   , m_dragPoint(dragPoint)
{
}

void DragPointWinUIPart::OnLButtonDown(int x, int y)
{
   IWinUIPart::OnLButtonDown(x, y);
   m_dragPoint->GetPTable()->SetDirtyDraw();
}

void DragPointWinUIPart::OnLButtonUp(int x, int y)
{
   IWinUIPart::OnLButtonUp(x, y);
   m_dragPoint->GetPTable()->SetDirtyDraw();
}

void DragPointWinUIPart::UpdateStatusBarObjectPos()
{
   SetStatusBarObjectPos(m_dragPoint->m_v.x, m_dragPoint->m_v.y);
}

int DragPointWinUIPart::GetMenuId() const
{
   return (m_dragPoint->GetIEditable()->GetItemType() == eItemRubber) ? IDR_POINTMENU_SMOOTH : IDR_POINTMENU;
}

void DragPointWinUIPart::EditMenu(CMenu& menu)
{
   menu.CheckMenuItem(ID_POINTMENU_SMOOTH, MF_BYCOMMAND | (m_dragPoint->m_smooth ? MF_CHECKED : MF_UNCHECKED));
   //EnableMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | (m_fSmooth ? MF_GRAYED : MF_ENABLED));
   menu.CheckMenuItem(ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | ((m_dragPoint->m_slingshot && !m_dragPoint->m_smooth) ? MF_CHECKED : MF_UNCHECKED));
}

void DragPointWinUIPart::DoCommand(int icmd, int x, int y)
{
   IWinUIPart::DoCommand(icmd, x, y);
   switch (icmd)
   {
   case ID_POINTMENU_SMOOTH: m_dragPoint->ToggleSmooth(); break;
   case ID_POINTMENU_SLINGSHOT: m_dragPoint->ToggleSlingshot(); break;
   }
}
