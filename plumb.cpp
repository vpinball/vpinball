#include "StdAfx.h"

// This comment is just plumb stupid
// What's your prob plumb? 
#undef  DEBUG_PLUMB

#define	VELOCITY_EPSILON 0.050f			// The threshold for zero velocity.


const F32 ac = 0.75f; // aspect ratio correction
static int tilted;

typedef struct
{
    F32 x,y;
    F32 vx,vy;
}
Plumb;

static Plumb gPlumb;
static F32 tiltsens;
static F32 nudgesens;

void plumb_set_sensitivity( F32 sens )
{
    tiltsens = sens;
}

void nudge_set_sensitivity( F32 sens )
{
    nudgesens = sens;
}

F32 nudge_get_sensitivity()
{
	return( nudgesens );
}

// If you modify this function... make sure you update the same function in the front-end!
//
// The physics on the plumb are not very accurate... but they seem to do good enough to convince players.
// In the real world, the plumb is a pendulum.  With force of gravity, the force of the string, the friction 
// force of the hook, and the dampening force of the air.  Here, force is exerted only by the table tilting 
// (the string) and dampening and friction forces are lumped together with a constant, and gravity is not 
// present.
void plumb_update(const U32 curr_time_msec)
{
    static U32 stamp = 0;
	// Get the time since the last frame.
    const F32 dt = (F32)((double)( curr_time_msec - stamp ) * 0.001);
    stamp = curr_time_msec;

	// Ignore large time slices... forces will get crazy!
	// Besides, we're probably loading/unloading.
    if( dt > 0.1f || dt <= 0.0f) 
	{
		return;
	}

    F32 &x = gPlumb.x;
    F32 &y = gPlumb.y;
    F32 &vx = gPlumb.vx;
    F32 &vy = gPlumb.vy;
    const F32 ax = sinf( ( GetX() - x ) * (float)( M_PI / 5.0 ) );
    const F32 ay = sinf( ( GetY() - y ) * (float)( M_PI / 5.0 ) );

	// Add force to the plumb.
    vx += (float)(825.0*0.25) * ax * dt;
    vy += (float)(825.0*0.25) * ay * dt;

	// Check if we hit the edge.
    const F32 len2 = x * x + y * y;
    if( len2 > ( 1.0f - tiltsens ) * ( 1.0f - tiltsens ) )
    {
		// Bounce the plumb and scrub velocity.
        const F32 oolen = (( 1.0f - tiltsens ) / sqrtf( len2 )) * 0.90f;
        x *= oolen;
        y *= oolen;
        vx *= -0.025f;
        vy *= -0.025f;

		// Check if tilt is enabled.
		if ( tiltsens > 0.0f )
		{
			// Flag that we tilted.
			tilted = 1;
		}
    }

	// Dampen the velocity.
    vx -= 2.50f * ( vx * dt );
    vy -= 2.50f * ( vy * dt );

	// Check if velocity is near zero and we near center.
	if ( ((vx + vy) > -VELOCITY_EPSILON) && ((vx + vy) < VELOCITY_EPSILON) &&
		 (x > -0.10f) && (x < 0.10f) && (y > -0.10f) && (y < 0.10f) )
	{
		// Set the velocity to zero.
		// This reduces annoying jittering when at rest.
		vx = 0.0f;
		vy = 0.0f;
	}

	// Update position.
    x += vx * dt;
    y += vy * dt;
}


int plumb_tilted()
{
    if( tilted )
    {
        tilted = 0;
        return 1;
    }

    return 0;
}

void draw_transparent_box( F32 sx, F32 sy, const F32 x, const F32 y, const U32 color )
{
    sx *= ((float) g_pplayer->m_pin3d.m_dwRenderHeight)*(float)(1.0/600.0);
    sy *= ((float) g_pplayer->m_pin3d.m_dwRenderWidth )*(float)(1.0/800.0);

	const F32 r = ((float) ((color             ) >> 24)) *(float)(1.0/255.0);
    const F32 g = ((float) ((color & 0x00ff0000) >> 16)) *(float)(1.0/255.0);
    const F32 b = ((float) ((color & 0x0000ff00) >>  8)) *(float)(1.0/255.0);
    const F32 a = ((float) ((color & 0x000000ff)      )) *(float)(1.0/255.0);

    Display_DrawSprite( g_pplayer->m_pin3d.m_pd3dDevice, 
                        y, x,
                        sy, sx,
                        r, g, b, a, //!! pass as DWORD directly? 
                        0.0f,
                        NULL, 1.0f, 1.0f,
                        DISPLAY_TEXTURESTATE_NOFILTER, DISPLAY_RENDERSTATE_TRANSPARENT );
}

#ifdef DEBUG_PLUMB
void invalidate_box( F32 sx, F32 sy, F32 x, F32 y )
{
	RECT	Rect;

	// Invalidate the window region to signal an update from the back buffer (after render is complete).
	Rect.left   = (int)( y - sy * 0.5f );
	Rect.right  = (int)( y + sy * 0.5f );
	Rect.top    = (int)( x - sx * 0.5f );
	Rect.bottom = (int)( x + sx * 0.5f );
	g_pplayer->InvalidateRect(&Rect);
}
#endif

F32 sPlumbPos[2] = { 300, 100 };
static int dirty;

void plumb_draw()
{
#ifdef DEBUG_PLUMB
	RenderStateType		RestoreRenderState;
	TextureStateType	RestoreTextureState;
	D3DMATRIX			RestoreWorldMatrix;
	HRESULT				ReturnCode;

	// Save the current transformation state.
	ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->GetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
	// Save the current render state.
	Display_GetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreRenderState));
	// Save the current texture state.
	Display_GetTextureState (g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreTextureState));

    F32 x = sPlumbPos[0];
    F32 y = sPlumbPos[1];

	draw_transparent_box( 5, 5, x, y, 0xffffffff );
	draw_transparent_box( 3, 3, x, y, 0x000000ff );
	draw_transparent_box( 3, 3, x+GetY()*100.0f, y+ac*(-(GetX()))*100.0f, 0xffffffff );

	draw_transparent_box( 5, 5, x, y, 0xffffffff );
	draw_transparent_box( 3, 3, x, y, 0x000000ff );
	draw_transparent_box( 3, 3, x+gPlumb.y*100.0f, y+ac*(-(gPlumb.x))*100.0f, 0xffffffff );

	// Restore the render states.
	Display_SetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreRenderState));
	// Restore the texture state.
	Display_SetTextureState(g_pplayer->m_pin3d.m_pd3dDevice, &(RestoreTextureState));
	// Restore the transformation state.
	ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 

    dirty = 1;
#endif
}

// Flags that the region behind the mixer volume should be refreshed.
void plumb_erase()
{
#ifdef DEBUG_PLUMB
    invalidate_box( 120, 120, sPlumbPos[0], sPlumbPos[1] );

	dirty = false;
#endif
}
