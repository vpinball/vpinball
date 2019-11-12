#ifndef H_DISPREEL_STATE_PROPERTY
#define H_DISPREEL_STATE_PROPERTY

class DispreelStateProperty: public BasePropertyDialog
{
public:
    DispreelStateProperty(VectorProtected<ISelect> *pvsel);
    virtual ~DispreelStateProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox   m_soundCombo;
    CEdit       m_motorStepsEdit;
    CEdit       m_updateIntervalEdit;

};

#endif // !H_DISPREEL_STATE_PROPERTY

