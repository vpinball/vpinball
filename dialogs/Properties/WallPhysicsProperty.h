#ifndef H_WALL_PHYSICS_PROPERTY
#define H_WALL_PHYSICS_PROPERTY

class WallPhysicsProperty: public BasePropertyDialog
{
public:
    WallPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~WallPhysicsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CEdit     m_hitThresholdEdit;
    CEdit     m_slingshotForceEdit;
    CEdit     m_slingshotThresholdEdit;
    CEdit     m_elasticityEdit;
    CEdit     m_frictionEdit;
    CEdit     m_scatterAngleEdit;
    CComboBox m_physicsMaterialCombo;
};

#endif
