#pragma once
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdio.h>
#include <ddraw.h>
#include <d3d.h>

#define     RGBA_TO_D3DARGB(r,g,b,a)	((((long)((a) * 255.0f)) << 24) | (((long)((r) * 255.0f)) << 16) | (((long)((g) * 255.0f)) << 8) | (long)((b) * 255.0f))
#define     RGBA_TO_D3DRGBA(r,g,b,a)	((((long)((r) * 255.0f)) << 24) | (((long)((g) * 255.0f)) << 16) | (((long)((b) * 255.0f)) << 8) | (long)((a) * 255.0f))

#define		DISPLAY_MAXTEXTURES			1

typedef struct _D3DTLVertexType			D3DTLVertexType;
struct _D3DTLVertexType
{
    // This structure is compatibile with DX7 flexible vertex formats.
    // These members must be in this order as defined by the SDK.
    // The flags to use this vertex type are... 
    // (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1).

    float                   X, Y;                           // Vertex2D screen position.                                         
    float                   Z;                              // Vertex2D z buffer depth.                                          
    float                   RHW;                            // Vertex2D rhw.                                                     
    DWORD                   DiffuseColor;                   // Vertex2D diffuse color.                                           
    DWORD                   SpecularColor;					// Vertex2D specular color.                                           
    float                   TU1, TV1;						// Vertex2D texture coordinate.                                      
    float                   TU2, TV2;						// Vertex2D texture coordinate.                                      
};


typedef enum _DISPLAY_RENDERSTATE		DISPLAY_RENDERSTATE;
enum _DISPLAY_RENDERSTATE 
{
	// Generic render state.
    DISPLAY_RENDERSTATE_GENERIC		= 0,

	// Custom render state indexes for DrawSprite.
  	DISPLAY_RENDERSTATE_OPAQUE,
  	DISPLAY_RENDERSTATE_TRANSPARENT,

	DISPLAY_RENDERSTATE_BALL,

	DISPLAY_RENDERSTATE_MAX
};


typedef enum _DISPLAY_TEXTURESTATE	DISPLAY_TEXTURESTATE;
enum _DISPLAY_TEXTURESTATE 
{
	// Generic texture state.
    DISPLAY_TEXTURESTATE_GENERIC	= 0,

	// Custom texture state indexes for DrawSprite.
  	DISPLAY_TEXTURESTATE_NOFILTER,

  	DISPLAY_TEXTURESTATE_BALL,

	DISPLAY_TEXTURESTATE_MAX
};


typedef struct _RenderStateType     RenderStateType;
struct _RenderStateType
{
    DWORD           ZEnable;
    DWORD           ZWriteEnable;
    DWORD           AlphaTestEnable;
    DWORD           AlphaRef;
    DWORD           AlphaFunc;
    DWORD           DitherEnable;
    DWORD           AlphaBlendEnable;
    DWORD           SpecularEnable;
    DWORD           SrcBlend;
    DWORD           DestBlend;
    DWORD           CullMode;
    DWORD           Lighting;
    DWORD           PerspectiveCorrection;
	DWORD			ColorKeyEnable;
	DWORD			ColorKeyBlendEnable;

    DWORD           Antialias;
    DWORD           FillMode;
    DWORD           ShadeMode;
    DWORD           LinePattern;
    DWORD           LastPixel;
    DWORD           ZFunction;
    DWORD           FogEnable;
    DWORD           ZVisible;
    DWORD           FogColor;
    DWORD           FogTableMode;
    DWORD           FogStart;
    DWORD           FogEnd;
    DWORD           FogDensity;
    DWORD           EdgeAntiAlias;
    DWORD           ZBias;
    DWORD           RangeFogEnable;
    DWORD           StencilEnable;
    DWORD           StencilFail;
    DWORD           StencilZFail;
    DWORD           StencilPass;
    DWORD           StencilFunc;
    DWORD           StencilRef;
    DWORD           StencilMask;
    DWORD           StencilWriteMask;
    DWORD           TextureFactor;
    DWORD           Wrap0;
    DWORD           Wrap1;
    DWORD           Wrap2;
    DWORD           Wrap3;
    DWORD           Wrap4;
    DWORD           Wrap5;
    DWORD           Wrap6;
    DWORD           Wrap7;
    DWORD           Clipping;
    DWORD           AmbientLightColor;
    DWORD           FogVertexMode;
    DWORD           ColorVertex;
    DWORD           LocalViewer;
    DWORD           NormalizeNormals;
    DWORD           DiffuseMaterialSource;
    DWORD           SpecularMaterialSource;
    DWORD           AmbientMaterialSource;
    DWORD           EmissiveMaterialSource;
    DWORD           VertexBlend;
    DWORD           ClipPlaneEnable;
};


typedef struct _TextureStateType     TextureStateType;
struct _TextureStateType
{
    void            *Texture[DISPLAY_MAXTEXTURES];

    DWORD           AddressU[DISPLAY_MAXTEXTURES];
    DWORD           AddressV[DISPLAY_MAXTEXTURES];

    DWORD           ColorArg1[DISPLAY_MAXTEXTURES];
    DWORD           ColorArg2[DISPLAY_MAXTEXTURES];
    DWORD           ColorOp[DISPLAY_MAXTEXTURES];

    DWORD           AlphaArg1[DISPLAY_MAXTEXTURES];
    DWORD           AlphaArg2[DISPLAY_MAXTEXTURES];
    DWORD           AlphaOp[DISPLAY_MAXTEXTURES];

    DWORD           MagFilter[DISPLAY_MAXTEXTURES];
    DWORD           MinFilter[DISPLAY_MAXTEXTURES];
    DWORD           MipFilter[DISPLAY_MAXTEXTURES];

    DWORD           TextureCoordinateIndex[DISPLAY_MAXTEXTURES];
    DWORD           TextureCoordinateTransformFlags[DISPLAY_MAXTEXTURES];
};

int Display_GetPowerOfTwo ( const int Value );
extern int NumVideoBytes;

//extern RenderStateType RenderStates[DISPLAY_RENDERSTATE_MAX];
//extern TextureStateType TextureStates[DISPLAY_TEXTURESTATE_MAX];

// Function headers.

//void Display_InitializeRenderStates ();
//void Display_GetRenderState ( LPDIRECT3DDEVICE7 Direct3DDevice, RenderStateType *RenderState );
//void Display_SetRenderState ( LPDIRECT3DDEVICE7 Direct3DDevice, RenderStateType *RenderState );
//void Display_ClearRenderState ( RenderStateType *RenderState );

//void Display_InitializeTextureStates ();
//void Display_GetTextureState ( LPDIRECT3DDEVICE7 Direct3DDevice, TextureStateType *TextureState );
//void Display_SetTextureState ( LPDIRECT3DDEVICE7 Direct3DDevice, TextureStateType *TextureState );
//void Display_ClearTextureState ( TextureStateType *TextureState );

void Display_CreateTexture ( const LPDIRECT3DDEVICE7 Direct3DDevice, const LPDIRECTDRAW7 DirectDrawObject, const LPDIRECTDRAWSURFACE7 DDrawSurface, const int Width, const int Height, LPDIRECTDRAWSURFACE7 * const DestD3DTexture, float * const u, float * const v );
void Display_CopyTexture ( LPDIRECT3DDEVICE7 Direct3DDevice, LPDIRECTDRAWSURFACE7 DestTexture, const RECT * const Rect, LPDIRECTDRAWSURFACE7 SourceTexture );
void Display_ClearTexture ( const LPDIRECT3DDEVICE7 Direct3DDevice, const LPDIRECTDRAWSURFACE7 Texture, const char Value );
void Display_DestroyTexture ( LPDIRECTDRAWSURFACE7 Texture );

HRESULT CALLBACK Display_EnumurateTransparentTextureFormats ( DDPIXELFORMAT *pddpf, VOID *param );

HRESULT Display_DrawIndexedPrimitive( LPDIRECT3DDEVICE7 Direct3DDevice, const D3DPRIMITIVETYPE d3dptPrimitiveType, const DWORD dwVertexTypeDesc, const LPVOID lpvVertices, const DWORD dwVertexCount, const LPWORD lpwIndices, const DWORD dwIndexCount, const DWORD dwFlags );

// Draws a sprite.
// x, y is the screen coordinate of the top-left corner of the sprite.
// Width, Height is determines the size of the sprite (in pixels).
// r, g, b, a controls the color of the sprite.  The exact behavior is dependent on the RenderState.
// Angle is in degrees.  Controls the orientation of the sprite centered about the top-left with clockwise as positive.
// Texture is the texure to use.  This can be NULL for solid color surfaces.
// TextureStateIndex controls how the sprite texture is combined with vertex colors and filtered.
// RenderStateIndex controls how the sprite is drawn.
inline void Display_DrawSprite_NoMatrix ( LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const float r, const float g, const float b, const float a, const float Angle, void * const Texture, const float u, const float v, const int TextureStateIndex, const int RenderStateIndex )
{
	// Calculate sin and cos theta.
	//const float Radians = Angle * (float)(2.0 * M_PI / 360.0);
	//const float SinTheta = sinf ( Radians );
	//const float CosTheta = cosf ( Radians );

	// ToDo: Calculate vertices with rotation applied.	
	//       We can probably get away without implementing a matrix library. -JEP

	const DWORD col = RGBA_TO_D3DARGB ( r, g, b, a );

	// Build a quad.
    D3DTLVertexType	Vertices[4];
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

	// Set the texture state.
	//Display_SetTextureState ( Direct3DDevice, &(TextureStates[TextureStateIndex]) );
	
	// Set the render state.
	//Display_SetRenderState ( Direct3DDevice, &(RenderStates[RenderStateIndex]) );

	// Set the texture.
	/*const HRESULT ReturnCode =*/ Direct3DDevice->SetTexture ( 0, (LPDIRECTDRAWSURFACE7) Texture );

	// WTF?  As soon as I do DrawPrimitive, the ball disappears.  Everything else works (ie saving and restoring states)... it's just the draw!

//	  // Draw the quad.
//    ReturnCode = Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1), (D3DTLVertexType *) Vertices, 4, 0 );  
    //ReturnCode = Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX, (D3DTLVertexType *) Vertices, 4, 0 );  

	Direct3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX,
										  (D3DTLVertexType *) Vertices, 4,
										  (LPWORD)rgi0123, 4, NULL);
}

inline void Display_DrawSprite ( LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const float r, const float g, const float b, const float a, const float Angle, void * const Texture, const float u, const float v, const int TextureStateIndex, const int RenderStateIndex )
{
    const D3DMATRIX WorldMatrix(1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f);

	// Apply the transformation.
    /*const HRESULT ReturnCode =*/ Direct3DDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, (LPD3DMATRIX)&WorldMatrix ); 

    Display_DrawSprite_NoMatrix ( Direct3DDevice, x, y, Width, Height, r, g, b, a, Angle, Texture, u, v, TextureStateIndex, RenderStateIndex );
}

inline void Display_DrawSprite_NoMatrix_NoStates ( LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const DWORD col, const float Angle, void * const Texture, const float u, const float v )
{
	// Calculate sin and cos theta.
	//const float Radians = Angle * (float)(2.0 * M_PI / 360.0);
	//const float SinTheta = sinf ( Radians );
	//const float CosTheta = cosf ( Radians );

	// ToDo: Calculate vertices with rotation applied.	
	//       We can probably get away without implementing a matrix library. -JEP

	// Build a quad.
    D3DTLVertexType	Vertices[4];
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

	// Set the texture.
	/*const HRESULT ReturnCode =*/ Direct3DDevice->SetTexture ( 0, (LPDIRECTDRAWSURFACE7) Texture );

	// WTF?  As soon as I do DrawPrimitive, the ball disappears.  Everything else works (ie saving and restoring states)... it's just the draw!

//	  // Draw the quad.
//    ReturnCode = Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1), (D3DTLVertexType *) Vertices, 4, 0 );  
    //ReturnCode = Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX, (D3DTLVertexType *) Vertices, 4, 0 );  

	Direct3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX,
										  (D3DTLVertexType *) Vertices, 4,
										  (LPWORD)rgi0123, 4, NULL);
}

inline void Display_DrawSprite ( LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const DWORD col, const float Angle, void * const Texture, const float u, const float v, const int TextureStateIndex, const int RenderStateIndex )
{
    const D3DMATRIX WorldMatrix(1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f);

	// Apply the transformation.
    /*const HRESULT ReturnCode =*/ Direct3DDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, (LPD3DMATRIX)&WorldMatrix ); 

	// Set the texture state.
	//Display_SetTextureState ( Direct3DDevice, &(TextureStates[TextureStateIndex]) );
	
	// Set the render state.
	//Display_SetRenderState ( Direct3DDevice, &(RenderStates[RenderStateIndex]) );

    Display_DrawSprite_NoMatrix_NoStates ( Direct3DDevice, x, y, Width, Height, col, Angle, Texture, u, v );
}

#endif
