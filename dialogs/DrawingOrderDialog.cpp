#include "StdAfx.h"
#include "resource.h"
#include "DrawingOrderDialog.h"

DrawingOrderDialog::DrawingOrderDialog(bool select) : CDialog(IDD_DRAWING_ORDER)
{
   m_drawingOrderSelect = select;
   hOrderList = NULL;
}

DrawingOrderDialog::~DrawingOrderDialog()
{
}

void DrawingOrderDialog::OnClose()
{
   CDialog::OnClose();
}

BOOL DrawingOrderDialog::OnInitDialog()
{
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
   hOrderList = GetDlgItem(IDC_DRAWING_ORDER_LIST).GetHwnd();
   LVCOLUMN lvc;
   LVITEM lv;

   ListView_SetExtendedListViewStyle(hOrderList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   memset(&lvc, 0, sizeof(LVCOLUMN));
   lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
   lvc.cx = 200;
   lvc.pszText = TEXT("Name");
   ListView_InsertColumn(hOrderList, 0, &lvc);
   lvc.cx = 100;
   lvc.pszText = TEXT("Height/Z");
   ListView_InsertColumn(hOrderList, 1, &lvc);
   lvc.cx = 100;
   lvc.pszText = TEXT("Type");
   ListView_InsertColumn(hOrderList, 2, &lvc);

   if (hOrderList != NULL)
      ListView_DeleteAllItems(hOrderList);
   lv.mask = LVIF_TEXT;

   char textBuf[256];
   // create a selection in the same drawing order as the selected elements are stored in the main vector
   Vector<ISelect> selection;
   if (m_drawingOrderSelect)
   {
      for (int i = pt->m_vedit.Size() - 1; i >= 0; i--)
         for (int t = 0; t < pt->m_vmultisel.Size(); t++)
         {
            if (pt->m_vmultisel.ElementAt(t) == pt->m_vedit.ElementAt(i)->GetISelect())
               selection.AddElement(pt->m_vmultisel.ElementAt(t));
         }
   }
   for (int i = 0; i < (m_drawingOrderSelect ? selection.Size() : pt->m_allHitElements.Size()); i++)
   {
      IEditable *pedit = m_drawingOrderSelect ? selection.ElementAt(i)->GetIEditable() : pt->m_allHitElements.ElementAt(i)->GetIEditable();
      if (pedit)
      {
         char *szTemp;
         szTemp = pt->GetElementName(pedit);
         if (szTemp)
         {
            lv.iItem = i;
            lv.iSubItem = 0;
            lv.pszText = szTemp;
            ListView_InsertItem(hOrderList, &lv);
            if (pedit->GetItemType() == eItemSurface)
            {
               Surface *sur = (Surface*)pedit;
               sprintf_s(textBuf, "%.02f", sur->m_d.m_heighttop);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Wall");
            }
            else if (pedit->GetItemType() == eItemPrimitive)
            {
               Primitive *prim = (Primitive*)pedit;
               sprintf_s(textBuf, "%.02f", prim->m_d.m_vPosition.z);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Primitive");
            }
            else if (pedit->GetItemType() == eItemRamp)
            {
               Ramp *ramp = (Ramp*)pedit;
               sprintf_s(textBuf, "%.02f", ramp->m_d.m_heighttop);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Ramp");
            }
            else if (pedit->GetItemType() == eItemFlasher)
            {
               Flasher *flasher = (Flasher*)pedit;
               sprintf_s(textBuf, "%.02f", flasher->m_d.m_height);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Flasher");
            }
            else if (pedit->GetItemType() == eItemRubber)
            {
               Rubber *rubber = (Rubber*)pedit;
               sprintf_s(textBuf, "%.02f", rubber->m_d.m_height);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Rubber");
            }
            else if (pedit->GetItemType() == eItemSpinner)
            {
               Spinner *spin = (Spinner*)pedit;
               sprintf_s(textBuf, "%.02f", spin->m_d.m_height);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Spinner");
            }
            else if (pedit->GetItemType() == eItemKicker)
            {
               Kicker *kick = (Kicker*)pedit;
               sprintf_s(textBuf, "%.02f", kick->m_d.m_hitAccuracy);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Kicker");
            }
            else if (pedit->GetItemType() == eItemLight)
            {
               Light *light = (Light*)pedit;
               ListView_SetItemText(hOrderList, i, 1, "n.a.");
               ListView_SetItemText(hOrderList, i, 2, "Light");
            }
            else if (pedit->GetItemType() == eItemBumper)
            {
               Bumper *bump = (Bumper*)pedit;
               ListView_SetItemText(hOrderList, i, 1, "n.a.");
               ListView_SetItemText(hOrderList, i, 2, "Bumper");
            }
            else if (pedit->GetItemType() == eItemFlipper)
            {
               Flipper *flip = (Flipper*)pedit;
               sprintf_s(textBuf, "%.02f", flip->m_d.m_height);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Flipper");
            }
            else if (pedit->GetItemType() == eItemGate)
            {
               Gate *gate = (Gate*)pedit;
               sprintf_s(textBuf, "%.02f", gate->m_d.m_height);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Gate");
            }
            else if (pedit->GetItemType() == eItemPlunger)
            {
               Plunger *plung = (Plunger*)pedit;
               sprintf_s(textBuf, "%.02f", plung->m_d.m_height);
               ListView_SetItemText(hOrderList, i, 1, textBuf);
               ListView_SetItemText(hOrderList, i, 2, "Plunger");
            }
         }
      }
   }
   return TRUE;
}

BOOL DrawingOrderDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_DRAWING_ORDER_UP:
      {
         UpdateDrawingOrder(NULL, true);
         break;
      }
      case IDC_DRAWING_ORDER_DOWN:
      {
         UpdateDrawingOrder(NULL, false);
         break;
      }
      default:
         return FALSE;
   }
   return TRUE;
}

void DrawingOrderDialog::OnOK()
{
   CDialog::OnOK();
}

void DrawingOrderDialog::UpdateDrawingOrder(IEditable *ptr, bool up)
{
   PinTable *pt = g_pvp->GetActiveTable();
   char text0[256], text1[256], text2[256];
   LVITEM lv;
   lv.mask = LVIF_TEXT;
   int idx = ListView_GetNextItem(hOrderList, -1, LVNI_FOCUSED);
   if (idx == -1)
      return;

   if (up)
   {
      if (idx > 0)
      {
         ListView_GetItemText(hOrderList, idx, 0, text0, 256);
         ListView_GetItemText(hOrderList, idx, 1, text1, 256);
         ListView_GetItemText(hOrderList, idx, 2, text2, 256);
         ListView_DeleteItem(hOrderList, idx);
         lv.iItem = idx - 1;
         lv.iSubItem = 0;
         lv.pszText = text0;
         ListView_InsertItem(hOrderList, &lv);
         ListView_SetItemText(hOrderList, idx - 1, 1, text1);
         ListView_SetItemText(hOrderList, idx - 1, 2, text2);
         ListView_SetItemState(hOrderList, -1, 0, LVIS_SELECTED);
         ListView_SetItemState(hOrderList, idx - 1, LVIS_SELECTED, LVIS_SELECTED);
         ListView_SetItemState(hOrderList, idx - 1, LVIS_FOCUSED, LVIS_FOCUSED);
         ::SetFocus(hOrderList);
         pt->SetNonUndoableDirty(eSaveDirty);
         if (m_drawingOrderSelect)
         {
            ISelect *psel = pt->m_vmultisel.ElementAt(idx);
            pt->m_vmultisel.RemoveElementAt(idx);

            pt->m_vmultisel.InsertElementAt(psel, idx - 1);

            for (int i = pt->m_vmultisel.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
               int t = pt->m_vedit.IndexOf(pedit);
               pt->m_vedit.RemoveElementAt(t);
            }

            for (int i = pt->m_vmultisel.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
               pt->m_vedit.AddElement(pedit);
            }
         }
         else
         {
            ISelect *psel = pt->m_allHitElements.ElementAt(idx);
            pt->m_allHitElements.RemoveElementAt(idx);

            pt->m_allHitElements.InsertElementAt(psel, idx - 1);

            for (int i = pt->m_allHitElements.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
               int t = pt->m_vedit.IndexOf(pedit);
               pt->m_vedit.RemoveElementAt(t);
            }

            for (int i = pt->m_allHitElements.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
               pt->m_vedit.AddElement(pedit);
            }
         }
      }
   }
   else
   {
      pt->SetNonUndoableDirty(eSaveDirty);
      if (m_drawingOrderSelect)
      {
         if (idx < pt->m_vmultisel.Size() - 1)
         {
            ListView_GetItemText(hOrderList, idx, 0, text0, 256);
            ListView_GetItemText(hOrderList, idx, 1, text1, 256);
            ListView_GetItemText(hOrderList, idx, 2, text2, 256);
            ListView_DeleteItem(hOrderList, idx);
            lv.iItem = idx + 1;
            lv.iSubItem = 0;
            lv.pszText = text0;
            ListView_InsertItem(hOrderList, &lv);
            ListView_SetItemText(hOrderList, idx + 1, 1, text1);
            ListView_SetItemText(hOrderList, idx + 1, 2, text2);
            ListView_SetItemState(hOrderList, -1, 0, LVIS_SELECTED);
            ListView_SetItemState(hOrderList, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
            ListView_SetItemState(hOrderList, idx + 1, LVIS_FOCUSED, LVIS_FOCUSED);
            ::SetFocus(hOrderList);
            ISelect *psel = pt->m_vmultisel.ElementAt(idx);
            pt->m_vmultisel.RemoveElementAt(idx);

            if (idx + 1 >= pt->m_vmultisel.Size())
               pt->m_vmultisel.AddElement(psel);
            else
               pt->m_vmultisel.InsertElementAt(psel, idx + 1);

            for (int i = pt->m_vmultisel.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
               int t = pt->m_vedit.IndexOf(pedit);
               pt->m_vedit.RemoveElementAt(t);
            }

            for (int i = pt->m_vmultisel.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
               pt->m_vedit.AddElement(pedit);
            }
         }
      }
      else
      {
         if (idx < pt->m_allHitElements.Size() - 1)
         {
            ListView_GetItemText(hOrderList, idx, 0, text0, 256);
            ListView_GetItemText(hOrderList, idx, 1, text1, 256);
            ListView_GetItemText(hOrderList, idx, 2, text2, 256);
            ListView_DeleteItem(hOrderList, idx);
            lv.iItem = idx + 1;
            lv.iSubItem = 0;
            lv.pszText = text0;
            ListView_InsertItem(hOrderList, &lv);
            ListView_SetItemText(hOrderList, idx + 1, 1, text1);
            ListView_SetItemText(hOrderList, idx + 1, 2, text2);
            ListView_SetItemState(hOrderList, -1, 0, LVIS_SELECTED);
            ListView_SetItemState(hOrderList, idx + 1, LVIS_SELECTED, LVIS_SELECTED);
            ListView_SetItemState(hOrderList, idx + 1, LVIS_FOCUSED, LVIS_FOCUSED);
            ::SetFocus(hOrderList);

            ISelect *psel = pt->m_allHitElements.ElementAt(idx);
            pt->m_allHitElements.RemoveElementAt(idx);

            if (idx + 1 >= pt->m_allHitElements.Size())
               pt->m_allHitElements.AddElement(psel);
            else
               pt->m_allHitElements.InsertElementAt(psel, idx + 1);

            for (int i = pt->m_allHitElements.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
               int t = pt->m_vedit.IndexOf(pedit);
               pt->m_vedit.RemoveElementAt(t);
            }

            for (int i = pt->m_allHitElements.Size() - 1; i >= 0; i--)
            {
               IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
               pt->m_vedit.AddElement(pedit);
            }
         }
      }
   }
}
