#include "StdAfx.h"

static HMIXER m_hMixer;
static MIXERCAPS sMxCaps;
static DWORD m_dwMinimum;
static DWORD m_dwMaximum;
static DWORD m_dwVolumeControlID;
static F32 gMixerVolume;
static int nmixers;
static F32 fade = 1.0f;
static U32 volume_stamp;
static bool m_bUpdate;

int mixer_init( HWND wnd )
{
	// get the number of mixer devices present in the system
	nmixers = ::mixerGetNumDevs();

	if( !nmixers ) return 0;

	m_hMixer = NULL;
	::ZeroMemory(&sMxCaps, sizeof(MIXERCAPS));

	m_dwMinimum = 0;
	m_dwMaximum = 0;
	m_dwVolumeControlID = 0;

	// open the first mixer
	// A "mapper" for audio mixer devices does not currently exist.
	if (nmixers != 0)
	{
		if (::mixerOpen(&m_hMixer,
						0,
						reinterpret_cast<DWORD>(wnd),
						NULL,
						MIXER_OBJECTF_MIXER | CALLBACK_WINDOW)
			!= MMSYSERR_NOERROR)
		{
			return 0;
		}

		if (::mixerGetDevCaps(reinterpret_cast<UINT>(m_hMixer),
							  &sMxCaps, sizeof(MIXERCAPS))
			!= MMSYSERR_NOERROR)
		{
			return 0;
		}
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

	gMixerVolume = mixer_get_volume();

	return 1;
}

void mixer_display_volume( void )
{
	volume_stamp = msec();
}

void mixer_shutdown( void )
{
	::mixerClose(m_hMixer);
	nmixers = 0;
}

static F32 volume_modulation = 1.0f;

static void set_cp_master_volume( void )
{
    F32 modded_volume = gMixerVolume * volume_modulation;

    if( modded_volume < 0.0f ) modded_volume = 0.0f;
    if( modded_volume > 1.0f ) modded_volume = 1.0f;

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

void mixer_volmod( F32 volmod )
{
    volume_modulation = volmod;

    mixer_get_volume();
}

void mixer_volume( F32 vol )
{
	if( !nmixers ) return;

	if( vol < 0.01f ) vol = 0.01f;
	if( vol >= 1 ) vol = 1;

	if( vol == gMixerVolume ) return;

	mixer_display_volume();

	if (!m_hMixer)
	{
		return;
	}

	if( vol > 1.0f ) vol = 1.0f;
	if( vol < 0.0f ) vol = 0.0f;

	gMixerVolume = vol;

    set_cp_master_volume();
}

F32 mixer_get_volume( void )
{
	if (m_hMixer == NULL || !nmixers)
	{
		return 0;
	}

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
		return 0;
	}
	
	if( m_dwMaximum > m_dwMinimum )
	{
		gMixerVolume = sqrt( ( ( F32 ) ( mxcdVolume.dwValue - m_dwMinimum ) ) / ( ( F32 ) ( m_dwMaximum - m_dwMinimum ) ) );
	}

    if( volume_modulation ) gMixerVolume /= volume_modulation;
    else gMixerVolume = 0.0f; // mute

	return gMixerVolume;
}

void mixer_update( void )
{

    F32 delta = 1.0f / 500.0f; 

    if( Down( PININ_VOL_DOWN ) )
    {
        mixer_volume( gMixerVolume - delta );
    }
    else if( Down( PININ_VOL_UP ) )
    {
        mixer_volume( gMixerVolume + delta );
    }

}


static F32 volume_adjustment_bar_pos[2] = { 15, 400 };
static F32 volume_adjustment_bar_big_size[2] = { 20, 4 };
static F32 volume_adjustment_bar_small_size[2] = { 10, 2 };
static F32 volume_adjustment_bar_ysize = 720.0f;
static U32 volume_adjustment_color[3] = { 0x00ff00df, 0xffff00df, 0xff0000df };
static U32 volume_adjustment_drop_color = 0x0000001f;

void mixer_draw( void )
{
	float	sX, sY;
	float	fX, fY;
	float	Width, Height;
	float	r, g, b, a;
	F32 fade = 0.0f;
	RenderStateType		RestoreRenderState;
	TextureStateType	RestoreTextureState;
	D3DMATRIX			RestoreWorldMatrix;
	HRESULT				ReturnCode;

	if( !volume_stamp ) return;
	fade = 1.0f - ( ( (F32) ( msec() - volume_stamp ) ) * 0.001f );

	m_bUpdate = true;

    if( fade > 1.0f ) fade = 1.0f;
    if( fade <= 0.0f )
	{
		volume_stamp = 0;
		return;
	}

	// Save the current transformation state.
	ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->GetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
	// Save the current render state.
	Display_GetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreRenderState));
	// Save the current texture state.
	Display_GetTextureState (g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreTextureState));

    U32 alpha = (U32) ( fade * 222.2f );

    F32 size[2];

    F32 ypos = (F32)( -((S32)g_pplayer->m_pin3d.m_dwRenderHeight/2) );
    F32 yoff = volume_adjustment_bar_big_size[1] * 2.0f;
    F32 y;
    F32 vol;

    for( vol = 0, y= - ( volume_adjustment_bar_ysize /2.0f );
            vol < 1.0f;
            vol += ( 1.0f * yoff / volume_adjustment_bar_ysize ), y += yoff )
    {
		U32 color = ( volume_adjustment_color[0] & 0xffffff00 ) | alpha;
        ypos += yoff;

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
			{
				color = ( volume_adjustment_color[0] & 0xffffff00 ) | alpha;
			}
			else if( vol < 0.90f )
			{
				color = ( volume_adjustment_color[1] & 0xffffff00 ) | alpha;
			}
			else
			{
				color = ( volume_adjustment_color[2] & 0xffffff00 ) | alpha;
			}
        }

		U32 drop_color = ( volume_adjustment_drop_color & 0xffffff00 ) | alpha;

//        draw_transparent_box( size[0]+2, size[1]+2, volume_adjustment_bar_pos[0], volume_adjustment_bar_pos[1]+y, drop_color );
//        draw_transparent_box( size[0], size[1], volume_adjustment_bar_pos[0], volume_adjustment_bar_pos[1]+y, color );

		// Calculate the scale.
		sX = -1.0f * (((float) g_pplayer->m_pin3d.m_dwRenderHeight)/600.0f);
		sY = -1.0f * (((float) g_pplayer->m_pin3d.m_dwRenderWidth)/800.0f);

		// Set the position.  
		fX = ((float) g_pplayer->m_pin3d.m_dwRenderHeight) + (volume_adjustment_bar_pos[0] * sX);
		fY = ((float) g_pplayer->m_pin3d.m_dwRenderWidth) + ((volume_adjustment_bar_pos[1] * sY) + (y * sY));

		// Set the width and height.
		Width = size[0] * sX;
		Height = size[1] * sY;

		// Set the color.
		r = ((float) ((drop_color & 0xff000000) >> 24)) / 255.0f;	
		g = ((float) ((drop_color & 0x00ff0000) >> 16)) / 255.0f;	
		b = ((float) ((drop_color & 0x0000ff00) >> 8)) / 255.0f;	
		a = ((float) ((drop_color & 0x000000ff) >> 0)) / 255.0f;	

		// Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
		Display_DrawSprite( g_pplayer->m_pin3d.m_pd3dDevice, 
							(fY + 1.0f), (fX + 1.0f),
							(Height - 2.0f), (Width - 2.0f), 
							r, g, b, a,
							0.0f,
							NULL, 1.0f, 1.0f,
							DISPLAY_TEXTURESTATE_NOFILTER, DISPLAY_RENDERSTATE_TRANSPARENT );

		// Set the color.
		r = ((float) ((color & 0xff000000) >> 24)) / 255.0f;	
		g = ((float) ((color & 0x00ff0000) >> 16)) / 255.0f;	
		b = ((float) ((color & 0x0000ff00) >> 8)) / 255.0f;	
		a = ((float) ((color & 0x000000ff) >> 0)) / 255.0f;	

		// Draw the tick mark.  (Reversed x and y to match coordinate system of front end.)
		Display_DrawSprite( g_pplayer->m_pin3d.m_pd3dDevice, 
							fY, fX,
							Height, Width, 
							r, g, b, a,
							0.0f,
							NULL, 1.0f, 1.0f,
							DISPLAY_TEXTURESTATE_NOFILTER, DISPLAY_RENDERSTATE_TRANSPARENT );
	}

	// Restore the render states.
	Display_SetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreRenderState));
	// Restore the texture state.
	Display_SetTextureState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreTextureState));
	// Restore the transformation state.
	ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
}

// Flags that the region behind the mixer volume should be refreshed.
void mixer_erase( void )
{
	float	sX;
	float	fX;
	float	Width;
	RECT	Rect;

	if( !m_bUpdate ) return;

	// Calculate the scale.
	sX = -1.0f * (((float) g_pplayer->m_pin3d.m_dwRenderHeight)/600.0f);

	// Set the position.  
	fX = ((float) g_pplayer->m_pin3d.m_dwRenderHeight) + (volume_adjustment_bar_pos[0] * sX);

	// Set the width and height.
	Width = volume_adjustment_bar_big_size[0] * sX;

	// Invalidate the window region to signal an update from the back buffer (after render is complete).
	Rect.top = (LONG) ((g_pplayer->m_pin3d.m_dwRenderHeight) + (volume_adjustment_bar_pos[0] * sX) + (Width));
	Rect.left = 0;			
	Rect.bottom = g_pplayer->m_pin3d.m_dwRenderHeight - 1;
	Rect.right = g_pplayer->m_pin3d.m_dwRenderWidth - 1;
	g_pplayer->InvalidateRect(&Rect);

	m_bUpdate = false;
}
