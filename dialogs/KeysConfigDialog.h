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
    HWND GetItemHwnd(int nID);
    void SetValue(int nID, char *regKey, char *regValue);
    void StartTimer(int nID);
};
#endif 
