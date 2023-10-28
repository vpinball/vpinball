#ifndef H_DIMENSION_MANAGER_DIALOG
#define H_DIMENSION_MANAGER_DIALOG

class DimensionDialog : public CDialog
{
public:
   DimensionDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);

private:
   void UpdateApplyState();

   CListView m_listView;
   struct ManufacturerDimensions
   {
      string name;
      float width;
      float height;
      float glassBottom;
      float glassTop;
      string comment;
   };
   vector<ManufacturerDimensions> m_dimensions;
   bool m_discardChangeNotification = false;
};

#endif
