#ifndef H_TABLE_CUSTOM_PROPERTY
#define H_TABLE_CUSTOM_PROPERTY

class TableCustomProperty: public BasePropertyDialog
{
public:
    TableCustomProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TableCustomProperty()
    {
    }

    void UpdateVisuals();
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CComboBox   m_SSAACombo;
    CComboBox   m_postProcAACombo;
    CComboBox   m_inGameAOCombo;
    CComboBox   m_ScreenReflectionCombo;
    CEdit       m_fpsLimiterEdit;
    CSlider     m_detailLevelSlider;
    HWND        m_hOverwriteDetailsCheck;
    CComboBox   m_ballReflectionCombo;
    CComboBox   m_ballTrailCombo;
    CEdit       m_ballTrailStrengthEdit;
    CSlider     m_nightDaySlider;
    HWND        m_hOverwriteNightDayCheck;
    CEdit       m_gameplayDifficultEdit;
    CComboBox   m_overwritePhysicsSetCombo;
    HWND        m_hOverwriteFlipperCheck;
    CEdit       m_soundEffectVolEdit;
    CEdit       m_musicVolEdit;

    vector<string> m_postAAList;
    vector<string> m_userList;
    vector<string> m_physicSetList;
};

#endif // !H_TABLE_CUSTOM_PROPERTY

