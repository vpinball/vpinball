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
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit   m_dampingEdit;
    CEdit   m_elasticityEdit;
};
#endif


