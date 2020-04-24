#ifndef H_TABLE_CUSTOM_PROPERTY
#define H_TABLE_CUSTOM_PROPERTY

class TableCustomProperty: public BasePropertyDialog
{
public:
    TableCustomProperty(VectorProtected<ISelect> *pvsel);
    virtual ~TableCustomProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CComboBox   m_SSAACombo;
    CComboBox   m_postProcAACombo;
    CComboBox   m_inGameAOCombo;
    CComboBox   m_ScreenReflectionCombo;
    CSlider     m_detailLevelSlider;
    HWND        m_hOverwriteDetailsCheck;
    CComboBox   m_ballReflectionCombo;
    CComboBox   m_ballTrailCombo;
    EditBox     m_gameplayDifficultEdit;
    EditBox     m_fpsLimiterEdit;
    EditBox     m_ballTrailStrengthEdit;
    EditBox     m_soundEffectVolEdit;
    EditBox     m_musicVolEdit;
    CSlider     m_nightDaySlider;
    HWND        m_hOverwriteNightDayCheck;
    CComboBox   m_overwritePhysicsSetCombo;
    HWND        m_hOverwriteFlipperCheck;

    vector<string> m_postAAList;
    vector<string> m_userList;
    vector<string> m_physicSetList;
};

#endif // !H_TABLE_CUSTOM_PROPERTY
