#ifndef H_BUMPER_PHYSICS_PROPERTY
#define H_BUMPER_PHYSICS_PROPERTY

class BumperPhysicsProperty: public BasePropertyDialog
{
public:
    BumperPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~BumperPhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CEdit   m_forceEdit;
    CEdit   m_hitThresholdEdit;
    CEdit   m_scatterAngleEdit;
};

#endif
