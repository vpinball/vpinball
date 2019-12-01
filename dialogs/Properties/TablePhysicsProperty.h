#ifndef H_TABLE_PHYSICS_PROPERTY
#define H_TABLE_PHYSICS_PROPERTY

class TablePhysicsProperty: public BasePropertyDialog
{
public:
    TablePhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TablePhysicsProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit   m_gravityConstantEdit;
    CEdit   m_playfieldFrictionEdit;
    CEdit   m_playfieldElasticityEdit;
    CEdit   m_playfieldElasticityFalloffEdit;
    CEdit   m_playfieldScatterEdit;
    CEdit   m_defaultScatterEdit;
    CButton m_importSetButton;
    CButton m_exportSetButton;
    CEdit   m_nudgeTimeEdit;
    CEdit   m_physicsLoopEdit;
    CEdit   m_mechPlungerAdjEdit;
    HWND    m_hFilterMechanicalPlungerCheck;
    CEdit   m_tableWidthEdit;
    CEdit   m_tableHeightEdit;
    CEdit   m_topGlassHeightEdit;
    CEdit   m_tableFieldHeightEdit;
    CEdit   m_minSlopeEdit;
    CEdit   m_maxSlopeEdit;
};

#endif // !H_TABLE_PHYSICS_PROPERTY
