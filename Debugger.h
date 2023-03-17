#pragma once

class DebuggerDialog;

class DbgMaterialDialog : public CDialog
{
public:
    DbgMaterialDialog();

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnClose() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    CComboBox    m_materialsCombo;
    CColorDialog m_colorDialog;
    ColorButton  m_colorButton1;
    ColorButton  m_colorButton2;
    ColorButton  m_colorButton3;
    ColorButton  m_colorButton4;
};

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
    CButton m_playButton;
    CButton m_pauseButton;
    CButton m_stepButton;
    CEdit   m_stepAmountEdit;
    CButton m_dbgLightsButton;
    CButton m_dbgMaterialsButton;
    HWND    m_hThrowBallsInPlayerCheck;
    HWND    m_hBallControlCheck;
    CEdit   m_ballSizeEdit;
    CEdit   m_ballMassEdit;

    DbgMaterialDialog m_materialDialog;

    CResizer  m_resizer;
    CEdit     m_notesEdit;
};
