// license:GPLv3+

#include "Helpers.fxh"

#define Filter_None     0.
#define Filter_Additive 1.
#define Filter_Overlay  2.
#define Filter_Multiply 3.
#define Filter_Screen   4.

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;

const float4 staticColor_Alpha;
const float4 alphaTestValueAB_filterMode_addBlend;     // last one is bool
const float3 amount_blend_modulate_vs_add_flasherMode; // last one is integer

const float4 lightCenter_doShadow;

texture Texture0; // base texture
texture Texture1; // second image

sampler2D tex_flasher_A : TEXUNIT0 = sampler_state // base texture
{
    Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
    //ADDRESSU  = Wrap; //!! ?
    //ADDRESSV  = Wrap;
    SRGBTexture = true;
};

sampler2D tex_flasher_B : TEXUNIT1 = sampler_state // texB
{
    Texture   = (Texture1);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU  = Wrap;
    ADDRESSV  = Wrap;
    SRGBTexture = true;
};

struct VS_OUTPUT_2D
{
   float4 pos      : POSITION;
   float3 tablePos : TEXCOORD0;
   float2 tex0     : TEXCOORD1;
};

VS_OUTPUT_2D vs_simple_main (const in float4 vPosition : POSITION0,
                             // const in float3 vNormal   : NORMAL0, Part of vertex format but unused
                             const in float2 tc        : TEXCOORD0)
{
   VS_OUTPUT_2D Out;
   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tablePos = vPosition.xyz;
   Out.tex0 = tc;
   return Out;
}

#include "BallShadows.fxh"

float4 ps_main_noLight(const in VS_OUTPUT_2D IN)
   : COLOR
{
   float4 pixel1,pixel2;
   BRANCH if (amount_blend_modulate_vs_add_flasherMode.z < 2.) // Mode 0 & 1
   {
      pixel1 = tex2D(tex_flasher_A, IN.tex0);
      if (pixel1.a <= alphaTestValueAB_filterMode_addBlend.x)
         return float4(0.0, 0.0, 0.0, 0.0);
   }
   BRANCH if (amount_blend_modulate_vs_add_flasherMode.z == 1.)
   {
      pixel2 = tex2D(tex_flasher_B, IN.tex0);
      if (pixel2.a <= alphaTestValueAB_filterMode_addBlend.y)
         return float4(0.0, 0.0, 0.0, 0.0);
   }

   float4 result = staticColor_Alpha; // Mode 2 wires this through

   if (amount_blend_modulate_vs_add_flasherMode.z == 0.) // Mode 0 mods it by Texture
      result *= pixel1;

   BRANCH if (amount_blend_modulate_vs_add_flasherMode.z == 1.) // Mode 1 allows blends between Tex 1 & 2, and then mods the staticColor with it
   {
      BRANCH if (alphaTestValueAB_filterMode_addBlend.z == Filter_Overlay)
         result *= OverlayHDR(pixel1,pixel2); // could be HDR
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Multiply)
         result *= Multiply(pixel1,pixel2, amount_blend_modulate_vs_add_flasherMode.x);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Additive)
         result *= Additive(pixel1,pixel2, amount_blend_modulate_vs_add_flasherMode.x);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Screen)
         result *= ScreenHDR(pixel1,pixel2); // could be HDR
   }

   BRANCH if (lightCenter_doShadow.w != 0.)
   {
      const float3 light_dir = IN.tablePos.xyz - lightCenter_doShadow.xyz;
      const float light_dist = length(light_dir);
      const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
      result.rgb *= shadow;
   }

   if (alphaTestValueAB_filterMode_addBlend.w == 0.)
      return float4(result.xyz, saturate(result.a)); // Need to clamp here or we get some saturation artifacts on some tables
   else if (amount_blend_modulate_vs_add_flasherMode.y > 0.) // AB_ADD
   {
      // Blend unit set to dst' = dst * (1 - src) - src * srcAlpha. Emitting a negative color (negative as it gets
      // blended with '1.0-thisvalue', the 1.0 being what modulates the underlying elements, and unwanted for the
      // term below) along a 1/m - 1 alpha gives the historical additive behavior dst' = dst + L * (1 - m * (1 - dst)),
      // for L = result.rgb * result.a the emitted light: the light is added, and what is behind the flasher is
      // amplified on top of it, the more so the brighter the flasher is
      return float4(result.xyz*(-amount_blend_modulate_vs_add_flasherMode.y*result.a), 1.0/amount_blend_modulate_vs_add_flasherMode.y - 1.0);
   }
   else // AB_ABSORB, which a negative 'modulate vs add' selects
   {
      // Blend unit set to the premultiplied alpha 'over' dst' = src + dst * (1 - srcAlpha). Emitting the
      // premultiplied light along an m * coverage alpha gives dst' = L + dst * (1 - m * result.a): the very same
      // light added, but over a background the flasher now absorbs instead of amplifying, and independently of how
      // bright it is. That is a glass like reflection, m being how reflective (hence opaque) the glass is. The
      // flasher alpha still drives the coverage, so its shape and its edges fade the absorption out as well.
      // Saturated since opacity may go above 100%: the added light keeps that boost, but absorbing more than all of
      // the background would take it negative, inverting it instead of blackening it
      return float4(result.xyz*result.a, saturate(-amount_blend_modulate_vs_add_flasherMode.y*result.a));
   }
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
