#include "stdafx.h"

#include <mmsystem.h>

static HMIXER m_hMixer;
//static MIXERCAPS sMxCaps;
static DWORD m_dwMinimum;
static DWORD m_dwMaximum;
static DWORD m_dwVolumeControlID;
static int nmixers;
static F32 m_mixerVolume;
static U32 m_mixerVolumeStamp = 0;

constexpr F32 volume_adjustment_bar_pos[2] = { (float)(15.0 / 1000.0), (float)(500.0 / 1000.0) };
constexpr F32 volume_adjustment_bar_big_size[2] = { (float)(20.0 / 1000.0), (float)(4.0 / 1000.0) };
constexpr F32 volume_adjustment_bar_small_size[2] = { (float)(10.0 / 1000.0), (float)(2.0 / 1000.0) };
constexpr F32 volume_adjustment_bar_ysize = (float)(720.0 / 1000.0);
constexpr U32 volume_adjustment_color[3] = { 0x00ff00, 0xffff, 0xff };

bool mixer_init(const HWND wnd)
{
#ifndef __STANDALONE__
   // get the number of mixer devices present in the system
   nmixers = ::mixerGetNumDevs();

   if (!nmixers)
      return false;

   m_hMixer = nullptr;
   //ZeroMemory(&sMxCaps, sizeof(MIXERCAPS));

   m_dwMinimum = 0;
   m_dwMaximum = 0;
   m_dwVolumeControlID = 0;

   // open the first mixer
   // A "mapper" for audio mixer devices does not currently exist.
      if (::mixerOpen(&m_hMixer,
         0,
         reinterpret_cast<size_t>(wnd),
         0,
         MIXER_OBJECTF_MIXER | CALLBACK_WINDOW)
         != MMSYSERR_NOERROR)
      {
         return false;
      }

      /*if (::mixerGetDevCaps(reinterpret_cast<UINT>(m_hMixer),
                       &sMxCaps, sizeof(MIXERCAPS))
                       != MMSYSERR_NOERROR)
                       {
                       return false;
                       }*/

   MIXERLINE mxl;
   mxl.cbStruct = sizeof(MIXERLINE);
   mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
   if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMixer),
      &mxl,
      MIXER_OBJECTF_HMIXER |
      MIXER_GETLINEINFOF_COMPONENTTYPE)
      != MMSYSERR_NOERROR)
   {
      return false;
   }

   MIXERCONTROL mxc;
   MIXERLINECONTROLS mxlc;
   mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
   mxlc.dwLineID = mxl.dwLineID;
   mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
   mxlc.cControls = 1;
   mxlc.cbmxctrl = sizeof(MIXERCONTROL);
   mxlc.pamxctrl = &mxc;
   if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_hMixer),
      &mxlc,
      MIXER_OBJECTF_HMIXER |
      MIXER_GETLINECONTROLSF_ONEBYTYPE)
      != MMSYSERR_NOERROR)
   {
      return false;
   }

   // store dwControlID
   m_dwMinimum = mxc.Bounds.dwMinimum;
   m_dwMaximum = mxc.Bounds.dwMaximum;
   m_dwVolumeControlID = mxc.dwControlID;

   mixer_get_volume();
#endif

   return true;
}

void mixer_shutdown()
{
#ifndef __STANDALONE__
   ::mixerClose(m_hMixer);
#endif
   nmixers = 0;
}

void mixer_get_volume()
{
   if (!m_hMixer || !nmixers)
      return;

#ifndef __STANDALONE__
   MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
   MIXERCONTROLDETAILS mxcd;
   mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
   mxcd.dwControlID = m_dwVolumeControlID;
   mxcd.cChannels = 1;
   mxcd.cMultipleItems = 0;
   mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
   mxcd.paDetails = &mxcdVolume;
   if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
      &mxcd,
      MIXER_OBJECTF_HMIXER |
      MIXER_GETCONTROLDETAILSF_VALUE)
      != MMSYSERR_NOERROR)
   {
      return;
   }

   if (m_dwMaximum > m_dwMinimum)
      m_mixerVolume = sqrtf((F32)(mxcdVolume.dwValue - m_dwMinimum) / (F32)(m_dwMaximum - m_dwMinimum));
#endif

   if (g_pplayer->m_ptable->m_tblVolmod != 0.0f)
      m_mixerVolume /= g_pplayer->m_ptable->m_tblVolmod;
   else
      m_mixerVolume = 0.01f; // mute is impossible
}

void mixer_update()
{
   if (!nmixers || !m_hMixer)
      return;

   constexpr F32 delta = (F32)(1.0 / 500.0);

   float vol;
   if (g_pplayer->m_pininput.m_mixerKeyDown)
       vol = m_mixerVolume - delta;
   else if (g_pplayer->m_pininput.m_mixerKeyUp)
       vol = m_mixerVolume + delta;
   else
       return;

   if (vol < 0.01f) vol = 0.01f; //hardcap minimum
   if (vol > 1.0f) vol = 1.0f;   //hardcap maximum

   m_mixerVolumeStamp = g_pplayer->m_time_msec;

   if (vol == m_mixerVolume)
       return;

   m_mixerVolume = vol;

   F32 modded_volume = m_mixerVolume * g_pplayer->m_ptable->m_tblVolmod;

   if (modded_volume < 0.01f)
      modded_volume = 0.01f; //hardcap minimum
   if (modded_volume > 1.0f)
      modded_volume = 1.0f;  //hardcap maximum

#ifndef __STANDALONE__
   const DWORD dwVal = (DWORD)((F32)m_dwMinimum + (modded_volume * modded_volume) * (F32)(m_dwMaximum - m_dwMinimum));

   MIXERCONTROLDETAILS_UNSIGNED mxcdVolume = { dwVal };
   MIXERCONTROLDETAILS mxcd;
   mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
   mxcd.dwControlID = m_dwVolumeControlID;
   mxcd.cChannels = 1;
   mxcd.cMultipleItems = 0;
   mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
   mxcd.paDetails = &mxcdVolume;
   if (::mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
      &mxcd,
      MIXER_OBJECTF_HMIXER |
      MIXER_SETCONTROLDETAILSF_VALUE)
      != MMSYSERR_NOERROR)
   {
      return;
   }
#endif
}

void mixer_draw()
{
   if (!m_mixerVolumeStamp)
      return;

   F32 fade = 1.0f - (F32)(g_pplayer->m_time_msec - m_mixerVolumeStamp) * 0.001f;
   if (fade > 1.0f)
      fade = 1.0f;
   if (fade <= 0.0f)
   {
      m_mixerVolumeStamp = 0;
      return;
   }

   const bool cabMode = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].mMode == VLM_WINDOW || fmodf(g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].mViewportRotation, 360.f) != 0.f;

   fade *= (float)(222.2 / 255.0);

   constexpr F32 yoff = volume_adjustment_bar_big_size[1] * 2.0f;

   for (F32 vol = 0.f, y = -volume_adjustment_bar_ysize * 0.5f;
      vol < 1.0f;
      vol += yoff / volume_adjustment_bar_ysize, y += yoff)
   {
      U32 color;
      F32 size[2];
      if (vol > m_mixerVolume)
      {
         size[0] = volume_adjustment_bar_small_size[0];
         size[1] = volume_adjustment_bar_small_size[1];

         color = volume_adjustment_color[0];
      }
      else
      {
         size[0] = volume_adjustment_bar_big_size[0];
         size[1] = volume_adjustment_bar_big_size[1];

         if (vol < 0.75f)
            color = volume_adjustment_color[0];
         else if (vol < 0.90f)
            color = volume_adjustment_color[1];
         else
            color = volume_adjustment_color[2];
      }

      // Set the position.  
      const float fX = 1.0f - (volume_adjustment_bar_pos[0] + size[0]);
      const float fY = 1.0f - (volume_adjustment_bar_pos[1] + size[1] + y);

      // Set the color.
      /*// Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
            g_pplayer->Spritedraw( (cabMode ? fX : fY) - (float)(1.0/1000.0), (cabMode ? fY : fX) - (float)(1.0/1000.0),
            (cabMode ? size[0] : size[1]) + (float)(2.0/1000.0), (cabMode ? size[1] : size[0]) + (float)(2.0/1000.0),
            drop_color,
            (Texture*)nullptr,
            fade);
            */
      // Set the color.
      // Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->ResetRenderState();
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->EnableAlphaBlend(true);
      g_pplayer->Spritedraw(cabMode ? fX : fY, cabMode ? fY : fX,
         cabMode ? size[0] : size[1], cabMode ? size[1] : size[0],
         color, (Texture*)nullptr, fade);
   }
}
