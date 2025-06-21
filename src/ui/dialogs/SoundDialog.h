// license:GPLv3+

#pragma once

#include "audio/AudioPlayer.h"

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
    void ListSounds();
    int AddListSound(VPX::Sound *const pps);

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

    std::unique_ptr<VPX::AudioPlayer> m_audioPlayer;
};

class SoundPositionDialog final : public CDialog
{
public:
   int m_volume, m_fade, m_balance; // from the CSliders
   VPX::SoundOutTypes m_cOutputTarget;

   SoundPositionDialog(VPX::AudioPlayer *const AudioPlayer, VPX::Sound *const pps);
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
   VPX::Sound *m_pSound;

   CSlider m_Volume;
   CSlider m_Fader;
   CSlider m_Balance;

   void GetDialogValues();
   void ReadValuesFromSliders();
   void TestSound();

   //int SliderToValue(const int Slider);
   //int ValueToSlider(const int Value);

   VPX::AudioPlayer *const m_audioPlayer;
};
