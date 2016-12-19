#include "StdAfx.h"

static HMIXER m_hMixer;
//static MIXERCAPS sMxCaps;
static DWORD m_dwMinimum;
static DWORD m_dwMaximum;
static DWORD m_dwVolumeControlID;
static int nmixers;
static F32 gMixerVolume;
static U32 gMixerVolumeStamp = 0;
// used extern in pininput:
bool gMixerKeyDown;
bool gMixerKeyUp;

const F32 volume_adjustment_bar_pos[2] = { (float)(15.0 / 1000.0), (float)(500.0 / 1000.0) };
const F32 volume_adjustment_bar_big_size[2] = { (float)(20.0 / 1000.0), (float)(4.0 / 1000.0) };
const F32 volume_adjustment_bar_small_size[2] = { (float)(10.0 / 1000.0), (float)(2.0 / 1000.0) };
const F32 volume_adjustment_bar_ysize = (float)(720.0 / 1000.0);
const U32 volume_adjustment_color[3] = { 0x00ff00, 0xffff, 0xff };

BOOL mixer_init(const HWND wnd)
{
   // get the number of mixer devices present in the system
   nmixers = ::mixerGetNumDevs();

   if (!nmixers)
      return 0;

   m_hMixer = NULL;
   //ZeroMemory(&sMxCaps, sizeof(MIXERCAPS));

   m_dwMinimum = 0;
   m_dwMaximum = 0;
   m_dwVolumeControlID = 0;

   // open the first mixer
   // A "mapper" for audio mixer devices does not currently exist.
   if (nmixers != 0)
   {
      if (::mixerOpen(&m_hMixer,
         0,
         reinterpret_cast<size_t>(wnd),
         NULL,
         MIXER_OBJECTF_MIXER | CALLBACK_WINDOW)
         != MMSYSERR_NOERROR)
      {
         return 0;
      }

      /*if (::mixerGetDevCaps(reinterpret_cast<UINT>(m_hMixer),
                       &sMxCaps, sizeof(MIXERCAPS))
                       != MMSYSERR_NOERROR)
                       {
                       return 0;
                       }*/
   }

   MIXERLINE mxl;
   mxl.cbStruct = sizeof(MIXERLINE);
   mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
   if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMixer),
      &mxl,
      MIXER_OBJECTF_HMIXER |
      MIXER_GETLINEINFOF_COMPONENTTYPE)
      != MMSYSERR_NOERROR)
   {
      return 0;
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
      return 0;
   }

   // store dwControlID
   m_dwMinimum = mxc.Bounds.dwMinimum;
   m_dwMaximum = mxc.Bounds.dwMaximum;
   m_dwVolumeControlID = mxc.dwControlID;

   mixer_get_volume();

   return 1;
}

void mixer_shutdown()
{
   ::mixerClose(m_hMixer);
   nmixers = 0;
}

void mixer_get_volume()
{
   if (!m_hMixer || !nmixers)
      return;

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
      gMixerVolume = sqrtf((F32)(mxcdVolume.dwValue - m_dwMinimum) / (F32)(m_dwMaximum - m_dwMinimum));

   if (g_pplayer->m_ptable->m_tblVolmod != 0.0f)
      gMixerVolume /= g_pplayer->m_ptable->m_tblVolmod;
   else
      gMixerVolume = 0.01f; // mute is impossible
}

void mixer_update()
{
   if (!nmixers || !m_hMixer)
      return;

   const F32 delta = (F32)(1.0 / 500.0);

   float vol;
   if (gMixerKeyDown)
       vol = gMixerVolume - delta;
   else if (gMixerKeyUp)
       vol = gMixerVolume + delta;
   else
       return;

   if (vol < 0.01f) vol = 0.01f; //hardcap minimum
   if (vol > 1.0f) vol = 1.0f;   //hardcap maximum

   gMixerVolumeStamp = g_pplayer->m_time_msec;

   if (vol == gMixerVolume)
       return;

   gMixerVolume = vol;

   F32 modded_volume = gMixerVolume * g_pplayer->m_ptable->m_tblVolmod;

   if (modded_volume < 0.01f)
      modded_volume = 0.01f; //hardcap minimum
   if (modded_volume > 1.0f)
      modded_volume = 1.0f;  //hardcap maximum

   DWORD dwVal = (DWORD)((F32)m_dwMinimum + (modded_volume * modded_volume) * (F32)(m_dwMaximum - m_dwMinimum));

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
}

void mixer_draw()
{
   if (!gMixerVolumeStamp)
      return;

   F32 fade = 1.0f - (F32)(g_pplayer->m_time_msec - gMixerVolumeStamp) * 0.001f;
   if (fade > 1.0f)
      fade = 1.0f;
   if (fade <= 0.0f)
   {
      gMixerVolumeStamp = 0;
      return;
   }

   const bool cabMode = fmodf(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set], 360.f) != 0.f;

   if (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
      g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

   g_pplayer->m_pin3d.EnableAlphaBlend(true);

   fade *= (float)(222.2 / 255.0);

   const F32 yoff = volume_adjustment_bar_big_size[1] * 2.0f;

   for (F32 vol = 0.f, y = -volume_adjustment_bar_ysize * 0.5f;
      vol < 1.0f;
      vol += yoff / volume_adjustment_bar_ysize, y += yoff)
   {
      U32 color;
      F32 size[2];
      if (vol > gMixerVolume)
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
      /*		// Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
            g_pplayer->Spritedraw( (cabMode ? fX : fY) - (float)(1.0/1000.0), (cabMode ? fY : fX) - (float)(1.0/1000.0),
            (cabMode ? size[0] : size[1]) + (float)(2.0/1000.0), (cabMode ? size[1] : size[0]) + (float)(2.0/1000.0),
            drop_color,
            (Texture*)NULL,
            fade);
            */
      // Set the color.
      // Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
      g_pplayer->Spritedraw(cabMode ? fX : fY, cabMode ? fY : fX,
         cabMode ? size[0] : size[1], cabMode ? size[1] : size[0],
         color,
         (Texture*)NULL,
         fade);
   }

   if (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
      g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}
