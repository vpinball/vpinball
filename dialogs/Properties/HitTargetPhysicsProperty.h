#ifndef H_HIT_TARGET_PHYSICS_PROPERTY
#define H_HIT_TARGET_PHYSICS_PROPERTY

class HitTargetPhysicsProperty: public BasePropertyDialog
{
public:
    HitTargetPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~HitTargetPhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CEdit       m_hitThresholdEdit;
    CComboBox   m_physicsMaterialCombo;
    CEdit       m_elasticityEdit;
    CEdit       m_elasticityFalloffEdit;
    CEdit       m_frictionEdit;
    CEdit       m_scatterAngleEdit;
    HWND        m_hLegacyModeCheck;
    HWND        m_hIsDroppedCheck;
};

#endif // !H_HIT_TARGET_PHYSICS_PROPERTY
