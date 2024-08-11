// license:GPLv3+

#pragma once

class RubberPhysicsProperty: public BasePropertyDialog
{
public:
    RubberPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~RubberPhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_physicsMaterialCombo;
    EditBox     m_elasticityEdit;
    EditBox     m_elasticityFallOffEdit;
    EditBox     m_frictionEdit;
    EditBox     m_scatterAngleEdit;
    EditBox     m_hitHeightEdit;
};
