#ifndef H_EDITOR_OPTIONS_DIALOG
#define H_EDITOR_OPTIONS_DIALOG

class EditorOptionsDialog : public CDialog
{
public:
    EditorOptionsDialog();

protected:
    virtual void OnClose();
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnOK();

private:
    void AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);

    CToolTip *m_toolTip;
};

#endif // !H_EDITOR_OPTIONS_DIALOG
