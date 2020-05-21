#pragma once

class DebuggerDialog;
class DbgLightDialog : public CDialog
{
public:
    DbgLightDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnClose();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    Light           *GetLight();
    void            SetCheckButtonState(Light *plight);

    CComboBox       m_lightsCombo;
    HWND            m_hLightOnCheck;
    HWND            m_hLightOffCheck;
    HWND            m_hLightBlinkCheck;
    CColorDialog    m_colorDialog;
    ColorButton     m_colorButton;
    ColorButton     m_colorButton2;
};

class DbgMaterialDialog : public CDialog
{
public:
    DbgMaterialDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnClose();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CComboBox    m_materialsCombo;
    CColorDialog m_colorDialog;
    ColorButton  m_colorButton1;
    ColorButton  m_colorButton2;
    ColorButton  m_colorButton3;
};

class DebuggerDialog : public CDialog
{
public:
    DebuggerDialog();
    BOOL IsSubDialogMessage(MSG& msg) const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnClose();
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

    DbgLightDialog    m_lightDialog;
    DbgMaterialDialog m_materialDialog;
};
