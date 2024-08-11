// license:GPLv3+

#pragma once

class PrimitivePhysicsProperty: public BasePropertyDialog
{
public:
    PrimitivePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~PrimitivePhysicsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND      m_hToyCheck;
    EditBox   m_reducePolyEdit;
    EditBox   m_elasticityFalloffEdit;
    EditBox   m_hitThresholdEdit;
    EditBox   m_elasticityEdit;
    EditBox   m_frictionEdit;
    EditBox   m_scatterAngleEdit;
    ComboBox  m_physicsMaterialCombo;
};
