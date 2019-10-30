#ifndef H_RAMP_PHYSICS_PROPERTY
#define H_RAMP_PHYSICS_PROPERTY

class RampPhysicsProperty: public BaseProperty
{
public:
    RampPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~RampPhysicsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);


private:
    CComboBox m_physicsMaterialCombo;
    CEdit     m_hitThresholdEdit;
    CEdit     m_leftWallEdit;
    CEdit     m_rightWallEdit;
    CEdit     m_elasticityEdit;
    CEdit     m_frictionEdit;
    CEdit     m_scatterAngleEdit;
};

#endif 


