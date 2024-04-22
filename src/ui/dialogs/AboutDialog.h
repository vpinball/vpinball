#ifndef H_ABOUT_DIALOG
#define H_ABOUT_DIALOG

class AboutDialog : public CDialog
{
public:
   AboutDialog();
   ~AboutDialog() override;

protected:
   void OnDestroy() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

   string m_urlString;
};

#endif