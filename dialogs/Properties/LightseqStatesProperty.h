#ifndef H_LIGHTSEQ_STATES_PROPERTY
#define H_LIGHTSEQ_STATES_PROPERTY

class LightseqStatesProperty: public BasePropertyDialog
{
public:
    LightseqStatesProperty(VectorProtected<ISelect> *pvsel);
    virtual ~LightseqStatesProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CEdit       m_posXEdit;
    CEdit       m_posYEdit;
    CComboBox   m_collectionCombo;
    CEdit       m_updateIntervalEdit;
};


#endif // !H_LIGHTSEQ_STATES_PROPERTY

