#ifndef H_VIDEO_OPTIONS_DIALOG
#define  H_VIDEO_OPTIONS_DIALOG

class VideoOptionsDialog : public CDialog
{
public:
   VideoOptionsDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnClose();

private:
   void AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
   void ResetVideoPreferences(const unsigned int profile); // 0 = default, 1 = lowend PC, 2 = highend PC
   void UpdateFullscreenModesList();
   void UpdateDisplayHeightFromWidth();

   vector<VideoMode> m_allVideoModes;
   int m_initialMaxTexDim;
};

#endif 
