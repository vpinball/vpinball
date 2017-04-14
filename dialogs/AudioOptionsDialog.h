#ifndef H_AUDIO_OPTIONS_DIALOG
#define H_AUDIO_OPTIONS_DIALOG

class AudioOptionsDialog : public CDialog
{
public:
   AudioOptionsDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnClose();
};

#endif // !H_AUDIO_OPTIONS_DIALOG
