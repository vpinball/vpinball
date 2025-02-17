// license:GPLv3+

#pragma once

class DeviceTreeView : public CTreeView
{
public:
   DeviceTreeView();
   ~DeviceTreeView() { }

   void ExpandRoot();
   virtual HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image);
   bool AddDevice(const string& name, bool checked);
   string GetCurrentDeviceName() const;
   bool IsItemChecked(HTREEITEM hItem) const;
   void SetItemCheck(HTREEITEM item, bool checked);
   void SetAllItemStates(const bool checked);
   void DeleteAll();
   void SetActiveDevice(const string& name);
   HTREEITEM GetRootItem() { return hRootItem; }
   HTREEITEM GetCurrentDeviceItem() { return hCurrentDeviceItem; }
   HTREEITEM GetFirstDevice() { return GetChild(hRootItem); }
   HTREEITEM GetDeviceByItem(HTREEITEM hChildItem);
   vector<HTREEITEM> GetAllDeviceItems() const;
   string GetDeviceName(HTREEITEM item) { return string { GetItemText(item) }; }
   bool IsDeviceChecked(const string& name);

protected:
   virtual void OnAttach();
   virtual void PreCreate(CREATESTRUCT& cs);
   virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

   virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
   virtual LRESULT OnNMClick(LPNMHDR lpnmh);
   virtual LRESULT OnTVNSelChanged(LPNMTREEVIEW pNMTV);

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

class InputDeviceDialog : public CDialog
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
