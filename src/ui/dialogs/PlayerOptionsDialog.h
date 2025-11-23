// license:GPLv3+

#pragma once

class PlayerOptionsDialog final : public CDialog
{
public:
   PlayerOptionsDialog();

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

private:
   void AddToolTip(const CWnd& wnd, const char* tip) const;
   void AddStringDOF(const string& name, const int idc) const;
   void SetDOFValue(int nID, const string& key) const;

   CToolTip m_tooltip;

   CButton m_bamHeadtracking; // TODO move to plugin
   CButton m_ballOverrideImages;
   CEdit m_ballImage;
   CEdit m_ballDecal;

   bool oldScaleValue = false;
   float scaleRelative = 1.0f;
   float scaleAbsolute = 55.0f;
};
