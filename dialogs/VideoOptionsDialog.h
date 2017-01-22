#ifndef H_VIDEO_OPTIONS_DIALOG
#define  H_VIDEO_OPTIONS_DIALOG

class VideoOptionsDialog : public CDialog
{
public:
   VideoOptionsDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnClose();

private:
   void AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
   void ResetVideoPreferences();
   void FillVideoModesList(const std::vector<VideoMode>& modes, const VideoMode* curSelMode = 0);

   std::vector<VideoMode> allVideoModes;
};

#endif 
