// license:GPLv3+

#pragma once

class PhysicsOptionsDialog final : public CDialog
{
public:
    PhysicsOptionsDialog();

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnOK() override;
    void OnDestroy() override;

private:
    bool LoadSetting();
    CString GetItemText(int id);
    void SetItemText(int id, float value);
    void SaveCurrentPhysicsSetting();
};
