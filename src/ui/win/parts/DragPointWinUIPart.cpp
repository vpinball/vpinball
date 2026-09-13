// license:GPLv3+

#include "core/stdafx.h"

#include "parts/dragpoint.h"
#include "ui/win/parts/DragPointWinUIPart.h"

DragPointWinUIPart::DragPointWinUIPart(PinTableWnd* editor, DragPoint* dragPoint)
   : m_editor(editor)
   , m_dragPoint(dragPoint)
{
}

void DragPointWinUIPart::EditMenu(CMenu& menu)
{
   menu.CheckMenuItem(ID_POINTMENU_SMOOTH, MF_BYCOMMAND | (m_dragPoint->m_smooth ? MF_CHECKED : MF_UNCHECKED));
   //EnableMenuItem(hmenu, ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | (m_fSmooth ? MF_GRAYED : MF_ENABLED));
   menu.CheckMenuItem(ID_POINTMENU_SLINGSHOT, MF_BYCOMMAND | ((m_dragPoint->m_slingshot && !m_dragPoint->m_smooth) ? MF_CHECKED : MF_UNCHECKED));
}
