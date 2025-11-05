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
   void AddStringDOF(const string& name, const int idc) const;
   void SetValue(int nID, const string& key);
};
