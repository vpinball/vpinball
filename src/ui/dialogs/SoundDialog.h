#ifndef H_SOUND_DIALOG
#define H_SOUND_DIALOG

class SoundDialog : public CDialog
{
public:
    SoundDialog();
    virtual ~SoundDialog();

protected:
    virtual void OnDestroy();
    virtual void OnClose();
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
    virtual void OnOK();
    virtual void OnCancel();
private:
    void Import();
    void ReImport();
    void ReImportFrom();
    void Export();
    void SoundToBG();
    void SoundPosition();

    void DeleteSound();
    void LoadPosition();
    void SavePosition();

    HWND hSoundList;
    static int m_columnSortOrder;
};

class SoundPositionDialog : public CDialog
{
public:
	int m_volume, m_fade, m_balance;
	SoundOutTypes m_cOutputTarget;
	PinSound *m_pps;

	SoundPositionDialog(PinSound * const pps);
	virtual ~SoundPositionDialog();

protected:
	virtual void OnDestroy();
	virtual void OnClose();
	virtual BOOL OnInitDialog();
	void SetSliderValues();
	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ReadTextValue(int item, int &oValue);
	void SetTextValues();
	void SetTextValue(int ctl, int val);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();
private:
	CSlider m_Volume;
	CSlider m_Fader;
	CSlider m_Balance;
	void GetDialogValues();
	void ReadValuesFromSliders();
	void TestSound();
	//int SliderToValue(const int Slider);
	//int ValueToSlider(const int Value);
};

#endif
