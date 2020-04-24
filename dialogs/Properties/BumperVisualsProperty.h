#ifndef H_BUMPER_VISUALS_PROPERTY
#define H_BUMPER_VISUALS_PROPERTY

class BumperVisualsProperty: public BasePropertyDialog
{
public:
    BumperVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~BumperVisualsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_capMaterialCombo;
    CComboBox m_bumpBaseMaterialCombo;
    CComboBox m_skirtMaterialCombo;
    CComboBox m_ringMaterialCombo;
    EditBox   m_radiusEdit;
    EditBox   m_heightScaleEdit;
    EditBox   m_orientationEdit;
    EditBox   m_ringSpeedEdit;
    EditBox   m_ringDropOffsetEdit;
    HWND      m_hCapVisibleCheck;
    HWND      m_hBaseVisibleCheck;
    HWND      m_hRingVisibleCheck;
    HWND      m_hSkirtVisibleCheck;
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    CComboBox m_surfaceCombo;
};

#endif // !H_BUMPER_VISUALS_PROPERTY
