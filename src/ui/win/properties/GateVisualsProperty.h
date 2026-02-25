// license:GPLv3+

#pragma once

class GateVisualsProperty final : public BasePropertyDialog
{
public:
    GateVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~GateVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
    BOOL OnInitDialog() override;

private:
    ComboBox  m_typeCombo;
    ComboBox  m_materialCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_xposEdit;
    EditBox   m_yposEdit;
    EditBox   m_lengthEdit;
    EditBox   m_heightEdit;
    EditBox   m_rotationEdit;
    EditBox   m_openAngleEdit;
    EditBox   m_closeAngleEdit;
    vector<string> m_typeList;
};
