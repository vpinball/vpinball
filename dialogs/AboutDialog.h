#ifndef H_ABOUT_DIALOG
#define H_ABOUT_DIALOG

class AboutDialog : public CDialog
{
public:
   AboutDialog();
   virtual ~AboutDialog();

protected:
   virtual void OnDestroy() override;
   virtual BOOL OnInitDialog() override;
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   virtual void OnOK() override;

   string m_urlString;
};

#endif