// license:GPLv3+

#pragma once

class ToolbarDialog : public CDialog
{
public:
    ToolbarDialog();
    virtual ~ToolbarDialog();
    virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);

    void EnableButtons();
    bool PreTranslateMessage(MSG* msg);

protected:
    virtual BOOL OnInitDialog();
    virtual void OnDestroy();
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

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

class CContainToolbar: public CDockContainer
{
public:
    CContainToolbar();
    ~CContainToolbar()
    {
    }

    ToolbarDialog *GetToolbarDialog()
    {
        return &m_toolbar;
    }

private:
    ToolbarDialog m_toolbar;
};

class CDockToolbar: public CDocker
{
public:
    CDockToolbar();
    virtual ~CDockToolbar()
    {
    }
    virtual void OnClose();

    CContainToolbar *GetContainToolbar()
    {
        return &m_toolbarContainer;
    }

private:
    CContainToolbar m_toolbarContainer;
};
