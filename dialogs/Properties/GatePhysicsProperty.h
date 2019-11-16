#ifndef H_GATE_PHYSICS_PROPERTY
#define H_GATE_PHYSICS_PROPERTY

class GatePhysicsProperty: public BasePropertyDialog
{
public:
    GatePhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~GatePhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CEdit     m_elasticityEdit;
    CEdit     m_frictionEdit;
    CEdit     m_dampingEdit;
    CEdit     m_gravityFactorEdit;
};

#endif
