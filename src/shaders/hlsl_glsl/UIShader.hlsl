// license:GPLv3+

#include "Helpers.fxh"

uniform float4x4 matWorldView : WORLDVIEWPROJ;

uniform float4 staticColor_Alpha;
#define stereoOfs staticColor_Alpha.x
#define sdrScale staticColor_Alpha.a

texture Texture0; // base texture

sampler2D tex_base_color : TEXUNIT0 = sampler_state // base texture
{
    Texture	  = (Texture0);
    SRGBTexture = true;
};

struct VS_OUTPUT_2D
{
   float4 pos      : POSITION;
   float4 color    : COLOR0;
   float2 tex0     : TEXCOORD1;
};

VS_OUTPUT_2D vs_liveui_main (const in float4 vPosition : POSITION0,
                             const in float3 vNormal   : NORMAL0,
                             const in float2 tc        : TEXCOORD0)
{
   VS_OUTPUT_2D Out;
   Out.pos = mul(float4(vPosition.x, vPosition.y, 0.0, 1.0), matWorldView);
   Out.color = sdrScale * float4(vNormal.rgb, vPosition.z);
   Out.tex0 = tc;
   return Out;
}

float4 ps_liveui_main(const in VS_OUTPUT_2D IN)
   : COLOR
{
   return tex2D(tex_base_color, IN.tex0) * IN.color;
}

//
// Techniques
//

technique LiveUI
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_liveui_main();
      PixelShader  = compile ps_3_0 ps_liveui_main();
   }
}
