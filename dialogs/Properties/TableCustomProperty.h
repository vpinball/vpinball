#ifndef H_TABLE_CUSTOM_PROPERTY
#define H_TABLE_CUSTOM_PROPERTY

class TableCustomProperty: public BasePropertyDialog
{
public:
    TableCustomProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TableCustomProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ComboBox    m_inGameAOCombo;
    ComboBox    m_ScreenReflectionCombo;
    ComboBox    m_ballReflectionCombo;
    ComboBox    m_ballTrailCombo;
    ComboBox    m_overwritePhysicsSetCombo;
    EditBox     m_gameplayDifficultEdit;
    EditBox     m_fpsLimiterEdit;
    EditBox     m_ballTrailStrengthEdit;
    EditBox     m_soundEffectVolEdit;
    EditBox     m_musicVolEdit;
    CSlider     m_nightDaySlider;
    HWND        m_hOverwriteNightDayCheck;
    HWND        m_hOverwriteFlipperCheck;

    vector<string> m_userList;
    vector<string> m_physicSetList;
};

#endif // !H_TABLE_CUSTOM_PROPERTY
