#ifndef H_AUDIO_OPTIONS_DIALOG
#define H_AUDIO_OPTIONS_DIALOG

class AudioOptionsDialog : public CDialog
{
public:
   AudioOptionsDialog();

protected:
   virtual BOOL OnInitDialog() override;
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   virtual void OnOK() override;
   virtual void OnClose() override;
};

#endif // !H_AUDIO_OPTIONS_DIALOG
