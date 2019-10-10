#ifndef H_TOOLBAR_DIALOG
#define H_TOOLBAR_DIALOG

class ToolbarDialog : public CDialog
{
public:
    ToolbarDialog();
    virtual ~ToolbarDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual void OnDestroy();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
};

#endif H_TOOLBAR_DIALOG
