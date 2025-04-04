// license:GPLv3+

#pragma once

class ToolbarDialog final : public CDialog
{
public:
    ToolbarDialog();
    ~ToolbarDialog() override;
    LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);

    void EnableButtons();
    BOOL PreTranslateMessage(MSG& msg) override;

protected:
    BOOL OnInitDialog() override;
    void OnDestroy() override;
    INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

private:
    HWND m_hwnd;
    CResizer m_resizer;
    CToolTip m_tooltip;

    CButton m_magnifyButton;
    CButton m_selectButton;
    CButton m_scriptButton;
    CButton m_backglassButton;
    CButton m_playButton;
    CButton m_playCameraButton;
    CButton m_wallButton;
    CButton m_gateButton;
    CButton m_rampButton;
    CButton m_flipperButton;
    CButton m_plungerButton;
    CButton m_ballButton;
    CButton m_bumperButton;
    CButton m_spinnerButton;
    CButton m_timerButton;
    CButton m_triggerButton;
    CButton m_lightButton;
    CButton m_kickerButton;
    CButton m_targetButton;
    CButton m_decalButton;
    CButton m_textboxButton;
    CButton m_reelButton;
    CButton m_lightseqButton;
    CButton m_primitiveButton;
    CButton m_flasherButton;
    CButton m_rubberButton;
};

class CContainToolbar final : public CDockContainer
{
public:
    CContainToolbar();
    ~CContainToolbar() override {}

    ToolbarDialog *GetToolbarDialog()
    {
        return &m_toolbar;
    }

private:
    ToolbarDialog m_toolbar;
};

class CDockToolbar final : public CDocker
{
public:
    CDockToolbar();
    ~CDockToolbar() override {}

    void OnClose() override;

    CContainToolbar *GetContainToolbar()
    {
        return &m_toolbarContainer;
    }

private:
    CContainToolbar m_toolbarContainer;
};
