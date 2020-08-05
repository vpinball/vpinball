#ifndef H_BUMPER_PHYSICS_PROPERTY
#define H_BUMPER_PHYSICS_PROPERTY

class BumperPhysicsProperty: public BasePropertyDialog
{
public:
    BumperPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~BumperPhysicsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    EditBox   m_forceEdit;
    EditBox   m_hitThresholdEdit;
    EditBox   m_scatterAngleEdit;
};

#endif
