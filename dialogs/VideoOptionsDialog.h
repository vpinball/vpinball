#ifndef H_VIDEO_OPTIONS_DIALOG
#define H_VIDEO_OPTIONS_DIALOG

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
  void AddToolTip(const char* const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
  void ResetVideoPreferences(
      const unsigned int profile); // 0 = default, 1 = lowend PC, 2 = highend PC
  void FillVideoModesList(const std::vector<VideoMode>& modes,
                          const VideoMode* curSelMode = nullptr);

  std::vector<VideoMode> allVideoModes;
};

#endif
