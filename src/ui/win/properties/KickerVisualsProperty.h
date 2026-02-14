// license:GPLv3+

#pragma once

class KickerVisualsProperty final : public BasePropertyDialog
{
public:
    KickerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~KickerVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox  m_materialCombo;
    ComboBox  m_displayCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_radiusEdit;
    EditBox   m_orientationEdit;
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    vector<string> m_typeList;
};
