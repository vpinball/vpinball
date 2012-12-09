#pragma once
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <ddraw.h>
#include "inc\d3d.h"

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
    struct _D3DTLVertexType
	{
    // This structure is compatibile with DX7 flexible vertex formats.
    // These members must be in this order as defined by the SDK.
    // The flags to use this vertex type are... 
	// (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)
    
	float                   X, Y;                           // Vertex2D screen position.                                         
    float                   Z;                              // Vertex2D z buffer depth.                                          
    float                   RHW;                            // Vertex2D rhw.                                                     
    DWORD                   DiffuseColor;                   // Vertex2D diffuse color.                                           
    DWORD                   SpecularColor;					// Vertex2D specular color.                                           
    float                   TU1, TV1;						// Vertex2D texture coordinate.                                      
    float                   TU2, TV2;						// Vertex2D texture coordinate.                                      
	} Vertices[4];
	Vertices[0].DiffuseColor = 
	Vertices[0].SpecularColor = col;
	Vertices[0].TU1 = 0.0f;
	Vertices[0].TV1 = v;
	Vertices[0].TU2 = 0.0f;
	Vertices[0].TV2 = v;
	Vertices[0].X = x; 
	Vertices[0].Y = y + Height; 
	Vertices[0].Z = 0.0f; 
	Vertices[0].RHW = 1.0f; 

	Vertices[1].DiffuseColor = 
	Vertices[1].SpecularColor = col;
	Vertices[1].TU1 = 0.0f;
	Vertices[1].TV1 = 0.0f;
	Vertices[1].TU2 = 0.0f;
	Vertices[1].TV2 = 0.0f;
	Vertices[1].X = x; 
	Vertices[1].Y = y;
	Vertices[1].Z = 0.0f; 
	Vertices[1].RHW = 1.0f; 

	Vertices[2].DiffuseColor = 
	Vertices[2].SpecularColor = col;
	Vertices[2].TU1 = u;
	Vertices[2].TV1 = v;
	Vertices[2].TU2 = u;
	Vertices[2].TV2 = v;
	Vertices[2].X = x + Width; 
	Vertices[2].Y = y + Height;
	Vertices[2].Z = 0.0f; 
	Vertices[2].RHW = 1.0f;

	Vertices[3].DiffuseColor = 
	Vertices[3].SpecularColor = col;
	Vertices[3].TU1 = u;
	Vertices[3].TV1 = 0.0f;
	Vertices[3].TU2 = u;
	Vertices[3].TV2 = 0.0f;
	Vertices[3].X = x + Width; 
	Vertices[3].Y = y; 
	Vertices[3].Z = 0.0f; 
	Vertices[3].RHW = 1.0f; 

	Direct3DDevice->SetTexture ( 0, (LPDIRECTDRAWSURFACE7) Texture );

    // Draw the quad.
    //Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX, Vertices, 4, 0 ); //!!!! retest

	Direct3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX,
											  Vertices, 4,
											  (LPWORD)rgi0123, 4, NULL);
}

#endif
