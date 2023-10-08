#ifndef H_KEYS_CONFIG_DIALOG
#define H_KEYS_CONFIG_DIALOG

class KeysConfigDialog :public CDialog
{
public:
    KeysConfigDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnOK();
    virtual void OnDestroy();

private:
    void AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
    HWND GetItemHwnd(int nID);
    void SetValue(int nID, const Settings::Section& section, const string& key);
    void StartTimer(int nID);
};
#endif
