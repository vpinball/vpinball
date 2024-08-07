$input v_texcoord0
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"

SAMPLER2D(tex_dmd, 0); // DMD
SAMPLER2D(dmdDotGlow, 1); // DMD glow
SAMPLER2D(dmdBackGlow, 2); // DMD back glow

uniform vec4 staticColor_Alpha;
uniform vec4 vRes_Alpha_time;
uniform vec4 vColor_Intensity;
uniform vec4 w_h_height;
uniform vec4 exposure;


#define dotColor      vColor_Intensity.rgb
#define brightness    vColor_Intensity.w
#define dotSize       w_h_height.x
#define dotSharpness  w_h_height.y
#define dotRounding   w_h_height.z
#define unlitDot      staticColor_Alpha.rgb
#define dmdSize       vRes_Alpha_time.xy
#define dotGlow       vRes_Alpha_time.z
#define backGlow      vRes_Alpha_time.w

#define UNLIT
//#define DOT_OVERLAP
#define DOTGLOW
#define BACKGLOW

// from http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float udRoundBox(vec2 p, float b, float r)
{
	vec2 q = abs(p) - b + r;
	return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r; // SDF with negative values inside the dot
    // return length(max(q, 0.0)) - r; // SDF with 0 distance inside the dot
}

// Apply brightness (for colored DMDs) or convert BW data to RGB one (for monochrome DMDs)
vec3 convertDmdToColor(vec4 sample)
{
	// FIXME PinMame does not push raw data but apply user range (default to 20..100)
	if (sample.a == 0.0) // brightness data
		return sample.r * (255. / 100.) * dotColor; // (max(0., sample.r * 255. - 20.) / 100.) * dotColor;
	else // RGB data
		return sample.rgb * brightness; // max(0., sample.rgb - 20./255.) * (255. / 235.) * brightness;
}

// Compute the dot color contribution from dot placed at 'ofs' to the fragment at dmdUv
vec3 computeDotColor(vec2 dmdUv, vec2 ofs)
{
	// Nearest filtering for base dots
	vec2 nearest = (floor(dmdUv * dmdSize) + ofs) / dmdSize;
	// Sampling position as (0,0) at dot center, (-1,-1) to (1,1) in dot corners
	vec2 pos = 2.0 * (fract(dmdUv * dmdSize) - ofs);
	// Dots from the lamp
	float dotThreshold = 0.01 + dotSize * (1.0 - dotSharpness);
	float dot = smoothstep(dotThreshold, -dotThreshold, udRoundBox(pos, dotSize, dotRounding * dotSize));
	vec3 dmd = convertDmdToColor(texNoLod(tex_dmd, nearest));
	#ifdef UNLIT
		dmd += unlitDot;
	#endif
	return dmd * dot;
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
vec3 agxDefaultContrastApprox(vec3 x)
{
    #if 1
    // 6th order polynomial approximation (used in three.js and Godot)
    // Mean error^2: 3.6705141e-06
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return + 15.5 * x4 * x2
           - 40.14 * x4 * x
           + 31.96 * x4
           - 6.868 * x2 * x
           + 0.4298 * x2
           + 0.1191 * x
           - 0.00232;
    #else
    // 7th order polynomial approximation (used in Filament)
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    vec3 x6 = x4 * x2;
    return - 17.86 * x6 * x
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
    color = pow(max(vec3(0.0, 0.0, 0.0), color), vec3(2.2, 2.2, 2.2)); // rec2020 to linear rec2020
    color = mul(color, LINEAR_REC2020_TO_LINEAR_SRGB);                 // linear rec2020 to linear rec709 (sRGB)
    color = FBGamma(color);                                            // linear sRGB to sRGB
    #endif

    return color;
}

#define MAX_BURST 1000.0

vec3 ReinhardToneMap(vec3 color)
{
    color *= exposure.x;

    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    const float l = min(dot(color, vec3(0.176204, 0.812985, 0.0108109)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}



#if !defined(CLIP)
	EARLY_DEPTH_STENCIL
#endif
void main()
{
	#ifdef CLIP
		if (v_clipDistance < 0.0)
			discard;
	#endif

	vec3 color;
	vec2 dmdUv = v_texcoord0;

	#ifdef DOT_OVERLAP
		// Base dot color, also taking in account the 9 nearest surrounding dots
		color = vec3_splat(0.0);
		for(int x = -1; x <= 1; x++) {
			for(int y = -1; y <= 1; y++) {
				color = max(color, computeDotColor(dmdUv, vec2(float(x), float(y)) + vec2_splat(0.5) ));
			}
		}
	#else
		// Base dot color, taken in account only the nearest dot
		color = computeDotColor(dmdUv, vec2_splat(0.5));
	#endif

	#ifdef DOTGLOW
		// glow from nearby lamps (taken from light DMD blur)
		color += convertDmdToColor(texNoLod(dmdDotGlow, dmdUv)) * dotGlow;
	#endif

	#ifdef BACKGLOW
		// add background diffuse light (very blurry from large DMD blur)
		color += convertDmdToColor(texNoLod(dmdBackGlow, dmdUv)) * backGlow;
	#endif

	#ifdef GLASS
		// Apply the glass as a tinted (lighten by the DMD using large blur, and the base color) additive blend.
		vec4 glass = texture(glassTexture, glassUv);
		vec3 glassLight = glassColor.rgb + 2.5 * glassColor.a * texture(dmdBackGlow, dmdUv).rgb * brightness;
		color += glass.rgb * glassLight.rgb;
	#endif

    #ifdef SRGB
    // Tonemapping and sRGB conversion.
	gl_FragColor = vec4(AgXToneMapping(color), 1.0);
	//gl_FragColor = vec4(FBGamma(ReinhardToneMap(color)), 1.0);
	//gl_FragColor = vec4(FBGamma(color), 1.0);

    #else
    // Rendering to a surface part of larger rendering process including tonemapping and sRGB conversion
	gl_FragColor = vec4(color, 1.0);

    #endif
}
