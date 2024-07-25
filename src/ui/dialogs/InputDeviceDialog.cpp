#include "core/stdafx.h"
#include "InputDeviceDialog.h"

//-------------------------------
// DeviceTreeView implementation
//-------------------------------

DeviceTreeView::DeviceTreeView()
{
}

HTREEITEM DeviceTreeView::AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image)
{
   TVITEM tvi = {};
   tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
   tvi.iImage = image;
   tvi.iSelectedImage = image;
   tvi.pszText = const_cast<LPTSTR>(text);
   tvi.lParam = (LPARAM)pedit;

   TVINSERTSTRUCT tvis = {};
   tvis.hParent = hParent;
   tvis.item = tvi;

   const HTREEITEM item = InsertItem(tvis);
   return item;
}

bool DeviceTreeView::AddDevice(const string& name, bool checked)
{
   hCurrentDeviceItem = AddItem(hRootItem, name.c_str(), nullptr, 1);
   SetItemCheck(hCurrentDeviceItem, checked);
   return hCurrentDeviceItem != nullptr;
}

bool DeviceTreeView::AddElement(const string& name, IEditable* const pedit) 
{ 
    return AddElementToDevice(hCurrentDeviceItem, name, pedit); 
}

bool DeviceTreeView::AddElementToDevice(const HTREEITEM hDeviceItem, const string& name, IEditable* const pedit)
{
   hCurrentElementItem = AddItem(hDeviceItem, name.c_str(), pedit, 2);
   ISelect* const psel = pedit->GetISelect();
   if (psel != nullptr)
   {
      SetItemCheck(hCurrentElementItem, psel->m_isVisible);
      if (psel->m_isVisible)
         SetItemCheck(hDeviceItem, true);
   }
   return hCurrentElementItem != nullptr;
}

string DeviceTreeView::GetCurrentDeviceName() const { return ""; }
void DeviceTreeView::SetAllItemStates(const bool checked) { }
void DeviceTreeView::DeleteAll() { }
void DeviceTreeView::SetActiveDevice(const string& name) { }

bool DeviceTreeView::PreTranslateMessage(MSG* msg) { return TRUE; }

vector<string> DeviceTreeView::GetAllDeviceNames() 
{ 
   vector<HTREEITEM> children;
   HTREEITEM item = GetChild(hRootItem);

   while (item)
   {
      children.push_back(item);
      item = GetNextItem(item, TVGN_NEXT);
   }

   vector<string> deviceList;
   deviceList.reserve(children.size());

   for (const auto& device : children)
   {
      deviceList.push_back(GetItemText(device).c_str());
   }

   return deviceList;
}

void DeviceTreeView::OnAttach()
{
   m_normalImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
   const CBitmap bm(IDB_LAYER_ICONS16);
   m_normalImages.Add(bm, RGB(255, 255, 255));
   SetImageList(m_normalImages, LVSIL_NORMAL);

   DWORD style = GetStyle();
   style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_CHECKBOXES;
   SetStyle(style);

   DeleteAllItems();
   hRootItem = AddItem(nullptr, _T("Input Devices"), nullptr, 0);
   ExpandRoot();
}

void DeviceTreeView::ExpandRoot()
{ 
   Expand(hRootItem, TVE_EXPAND);
}

void DeviceTreeView::PreCreate(CREATESTRUCT& cs)
{
   cs.style = TVS_NOTOOLTIPS | WS_CHILD;
   cs.lpszClass = WC_TREEVIEW;
}

LRESULT DeviceTreeView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
   switch (msg)
   {
      case WM_MOUSEACTIVATE: SetFocus(); break;
   }

   return WndProcDefault(msg, wparam, lparam);
}

vector<HTREEITEM> DeviceTreeView::GetSubItems(HTREEITEM hParent)
{
   vector<HTREEITEM> allSubItems;
   HTREEITEM item = GetChild(hParent);
   while (item)
   {
      allSubItems.push_back(item);
      item = GetNextItem(item, TVGN_NEXT);
   }
   return allSubItems;
}

int DeviceTreeView::GetSubItemsCount(HTREEITEM hParent) const
{
   int count = 0;
   HTREEITEM item = GetChild(hParent);
   while (item)
   {
      count++;
      item = GetNextItem(item, TVGN_NEXT);
   }
   return count;
}

vector<HTREEITEM> DeviceTreeView::GetAllDeviceItems() const
{
   vector<HTREEITEM> children;
   HTREEITEM item = GetChild(hRootItem);
   while (item)
   {
      children.push_back(item);
      item = GetNextItem(item, TVGN_NEXT);
   }
   return children;
}

HTREEITEM DeviceTreeView::GetDeviceByItem(HTREEITEM hChildItem)
{
   for (const HTREEITEM child : GetAllDeviceItems())
   {
      if (hChildItem == child)
         return child;

      HTREEITEM subItem = GetChild(child);
      while (subItem)
      {
         if (hChildItem == subItem)
            return child;
         subItem = GetNextItem(subItem, TVGN_NEXT);
      }
   }

   return nullptr;
}

bool DeviceTreeView::IsItemChecked(HTREEITEM hItem) const
{
   TVITEM tvItem;
   tvItem.mask = TVIF_HANDLE | TVIF_STATE;
   tvItem.stateMask = TVIS_STATEIMAGEMASK;
   tvItem.hItem = hItem;
   GetItem(tvItem);
   return ((tvItem.state >> 12) - 1) == 0;
}

void DeviceTreeView::SetItemCheck(HTREEITEM item, bool checked)
{
   if (checked)
   {
      TreeView_SetCheckState(GetHwnd(), item, 1);
   }
   else
   {
      TreeView_SetCheckState(GetHwnd(), item, 0);
   }
}

LRESULT DeviceTreeView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
   const LPNMHDR lpnmh = (LPNMHDR)lparam;

   switch (lpnmh->code)
   {
       case TVN_SELCHANGED: return OnTVNSelChanged((LPNMTREEVIEW)lparam);
       case NM_CLICK: return OnNMClick(lpnmh);
   }

   return FALSE;
}

LRESULT DeviceTreeView::OnNMClick(LPNMHDR lpnmh)
{
   const DWORD dwpos = GetMessagePos();
   TVHITTESTINFO ht = {};
   ht.pt.x = GET_X_LPARAM(dwpos);
   ht.pt.y = GET_Y_LPARAM(dwpos);
   ::MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
   HitTest(ht);

   if (ht.flags & TVHT_ONITEMSTATEICON)
   {
      if (ht.hItem == hRootItem)
         SetAllItemStates(IsItemChecked(hRootItem));
      else
      {
         TVITEM tvItem = {};
         tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
         tvItem.hItem = ht.hItem;
         if (GetItem(tvItem))
         {
            if (tvItem.cChildren == 1) //!! <= ? // layer checkbox was clicked
            {
               const bool checked = IsItemChecked(tvItem.hItem);
               HTREEITEM subItem = GetChild(tvItem.hItem);
               while (subItem)
               {
                  tvItem.hItem = subItem;
                  if (GetItem(tvItem))
                  {
                     IEditable* const pedit = (IEditable*)tvItem.lParam;
                     if (pedit != nullptr)
                     {
                        ISelect* const psel = pedit->GetISelect();
                        if (psel != nullptr)
                           psel->m_isVisible = checked;
                     }
                  }

                  TreeView_SetCheckState(GetHwnd(), subItem, checked);
                  subItem = GetNextItem(subItem, TVGN_NEXT);
               }
            }
            else // element checkbox was clicked
            {
               IEditable* const pedit = (IEditable*)tvItem.lParam;
               if (pedit != nullptr)
               {
                  ISelect* const psel = pedit->GetISelect();
                  if (psel != nullptr)
                     psel->m_isVisible = IsItemChecked(tvItem.hItem);
               }
            }
         }
      }
   }

   return 0;
}

LRESULT DeviceTreeView::OnTVNSelChanged(LPNMTREEVIEW pNMTV)
{
   TVITEM tvItem = {};
   tvItem.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM;
   tvItem.hItem = pNMTV->itemNew.hItem;

   if (GetItem(tvItem))
   {
      if (tvItem.hItem != hRootItem)
      {
         if (GetSubItemsCount(tvItem.hItem) > 0)
            hCurrentDeviceItem = tvItem.hItem;
         else
         {
            hCurrentElementItem = tvItem.hItem;
            hCurrentDeviceItem = GetDeviceByItem(tvItem.hItem);
         }
      }
   }

   return 0;
}



//----------------------------------
// InputDeviceDialog implementation
//----------------------------------

InputDeviceDialog::InputDeviceDialog(CRect* sourcePos, PinInput* const pi) : CDialog(IDD_INPUT_DEVICES)
{
   startPos = new CRect(*sourcePos);
   pinInput = pi;
}

InputDeviceDialog::~InputDeviceDialog()
{
}

void InputDeviceDialog::OnDestroy()
{
}

INT_PTR InputDeviceDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   return DialogProcDefault(uMsg, wParam, lParam);
}

//BOOL InputDeviceDialog::OnCommand(WPARAM wParam, LPARAM lParam)
//{
//   UNREFERENCED_PARAMETER(lParam);
//   switch (LOWORD(wParam))
//   {
//      case IDC_WEBSITE:
//      case IDC_TRANSSITE:
//      {
//         return TRUE;
//      }
//   }
//
//   return FALSE;
//}

BOOL InputDeviceDialog::OnInitDialog()
{
   CRect myPos = GetWindowRect();
   startPos->bottom = startPos->top + (myPos.bottom - myPos.top);
   startPos->right = startPos->left + (myPos.right - myPos.left);
   MoveWindow(*startPos);

   AttachItem(IDC_TREE_DEVICES, m_deviceTreeView);

   LPDIRECTINPUTDEVICE joystick = nullptr;
   int joyIndex = 0;
   for (int i = 0; i < PININ_JOYMXCNT; i++)
   {
      joystick = pinInput->GetJoystick(joyIndex++);
      if (joystick != nullptr)
      {
         //CString joyName = joystick->GetDeviceInfo();
         const int strLen = 261;
         char nameStr[strLen];

         DIDEVICEINSTANCE deviceInfo;
         deviceInfo.dwSize = sizeof(DIDEVICEINSTANCE);

         joystick->GetDeviceInfo(&deviceInfo);
         std::snprintf(nameStr, strLen, "%s\n", deviceInfo.tszInstanceName);

         AddDevice(nameStr, true);
      }
   }

   m_deviceTreeView.ExpandRoot();

   return TRUE;
}

bool InputDeviceDialog::AddDevice(const string& name, bool checked)
{
   bool success = m_deviceTreeView.AddDevice(name, checked);

   if (success)
         m_deviceTreeView.InvalidateRect();

   return success;
}

void InputDeviceDialog::OnOK()
{
   CDialog::OnOK();
}
