// license:GPLv3+

#pragma once

class TriggerVisualsProperty: public BasePropertyDialog
{
public:
    TriggerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TriggerVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_shapeCombo;
    ComboBox    m_materialCombo;
    ComboBox    m_surfaceCombo;
    EditBox     m_wireThicknessEdit;
    EditBox     m_starRadiusEdit;
    EditBox     m_rotationEdit;
    EditBox     m_animationSpeedEdit;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    vector<string> m_shapeList;
};
