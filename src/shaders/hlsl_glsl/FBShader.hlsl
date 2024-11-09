// license:GPLv3+

#include "Helpers.fxh"

// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
const float4 w_h_height;

const float4 ms_zpd_ya_td;

const float2 AO_scale_timeblur;

const float4 bloom_dither_colorgrade;
#define do_bloom (bloom_dither_colorgrade.x == 1.)
#define do_dither (bloom_dither_colorgrade.y == 1.)
#define color_grade (bloom_dither_colorgrade.z == 1.)

texture Texture0; // FB
texture Texture1; // Bloom
texture Texture2; // Color grade
texture Texture3; // AO Result
texture Texture4; // DepthBuffer
texture Texture5; // AO Dither

sampler2D tex_fb_unfiltered : TEXUNIT0 = sampler_state // Framebuffer (unfiltered)
{
   Texture   = (Texture0);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = POINT;
   MINFILTER = POINT;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};

sampler2D tex_fb_filtered : TEXUNIT0 = sampler_state // Framebuffer (filtered)
{
   Texture   = (Texture0);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};

sampler2D tex_bloom : TEXUNIT1 = sampler_state // Bloom
{
   Texture   = (Texture1);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU = Clamp;
   ADDRESSV = Clamp;
};

/*sampler2D texSamplerNormals : TEXUNIT1 = sampler_state // Normals (unfiltered)
{
   Texture   = (Texture1);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = POINT;
   MINFILTER = POINT;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};*/

sampler2D tex_color_lut : TEXUNIT2 = sampler_state // Color grade LUT
{
   Texture   = (Texture2);
   MIPFILTER = NONE;
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};

sampler2D tex_ao : TEXUNIT3 = sampler_state // AO Result
{
   Texture   = (Texture3);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};

sampler2D tex_depth : TEXUNIT4 = sampler_state // Depth
{
   Texture   = (Texture4);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = POINT;
   MINFILTER = POINT;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};

sampler2D tex_ao_dither : TEXUNIT5 = sampler_state // AO dither
{
   Texture   = (Texture5);
   MIPFILTER = NONE;
   MAGFILTER = POINT;
   MINFILTER = POINT;
   ADDRESSU  = Wrap;
   ADDRESSV  = Wrap;
};

struct VS_OUTPUT_2D
{
   float4 pos  : POSITION; 
   float2 tex0 : TEXCOORD0;
};


#include "FBShader.fxh"


//
//
//

/*static const float bayer_dither_pattern[8][8] = {
    {( 0/64.0-0.5)/255.0, (32/64.0-0.5)/255.0, ( 8/64.0-0.5)/255.0, (40/64.0-0.5)/255.0, ( 2/64.0-0.5)/255.0, (34/64.0-0.5)/255.0, (10/64.0-0.5)/255.0, (42/64.0-0.5)/255.0},   
    {(48/64.0-0.5)/255.0, (16/64.0-0.5)/255.0, (56/64.0-0.5)/255.0, (24/64.0-0.5)/255.0, (50/64.0-0.5)/255.0, (18/64.0-0.5)/255.0, (58/64.0-0.5)/255.0, (26/64.0-0.5)/255.0},   
    {(12/64.0-0.5)/255.0, (44/64.0-0.5)/255.0, ( 4/64.0-0.5)/255.0, (36/64.0-0.5)/255.0, (14/64.0-0.5)/255.0, (46/64.0-0.5)/255.0, ( 6/64.0-0.5)/255.0, (38/64.0-0.5)/255.0},   
    {(60/64.0-0.5)/255.0, (28/64.0-0.5)/255.0, (52/64.0-0.5)/255.0, (20/64.0-0.5)/255.0, (62/64.0-0.5)/255.0, (30/64.0-0.5)/255.0, (54/64.0-0.5)/255.0, (22/64.0-0.5)/255.0},   
    {( 3/64.0-0.5)/255.0, (35/64.0-0.5)/255.0, (11/64.0-0.5)/255.0, (43/64.0-0.5)/255.0, ( 1/64.0-0.5)/255.0, (33/64.0-0.5)/255.0, ( 9/64.0-0.5)/255.0, (41/64.0-0.5)/255.0},   
    {(51/64.0-0.5)/255.0, (19/64.0-0.5)/255.0, (59/64.0-0.5)/255.0, (27/64.0-0.5)/255.0, (49/64.0-0.5)/255.0, (17/64.0-0.5)/255.0, (57/64.0-0.5)/255.0, (25/64.0-0.5)/255.0},
    {(15/64.0-0.5)/255.0, (47/64.0-0.5)/255.0, ( 7/64.0-0.5)/255.0, (39/64.0-0.5)/255.0, (13/64.0-0.5)/255.0, (45/64.0-0.5)/255.0, ( 5/64.0-0.5)/255.0, (37/64.0-0.5)/255.0},
    {(63/64.0-0.5)/255.0, (31/64.0-0.5)/255.0, (55/64.0-0.5)/255.0, (23/64.0-0.5)/255.0, (61/64.0-0.5)/255.0, (29/64.0-0.5)/255.0, (53/64.0-0.5)/255.0, (21/64.0-0.5)/255.0} };
*/

/*float triangularPDF(const float r) // from -1..1, c=0 (with random no r=0..1)
{
   float p = 2.*r;
   const bool b = (p > 1.);
   if (b)
      p = 2.-p;
   p = 1.-sqrt(p);
   return b ? p : -p;
}
float3 triangularPDF(const float3 r) // from -1..1, c=0 (with random no r=0..1)
{
   return float3(triangularPDF(r.x), triangularPDF(r.y), triangularPDF(r.z));
}*/

/*float DitherGradientNoise(const float2 tex0)
{
   // Interleaved Gradient Noise from "NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE" http://advances.realtimerendering.com/s2014/index.html
   const float3 magic = float3(0.06711056, 0.00583715, 52.9829189);

   return frac(magic.z * frac(dot(tex0, magic.xy)));
}

float Dither64(const float2 tex0, const float time)
{
   const float3 k0 = float3(33., 52., 25.);

   return frac(dot(float3(tex0, time), k0 * (1.0/64.0)));
}*/

float3 blue_gauss_noise(const float2 c1, const float3 rgb)
{
   // https://www.shadertoy.com/view/XljyRR
   //float2 c0 = float2(c1.x - 1., c1.y);
   //float2 c2 = float2(c1.x + 1., c1.y);
   const float3 cx = float3(c1.x - 1., c1.x, c1.x + 1.);
   const float4 f0 = frac(float4(cx * 9.1031, c1.y * 8.1030));
   const float4 f1 = frac(float4(cx * 7.0973, c1.y * 6.0970));
   const float4 t0 = float4(f0.xw,f1.xw); //frac(c0.xyxy * float4(.1031,.1030,.0973,.0970));
   const float4 t1 = float4(f0.yw,f1.yw); //frac(c1.xyxy * float4(.1031,.1030,.0973,.0970));
   const float4 t2 = float4(f0.zw,f1.zw); //frac(c2.xyxy * float4(.1031,.1030,.0973,.0970));
   const float4 p0 = t0 + dot(t0, t0.wzxy + 19.19);
   const float4 p1 = t1 + dot(t1, t1.wzxy + 19.19);
   const float4 p2 = t2 + dot(t2, t2.wzxy + 19.19);
   const float4 n0 = frac(p0.zywx * (p0.xxyz + p0.yzzw));
   const float4 n1 = frac(p1.zywx * (p1.xxyz + p1.yzzw));
   const float4 n2 = frac(p2.zywx * (p2.xxyz + p2.yzzw));
   const float4 r4 = 0.5*n1 - 0.125*(n0 + n2);
   const float  r  = r4.x+r4.y+r4.z+r4.w - 0.5;

   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??
   return rgb + float3(r,-r,r) * (1.0/(quantSteps - 1.0));
}

#if 0
// seems like everybody uses this, but the amount of flickering patterns generated is very noticeable
float3 DitherVlachos(const float2 tex0, const float3 rgb)
{
   // Vlachos 2016, "Advanced VR Rendering", but using a TPDF
   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??

   const float noise = dot(float2(171.0, 231.0), tex0);
   const float3 noise3 = frac(noise * (1.0 / float3(103.0, 71.0, 97.0)));

   return rgb + triangularPDF(noise3)/*(noise3*2.0-1.0)*/ * (1.0/(quantSteps - 1.0));
}
#endif

float3 FBDither(const float3 color, /*const int2 pos*/const float2 tex0)
{
   BRANCH if (!do_dither)
       return color;

   //return color + bayer_dither_pattern[pos.x%8][pos.y%8];

#ifndef BLUE_NOISE_DITHER
   //return DitherVlachos(floor(tex0*(1.0/w_h_height.xy)+0.5) + w_h_height.z/*w*/, color);       // note that w_h_height.w is the same nowadays
   return blue_gauss_noise(floor(tex0*(1.0/w_h_height.xy)+0.5) + 0.07*w_h_height.z/*w*/, color); // dto.
#else // needs tex_ao_dither
   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??

   // TPDF:
   const float3 dither = /*float3(GradientNoise(tex0 / w_h_height.xy + w_h_height.zw*3.141),
      GradientNoise(tex0.yx / w_h_height.yx + w_h_height.zw*1.618),
      GradientNoise(tex0 / w_h_height.xy + w_h_height.wz*2.718281828459));*/
   // Dither64(tex0 / w_h_height.xy+w_h_height.zw, 0.);
       texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.zw*3.141).xyz;
   return color + triangularPDF(dither) * (1.0/quantSteps); // quantSteps-1. ?

   /*const float3 dither = texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.zw*2.718281828459).xyz;
   const float3 dither2 = texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.wz*3.14159265358979).xyz;
   return color + (dither - dither2) / quantSteps;*/

   //const float3 dither = texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.wz*3.141).xyz*2.0 - 1.0;

   // Lottes (1st one not working, 2nd 'quality', 3rd 'tradeoff'), IMHO too much magic:
   /*const float blackLimit = 0.5 * InvGamma(1.0/(quantSteps - 1.0));
   const float amount = 0.75 * (InvGamma(1.0/(quantSteps - 1.0)) - 1.0);
   return color + dither*min(color + blackLimit, amount);*/

   //const float3 amount = InvGamma(FBGamma(color) + (4. / quantSteps)) - color;
   
   //const float luma = saturate(dot(color,float3(0.212655,0.715158,0.072187)));
   //const float3 amount = lerp(
   //  InvGamma(4. / quantSteps), //!! precalc? would also solve 5/6/5bit issue!
   //  InvGamma((4. / quantSteps)+1.)-1.,
   //  luma);

   //return color + dither*amount;

   // RPDF:
   //return color + dither/quantSteps; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
#endif
}

float2 FBDither(const float2 color, /*const int2 pos*/const float2 tex0)
{
   return color; // on RG-only rendering do not dither anything for performance
}

float FBDither(const float color, /*const int2 pos*/const float2 tex0)
{
   return color; // on R-only rendering do not dither anything for performance
}

//

float3 FBColorGrade(float3 color)
{
   BRANCH if (!color_grade)
       return color;

   color.xy = color.xy*(15.0/16.0) + 1.0/32.0; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0;

   const float x = (color.x + floor(color.z))/16.0;
   const float3 lut1 = texNoLod(tex_color_lut, float2(x,          color.y)).xyz; // two lookups to blend/lerp between blue 2D regions
   const float3 lut2 = texNoLod(tex_color_lut, float2(x+1.0/16.0, color.y)).xyz;
   return lerp(lut1,lut2, frac(color.z));
}

#include "FXAAStereoAO.fxh"

#include "SSR.fxh"

//
//
//

VS_OUTPUT_2D vs_main_no_trafo (const in float4 vPosition  : POSITION0,
                               const in float2 tc         : TEXCOORD0)
{
   VS_OUTPUT_2D Out;
   Out.pos = float4(vPosition.xy, 0.0, 1.0);
   // DirectX has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
   Out.tex0 = tc + 0.5 * w_h_height.xy;
   return Out;
}

VS_OUTPUT_2D vs_main_no_trafo_subpixel(const in float4 vPosition : POSITION0, const in float2 tc : TEXCOORD0)
{
   VS_OUTPUT_2D Out;
   Out.pos = float4(vPosition.xy, 0.0, 1.0);
   // DirectX has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
   // Here we do not apply it to sample in between the samples for filtering
   Out.tex0 = tc;
   return Out;
}

//
// PS functions
//

// For reference, see https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/
float3 bloom_cutoff(const float3 c)
{
   const float Threshold = 2.5; //!! magic
   const float SoftThreshold = 1.0;
   const float Knee = Threshold * SoftThreshold;

   const float brightness = max(c.r, max(c.g, c.b));
   float soft = brightness - (Threshold - Knee);
   soft = clamp(soft, 0., 2. * Knee);
   soft *= soft * (1.0 / (4. * Knee + 0.00006103515625)); // 0.00006103515625 due to 16bit half float
   const float contribution = max(soft, brightness - Threshold) / max(brightness, 0.00006103515625); // dto.
   return c * contribution;
}

float4 ps_main_fb_bloom(const in VS_OUTPUT_2D IN) : COLOR
{
   // collect clipped contribution of the 4x4 texels (via box blur, NOT gaussian, as this is wrong) from original FB
   const float3 result = (texNoLod(tex_fb_filtered, IN.tex0 - w_h_height.xy).xyz
                       +  texNoLod(tex_fb_filtered, IN.tex0 + w_h_height.xy).xyz
                       +  texNoLod(tex_fb_filtered, IN.tex0 + float2(w_h_height.x,-w_h_height.y)).xyz
                       +  texNoLod(tex_fb_filtered, IN.tex0 + float2(-w_h_height.x,w_h_height.y)).xyz)*0.25; //!! offset for useAA?
   return float4(max(bloom_cutoff(ReinhardToneMap(result)), float3(0., 0., 0.)) * w_h_height.z, 1.0);
}

float4 ps_main_fb_AO(const in VS_OUTPUT_2D IN) : COLOR
{
   float result = texNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   result = FBGamma(saturate(result));
   return float4(result,result,result, 1.0);
}

float4 ps_main_fb_AO_static(const in VS_OUTPUT_2D IN) : COLOR
{
   // tex0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   const float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz // moving AO before tonemap does not really change the look
                       * texNoLod(tex_ao,          IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   return float4(result, 1.0);
}

float4 ps_main_fb_AO_no_filter_static(const in VS_OUTPUT_2D IN) : COLOR
{
   const float3 result = texNoLod(tex_fb_unfiltered, IN.tex0).rgb
                       * texNoLod(tex_ao,            IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   return float4(result, 1.0);
}

// ////////////////// Tonemapping /////////////////////////


float4 ps_main_fb_rhtonemap(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = ReinhardToneMap(result);
   return float4(FBColorGrade(FBGamma(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_fmtonemap(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(FilmicToneMap(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}

float4 ps_main_fb_nttonemap(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = PBRNeutralToneMapping(result);
   return float4(FBColorGrade(FBGamma22(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_agxtonemap(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(AgXToneMapping(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}



float4 ps_main_fb_rhtonemap_AO(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   // tex0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = ReinhardToneMap(result);
   return float4(FBColorGrade(FBGamma(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_fmtonemap_AO(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   // tex0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(FilmicToneMap(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}

float4 ps_main_fb_nttonemap_AO(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   // tex0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = PBRNeutralToneMapping(result);
   return float4(FBColorGrade(FBGamma22(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_agxtonemap_AO(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_filtered, IN.tex0).rgb;
   // tex0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(AgXToneMapping(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}



float4 ps_main_fb_rhtonemap_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = ReinhardToneMap(result);
   return float4(FBColorGrade(FBGamma(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_fmtonemap_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(FilmicToneMap(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}

float4 ps_main_fb_nttonemap_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = PBRNeutralToneMapping(result);
   return float4(FBColorGrade(FBGamma22(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_agxtonemap_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(AgXToneMapping(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}



float4 ps_main_fb_rhtonemap_AO_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = ReinhardToneMap(result);
   return float4(FBColorGrade(FBGamma(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_fmtonemap_AO_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(FilmicToneMap(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}

float4 ps_main_fb_nttonemap_AO_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = PBRNeutralToneMapping(result);
   return float4(FBColorGrade(FBGamma22(saturate(FBDither(result, IN.tex0)))), 1.0);
}

float4 ps_main_fb_agxtonemap_AO_no_filter(const in VS_OUTPUT_2D IN) : COLOR
{
   float3 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rgb;
   // moving AO before tonemap does not really change the look
   result *= texStereoNoLod(tex_ao, IN.tex0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rgb; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = saturate(FBDither(AgXToneMapping(result), IN.tex0));
   else
      result = FBGamma(saturate(FBDither(result, IN.tex0)));
   return float4(FBColorGrade(result), 1.0);
}



float4 ps_main_fb_rhtonemap_no_filterRG(const in VS_OUTPUT_2D IN) : COLOR
{
   float2 result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).rg;
   BRANCH
   if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).rg; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH
   if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = ReinhardToneMap(result);
   const float rg = /*FBColorGrade*/(FBGamma(saturate(dot(FBDither(result, IN.tex0), float2(0.176204 + 0.0108109 * 0.5, 0.812985 + 0.0108109 * 0.5)))));
   return float4(rg, rg, rg, 1.0);
}

float4 ps_main_fb_rhtonemap_no_filterR(const in VS_OUTPUT_2D IN) : COLOR
{
   float result = texStereoNoLod(tex_fb_unfiltered, IN.tex0).r;
   BRANCH
   if (do_bloom)
      result += texStereoNoLod(tex_bloom, IN.tex0).r; //!! offset?
   const float depth0 = texStereoNoLod(tex_depth, IN.tex0).x;
   BRANCH
   if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      result = ReinhardToneMap(result);
   const float gray = /*FBColorGrade*/(FBGamma(saturate(FBDither(result, IN.tex0))));
   return float4(gray, gray, gray, 1.0);
}



//
// Gaussian Blur Kernels
//
// Separable (approximate) Gaussian blur kernels, using GPU linear sampling to perform 2 samples in a single tex fetch (similar to https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/)
// Use e.g. https://observablehq.com/@s4l4x/efficient-gaussian-blur-with-linear-sampling to compute centered coefficients/offsets
// Use e.g. https://www.intel.com/content/www/us/en/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html to compue not centered coefficients/offsets

float4 ps_main_fb_blur_horiz5x5(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset5x5[2] = { 0.0, 1.3333333333333333 };
   const float weight5x5[2] = { 0.29411764705882354, 0.35294117647058826 };
   float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz*weight5x5[0];
   result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset5x5[1],0.0)).xyz
             +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset5x5[1],0.0)).xyz)*weight5x5[1];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert5x5(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset5x5[2] = { 0.0, 1.3333333333333333 };
   const float weight5x5[2] = { 0.29411764705882354, 0.35294117647058826 };
   float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz*weight5x5[0];
   result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset5x5[1])).xyz
             +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset5x5[1])).xyz)*weight5x5[1];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz7x7(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset7x7[2] = { 0.53473, 2.05896 }; //7 (no center!)
   const float weight7x7[2] = { 0.45134, 0.04866 }; //7 (no center!)
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 2; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset7x7[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset7x7[i],0.0)).xyz)*weight7x7[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert7x7(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset7x7[2] = { 0.53473, 2.05896 }; //7 (no center!)
   const float weight7x7[2] = { 0.45134, 0.04866 }; //7 (no center!)
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 2; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset7x7[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset7x7[i])).xyz)*weight7x7[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz9x9(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset9x9[3] = { 0.0, 1.3846153846153846, 3.2307692307692304 };
   const float weight9x9[3] = { 0.22697126013264554, 0.31613854089904203, 0.070253009088676 };
   float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz*weight9x9[0];
   UNROLL for(int i = 1; i < 3; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset9x9[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset9x9[i],0.0)).xyz)*weight9x9[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert9x9(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset9x9[3] = { 0.0, 1.3846153846153846, 3.2307692307692304 };
   const float weight9x9[3] = { 0.22697126013264554, 0.31613854089904203, 0.070253009088676 };
   float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz*weight9x9[0];
   UNROLL for(int i = 1; i < 3; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset9x9[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset9x9[i])).xyz)*weight9x9[i];
   return float4(result, 1.0);
}

//!! would be smoother fadeout on edges:
//const float offset11x11h[3] = { 0.59804, 2.18553, 4.06521 }; //no center!
//const float weight11x11h[3] = { 0.38173, 0.11538, 0.00289 }; //no center!

float4 ps_main_fb_blur_horiz11x11(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset11x11[3] = { 0.62195, 2.27357, 4.14706 }; //no center!
   const float weight11x11[3] = { 0.32993, 0.15722, 0.01285 }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 3; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset11x11[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset11x11[i],0.0)).xyz)*weight11x11[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert11x11(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset11x11[3] = { 0.62195, 2.27357, 4.14706 }; //no center!
   const float weight11x11[3] = { 0.32993, 0.15722, 0.01285 }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 3; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset11x11[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset11x11[i])).xyz)*weight11x11[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz13x13(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset13x13[4] = { 0.0, 1.4117647058823528, 3.2941176470588234, 5.176470588235294 }; //13
   const float weight13x13[4] = { 0.1964795505549364, 0.2969024319496817, 0.09446895562035326, 0.010381203914324535 }; //13
   float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz * weight13x13[0];
   UNROLL for(int i = 1; i < 4; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset13x13[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset13x13[i],0.0)).xyz)*weight13x13[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert13x13(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset13x13[4] = { 0.0, 1.4117647058823528, 3.2941176470588234, 5.176470588235294 }; //13
   const float weight13x13[4] = { 0.1964795505549364, 0.2969024319496817, 0.09446895562035326, 0.010381203914324535 }; //13
   float3 result = texNoLod(tex_fb_filtered, IN.tex0).xyz * weight13x13[0];
   UNROLL for(int i = 1; i < 4; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset13x13[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset13x13[i])).xyz)*weight13x13[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz15x15(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset15x15[4] = { 0.64417, 2.37795, 4.28970, 6.21493 }; //15 (no center!)
   const float weight15x15[4] = { 0.25044, 0.19233, 0.05095, 0.00628 }; //15 (no center!)
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 4; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset15x15[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset15x15[i],0.0)).xyz)*weight15x15[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert15x15(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset15x15[4] = { 0.64417, 2.37795, 4.28970, 6.21493 }; //15 (no center!)
   const float weight15x15[4] = { 0.25044, 0.19233, 0.05095, 0.00628 }; //15 (no center!)
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 4; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset15x15[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset15x15[i])).xyz)*weight15x15[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz19x19(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset19x19[5] = { 0.65323, 2.42572, 4.36847, 6.31470, 8.26547 }; //no center!
   const float weight19x19[5] = { 0.19923, 0.18937, 0.08396, 0.02337, 0.00408 }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 5; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset19x19[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset19x19[i],0.0)).xyz)*weight19x19[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert19x19(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset19x19[5] = { 0.65323, 2.42572, 4.36847, 6.31470, 8.26547 }; //no center!
   const float weight19x19[5] = { 0.19923, 0.18937, 0.08396, 0.02337, 0.00408 }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 5; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset19x19[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset19x19[i])).xyz)*weight19x19[i];
   return float4(result, 1.0);
}

#if 0
//!! would be smoother fadeout on edges:
//const float offset19x19h[5] = { 0.64625, 2.38872, 4.30686, 6.23559, 8.17666 };
//const float weight19x19h[5] = { 0.23996, 0.19335, 0.05753, 0.00853, 0.00063 };

//!! would be smoothestest fadeout on edges:
//const float offset19x19h[5] = { 0.63232, 2.31979, 4.20448, 6.12322, 8.07135 };
//const float weight19x19h[5] = { 0.29809, 0.17619, 0.02462, 0.00109, 0.00002 };

//!! would be smoothest fadeout on edges:
float4 ps_main_fb_blur_horiz19x19h(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset19x19h[5] = { 0.63918, 2.35282, 4.25124, 6.17117, 8.11278 }; //no center!
   const float weight19x19h[5] = { 0.27233, 0.18690, 0.03767, 0.00301, 0.00009 }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 5; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset19x19h[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset19x19h[i],0.0)).xyz)*weight19x19h[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert19x19h(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset19x19h[5] = { 0.63918, 2.35282, 4.25124, 6.17117, 8.11278 }; //no center!
   const float weight19x19h[5] = { 0.27233, 0.18690, 0.03767, 0.00301, 0.00009 }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 5; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset19x19h[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset19x19h[i])).xyz)*weight19x19h[i];
   return float4(result, 1.0);
}
#endif

float4 ps_main_fb_blur_horiz23x23(const in VS_OUTPUT_2D IN) : COLOR
{
   // smoother fadeout on edges
   const float offset23x23[6] = { //0.65769, 2.45001, 4.41069, 6.37247, 8.33578, 10.30098 //23 (no center!)
      0.64851, 2.40054, 4.32612, 6.25954, 8.20247, 10.15531
   };
   const float weight23x23[6] = { //0.16526, 0.17520, 0.10103, 0.04252, 0.01306, 0.00293  //23 (no center!)
      0.22791, 0.19358, 0.06543, 0.01187, 0.00115, 0.00006
   };
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 6; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset23x23[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset23x23[i],0.0)).xyz)*weight23x23[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert23x23(const in VS_OUTPUT_2D IN) : COLOR
{
   // smoother fadeout on edges
   const float offset23x23[6] = { //0.65769, 2.45001, 4.41069, 6.37247, 8.33578, 10.30098 //23 (no center!)
      0.64851, 2.40054, 4.32612, 6.25954, 8.20247, 10.15531
   };
   const float weight23x23[6] = { //0.16526, 0.17520, 0.10103, 0.04252, 0.01306, 0.00293  //23 (no center!)
      0.22791, 0.19358, 0.06543, 0.01187, 0.00115, 0.00006
   };
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 6; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset23x23[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset23x23[i])).xyz)*weight23x23[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz27x27(const in VS_OUTPUT_2D IN) : COLOR
{
   // 'h'-versions:  (smoother fadeout on edges)
   const float weight27x27[7] = { /*0.19592,0.18829,0.08573,0.02488,0.00460,0.00054,0.00004*/
      0.20227,0.19003,0.08192,0.02181,0.00358,0.00036,0.00002
   };
   const float offset27x27[7] = { /*0.65369,2.42819,4.37271,6.32039,8.27222,10.22886,12.19059*/
      0.65275,2.42313,4.36403,6.30877,8.25848,10.21386,12.17512
   };
   // org-version
   //const float offset27x27[7] = { 0.66025, 2.46412, 4.43566, 6.40762, 8.38017, 10.35347, 12.32765 }; //no center!
   //const float weight27x27[7] = { 0.14096, 0.15932, 0.10714, 0.05743, 0.02454, 0.00835, 0.00227 };   //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 7; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset27x27[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset27x27[i],0.0)).xyz)*weight27x27[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert27x27(const in VS_OUTPUT_2D IN) : COLOR
{
   // 'h'-versions:  (smoother fadeout on edges)
   const float weight27x27[7] = { /*0.19592,0.18829,0.08573,0.02488,0.00460,0.00054,0.00004*/
      0.20227,0.19003,0.08192,0.02181,0.00358,0.00036,0.00002
   };
   const float offset27x27[7] = { /*0.65369,2.42819,4.37271,6.32039,8.27222,10.22886,12.19059*/
      0.65275,2.42313,4.36403,6.30877,8.25848,10.21386,12.17512
   };
   // org-version
   //const float offset27x27[7] = { 0.66025, 2.46412, 4.43566, 6.40762, 8.38017, 10.35347, 12.32765 }; //no center!
   //const float weight27x27[7] = { 0.14096, 0.15932, 0.10714, 0.05743, 0.02454, 0.00835, 0.00227 };   //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 7; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset27x27[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset27x27[i])).xyz)*weight27x27[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_horiz39x39(const in VS_OUTPUT_2D IN) : COLOR
{
   // smoother fadeout on edges
   const float offset39x39[10] = { 0.66063, 2.46625, 4.43946, 6.41301, 8.38706, 10.36173, 12.33715, 14.31341, 16.29062, 18.26884
                                 /*0.66214, 2.47462, 4.45440, 6.43433, 8.41447, 10.39489, 12.37564, 14.35677, 16.33834, 18.32039*/
                                 /*0.66368, 2.48326, 4.46990, 6.45658, 8.44332, 10.43015, 12.41707, 14.40410, 16.39127, 18.37859*/ }; //no center!
   const float weight39x39[10] = { 0.13669, 0.15600, 0.10738, 0.05971, 0.02682, 0.00973, 0.00285, 0.00067, 0.00013, 0.00002
                                 /*0.11904, 0.14115, 0.10650, 0.06841, 0.03741, 0.01742, 0.00690, 0.00233, 0.00067, 0.00016*/
                                 /*0.09721, 0.11993, 0.09955, 0.07429, 0.04984, 0.03006, 0.01630, 0.00795, 0.00348, 0.00137*/ }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 10; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(w_h_height.x*offset39x39[i],0.0)).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(w_h_height.x*offset39x39[i],0.0)).xyz)*weight39x39[i];
   return float4(result, 1.0);
}

float4 ps_main_fb_blur_vert39x39(const in VS_OUTPUT_2D IN) : COLOR
{
   const float offset39x39[10] = { 0.66063, 2.46625, 4.43946, 6.41301, 8.38706, 10.36173, 12.33715, 14.31341, 16.29062, 18.26884
                                 /*0.66214, 2.47462, 4.45440, 6.43433, 8.41447, 10.39489, 12.37564, 14.35677, 16.33834, 18.32039*/
                                 /*0.66368, 2.48326, 4.46990, 6.45658, 8.44332, 10.43015, 12.41707, 14.40410, 16.39127, 18.37859*/ }; //no center!
   const float weight39x39[10] = { 0.13669, 0.15600, 0.10738, 0.05971, 0.02682, 0.00973, 0.00285, 0.00067, 0.00013, 0.00002
                                 /*0.11904, 0.14115, 0.10650, 0.06841, 0.03741, 0.01742, 0.00690, 0.00233, 0.00067, 0.00016*/
                                 /*0.09721, 0.11993, 0.09955, 0.07429, 0.04984, 0.03006, 0.01630, 0.00795, 0.00348, 0.00137*/ }; //no center!
   float3 result = float3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < 10; ++i)
      result += (texNoLod(tex_fb_filtered, IN.tex0+float2(0.0,w_h_height.y*offset39x39[i])).xyz
                +texNoLod(tex_fb_filtered, IN.tex0-float2(0.0,w_h_height.y*offset39x39[i])).xyz)*weight39x39[i];
   return float4(result, 1.0);
}


// mirror
float4 ps_main_fb_mirror(const in VS_OUTPUT_2D IN) : COLOR
{
   return float4(tex2D(tex_fb_unfiltered, IN.tex0).rgb * (1. / w_h_height.z), 1.0);
}


// upscale
float4 ps_main_fb_copy(const in VS_OUTPUT_2D IN) : COLOR
{
   return float4(tex2D(tex_fb_filtered, IN.tex0).rgb, 1.0);
}


// cos_hemisphere_sample & rotate_to_vector_upper are defined in FXAAStereoAO
float4 ps_main_fb_irradiance(const in VS_OUTPUT_2D IN) : COLOR
{
   // Compute sample direction
   const float phi   = PI + IN.tex0.x * (2.0 * PI);
   const float theta =      IN.tex0.y        * PI;
   const float3 N = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

   // Monte Carlo integration of hemispherical radiance -> irradiance
   const int NumSamples = 65536; // Limit to 32768 or 65536, will fail on macOS Metal otherwise
   const float InvNumSamples = 1.0 / float(NumSamples);
   const float g = 25962.0 / float(NumSamples); // 25962 = matching rank-1 generator constant for 65536 samples, 15936 for 32768 samples
   float3 irradiance = float3(0., 0., 0.);
   [unroll(1)] for (int i=0; i<NumSamples; ++i) {
      const float2 u = float2(float(i) * InvNumSamples, frac(float(i) * g));
      const float3 Li = rotate_to_vector_upper(cos_hemisphere_sample(u), N);
      const float2 uv = ray_to_equirectangular_uv(Li);
      irradiance += texNoLod(tex_fb_unfiltered, uv).rgb * InvNumSamples;
   }
   return float4(irradiance, 1.0);
}


//
// Techniques
//

/*technique normals // generate normals into 8bit RGB
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_normals();
   }
}*/

technique AO
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_ao();
   }
}

technique NFAA
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_nfaa();
	}
}

technique DLAA_edge
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_dlaa_edge();
	}
}

technique DLAA
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_dlaa();
	}
}

technique FXAA1
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fxaa1();
   }
}

technique FXAA2
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fxaa2();
   }
}

technique FXAA3
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fxaa3();
   }
}

technique fb_bloom
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_bloom();
   }
}

technique fb_AO { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader  = compile ps_3_0 ps_main_fb_AO(); }}
technique fb_AO_static { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_AO_static(); }}
technique fb_AO_no_filter_static { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_AO_no_filter_static(); }}

technique fb_rhtonemap { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_rhtonemap(); }}
technique fb_rhtonemap_AO { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_rhtonemap_AO(); }}
technique fb_rhtonemap_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_rhtonemap_no_filter(); }}
technique fb_rhtonemap_AO_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_rhtonemap_AO_no_filter(); }}

technique fb_fmtonemap { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_fmtonemap(); }}
technique fb_fmtonemap_AO { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_fmtonemap_AO(); }}
technique fb_fmtonemap_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_fmtonemap_no_filter(); }}
technique fb_fmtonemap_AO_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_fmtonemap_AO_no_filter(); }}

technique fb_nttonemap { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_nttonemap(); }}
technique fb_nttonemap_AO { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_nttonemap_AO(); }}
technique fb_nttonemap_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_nttonemap_no_filter(); }}
technique fb_nttonemap_AO_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_nttonemap_AO_no_filter(); }}

technique fb_agxtonemap { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_agxtonemap(); }}
technique fb_agxtonemap_AO { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo_subpixel(); PixelShader = compile ps_3_0 ps_main_fb_agxtonemap_AO(); }}
technique fb_agxtonemap_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_agxtonemap_no_filter(); }}
technique fb_agxtonemap_AO_no_filter { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_agxtonemap_AO_no_filter(); }}

technique fb_rhtonemap_no_filterRG { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_rhtonemap_no_filterRG(); }}
technique fb_rhtonemap_no_filterR { pass P0 { VertexShader = compile vs_3_0 vs_main_no_trafo(); PixelShader = compile ps_3_0 ps_main_fb_rhtonemap_no_filterR(); }}

// All Bloom variants:

technique fb_blur_horiz7x7
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz7x7();
   }
}

technique fb_blur_vert7x7
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_vert7x7();
   }
}

technique fb_blur_horiz9x9
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz9x9();
   }
}

technique fb_blur_vert9x9
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_vert9x9();
   }
}

technique fb_blur_horiz11x11
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz11x11();
   }
}

technique fb_blur_vert11x11
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_vert11x11();
   }
}

technique fb_blur_horiz13x13
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz13x13();
   }
}

technique fb_blur_vert13x13
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_vert13x13();
   }
}

technique fb_blur_horiz15x15
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz15x15();
   }
}

technique fb_blur_vert15x15
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_vert15x15();
   }
}

technique fb_blur_horiz19x19
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz19x19();
	}
}

technique fb_blur_vert19x19
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_vert19x19();
	}
}

#if 0
technique fb_blur_horiz19x19h
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz19x19h();
	}
}

technique fb_blur_vert19x19h
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_vert19x19h();
	}
}
#endif

technique fb_blur_horiz23x23
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz23x23();
   }
}

technique fb_blur_vert23x23
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader  = compile ps_3_0 ps_main_fb_blur_vert23x23();
   }
}

technique fb_blur_horiz27x27
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz27x27();
	}
}

technique fb_blur_vert27x27
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_vert27x27();
	}
}

technique fb_blur_horiz39x39
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_horiz39x39();
	}
}

technique fb_blur_vert39x39
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_blur_vert39x39();
	}
}

//

technique fb_mirror
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_mirror();
	}
}

//

technique fb_copy
{
    pass P0
    {
        VertexShader = compile vs_3_0 vs_main_no_trafo();
        PixelShader = compile ps_3_0 ps_main_fb_copy();
    }
}

//

technique fb_irradiance
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_irradiance();
	}
}

//

technique SSReflection
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_fb_ss_refl();
	}
}

//

technique CAS
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_CAS();
	}
}

//

technique BilateralSharp_CAS
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader  = compile ps_3_0 ps_main_BilateralSharp_CAS();
	}
}

//

#include "SMAA.hlsl"
