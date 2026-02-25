// license:GPLv3+

#pragma once

class RubberVisualsProperty final : public BasePropertyDialog
{
public:
    RubberVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~RubberVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
