#include "stdafx.h"
#include "resource.h"
#include "AudioOptionsDialog.h"

AudioOptionsDialog::AudioOptionsDialog() : CDialog(IDD_AUDIO_OPTIONS)
{
}

BOOL AudioOptionsDialog::OnInitDialog()
{
   SendDlgItemMessage(IDC_APPLICATION_SETTINGS, BM_SETCHECK, BST_CHECKED, 0);
   GetDlgItem(IDC_TABLE_OVERRIDE).EnableWindow(g_pvp->m_ptableActive != nullptr);

   SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
   SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_SETTICFREQ, 10, 0);
   SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_SETLINESIZE, 0, 1);
   SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_SETPAGESIZE, 0, 10);
   SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_SETTHUMBLENGTH, 10, 0);

   SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
   SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_SETTICFREQ, 10, 0);
   SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_SETLINESIZE, 0, 1);
   SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_SETPAGESIZE, 0, 10);
   SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_SETTHUMBLENGTH, 10, 0);

   SendDlgItemMessage(IDC_SoundList, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendDlgItemMessage(IDC_SoundListBG, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendDlgItemMessage(IDC_SoundList, LB_RESETCONTENT, 0, 0);
   SendDlgItemMessage(IDC_SoundListBG, LB_RESETCONTENT, 0, 0);
   DSAudioDevices DSads;
   if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
   {
      for (size_t i = 0; i < DSads.size(); i++)
      {
         const size_t index = SendDlgItemMessage(IDC_SoundList, LB_ADDSTRING, 0, (size_t)DSads[i]->description.c_str());
         SendDlgItemMessage(IDC_SoundList, LB_SETITEMDATA, index, (LPARAM)i);
         const size_t indexbg = SendDlgItemMessage(IDC_SoundListBG, LB_ADDSTRING, 0, (size_t)DSads[i]->description.c_str());
         SendDlgItemMessage(IDC_SoundListBG, LB_SETITEMDATA, indexbg, (LPARAM)i);
         delete DSads[i];
      }
   }
   SendDlgItemMessage(IDC_SoundList, WM_SETREDRAW, TRUE, 0);
   SendDlgItemMessage(IDC_SoundListBG, WM_SETREDRAW, TRUE, 0);

   OnCommand(IDC_APPLICATION_SETTINGS, 0L);

   return TRUE;
}

BOOL AudioOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
   case IDC_SAVE_ALL:
   {
      SaveSettings(true);
      Close();
      break;
   }
   case IDC_TABLE_OVERRIDE:
   {
      // for the time being, only the volume settings can be overriden by the table
      GetDlgItem(IDC_SoundList).EnableWindow(false);
      GetDlgItem(IDC_SoundListBG).EnableWindow(false);
      GetDlgItem(IDC_RADIO_SND3D2CH).EnableWindow(false);
      GetDlgItem(IDC_RADIO_SND3DALLREAR).EnableWindow(false);
      GetDlgItem(IDC_RADIO_SND3DFRONTISFRONT).EnableWindow(false);
      GetDlgItem(IDC_RADIO_SND3DFRONTISREAR).EnableWindow(false);
      GetDlgItem(IDC_RADIO_SND3D6CH).EnableWindow(false);
      GetDlgItem(IDC_RADIO_SND3DSSF).EnableWindow(false);
      GetDlgItem(IDC_SAVE_ALL).ShowWindow(true);
      GetDlgItem(IDOK).SetWindowText("Save overrides");
      LoadSettings();
      break;
   }
   case IDC_APPLICATION_SETTINGS:
   {
      // for the time being, only the volume settings can be overriden by the table
      GetDlgItem(IDC_SoundList).EnableWindow(true);
      GetDlgItem(IDC_SoundListBG).EnableWindow(true);
      GetDlgItem(IDC_RADIO_SND3D2CH).EnableWindow(true);
      GetDlgItem(IDC_RADIO_SND3DALLREAR).EnableWindow(true);
      GetDlgItem(IDC_RADIO_SND3DFRONTISFRONT).EnableWindow(true);
      GetDlgItem(IDC_RADIO_SND3DFRONTISREAR).EnableWindow(true);
      GetDlgItem(IDC_RADIO_SND3D6CH).EnableWindow(true);
      GetDlgItem(IDC_RADIO_SND3DSSF).EnableWindow(true);
      GetDlgItem(IDC_SAVE_ALL).ShowWindow(false);
      GetDlgItem(IDOK).SetWindowText("OK");
      LoadSettings();
      break;
   }
   case IDC_PLAY_MUSIC:
   {
      const size_t checked = SendDlgItemMessage(IDC_PLAY_MUSIC, BM_GETCHECK, 0, 0);
      GetDlgItem(IDC_MUSIC_SLIDER).EnableWindow(checked == BST_CHECKED);
      GetDlgItem(IDC_STATIC_MUSIC).EnableWindow(checked == BST_CHECKED);
      break;
   }
   case IDC_PLAY_SOUND:
   {
      const size_t checked = SendDlgItemMessage(IDC_PLAY_SOUND, BM_GETCHECK, 0, 0);
      GetDlgItem(IDC_SOUND_SLIDER).EnableWindow(checked == BST_CHECKED);
      GetDlgItem(IDC_STATIC_SOUND).EnableWindow(checked == BST_CHECKED);
      break;
   }
   default: 
      return FALSE;
   }
   return TRUE;
}

void AudioOptionsDialog::OnOK()
{
   SaveSettings(SendDlgItemMessage(IDC_APPLICATION_SETTINGS, BM_GETCHECK, 0, 0) == BST_CHECKED);
   CDialog::OnOK();
}

Settings& AudioOptionsDialog::GetEditedSettings()
{
   if (g_pvp->m_ptableActive && SendDlgItemMessage(IDC_TABLE_OVERRIDE, BM_GETCHECK, 0, 0) == BST_CHECKED)
      return g_pvp->m_ptableActive->m_settings;
   SendDlgItemMessage(IDC_APPLICATION_SETTINGS, BM_SETCHECK, BST_CHECKED, 0);
   return g_pvp->m_settings;
}

void AudioOptionsDialog::LoadSettings()
{
   Settings& settings = GetEditedSettings();
   
   bool fsound = settings.LoadValueWithDefault(Settings::Player, "PlayMusic"s, true);
   SendDlgItemMessage(IDC_PLAY_MUSIC, BM_SETCHECK, fsound ? BST_CHECKED : BST_UNCHECKED, 0);
   OnCommand(IDC_PLAY_MUSIC, 0L);

   fsound = settings.LoadValueWithDefault(Settings::Player, "PlaySound"s, true);
   SendDlgItemMessage(IDC_PLAY_SOUND, BM_SETCHECK, fsound ? BST_CHECKED : BST_UNCHECKED, 0);
   OnCommand(IDC_PLAY_SOUND, 0L);
   
   int fmusic = settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, 0);
   switch (fmusic)
   {
   case SNDCFG_SND3DALLREAR: SendDlgItemMessage(IDC_RADIO_SND3DALLREAR, BM_SETCHECK, BST_CHECKED, 0); break;
   case SNDCFG_SND3DFRONTISFRONT: SendDlgItemMessage(IDC_RADIO_SND3DFRONTISFRONT, BM_SETCHECK, BST_CHECKED, 0); break;
   case SNDCFG_SND3DFRONTISREAR: SendDlgItemMessage(IDC_RADIO_SND3DFRONTISREAR, BM_SETCHECK, BST_CHECKED, 0); break;
   case SNDCFG_SND3D6CH: SendDlgItemMessage(IDC_RADIO_SND3D6CH, BM_SETCHECK, BST_CHECKED, 0); break;
   case SNDCFG_SND3DSSF: SendDlgItemMessage(IDC_RADIO_SND3DSSF, BM_SETCHECK, BST_CHECKED, 0); break;
   default: SendDlgItemMessage(IDC_RADIO_SND3D2CH, BM_SETCHECK, BST_CHECKED, 0); break;
   }

   SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_SETPOS, TRUE, settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100));
   SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_SETPOS, TRUE, settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100));

   SendDlgItemMessage(IDC_SoundList, LB_SETCURSEL, settings.LoadValueWithDefault(Settings::Player, "SoundDevice"s, 0), 0);
   SendDlgItemMessage(IDC_SoundListBG, LB_SETCURSEL, settings.LoadValueWithDefault(Settings::Player, "SoundDeviceBG"s, 0), 0);
}

void AudioOptionsDialog::SaveSettings(const bool saveAll)
{
   Settings& settings = GetEditedSettings();

   size_t checked = SendDlgItemMessage(IDC_PLAY_MUSIC, BM_GETCHECK, 0, 0);
   settings.SaveValue(Settings::Player, "PlayMusic"s, checked == BST_CHECKED, !saveAll);

   checked = SendDlgItemMessage(IDC_PLAY_SOUND, BM_GETCHECK, 0, 0);
   settings.SaveValue(Settings::Player, "PlaySound"s, (checked == BST_CHECKED), !saveAll);

   int fmusic = SNDCFG_SND3D2CH;
   if (SendDlgItemMessage(IDC_RADIO_SND3DALLREAR, BM_GETCHECK, 0, 0) == BST_CHECKED)
	   fmusic = SNDCFG_SND3DALLREAR;
   if (SendDlgItemMessage(IDC_RADIO_SND3DFRONTISFRONT, BM_GETCHECK, 0, 0) == BST_CHECKED)
	   fmusic = SNDCFG_SND3DFRONTISFRONT;
   if (SendDlgItemMessage(IDC_RADIO_SND3DFRONTISREAR, BM_GETCHECK, 0, 0) == BST_CHECKED)
	   fmusic = SNDCFG_SND3DFRONTISREAR;
   if (SendDlgItemMessage(IDC_RADIO_SND3D6CH, BM_GETCHECK, 0, 0) == BST_CHECKED)
	   fmusic = SNDCFG_SND3D6CH;
   if (SendDlgItemMessage(IDC_RADIO_SND3DSSF, BM_GETCHECK, 0, 0) == BST_CHECKED)
	   fmusic = SNDCFG_SND3DSSF;
   settings.SaveValue(Settings::Player, "Sound3D"s, fmusic, !saveAll);

   size_t volume = SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_GETPOS, 0, 0);
   settings.SaveValue(Settings::Player, "MusicVolume"s, (int)volume, !saveAll);

   volume = SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_GETPOS, 0, 0);
   settings.SaveValue(Settings::Player, "SoundVolume"s, (int)volume, !saveAll);

   size_t soundindex = SendDlgItemMessage(IDC_SoundList, LB_GETCURSEL, 0, 0);
   size_t sd = SendDlgItemMessage(IDC_SoundList, LB_GETITEMDATA, soundindex, 0);
   settings.SaveValue(Settings::Player, "SoundDevice"s, (int)sd, !saveAll);
   
   soundindex = SendDlgItemMessage(IDC_SoundListBG, LB_GETCURSEL, 0, 0);
   sd = SendDlgItemMessage(IDC_SoundListBG, LB_GETITEMDATA, soundindex, 0);
   settings.SaveValue(Settings::Player, "SoundDeviceBG"s, (int)sd, !saveAll);
   
   g_pvp->ReInitSound();
}

