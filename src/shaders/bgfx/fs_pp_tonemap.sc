// license:GPLv3+

$input v_texcoord0

#ifdef STEREO
$input v_eye
#endif

#include "common.sh"

// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

uniform vec4 do_color_grade; // converted to vec4 for BGFX
uniform vec4 do_dither; // converted to vec4 for BGFX
uniform vec4 do_bloom; // converted to vec4 for BGFX

uniform vec4 bloom_dither_colorgrade;
#define do_bloom       (bloom_dither_colorgrade.x == 1.)
#define do_dither      (bloom_dither_colorgrade.y == 1.)
#define do_color_grade (bloom_dither_colorgrade.z == 1.)

uniform vec4 exposure_wcg;
#define exposure                    (exposure_wcg.x)
#define sceneLum_x_invDisplayMaxLum (exposure_wcg.y)
#define displayMaxLum               (exposure_wcg.z)
#define isHDR2020                   (exposure_wcg.w == 1.)
#define isLinearFrameBuffer         (exposure_wcg.w == 2.)

// allenwp AgX tonemapping curve parameters, computed on the CPU: x = contrast, y = toe_a, z = slope, w = w
uniform vec4 agx_params;

// HDR->HDR spline based remapping
uniform vec4 spline1;
uniform vec2 spline2;
#define spline_src_pivot            (spline1.x)
#define spline_dst_pivot            (spline1.y)
#define spline_slope                (spline1.z)
#define spline_pa                   (spline1.w)
#define spline_qa                   (spline2.x)
#define spline_qb                   (spline2.y)

// Define which tonemapper outputs are in linear sRGB and which are in gamma compressed sRGB
#if defined(FILMIC) || defined(AGX)
#define TM_OUT_GAMMA
#else
#define TM_OUT_LINEAR
#endif

#ifdef NOFILTER
  #define tex_fb tex_fb_unfiltered
  SAMPLER2DSTEREO(tex_fb_unfiltered,  0); // Framebuffer
#else
  #define tex_fb tex_fb_filtered
  SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer
#endif

SAMPLER2DSTEREO(tex_bloom,        1); // Bloom
SAMPLER2D      (tex_color_lut,    2); // Color grade
SAMPLER2DSTEREO(tex_ao,           3); // Ambient Occlusion
SAMPLER2DSTEREO(tex_depth,        4); // Depth Buffer
SAMPLER2D      (tex_ao_dither,    5); // Ambient Occlusion Dither
//SAMPLER2D      (tex_tonemap_lut,  6); // Precomputed Tonemapping LUT


////////////////////////////////////////////////////////////////////////////////////////////////////
// Tonemapping

#define MAX_BURST 1000.0

#ifdef REINHARD
float ReinhardToneMap(float l)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    return min(l * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)), MAX_BURST); // overflow is handled by bloom
}
vec2 ReinhardToneMap(vec2 color)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, vec2(0.176204 + 0.0108109 * 0.5, 0.812985 + 0.0108109 * 0.5)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
vec3 ReinhardToneMap(vec3 color)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, vec3(0.176204, 0.812985, 0.0108109)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
#endif


#ifdef FILMIC
vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + vec3_splat(0.0245786)) - vec3_splat(0.000090537);
    vec3 b = v * (vec3_splat(0.983729) * v + vec3_splat(0.4329510)) + vec3_splat(0.238081);
    return a / b;
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat = mtxFromRows3
(
    vec3(0.59719, 0.07600, 0.02840),
    vec3(0.35458, 0.90834, 0.13383),
    vec3(0.04823, 0.01566, 0.83777)
);
// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mtxFromRows3
(
    vec3( 1.60475, -0.10208, -0.00327),
    vec3(-0.53108,  1.10813, -0.07276),
    vec3(-0.07367, -0.00605,  1.07602)
);
vec3 ACESFitted(vec3 color)
{
    color = mul(color, ACESInputMat);
    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    return mul(color, ACESOutputMat);
}

// There are numerous filmic curve fitting implementation shared publicly
// I gathered a few here to be able to test and find the best result (also performance wise)
// Warning: The returned value is already gamma corrected
vec3 FilmicToneMap(vec3 color)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    color = min(color, vec3(MAX_BURST, MAX_BURST, MAX_BURST));

    // Filmic Tonemapping prefitted curve from John Hable, including linear to sRGB (gamma)
    // http://filmicworlds.com/blog/filmic-tonemapping-operators/
    const vec3 x = max(vec3(0., 0., 0.), color - 0.004); // Filmic Curve
    color = (x * (6.2 * x + .5)) / (x * (6.2 * x + 1.7) + 0.06);

    // Filmic ACES fitted curve by Krzysztof Narkowicz (luminance only causing slightly oversaturate brights). Linear RGB to Linear RGB, with exposure included (1.0 -> 0.8).
    // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    /*color = 0.6 * color; // remove the included exposure using the value given in the blog post
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    color = (color*(a*color+b))/(color*(c*color+d)+e);
    color = FBGamma(color); */

    // Filmic ACES fitted curve by Stephen Hill. sRGB to sRGB (as stated in the code, still get surprising results, would need more tests)
    // https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
    /*color = FBGamma(color);
    color = ACESFitted(color);*/

    // Filmic ACES fitted curve by Jim Hejl 
    // https://twitter.com/jimhejl/status/633777619998130176
    /*const vec4 vh = vec4(color, 11.2); // 11.2 is whitepoint
    const vec4 va = 1.425*vh + 0.05;
    const vec4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    color = vf.rgb/vf.aaa;
    color = FBGamma(color); */

    return color;
}
vec2 FilmicToneMap(vec2 color) { return color; } // Unimplemented
float FilmicToneMap(float color) { return color; } // Unimplemented
#endif


#ifdef NEUTRAL
vec3 PBRNeutralToneMapping(vec3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    float x = min(color.x, min(color.y, color.z));
    float offset = x < 0.08 ? x - 6.25 * (x * x) : 0.04;
    color -= offset;

    const float peak = max(color.x, max(color.y, color.z));
    if (peak < startCompression) return color;

    const float d = 1. - startCompression;
    const float newPeak = 1. - (d * d) / (peak + (d - startCompression));

    const float inv_g = desaturation * (peak - newPeak) + 1.;
    const float w = newPeak / (inv_g*peak);
    const float n = newPeak - newPeak/inv_g;
    return n + color*w;
}
vec2 PBRNeutralToneMapping(vec2 color) { return color; } // Unimplemented
float PBRNeutralToneMapping(float color) { return color; } // Unimplemented
#endif


#ifdef AGX
// AgX Tone Mapping implementation, derived from Godot's implementation
// (see https://github.com/godotengine/godot/pull/106940), itself an approximation
// and simplification of EaryChow's AgX implementation used by Blender.
// It uses the allenwp tonemapping curve instead of Blender's log2 encoded sigmoid,
// closely matching it for dark-to-mid values while being cheaper, stable across
// variable dynamic range (SDR, HDR, EDR) and supporting adjustable white point and
// contrast, computed on the CPU and passed through the agx_params uniform.
// allenwp curve: https://allenwp.com/blog/2025/05/29/allenwp-tonemapping-curve/
// Colorspace transformations: https://www.colour-science.org:8010/apps/rgb_colourspace_transformation_matrix
// Inputs are linear sRGB (Rec. 709), output is sRGB encoded.
// TODO Add output_max_value support when adding HDR output support

// allenwp tonemapping curve; input must be a non-negative linear scene value.
vec3 allenwpCurve(vec3 x)
{
    const float output_max_value = 1.0; // SDR always has an output_max_value of 1.0

    // These constants must match the ones in the C++ code that calculates the parameters.
    // 18% "middle gray" is perceptually 50% of the brightness of reference white.
    const float awp_crossover_point = 0.18;
    // If output_max_value and/or awp_crossover_point are no longer constant,
    // awp_shoulder_max can be calculated on the CPU and passed in agx_params.
    const float awp_shoulder_max = output_max_value - awp_crossover_point;

    // Reinhard-like shoulder:
    vec3 s = x - vec3_splat(awp_crossover_point);
    const vec3 slope_s = agx_params.z * s;
    s = slope_s * (vec3_splat(1.0) + s / vec3_splat(agx_params.w)) / (vec3_splat(1.0) + slope_s / vec3_splat(awp_shoulder_max));
    s += vec3_splat(awp_crossover_point);

    // Sigmoid power function toe:
    vec3 t = pow(x, vec3_splat(agx_params.x));
    t = t / (t + vec3_splat(agx_params.y));

    return mix(t, s, step(vec3_splat(awp_crossover_point), x));
}

vec3 AgXToneMapping(vec3 color)
{
    // Combined Rec. 709 to Rec. 2020 conversion and Blender AgX inset matrix:
    const mat3 AgXInsetMatrix = mtxFromRows3
    (
        vec3(0.544814746488245, 0.140416948464053, 0.0888104196149096),
        vec3(0.373787398372697, 0.754137554567394, 0.178871756420858),
        vec3(0.0813978551390581, 0.105445496968552, 0.732317823964232)
    );
    // Combined inverse AgX outset matrix and Rec. 2020 to Rec. 709 conversion:
    const mat3 AgXOutsetMatrix = mtxFromRows3
    (
        vec3(1.96488741169489, -0.299313364904742, -0.164352742528393),
        vec3(-0.855988495690215, 1.32639796461980, -0.238183969428088),
        vec3(-0.108898916004672, -0.0270845997150571, 1.40253671195648)
    );

    // Clamping to non-negative values is required as negative values would result in
    // darker and more saturated colors after applying the inset matrix, and the
    // curve's pow would not evaluate correctly on them.
    color = max(color, vec3_splat(0.0));

    // Apply inset matrix.
    color = mul(color, AgXInsetMatrix);

    // Apply the allenwp tonemapping curve.
    color = allenwpCurve(color);

    // Clipping to output_max_value is required to address a cyan color shift that occurs with very bright inputs.
    color = min(color, vec3_splat(1.0));

    // Apply outset matrix (makes the result more chroma laden and goes back to Rec. 709).
    color = mul(color, AgXOutsetMatrix);

    // Convert to sRGB encoded output.
    color = FBGamma(color);

    return color;
}
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Dithering

/*const float bayer_dither_pattern[8][8] = float[][] (
    {( 0/64.0-0.5)/255.0, (32/64.0-0.5)/255.0, ( 8/64.0-0.5)/255.0, (40/64.0-0.5)/255.0, ( 2/64.0-0.5)/255.0, (34/64.0-0.5)/255.0, (10/64.0-0.5)/255.0, (42/64.0-0.5)/255.0},   
    {(48/64.0-0.5)/255.0, (16/64.0-0.5)/255.0, (56/64.0-0.5)/255.0, (24/64.0-0.5)/255.0, (50/64.0-0.5)/255.0, (18/64.0-0.5)/255.0, (58/64.0-0.5)/255.0, (26/64.0-0.5)/255.0},   
    {(12/64.0-0.5)/255.0, (44/64.0-0.5)/255.0, ( 4/64.0-0.5)/255.0, (36/64.0-0.5)/255.0, (14/64.0-0.5)/255.0, (46/64.0-0.5)/255.0, ( 6/64.0-0.5)/255.0, (38/64.0-0.5)/255.0},   
    {(60/64.0-0.5)/255.0, (28/64.0-0.5)/255.0, (52/64.0-0.5)/255.0, (20/64.0-0.5)/255.0, (62/64.0-0.5)/255.0, (30/64.0-0.5)/255.0, (54/64.0-0.5)/255.0, (22/64.0-0.5)/255.0},   
    {( 3/64.0-0.5)/255.0, (35/64.0-0.5)/255.0, (11/64.0-0.5)/255.0, (43/64.0-0.5)/255.0, ( 1/64.0-0.5)/255.0, (33/64.0-0.5)/255.0, ( 9/64.0-0.5)/255.0, (41/64.0-0.5)/255.0},   
    {(51/64.0-0.5)/255.0, (19/64.0-0.5)/255.0, (59/64.0-0.5)/255.0, (27/64.0-0.5)/255.0, (49/64.0-0.5)/255.0, (17/64.0-0.5)/255.0, (57/64.0-0.5)/255.0, (25/64.0-0.5)/255.0},
    {(15/64.0-0.5)/255.0, (47/64.0-0.5)/255.0, ( 7/64.0-0.5)/255.0, (39/64.0-0.5)/255.0, (13/64.0-0.5)/255.0, (45/64.0-0.5)/255.0, ( 5/64.0-0.5)/255.0, (37/64.0-0.5)/255.0},
    {(63/64.0-0.5)/255.0, (31/64.0-0.5)/255.0, (55/64.0-0.5)/255.0, (23/64.0-0.5)/255.0, (61/64.0-0.5)/255.0, (29/64.0-0.5)/255.0, (53/64.0-0.5)/255.0, (21/64.0-0.5)/255.0} );
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
vec3 triangularPDF(const vec3 r) // from -1..1, c=0 (with random no r=0..1)
{
   return vec3(triangularPDF(r.x), triangularPDF(r.y), triangularPDF(r.z));
}*/

/*float DitherGradientNoise(const vec2 tex0)
{
   // Interleaved Gradient Noise from "NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE" http://advances.realtimerendering.com/s2014/index.html
   const vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);

   return fract(magic.z * fract(dot(tex0, magic.xy)));
}

float Dither64(const vec2 tex0, const float time)
{
   const vec3 k0 = vec3(33., 52., 25.);

   return fract(dot(vec3(tex0, time), k0 * (1.0/64.0)));
}*/

vec3 blue_gauss_noise(const vec2 c1, const vec3 rgb)
{
   // https://www.shadertoy.com/view/XljyRR
   //vec2 c0 = vec2(c1.x - 1., c1.y);
   //vec2 c2 = vec2(c1.x + 1., c1.y);
   const vec3 cx = vec3(c1.x - 1., c1.x, c1.x + 1.);
   const vec4 f0 = fract(vec4(cx * 9.1031, c1.y * 8.1030));
   const vec4 f1 = fract(vec4(cx * 7.0973, c1.y * 6.0970));
   const vec4 t0 = vec4(f0.xw,f1.xw); //fract(c0.xyxy * vec4(.1031,.1030,.0973,.0970));
   const vec4 t1 = vec4(f0.yw,f1.yw); //fract(c1.xyxy * vec4(.1031,.1030,.0973,.0970));
   const vec4 t2 = vec4(f0.zw,f1.zw); //fract(c2.xyxy * vec4(.1031,.1030,.0973,.0970));
   const vec4 p0 = t0 + dot(t0, t0.wzxy + 19.19);
   const vec4 p1 = t1 + dot(t1, t1.wzxy + 19.19);
   const vec4 p2 = t2 + dot(t2, t2.wzxy + 19.19);
   const vec4 n0 = fract(p0.zywx * (p0.xxyz + p0.yzzw));
   const vec4 n1 = fract(p1.zywx * (p1.xxyz + p1.yzzw));
   const vec4 n2 = fract(p2.zywx * (p2.xxyz + p2.yzzw));
   const vec4 r4 = 0.5*n1 - 0.125*(n0 + n2);
   const float  r  = r4.x+r4.y+r4.z+r4.w - 0.5;

   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??
   return rgb + vec3(r,-r,r) * (1.0/(quantSteps - 1.0));
}

#if 0
// seems like everybody uses this, but the amount of flickering patterns generated is very noticeable
vec3 DitherVlachos(const vec2 tex0, const vec3 rgb)
{
   // Vlachos 2016, "Advanced VR Rendering", but using a TPDF
   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??

   const float noise = dot(vec2(171.0, 231.0), tex0);
   const vec3 noise3 = fract(noise * (1.0 / vec3(103.0, 71.0, 97.0)));

   return rgb + triangularPDF(noise3)/*(noise3*2.0-1.0)*/ * (1.0/(quantSteps - 1.0));
}
#endif

vec3 FBDither(const vec3 color, /*const int2 pos*/const vec2 tex0)
{
   BRANCH if (!(do_dither))
       return color;

   //return color + bayer_dither_pattern[pos.x%8][pos.y%8];

#ifndef BLUE_NOISE_DITHER
   //return DitherVlachos(floor(tex0*(1.0/w_h_height.xy)+0.5) + w_h_height.z/*w*/, color);       // note that w_h_height.w is the same nowadays
   return blue_gauss_noise(floor(tex0*(1.0/w_h_height.xy)+0.5) + 0.07*w_h_height.z/*w*/, color); // dto.
#else // needs tex_ao_dither
   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??

   // TPDF:
   const vec3 dither = /*vec3(GradientNoise(tex0 / w_h_height.xy + w_h_height.zw*3.141),
      GradientNoise(tex0.yx / w_h_height.yx + w_h_height.zw*1.618),
      GradientNoise(tex0 / w_h_height.xy + w_h_height.wz*2.718281828459));*/
   // Dither64(tex0 / w_h_height.xy+w_h_height.zw, 0.);
       texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.zw*3.141).xyz;
   return color + triangularPDF(dither) * (1.0/quantSteps); // quantSteps-1. ?

   /*const vec3 dither = texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.zw*2.718281828459).xyz;
   const vec3 dither2 = texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.wz*3.14159265358979).xyz;
   return color + (dither - dither2) / quantSteps;*/

   //const vec3 dither = texNoLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.wz*3.141).xyz*2.0 - 1.0;

   // Lottes (1st one not working, 2nd 'quality', 3rd 'tradeoff'), IMHO too much magic:
   /*const float blackLimit = 0.5 * InvGamma(1.0/(quantSteps - 1.0));
   const float amount = 0.75 * (InvGamma(1.0/(quantSteps - 1.0)) - 1.0);
   return color + dither*min(color + blackLimit, amount);*/

   //const vec3 amount = InvGamma(FBGamma(color) + (4. / quantSteps)) - color;

   //const float luma = saturate(dot(color,vec3(0.212655,0.715158,0.072187)));
   //const vec3 amount = mix(
   //  InvGamma(4. / quantSteps), //!! precalc? would also solve 5/6/5bit issue!
   //  InvGamma((4. / quantSteps)+1.)-1.,
   //  luma);

   //return color + dither*amount;

   // RPDF:
   //return color + dither/quantSteps; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
#endif
}

vec2 FBDither(const vec2 color, /*const int2 pos*/const vec2 tex0)
{
   return color; // on RG-only rendering do not dither anything for performance
}

float FBDither(const float color, /*const int2 pos*/const vec2 tex0)
{
   return color; // on R-only rendering do not dither anything for performance
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Color grading

vec3 FBColorGrade(vec3 color)
{
   BRANCH if (!(do_color_grade))
      return color;

   color.xy = color.xy*(15.0/16.0) + 1.0/32.0; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0;

   const float x = (color.x + floor(color.z))/16.0;
   const vec3 lut1 = texNoLod(tex_color_lut, vec2(x,          color.y)).xyz; // two lookups to blend/lerp between blue 2D regions
   const vec3 lut2 = texNoLod(tex_color_lut, vec2(x+1.0/16.0, color.y)).xyz;
   return mix(lut1,lut2, fract(color.z));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main fragment shader

void main()
{
   // v_texcoord0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)

   vec3 result = texStereoNoLod(tex_fb, v_texcoord0).rgb;

   // moving AO before tonemap does not really change the look
   #ifdef AO
      result *= texStereoNoLod(tex_ao, v_texcoord0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   #endif

   BRANCH if (do_bloom)
      result += texStereoNoLod(tex_bloom, v_texcoord0).rgb; //!! offset?

   result *= exposure;

   if (isHDR2020) // scale by 1/hdr_headroom (so everything that is within the displays range is now mapped to 0..1)
      result *= sceneLum_x_invDisplayMaxLum; // scale by scene luminance to get nits, then divide by display max luminance (in nits) to get a display-matching 0..1 range before tonemapping

   const float depth0 = texStereoNoLod(tex_depth, v_texcoord0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
   {
      #ifdef REINHARD
         result = ReinhardToneMap(result);       // linear sRGB -> linear sRGB
      #elif defined(FILMIC)
         result = FilmicToneMap(result);         // linear sRGB -> sRGB
      #elif defined(NEUTRAL)
         result = PBRNeutralToneMapping(result); // linear sRGB -> linear sRGB
      #elif defined(AGX)
         result = AgXToneMapping(result);        // linear sRGB -> sRGB
      /*#elif defined(BT2446)                    // linear sRGB -> linear sRGB
         // NOTE: the following BT2446 conversion (see method A in the spec) was not designed for target displays exceeding ~1000 nits (actually it defines source HDR at 1000 nits -> and the target at only 100 nits (=SDR)).
         //       In practice, its still pretty stable though over the full output range from 100 to 10000 nits it seems.
         //       E.g. videolan used a tweaked version of this (IMHO worse) for a downscaling range of 2..10x

         // bring back into original range (so NOT scaled to display range)
         if (isHDR2020)
            result /= sceneLum_x_invDisplayMaxLum;

         const float master_max = max(1000.,displayMaxLum * 10000.); //!! or: first number: rather 10000.? but 1000 fits better to the BT2446 spec

         // precompute constants according to spec, note that spec does use nits
         // note that we adapt the max of 1000 nits as source from the spec, if the target display supports more than that!
         const float input_max = master_max / (sceneLum_x_invDisplayMaxLum * displayMaxLum * 10000.);
         const float phdr = 1. + 32. * pow(master_max / 10000., 1./2.4);
         const float psdr = 1. + 32. * pow(displayMaxLum, 1./2.4);

         #ifdef APPROX_BT2446
         const float y_old = dot(result,vec3(0.2627,0.6780,0.0593));
         float y = pow(y_old/input_max, 1./2.4);
         #else // BT2446 actually does the ^1./2.4 per component
         const vec3 rgb_old = pow(result/input_max, 1./2.4);
         const float y_old = dot(rgb_old,vec3(0.2627,0.6780,0.0593));
         float y = y_old;
         #endif

         if(y_old >= FLT_MIN_VALUE)
         {
            y = log(1. + (phdr - 1.) * y) * (1. / log(phdr));

            if (y <= 0.7399)
               y *= 1.0770;
            else if (y < 0.9909)
               y = (-1.1510*y + 2.7811)*y - 0.6302;
            else
               y = 0.5*y + 0.5;

            y = (pow(psdr, y) - 1.) * (1. / (psdr - 1.));

            #ifdef APPROX_BT2446 // use BT1886 EOTF and scale instead of proper color correction as in the BT2446 spec
            const float lw = pow(1./sceneLum_x_invDisplayMaxLum, 1./2.4);
            y = pow(lw * y, 2.4);
            result *= y/y_old;
            #else
            const float fy = y/(1.1*y_old);
            const float cb = fy*(rgb_old.b-y_old);
            const float cr = fy*(rgb_old.r-y_old);
            const float ytmo = y - max(0.1/1.4746*cr, 0.);
            result.r = ytmo + cr;
            result.g = ytmo -0.16455312684366/1.8814*cb -0.57135312684366/1.4746*cr;
            result.b = ytmo + cb;
            result = pow(result, 2.4);
            #endif
         }*/
      #elif defined(WCG) // linear sRGB -> linear sRGB
         // UHD-Guidelines spec:
         // "Note that some displays ignore some or all static metadata (i.e., ST 2086, MaxFALL, and
         //  MaxCLL); however, HDR10 distribution systems must deliver the static metadata, when present."
         // -> thus, do own tonemapping (also as BGFX sets the static metadata (state: 11/2024) just to the displays capacities)

         // bring back into original range (so NOT scaled to display range)
         result /= sceneLum_x_invDisplayMaxLum;

         const float y_old = dot(result,vec3(0.2627,0.6780,0.0593));
         if(y_old >= FLT_MIN_VALUE)
         {
            float y = y_old;

            // simple spline based curve from videolan, which ends up almost linear in practice with the parameters we use, up to being fully linear on ~1000+ nits displays
            y -= spline_src_pivot;
            y = y > 0. ? ((spline_qa * y + spline_qb) * y + spline_slope) * y : (spline_pa * y + spline_slope) * y;
            y += spline_dst_pivot;

            result *= y/y_old; //!! very minimalistic simplistic "brightness" scale
         }

         // do not scale by headroom twice, so divide by it (see col *= displayMaxLum;)
         result *= sceneLum_x_invDisplayMaxLum;
      #endif
   }
   #ifdef TM_OUT_GAMMA
   else result = FBGamma(result);
   #endif

   if (!isHDR2020)
   {
      if (isLinearFrameBuffer)
      {
          #ifdef TM_OUT_GAMMA
             result = InvGamma(FBColorGrade(saturate(FBDither(result, v_texcoord0))));
          #else
             result = saturate(FBDither(result, v_texcoord0));
             BRANCH if (do_color_grade)
                result = InvGamma(FBColorGrade(FBGamma(result)));
          #endif
          gl_FragColor = vec4(result, 1.0);
      }
      else
      {
          #ifdef TM_OUT_GAMMA
             result =           saturate(FBDither(result, v_texcoord0));
          #elif defined(NEUTRAL)
             result = FBGamma22(saturate(FBDither(result, v_texcoord0)));
          #else
             result = FBGamma(  saturate(FBDither(result, v_texcoord0)));
          #endif
          gl_FragColor = vec4(FBColorGrade(result), 1.0);
      }
   }

   // WCG HDR support for DXGI, based on https://learn.microsoft.com/en-us/windows/win32/direct3darticles/high-dynamic-range
   else
   {
      vec3 col = result;
      BRANCH if (do_color_grade)
      {
         #ifdef TM_OUT_LINEAR
         col = FBGamma(col);
         #endif
         
         // Perform color grading by using the tonemapped value rescaled to initial luminance range
         const vec3 satCol = clamp(col / sceneLum_x_invDisplayMaxLum, vec3_splat(0.0001), vec3_splat(1.0)); // 0 would lead to NaNs after division
         col *= FBColorGrade(satCol) / satCol;

         #ifdef TM_OUT_LINEAR
         col = InvGamma(col);
         #endif
      }

      #ifdef TM_OUT_GAMMA
      // sRGB to linear sRGB
      col = InvGamma(col);
      #endif

      // After tonemapping, color should still be in the range 0..1 where 1 stands for display's maximum luminance, otherwise force it to be
      #ifndef WCG
      col = saturate(col);
      #endif

      // Apply display max luminance to scale the 0..1 tonemapped range to the maximum nits supported
      col *= displayMaxLum;

      // Linear sRGB to REC2020
      const mat3 LINEAR_SRGB_TO_LINEAR_REC2020 = mtxFromRows3
      (
         vec3(0.6274, 0.0691, 0.0164),
         vec3(0.3293, 0.9195, 0.0880),
         vec3(0.0433, 0.0113, 0.8956)
      );
      col = mul(col, LINEAR_SRGB_TO_LINEAR_REC2020);

      // Apply REC20 PQ
      // HDR10 ( 497,  497,  497) encodes exactly D65 white at 80 nits luminance
      // HDR10 (1023, 1023, 1023) encodes exactly D65 white at 10000 nits luminance
      // Linear -> ST2084
      const float m1 = (2610. / 4096.) / 4.;
      const float m2 = (2523. / 4096.) * 128.;
      const float c1 =  3424. / 4096.;
      const float c2 = (2413. / 4096.) * 32.;
      const float c3 = (2392. / 4096.) * 32.;
      const vec3 cp = pow(col, vec3_splat(m1));
      col = pow((c1 + c2 * cp) / (1.0 + c3 * cp), vec3_splat(m2));

      gl_FragColor = vec4(col, 1.0);
   }
}
