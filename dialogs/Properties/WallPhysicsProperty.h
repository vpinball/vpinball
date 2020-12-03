#ifndef H_WALL_PHYSICS_PROPERTY
#define H_WALL_PHYSICS_PROPERTY

class WallPhysicsProperty: public BasePropertyDialog
{
public:
    WallPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~WallPhysicsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    EditBox   m_hitThresholdEdit;
    EditBox   m_slingshotForceEdit;
    EditBox   m_slingshotThresholdEdit;
    EditBox   m_elasticityEdit;
    EditBox   m_elasticityFallOffEdit;
    EditBox   m_frictionEdit;
    EditBox   m_scatterAngleEdit;
    ComboBox  m_physicsMaterialCombo;
};

#endif
