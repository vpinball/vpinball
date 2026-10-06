// license:GPLv3+

#ifdef GLSL
uniform float4 exposure_wcg; // overall scene exposure

// allenwp AgX tonemapping curve parameters, computed on the CPU: x = contrast, y = toe_a, z = slope, w = w
uniform float4 agx_params;

// HDR->HDR spline based remapping // only BGFX
uniform float4 spline1;
uniform float2 spline2;

#else // HLSL

const float4 exposure_wcg; // overall scene exposure

// allenwp AgX tonemapping curve parameters, computed on the CPU: x = contrast, y = toe_a, z = slope, w = w
const float4 agx_params;

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


// AgX Tone Mapping implementation, derived from Godot's implementation
// (see https://github.com/godotengine/godot/pull/106940), itself an approximation
// and simplification of EaryChow's AgX implementation used by Blender.
// It uses the allenwp tonemapping curve instead of Blender's log2 encoded sigmoid,
// closely matching it for dark-to-mid values while being cheaper, stable across
// variable dynamic range (SDR, HDR, EDR) and supporting adjustable white point and
// contrast, computed on the CPU and passed through the agx_params uniform.
// allenwp curve: https://allenwp.com/blog/2025/05/29/allenwp-tonemapping-curve/
// Colorspace transformations: https://www.colour-science.org:8010/apps/rgb_colourspace_transformation_matrix
// Input is expected in 'linear sRGB' (direct output of VPX rendering), output is in sRGB (non linear)
// TODO Add output_max_value support when adding HDR output support

// allenwp tonemapping curve; input must be a non-negative linear scene value.
float3 allenwpCurve(float3 x)
{
    const float output_max_value = 1.0; // SDR always has an output_max_value of 1.0

    // These constants must match the ones in the C++ code that calculates the parameters.
    // 18% "middle gray" is perceptually 50% of the brightness of reference white.
    const float awp_crossover_point = 0.18;
    // If output_max_value and/or awp_crossover_point are no longer constant,
    // awp_shoulder_max can be calculated on the CPU and passed in agx_params.
    const float awp_shoulder_max = output_max_value - awp_crossover_point;

    // Reinhard-like shoulder:
    float3 s = x - awp_crossover_point;
    const float3 slope_s = agx_params.z * s;
    s = slope_s * (1.0 + s / agx_params.w) / (1.0 + slope_s / awp_shoulder_max);
    s += awp_crossover_point;

    // Sigmoid power function toe:
    float3 t = pow(x, agx_params.x);
    t = t / (t + agx_params.y);

    return lerp(t, s, step(awp_crossover_point, x));
}

float3 AgXToneMapping(float3 color)
{
    // Combined Rec. 709 to Rec. 2020 conversion and Blender AgX inset matrix:
    const float3x3 AgXInsetMatrix =
    MAT3_BEGIN
        MAT_ROW3_BEGIN 0.544814746488245, 0.140416948464053, 0.0888104196149096 MAT_ROW_END,
        MAT_ROW3_BEGIN 0.373787398372697, 0.754137554567394, 0.178871756420858 MAT_ROW_END,
        MAT_ROW3_BEGIN 0.0813978551390581, 0.105445496968552, 0.732317823964232 MAT_ROW_END
    MAT_END;
    // Combined inverse AgX outset matrix and Rec. 2020 to Rec. 709 conversion:
    const float3x3 AgXOutsetMatrix =
    MAT3_BEGIN
        MAT_ROW3_BEGIN  1.96488741169489, -0.299313364904742, -0.164352742528393 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.855988495690215,  1.32639796461980, -0.238183969428088 MAT_ROW_END,
        MAT_ROW3_BEGIN -0.108898916004672, -0.0270845997150571,  1.40253671195648 MAT_ROW_END
    MAT_END;

    color *= exposure;

    // Clamping to non-negative values is required as negative values would result in
    // darker and more saturated colors after applying the inset matrix, and the
    // curve's pow would not evaluate correctly on them.
    color = max(color, float3(0.0, 0.0, 0.0));

    // Apply inset matrix.
    color = mul(color, AgXInsetMatrix);

    // Apply the allenwp tonemapping curve.
    color = allenwpCurve(color);

    // Clipping to output_max_value is required to address a cyan color shift that occurs with very bright inputs.
    color = min(color, 1.0);

    // Apply outset matrix (makes the result more chroma laden and goes back to Rec. 709).
    color = mul(color, AgXOutsetMatrix);

    // Convert to sRGB encoded output.
    color = FBGamma(color);

    return color;
}
