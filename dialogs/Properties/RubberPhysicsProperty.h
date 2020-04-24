#ifndef H_RUBBER_PHYSICS_PROPERTY
#define H_RUBBER_PHYSICS_PROPERTY

class RubberPhysicsProperty: public BasePropertyDialog
{
public:
    RubberPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~RubberPhysicsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_physicsMaterialCombo;
    EditBox     m_elasticityEdit;
    EditBox     m_elasticityFallOffEdit;
    EditBox     m_frictionEdit;
    EditBox     m_scatterAngleEdit;
    EditBox     m_hitHeightEdit;
};

#endif // !H_RUBBER_PHYSICS_PROPERTY
