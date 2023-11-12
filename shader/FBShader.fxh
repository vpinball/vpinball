#ifdef GLSL
uniform sampler2D tex_tonemap_lut; // Precomputed Tonemapping LUT

#else // HLSL

texture Texture6; // Precomputed Tonemapping LUT
sampler2D tex_tonemap_lut : TEXUNIT6 = sampler_state
{
    Texture = (Texture6);
    MIPFILTER = NONE;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU = Clamp;
    ADDRESSV = Clamp;
    SRGBTexture = false;
};

#endif

// //////////////////////////////////////////////////////////////////////////////////////////////////
// Tonemapping

#define MAX_BURST 1000.0

float ReinhardToneMap(const float l)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    return min(l * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)), MAX_BURST); // overflow is handled by bloom
}
float2 ReinhardToneMap(const float2 color)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, float2(0.176204 + 0.0108109 * 0.5, 0.812985 + 0.0108109 * 0.5)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
float3 ReinhardToneMap(const float3 color)
{
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
// Warning: The retrned value is already gamam corrected
float3 FilmicToneMap(float3 color)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    color = min(color, float3(MAX_BURST, MAX_BURST, MAX_BURST));

    // Filmic Tonemapping prefitted curve from John Hable, including linear to sRGB (gamma)
    // http://filmicworlds.com/blog/filmic-tonemapping-operators/
    const float3 x = max(float3(0., 0., 0.), color - 0.004); // Filmic Curve
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
    /*const float4 vh = float4(color, 11.2); // 11.2 is whitepoint
    const float4 va = 1.425*vh + 0.05;
    const float4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    color = vf.rgb/vf.aaa;
    color = FBGamma(color); */

    return color;
}

// Tony Mc MapFace Tonemapping (MIT licensed): see https://github.com/h3r2tic/tony-mc-mapface
// This tonemapping is similar to Reinhard but handles better highly saturated over powered colors
// (for these, it looks somewhat similar to AGX by desaturating them)
// It is more or less a Reinhard tonemapping followed by a color grade color correction
float3 TonyMcMapfaceToneMap(float3 color)
{
    const float LUT_DIMS = 48.0;

    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    color = min(color, float3(MAX_BURST, MAX_BURST, MAX_BURST));

    // Apply a non-linear transform that the LUT is encoded with.
    float3 encoded = color / (color + float3(1.0, 1.0, 1.0));

    // Align the encoded range to texel centers.
    encoded.xy = encoded.xy * ((LUT_DIMS - 1.0) / LUT_DIMS) + 1.0 / (2.0 * LUT_DIMS);
    encoded.z *= (LUT_DIMS - 1.0);

    // We use a 2D texture so we need to do the linear filtering ourself.
    // This is fairly inefficient but needed until 3D textures are supported.
    const float y = (1.0 - encoded.y + floor(encoded.z)) / LUT_DIMS;
    const float3 a = texNoLod(tex_tonemap_lut, float2(encoded.x, y)).rgb;
    const float3 b = texNoLod(tex_tonemap_lut, float2(encoded.x, y + 1.0 / LUT_DIMS)).rgb;
    return lerp(a, b, frac(encoded.z));
}
