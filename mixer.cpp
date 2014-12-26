#include "StdAfx.h"

static HMIXER m_hMixer;
//static MIXERCAPS sMxCaps;
static DWORD m_dwMinimum;
static DWORD m_dwMaximum;
static DWORD m_dwVolumeControlID;
static F32 gMixerVolume;
static int nmixers;
static U32 volume_stamp = 0;

const F32 volume_adjustment_bar_pos[2] = { 15.0f, 400.0f };
const F32 volume_adjustment_bar_big_size[2] = { 20.0f, 4.0f };
const F32 volume_adjustment_bar_small_size[2] = { 10.0f, 2.0f };
const F32 volume_adjustment_bar_ysize = 720.0f;
const U32 volume_adjustment_color[3] = { 0x00ff00df, 0xffff00df, 0xff0000df };
const U32 volume_adjustment_drop_color = 0x0000001f;

BOOL mixer_init( const HWND wnd )
{
	// get the number of mixer devices present in the system
	nmixers = ::mixerGetNumDevs();

	if( !nmixers )
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
	
	if( m_dwMaximum > m_dwMinimum )
		gMixerVolume = sqrtf( ( F32 ) ( mxcdVolume.dwValue - m_dwMinimum ) / ( F32 ) ( m_dwMaximum - m_dwMinimum ) );

	if( g_pplayer->m_ptable->m_tblVolmod != 0.0f )
		gMixerVolume /= g_pplayer->m_ptable->m_tblVolmod;
    else
		gMixerVolume = 0.01f; // mute is impossible
}

void mixer_update(const PinInput &pininput)
{
	if(!nmixers || !m_hMixer)
		return;

	const F32 delta = (F32)(1.0 / 500.0);

	float vol;
    if( pininput.Down( PININ_VOL_DOWN ) )
        vol = gMixerVolume - delta;
    else if( pininput.Down( PININ_VOL_UP ) )
        vol = gMixerVolume + delta;
	else
		return;

	if(vol < 0.01f) vol = 0.01f;//hardcap minimum
	if(vol > 1.0f) vol = 1.0f;//hardcap maximum

	if(vol == gMixerVolume)
		return;

	gMixerVolume = vol;

	volume_stamp = g_pplayer->m_time_msec;

    F32 modded_volume = gMixerVolume * g_pplayer->m_ptable->m_tblVolmod;

    if( modded_volume < 0.01f )
		modded_volume = 0.01f; //hardcap minimum
    if( modded_volume > 1.0f )
		modded_volume = 1.0f; //hardcap maximum

	DWORD dwVal = (DWORD) ( ((F32)m_dwMinimum) + ( modded_volume * modded_volume ) * ((F32)(m_dwMaximum-m_dwMinimum)));

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
	if( !volume_stamp )
		return;

	F32 fade = 1.0f - ( ( (F32) ( g_pplayer->m_time_msec - volume_stamp ) ) * 0.001f );
    if( fade > 1.0f )
		fade = 1.0f;
    if( fade <= 0.0f )
	{
		volume_stamp = 0;
		return;
	}

	//RenderStateType	RestoreRenderState;
	//TextureStateType	RestoreTextureState;
	D3DMATRIX			RestoreWorldMatrix;

	// Save the current transformation state.
	g_pplayer->m_pin3d.m_pd3dDevice->GetTransform ( TRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
	// Save the current render state.
	//Display_GetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreRenderState));
	// Save the current texture state.
	//Display_GetTextureState (g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreTextureState));

    static /* const */ Matrix3D WorldMatrix; //(1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f);
    WorldMatrix.SetIdentity();
	g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( TRANSFORMSTATE_WORLD, (D3DMATRIX*)&WorldMatrix );

    const U32 alpha = (U32) ( fade * 222.2f );

    F32 ypos = (F32)( -((S32)g_pplayer->m_pin3d.m_dwRenderHeight/2) );
    const F32 yoff = volume_adjustment_bar_big_size[1] * 2.0f;

    for(F32 vol = 0.f, y= - ( volume_adjustment_bar_ysize*0.5f );
            vol < 1.0f;
            vol += ( 1.0f * yoff / volume_adjustment_bar_ysize ), y += yoff )
    {
		U32 color = ( volume_adjustment_color[0] & 0xffffff00 ) | alpha;
        ypos += yoff;

		F32 size[2];
        if( vol > gMixerVolume )
        {
            size[0] = volume_adjustment_bar_small_size[0];
            size[1] = volume_adjustment_bar_small_size[1];
        }
        else
        {
            size[0] = volume_adjustment_bar_big_size[0];
            size[1] = volume_adjustment_bar_big_size[1];

			if( vol < 0.75f )
				color = ( volume_adjustment_color[0] & 0xffffff00 ) | alpha;
			else if( vol < 0.90f )
				color = ( volume_adjustment_color[1] & 0xffffff00 ) | alpha;
			else
				color = ( volume_adjustment_color[2] & 0xffffff00 ) | alpha;
        }

		const U32 drop_color = ( volume_adjustment_drop_color & 0xffffff00 ) | alpha;

		// Calculate the scale.
		const float sX = - (float)g_pplayer->m_pin3d.m_dwRenderHeight*(float)(1.0/601.0); //changed from 600 to 601 to correct fadeout shadow lines
		const float sY = - (float)g_pplayer->m_pin3d.m_dwRenderWidth *(float)(1.0/800.0);

		// Set the position.  
		const float fX = (float)g_pplayer->m_pin3d.m_dwRenderHeight + (volume_adjustment_bar_pos[0] * sX);
		const float fY = (float)g_pplayer->m_pin3d.m_dwRenderWidth  + (volume_adjustment_bar_pos[1] * sY) + (y * sY);

		// Set the width and height.
		const float Width = size[0] * sX;
		const float Height = size[1] * sY;

		// Set the color.
		{
		const DWORD r = (drop_color             ) >> 24;
		const DWORD g = (drop_color & 0x00ff0000) >> 16;
		const DWORD b = (drop_color & 0x0000ff00) >>  8;
		const DWORD a = (drop_color & 0x000000ff)      ;
		const DWORD col = (a << 24) | (r << 16) | (g << 8) | b;

		// Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
		g_pplayer->Spritedraw( (fY + 1.0f), (fX + 1.0f),
							   (Height - 2.0f), (Width - 2.0f), 
							   col,
							   (Texture*)NULL); //!!
		}
		// Set the color.
		{
		const DWORD r = (color             ) >> 24;
		const DWORD g = (color & 0x00ff0000) >> 16;
		const DWORD b = (color & 0x0000ff00) >>  8;
		const DWORD a = (color & 0x000000ff)      ;
		const DWORD col = (a << 24) | (r << 16) | (g << 8) | b;

		// Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
		g_pplayer->Spritedraw( fY, fX,
							   Height, Width, 
							   col,
							   (Texture*)NULL); //!!
		}
	}

	// Restore the render states.
	//Display_SetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreRenderState));
	// Restore the texture state.
	//Display_SetTextureState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreTextureState));
	// Restore the transformation state.
	g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( TRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
}
