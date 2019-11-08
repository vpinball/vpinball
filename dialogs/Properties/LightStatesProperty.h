#ifndef H_LIGHT_STATES_PROPERTY
#define H_LIGHT_STATES_PROPERTY

class LightStatesProperty: public BasePropertyDialog
{
public:
    LightStatesProperty(VectorProtected<ISelect> *pvsel);
    virtual ~LightStatesProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit       m_blinkPatternEdit;
    CEdit       m_blinkIntervalEdit;
    CComboBox   m_stateCombo;
    vector<string> m_stateList;

};

#endif // !H_LIGHT_STATES_PROPERTY

