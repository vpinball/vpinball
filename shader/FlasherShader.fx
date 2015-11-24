#include "Helpers.fxh"

#define Filter_None	    0.
#define Filter_Additive	1.
#define Filter_Overlay	2.
#define Filter_Multiply	3.
#define Filter_Screen   4.

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;

float4 staticColor_Alpha;
float4 alphaTestValueAB_filterMode_addBlend; // last one bool
float4 amount__blend_modulate_vs_add__hdrTexture01; // last two are bools

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
   float4 pixel = tex2D(texSampler0, IN.tex0);

   if (pixel.a<=alphaTestValueAB_filterMode_addBlend.x)
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   if(amount__blend_modulate_vs_add__hdrTexture01.z == 0.)
       pixel.xyz = InvGamma(pixel.xyz);

   float4 result;
   result.xyz = staticColor_Alpha.xyz*pixel.xyz;
   result.a = pixel.a*staticColor_Alpha.w;

   if(alphaTestValueAB_filterMode_addBlend.w == 0.)
      return result;
   else
      return float4(result.xyz*(-amount__blend_modulate_vs_add__hdrTexture01.y*result.a), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
                    1.0/amount__blend_modulate_vs_add__hdrTexture01.y - 1.0);
}

float4 ps_main_textureAB_noLight(in VS_OUTPUT_2D IN) : COLOR
{
   float4 pixel1 = tex2D(texSampler0, IN.tex0);
   float4 pixel2 = tex2D(texSampler1, IN.tex0);

   if (pixel1.a<=alphaTestValueAB_filterMode_addBlend.x || pixel2.a<=alphaTestValueAB_filterMode_addBlend.y)
    clip(-1);           //stop the pixel shader if alpha test should reject pixel

   if(amount__blend_modulate_vs_add__hdrTexture01.z == 0.)
      pixel1.xyz = InvGamma(pixel1.xyz);
   if(amount__blend_modulate_vs_add__hdrTexture01.w == 0.)
      pixel2.xyz = InvGamma(pixel2.xyz);

   float4 result = staticColor_Alpha;

   if ( alphaTestValueAB_filterMode_addBlend.z == Filter_Overlay )
      result *= OverlayHDR(pixel1,pixel2); // could be HDR
   else if ( alphaTestValueAB_filterMode_addBlend.z == Filter_Multiply )
      result *= Multiply(pixel1,pixel2, amount__blend_modulate_vs_add__hdrTexture01.x);
   else if ( alphaTestValueAB_filterMode_addBlend.z == Filter_Additive )
      result *= Additive(pixel1,pixel2, amount__blend_modulate_vs_add__hdrTexture01.x);
   else if ( alphaTestValueAB_filterMode_addBlend.z == Filter_Screen )
      result *= ScreenHDR(pixel1,pixel2); // could be HDR

   if(alphaTestValueAB_filterMode_addBlend.w == 0.)
      return result;
   else
      return float4(result.xyz*(-amount__blend_modulate_vs_add__hdrTexture01.y*result.a), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
                    1.0/amount__blend_modulate_vs_add__hdrTexture01.y - 1.0);
}

float4 ps_main_noLight(in VS_OUTPUT_2D IN) : COLOR
{
	if(alphaTestValueAB_filterMode_addBlend.w == 0.)
      return staticColor_Alpha;
	else
	  return float4(staticColor_Alpha.xyz*(-amount__blend_modulate_vs_add__hdrTexture01.y*staticColor_Alpha.w), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	                1.0/amount__blend_modulate_vs_add__hdrTexture01.y - 1.0);
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
