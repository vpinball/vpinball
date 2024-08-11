// license:GPLv3+

#pragma once

class SpinnerVisualsProperty: public BasePropertyDialog
{
public:
    SpinnerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~SpinnerVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
