#ifndef H_RENDERPROBE_DIALOG
#define H_RENDERPROBE_DIALOG

class RenderProbeDialog : public CDialog
{
public:
   RenderProbeDialog();

protected:
   BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
   void UpdateList();

   HWND hListHwnd;
};

#endif
