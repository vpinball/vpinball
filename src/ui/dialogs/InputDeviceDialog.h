#ifndef H_INPUT_DEVICES_DIALOG
#define H_INPUT_DEVICES_DIALOG

class InputDeviceDialog : public CDialog
{
public:
   InputDeviceDialog(CRect* sourcePos);
   ~InputDeviceDialog() override;

protected:
   void OnDestroy() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

   string m_urlString;
   CRect* startPos;
};

#endif