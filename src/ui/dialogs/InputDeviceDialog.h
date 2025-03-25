// license:GPLv3+

#pragma once

class DeviceTreeView final : public CTreeView
{
public:
   DeviceTreeView();
   ~DeviceTreeView() override { }

   void ExpandRoot();
   HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image);
   bool AddDevice(const string& name, bool checked);
   string GetCurrentDeviceName() const;
   bool IsItemChecked(HTREEITEM hItem) const;
   void SetItemCheck(HTREEITEM item, bool checked);
   void SetAllItemStates(const bool checked);
   void DeleteAll();
   void SetActiveDevice(const string& name);
   HTREEITEM GetRootItem() const { return hRootItem; }
   HTREEITEM GetCurrentDeviceItem() const { return hCurrentDeviceItem; }
   HTREEITEM GetFirstDevice() const { return GetChild(hRootItem); }
   HTREEITEM GetDeviceByItem(HTREEITEM hChildItem);
   vector<HTREEITEM> GetAllDeviceItems() const;
   string GetDeviceName(HTREEITEM item) const { return string { GetItemText(item) }; }
   bool IsDeviceChecked(const string& name);

protected:
   void OnAttach() override;
   void PreCreate(CREATESTRUCT& cs) override;
   LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
   LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam) override;

   LRESULT OnNMClick(LPNMHDR lpnmh);
   LRESULT OnTVNSelChanged(LPNMTREEVIEW pNMTV);

   vector<HTREEITEM> GetSubItems(HTREEITEM hParent);
   int GetSubItemsCount(HTREEITEM hParent) const;

private:
   HTREEITEM hRootItem;
   HTREEITEM hCurrentDeviceItem;
   CImageList m_normalImages;
   std::map<string, HTREEITEM> m_deviceItems;
};


struct InputDeviceInfo
{
   string name;
   bool state;
};

class InputDeviceDialog final : public CDialog
{
public:
   InputDeviceDialog(CRect* sourcePos, PinInput* const pi);
   ~InputDeviceDialog() override;

protected:
   void OnDestroy() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

   bool AddDevice(const string& name, bool checked);
   InputDeviceInfo LoadDevicePrefs(int index);
   void SaveDevicePrefs();
   void LoadAndReconcileInputDevicePrefs();
   void UpdateDeviceStates();
   void OnOK() override;

private:
   CRect* startPos;
   DeviceTreeView m_deviceTreeView;
   PinInput* pinInput;

   vector<InputDeviceInfo> m_attachedDeviceInfo;
};
