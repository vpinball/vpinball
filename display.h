#pragma once
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <ddraw.h>
#include "d3d.h"

inline unsigned int RGB_TO_BGR(const unsigned int c)
{
	return (c&0x00FF00u) | ((c>>16)&0xFFu) | ((c&0xFFu)<<16);
}

#define     RGBA_TO_D3DARGB(r,g,b,a)	((((long)((a) * 255.0f)) << 24) | (((long)((r) * 255.0f)) << 16) | (((long)((g) * 255.0f)) << 8) | (long)((b) * 255.0f))
#define     RGBA_TO_D3DRGBA(r,g,b,a)	((((long)((r) * 255.0f)) << 24) | (((long)((g) * 255.0f)) << 16) | (((long)((b) * 255.0f)) << 8) | (long)((a) * 255.0f))

//!! this is only used for the mixer and debugging the plumb, so delete:
inline void Display_DrawSprite( const LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const DWORD col, void * const Texture, const float u, const float v )
{
	// Build a quad.
    Vertex3D_NoTex2 Vertices[4];

	Vertices[0].color =
	Vertices[0].specular = col;
	Vertices[0].tu = 0.0f;
	Vertices[0].tv = v;
	Vertices[0].x = x;
	Vertices[0].y = y + Height;
	Vertices[0].z = 0.0f;
	Vertices[0].rhw = 1.0f;

	Vertices[1].color =
	Vertices[1].specular = col;
	Vertices[1].tu = 0.0f;
	Vertices[1].tv = 0.0f;
	Vertices[1].x = x;
	Vertices[1].y = y;
	Vertices[1].z = 0.0f;
	Vertices[1].rhw = 1.0f;

	Vertices[2].color =
	Vertices[2].specular = col;
	Vertices[2].tu = u;
	Vertices[2].tv = v;
	Vertices[2].x = x + Width;
	Vertices[2].y = y + Height;
	Vertices[2].z = 0.0f;
	Vertices[2].rhw = 1.0f;

	Vertices[3].color =
	Vertices[3].specular = col;
	Vertices[3].tu = u;
	Vertices[3].tv = 0.0f;
	Vertices[3].x = x + Width;
	Vertices[3].y = y;
	Vertices[3].z = 0.0f;
	Vertices[3].rhw = 1.0f;

	Direct3DDevice->SetTexture ( 0, (LPDIRECTDRAWSURFACE7) Texture );

    // Draw the quad.
    //Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_NOTEX2_VERTEX, Vertices, 4, 0 ); //!!!! retest

	Direct3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_NOTEX2_VERTEX,
											  Vertices, 4,
											  (LPWORD)rgi0123, 4, NULL);
}

#endif
