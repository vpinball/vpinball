#ifndef H_PRIMITIVE_PHYSICS_PROPERTY
#define H_PRIMITIVE_PHYSICS_PROPERTY

class PrimitivePhysicsProperty: public BasePropertyDialog
{
public:
    PrimitivePhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~PrimitivePhysicsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    HWND    m_hToyCheck;
    CEdit   m_recudePolyEdit;
    CEdit   m_elasticityFalloffEdit;
    CEdit   m_hitThresholdEdit;
    CEdit   m_elasticityEdit;
    CEdit   m_frictionEdit;
    CEdit   m_scatterAngleEdit;
    CComboBox m_physicsMatrtialCombo;

};
#endif
