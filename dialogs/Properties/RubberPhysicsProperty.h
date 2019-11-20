#ifndef H_RUBBER_PHYSICS_PROPERTY
#define H_RUBBER_PHYSICS_PROPERTY

class RubberPhysicsProperty: public BasePropertyDialog
{
public:
    RubberPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~RubberPhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_physicsMaterialCombo;
    CEdit       m_elasticityEdit;
    CEdit       m_elasticityFallOffEdit;
    CEdit       m_frictionEdit;
    CEdit       m_scatterAngleEdit;
    CEdit       m_hitHeightEdit;

};

#endif // !H_RUBBER_PHYSICS_PROPERTY

