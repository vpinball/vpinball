// license:GPLv3+

#pragma once

class PlayerOptionsDialog final : public CDialog
{
public:
   PlayerOptionsDialog();

protected:
   BOOL OnInitDialog() override;
   void OnOK() override;

private:
   void AddToolTip(const CWnd& wnd, const char* tip) const;

   CToolTip m_tooltip;
};
