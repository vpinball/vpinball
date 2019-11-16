#ifndef H_SPINNER_PHYSICS_PROPERTY
#define H_SPINNER_PHYSICS_PROPERTY

class SpinnerPhysicsProperty: public BasePropertyDialog
{
public:
    SpinnerPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~SpinnerPhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CEdit   m_dampingEdit;
    CEdit   m_elasticityEdit;
};
#endif
