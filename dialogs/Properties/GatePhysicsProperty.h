#ifndef H_GATE_PHYSICS_PROPERTY
#define H_GATE_PHYSICS_PROPERTY

class GatePhysicsProperty: public BasePropertyDialog
{
public:
    GatePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~GatePhysicsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    EditBox    m_elasticityEdit;
    EditBox    m_frictionEdit;
    EditBox    m_dampingEdit;
    EditBox    m_gravityFactorEdit;
};

#endif
