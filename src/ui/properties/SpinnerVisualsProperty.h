// license:GPLv3+

#pragma once

class SpinnerVisualsProperty final : public BasePropertyDialog
{
public:
    SpinnerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~SpinnerVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    EditBox   m_lengthEdit;
    EditBox   m_heightEdit;
    EditBox   m_rotationEdit;
    EditBox   m_angleMaxEdit;
    EditBox   m_angleMinEdit;
    EditBox   m_elasticityEdit;
    ComboBox  m_imageCombo;
    ComboBox  m_materialCombo;
    ComboBox  m_surfaceCombo;
    HWND      m_hShowBracketCheck;
};
