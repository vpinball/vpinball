#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

uniform vec4 color_grade; // converted to vec4 for BGFX
uniform vec4 do_dither; // converted to vec4 for BGFX
uniform vec4 do_bloom; // converted to vec4 for BGFX

uniform vec4 exposure; // converted to vec4 for BGFX


#ifdef GRAY
  #define rtype float
  #define swizzle r
#elif defined(RG)
  #define rtype vec2
  #define swizzle rg
#else
  #define rtype vec3
  #define swizzle rgb
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
SAMPLER2D      (tex_tonemap_lut,  6); // Precomputed Tonemapping LUT


// //////////////////////////////////////////////////////////////////////////////////////////////////
// Tonemapping

#define MAX_BURST 1000.0

#ifdef REINHARD
float ReinhardToneMap(float l)
{
	l *= exposure.x;
	
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    return min(l * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)), MAX_BURST); // overflow is handled by bloom
}
vec2 ReinhardToneMap(vec2 color)
{
	color *= exposure.x;
	
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, vec2(0.176204 + 0.0108109 * 0.5, 0.812985 + 0.0108109 * 0.5)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
vec3 ReinhardToneMap(vec3 color)
{
	color *= exposure.x;
	
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, vec3(0.176204, 0.812985, 0.0108109)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
#endif


#ifdef FILMIC
vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat = mtxFromRows3
(
    vec3(0.59719, 0.35458, 0.04823),
    vec3(0.07600, 0.90834, 0.01566),
    vec3(0.02840, 0.13383, 0.83777)
);
// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mtxFromRows3
(
    vec3( 1.60475, -0.53108, -0.07367),
    vec3(-0.10208,  1.10813, -0.00605),
    vec3(-0.00327, -0.07276,  1.07602)
);
vec3 ACESFitted(vec3 color)
{
    color = mul(ACESInputMat, color);
    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    color = mul(ACESOutputMat, color);
    return color;
}

// There are numerous filmic curve fitting implementation shared publicly
// I gathered a few here to be able to test and find the best result (also performance wise)
// Warning: The retrned value is already gamam corrected
vec3 FilmicToneMap(vec3 color)
{
	color *= exposure.x;
	
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    color = min(color, vec3(MAX_BURST, MAX_BURST, MAX_BURST));

    // Filmic Tonemapping prefitted curve from John Hable, including linear to sRGB (gamma)
    // http://filmicworlds.com/blog/filmic-tonemapping-operators/
    const vec3 x = max(vec3(0., 0., 0.), color - 0.004); // Filmic Curve
    color = (x * (6.2 * x + .5)) / (x * (6.2 * x + 1.7) + 0.06);

    // Filmic ACES fitted curve by Krzysztof Narkowicz (luminance only causing slightly oversaturate brights). Linear RGB to Linear RGB, with exposure included (1.0 -> 0.8).
    // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    /*color = 0.6 * color; // remove the included exposure using the value given in the blog post
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
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


#ifdef TONY
// Tony Mc MapFace Tonemapping (MIT licensed): see https://github.com/h3r2tic/tony-mc-mapface
// This tonemapping is similar to Reinhard but handles better highly saturated over powered colors
// (for these, it looks somewhat similar to AGX by desaturating them)
// It is more or less a Reinhard tonemapping followed by a color grade color correction
vec3 TonyMcMapfaceToneMap(vec3 color)
{
    const float LUT_DIMS = 48.0;

	color *= exposure.x;
	
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    color = min(color, vec3(MAX_BURST, MAX_BURST, MAX_BURST));

    // Apply a non-linear transform that the LUT is encoded with.
    vec3 encoded = color / (color + vec3(1.0, 1.0, 1.0));

    // Align the encoded range to texel centers.
    encoded.xy = encoded.xy * ((LUT_DIMS - 1.0) / LUT_DIMS) + 1.0 / (2.0 * LUT_DIMS);
    encoded.z *= (LUT_DIMS - 1.0);

    // We use a 2D texture so we need to do the linear filtering ourself.
    // This is fairly inefficient but needed until 3D textures are supported.
    const float y = (1.0 - encoded.y + floor(encoded.z)) / LUT_DIMS;
    const vec3 a = texNoLod(tex_tonemap_lut, vec2(encoded.x, y)).rgb;
    const vec3 b = texNoLod(tex_tonemap_lut, vec2(encoded.x, y + 1.0 / LUT_DIMS)).rgb;
    return mix(a, b, fract(encoded.z));
}
vec2 TonyMcMapfaceToneMap(vec2 color) { return color; } // Unimplemented
float TonyMcMapfaceToneMap(float color) { return color; } // Unimplemented
#endif


#ifdef NEUTRAL
vec3 PBRNeutralToneMapping(vec3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

	color *= exposure.x;
	
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
// AgX derived from the following references:
// - Blog post: https://iolite-engine.com/blog_posts/minimal_agx_implementation
// - Godot: https://github.com/godotengine/godot/pull/87260
// - ThreeJS: https://github.com/mrdoob/three.js/blob/master/src/renderers/shaders/ShaderChunk/tonemapping_pars_fragment.glsl.js
// - Filament: https://github.com/google/filament/blob/main/filament/src/ToneMapper.cpp
// All of these trying to match Blender's implementation and reference OCIO profile.
// TODO Add black/white point deifnition support when adding HDR output support
// TODO Add 'punchy' look transform (less desaturated)

// https://iolite-engine.com/blog_posts/minimal_agx_implementation
vec3 agxDefaultContrastApprox(vec3 x)
{
    #if 1
    // 6th order polynomial approximation (used in three.js and Godot)
    // Mean error^2: 3.6705141e-06
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return +15.5 * x4 * x2
		- 40.14 * x4 * x
		+ 31.96 * x4
		- 6.868 * x2 * x
		+ 0.4298 * x2
		+ 0.1191 * x
		- 0.00232;
    #else
    // 7th order polynomial approximation (used int Filament)
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    vec3 x6 = x4 * x2;
    return -17.86 * x6 * x
            + 78.01 * x6
            - 126.7 * x4 * x
            + 92.06 * x4
            - 28.72 * x2 * x
            + 4.361 * x2
            - 0.1718 * x
            + 0.002857;
    #endif
}

// AgX Tone Mapping implementation taken from three.js, based on Filament, 
// which in turn is based on Blender's implementation using rec 2020 primaries
// https://github.com/google/filament/pull/7236
// Inputs and outputs are encoded as Linear-sRGB.

vec3 AgXToneMapping(vec3 color)
{
    #if 0
    // AgX transform constants taken from Blender https://github.com/EaryChow/AgX_LUT_Gen/blob/main/AgXBaseRec2020.py
    // These operates on rec2020 value, therefore they require additional colorspace conversions
    // (for AgXOutsetMatrix, the inverse is precomputed. Note that input and output matrices are not mutual inverses)
    const mat3 AgXInsetMatrix = mtxFromRows3
    (
        vec3( 0.8566271533159830, 0.137318972929847, 0.1118982129999500),
        vec3( 0.0951212405381588, 0.761241990602591, 0.0767994186031903),
        vec3( 0.0482516061458583, 0.101439036467562, 0.8113023683968590)
	);
    const mat3 AgXOutsetMatrix = mtxFromRows3
    (
        vec3( 1.127100581814436800, -0.141329763498438300, -0.14132976349843826),
        vec3(-0.110606643096603230,  1.157823702216272000, -0.11060664309660294),
        vec3(-0.016493938717834573, -0.016493938717834257,  1.25193640659504050)
    );
    // Matrices for rec 2020 <> rec 709 color space conversion
    // matrix provided in row-major order so it has been transposed
    // https://www.itu.int/pub/R-REP-BT.2407-2017
    const mat3 LINEAR_REC2020_TO_LINEAR_SRGB = mtxFromRows3
    (
        vec3(1.6605, -0.1246, -0.0182),
        vec3(-0.5876, 1.1329, -0.1006),
        vec3(-0.0728, -0.0083, 1.1187)
    );
    const mat3 LINEAR_SRGB_TO_LINEAR_REC2020 = mtxFromRows3
    (
        vec3(0.6274, 0.0691, 0.0164),
        vec3(0.3293, 0.9195, 0.0880),
        vec3(0.0433, 0.0113, 0.8956)
    );

    #else
    // AgX transformation constants taken from https://iolite-engine.com/blog_posts/minimal_agx_implementation (also used in Godot)
    // It is supposed that they are ok for rec709 input values.
    // (note that out transform is the inverse of in transform)
    const mat3 AgXInsetMatrix = mtxFromRows3
    (
        vec3( 0.8424790622530940,  0.0423282422610123, 0.0423756549057051),
        vec3( 0.0784335999999992,  0.8784686364697720, 0.0784336000000000),
        vec3( 0.0792237451477643,  0.0791661274605434, 0.8791429737931040)
    );
    const mat3 AgXOutsetMatrix = mtxFromRows3
    (
        vec3( 1.1968790051201700, -0.0528968517574562, -0.0529716355144438),
        vec3(-0.0980208811401368,  1.1519031299041700, -0.0980434501171241),
        vec3(-0.0990297440797205, -0.0989611768448433,  1.1510736726411600)
    );
    #endif

	// LOG2_MIN      = -10.0
	// LOG2_MAX      =  +6.5
	// MIDDLE_GRAY   =  0.18
    const float AgxMinEv = -12.47393; // log2( pow( 2, LOG2_MIN ) * MIDDLE_GRAY )
    const float AgxMaxEv = 4.026069; // log2( pow( 2, LOG2_MAX ) * MIDDLE_GRAY )

	color *= exposure.x;
	
    #if 0
    color = mul(color, LINEAR_SRGB_TO_LINEAR_REC2020);
    #endif

    color = mul(color, AgXInsetMatrix);

	// Log2 encoding
    color = max(color, 1e-10); // avoid 0 or negative numbers for log2
    color = log2(color);
    color = (color - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    color = clamp(color, 0.0, 1.0);

	// Apply sigmoid
    color = agxDefaultContrastApprox(color);

	// TODO Apply AgX look
	// v = agxLook(v, look);

    color = mul(color, AgXOutsetMatrix);

    #if 0
    color = pow(max(vec3(0.0, 0.0, 0.0), color), vec3(2.2, 2.2, 2.2)); // rec2020 to linear rec2020
    color = mul(color, LINEAR_REC2020_TO_LINEAR_SRGB);                 // linear rec2020 to linear rec709 (sRGB)
    color = FBGamma(color);                                            // linear sRGB to sRGB
    #endif

    return color;
}
#endif

// //////////////////////////////////////////////////////////////////////////////////////////////////
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
   BRANCH if (!(do_dither.x > 0.))
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


// //////////////////////////////////////////////////////////////////////////////////////////////////
// Color grading

vec3 FBColorGrade(vec3 color)
{
   BRANCH if (!(color_grade.x > 0.))
       return color;

   color.xy = color.xy*(15.0/16.0) + 1.0/32.0; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0;

   const float x = (color.x + floor(color.z))/16.0;
   const vec3 lut1 = texNoLod(tex_color_lut, vec2(x,          color.y)).xyz; // two lookups to blend/lerp between blue 2D regions
   const vec3 lut2 = texNoLod(tex_color_lut, vec2(x+1.0/16.0, color.y)).xyz;
   return mix(lut1,lut2, fract(color.z));
}


// //////////////////////////////////////////////////////////////////////////////////////////////////
// Main fragment shader

void main()
{
   // v_texcoord0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)

   rtype result = texStereoNoLod(tex_fb, v_texcoord0).swizzle;

   // moving AO before tonemap does not really change the look
   #ifdef AO
      result *= texStereoNoLod(tex_ao, v_texcoord0 - 0.5*w_h_height.xy).x; // shift half a texel to blurs over 2x2 window
   #endif

   BRANCH if (do_bloom.x > 0.)
      result += texStereoNoLod(tex_bloom, v_texcoord0).swizzle; //!! offset?

   const float depth0 = texStereoNoLod(tex_depth, v_texcoord0).x;
   BRANCH if ((depth0 != 1.0) && (depth0 != 0.0)) //!! early out if depth too large (=BG) or too small (=DMD)
      #ifdef REINHARD
         result = ReinhardToneMap(result);
      #elif defined(TONY)
         result = TonyMcMapfaceToneMap(result);
      #elif defined(FILMIC)
         result = FilmicToneMap(result); else result = FBGamma(result);
      #elif defined(NEUTRAL)
         result = PBRNeutralToneMapping(result);
      #elif defined(AGX)
         result = AgXToneMapping(result); else result = FBGamma(result);
      #endif

   #ifdef GRAY
      #ifdef FILMIC
         result =         saturate(FBDither(result, v_texcoord0));
      #else
         result = FBGamma(saturate(FBDither(result, v_texcoord0)));
      #endif
      gl_FragColor = vec4(result, result, result, 1.0);
   
   #elif defined(RG)
      //float grey = dot(result, vec2(0.176204+0.0108109*0.5,0.812985+0.0108109*0.5));
      #ifdef FILMIC
         float grey =         saturate(dot(FBDither(result, v_texcoord0), vec2(0.176204+0.0108109*0.5,0.812985+0.0108109*0.5)));
      #else
         float grey = FBGamma(saturate(dot(FBDither(result, v_texcoord0), vec2(0.176204+0.0108109*0.5,0.812985+0.0108109*0.5))));
      #endif
      gl_FragColor = vec4(grey, grey, grey, 1.0);
   
   #else
      #if defined(FILMIC) || defined(AGX)
         result =         saturate(FBDither(result, v_texcoord0));
      #elif defined(NEUTRAL)
         result = FBGamma22(saturate(FBDither(result, v_texcoord0)));
      #else
         result = FBGamma(saturate(FBDither(result, v_texcoord0)));
      #endif
      gl_FragColor = vec4(FBColorGrade(result), 1.0);
   
   #endif
}
