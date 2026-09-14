// license:GPLv3+

#include "core/stdafx.h"

#include "core/ieditable.h"
#include "core/iselect.h"
#include "parts/pintable.h"
#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/TableWinUIPart.h"

void IWinUIPart::OnLButtonDown(int x, int y)
{
#ifndef __STANDALONE__
   m_dragging = true;
   m_markedForUndo = false; // So we will be marked when and if we are dragged

   m_select->GetPTable()->SetMouseCapture();

   UpdateStatusBarObjectPos();
#endif
}

void IWinUIPart::OnLButtonUp(int x, int y)
{
#ifndef __STANDALONE__
   m_dragging = false;

   ReleaseCapture();

   if (m_markedForUndo)
   {
      m_markedForUndo = false;
      m_select->GetIEditable()->EndUndo();
      if (m_select->GetPTable())
         m_select->GetPTable()->SetDirtyDraw();
   }
#endif
}

void IWinUIPart::UpdateStatusBarObjectPos()
{
#ifndef __STANDALONE__
   m_editor->m_vpxEditor->ClearObjectPosCur();
#endif
}

void IWinUIPart::SetStatusBarObjectPos(const float x, const float y)
{
#ifndef __STANDALONE__
   m_editor->m_vpxEditor->SetObjectPosCur(x, y);
#endif
}

void IWinUIPart::DoCommand(int icmd, int x, int y)
{
#ifndef __STANDALONE__
   // Commands that are handled by the table element
   if ((((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020)) // Assign to collection
      || ((icmd >= ID_ASSIGN_TO_LAYER1) && (icmd <= ID_ASSIGN_TO_LAYER1 + NUM_ASSIGN_LAYERS - 1)) // Assign to layer
      || (icmd == ID_EDIT_DRAWINGORDER_HIT) || (icmd == ID_EDIT_DRAWINGORDER_SELECT) || (icmd == ID_ASSIGN_TO_CURRENT_LAYER) || (icmd == IDC_COPY) || (icmd == IDC_PASTE)
      || (icmd == IDC_PASTEAT))
   {
      m_editor->m_tablePart.DoCommand(icmd, x, y);
      return;
   }

   IEditable *const piedit = m_select->GetIEditable();
   if ((icmd & 0x0000FFFF) == ID_SELECT_ELEMENT)
   {
      const int ksshift = GetKeyState(VK_SHIFT);
      //const int ksctrl = GetKeyState(VK_CONTROL);

      PinTable *const currentTable = m_select->GetPTable();
      const int i = (icmd & 0x00FF0000) >> 16;
      ISelect *const pisel = currentTable->m_allHitElements[i];

      const bool add = ((ksshift & 0x80000000) != 0);

      if (pisel == (ISelect *)currentTable && add)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         m_editor->m_tablePart.OnLButtonDown(x, y); // Start the band select
         return;
      }

      m_editor->AddMultiSel(pisel, add, true, true);
      return;
   }
   switch (icmd)
   {
   case ID_DRAWINFRONT: m_select->GetPTable()->MovePartToFront(piedit); break;
   case ID_DRAWINBACK: m_select->GetPTable()->MovePartToBack(piedit); break;
   case ID_SETASDEFAULT: piedit->WriteRegDefaults(); break;
   case ID_LOCK:
      piedit->BeginUndo();
      piedit->MarkForUndo();
      piedit->SetUILock(!piedit->IsUILocked());
      piedit->EndUndo();
      if (m_select->GetPTable())
         m_select->GetPTable()->SetDirtyDraw();
      break;
   }
#endif
}
