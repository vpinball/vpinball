#ifndef H_INPUT_DEVICE_DIALOG
#define H_INPUT_DEVICE_DIALOG

class DeviceTreeView : public CTreeView
{
public:
   DeviceTreeView();
   ~DeviceTreeView() { }

   void ExpandRoot();
   virtual HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR text, IEditable* const pedit, int image);
   bool AddDevice(const string& name, bool checked);
   bool AddElement(const string& name, IEditable* const pedit);
   bool AddElementToDevice(const HTREEITEM hDeviceItem, const string& name, IEditable* const pedit);
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
   bool PreTranslateMessage(MSG* msg);
   vector<string> GetAllDeviceNames();


protected:
   virtual void OnAttach();
   virtual void PreCreate(CREATESTRUCT& cs);
   virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

   virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
   virtual LRESULT OnNMClick(LPNMHDR lpnmh);
   virtual LRESULT OnTVNSelChanged(LPNMTREEVIEW pNMTV);

private:
   vector<HTREEITEM> GetSubItems(HTREEITEM hParent);
   int GetSubItemsCount(HTREEITEM hParent) const;

	
   HTREEITEM hRootItem;
   HTREEITEM hCurrentDeviceItem;
   HTREEITEM hCurrentElementItem;
   CImageList m_normalImages;
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
   void OnOK() override;
   bool AddDevice(const string& name, bool checked);

private:
   CRect* startPos;
   DeviceTreeView m_deviceTreeView;
   PinInput* pinInput;
};

#endif