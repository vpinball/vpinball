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
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    CResizer m_resizer;
};

#endif H_TOOLBAR_DIALOG
