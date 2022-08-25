#ifndef H_AUDIO_OPTIONS_DIALOG
#define H_AUDIO_OPTIONS_DIALOG

class AudioOptionsDialog : public CDialog
{
public:
   AudioOptionsDialog();

protected:
   BOOL OnInitDialog() final;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) final;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) final;
   void OnOK() final;
   void OnClose() final;
};

#endif // !H_AUDIO_OPTIONS_DIALOG
