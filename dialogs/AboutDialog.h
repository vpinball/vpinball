#ifndef H_ABOUT_DIALOG
#define H_ABOUT_DIALOG

class AboutDialog : public CDialog
{
public:
   AboutDialog();
   virtual ~AboutDialog();

protected:
   virtual void OnDestroy();
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();

};

#endif