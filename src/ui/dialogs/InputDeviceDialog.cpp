// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
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

   m_deviceItems.insert(std::pair<string, HTREEITEM>(name, hCurrentDeviceItem));

   return hCurrentDeviceItem != nullptr;
}

string DeviceTreeView::GetCurrentDeviceName() const { return string(GetItemText(hCurrentDeviceItem)); }

void DeviceTreeView::SetAllItemStates(const bool checked) 
{ 
   TVITEM tvItem = {};
   tvItem.mask = TVIF_PARAM;

   vector<HTREEITEM> children = GetAllDeviceItems();
   for (const HTREEITEM child : children)
   {
      TreeView_SetCheckState(GetHwnd(), child, checked);
      HTREEITEM subItem = GetChild(child);
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
}

void DeviceTreeView::DeleteAll() { }
void DeviceTreeView::SetActiveDevice(const string& name) { }

bool DeviceTreeView::IsDeviceChecked(const string& name)
{
   auto itemIt = m_deviceItems.find(name);
   HTREEITEM item = itemIt->second;
   return IsItemChecked(item);
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
   return ((tvItem.state >> 12) - 1) != 0;
}

void DeviceTreeView::SetItemCheck(HTREEITEM item, bool checked)
{
   TreeView_SetCheckState(GetHwnd(), item, checked ? 1 : 0);
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
         //string checkName = string(GetItemText(ht.hItem));
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
         hCurrentDeviceItem = tvItem.hItem;
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

InputDeviceInfo InputDeviceDialog::LoadDevicePrefs(int index)
{
   static const string kDefaultName = "None"s;

   InputDeviceInfo info;

   const string deviceNameKey = "Device" + std::to_string(index) + "_Name";
   info.name = g_pvp->m_settings.LoadValueWithDefault(Settings::ControllerDevices, deviceNameKey, kDefaultName);

   const string deviceStateKey = "Device" + std::to_string(index) + "_State";
   info.state = g_pvp->m_settings.LoadValueWithDefault(Settings::ControllerDevices, deviceStateKey, true);

   return info;
}

void InputDeviceDialog::SaveDevicePrefs()
{
   const size_t numDevices = m_attachedDeviceInfo.size();
   for (size_t i = 0; i < numDevices; i++)
   {
      InputDeviceInfo info = m_attachedDeviceInfo[i];

      const string deviceNameKey = "Device" + std::to_string(i) + "_Name";
      g_pvp->m_settings.SaveValue(Settings::ControllerDevices, deviceNameKey, info.name);

      const string deviceStateKey = "Device" + std::to_string(i) + "_State";
      g_pvp->m_settings.SaveValue(Settings::ControllerDevices, deviceStateKey, info.state);
   }

   g_pvp->m_settings.Save();
}

void InputDeviceDialog::LoadAndReconcileInputDevicePrefs()
{ 
   // First, get any saved device prefs
   vector<InputDeviceInfo> deviceInfoPrefs;

   for (int i = 0; i < PININ_JOYMXCNT; i++)
   {
       deviceInfoPrefs.push_back(LoadDevicePrefs(i));
   }

   // Build the list of currently-attached devices
   m_attachedDeviceInfo.clear();

   for (int i = 0; i < PININ_JOYMXCNT; i++)
   {
      #ifdef USE_DINPUT8
      LPDIRECTINPUTDEVICE8 joystick = pinInput->GetJoystick(i);
      #else
      LPDIRECTINPUTDEVICE joystick = pinInput->GetJoystick(i);
      #endif

      if (joystick != nullptr)
      {
         DIDEVICEINSTANCE deviceInfo;
         deviceInfo.dwSize = sizeof(DIDEVICEINSTANCE);
         joystick->GetDeviceInfo(&deviceInfo);

         InputDeviceInfo info;
         info.name = deviceInfo.tszInstanceName;
         info.state = true;
         m_attachedDeviceInfo.push_back(info);
      }
   }

   // See if any attached devices have a corresponding saved preference
   const size_t numDevices = m_attachedDeviceInfo.size();
   for (size_t i = 0; i < numDevices; i++)
   {
      const string lookForName = m_attachedDeviceInfo[i].name;
      for (int j = 0; j < PININ_JOYMXCNT; j++)
      {
         const InputDeviceInfo& testInfo = deviceInfoPrefs[j];
         if (lookForName == testInfo.name)
         {
            m_attachedDeviceInfo[i].state = testInfo.state;
            break;
         }
      }
   }

   // Finally, add reconciled devices to the dialog
   for (size_t i = 0; i < numDevices; i++)
   {
      AddDevice(m_attachedDeviceInfo[i].name, m_attachedDeviceInfo[i].state);
   }
}

BOOL InputDeviceDialog::OnInitDialog()
{
   CRect myPos = GetWindowRect();
   startPos->bottom = startPos->top + (myPos.bottom - myPos.top);
   startPos->right = startPos->left + (myPos.right - myPos.left);
   MoveWindow(*startPos);

   AttachItem(IDC_TREE_DEVICES, m_deviceTreeView);

   LoadAndReconcileInputDevicePrefs();

   m_deviceTreeView.ExpandRoot();

   return TRUE;
}

bool InputDeviceDialog::AddDevice(const string& name, bool checked)
{
   bool success = m_deviceTreeView.AddDevice(name, checked);
   if (success)
   {
      m_deviceTreeView.InvalidateRect();
   }

   return success;
}

void InputDeviceDialog::UpdateDeviceStates()
{
   const size_t numDevices = m_attachedDeviceInfo.size();
   for (size_t i = 0; i < numDevices; i++)
   {
      m_attachedDeviceInfo[i].state = m_deviceTreeView.IsDeviceChecked(m_attachedDeviceInfo[i].name);
   }
}

void InputDeviceDialog::OnOK()
{
   UpdateDeviceStates();
   SaveDevicePrefs();
   CDialog::OnOK();
}
