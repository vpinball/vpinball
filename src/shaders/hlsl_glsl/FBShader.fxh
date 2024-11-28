// license:GPLv3+

#ifdef GLSL
uniform float4 exposure_wcg; // overall scene exposure

// HDR->HDR spline based remapping // only BGFX
uniform float4 spline1;
uniform float2 spline2;

#else // HLSL

const float4 exposure_wcg; // overall scene exposure

// HDR->HDR spline based remapping // only BGFX
const float4 spline1;
const float2 spline2;

#endif

#define exposure (exposure_wcg.x)

// //////////////////////////////////////////////////////////////////////////////////////////////////
// Tonemapping

#define MAX_BURST 1000.0

float ReinhardToneMap(float l)
{
    l *= exposure;

    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    return min(l * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)), MAX_BURST); // overflow is handled by bloom
}
float2 ReinhardToneMap(float2 color)
{
    color *= exposure;

    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, float2(0.176204 + 0.0108109 * 0.5, 0.812985 + 0.0108109 * 0.5)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
float3 ReinhardToneMap(float3 color)
{
    color *= exposure;

    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, float3(0.176204, 0.812985, 0.0108109)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}
#ifdef GLSL
// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
);
// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602
);
vec3 ACESFitted(vec3 color)
{
    color = color * ACESInputMat;
    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    color = color * ACESOutputMat;
    return color;
}
#else
// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};
// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};
float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);
    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    color = mul(ACESOutputMat, color);
    return color;
}
#endif

// There are numerous filmic curve fitting implementation shared publicly
// I gathered a few here to be able to test and find the best result (also performance wise)
// Warning: The returned value is already gamma corrected
float3 FilmicToneMap(float3 color)
{
    color *= exposure;

    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    color = min(color, float3(MAX_BURST, MAX_BURST, MAX_BURST));

    // Filmic Tonemapping prefitted curve from John Hable, including linear to sRGB (gamma)
    // http://filmicworlds.com/blog/filmic-tonemapping-operators/
    const float3 x = max(float3(0., 0., 0.), color - 0.004); // Filmic Curve
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
    /*const float4 vh = float4(color, 11.2); // 11.2 is whitepoint
    const float4 va = 1.425*vh + 0.05;
    const float4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    color = vf.rgb/vf.aaa;
    color = FBGamma(color); */

    return color;
}

float3 PBRNeutralToneMapping(float3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    color *= exposure;

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


// AgX derived from the following references:
// - Blog post: https://iolite-engine.com/blog_posts/minimal_agx_implementation
// - Godot: https://github.com/godotengine/godot/pull/87260
// - ThreeJS: https://github.com/mrdoob/three.js/blob/master/src/renderers/shaders/ShaderChunk/tonemapping_pars_fragment.glsl.js
// - Filament: https://github.com/google/filament/blob/main/filament/src/ToneMapper.cpp
// All of these trying to match Blender's implementation and reference OCIO profile.
// TODO Add black/white point definition support when adding HDR output support
// TODO Add 'punchy' look transform (less desaturated)

// https://iolite-engine.com/blog_posts/minimal_agx_implementation
float3 agxDefaultContrastApprox(float3 x)
{
    #if 1
    // 6th order polynomial approximation (used in three.js and Godot)
    // Mean error^2: 3.6705141e-06
    float3 x2 = x * x;
    float3 x4 = x2 * x2;
    return  + 15.5 * x4 * x2
            - 40.14 * x4 * x
            + 31.96 * x4
            - 6.868 * x2 * x
            + 0.4298 * x2
            + 0.1191 * x
            - 0.00232;
    #else
    // 7th order polynomial approximation (used in Filament)
    float3 x2 = x * x;
    float3 x4 = x2 * x2;
    float3 x6 = x4 * x2;
    return  - 17.86 * x6 * x
            + 78.01 * x6
            - 126.7 * x4 * x
            + 92.06 * x4
            - 28.72 * x2 * x
            + 4.361 * x2
            - 0.1718 * x
            + 0.002857;
    #endif
}

// AgX Tone Mapping implementation
// Input is expected in 'linear sRGB' (direct output of VPX rendering), output is in sRGB (non linear)
float3 AgXToneMapping(float3 color)
{
    #if 0
    // AgX transform constants taken from Blender https://github.com/EaryChow/AgX_LUT_Gen/blob/main/AgXBaseRec2020.py
    // These operates on rec2020 value, therefore they require additional colorspace conversions
    // (for AgXOutsetMatrix, the inverse is precomputed. Note that input and output matrices are not mutual inverses)
    const float3x3 AgXInsetMatrix =
    MAT3_BEGIN
        MAT_ROW3_BEGIN 0.8566271533159830, 0.137318972929847, 0.1118982129999500 MAT_ROW_END,
        MAT_ROW3_BEGIN 0.0951212405381588, 0.761241990602591, 0.0767994186031903 MAT_ROW_END,
        MAT_ROW3_BEGIN 0.0482516061458583, 0.101439036467562, 0.8113023683968590 MAT_ROW_END
    MAT_END;
    const float3x3 AgXOutsetMatrix =
    MAT3_BEGIN
        MAT_ROW3_BEGIN  1.127100581814436800, -0.141329763498438300, -0.14132976349843826 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.110606643096603230,  1.157823702216272000, -0.11060664309660294 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.016493938717834573, -0.016493938717834257,  1.25193640659504050 MAT_ROW_END
    MAT_END;
    // Matrices for rec 2020 <> rec 709 color space conversion
    // matrix provided in row-major order so it has been transposed
    // https://www.itu.int/pub/R-REP-BT.2407-2017
    const float3x3 LINEAR_REC2020_TO_LINEAR_SRGB =
    MAT3_BEGIN
        MAT_ROW3_BEGIN 1.6605, -0.1246, -0.0182 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.5876, 1.1329, -0.1006 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.0728, -0.0083, 1.1187 MAT_ROW_END
    MAT_END;
    const float3x3 LINEAR_SRGB_TO_LINEAR_REC2020 =
    MAT3_BEGIN
        MAT_ROW3_BEGIN 0.6274, 0.0691, 0.0164 MAT_ROW_END,
        MAT_ROW3_BEGIN 0.3293, 0.9195, 0.0880 MAT_ROW_END,
        MAT_ROW3_BEGIN 0.0433, 0.0113, 0.8956 MAT_ROW_END
    MAT_END;

    #else

    // AgX transformation constants taken from https://iolite-engine.com/blog_posts/minimal_agx_implementation (also used in Godot)
    // It is supposed that they are ok for rec709 input values.
    // (note that out transform is the inverse of in transform)
    const float3x3 AgXInsetMatrix =
    MAT3_BEGIN
        MAT_ROW3_BEGIN  0.8424790622530940,  0.0423282422610123,  0.0423756549057051 MAT_ROW_END,
        MAT_ROW3_BEGIN  0.0784335999999992,  0.8784686364697720,  0.0784336000000000 MAT_ROW_END,
        MAT_ROW3_BEGIN  0.0792237451477643,  0.0791661274605434,  0.8791429737931040 MAT_ROW_END
    MAT_END;
    const float3x3 AgXOutsetMatrix =
    MAT3_BEGIN
        MAT_ROW3_BEGIN  1.1968790051201700, -0.0528968517574562, -0.0529716355144438 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.0980208811401368,  1.1519031299041700, -0.0980434501171241 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.0990297440797205, -0.0989611768448433,  1.1510736726411600 MAT_ROW_END
    MAT_END;
    #endif

    // LOG2_MIN      = -10.0
    // LOG2_MAX      =  +6.5
    // MIDDLE_GRAY   =  0.18
    const float AgxMinEv = -12.47393; // log2( pow( 2, LOG2_MIN ) * MIDDLE_GRAY )
    const float AgxMaxEv =  4.026069; // log2( pow( 2, LOG2_MAX ) * MIDDLE_GRAY )

    color *= exposure;

    #if 0
    color = mul(color, LINEAR_SRGB_TO_LINEAR_REC2020); // linear sRGB (rec709) to linear rec2020 expected by Blender/threeejs matrices
    #endif

    color = mul(color, AgXInsetMatrix);

    // Log2 encoding
    color = max(color, FLT_MIN_VALUE); // avoid 0 or negative numbers for log2
    color = log2(color);
    color = (color - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    color = clamp(color, 0.0, 1.0);

    // Apply sigmoid
    color = agxDefaultContrastApprox(color);

    // TODO Apply AgX look
    // v = agxLook(v, look);

    color = mul(color, AgXOutsetMatrix);

    #if 0
    color = pow(max(float3(0.0, 0.0, 0.0), color), float3(2.2, 2.2, 2.2)); // rec2020 to linear rec2020
    color = mul(color, LINEAR_REC2020_TO_LINEAR_SRGB);                     // linear rec2020 to linear rec709 (sRGB)
    color = FBGamma(color);                                                // linear sRGB to sRGB
    #endif

    return color;
}
