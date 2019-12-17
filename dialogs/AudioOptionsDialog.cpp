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
   bool fsound = LoadValueBoolWithDefault("Player", "PlayMusic", true);

   HWND hwndControl = GetDlgItem(IDC_PLAY_MUSIC).GetHwnd();
   SendMessage(hwndControl, BM_SETCHECK, fsound ? BST_CHECKED : BST_UNCHECKED, 0);
   if (!fsound)
   {
      ::EnableWindow(hwndMusicSlider, FALSE);
      ::EnableWindow(GetDlgItem(IDC_STATIC_MUSIC).GetHwnd(), FALSE);
   }

   fsound = LoadValueBoolWithDefault("Player", "PlaySound", true);

   hwndControl = GetDlgItem(IDC_PLAY_SOUND).GetHwnd();
   SendMessage(hwndControl, BM_SETCHECK, fsound ? BST_CHECKED : BST_UNCHECKED, 0);
   if (!fsound)
   {
      ::EnableWindow(hwndSoundSlider, FALSE);
      ::EnableWindow(GetDlgItem(IDC_STATIC_SOUND).GetHwnd(), FALSE);
   }

   int fmusic = LoadValueIntWithDefault("Player", "Sound3D", 0);

   switch (fmusic)
   {
   case SNDCFG_SND3DALLREAR:
	   hwndControl = GetDlgItem(IDC_RADIO_SND3DALLREAR).GetHwnd();
	   SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3DFRONTISFRONT:
	   hwndControl = GetDlgItem(IDC_RADIO_SND3DFRONTISFRONT).GetHwnd();
	   SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3DFRONTISREAR:
	   hwndControl = GetDlgItem(IDC_RADIO_SND3DFRONTISREAR).GetHwnd();
	   SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
	   break;
   case SNDCFG_SND3D6CH:
	   hwndControl = GetDlgItem(IDC_RADIO_SND3D6CH).GetHwnd();
	   SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
	   break;
   default:
	   hwndControl = GetDlgItem(IDC_RADIO_SND3D2CH).GetHwnd();
	   SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
	   break;
   }

   fmusic = LoadValueIntWithDefault("Player", "MusicVolume", 100);
   ::SendMessage(hwndMusicSlider, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
   ::SendMessage(hwndMusicSlider, TBM_SETTICFREQ, 10, 0);
   ::SendMessage(hwndMusicSlider, TBM_SETLINESIZE, 0, 1);
   ::SendMessage(hwndMusicSlider, TBM_SETPAGESIZE, 0, 10);
   ::SendMessage(hwndMusicSlider, TBM_SETTHUMBLENGTH, 10, 0);
   ::SendMessage(hwndMusicSlider, TBM_SETPOS, TRUE, fmusic);

   fmusic = LoadValueIntWithDefault("Player", "SoundVolume", 100);
   ::SendMessage(hwndSoundSlider, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
   ::SendMessage(hwndSoundSlider, TBM_SETTICFREQ, 10, 0);
   ::SendMessage(hwndSoundSlider, TBM_SETLINESIZE, 0, 1);
   ::SendMessage(hwndSoundSlider, TBM_SETPAGESIZE, 0, 10);
   ::SendMessage(hwndSoundSlider, TBM_SETTHUMBLENGTH, 10, 0);
   ::SendMessage(hwndSoundSlider, TBM_SETPOS, TRUE, fmusic);

   const int sd = LoadValueIntWithDefault("Player", "SoundDevice", 0);
   const int sdbg = LoadValueIntWithDefault("Player", "SoundDeviceBG", 0);
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
   HWND hwndControl;
   size_t checked;

   hwndControl = GetDlgItem(IDC_PLAY_MUSIC).GetHwnd();
   checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
   SaveValueBool("Player", "PlayMusic", (checked == BST_CHECKED));

   hwndControl = GetDlgItem(IDC_PLAY_SOUND).GetHwnd();
   checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
   SaveValueBool("Player", "PlaySound", (checked == BST_CHECKED));

   int fmusic;
   fmusic = SNDCFG_SND3D2CH;
   hwndControl = GetDlgItem(IDC_RADIO_SND3DALLREAR).GetHwnd();
   checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DALLREAR;
   }
   hwndControl = GetDlgItem(IDC_RADIO_SND3DFRONTISFRONT).GetHwnd();
   checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DFRONTISFRONT;
   }
   hwndControl = GetDlgItem(IDC_RADIO_SND3DFRONTISREAR).GetHwnd();
   checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3DFRONTISREAR;
   }
   hwndControl = GetDlgItem(IDC_RADIO_SND3D6CH).GetHwnd();
   checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
   if (checked)
   {
	   fmusic = SNDCFG_SND3D6CH;
   }
   SaveValueInt("Player", "Sound3D", fmusic);

   size_t volume;
   volume = SendMessage(GetDlgItem(IDC_MUSIC_SLIDER).GetHwnd(), TBM_GETPOS, 0, 0);
   SaveValueInt("Player", "MusicVolume", (int)volume);

   volume = SendMessage(GetDlgItem(IDC_SOUND_SLIDER).GetHwnd(), TBM_GETPOS, 0, 0);
   SaveValueInt("Player", "SoundVolume", (int)volume);

   HWND hwndSoundList = GetDlgItem(IDC_SoundList).GetHwnd();
   size_t soundindex = SendMessage(hwndSoundList, LB_GETCURSEL, 0, 0);
   size_t sd = SendMessage(hwndSoundList, LB_GETITEMDATA, soundindex, 0);
   SaveValueInt("Player", "SoundDevice", (int)sd);
   hwndSoundList = GetDlgItem(IDC_SoundListBG).GetHwnd();
   soundindex = SendMessage(hwndSoundList, LB_GETCURSEL, 0, 0);
   sd = SendMessage(hwndSoundList, LB_GETITEMDATA, soundindex, 0);
   SaveValueInt("Player", "SoundDeviceBG", (int)sd);
   g_pvp->ReInitSound();

   CDialog::OnOK();
}

void AudioOptionsDialog::OnClose()
{
   SendMessage(RESET_SOUNDLIST_CONTENT, 0, 0);
}
