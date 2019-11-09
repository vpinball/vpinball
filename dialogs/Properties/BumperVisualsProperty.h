#ifndef H_BUMPER_VISUALS_PROPERTY
#define H_BUMPER_VISUALS_PROPERTY

class BumperVisualsProperty: public BasePropertyDialog
{
public:
    BumperVisualsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~BumperVisualsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();

private:
    CComboBox m_capMaterialCombo;
    CComboBox m_bumpBaseMaterialCombo;
    CComboBox m_skirtMaterialCombo;
    CComboBox m_ringMaterialCombo;
    CEdit     m_radiusEdit;
    CEdit     m_heightScaleEdit;
    CEdit     m_orientationEdit;
    CEdit     m_ringSpeedEdit;
    CEdit     m_ringSpeedOffsetEdit;
    HWND      m_hCapVisibleCheck;
    HWND      m_hBaseVisibleCheck;
    HWND      m_hRingVisibleCheck;
    HWND      m_hSkirtVisibleCheck;
    CEdit     m_posXEdit;
    CEdit     m_posYEdit;
    CComboBox m_surfaceCombo;


};
#endif // !H_BUMPER_VISUALS_PROPERTY

