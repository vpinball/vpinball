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


// Function headers.
void Display_DrawSprite ( LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const float r, const float g, const float b, const float a, const float Angle, void * const Texture, const float u, const float v, const int TextureStateIndex, const int RenderStateIndex );

void Display_InitializeRenderStates ();
void Display_GetRenderState ( LPDIRECT3DDEVICE7 Direct3DDevice, RenderStateType *RenderState );
void Display_SetRenderState ( LPDIRECT3DDEVICE7 Direct3DDevice, RenderStateType *RenderState );
void Display_ClearRenderState ( RenderStateType *RenderState );

void Display_InitializeTextureStates ();
void Display_GetTextureState ( LPDIRECT3DDEVICE7 Direct3DDevice, TextureStateType *TextureState );
void Display_SetTextureState ( LPDIRECT3DDEVICE7 Direct3DDevice, TextureStateType *TextureState );
void Display_ClearTextureState ( TextureStateType *TextureState );

void Display_CreateTexture ( const LPDIRECT3DDEVICE7 Direct3DDevice, const LPDIRECTDRAW7 DirectDrawObject, const LPDIRECTDRAWSURFACE7 DDrawSurface, const int Width, const int Height, LPDIRECTDRAWSURFACE7 * const DestD3DTexture, float * const u, float * const v );
void Display_CopyTexture ( LPDIRECT3DDEVICE7 Direct3DDevice, LPDIRECTDRAWSURFACE7 DestTexture, RECT *Rect, LPDIRECTDRAWSURFACE7 SourceTexture );
void Display_ClearTexture ( const LPDIRECT3DDEVICE7 Direct3DDevice, const LPDIRECTDRAWSURFACE7 Texture, const char Value );
void Display_DestroyTexture ( LPDIRECTDRAWSURFACE7 Texture );

HRESULT CALLBACK Display_EnumurateTransparentTextureFormats ( DDPIXELFORMAT *pddpf, VOID *param );

int Display_GetPowerOfTwo ( const int Value );
extern int NumVideoBytes;

extern RenderStateType RenderStates[DISPLAY_RENDERSTATE_MAX];
extern TextureStateType TextureStates[DISPLAY_TEXTURESTATE_MAX];

HRESULT Display_DrawIndexedPrimitive( LPDIRECT3DDEVICE7 Direct3DDevice, D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags );

#endif
