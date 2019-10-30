#ifndef H_DRAGPOINT_VISUALS_PROPERTY
#define H_DRAGPOINT_VISUALS_PROPERTY

class DragpointVisualsProperty: public BaseProperty
{
public:
    DragpointVisualsProperty(int id, VectorProtected<ISelect> *pvsel);
    virtual ~DragpointVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit m_posXEdit;
    CEdit m_posYEdit;
    CEdit m_heightOffsetEdit;
    CEdit m_realHeightEdit;
    CEdit m_textureCoordEdit;
    CButton m_copyButton;
    CButton m_pasteButton;
    int m_id;

};

#endif // !H_DRAGPOINT_VISUALS_PROPERTY

