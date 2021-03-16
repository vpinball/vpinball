#include "Helpers.fxh"

#define Filter_None	    0.
#define Filter_Additive	1.
#define Filter_Overlay	2.
#define Filter_Multiply	3.
#define Filter_Screen   4.

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;

const float4 staticColor_Alpha;
const float4 alphaTestValueAB_filterMode_addBlend;        // last one is bool
const float4 amount__blend_modulate_vs_add__hdrTexture01; // last two are bools
const float  flasherMode;

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
    Texture   = (Texture1);
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

VS_OUTPUT_2D vs_simple_main (const in float4 vPosition : POSITION0,
                             const in float2 tc        : TEXCOORD0)
{
   VS_OUTPUT_2D Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;

   return Out;
}

float4 ps_main_noLight(const in VS_OUTPUT_2D IN) : COLOR
{
   float4 pixel1,pixel2;
   bool stop = false;

   [branch] if (flasherMode < 2.) // Mode 0 & 1
   {
      pixel1 = tex2D(texSampler0, IN.tex0);
      stop = (pixel1.a <= alphaTestValueAB_filterMode_addBlend.x);
   }
   [branch] if (flasherMode == 1.)
   {
      pixel2 = tex2D(texSampler1, IN.tex0);
      stop = (stop || pixel2.a <= alphaTestValueAB_filterMode_addBlend.y);
   }

   clip(stop ? -1 : 1); // stop the pixel shader if alpha test should reject pixel (Mode 0 & 1)

   [branch] if ((flasherMode < 2.) && (amount__blend_modulate_vs_add__hdrTexture01.z == 0.)) // Mode 0 & 1
      pixel1.xyz = InvGamma(pixel1.xyz);

   float4 result = staticColor_Alpha; // Mode 2 wires this through

   if (flasherMode == 0.) // Mode 0 mods it by Texture
      result *= pixel1;

   [branch] if (flasherMode == 1.) // Mode 1 allows blends between Tex 1 & 2, and then mods the staticColor with it
   {
      [branch] if (amount__blend_modulate_vs_add__hdrTexture01.w == 0.)
         pixel2.xyz = InvGamma(pixel2.xyz);

      [branch] if (alphaTestValueAB_filterMode_addBlend.z == Filter_Overlay)
         result *= OverlayHDR(pixel1,pixel2); // could be HDR
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Multiply)
         result *= Multiply(pixel1,pixel2, amount__blend_modulate_vs_add__hdrTexture01.x);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Additive)
         result *= Additive(pixel1,pixel2, amount__blend_modulate_vs_add__hdrTexture01.x);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Screen)
         result *= ScreenHDR(pixel1,pixel2); // could be HDR
   }

   if (alphaTestValueAB_filterMode_addBlend.w == 0.)
      return result;
   else
      return float4(result.xyz*(-amount__blend_modulate_vs_add__hdrTexture01.y*result.a), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
                    1.0/amount__blend_modulate_vs_add__hdrTexture01.y - 1.0);
}

//
// Techniques
//

technique basic_noLight
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_simple_main();
      PixelShader  = compile ps_3_0 ps_main_noLight();
   }
}
