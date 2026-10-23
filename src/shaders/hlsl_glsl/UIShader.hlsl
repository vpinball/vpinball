// license:GPLv3+

#include "Helpers.fxh"

uniform float4x4 matWorldView : WORLDVIEWPROJ;

uniform float4 clip_plane;

texture Texture0; // base texture

sampler2D tex_base_color : TEXUNIT0 = sampler_state // base texture
{
    Texture	  = (Texture0);
    SRGBTexture = false; // ImGui works in sRGB: sample raw values (GL/BGFX re-encode instead, as decoding is part of their texture format)
};

struct VS_OUTPUT_2D
{
   float4 pos      : POSITION;
   float4 color    : COLOR0;
   float2 tex0     : TEXCOORD1;
   float2 clipPos  : TEXCOORD2;
};

VS_OUTPUT_2D vs_liveui_main (const in float4 vPosition : POSITION0,
                             const in float3 vNormal   : NORMAL0,
                             const in float2 tc        : TEXCOORD0)
{
   VS_OUTPUT_2D Out;
   Out.clipPos = vPosition.xy;
   Out.pos = mul(float4(vPosition.x, vPosition.y, 0.0, 1.0), matWorldView);
   Out.color = float4(vNormal.rgb, vPosition.z); // No WCG (HDR) backbuffer on DX9, so no SDR white level conversion (see fs_imgui.sc)
   Out.tex0 = tc;
   return Out;
}

float4 ps_liveui_main(const in VS_OUTPUT_2D IN)
   : COLOR
{
   if (IN.clipPos.x < clip_plane.x || IN.clipPos.y < clip_plane.y || IN.clipPos.x > clip_plane.z || IN.clipPos.y > clip_plane.w)
      discard;
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
