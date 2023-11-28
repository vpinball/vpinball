/*
 * Portions of this code was derived from MFC reference material:
 *
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cmenu-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cwnd-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cstatic-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cbutton-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/ccombobox-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class
 */

extern "C" void external_log_info(const char* format, ...);
extern "C" void external_log_error(const char* format, ...);

// 560
class CDC { };

// 1135 
class CMenu { };

// 2181
class CWnd {
public:
   virtual void PreRegisterClass(WNDCLASS& wc) { };
   virtual void PreCreate(CREATESTRUCT& cs) { }
   virtual int OnCreate(CREATESTRUCT& cs) { return 0; };
   virtual void OnClose() { };
   virtual LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam) { return 0; };
   virtual BOOL OnEraseBkgnd(CDC& dc) { return false; };
   virtual void OnInitialUpdate() { };
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; };
   virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) { return false; };
   virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) { return 0; };
   virtual void Destroy() { };

   BOOL Create(CWnd* pParentWnd) { return true; }

   int MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) 
   {
      string caption = lpCaption;
      std::transform(caption.begin(), caption.end(), caption.begin(), ::tolower);

      if (caption.find("error") == string::npos)
         external_log_info("MessageBox(): caption=%s, text=%s", lpCaption, lpText);
      else
         external_log_error("MessageBox(): caption=%s, text=%s", lpCaption, lpText);

      return 0;
   }

   HWND GetHwnd() { return 0L; }
};

// 3323
class CDialog : public CWnd {
public:
   CDialog(int id){};

protected:
   virtual BOOL OnInitDialog() { return true; };
   virtual void OnOK() { };
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) { return false; };
   virtual void OnClose() { };
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {return 0; };
   virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) { return 0; };
};

// 3431
class CStatic : public CWnd { };

// 3462
class CButton : public CWnd { };

// 3718
class CComboBox : public CWnd { };

// 3821
class CEdit : public CWnd { };

// 5135
class CWinApp {
public:
   virtual BOOL InitInstance() = 0;
   virtual int Run() = 0;
};


