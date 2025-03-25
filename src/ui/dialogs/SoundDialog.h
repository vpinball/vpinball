// license:GPLv3+

#pragma once

class SoundDialog final : public CDialog
{
public:
    SoundDialog();
    ~SoundDialog() override;

protected:
    void OnDestroy() override;
    void OnClose() override;
    BOOL OnInitDialog() override;
    INT_PTR DialogProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) override;
    BOOL OnCommand( WPARAM wParam, LPARAM lParam ) override;
    void OnOK() override;
    void OnCancel() override;

private:
    CResizer m_resizer;
    void Import();
    void ReImport();
    void ReImportFrom();
    void Export();
    void SoundToBG();
    void SoundPosition();

    void DeleteSound();
    void LoadPosition();
    void SavePosition();
    void AddToolTip(const char *const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);

    HWND hSoundList;
    static int m_columnSortOrder;
};

class SoundPositionDialog final : public CDialog
{
public:
   int m_volume, m_fade, m_balance; // from the CSliders
   SoundOutTypes m_cOutputTarget;

   SoundPositionDialog(PinSound * const pps);
   ~SoundPositionDialog() override;

protected:
   void OnDestroy() override;
   void OnClose() override;
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;
   void OnCancel() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

   void SetSliderValues();
   void ReadTextValue(int item, int &oValue);
   void SetTextValues();
   void SetTextValue(int ctl, int val);

private:
   PinSound *m_pPinSound;

   CSlider m_Volume;
   CSlider m_Fader;
   CSlider m_Balance;

   void GetDialogValues();
   void ReadValuesFromSliders();
   void TestSound();

   //int SliderToValue(const int Slider);
   //int ValueToSlider(const int Value);
};
