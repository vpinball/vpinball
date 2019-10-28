#ifndef H_GATE_PHYSICS_PROPERTY
#define H_GATE_PHYSICS_PROPERTY

class GatePhysicsProperty: public BaseProperty
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
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit     m_elasticityEdit;
    CEdit     m_frictionEdit;
    CEdit     m_dampingEdit;
    CEdit     m_gravityFactorEdit;

};

#endif
