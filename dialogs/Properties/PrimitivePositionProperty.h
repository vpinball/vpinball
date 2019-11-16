#ifndef H_PRIMITIVE_POSITION_PROPERTY
#define H_PRIMITIVE_POSITION_PROPERTY

class PrimitivePositionProperty : public BasePropertyDialog
{
public:
    PrimitivePositionProperty(VectorProtected<ISelect> *pvsel);
    virtual ~PrimitivePositionProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CEdit   m_posXEdit;
    CEdit   m_posYEdit;
    CEdit   m_posZEdit;
    CEdit   m_scaleXEdit;
    CEdit   m_scaleYEdit;
    CEdit   m_scaleZEdit;
    CEdit   m_rotXEdit;
    CEdit   m_rotYEdit;
    CEdit   m_rotZEdit;
    CEdit   m_transXEdit;
    CEdit   m_transYEdit;
    CEdit   m_transZEdit;
    CEdit   m_objRotXEdit;
    CEdit   m_objRotYEdit;
    CEdit   m_objRotZEdit;
};

#endif // !H_PRIMITIVE_POSITION_PROPERTY
