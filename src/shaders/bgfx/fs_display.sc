// license:GPLv3+

// 
// Shader for DMD, CRT and segment displays
//
// They all follow the same model: an emitting surface with a glass above it.
// The difference between them is how the emitter is modeled.
//
// The shading is evaluated at the glass level which is defined by a tint and 
// a roughness (no refraction, but a slight parralax is applied, at vertex shader
// for simplicity and performance). When roughness is 0, the glass transmit what 
// is behind it, resulting in a tinted view of the emitter. When roughness is 1,
// the glass transmit a diffuse accumulation of light incoming from surroundings.
// 
// The glass is defined by a uniform color/roughness pair which can be modulated
// by a texture.
//

$input v_texcoord0, v_texcoord1
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"

// Common emitter uniforms
SAMPLER2D(displayTex, 0); // Display texture (meaning depends on display type)
#define displayUv v_texcoord1

uniform vec4 vColor_Intensity;
#define lit               (vColor_Intensity.rgb)
#define diffuseStrength   (vColor_Intensity.w)

uniform vec4 staticColor_Alpha;
#define unlit             (staticColor_Alpha.rgb)
#define displayOutputMode (staticColor_Alpha.w)

// Base glass properties
uniform vec4 glassTint_Roughness;
#define glassTint         (glassTint_Roughness.rgb)
#define glassRoughness    (glassTint_Roughness.w)

// Textured glass
SAMPLER2D(displayGlass, 1); // Glass over display texture (usually dirt and scratches, eventually tinting)
uniform vec4 w_h_height;
#define glassAmbient      (w_h_height.rgb)
#define hasGlass          (w_h_height.w != 0.0)
#define glassUv           v_texcoord0

// Common uniform for display specific properties
uniform vec4 displayProperties;


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
// Segment display
//

#ifdef SEG
	uniform vec4 alphaSegState[4];
#endif


/////////////////////////////////////////////////////////////////////////////////////////
//
// Dot Matrix Display
//

#ifdef DMD
	#define N_SAMPLES      2                     // Number of surrounding dots in diffuse evaluation (this has a big performance impact)
	uniform vec4 vRes_Alpha_time;
	#define dmdSize        (vRes_Alpha_time.xy)  // display size in dots
	#define coloredDMD     (displayProperties.x != 0.0) // Linear luminance or sRGB color
	#define sdfOffset      (displayProperties.y)        // Offset needed for SDF=0.5 at border decreasing to 0.0: 0.5 * (1.0 + (1.0 / (float(N_SAMPLES) + 0.5)) * dotSize / 2.0)
	#define dotThreshold   (displayProperties.z)        // Threshold inside SDF (so > 0.5): 0.5 + 0.5 * (0.025 /* Antialiasing */ + dotSize * (1.0 - dotSharpness) /* Darkening around border inside dot */);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//
// CRT display
//

#ifdef CRT

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
	vec2 uv = displayUv + depthAmount * vec2(dot(T, eye), dot(B, eye)); */

	vec4 glass;
	float roughness;
	if (hasGlass)
	{
		glass = texture2D(displayGlass, glassUv);
		glass.rgb *= glassTint;
		roughness = mix(glass.a, 1.0, glassRoughness);
	}
	else
	{
		glass = vec4(glassTint, 0.0);
		roughness = glassRoughness;
	}

	// Accumulate light from surrounding emitters (somewhat heavy)
	#if defined(SEG)
		vec4 litLum4 = vec4_splat(0.0);
		vec4 unlitLum4 = vec4_splat(0.0);
		for (int i = 0; i < 4; i++)
		{
			// SDF for 16 segments (4 RGBA tiles, each RGBA channel being the SDF for a given segment)
			vec4 sdf = texture2D(displayTex, vec2(0.25 * (displayUv.x + float(i)), displayUv.y));
			vec4 sharp = smoothstep(vec4_splat(0.475), vec4_splat(0.525), sdf); // Resolve SDF after texture filtering
			vec4 diffuse = diffuseStrength * sdf * sdf; // Magic formula to simulate light dispersion at maximum glass roughness
			vec4 light = mix(sharp, diffuse, roughness);
			unlitLum4 += light;
			litLum4   += light * alphaSegState[i];
		}
		vec3 litLum = dot(litLum4, vec4_splat(1.0)) * lit;
		float unlitLum = dot(unlitLum4, vec4_splat(1.0));
		
	#elif defined(DMD)
		float unlitLum = 0.0;
		vec3 litLum = vec3_splat(0.0);
		vec2 dmdUv = displayUv * dmdSize;
		ivec2 dotPos = ivec2(floor(dmdUv));
		vec2 dotRelativePos = fract(dmdUv) - vec2_splat(0.5);
		for (int y = -N_SAMPLES; y <= N_SAMPLES; y++)
			for (int x = -N_SAMPLES; x <= N_SAMPLES; x++)
			{
				ivec2 dotUv = dotPos + ivec2(x,y);
				if (all(greaterThanEqual(dotUv, vec2_splat(0.0))) && all(lessThan(dotUv, dmdSize)))
				{
					// SDF is 0.5 at dot border, increasing inside, linearly decreasing to 0 outside
					float sdf = clamp(sdfOffset - length(dotRelativePos - vec2(x,y)) * (0.5 / (float(N_SAMPLES) + 0.5)), 0.0, 1.0);
					float sharp = smoothstep(0.5, dotThreshold, sdf); // Compute dot lighting contribution (0 outside of this dot)
					float diffuse = diffuseStrength * sdf * sdf; // Magic formula to evaluate light dispersion at maximum glass roughness
					float light = mix(sharp, diffuse, roughness);
					unlitLum += light;
					if (coloredDMD) // RGB data (eventually sRGB but this is handled by the hardware sampler)
						litLum += light * texelFetch(displayTex, dotUv, 0).rgb * lit;
					else // linear brightness data
						litLum += light * texelFetch(displayTex, dotUv, 0).r * lit;
				}
			}
		
	#elif defined(CRT)
		// TODO implement CRT shading (see Lottes public domain CRT shader)
		float unlitLum = 0.0;
		vec3 litLum = vec3_splat(0.0);
		
	#endif

	// Shading is a mix of basic shading (just tinted texture ambient if any) and transmitted (tinted emitter ambient + light)
	vec3 lum = (unlitLum * unlit + litLum);
	if (hasGlass)
		lum = mix(lum, glassAmbient, 0.5 * glass.a);
	lum *=  glass.rgb;

	// Convert to output color space
	if (displayOutputMode == 0.0) // No tonemap, linear Color space
		gl_FragColor = vec4(lum, 1.0);
	else if (displayOutputMode == 1.0) // Reinhard tonemapping, linear colorspace
		gl_FragColor = vec4(ReinhardToneMap(lum), 1.0);
	else if (displayOutputMode == 2.0) // Reinhard tonemapping, sRGB colorspace
		gl_FragColor = vec4(FBGamma(ReinhardToneMap(lum)), 1.0);
}
