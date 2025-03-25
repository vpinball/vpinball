// license:GPLv3+

#pragma once

class KeysConfigDialog final : public CDialog
{
public:
    KeysConfigDialog();

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnOK() override;
    void OnDestroy() override;

private:
    void AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
    void AddStringDOF(const string& name, const int idc);
    void AddStringAxis(const string& name, const int idc, const int def);
    void AddJoyCustomKey(const string& name, const int idc, const int default_key);
    void AddWndProc(const int idc, const size_t MyKeyButtonProc, const size_t pksw);
    HWND GetItemHwnd(int nID);
    void SetValue(int nID, const Settings::Section& section, const string& key);
    void StartTimer(int nID);
};
