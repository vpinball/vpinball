#include "Helpers.fxh"

#define Filter_None	    0
#define Filter_Additive	1
#define Filter_Overlay	2
#define Filter_Multiply	3
#define Filter_Screen   4

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;

float3 staticColor = float3(1.,1.,1.);
float  fAlpha = 1.0;
float  fAlphaTestValue = 128.0/255.0;
float2 amount__blend_modulate_vs_add;
int    filterMode;

bool bPerformAlphaTest;
bool bAdd_Blend;

texture Texture0; // base texture
texture Texture1; // second image

sampler2D texSampler0 : TEXUNIT0 = sampler_state // base texture
{
	Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
	//ADDRESSU  = Wrap; //!! ?
	//ADDRESSV  = Wrap;
	//!! SRGBTexture = true;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state // texB
{
	Texture	  = (Texture1);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

struct VS_OUTPUT_2D
{
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
};

VS_OUTPUT_2D vs_simple_main (float4 vPosition : POSITION0, 
                             float2 tc        : TEXCOORD0)
{
   VS_OUTPUT_2D Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;

   return Out;
}

float4 ps_main_textureOne_noLight(in VS_OUTPUT_2D IN) : COLOR
{
   const float4 pixel = tex2D(texSampler0, IN.tex0);

   if (bPerformAlphaTest && pixel.a<=fAlphaTestValue )
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   float4 result;
   result.xyz = staticColor*InvGamma(pixel.xyz);
   result.a = pixel.a*fAlpha;

   if(!bAdd_Blend)
      return result;
   else
      return float4(result.xyz*(-amount__blend_modulate_vs_add.y*result.a), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
                    1.0/amount__blend_modulate_vs_add.y - 1.0);
}

float4 ps_main_textureAB_noLight(in VS_OUTPUT_2D IN) : COLOR
{
   float4 pixel1 = tex2D(texSampler0, IN.tex0);
   float4 pixel2 = tex2D(texSampler1, IN.tex0);

   if (bPerformAlphaTest && (pixel1.a<=fAlphaTestValue || pixel2.a<=fAlphaTestValue))
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   pixel1.xyz = InvGamma(pixel1.xyz);
   pixel2.xyz = InvGamma(pixel2.xyz);

   float4 result;
   result.xyz = staticColor;
   result.a = fAlpha;

   if ( filterMode == Filter_Overlay )
      result *= Overlay(pixel1,pixel2);
   else if ( filterMode == Filter_Multiply )
      result *= Multiply(pixel1,pixel2, amount__blend_modulate_vs_add.x);
   else if ( filterMode == Filter_Additive )
      result *= Additive(pixel1,pixel2, amount__blend_modulate_vs_add.x);
   else if ( filterMode == Filter_Screen )
      result *= Screen(pixel1,pixel2);

   if(!bAdd_Blend)
      return result;
   else
      return float4(result.xyz*(-amount__blend_modulate_vs_add.y*result.a), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
                    1.0/amount__blend_modulate_vs_add.y - 1.0);
}

float4 ps_main_noLight(in VS_OUTPUT_2D IN) : COLOR
{
	if(!bAdd_Blend)
      return float4(staticColor,fAlpha);
	else
	  return float4(staticColor*(-amount__blend_modulate_vs_add.y*fAlpha), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	                1.0/amount__blend_modulate_vs_add.y - 1.0);
}

//
// Techniques
//

technique basic_with_textureOne_noLight
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_simple_main(); 
	  PixelShader = compile ps_3_0 ps_main_textureOne_noLight();
   } 
}

technique basic_with_textureAB_noLight
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_simple_main(); 
	  PixelShader = compile ps_3_0 ps_main_textureAB_noLight();
   } 
}

technique basic_with_noLight
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_simple_main(); 
	  PixelShader = compile ps_3_0 ps_main_noLight();
   } 
}
