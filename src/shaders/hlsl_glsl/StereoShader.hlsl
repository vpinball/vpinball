// license:GPLv3+

#include "Helpers.fxh"

texture Texture0; // FB
texture Texture4; // DepthBuffer

sampler2D tex_stereo_fb : TEXUNIT0 = sampler_state // Framebuffer (unfiltered)
{
    Texture = (Texture0);
    MIPFILTER = NONE;
    MAGFILTER = NONE;
    MINFILTER = NONE;
    ADDRESSU = Wrap;
    ADDRESSV = Wrap;
};

sampler2D tex_stereo_depth : TEXUNIT4 = sampler_state // Depth
{
    Texture = (Texture4);
    MIPFILTER = NONE;
    MAGFILTER = NONE;
    MINFILTER = NONE;
    ADDRESSU = Wrap;
    ADDRESSV = Wrap;
};

#include "Stereo.fxh"

struct VS_OUTPUT_2D
{
   float4 pos  : POSITION; 
   float2 tex0 : TEXCOORD0;
};


VS_OUTPUT_2D vs_main_no_trafo (const in float4 vPosition  : POSITION0,
                               const in float2 tc         : TEXCOORD0)
{
   VS_OUTPUT_2D Out;
   Out.pos = float4(vPosition.xy, 0.0, 1.0);
   // DirectX has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
   Out.tex0 = tc + 0.5 * w_h_height.xy;
   return Out;
}


float4 ps_main_tb(const in VS_OUTPUT_2D IN) : COLOR
{
   return float4(gatherEyeColor(float2(IN.tex0.x, IN.tex0.y * 2.0), IN.tex0.y < 0.5), 1.0);
}


float4 ps_main_sbs(const in VS_OUTPUT_2D IN) : COLOR
{
   return float4(gatherEyeColor(float2(IN.tex0.x * 2.0, IN.tex0.y), IN.tex0.x < 0.5), 1.0);
}


float4 ps_main_int(const in VS_OUTPUT_2D IN) : COLOR
{
   const int y = w_h_height.z * IN.tex0.y;
   return float4(gatherEyeColor(IN.tex0, ((y + 1) / 2 == y / 2)), 1.0); //last check actually means (y&1)
}


float4 ps_main_flipped_int(const in VS_OUTPUT_2D IN) : COLOR
{
   const int y = w_h_height.z * IN.tex0.y;
   return float4(gatherEyeColor(IN.tex0, ((y + 1) / 2 != y / 2)), 1.0); //last check actually means (y&1)
}


float4 ps_main_stereo_srgb_anaglyph(const in VS_OUTPUT_2D IN) : COLOR
{
    float3 lCol, rCol;
    gatherLeftRightColors(IN.tex0, lCol, rCol);
    return float4(FBGamma(LinearAnaglyph(InvGamma(lCol), InvGamma(rCol))), 1.0);
}


float4 ps_main_stereo_gamma_anaglyph(const in VS_OUTPUT_2D IN) : COLOR
{
    float3 lCol, rCol;
    gatherLeftRightColors(IN.tex0, lCol, rCol);
    lCol = pow(lCol, float3(Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w));
    rCol = pow(rCol, float3(Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w));
    return float4(pow(LinearAnaglyph(lCol, rCol), float3(1. / Stereo_LeftLuminance_Gamma.w, 1. / Stereo_LeftLuminance_Gamma.w, 1. / Stereo_LeftLuminance_Gamma.w)), 1.0);
}


float4 ps_main_stereo_srgb_dyndesat_anaglyph(const in VS_OUTPUT_2D IN) : COLOR
{
    float3 lCol, rCol, lColDesat, rColDesat;
    gatherLeftRightColors(IN.tex0, lCol, rCol);
    DynamicDesatAnaglyph(InvGamma(lCol), InvGamma(rCol), lColDesat, rColDesat);
    return float4(FBGamma(LinearAnaglyph(lColDesat, rColDesat)), 1.0);
}


float4 ps_main_stereo_gamma_dyndesat_anaglyph(const in VS_OUTPUT_2D IN) : COLOR
{
    float3 lCol, rCol;
    gatherLeftRightColors(IN.tex0, lCol, rCol);
    lCol = pow(lCol, float3(Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w));
    rCol = pow(rCol, float3(Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w, Stereo_LeftLuminance_Gamma.w));
    float3 lColDesat, rColDesat;
    DynamicDesatAnaglyph(lCol, rCol, lColDesat, rColDesat);
    return float4(pow(LinearAnaglyph(lColDesat, rColDesat), float3(1. / Stereo_LeftLuminance_Gamma.w, 1. / Stereo_LeftLuminance_Gamma.w, 1. / Stereo_LeftLuminance_Gamma.w)), 1.0);
}


float4 ps_main_stereo_deghost_anaglyph(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 lCol, rCol;
   gatherLeftRightColors(IN.tex0, lCol, rCol);
   return float4(DeghostAnaglyph(lCol, rCol), 1.0);
}


////TECHNIQUES

technique stereo_SBS { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_sbs(); } }
technique stereo_TB { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_tb(); } }
technique stereo_Int { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_int(); } }
technique stereo_Flipped_Int { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_flipped_int(); } }
technique Stereo_sRGBAnaglyph { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_stereo_srgb_anaglyph(); } }
technique Stereo_GammaAnaglyph { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_stereo_gamma_anaglyph(); } }
technique Stereo_sRGBDynDesatAnaglyph { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_stereo_srgb_dyndesat_anaglyph(); } }
technique Stereo_GammaDynDesatAnaglyph { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_stereo_gamma_dyndesat_anaglyph(); } }
technique Stereo_DeghostAnaglyph { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_stereo_deghost_anaglyph(); } }
