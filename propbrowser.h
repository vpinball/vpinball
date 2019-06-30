#pragma once

#ifndef IMGERMAN
enum
{
   eSmartBrowserWidth = 200
};
#else
enum
{
   eSmartBrowserWidth = 160
};
#endif

#define COLOR_CHANGED WM_USER+100
#define FONT_CHANGED WM_USER+101
#define CHANGE_COLOR WM_USER+102
#define CHANGE_FONT WM_USER+103
#define GET_COLOR_TABLE WM_USER+104

INT_PTR CALLBACK PropertyProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct PropertyPane
{
   PropertyPane(const int dialog, const int title) : m_dialogid(dialog), m_titlestringid(title), m_ptemplate(NULL)
   {
   }

   PropertyPane(LPDLGTEMPLATE pdt, const int title) : m_dialogid(0), m_titlestringid(title), m_ptemplate(pdt)
   {
   }

   int m_dialogid;
   int m_titlestringid; // If NULL, this represents a non-expandable section
   LPDLGTEMPLATE m_ptemplate; // For custom controls
   HWND m_dialogHwnd;
};

class SmartBrowser
{
public:
   SmartBrowser();
   ~SmartBrowser();

   void CreateFromDispatch(HWND hwndParent, VectorProtected<ISelect> *pvsel);
   void GetControlValue(HWND hwndControl);
   HWND GetHWnd() const { return m_hwndFrame; }
   void Init(HWND hwndParent);
   void SetVisible(const bool visible);
   bool GetVisible() const;
   void DrawHeader(HDC hdc);
   void SetProperty(int dispid, VARIANT *pvar, const bool fPutRef);

   void PopulateDropdowns();
   void RefreshProperties();

   void LayoutExpandoWidth();
   void RelayoutExpandos();

   void FreePropPanes();
   void RemoveSelection();
   void ResetPriority(int expandoid);

   ISelect *GetBaseISel() { if (!m_pvsel || m_pvsel->Size() == 0) { return NULL; } return m_pvsel->ElementAt(0); }
   IDispatch *GetBaseIDisp() { if (!m_pvsel || m_pvsel->Size() == 0) { return NULL; } return m_pvsel->ElementAt(0)->GetDispatch(); }

   HWND m_hwndFrame;

   std::vector<HWND> m_vhwndExpand;
   std::vector<HWND> m_vhwndDialog;

   vector<PropertyPane*> m_vproppane;

   std::vector<int> m_vproppriority;

   HFONT m_hfontHeader;
   HFONT m_hfontHeader2;

   int m_olddialog;

   char m_szHeader[64];
   char m_szHeaderCollection[64];
   ISelect *m_prevSelection;

   VectorProtected<ISelect> *m_pvsel;
   int m_maxdialogwidth;
};

struct ExpandoInfo
{
   SmartBrowser *m_psb;
   int m_id;
   int m_dialogheight;
   bool m_expanded;
   bool m_hasCaption; // Also means 'can expand/collapse'
};

