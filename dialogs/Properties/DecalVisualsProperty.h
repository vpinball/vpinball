#ifndef H_DECAL_VISUALS_PROPERTY
#define H_DECAL_VISUALS_PROPERTY

class DecalVisualsProperty: public BasePropertyDialog
{
public:
    DecalVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~DecalVisualsProperty();

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CComboBox   m_materialCombo;
    CComboBox   m_typeCombo;
    CEdit       m_textEdit;
    HWND        m_hVerticalTextCheck;
    ColorButton m_fontColorButton;
    CColorDialog m_colorDialog;
    CButton     m_fontDialogButton;
    CFontDialog m_fontDialog;
    CComboBox   m_imageCombo;
    CComboBox   m_sizingCombo;
    CEdit       m_posXEdit;
    CEdit       m_posYEdit;
    CEdit       m_widthEdit;
    CEdit       m_heigthEdit;
    CEdit       m_rotationEdit;
    CComboBox   m_surfaceCombo;
    CFont       *m_font;
    vector<string> m_typeList;
    vector<string> m_sizingType;
};

#endif // !H_DECAL_VISUALS_PROPERTY

