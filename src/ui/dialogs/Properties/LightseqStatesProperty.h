#ifndef H_LIGHTSEQ_STATES_PROPERTY
#define H_LIGHTSEQ_STATES_PROPERTY

class LightseqStatesProperty: public BasePropertyDialog
{
public:
    LightseqStatesProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~LightseqStatesProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    EditBox     m_updateIntervalEdit;
    ComboBox    m_collectionCombo;
};

#endif // !H_LIGHTSEQ_STATES_PROPERTY
