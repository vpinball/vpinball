#include "StdAfx.h"

#include "display.h"

#include <math.h>


// Global render and texture states for calls to DrawSprite.
RenderStateType		RenderStates[DISPLAY_RENDERSTATE_MAX];
TextureStateType	TextureStates[DISPLAY_TEXTURESTATE_MAX];

int NumVideoBytes = 0;


// Initializes the array of render states.
// The render states are hard-coded.  
// If time permits, we should parse these states from a text file.
void Display_InitializeRenderStates ()
{

	// Define DISPLAY_RENDERSTATE_GENERIC.
	// This render state is good for debugging.
	Display_ClearRenderState ( &(RenderStates[DISPLAY_RENDERSTATE_GENERIC]) );

	// Define DISPLAY_RENDERSTATE_OPAQUE.
	// This render state is good for textured sprites that are opaque.
	Display_ClearRenderState ( &(RenderStates[DISPLAY_RENDERSTATE_OPAQUE]) );
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].ZEnable = D3DZB_FALSE;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].ZFunction = D3DCMP_LESSEQUAL;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].ZWriteEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].AlphaTestEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].AlphaRef = 0;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].AlphaFunc = D3DCMP_ALWAYS;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].AlphaBlendEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].SrcBlend = D3DBLEND_ONE;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].DestBlend = D3DBLEND_ZERO;
	RenderStates[DISPLAY_RENDERSTATE_OPAQUE].ColorVertex = TRUE;

	// Define DISPLAY_RENDERSTATE_TRANSPARENT.
	// This render state is good for textured sprites with any level of transparency.
	Display_ClearRenderState ( &(RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT]) );
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].ZEnable = D3DZB_FALSE;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].ZFunction = D3DCMP_LESSEQUAL;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].ZWriteEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].AlphaTestEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].AlphaRef = 0;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].AlphaFunc = D3DCMP_ALWAYS;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].AlphaBlendEnable = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].SrcBlend = D3DBLEND_SRCALPHA;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].DestBlend = D3DBLEND_INVSRCALPHA;
	RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT].ColorVertex = TRUE;

	// Define DISPLAY_RENDERSTATE_BALL.
	// This render state is used for drawing the ball.
	Display_ClearRenderState ( &(RenderStates[DISPLAY_RENDERSTATE_BALL]) );
	RenderStates[DISPLAY_RENDERSTATE_BALL].ZEnable = D3DZB_TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ZVisible = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ZBias = 0;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ZFunction = D3DCMP_LESSEQUAL;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ZWriteEnable = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].AlphaTestEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].AlphaRef = 128;
	RenderStates[DISPLAY_RENDERSTATE_BALL].AlphaFunc = D3DCMP_GREATER;
	RenderStates[DISPLAY_RENDERSTATE_BALL].DitherEnable = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].SpecularEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].AlphaBlendEnable = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].SrcBlend = D3DBLEND_SRCALPHA;				
	RenderStates[DISPLAY_RENDERSTATE_BALL].DestBlend = D3DBLEND_INVSRCALPHA;					
	RenderStates[DISPLAY_RENDERSTATE_BALL].FillMode = D3DFILL_SOLID;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ShadeMode = D3DSHADE_GOURAUD;
	RenderStates[DISPLAY_RENDERSTATE_BALL].CullMode = D3DCULL_CCW;
	RenderStates[DISPLAY_RENDERSTATE_BALL].PerspectiveCorrection = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].Lighting = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].Clipping = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ClipPlaneEnable = 0;
	RenderStates[DISPLAY_RENDERSTATE_BALL].ColorVertex = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].NormalizeNormals = FALSE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].LocalViewer = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].LinePattern = 0;
	RenderStates[DISPLAY_RENDERSTATE_BALL].LastPixel = TRUE;
	RenderStates[DISPLAY_RENDERSTATE_BALL].DiffuseMaterialSource = D3DMCS_COLOR1;
	RenderStates[DISPLAY_RENDERSTATE_BALL].SpecularMaterialSource = D3DMCS_COLOR2;
	RenderStates[DISPLAY_RENDERSTATE_BALL].AmbientMaterialSource = D3DMCS_MATERIAL;
	RenderStates[DISPLAY_RENDERSTATE_BALL].EmissiveMaterialSource = D3DMCS_MATERIAL;

}


// Initializes the array of texture states.
// The texture states are hard-coded.  
// If time permits, we should parse these states from a text file.
void Display_InitializeTextureStates ()
{
	// Define DISPLAY_TEXTURESTATE_GENERIC.
	// This texture state is good for debugging.
	Display_ClearTextureState ( &(TextureStates[DISPLAY_TEXTURESTATE_GENERIC]) );

	// Define DISPLAY_TEXTURESTATE_NOFILTER.
	// This texture state is good for sprites that have fine detail such as written text.
	Display_ClearTextureState ( &(TextureStates[DISPLAY_TEXTURESTATE_NOFILTER]) );
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].ColorArg1[0] = D3DTA_TEXTURE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].ColorArg2[0] = D3DTA_DIFFUSE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].ColorOp[0] = D3DTOP_MODULATE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].AlphaArg1[0] = D3DTA_TEXTURE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].AlphaArg2[0] = D3DTA_DIFFUSE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].AlphaOp[0] = D3DTOP_MODULATE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].MagFilter[0] = D3DTFG_POINT;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].MinFilter[0] = D3DTFN_POINT;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].MipFilter[0] = D3DTFP_NONE;
	TextureStates[DISPLAY_TEXTURESTATE_NOFILTER].ColorOp[1] = D3DTOP_DISABLE;

	// Define DISPLAY_TEXTURESTATE_BALL.
	// This texture state is used to draw the ball.
	Display_ClearTextureState ( &(TextureStates[DISPLAY_TEXTURESTATE_BALL]) );
	TextureStates[DISPLAY_TEXTURESTATE_BALL].AddressU[0] = D3DTADDRESS_WRAP;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].AddressV[0] = D3DTADDRESS_WRAP;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].ColorArg1[0] = D3DTA_TEXTURE;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].ColorArg2[0] = D3DTA_DIFFUSE;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].ColorOp[0] = D3DTOP_MODULATE;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].AlphaArg1[0] = D3DTA_TEXTURE;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].AlphaArg2[0] = D3DTA_CURRENT;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].AlphaOp[0] = D3DTOP_SELECTARG1;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].MagFilter[0] = D3DTFG_LINEAR;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].MinFilter[0] = D3DTFN_LINEAR;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].MipFilter[0] = D3DTFP_LINEAR;
	TextureStates[DISPLAY_TEXTURESTATE_BALL].ColorOp[1] = D3DTOP_DISABLE;
}


// Draws a sprite.
// x, y is the screen coordinate of the top-left corner of the sprite.
// Width, Height is determines the size of the sprite (in pixels).
// r, g, b, a controls the color of the sprite.  The exact behavior is dependent on the RenderState.
// Angle is in degrees.  Controls the orientation of the sprite centered about the top-left with clockwise as positive.
// Texture is the texure to use.  This can be NULL for solid color surfaces.
// TextureStateIndex controls how the sprite texture is combined with vertex colors and filtered.
// RenderStateIndex controls how the sprite is drawn.
void Display_DrawSprite ( LPDIRECT3DDEVICE7 Direct3DDevice, const float x, const float y, const float Width, const float Height, const float r, const float g, const float b, const float a, const float Angle, void * const Texture, const float u, const float v, const int TextureStateIndex, const int RenderStateIndex )
{
    D3DMATRIX       WorldMatrix;
    HRESULT	        ReturnCode;
    D3DTLVertexType	Vertices[4];

	// Calculate sin and cos theta.
	//const float Radians = Angle * (float)(2.0 * M_PI / 360.0);
	//const float SinTheta = sinf ( Radians );
	//const float CosTheta = cosf ( Radians );

	// ToDo: Calculate vertices with rotation applied.	
	//       We can probably get away without implementing a matrix library. -JEP

	// Build a quad.
	Vertices[0].DiffuseColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[0].SpecularColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[0].TU1 = 0.0f;
	Vertices[0].TV1 = v;
	Vertices[0].TU2 = 0.0f;
	Vertices[0].TV2 = v;
	Vertices[0].X = x; 
	Vertices[0].Y = y + Height; 
	Vertices[0].Z = 0.0f; 
	Vertices[0].RHW = 1.0f; 

	Vertices[1].DiffuseColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[1].SpecularColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[1].TU1 = 0.0f;
	Vertices[1].TV1 = 0.0f;
	Vertices[1].TU2 = 0.0f;
	Vertices[1].TV2 = 0.0f;
	Vertices[1].X = x; 
	Vertices[1].Y = y;
	Vertices[1].Z = 0.0f; 
	Vertices[1].RHW = 1.0f; 

	Vertices[2].DiffuseColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[2].SpecularColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[2].TU1 = u;
	Vertices[2].TV1 = v;
	Vertices[2].TU2 = u;
	Vertices[2].TV2 = v;
	Vertices[2].X = x + Width; 
	Vertices[2].Y = y + Height;
	Vertices[2].Z = 0.0f; 
	Vertices[2].RHW = 1.0f;

	Vertices[3].DiffuseColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[3].SpecularColor = RGBA_TO_D3DARGB ( r, g, b, a );
	Vertices[3].TU1 = u;
	Vertices[3].TV1 = 0.0f;
	Vertices[3].TU2 = u;
	Vertices[3].TV2 = 0.0f;
	Vertices[3].X = x + Width; 
	Vertices[3].Y = y; 
	Vertices[3].Z = 0.0f; 
	Vertices[3].RHW = 1.0f; 

	// Clear the transform.
	WorldMatrix._11 = 1.0f;
	WorldMatrix._12 = 0.0f;
	WorldMatrix._13 = 0.0f;
	WorldMatrix._14 = 0.0f;
	WorldMatrix._21 = 0.0f;
	WorldMatrix._22 = 1.0f;
	WorldMatrix._23 = 0.0f;
	WorldMatrix._24 = 0.0f;
	WorldMatrix._31 = 0.0f;
	WorldMatrix._32 = 0.0f;
	WorldMatrix._33 = 1.0f;
	WorldMatrix._34 = 0.0f;
	WorldMatrix._41 = 0.0f;
	WorldMatrix._42 = 0.0f;
	WorldMatrix._43 = 0.0f;
	WorldMatrix._44 = 1.0f;

	// Apply the transformation.
    ReturnCode = Direct3DDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, &WorldMatrix ); 

	// Set the texture state.
	Display_SetTextureState ( Direct3DDevice, &(TextureStates[TextureStateIndex]) );
	
	// Set the render state.
	Display_SetRenderState ( Direct3DDevice, &(RenderStates[RenderStateIndex]) );

	// Set the texture.
	ReturnCode = Direct3DDevice->SetTexture ( 0, (LPDIRECTDRAWSURFACE7) Texture );

	// WTF?  As soon as I do DrawPrimitive, the ball disappears.  Everything else works (ie saving and restoring states)... it's just the draw!

//	  // Draw the quad.
//    ReturnCode = Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1), (D3DTLVertexType *) Vertices, 4, 0 );  
    ReturnCode = Direct3DDevice->DrawPrimitive ( D3DPT_TRIANGLESTRIP, MY_D3DTRANSFORMED_VERTEX, (D3DTLVertexType *) Vertices, 4, 0 );  

//#define MY_D3DFVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2)
//#define MY_D3DTRANSFORMED_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)
}


// Gets the current render state from the D3D device.
// This is costly on performance and should only be used for debugging.
void Display_GetRenderState ( LPDIRECT3DDEVICE7 Direct3DDevice, RenderStateType *RenderState )
{
    HRESULT     ReturnCode;

    // Get the render state.
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ZENABLE, &(RenderState->ZEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FILLMODE, &(RenderState->FillMode) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_SHADEMODE, &(RenderState->ShadeMode) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_LINEPATTERN, &(RenderState->LinePattern) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ZWRITEENABLE, &(RenderState->ZWriteEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ALPHATESTENABLE, &(RenderState->AlphaTestEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ANTIALIAS, &(RenderState->Antialias) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_TEXTUREPERSPECTIVE, &(RenderState->PerspectiveCorrection) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_LASTPIXEL, &(RenderState->LastPixel) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_COLORKEYENABLE, &(RenderState->ColorKeyEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_COLORKEYBLENDENABLE, &(RenderState->ColorKeyBlendEnable) );
	ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_SRCBLEND, &(RenderState->SrcBlend) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_DESTBLEND, &(RenderState->DestBlend) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_CULLMODE, &(RenderState->CullMode) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ZFUNC, &(RenderState->ZFunction) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ALPHAREF, &(RenderState->AlphaRef) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ALPHAFUNC, &(RenderState->AlphaFunc) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_DITHERENABLE, &(RenderState->DitherEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, &(RenderState->AlphaBlendEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGENABLE, &(RenderState->FogEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_SPECULARENABLE, &(RenderState->SpecularEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ZVISIBLE, &(RenderState->ZVisible) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGCOLOR, &(RenderState->FogColor) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGTABLEMODE, &(RenderState->FogTableMode) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGSTART, &(RenderState->FogStart) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGEND, &(RenderState->FogEnd) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGDENSITY, &(RenderState->FogDensity) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_EDGEANTIALIAS, &(RenderState->EdgeAntiAlias) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_ZBIAS, &(RenderState->ZBias) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_RANGEFOGENABLE, &(RenderState->RangeFogEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILENABLE, &(RenderState->StencilEnable) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILFAIL, &(RenderState->StencilFail) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILZFAIL, &(RenderState->StencilZFail) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILPASS, &(RenderState->StencilPass) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILFUNC, &(RenderState->StencilFunc) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILREF, &(RenderState->StencilRef) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILMASK, &(RenderState->StencilMask) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_STENCILWRITEMASK, &(RenderState->StencilWriteMask) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_TEXTUREFACTOR, &(RenderState->TextureFactor) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP0, &(RenderState->Wrap0) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP1, &(RenderState->Wrap1) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP2, &(RenderState->Wrap2) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP3, &(RenderState->Wrap3) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP4, &(RenderState->Wrap4) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP5, &(RenderState->Wrap5) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP6, &(RenderState->Wrap6) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_WRAP7, &(RenderState->Wrap7) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_CLIPPING, &(RenderState->Clipping) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_LIGHTING, &(RenderState->Lighting) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_AMBIENT, &(RenderState->AmbientLightColor) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_FOGVERTEXMODE, &(RenderState->FogVertexMode) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_COLORVERTEX, &(RenderState->ColorVertex) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_LOCALVIEWER, &(RenderState->LocalViewer) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_NORMALIZENORMALS, &(RenderState->NormalizeNormals) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, &(RenderState->DiffuseMaterialSource) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_SPECULARMATERIALSOURCE, &(RenderState->SpecularMaterialSource) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_AMBIENTMATERIALSOURCE, &(RenderState->AmbientMaterialSource) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_EMISSIVEMATERIALSOURCE, &(RenderState->EmissiveMaterialSource) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_VERTEXBLEND, &(RenderState->VertexBlend) );
    ReturnCode = Direct3DDevice->GetRenderState ( D3DRENDERSTATE_CLIPPLANEENABLE, &(RenderState->ClipPlaneEnable) );
}


// Sets the current render state of the D3D device.
void Display_SetRenderState ( LPDIRECT3DDEVICE7 Direct3DDevice, RenderStateType *RenderState )
{
    return;

	HRESULT     ReturnCode;

    // Set the render state.
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ZENABLE, RenderState->ZEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FILLMODE, RenderState->FillMode );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_SHADEMODE, RenderState->ShadeMode );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_LINEPATTERN, RenderState->LinePattern );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ZWRITEENABLE, RenderState->ZWriteEnable );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ALPHATESTENABLE, RenderState->AlphaTestEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_LASTPIXEL, RenderState->LastPixel );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_SRCBLEND, RenderState->SrcBlend );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_DESTBLEND, RenderState->DestBlend );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ANTIALIAS, RenderState->Antialias );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_COLORKEYENABLE, RenderState->ColorKeyEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_COLORKEYBLENDENABLE, RenderState->ColorKeyBlendEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_TEXTUREPERSPECTIVE, RenderState->PerspectiveCorrection );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_CULLMODE, RenderState->CullMode );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ZFUNC, RenderState->ZFunction );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ALPHAREF, RenderState->AlphaRef );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ALPHAFUNC, RenderState->AlphaFunc );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_DITHERENABLE, RenderState->DitherEnable );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, RenderState->AlphaBlendEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGENABLE, RenderState->FogEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, RenderState->SpecularEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ZVISIBLE, RenderState->ZVisible );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGCOLOR, RenderState->FogColor );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGTABLEMODE, RenderState->FogTableMode );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGSTART, RenderState->FogStart );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGEND, RenderState->FogEnd );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGDENSITY, RenderState->FogDensity );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_EDGEANTIALIAS, RenderState->EdgeAntiAlias );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_ZBIAS, RenderState->ZBias );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_RANGEFOGENABLE, RenderState->RangeFogEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILENABLE, RenderState->StencilEnable );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILFAIL, RenderState->StencilFail );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILZFAIL, RenderState->StencilZFail );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILPASS, RenderState->StencilPass );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILFUNC, RenderState->StencilFunc );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILREF, RenderState->StencilRef );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILMASK, RenderState->StencilMask );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_STENCILWRITEMASK, RenderState->StencilWriteMask );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_TEXTUREFACTOR, RenderState->TextureFactor );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP0, RenderState->Wrap0 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP1, RenderState->Wrap1 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP2, RenderState->Wrap2 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP3, RenderState->Wrap3 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP4, RenderState->Wrap4 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP5, RenderState->Wrap5 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP6, RenderState->Wrap6 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_WRAP7, RenderState->Wrap7 );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_CLIPPING, RenderState->Clipping );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_LIGHTING, RenderState->Lighting );                
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_AMBIENT, RenderState->AmbientLightColor );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_FOGVERTEXMODE, RenderState->FogVertexMode );
    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_COLORVERTEX, RenderState->ColorVertex );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_LOCALVIEWER, RenderState->LocalViewer );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_NORMALIZENORMALS, RenderState->NormalizeNormals );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, RenderState->DiffuseMaterialSource );     
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_SPECULARMATERIALSOURCE, RenderState->SpecularMaterialSource );  
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_AMBIENTMATERIALSOURCE, RenderState->AmbientMaterialSource );    
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_EMISSIVEMATERIALSOURCE, RenderState->EmissiveMaterialSource ); 
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_VERTEXBLEND, RenderState->VertexBlend );
//    ReturnCode = Direct3DDevice->SetRenderState ( D3DRENDERSTATE_CLIPPLANEENABLE, RenderState->ClipPlaneEnable );

}


// Sets the render state to a generic state.
// Here mainly for debugging purposes.
void Display_ClearRenderState ( RenderStateType *RenderState )
{
    RenderState->ZEnable = D3DZB_TRUE;
    RenderState->FillMode = D3DFILL_SOLID;
    RenderState->ShadeMode = D3DSHADE_GOURAUD;
//    RenderState->LinePattern = 0;
    RenderState->ZWriteEnable = FALSE;
    RenderState->AlphaTestEnable = FALSE;
    RenderState->LastPixel = TRUE;
    RenderState->SrcBlend = D3DBLEND_ONE;
    RenderState->DestBlend = D3DBLEND_ZERO;
    RenderState->CullMode = D3DCULL_NONE;
    RenderState->ZFunction = D3DCMP_LESSEQUAL;
    RenderState->AlphaRef = 0;
    RenderState->AlphaFunc = D3DCMP_ALWAYS;
    RenderState->DitherEnable = TRUE;
    RenderState->AlphaBlendEnable = FALSE;
    RenderState->ColorKeyEnable = FALSE;
    RenderState->ColorKeyBlendEnable = FALSE;
//    RenderState->FogEnable = FALSE;
    RenderState->SpecularEnable = FALSE;
//    RenderState->ZVisible = ;
//    RenderState->FogColor = RGBA_TO_D3DARGB ( 0.0f, 0.0f, 0.0f, 0.0f );
//    RenderState->FogTableMode = D3DFOG_NONE;
//    RenderState->FogStart = (unsigned long) 0.0f;
//    RenderState->FogEnd = (unsigned long) 0.0f;
//    RenderState->FogDensity = 0.0f;
    RenderState->EdgeAntiAlias = FALSE;
//    RenderState->ZBias = 0;
//    RenderState->RangeFogEnable = FALSE;
//    RenderState->StencilEnable = FALSE;
//    RenderState->StencilFail = D3DSTENCILOP_KEEP;
//    RenderState->StencilZFail = D3DSTENCILOP_KEEP;
//    RenderState->StencilPass = D3DSTENCILOP_KEEP;
//    RenderState->StencilFunc = D3DCMP_ALWAYS;
//    RenderState->StencilRef = 0;
//    RenderState->StencilMask = 0xffffffff;
//    RenderState->StencilWriteMask = 0xffffffff;
//    RenderState->TextureFactor = D3DCOLOR_ARGB ( 255, 255, 255, 255 );
//    RenderState->Wrap0 = 0;
//    RenderState->Wrap1 = 0;
//    RenderState->Wrap2 = 0;
//    RenderState->Wrap3 = 0;
//    RenderState->Wrap4 = 0;
//    RenderState->Wrap5 = 0;
//    RenderState->Wrap6 = 0;
//    RenderState->Wrap7 = 0;
    RenderState->Clipping = FALSE;
    RenderState->Lighting = FALSE;
    RenderState->AmbientLightColor = RGBA_TO_D3DRGBA ( 0.0f, 0.0f, 0.0f, 0.0f );
//    RenderState->FogVertexMode = D3DFOG_NONE;
    RenderState->ColorVertex = TRUE;
    RenderState->LocalViewer = TRUE;
    RenderState->NormalizeNormals = FALSE;
    RenderState->DiffuseMaterialSource = D3DMCS_COLOR1;
    RenderState->SpecularMaterialSource = D3DMCS_MATERIAL; 
    RenderState->AmbientMaterialSource = D3DMCS_MATERIAL; 
    RenderState->EmissiveMaterialSource = D3DMCS_MATERIAL;
//    RenderState->VertexBlend = D3DVBF_DISABLE;
    RenderState->ClipPlaneEnable = FALSE;
    RenderState->PerspectiveCorrection = FALSE;
}


// Gets the current texture state.
// Here for debugging purposes.
void Display_GetTextureState ( LPDIRECT3DDEVICE7 Direct3DDevice, TextureStateType *TextureState )
{
	for (int i=0; i<DISPLAY_MAXTEXTURES; ++i)
	{
	    HRESULT         ReturnCode;
		// Get the texture.
		ReturnCode = Direct3DDevice->GetTexture ( i, (LPDIRECTDRAWSURFACE7 *) &(TextureState->Texture[i]) );

		// Get the stage.
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_ADDRESSU, &(TextureState->AddressU[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_ADDRESSV, &(TextureState->AddressV[i]) );

		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_COLORARG1, &(TextureState->ColorArg1[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_COLORARG2, &(TextureState->ColorArg2[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_COLOROP, &(TextureState->ColorOp[i]) );

		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_ALPHAARG1, &(TextureState->AlphaArg1[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_ALPHAARG2, &(TextureState->AlphaArg2[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_ALPHAOP, &(TextureState->AlphaOp[i]) );

		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_MAGFILTER, &(TextureState->MagFilter[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_MINFILTER, &(TextureState->MinFilter[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_MIPFILTER, &(TextureState->MipFilter[i]) );

		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_TEXCOORDINDEX, &(TextureState->TextureCoordinateIndex[i]) );
		ReturnCode = Direct3DDevice->GetTextureStageState ( i, D3DTSS_TEXTURETRANSFORMFLAGS, &(TextureState->TextureCoordinateTransformFlags[i]) );
	}
}
    
    
// Sets the texture state.
void Display_SetTextureState ( LPDIRECT3DDEVICE7 Direct3DDevice, TextureStateType *TextureState )
{
	return;

	for (int i=0; i<DISPLAY_MAXTEXTURES; ++i)
	{
		HRESULT         ReturnCode;

		// Set the texture.
		ReturnCode = Direct3DDevice->SetTexture ( i, (LPDIRECTDRAWSURFACE7) TextureState->Texture[i] );

		// Set the stage.
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ADDRESSU, TextureState->AddressU[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ADDRESSV, TextureState->AddressV[i] );

		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_COLORARG1, TextureState->ColorArg1[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_COLORARG2, TextureState->ColorArg2[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_COLOROP, TextureState->ColorOp[i] );

		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ALPHAARG1, TextureState->AlphaArg1[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ALPHAARG2, TextureState->AlphaArg2[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ALPHAOP, TextureState->AlphaOp[i] );

		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_MAGFILTER, TextureState->MagFilter[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_MINFILTER, TextureState->MinFilter[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_MIPFILTER, TextureState->MipFilter[i] );

		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_TEXCOORDINDEX, TextureState->TextureCoordinateIndex[i] );
		ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_TEXTURETRANSFORMFLAGS, TextureState->TextureCoordinateTransformFlags[i] );
	}

#if 0				
	// Clear all remaining stages.
	for (int i=DISPLAY_MAXTEXTURES; i<8; ++i)
	{
		HRESULT         ReturnCode;

		// Set the texture.
		ReturnCode = Direct3DDevice->SetTexture ( i, (LPDIRECTDRAWSURFACE7) NULL );

        // Set the stage.
	    ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
	    ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );

        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_COLOROP, D3DTOP_DISABLE );

        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_MAGFILTER, D3DTFG_POINT );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_MINFILTER, D3DTFN_POINT );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_MIPFILTER, D3DTFP_POINT );

        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_TEXCOORDINDEX, 0 );
        ReturnCode = Direct3DDevice->SetTextureStageState ( i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	}
#endif
}

    
// Clears the texture state for the display device.
// Here for debugging purposes.
void Display_ClearTextureState ( TextureStateType *TextureState )
{
	// Set the texture.
	TextureState->Texture[0] = NULL;

	// Set the state.
	TextureState->AddressU[0] = D3DTADDRESS_WRAP;
	TextureState->AddressV[0] = D3DTADDRESS_WRAP;

	TextureState->ColorArg1[0] = D3DTA_TEXTURE;
	TextureState->ColorArg2[0] = D3DTA_CURRENT;
	TextureState->ColorOp[0] = D3DTOP_MODULATE;

	TextureState->AlphaArg1[0] = D3DTA_TEXTURE;
	TextureState->AlphaArg2[0] = D3DTA_DIFFUSE;
	TextureState->AlphaOp[0] = D3DTOP_SELECTARG1;

	TextureState->MagFilter[0] = D3DTFG_POINT;
	TextureState->MinFilter[0] = D3DTFN_POINT;
	TextureState->MipFilter[0] = D3DTFP_POINT;

	TextureState->TextureCoordinateIndex[0] = 0;
	TextureState->TextureCoordinateTransformFlags[0] = D3DTTFF_DISABLE;

	// Clear all remaining stages.
	for (int i=1; i<DISPLAY_MAXTEXTURES; ++i)
	{
		// Set the texture.
		TextureState->Texture[i] = NULL;

		// Set the state.
		TextureState->AddressU[i] = D3DTADDRESS_WRAP;
		TextureState->AddressV[i] = D3DTADDRESS_WRAP;

		TextureState->ColorArg1[i] = D3DTA_TEXTURE;
		TextureState->ColorArg2[i] = D3DTA_CURRENT;
		TextureState->ColorOp[i] = D3DTOP_DISABLE;

		TextureState->AlphaArg1[i] = D3DTA_TEXTURE;
		TextureState->AlphaArg2[i] = D3DTA_DIFFUSE;
		TextureState->AlphaOp[i] = D3DTOP_SELECTARG1;

		TextureState->MagFilter[i] = D3DTFG_POINT;
		TextureState->MinFilter[i] = D3DTFN_POINT;
		TextureState->MipFilter[i] = D3DTFP_POINT;

		TextureState->TextureCoordinateIndex[i] = 0;
		TextureState->TextureCoordinateTransformFlags[i] = D3DTTFF_DISABLE;
	}
}


// Creates a D3D texture surface from an existing DDraw surface. 
void Display_CreateTexture ( const LPDIRECT3DDEVICE7 Direct3DDevice, const LPDIRECTDRAW7 DirectDrawObject, const LPDIRECTDRAWSURFACE7 SourceTexture, const int Width, const int Height, LPDIRECTDRAWSURFACE7 * const DestD3DTexture, float * const u, float * const v )
{
    DDSURFACEDESC2 SourceSurfaceDescription, DestSurfaceDescription;
   
	// Initialize.
	*DestD3DTexture = NULL;
	*u = 0.0f;
	*v = 0.0f;

	// Find the closest width and height that are powers of 2.
	const int TextureWidth = Display_GetPowerOfTwo ( Width );
	const int TextureHeight = Display_GetPowerOfTwo ( Height );

	// The texture's dimensions will probably be larger than the original DDraw surface.
	// Calculate the texture coordinates so that it leaves out unwanted area.
	*u = (float)Width  / (float)TextureWidth;
	*v = (float)Height / (float)TextureHeight;

    // Describe the type of surface we want to create.
    memset ( &DestSurfaceDescription, 0, sizeof ( DestSurfaceDescription ) );
    DestSurfaceDescription.dwSize = sizeof ( DDSURFACEDESC2 );
    DestSurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
    DestSurfaceDescription.dwWidth = TextureWidth;
    DestSurfaceDescription.dwHeight = TextureHeight;

	// Check if we are rendering with hardware.
	if ( g_pvp->m_pdd.m_fHardwareAccel )
	{
#if 1
		// Create the texture and let D3D driver decide where it store it.
		DestSurfaceDescription.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;
		DestSurfaceDescription.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#else
		// Create the texture using video memory.
		DestSurfaceDescription.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
#endif
	}
	else
	{
		// Create the texture using system memory.
		DestSurfaceDescription.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;  
	}

	// Find a transparent pixel format with 8 bit alpha.
    Direct3DDevice->EnumTextureFormats ( Display_EnumurateTransparentTextureFormats, &(DestSurfaceDescription.ddpfPixelFormat) );

    // Create a new surface for the texture.
	LPDIRECTDRAWSURFACE7	VidSurface;
	HRESULT					ReturnCode;
    if( FAILED( ReturnCode = DirectDrawObject->CreateSurface ( &DestSurfaceDescription, &VidSurface, NULL ) ) )
		{
		ShowError("Could not create texture surface.");
		}

	// Update the count.
//	NumVideoBytes += DestSurfaceDescription.dwWidth * DestSurfaceDescription.dwHeight * 4;

	// Check if we have a source surface to copy from.
	if ( SourceTexture != NULL )
	{
		// Copy from the DDraw surface to the new D3D texture.
		// We need to copy each pixel (rather than a blt) so we can convert to the different pixel format.
		if ( ReturnCode == DD_OK ) 
		{
			// Lock the destination texture surface so we can fill it with pixels.
			memset ( &DestSurfaceDescription, 0, sizeof ( DestSurfaceDescription ) );
			DestSurfaceDescription.dwSize = sizeof ( DestSurfaceDescription );
			ReturnCode = VidSurface->Lock ( NULL, &DestSurfaceDescription, 0, NULL );

			// Make sure we locked destination the surface.
			if ( ReturnCode == 0 )
			{
				// Lock the source DDraw surface so we can read its pixels.
				memset ( &SourceSurfaceDescription, 0, sizeof ( SourceSurfaceDescription ) );
				SourceSurfaceDescription.dwSize = sizeof ( SourceSurfaceDescription );
				ReturnCode = SourceTexture->Lock ( NULL, &SourceSurfaceDescription, 0, NULL );

				// Make sure we locked the source surface.
				if ( ReturnCode == 0 )
				{
					// Get the pointer to the actual pixel data.
					const unsigned char * const SourceLockedSurface = (unsigned char *) SourceSurfaceDescription.lpSurface;
					unsigned char * const DestLockedSurface = (unsigned char *) DestSurfaceDescription.lpSurface;

					int offset0 = 0;
					int offset1 = 0;

					// Copy the pixels.
					for (int i=0; i<Height; ++i)
					{
						for (int r=0; r<Width*4; r+=4)
						{
							// Copy the pixel.
#if 1
							*((unsigned int*)&(DestLockedSurface[offset0 + r])) = *((unsigned int*)&(SourceLockedSurface[offset1 + r])) | 0xFF000000;
#else
							*((unsigned int*)&(DestLockedSurface[offset0 + r])) = *((unsigned int*)&(SourceLockedSurface[offset1 + r]));
#endif
						}

						// Increment to the next horizontal line.
						offset1 += SourceSurfaceDescription.lPitch;
						offset0 += DestSurfaceDescription.lPitch;
					}

					// Unlock the source surface.
					SourceTexture->Unlock ( NULL );
				}

				// Unlock the destination texture surface.
				VidSurface->Unlock ( NULL );
			}
		}
	}

	// Copy the pointer.
	*DestD3DTexture = VidSurface;
}


// Destroys a texture that was created with Display_CreateTexture.
void Display_DestroyTexture ( LPDIRECTDRAWSURFACE7 Texture )
{
    // Destroy the texture.
	/*const HRESULT ReturnCode =*/ Texture->Release ( ); 
}


// Enumeration callback routine to find a texture format.
// Finds the best suited texture format for multi bit transparent textures.
HRESULT CALLBACK Display_EnumurateTransparentTextureFormats ( DDPIXELFORMAT *pddpf, VOID *param )
{
    // Skip any funky modes.
    if ( pddpf->dwFlags & (DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV) )
    {
        // Keep looking for texture formats.
        return ( DDENUMRET_OK );
    }

    // Skip any FourCC formats.
    if ( pddpf->dwFourCC != 0 )
    {
        // Keep looking for texture formats.
        return ( DDENUMRET_OK );
    }

    // We only want 32-bit formats.  Skip all others.
    if( pddpf->dwRGBBitCount != 32 )
    {
        // Keep looking for texture formats.
        return ( DDENUMRET_OK );
    }

    // Check if this format is ARGB = 8888.
    if ( (pddpf->dwRGBAlphaBitMask == 0xff000000) & (pddpf->dwRBitMask == 0x00ff0000) & (pddpf->dwGBitMask == 0x0000ff00) & (pddpf->dwBBitMask == 0x000000ff) )
    {
        // We found a good match. Copy the current pixel format.
        memcpy ( (DDPIXELFORMAT *) param, pddpf, sizeof ( DDPIXELFORMAT ) );

        // Return with DDENUMRET_CANCEL to end enumeration.
        return ( DDENUMRET_CANCEL );
    }
    else
    {
        // Keep looking for texture formats.
        return ( DDENUMRET_OK );
    }
}


// Returns the current Value if it is a already a power of 2...
// or the next power of two that is larger than Value.
int Display_GetPowerOfTwo ( const int Value )
{
    // Find a power of two which is 
    // greater than or equal to value. 
	int PowerOfTwo = 1;
    do
    {
        PowerOfTwo <<= 1;
	}
	while ( PowerOfTwo < Value );

    return PowerOfTwo;
}


// Copies a texture from the source to the destination.  
// Only the region inside Rect is copied.
void Display_CopyTexture ( LPDIRECT3DDEVICE7 Direct3DDevice, LPDIRECTDRAWSURFACE7 DestTexture, RECT *Rect, LPDIRECTDRAWSURFACE7 SourceTexture )
{
    HRESULT					ReturnCode;
    DDSURFACEDESC2			SourceSurfaceDescription, DestSurfaceDescription;
    RECT					ClippedRect;

	// Check if we have a source and destination texture.
	if ( (SourceTexture != NULL) &&
		 (DestTexture != NULL) )
	{
		// Lock the destination texture so we can fill it with pixels.
		memset ( &DestSurfaceDescription, 0, sizeof ( DestSurfaceDescription ) );
		DestSurfaceDescription.dwSize = sizeof ( DestSurfaceDescription );
		ReturnCode = DestTexture->Lock ( NULL, &DestSurfaceDescription, 0, NULL );

		// Make sure we locked the destination texture.
		if ( ReturnCode == 0 )
		{
			// Lock the source texture so we can read its pixels.
			memset ( &SourceSurfaceDescription, 0, sizeof ( SourceSurfaceDescription ) );
			SourceSurfaceDescription.dwSize = sizeof ( SourceSurfaceDescription );
			ReturnCode = SourceTexture->Lock ( NULL, &SourceSurfaceDescription, 0, NULL );

			// Make sure we locked the source texture.
			if ( ReturnCode == 0 )
			{
				// Get the pointer to the actual pixel data.
				const unsigned char * const SourceLockedSurface = (unsigned char *) SourceSurfaceDescription.lpSurface;
				unsigned char * const DestLockedSurface = (unsigned char *) DestSurfaceDescription.lpSurface;

				// Sanity check.
				if ( (SourceLockedSurface != NULL) && (DestLockedSurface != NULL) )
				{
					// Clip the rectangle to the dimensions of the source surface.
					ClippedRect.top = max ( Rect->top, 0 );
					ClippedRect.left = max ( Rect->left, 0 );
					ClippedRect.bottom = min ( Rect->bottom, g_pplayer->m_pin3d.m_dwRenderHeight );
					ClippedRect.right = min ( Rect->right, g_pplayer->m_pin3d.m_dwRenderWidth );

					// Calculate the width and height (so we don't extend outside of memory on either surface).
					const int Width = min ( (int)min ( SourceSurfaceDescription.dwWidth, DestSurfaceDescription.dwWidth ) , (ClippedRect.right - ClippedRect.left) );
					const int Height = min ( (int)min ( SourceSurfaceDescription.dwHeight, DestSurfaceDescription.dwHeight ) , (ClippedRect.bottom - ClippedRect.top) );

					int offset0 = 0;
					int offset1 = (ClippedRect.top * SourceSurfaceDescription.lPitch) + ClippedRect.left * 4;

					// Copy the pixels.
					for (int i=0; i<Height; ++i)
					{
						for (int r=0; r<Width*4; r+=4)
						{
							// Copy the pixel.
#if 1
							*((unsigned int*)&(DestLockedSurface[offset0 + r])) = *((unsigned int*)&(SourceLockedSurface[offset1 + r]));
#else
							*((unsigned int*)&(DestLockedSurface[offset0 + r])) = *((unsigned int*)&(SourceLockedSurface[offset1 + r])) | 0xFF000000;
#endif
						}

						// Increment to the next horizontal line.
						offset1 += SourceSurfaceDescription.lPitch;
						offset0 += DestSurfaceDescription.lPitch;
					}
				}

				// Unlock the source texture.
				SourceTexture->Unlock ( NULL );
			}
			else
			{
				ShowError("Could not lock source texture.");
			}

			// Unlock the destination texture.
			DestTexture->Unlock ( NULL );
		}
		else
		{
			ShowError("Could not lock destination texture.");
		}
	}
}

// Fills a texture with a constant Value in all channels.
void Display_ClearTexture ( const LPDIRECT3DDEVICE7 Direct3DDevice, const LPDIRECTDRAWSURFACE7 Texture, const char Value )
{
    // Check if we have a texture.
	if ( Texture != NULL )
	{
		// Lock the texture so we can clear the pixels.
		DDSURFACEDESC2 SurfaceDescription;
		memset ( &SurfaceDescription, 0, sizeof ( SurfaceDescription ) );
		SurfaceDescription.dwSize = sizeof ( SurfaceDescription );
		const HRESULT ReturnCode = Texture->Lock ( NULL, &SurfaceDescription, 0, NULL );

		// Make sure we locked the texture.
		if ( ReturnCode == 0 )
		{
			// Get the pointer to the actual pixel data.
			unsigned int * const LockedSurface = (unsigned int *) SurfaceDescription.lpSurface;

			const unsigned int ValueU = (unsigned int)Value | ((unsigned int)Value<<8) | ((unsigned int)Value<<16) | ((unsigned int)Value<<24);

			DWORD offset = 0;
			// Clear the pixels.
			for (DWORD i=0; i<SurfaceDescription.dwHeight; ++i)
			{
				const DWORD end = SurfaceDescription.dwWidth+offset;
				for (DWORD r=offset; r<end; ++r)
					// Clear the pixel.
					LockedSurface[r] = ValueU;

				// Increment to the next horizontal line.
				offset += SurfaceDescription.lPitch/4;
			}

			// Unlock the texture.
			Texture->Unlock ( NULL );
		}
	}
}


// Draws the primitive to the current render target.
// If D3D blitting is enabled, the primitive is also drawn to the texture buffer.
HRESULT Display_DrawIndexedPrimitive ( LPDIRECT3DDEVICE7 Direct3DDevice, D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags )
{
	// Draw the primitive.
	HRESULT	ReturnCode = Direct3DDevice->DrawIndexedPrimitive( d3dptPrimitiveType, dwVertexTypeDesc, lpvVertices, dwVertexCount, lpwIndices, dwIndexCount, dwFlags );

	// Check if we are blitting with D3D.
	if ( g_pvp->m_pdd.m_fUseD3DBlit )
	{
		LPDIRECTDRAWSURFACE7	RestoreRenderTarget;
		// Save the render target.
		Direct3DDevice->GetRenderTarget ( &RestoreRenderTarget );	
		
		// Direct all renders to the back texture buffer.
		Direct3DDevice->SetRenderTarget ( g_pplayer->m_pin3d.m_pddsBackTextureBuffer, 0L );					
		
		// Redraw... this time to back texture buffer.
		ReturnCode = Direct3DDevice->DrawIndexedPrimitive( d3dptPrimitiveType, dwVertexTypeDesc, lpvVertices, dwVertexCount, lpwIndices, dwIndexCount, dwFlags );

		// Restore the render target.
		Direct3DDevice->SetRenderTarget ( RestoreRenderTarget, 0L );	
	}

	return ReturnCode;
}
