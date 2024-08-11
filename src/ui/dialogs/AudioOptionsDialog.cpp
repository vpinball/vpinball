// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "AudioOptionsDialog.h"

AudioOptionsDialog::AudioOptionsDialog() : CDialog(IDD_AUDIO_OPTIONS)
{
}

BOOL AudioOptionsDialog::OnInitDialog()
{
   // Do a copy of the edited settings
   m_editedSettings = nullptr;
   m_appSettings = g_pvp->m_settings;
   if (g_pvp->m_ptableActive)
   {
      m_tableSettings = g_pvp->m_ptableActive->m_settings;
      m_tableSettings.SetParent(&m_appSettings);
   }

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
   if (SUCCEEDED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
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
   case IDC_SAVE_OVERRIDES:
   case IDC_SAVE_ALL:
   {
      if (IsDlgButtonChecked(IDC_APPLICATION_SETTINGS) == BST_CHECKED)
      {
         SaveSettings(true);
         g_pvp->m_settings = m_appSettings;
      }
      else if (g_pvp->m_ptableActive && LOWORD(wParam) == IDC_SAVE_ALL)
      {
         SaveSettings(true);
         g_pvp->m_ptableActive->m_settings = m_tableSettings;
      }
      else if (g_pvp->m_ptableActive && LOWORD(wParam) == IDC_SAVE_OVERRIDES)
      {
         SaveSettings(false);
         g_pvp->m_ptableActive->m_settings.CopyOverrides(m_tableSettings);
      }
      g_pvp->m_settings.Save();
      if (g_pvp->m_ptableActive && !g_pvp->m_ptableActive->GetSettingsFileName().empty())
         g_pvp->m_ptableActive->m_settings.SaveToFile(g_pvp->m_ptableActive->GetSettingsFileName());
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
      GetDlgItem(IDC_SAVE_OVERRIDES).ShowWindow(true);
      SetDlgItemText(IDC_SAVE_ALL, "Save All");
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
      GetDlgItem(IDC_SAVE_OVERRIDES).ShowWindow(false);
      SetDlgItemText(IDC_SAVE_ALL, "Save Changes");
      LoadSettings();
      break;
   }
   case IDC_PLAY_MUSIC:
   {
      const size_t checked = IsDlgButtonChecked(IDC_PLAY_MUSIC);
      GetDlgItem(IDC_MUSIC_SLIDER).EnableWindow(checked == BST_CHECKED);
      GetDlgItem(IDC_STATIC_MUSIC).EnableWindow(checked == BST_CHECKED);
      break;
   }
   case IDC_PLAY_SOUND:
   {
      const size_t checked = IsDlgButtonChecked(IDC_PLAY_SOUND);
      GetDlgItem(IDC_SOUND_SLIDER).EnableWindow(checked == BST_CHECKED);
      GetDlgItem(IDC_STATIC_SOUND).EnableWindow(checked == BST_CHECKED);
      break;
   }
   default: 
      return FALSE;
   }
   return TRUE;
}

Settings& AudioOptionsDialog::GetEditedSettings()
{
   if (g_pvp->m_ptableActive && IsDlgButtonChecked(IDC_TABLE_OVERRIDE) == BST_CHECKED)
      return m_tableSettings;
   SendDlgItemMessage(IDC_APPLICATION_SETTINGS, BM_SETCHECK, BST_CHECKED, 0);
   return m_appSettings;
}

void AudioOptionsDialog::LoadSettings()
{
   Settings& settings = GetEditedSettings();
   // persist user edition to local copy of settings
   if (m_editedSettings != nullptr && m_editedSettings != &settings)
      SaveSettings(m_editedSettings == &m_appSettings);
   m_editedSettings = &settings;

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
   if (m_editedSettings == nullptr)
      return;
   Settings& settings = *m_editedSettings;

   size_t checked = IsDlgButtonChecked(IDC_PLAY_MUSIC);
   settings.SaveValue(Settings::Player, "PlayMusic"s, checked == BST_CHECKED, !saveAll);

   checked = IsDlgButtonChecked(IDC_PLAY_SOUND);
   settings.SaveValue(Settings::Player, "PlaySound"s, (checked == BST_CHECKED), !saveAll);

   if (m_editedSettings == &m_appSettings)
   {
      int fmusic = SNDCFG_SND3D2CH;
      if (IsDlgButtonChecked(IDC_RADIO_SND3DALLREAR) == BST_CHECKED)
         fmusic = SNDCFG_SND3DALLREAR;
      if (IsDlgButtonChecked(IDC_RADIO_SND3DFRONTISFRONT) == BST_CHECKED)
         fmusic = SNDCFG_SND3DFRONTISFRONT;
      if (IsDlgButtonChecked(IDC_RADIO_SND3DFRONTISREAR) == BST_CHECKED)
         fmusic = SNDCFG_SND3DFRONTISREAR;
      if (IsDlgButtonChecked(IDC_RADIO_SND3D6CH) == BST_CHECKED)
         fmusic = SNDCFG_SND3D6CH;
      if (IsDlgButtonChecked(IDC_RADIO_SND3DSSF) == BST_CHECKED)
         fmusic = SNDCFG_SND3DSSF;
      settings.SaveValue(Settings::Player, "Sound3D"s, fmusic, !saveAll);
   }

   size_t volume = SendDlgItemMessage(IDC_MUSIC_SLIDER, TBM_GETPOS, 0, 0);
   settings.SaveValue(Settings::Player, "MusicVolume"s, (int)volume, !saveAll);

   volume = SendDlgItemMessage(IDC_SOUND_SLIDER, TBM_GETPOS, 0, 0);
   settings.SaveValue(Settings::Player, "SoundVolume"s, (int)volume, !saveAll);

   if (m_editedSettings == &m_appSettings)
   {
      size_t soundindex = SendDlgItemMessage(IDC_SoundList, LB_GETCURSEL, 0, 0);
      size_t sd = SendDlgItemMessage(IDC_SoundList, LB_GETITEMDATA, soundindex, 0);
      settings.SaveValue(Settings::Player, "SoundDevice"s, (int)sd, !saveAll);

      soundindex = SendDlgItemMessage(IDC_SoundListBG, LB_GETCURSEL, 0, 0);
      sd = SendDlgItemMessage(IDC_SoundListBG, LB_GETITEMDATA, soundindex, 0);
      settings.SaveValue(Settings::Player, "SoundDeviceBG"s, (int)sd, !saveAll);
   }
   
   g_pvp->ReInitSound();
}
