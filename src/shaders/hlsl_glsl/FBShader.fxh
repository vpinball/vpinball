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
// Warning: The returned value is already gamma corrected
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

float3 PBRNeutralToneMapping(float3 color)
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

#ifdef GLSL
// Matrices for rec 2020 <> rec 709 color space conversion
// matrix provided in row-major order so it has been transposed
// https://www.itu.int/pub/R-REP-BT.2407-2017
const mat3 LINEAR_REC2020_TO_LINEAR_SRGB = mat3(
	vec3(1.6605, -0.1246, -0.0182),
	vec3(-0.5876, 1.1329, -0.1006),
	vec3(-0.0728, -0.0083, 1.1187)
);

const mat3 LINEAR_SRGB_TO_LINEAR_REC2020 = mat3(
	vec3(0.6274, 0.0691, 0.0164),
	vec3(0.3293, 0.9195, 0.0880),
	vec3(0.0433, 0.0113, 0.8956)
);
#else
static const float3x3 LINEAR_REC2020_TO_LINEAR_SRGB =
{
    { 1.6605, -0.1246, -0.0182 },
    { -0.5876, 1.1329, -0.1006 },
    { -0.0728, -0.0083, 1.1187 }
};
static const float3x3 LINEAR_SRGB_TO_LINEAR_REC2020 =
{
    { 0.6274, 0.0691, 0.0164 },
    { 0.3293, 0.9195, 0.0880 },
    { 0.0433, 0.0113, 0.8956 }
};
#endif

// https://iolite-engine.com/blog_posts/minimal_agx_implementation
// Mean error^2: 3.6705141e-06
float3 agxDefaultContrastApprox(float3 x)
{

    float3 x2 = x * x;
    float3 x4 = x2 * x2;

    return +15.5 * x4 * x2
		- 40.14 * x4 * x
		+ 31.96 * x4
		- 6.868 * x2 * x
		+ 0.4298 * x2
		+ 0.1191 * x
		- 0.00232;

}

// AgX Tone Mapping implementation taken from three.js, based on Filament, 
// which in turn is based on Blender's implementation using rec 2020 primaries
// https://github.com/google/filament/pull/7236
// Inputs and outputs are encoded as Linear-sRGB.

float3 AgXToneMapping(float3 color)
{
    #ifdef GLSL
	// AgX constants
    const mat3 AgXInsetMatrix = mat3(
		vec3(0.856627153315983, 0.137318972929847, 0.11189821299995),
		vec3(0.0951212405381588, 0.761241990602591, 0.0767994186031903),
		vec3(0.0482516061458583, 0.101439036467562, 0.811302368396859)
	);

	// explicit AgXOutsetMatrix generated from Filaments AgXOutsetMatrixInv
    const mat3 AgXOutsetMatrix = mat3(
		vec3(1.1271005818144368, -0.1413297634984383, -0.14132976349843826),
		vec3(-0.11060664309660323, 1.157823702216272, -0.11060664309660294),
		vec3(-0.016493938717834573, -0.016493938717834257, 1.2519364065950405)
	);
    #else
    static const float3x3 AgXInsetMatrix =
    {
        { 0.856627153315983, 0.137318972929847, 0.11189821299995 },
        { 0.0951212405381588, 0.761241990602591, 0.0767994186031903 },
        { 0.0482516061458583, 0.101439036467562, 0.811302368396859 }
    };
    static const float3x3 AgXOutsetMatrix =
    {
        { 1.1271005818144368, -0.1413297634984383, -0.14132976349843826 },
        { -0.11060664309660323, 1.157823702216272, -0.11060664309660294 },
        { -0.016493938717834573, -0.016493938717834257, 1.2519364065950405 }
    };
    #endif

	// LOG2_MIN      = -10.0
	// LOG2_MAX      =  +6.5
	// MIDDLE_GRAY   =  0.18
    const float AgxMinEv = -12.47393; // log2( pow( 2, LOG2_MIN ) * MIDDLE_GRAY )
    const float AgxMaxEv = 4.026069; // log2( pow( 2, LOG2_MAX ) * MIDDLE_GRAY )

    //color *= toneMappingExposure;

    color = mul(color, LINEAR_SRGB_TO_LINEAR_REC2020);

    color = mul(color, AgXInsetMatrix);

	// Log2 encoding
    color = max(color, 1e-10); // avoid 0 or negative numbers for log2
    color = log2(color);
    color = (color - AgxMinEv) / (AgxMaxEv - AgxMinEv);

    color = clamp(color, 0.0, 1.0);

	// Apply sigmoid
    color = agxDefaultContrastApprox(color);

	// Apply AgX look
	// v = agxLook(v, look);

    color = mul(color, AgXOutsetMatrix);

	// Linearize
    color = pow(max(float3(0.0, 0.0, 0.0), color), float3(2.2, 2.2, 2.2));

    color = mul(color, LINEAR_REC2020_TO_LINEAR_SRGB);

	// Gamut mapping. Simple clamp for now.
    color = clamp(color, 0.0, 1.0);

    return color;
}
