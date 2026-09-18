// license:GPLv3+

#include "core/stdafx.h"

#include "parts/pintable.h"
#include "parts/PartGroup.h"
#include "ui/win/DragPointDialogs.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/TableWinUIPart.h"

TableWinUIPart::TableWinUIPart(PinTableWnd *editor, PinTable *table)
   : IWinUIPart(editor, table)
   , m_table(table)
{
}

void TableWinUIPart::OnLButtonDown(int x, int y)
{
   const Vertex2D v = m_editor->TransformPoint(x, y);

   m_table->m_rcDragRect.left = v.x;
   m_table->m_rcDragRect.right = v.x;
   m_table->m_rcDragRect.top = v.y;
   m_table->m_rcDragRect.bottom = v.y;

   m_dragging = true;

   m_editor->SetCapture();

   m_table->SetDirtyDraw();
}

void TableWinUIPart::DoCommand(int icmd, int x, int y)
{
   if (((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020))
   {
      vector<IEditable *> selection;
      for (IWinUIPart *const uiPart : m_editor->GetMultiSelParts())
         selection.push_back(uiPart->GetEditable());
      m_table->ToggleCollectionMembership(icmd & 0x000000FF, selection);
      return;
   }

   constexpr unsigned int ID_ASSIGN_TO_LAYER_MAX = ID_ASSIGN_TO_LAYER1 + NUM_ASSIGN_LAYERS - 1;
   if ((icmd >= ID_ASSIGN_TO_LAYER1) && (icmd <= ID_ASSIGN_TO_LAYER_MAX))
   {
      PartGroup *group = nullptr;
      int layerIndex = icmd - ID_ASSIGN_TO_LAYER1;
      for (IEditable *edit : m_table->GetParts())
      {
         if (edit->GetItemType() == eItemPartGroup && edit->GetPartGroup() == nullptr)
         {
            if (layerIndex == 0)
               group = static_cast<PartGroup *>(edit);
            layerIndex--;
            if (layerIndex < 0)
               break;
         }
      }
      if (group)
         m_editor->AssignSelectionToPartGroup(group);
      return;
   }

   if ((icmd & 0x0000FFFF) == ID_SELECT_ELEMENT)
   {
      const int i = (icmd & 0x00FF0000) >> 16;
      IWinUIPart *const uiPart = m_editor->m_allHitElements[i];
      // The hit elements can contain the table itself, whose UI part is this part: do not recurse into our own DoCommand
      if (uiPart && uiPart != this)
         uiPart->DoCommand(icmd, x, y);
      return;
   }

   switch (icmd)
   {
   case ID_DRAWINFRONT:
   case ID_DRAWINBACK:
   {
      for (IWinUIPart *const uiPart : m_editor->GetMultiSelParts())
      {
         _ASSERTE(uiPart->GetEditable() != m_table); // Would make an infinite loop
         if (uiPart != this)
            uiPart->DoCommand(icmd, x, y);
      }
      break;
   }
   case ID_ASSIGN_TO_CURRENT_LAYER: m_editor->m_vpxEditor->GetLayersListDialog()->AssignToSelectedGroup(); break;
   case ID_EDIT_DRAWINGORDER_HIT: m_editor->m_vpxEditor->ShowDrawingOrderDialog(false); break;
   case ID_EDIT_DRAWINGORDER_SELECT: m_editor->m_vpxEditor->ShowDrawingOrderDialog(true); break;
   case ID_LOCK: m_editor->LockElements(); break;
   case ID_WALLMENU_FLIP: m_editor->FlipYMultiSel(m_editor->GetMultiSelCenter()); break;
   case ID_WALLMENU_MIRROR: m_editor->FlipXMultiSel(m_editor->GetMultiSelCenter()); break;
   case IDC_COPY: m_editor->Copy(x, y); break;
   case IDC_PASTE: m_editor->Paste(false, x, y); break;
   case IDC_PASTEAT: m_editor->Paste(true, x, y); break;
   case ID_WALLMENU_ROTATE: (void)VPX::WinUI::RotatePointsDialog(m_editor); break;
   case ID_WALLMENU_SCALE: (void)VPX::WinUI::ScalePointsDialog(m_editor); break;
   case ID_WALLMENU_TRANSLATE: (void)VPX::WinUI::TranslatePointsDialog(m_editor); break;
   }
}
