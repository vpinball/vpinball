#ifndef H_DISPREEL_STATE_PROPERTY
#define H_DISPREEL_STATE_PROPERTY

class DispreelStateProperty: public BasePropertyDialog
{
public:
    DispreelStateProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~DispreelStateProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    ComboBox    m_soundCombo;
    EditBox     m_motorStepsEdit;
    EditBox     m_updateIntervalEdit;
};

#endif // !H_DISPREEL_STATE_PROPERTY
