// license:GPLv3+

$input v_texcoord0, v_texcoord1
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"


SAMPLER2D(displayTex, 0); // Display texture (meaning depends on display type)
#define displayUv v_texcoord1

SAMPLER2D(displayGlass, 1); // Glass over display texture (usually dirt and scratches, eventually tinting)
#define glassUv v_texcoord0

uniform vec4 vColor_Intensity;
#define lit               (vColor_Intensity.rgb)
#define backGlow          (vColor_Intensity.w)

uniform vec4 staticColor_Alpha;
#define unlit             (staticColor_Alpha.rgb)
#define displayOutputMode (staticColor_Alpha.w)

uniform vec4 glassAmbient_Roughness;
#define glassAmbient      (glassAmbient_Roughness.rgb)
#define glassRoughness    (glassAmbient_Roughness.w)

uniform vec4 w_h_height;
#define hasGlass          (w_h_height.x != 0.0)

// Basic Reinhard implementation while waiting for full tonemapping support
#define BURN_HIGHLIGHTS 0.25
#define MAX_BURST 1000.0
vec3 ReinhardToneMap(vec3 color)
{
    // The clamping (to an arbitrary high value) prevents overflow leading to nan/inf in turn rendered as black blobs (at least on NVidia hardware)
    float l = min(dot(color, vec3(0.176204, 0.812985, 0.0108109)), MAX_BURST); // CIE RGB to XYZ, Y row (relative luminance)
    return color * ((l * BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// Alphanumeric segment display
//

#ifdef ALPHASEG
	uniform vec4 alphaSegState[4];
	#define segSDF displayTex // Segment SDF for 16 segments (4 RGBA tiles, each channel being a SDF)

	void shade4(vec4 state, sampler2D sdfTex, vec2 uv, inout vec3 color, inout vec4 glow)
	{
		vec4 sdf = texture2D(sdfTex, uv);
		vec4 inner = smoothstep(vec4_splat(0.475), vec4_splat(0.525), sdf);
		color += inner.x * (unlit + lit * state.x);
		color += inner.y * (unlit + lit * state.y);
		color += inner.z * (unlit + lit * state.z);
		color += inner.w * (unlit + lit * state.w);
		glow = max(glow, state * sdf);
	}
#endif


/////////////////////////////////////////////////////////////////////////////////////////
//
// Dot Matrix Display
//

#ifdef DMD
	uniform vec4 vRes_Alpha_time;
	#define dmdSize        (vRes_Alpha_time.xy)  // display size in dots
	#define dotSize        (vRes_Alpha_time.z)   // dot size / 2.0 (i.e. radius), therefore 0.5 correspond to a fully covered dot
	#define dotRounding    (vRes_Alpha_time.w)   // dot rounding * dot size / 2.0 (i.e. rounding scaled by dot radius)
	#define dotThreshold   (w_h_height.z)        // Threshold in inner SDF
	#define coloredDMD     (w_h_height.w != 0.0) // Linear luminance or sRGB color
	#define dmdTex         displayTex            // DMD texture (either single channel linear luminance, or sRGB image)
	SAMPLER2D(dmdGlowTex, 2);                    // DMD glow, computed through blurring for back and glass lighting

	// from http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
	// Compute SDF with negative values inside the dot (to allow shading light inside dot)
	float udRoundBox(vec2 p, float b, float r)
	{
		vec2 q = abs(p) - b + r;
		return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
	}

	// Apply dot tint and brightness scale
	vec3 sampleDmd(vec3 samp)
	{
		if (coloredDMD) // RGB data (eventually sRGB but this is handled by the hardware sampler)
			return samp.rgb * lit;
		else // linear brightness data
			return samp.r * lit;
	}
#endif

#if !defined(CLIP)
	EARLY_DEPTH_STENCIL
#endif
void main()
{
	#ifdef CLIP
		if (v_clipDistance < 0.0)
			discard;
	#endif

	// Parallax offset between glass and display
	/* Disabled as this is somewhat overkill and a satisfying enough approximation can be applied as a simple texture offset
	float depthAmount = 0.05;
	vec3 dpx = dFdx(pos);
	vec3 dpy = dFdy(pos);
	vec2 duvx = dFdx(glassUV);
	vec2 duvy = dFdy(glassUV);
	vec3 dp2perp = cross(normal, dpy);
	vec3 dp1perp = cross(dpx, normal);
	vec3 T = normalize(dp2perp * duvx.x + dp1perp * duvy.x);
	vec3 B = normalize(dp2perp * duvx.y + dp1perp * duvy.y);
	vec3 eye = normalize(pos);
	vec2 uv = vec2(displayUv.x / 4.0, displayUv.y);
	uv.x -= depthAmount * dot(T, eye);
	uv.y -= depthAmount * dot(B, eye); */

	// Compute display emitted light
	vec3 lum = vec3_splat(0.0);

	#ifdef ALPHASEG
		// Accumulate segment lighting
		vec4 glow4 = vec4_splat(0.0);
		shade4(alphaSegState[0], segSDF, vec2(displayUv.x * 0.25       , displayUv.y), lum, glow4);
		shade4(alphaSegState[1], segSDF, vec2(displayUv.x * 0.25 + 0.25, displayUv.y), lum, glow4);
		shade4(alphaSegState[2], segSDF, vec2(displayUv.x * 0.25 + 0.50, displayUv.y), lum, glow4);
		shade4(alphaSegState[3], segSDF, vec2(displayUv.x * 0.25 + 0.75, displayUv.y), lum, glow4);
		
		// Compute indirect glow on back and glass as the max of individual segment lights (since this is approximated from a single distance)
		float glowLum = max(max(max(glow4.x, glow4.y), glow4.z), glow4.w);
		vec3 glow = glowLum * glowLum * lit;
		
	#elif defined(DMD)
		// Coordinate of nearest dot center (if any)
		vec2 nearest = (floor(displayUv * dmdSize) + vec2_splat(0.5)) / dmdSize;
		if ((nearest.x >= 0.0) && (nearest.y >= 0.0) && (nearest.x <= 1.0) && (nearest.y <= 1.0))
		{
			// Compute SDF with (0,0) at dot center, (-1,-1) to (1,1) in dot corners
			vec2 pos = fract(displayUv * dmdSize) - vec2_splat(0.5);
			float sdf = udRoundBox(pos, dotSize, dotRounding);
			
			// Compute inner lighting
			float inner = smoothstep(0.0, dotThreshold, sdf);
			lum = inner * (unlit + sampleDmd(texNoLod(dmdTex, nearest).rgb));
		}
		
		// Compute indirect glow on back and glass
		#if BGFX_SHADER_LANGUAGE_GLSL
			vec3 glow = sampleDmd(texNoLod(dmdGlowTex, vec2(0.25, 0.25) + 0.5 * vec2(displayUv.x, 1.0 - displayUv.y)).rgb);
		#else
			vec3 glow = sampleDmd(texNoLod(dmdGlowTex, vec2(0.25, 0.25) + 0.5 * displayUv).rgb);
		#endif
		
	#elif defined(CRT)
		// TODO implement CRT shading (see Lottes public domain CRT shader)
		
	#endif

	// Compute back glow
	lum += backGlow * glow;

	// Compute glass shading
	if (hasGlass)
	{
		vec4 glass = texture2D(displayGlass, glassUv);
		lum = mix(lum, (glassAmbient + glow), glassRoughness * glass.a) * glass.rgb;
	}

	// Convert to output color space
	if (displayOutputMode == 0.0) // No tonemap, linear Color space
		gl_FragColor = vec4(lum, 1.0);
	else if (displayOutputMode == 1.0) // Reinhard tonemapping, linear colorspace
		gl_FragColor = vec4(ReinhardToneMap(lum), 1.0);
	else if (displayOutputMode == 2.0) // Reinhard tonemapping, sRGB colorspace
		gl_FragColor = vec4(FBGamma(ReinhardToneMap(lum)), 1.0);
}
