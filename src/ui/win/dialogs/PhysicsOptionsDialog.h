// license:GPLv3+

#pragma once

class PinTableWnd;

class PhysicsOptionsDialog final : public CDialog
{
public:
   explicit PhysicsOptionsDialog(PinTableWnd *tableEditor);

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

private:
   PinTableWnd *const m_tableEditor;

   bool LoadSetting();
   string GetItemText(int id) const;
   void SetItemText(int id, float value);
   void SaveCurrentPhysicsSetting();
};
