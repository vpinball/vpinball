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

private:
   bool LoadSetting();
   string GetItemText(int id) const;
   void SetItemText(int id, float value);
   void SaveCurrentPhysicsSetting();
};
