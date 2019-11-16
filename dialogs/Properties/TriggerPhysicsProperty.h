#ifndef H_TRIGGER_PHYSICS_PROPERTY
#define H_TRIGGER_PHYSICS_PROPERTY

class TriggerPhysicsProperty: public BasePropertyDialog
{
public:
    TriggerPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TriggerPhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    HWND    m_hEnableCheck;
    CEdit   m_hitHeightEdit;
};

#endif // !H_TRIGGER_PHYSICS_PROPERTY
