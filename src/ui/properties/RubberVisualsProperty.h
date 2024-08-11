// license:GPLv3+

#pragma once

class RubberVisualsProperty: public BasePropertyDialog
{
public:
    RubberVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~RubberVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_imageCombo;
    ComboBox    m_materialCombo;
    HWND        m_hStaticRenderingCheck;
    EditBox     m_heightEdit;
    EditBox     m_thicknessEdit;
    EditBox     m_rotXEdit;
    EditBox     m_rotYEdit;
    EditBox     m_rotZEdit;
    HWND        m_hShowInEditorCheck;
};
