#include "stdafx.h"


ISelect::ISelect()
{
   m_dragging = false;
   m_markedForUndo = false;
   m_selectstate = eNotSelected;

   m_locked = false;

   m_menuid = -1;
   m_oldLayerIndex = 0;
   m_isVisible = true;
   m_vpinball = g_pvp;
}

void ISelect::SetObjectPos()
{
    m_vpinball->ClearObjectPosCur();
}

void ISelect::OnLButtonDown(int x, int y)
{
   m_dragging = true;
   m_markedForUndo = false; // So we will be marked when and if we are dragged
   m_ptLast.x = x;
   m_ptLast.y = y;

   GetPTable()->SetMouseCapture();

   SetObjectPos();
}

void ISelect::OnLButtonUp(int x, int y)
{
   m_dragging = false;

#ifndef __STANDALONE__
   ReleaseCapture();
#endif

   if (m_markedForUndo)
   {
      m_markedForUndo = false;
      STOPUNDOSELECT
   }
}

void ISelect::OnRButtonDown(int x, int y, HWND hwnd)
{
}

void ISelect::OnRButtonUp(int x, int y)
{
}

void ISelect::OnMouseMove(int x, int y)
{
   if ((x == m_ptLast.x) && (y == m_ptLast.y))
      return;

   if (m_dragging && !GetIEditable()->GetISelect()->m_locked) // For drag points, follow the lock of the parent
   {
      PinTable * const ptable = GetPTable();
      const float inv_zoom = 1.0f / ptable->m_zoom;

      if (!m_markedForUndo)
      {
         m_markedForUndo = true;
         STARTUNDOSELECT
      }
      MoveOffset((x - m_ptLast.x)*inv_zoom, (y - m_ptLast.y)*inv_zoom);

      ptable->SetDirtyDraw();

      m_ptLast.x = x;
      m_ptLast.y = y;
      SetObjectPos();
   }
}

void ISelect::MoveOffset(const float dx, const float dy)
{
   // Implement in child class to enable dragging
}

void ISelect::EditMenu(CMenu &menu)
{
}

void ISelect::DoCommand(int icmd, int x, int y)
{
#ifndef __STANDALONE__
   IEditable * const piedit = GetIEditable();

   if ((icmd & 0x0000FFFF) == ID_SELECT_ELEMENT)
   {
      const int ksshift = GetKeyState(VK_SHIFT);
      //const int ksctrl = GetKeyState(VK_CONTROL);

      PinTable * const currentTable = GetPTable();
      const int i = (icmd & 0x00FF0000) >> 16;
      ISelect * const pisel = currentTable->m_allHitElements[i];

      const bool add = ((ksshift & 0x80000000) != 0);

      if (pisel == (ISelect *)currentTable && add)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         currentTable->OnLButtonDown(x, y); // Start the band select
         return;
      }

      currentTable->AddMultiSel(pisel, add, true, true);
      return;
   }
   if (((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020))
   {
      /*add to collection*/
      //const int ksshift = GetKeyState(VK_SHIFT);
      //const int ksctrl = GetKeyState(VK_CONTROL);

      PinTable * const currentTable = GetPTable();
      const int i = icmd & 0x000000FF;
      currentTable->UpdateCollection(i);
   }
   if ((icmd >= ID_ASSIGN_TO_LAYER1) && (icmd <= ID_ASSIGN_TO_LAYER1+NUM_ASSIGN_LAYERS-1))
   {
      /*add to layer*/
      m_vpinball->GetLayersListDialog()->AssignToLayerByIndex(icmd - ID_ASSIGN_TO_LAYER1);
   }
   switch (icmd)
   {
   case ID_EDIT_DRAWINGORDER_HIT:
       m_vpinball->ShowDrawingOrderDialog(false);
      break;
   case ID_EDIT_DRAWINGORDER_SELECT:
       m_vpinball->ShowDrawingOrderDialog(true);
      break;
   case ID_ASSIGN_TO_CURRENT_LAYER:
   {
       m_vpinball->GetLayersListDialog()->OnAssignButton();
       break;
   }
   case ID_DRAWINFRONT:
   {
      PinTable * const ptable = GetPTable();
      RemoveFromVectorSingle(ptable->m_vedit, piedit);
      ptable->m_vedit.push_back(piedit);
      ptable->SetDirtyDraw();
      break;
   }
   case ID_DRAWINBACK:
   {
      PinTable * const ptable = GetPTable();
      RemoveFromVectorSingle(ptable->m_vedit, piedit);
      ptable->m_vedit.insert(ptable->m_vedit.begin(), piedit);
      ptable->SetDirtyDraw();
      break;
   }
   case ID_SETASDEFAULT:
      piedit->WriteRegDefaults();
      break;
   case ID_LOCK:
      STARTUNDOSELECT
      m_locked = !m_locked;
      STOPUNDOSELECT
      break;

   case IDC_COPY:
   {
      GetPTable()->Copy(x,y);
      break;
   }
   case IDC_PASTE:
   {
      GetPTable()->Paste(false, x, y);
      break;
   }
   case IDC_PASTEAT:
   {
      GetPTable()->Paste(true, x, y);
      break;
   }
   /*default:
      psel->DoCommand(command, x, y);
      break;*/
   }
#endif
}

#define COLOR_LOCKED RGB(160,160,160)
//GetSysColor(COLOR_GRAYTEXT)

void ISelect::SetSelectFormat(Sur *psur)
{
   const DWORD color = m_locked ? m_vpinball->m_elemSelectLockedColor
                                : m_vpinball->m_elemSelectColor;//GetSysColor(COLOR_HIGHLIGHT);

   psur->SetBorderColor(color, false, 4);
   psur->SetLineColor(color, false, 4);
}

void ISelect::SetMultiSelectFormat(Sur *psur)
{
   const DWORD color = m_locked ?
       m_vpinball->m_elemSelectLockedColor :
       m_vpinball->m_elemSelectColor;//GetSysColor(COLOR_HIGHLIGHT);

   psur->SetBorderColor(color, false, 3);
   psur->SetLineColor(color, false, 3);
}

void ISelect::SetLockedFormat(Sur *psur)
{
   psur->SetBorderColor(m_vpinball->m_elemSelectLockedColor, false, 1);
   psur->SetLineColor(m_vpinball->m_elemSelectLockedColor, false, 1);
}

void ISelect::FlipY(const Vertex2D& pvCenter)
{
   GetIEditable()->MarkForUndo(); // Start/EndUndo cycle is around the loop that calls this

   Vertex2D vCenter = GetCenter();
   const float delta = vCenter.y - pvCenter.y;
   vCenter.y -= delta * 2.f;
   PutCenter(vCenter);
}

void ISelect::FlipX(const Vertex2D& pvCenter)
{
   GetIEditable()->MarkForUndo(); // Start/EndUndo cycle is around the loop that calls this

   Vertex2D vCenter = GetCenter();
   const float delta = vCenter.x - pvCenter.x;
   vCenter.x -= delta * 2.f;
   PutCenter(vCenter);
}

void ISelect::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   GetIEditable()->MarkForUndo(); // Start/EndUndo cycle is around the loop that calls this

   Vertex2D vCenter = GetCenter();

   const float sn = sinf(ANGTORAD(ang));
   const float cs = cosf(ANGTORAD(ang));

   const float dx = vCenter.x - pvCenter.x;
   const float dy = vCenter.y - pvCenter.y;

   vCenter.x = pvCenter.x + cs*dx - sn*dy;
   vCenter.y = pvCenter.y + cs*dy + sn*dx;
   PutCenter(vCenter);
}

void ISelect::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   GetIEditable()->MarkForUndo(); // Start/EndUndo cycle is around the loop that calls this

   Vertex2D vCenter = GetCenter();

   const float dx = vCenter.x - pvCenter.x;
   const float dy = vCenter.y - pvCenter.y;

   vCenter.x = pvCenter.x + dx*scalex;
   vCenter.y = pvCenter.y + dy*scaley;
   PutCenter(vCenter);
}

void ISelect::Translate(const Vertex2D &pvOffset)
{
   GetIEditable()->MarkForUndo(); // Start/EndUndo cycle is around the loop that calls this

   Vertex2D vCenter = GetCenter();

   vCenter.x += pvOffset.x;
   vCenter.y += pvOffset.y;
   PutCenter(vCenter);
}

HRESULT ISelect::GetTypeName(BSTR *pVal)
{
   WCHAR buf[256];
   GetTypeNameForType(GetItemType(), buf);
   *pVal = SysAllocString(buf);
   return S_OK;
}

void ISelect::GetTypeNameForType(const ItemTypeEnum type, WCHAR * const buf) const
{
   UINT strID;

   switch (type)
   {
   case eItemTable:        strID = IDS_TABLE; break;
   case eItemLightCenter:  strID = IDS_TB_LIGHT; break;
   case eItemDragPoint:    strID = IDS_CONTROLPOINT; break;
   //case eItemLightSeqCenter: strID = IDS_TB_LIGHTSEQ; break;
   default:
      strID = EditableRegistry::GetTypeNameStringID(type); break;
   }

#ifndef __STANDALONE__
   /*const int len =*/ LoadStringW(m_vpinball->theInstance, strID, buf, 256);
#endif
}

bool ISelect::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
       case FID(LOCK): pbr->GetBool(m_locked); break;
       case FID(LAYR):
       {
          int tmp;
          pbr->GetInt(tmp);
          m_oldLayerIndex = (char)tmp;
          break;
       }
       case FID(LANR):
       {
           pbr->GetString(m_layerName);
           break;
       }
       case FID(LVIS):
       {
           pbr->GetBool(m_isVisible);
           break;
       }
   }
   return true;
}

HRESULT ISelect::SaveData(IStream *pstm, HCRYPTHASH hcrypthash)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteBool(FID(LOCK), m_locked);
   bw.WriteInt(FID(LAYR), m_oldLayerIndex);
   bw.WriteString(FID(LANR), m_layerName);
   bw.WriteBool(FID(LVIS), m_isVisible);

   return S_OK;
}

void ISelect::UpdateStatusBarInfo()
{
    m_vpinball->SetStatusBarUnitInfo(string(), false);
}
