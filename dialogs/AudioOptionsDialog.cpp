#include "stdafx.h"
#include "resource.h"
#include "AudioOptionsDialog.h"

#define GET_SOUNDDEVICES		WM_USER+103
#define RESET_SOUNDLIST_CONTENT	WM_USER+104

AudioOptionsDialog::AudioOptionsDialog() : CDialog(IDD_AUDIO_OPTIONS)
{
}

BOOL AudioOptionsDialog::OnInitDialog()
{
   const HWND hwndMusicSlider = GetDlgItem(IDC_MUSIC_SLIDER).GetHwnd();
   const HWND hwndSoundSlider = GetDlgItem(IDC_SOUND_SLIDER).GetHwnd();
   bool fsound = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlayMusic"s, true);

   SendMessage(GetDlgItem(IDC_PLAY_MUSIC).GetHwnd(), BM_SETCHECK, fsound ? BST_CHECKED : BST_UNCHECKED, 0);
   if (!fsound)
   {
      ::EnableWindow(hwndMusicSlider, FALSE);
      ::EnableWindow(GetDlgItem(IDC_STATIC_MUSIC).GetHwnd(), FALSE);
   }

   fsound = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlaySound"s, true);

   SendMessage(GetDlgItem(IDC_PLAY_SOUND).GetHwnd(), BM_SETCHECK, fsound ? BST_CHECKED : BST_UNCHECKED, 0);
   if (!fsound)
   {
      ::EnableWindow(hwndSoundSlider, FALSE);
      ::EnableWindow(GetDlgItem(IDC_STATIC_SOUND).GetHwnd(), FALSE);
   }

   int fmusic = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, 0);

   switch (fmusic)
   {
   case SNDCFG_SND3DALLREAR:
	   SendMessage(GetDlgItem(IDC_RADIO_SND3DALLREAR).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3DFRONTISFRONT:
	   SendMessage(GetDlgItem(IDC_RADIO_SND3DFRONTISFRONT).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3DFRONTISREAR:
	   SendMessage(GetDlgItem(IDC_RADIO_SND3DFRONTISREAR).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3D6CH:
	   SendMessage(GetDlgItem(IDC_RADIO_SND3D6CH).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3DSSF:
	   SendMessage(GetDlgItem(IDC_RADIO_SND3DSSF).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
	   break;
   default:
	   SendMessage(GetDlgItem(IDC_RADIO_SND3D2CH).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
	   break;
   }

   fmusic = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
   ::SendMessage(hwndMusicSlider, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
   ::SendMessage(hwndMusicSlider, TBM_SETTICFREQ, 10, 0);
   ::SendMessage(hwndMusicSlider, TBM_SETLINESIZE, 0, 1);
   ::SendMessage(hwndMusicSlider, TBM_SETPAGESIZE, 0, 10);
   ::SendMessage(hwndMusicSlider, TBM_SETTHUMBLENGTH, 10, 0);
   ::SendMessage(hwndMusicSlider, TBM_SETPOS, TRUE, fmusic);

   fmusic = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);
   ::SendMessage(hwndSoundSlider, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
   ::SendMessage(hwndSoundSlider, TBM_SETTICFREQ, 10, 0);
   ::SendMessage(hwndSoundSlider, TBM_SETLINESIZE, 0, 1);
   ::SendMessage(hwndSoundSlider, TBM_SETPAGESIZE, 0, 10);
   ::SendMessage(hwndSoundSlider, TBM_SETTHUMBLENGTH, 10, 0);
   ::SendMessage(hwndSoundSlider, TBM_SETPOS, TRUE, fmusic);

   const int sd = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SoundDevice"s, 0);
   const int sdbg = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SoundDeviceBG"s, 0);
   SendMessage(GET_SOUNDDEVICES, sd, sdbg);

   return TRUE;
}

INT_PTR AudioOptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case GET_SOUNDDEVICES:
      {
         SendMessage(RESET_SOUNDLIST_CONTENT, 0, 0);
         const HWND hwndList = GetDlgItem(IDC_SoundList).GetHwnd();
         const HWND hwndListBG = GetDlgItem(IDC_SoundListBG).GetHwnd();

         DSAudioDevices DSads;
         if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
         {
            for (size_t i = 0; i < DSads.size(); i++)
            {
               const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)DSads[i]->description.c_str());
               SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)i);
               const size_t indexbg = SendMessage(hwndListBG, LB_ADDSTRING, 0, (size_t)DSads[i]->description.c_str());
               SendMessage(hwndListBG, LB_SETITEMDATA, indexbg, (LPARAM)i);
               delete DSads[i];
            }
         }

         SendMessage(hwndList, LB_SETCURSEL, (wParam < DSads.size()) ? wParam : 0, 0);
         SendMessage(hwndListBG, LB_SETCURSEL, ((size_t)lParam < DSads.size()) ? lParam : 0, 0);
         break;
      }
      case RESET_SOUNDLIST_CONTENT:
      {
         SendMessage(GetDlgItem(IDC_SoundList).GetHwnd(), LB_RESETCONTENT, 0, 0);
         SendMessage(GetDlgItem(IDC_SoundListBG).GetHwnd(), LB_RESETCONTENT, 0, 0);
      }
      break;
   }

   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL AudioOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_PLAY_MUSIC:
      {
         const size_t checked = SendDlgItemMessage(IDC_PLAY_MUSIC, BM_GETCHECK, 0, 0);
         ::EnableWindow(GetDlgItem(IDC_MUSIC_SLIDER).GetHwnd(), (checked == BST_CHECKED));
         ::EnableWindow(GetDlgItem(IDC_STATIC_MUSIC).GetHwnd(), (checked == BST_CHECKED));
         break;
      }
      case IDC_PLAY_SOUND:
      {
         const size_t checked = SendDlgItemMessage(IDC_PLAY_SOUND, BM_GETCHECK, 0, 0);
         ::EnableWindow(GetDlgItem(IDC_SOUND_SLIDER).GetHwnd(), (checked == BST_CHECKED));
         ::EnableWindow(GetDlgItem(IDC_STATIC_SOUND).GetHwnd(), (checked == BST_CHECKED));
         break;
      }
      default: 
         return FALSE;
   }
   return TRUE;
}

void AudioOptionsDialog::OnOK()
{
   size_t checked;

   checked = SendMessage(GetDlgItem(IDC_PLAY_MUSIC).GetHwnd(), BM_GETCHECK, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::Player, "PlayMusic"s, (checked == BST_CHECKED));

   checked = SendMessage(GetDlgItem(IDC_PLAY_SOUND).GetHwnd(), BM_GETCHECK, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::Player, "PlaySound"s, (checked == BST_CHECKED));

   int fmusic = SNDCFG_SND3D2CH;
   checked = SendMessage(GetDlgItem(IDC_RADIO_SND3DALLREAR).GetHwnd(), BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DALLREAR;
   }
   checked = SendMessage(GetDlgItem(IDC_RADIO_SND3DFRONTISFRONT).GetHwnd(), BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DFRONTISFRONT;
   }
   checked = SendMessage(GetDlgItem(IDC_RADIO_SND3DFRONTISREAR).GetHwnd(), BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DFRONTISREAR;
   }
   checked = SendMessage(GetDlgItem(IDC_RADIO_SND3D6CH).GetHwnd(), BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3D6CH;
   }
   checked = SendMessage(GetDlgItem(IDC_RADIO_SND3DSSF).GetHwnd(), BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DSSF;
   }
   g_pvp->m_settings.SaveValue(Settings::Player, "Sound3D"s, fmusic);

   size_t volume = SendMessage(GetDlgItem(IDC_MUSIC_SLIDER).GetHwnd(), TBM_GETPOS, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::Player, "MusicVolume"s, (int)volume);

   volume = SendMessage(GetDlgItem(IDC_SOUND_SLIDER).GetHwnd(), TBM_GETPOS, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::Player, "SoundVolume"s, (int)volume);

   HWND hwndSoundList = GetDlgItem(IDC_SoundList).GetHwnd();
   size_t soundindex = SendMessage(hwndSoundList, LB_GETCURSEL, 0, 0);
   size_t sd = SendMessage(hwndSoundList, LB_GETITEMDATA, soundindex, 0);
   g_pvp->m_settings.SaveValue(Settings::Player, "SoundDevice"s, (int)sd);
   hwndSoundList = GetDlgItem(IDC_SoundListBG).GetHwnd();
   soundindex = SendMessage(hwndSoundList, LB_GETCURSEL, 0, 0);
   sd = SendMessage(hwndSoundList, LB_GETITEMDATA, soundindex, 0);
   g_pvp->m_settings.SaveValue(Settings::Player, "SoundDeviceBG"s, (int)sd);
   g_pvp->ReInitSound();

   CDialog::OnOK();
}

void AudioOptionsDialog::OnClose()
{
   SendMessage(RESET_SOUNDLIST_CONTENT, 0, 0);
}
