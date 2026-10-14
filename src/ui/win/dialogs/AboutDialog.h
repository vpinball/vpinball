// license:GPLv3+

#pragma once

#include <wxx_dialog.h>

class AboutDialog final : public CDialog
{
public:
   AboutDialog();
   ~AboutDialog() override;

protected:
   void OnDestroy() override;
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

   string m_urlString;
};
