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

float ReinhardToneMap(const float l)
{
    return l * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
float2 ReinhardToneMap(const float2 color)
{
    const float l = dot(color, float2(0.176204 + 0.0108109 * 0.5, 0.812985 + 0.0108109 * 0.5)); // CIE RGB to XYZ, Y row (relative luminance) adjusted for RG colors
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
float3 ReinhardToneMap(const float3 color)
{
    const float l = dot(color, float3(0.176204, 0.812985, 0.0108109)); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}

#if 0
float3 FilmicToneMap(const float3 hdr, const float whitepoint) //!! test/experimental
{
    const float4 vh = float4(hdr,whitepoint);
    const float4 va = 1.425*vh + 0.05;
    const float4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    return vf.rgb/vf.aaa;
}

float3 FilmicToneMap2(const float3 texColor) //!! test/experimental
{
    const float3 x = max(0.,texColor-0.004); // Filmic Curve
    return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}
#endif

// Tony Mc MapFace Tonemapping (MIT licensed): see https://github.com/h3r2tic/tony-mc-mapface
// This tonemapping is similar to Reinhard but handles better highly saturated over powered colors
// (for these, it looks somewhat similar to AGX by desaturating them)
// It is more or less a Reinhard tonemapping followed by a color grade color correction
float3 TonyMcMapfaceToneMap(const float3 color)
{
    const float LUT_DIMS = 48.0;

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
