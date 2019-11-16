#ifndef H_KICKER_PHYSICS_PROPERTY
#define H_KICKER_PHYSICS_PROPERTY

class KickerPhysicsProperty: public BasePropertyDialog
{
public:
    KickerPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~KickerPhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    HWND    m_hEnableCheck;
    HWND    m_hFallThroughCheck;
    HWND    m_hLegacyCheck;
    CEdit   m_scatterAngleEdit;
    CEdit   m_hitAccuracyEdit;
    CEdit   m_hitHeightEdit;
};

#endif // !H_KICKER_PHYSICS_PROPERTY
