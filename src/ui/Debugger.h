// license:GPLv3+

#pragma once

class DebuggerDialog : public CDialog
{
public:
    DebuggerDialog();
    BOOL IsSubDialogMessage(MSG& msg) const;

protected:
    BOOL OnInitDialog() override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnClose() override;
    LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    void LoadPosition();
    void SavePosition();

private:
    CButton m_playButton;
    CButton m_pauseButton;
    CButton m_stepButton;
    CEdit   m_stepAmountEdit;
    CButton m_dbgLightsButton;
    CButton m_dbgMaterialsButton;
    CEdit   m_ballSizeEdit;
    CEdit   m_ballMassEdit;
    CResizer m_resizer;
    CEdit   m_notesEdit;
};
