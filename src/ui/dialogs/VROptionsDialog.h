// license:GPLv3+

#pragma once

class VROptionsDialog final : public CDialog
{
public:
   VROptionsDialog();

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

private:
   void AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);
};
