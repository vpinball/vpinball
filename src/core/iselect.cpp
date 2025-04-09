// license:GPLv3+

#include "core/stdafx.h"

ISelect::ISelect()
   : m_vpinball(g_pvp)
{
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
      MoveOffset((float)(x - m_ptLast.x)*inv_zoom, (float)(y - m_ptLast.y)*inv_zoom);

      ptable->SetDirtyDraw();

      m_ptLast.x = x;
      m_ptLast.y = y;
      SetObjectPos();
   }
}

void ISelect::DoCommand(int icmd, int x, int y)
{
#ifndef __STANDALONE__
   // Commands that are handled by the table element
   if (  ((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020) // Assign to collection
      || ((icmd >= ID_ASSIGN_TO_LAYER1) && (icmd <= ID_ASSIGN_TO_LAYER1+NUM_ASSIGN_LAYERS-1)) // Assign to layer
      || (icmd == ID_EDIT_DRAWINGORDER_HIT)
      || (icmd == ID_EDIT_DRAWINGORDER_SELECT)
      || (icmd == ID_ASSIGN_TO_CURRENT_LAYER)
      || (icmd == IDC_COPY)
      || (icmd == IDC_PASTE)
      || (icmd == IDC_PASTEAT))
   {
      GetPTable()->DoCommand(icmd, x, y);
      return;
   }

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
   switch (icmd)
   {
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
   }
#endif
}

void ISelect::SetSelectFormat(Sur *psur)
{
   const COLORREF color = m_locked ? m_vpinball->m_elemSelectLockedColor
                                   : m_vpinball->m_elemSelectColor;//GetSysColor(COLOR_HIGHLIGHT);

   psur->SetBorderColor(color, false, 4);
   psur->SetLineColor(color, false, 4);
}

void ISelect::SetMultiSelectFormat(Sur *psur)
{
   const COLORREF color = m_locked ? m_vpinball->m_elemSelectLockedColor :
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

HRESULT ISelect::GetTypeName(BSTR *pVal) const
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
#else
   const LocalStringW wzString(strID);
   wcscpy(buf, wzString.m_szbuffer);
#endif
}

static void SetPartGroup(ISelect* const me, const string& layerName)
{
   if (me->GetIEditable() && (me->GetItemType() != eItemDragPoint) && (me->GetItemType() != eItemLightCenter))
   {
      if (me->GetIEditable()->GetPartGroup())
      {
         PartGroup* legacyPartGroup = me->GetIEditable()->GetPartGroup();
         me->GetIEditable()->SetPartGroup(nullptr);
         auto users = std::ranges::find_if(me->GetPTable()->m_vedit, [legacyPartGroup](IEditable *editable) { return editable->GetPartGroup() == legacyPartGroup; });
         if (users == me->GetPTable()->m_vedit.end())
            legacyPartGroup->GetISelect()->Uncreate();
      }
      auto partGroupF = std::ranges::find_if(me->GetPTable()->m_vedit,
         [layerName](IEditable *editable)
         {
            return (editable->GetItemType() == ItemTypeEnum::eItemPartGroup) && editable->GetName() == layerName;
         });
      if (partGroupF == me->GetPTable()->m_vedit.end())
      {
         PartGroup *const newGroup = static_cast<PartGroup *>(EditableRegistry::CreateAndInit(eItemPartGroup, me->GetPTable(), 0, 0));
         const int len = (int)(sizeof(newGroup->GetScriptable()->m_wzName)/sizeof(newGroup->GetScriptable()->m_wzName[0]));
         WCHAR newName[len];
         MultiByteToWideCharNull(CP_ACP, 0, layerName.c_str(), -1, newName, len);
         me->GetPTable()->m_pcv->ReplaceName(newGroup->GetIEditable()->GetScriptable(), newName);
         lstrcpynW(newGroup->GetScriptable()->m_wzName, newName, len);
         me->GetPTable()->m_vedit.push_back(newGroup);
         me->GetIEditable()->SetPartGroup(newGroup);
      }
      else
      {
         me->GetIEditable()->SetPartGroup(static_cast<PartGroup *>(*partGroupF));
      }
   }
}

bool ISelect::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
      case FID(LOCK): pbr->GetBool(m_locked); break;
      case FID(LVIS): pbr->GetBool(m_isVisible); break;
      case FID(LAYR): // Old layer style (limited number of unnamed layers)
      {
         int layerIndex;
         pbr->GetInt(layerIndex);
         SetPartGroup(this, (layerIndex < 9 ? "Layer_0" : "Layer_") + std::to_string(layerIndex + 1));
         break;
      }
      case FID(LANR): // 10.7 layers (limited number of named layers)
      {
         string layerName;
         pbr->GetString(layerName);
         std::ranges::replace(layerName, ' ', '_');
         SetPartGroup(this, "Layer_" + layerName);
         break;
      }
      case FID(GRUP): // 10.8.1 groups (unlimited number of hierarchical parenting with properties)
      {
         string partGroupName;
         pbr->GetString(partGroupName);
         SetPartGroup(this, partGroupName);
         break;
      }
   }
   return true;
}

HRESULT ISelect::SaveData(IStream *pstm, HCRYPTHASH hcrypthash)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteBool(FID(LOCK), m_locked);
   if (GetIEditable() && (GetItemType() != eItemDragPoint) && (GetItemType() != eItemLightCenter) && GetIEditable()->GetPartGroup())
   {
      // Implement backward 'readability' (file will open in previous versions, with unsupported content dropped)
      const PartGroup* layer = GetIEditable()->GetPartGroup();
      while (layer->GetPartGroup() != nullptr)
         layer = layer->GetPartGroup();
      int index = 0;
      for (const auto edit : GetPTable()->m_vedit)
      {
         if (edit == layer)
            break;
         if (edit->GetItemType() == eItemPartGroup && edit->GetPartGroup() == nullptr)
            index++;
      }
      bw.WriteInt(FID(LAYR), min(index, 11));
      bw.WriteString(FID(LANR), layer->GetName());
      bw.WriteString(FID(GRUP), GetIEditable()->GetPartGroup()->GetName());
   }
   bw.WriteBool(FID(LVIS), m_isVisible);
   
   return S_OK;
}

void ISelect::UpdateStatusBarInfo()
{
   m_vpinball->SetStatusBarUnitInfo(string(), false);
}
