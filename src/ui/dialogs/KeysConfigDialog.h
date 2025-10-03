// license:GPLv3+

#pragma once

class KeysConfigDialog final : public CDialog
{
public:
    KeysConfigDialog();

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;

private:
   void AddToolTip(char* text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd) const;
   void AddStringDOF(const string& name, const int idc) const;
   void AddStringAxis(const string& name, const int idc, const int def) const;
   HWND GetItemHwnd(int nID);
   void SetValue(int nID, const Settings::Section& section, const string& key);
};
