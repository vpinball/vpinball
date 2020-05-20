#pragma once


class DebuggerDialog : public CDialog
{
public:
    DebuggerDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CButton m_playButton;
    CButton m_pauseButton;
    CButton m_stepButton;
    CEdit   m_stepAmountEdit;
    CButton m_editorExpandButton;
    CButton m_dbgLightsButton;
    CButton m_dbgMaterialsButton;
    HWND    m_hThrowBallsInPlayerCheck;
    HWND    m_hBallControlCheck;
    CEdit   m_ballSizeEdit;
    CEdit   m_ballMassEdit;


};
