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
// by a texture
//

$input v_texcoord0, v_texcoord1
#ifdef CLIP
	$input v_clipDistance
#endif

// CRTNUANCE is a CRT shader build with the other filter, so every CRT path below applies to it as well and only the
// filter selection differs. Aliasing it here keeps that single difference in one place
#if defined(CRTNUANCE) && !defined(CRT)
	#define CRT
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
SAMPLER2D(displayGlass, 1); // Glass over display texture (usually dirt and scratches, optionally tinting)
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

#if defined(TARGET_essl)
	// Offset to the texel center, as sampling on the texel border can be unstable
	#define texFetch(tex, pos, size) texNoLod(tex, (vec2(pos) + vec2_splat(0.5)) / size)
#else
	#define texFetch(tex, pos, size) texelFetch(tex, pos, 0)
#endif


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
	#define N_SAMPLES      2                            // Number of surrounding dots in diffuse evaluation (this has a big performance impact)
	uniform vec4 vRes_Alpha_time;
	#define dmdSize        (vRes_Alpha_time.xy)         // Display size in dots
	#define coloredDMD     (displayProperties.x != 0.0) // Linear luminance or sRGB color
	#define sdfOffset      (displayProperties.y)        // Offset needed for SDF=0.5 at border decreasing to 0.0: 0.5 * (1.0 + (1.0 / (float(N_SAMPLES) + 0.5)) * dotSize / 2.0)
	#define dotThreshold   (displayProperties.z)        // Threshold inside SDF (so > 0.5): 0.5 + 0.5 * (0.025 /* Antialiasing */ + dotSize * (1.0 - dotSharpness) /* Darkening around border inside dot */);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//
// CRT display
//

#ifdef CRT
	uniform vec4 vRes_Alpha_time;
	#define crtSize        (vRes_Alpha_time.xy)   // input display size in pixels
	#define crtMode        (displayProperties.x)  // main render mode (pixelated, smoothed, CRT)
	// Output size in pixels is evaluated per pixel in main(), see 'outSize' there

	// Number of jittered samples per pixel when a CRT filter is downscaled, 1 disables oversampling path
	#define CRT_OVERSAMPLE          9
	#define CRT_OVERSAMPLE_KOROBOV  2 // Generator for Korobov (4 -> 1 or 3, 8 -> 5, 13 -> 8, 21 -> 13, etc)
	#define CRT_OVERSAMPLE2         4
	#define CRT_OVERSAMPLE_KOROBOV2 1 // Generator for Korobov (4 -> 1 or 3, 8 -> 5, 13 -> 8, 21 -> 13, etc)

	// CRT emulation, one permutation each:
	//   CRT        Timothy Lottes' CRTS filter (scanlines, warp, shadow mask, tonemapping)                    // from testing: preferrable for low res  (Bally Vidpins, Mr. Games, Gottlieb Caveman)
	//   CRTNUANCE  Nuance's CRT filter (convergence errors, ghosting, vignetting, scanlines, aperture grille) // from testing: preferrable for high res (Pin2K)
	// The renderer picks between them per display, see Renderer::SetupCRTRender
#ifndef CRTNUANCE

	// See definition in include header, and experiment here: https://www.shadertoy.com/view/MtSfRK
	// #define CRTS_DEBUG 1
	#define CRTS_GPU 1
	#define CRTS_GLSL 1
	//#define CRTS_2_TAP 1
	#define CRTS_TONE 1
	#define CRTS_CONTRAST 0
	#define CRTS_SATURATION 0
	#define CRTS_WARP 1
	//#define CRTS_MASK_GRILLE 1
	//#define CRTS_MASK_GRILLE_LITE 1
	//#define CRTS_MASK_NONE 1
	#define CRTS_MASK_SHADOW 1
	// Setup the function which returns input image color, which CRTS wants linear. displayTex is bound as sRGB so the
	// sampler has already decoded it, and an InvGamma here would decode twice and render the display far too dark
	vec3 CrtsFetch(vec2 uv) {
		return texFetch(displayTex, ivec2(uv * crtSize), crtSize).rgb;
	}
	
	#include "fs_crt_lottes.fs"

#else

	// Setup the function which returns input image color (here its in non linear 'display gamma' space), so the linear
	// that the sRGB-bound sampler hands back has to be re-encoded. Without it the display comes out far too dark
	// Explicit LOD as this is called from the oversampling loop, where implicit derivatives are meaningless (see CrtEmitter)
	vec3 CrtsNuanceFetch(vec2 uv) {
		return FBGamma(texNoLod(displayTex, clamp(uv, vec2_splat(0.0), vec2_splat(1.0))).rgb);
	}

	#include "fs_crt_nuance.fs"

#endif

// Evaluate the emitter at a single sample position (uv is normalized to the display, outSize is on-screen size in pixels).
// dUvDx/dUvDy (screen space derivatives of uv) passed explicitly since this is also called from the oversampling
// loop of main(), where the implicit ones would be those of the jittered positions, hence meaningless
vec3 CrtEmitter(const vec2 uv, const vec2 outSize, const vec2 dUvDx, const vec2 dUvDy)
{
	// Pixelated and smoothed only differ by the sampler they are bound with (point magnification for the former,
	// linear for the latter), both relying on implicit mipmapping/anisotropic filtering when the display is downscaled
	BRANCH if (crtMode != 2.0)
	{
		return texture2DGrad(displayTex, uv, dUvDx, dUvDy).rgb;
	}
	else // CRT
	{
	#ifdef CRTNUANCE
		return CrtsNuanceFilter(
		  uv,       // Input position (normalized)
		  crtSize,  // input size (in pixels)
		  outSize); // output size (in pixels)
	#else
		return CrtsFilter(
		  uv * outSize, // Input position
		  crtSize / outSize, // inputSize / outputSize (in pixels)
		  crtSize * vec2(0.5,0.5), // half input size
		  1.0 / crtSize, // 1.0 / input size
		  1.0 / outSize, // 1.0 / output size
		  2.0 / outSize, // 2.0 / output size
		  crtSize.y, // input height
		  vec2(1.0/48.0,1.0/24.0), // x and y warp
		  0.7, // Scanline thinness (same as third of CrtsTone below)
		  -2.5, // Horizonal scan blur
		  0.5, // Shadow mask effect (same as last of CrtsTone below)
		  CrtsTone(1.0,0.0,0.7,0.5));
	#endif
	}
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
	vec2 uv = displayUv + depthAmount * vec2(dot(T, eye), dot(B, eye)); */

	vec4 glass;
	float roughness;
	BRANCH if (hasGlass)
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
			// Resolve SDF after texture filtering, widening the transition to the on-screen gradient of the SDF (fwidth) so
			// that it always spans about one pixel: this antialiases the segment borders, which would otherwise alias more when
			// the display is downscaled. The lower bound is the fixed width used before, reached as soon as it is magnified.
			// Not perfect, but it helps
			vec4 aa = max(0.5 * (abs(dFdx(sdf)) + abs(dFdy(sdf))), vec4_splat(0.025));
			vec4 sharp = smoothstep(vec4_splat(0.5) - aa, vec4_splat(0.5) + aa, sdf);
			vec4 diffuse = diffuseStrength * sdf * sdf; // Magic formula to simulate light dispersion at maximum glass roughness
			vec4 light = mix(sharp, diffuse, roughness);
			//unlitLum4 += light;
			//litLum4   += light * alphaSegState[i];
			unlitLum4 = max(light, unlitLum4); // Max gives slightly better results than additive, especially for corners between segs that are overlit otherwise
			litLum4 = max(light * alphaSegState[i], litLum4);
		}
		//vec3 litLum = dot(litLum4, vec4_splat(1.0));
		//float unlitLum = dot(unlitLum4, vec4_splat(1.0));
		vec3 litLum = vec3_splat(max(max(litLum4.x, litLum4.y), max(litLum4.z, litLum4.w)));
		float unlitLum = max(max(unlitLum4.x, unlitLum4.y), max(unlitLum4.z, unlitLum4.w));

	#elif defined(DMD)
		float unlitLum = 0.0;
		vec3 litLum = vec3_splat(0.0);
		vec2 dmdUv = displayUv * dmdSize;
		ivec2 dotPos = ivec2(floor(dmdUv));
		vec2 dotRelativePos = fract(dmdUv) - vec2_splat(0.5);
		float thr = 0.15 * (abs(dFdx(dmdUv.x)) + abs(dFdx(dmdUv.y))); // Magic formula to adjust antialiasing to actual gradient
		float minThr = 0.5 - thr, maxThr = dotThreshold + thr;
		for (int y = -N_SAMPLES; y <= N_SAMPLES; y++)
		{
			for (int x = -N_SAMPLES; x <= N_SAMPLES; x++)
			{
				ivec2 dotUv = dotPos + ivec2(x,y);
				if (all(greaterThanEqual(dotUv, ivec2(0,0))) && all(lessThan(dotUv, ivec2(dmdSize))))
				{
					// SDF is 0.5 at dot border, increasing inside, linearly decreasing to 0 outside
					float sdf = clamp(sdfOffset - length(dotRelativePos - vec2(x,y)) * (0.5 / (float(N_SAMPLES) + 0.5)), 0.0, 1.0);
					float sharp = smoothstep(minThr, maxThr, sdf); // Compute dot lighting contribution (0 outside of this dot)
					float diffuse = diffuseStrength * sdf * sdf; // Magic formula to evaluate light dispersion at maximum glass roughness
					float light = mix(sharp, diffuse, roughness);
					unlitLum += light;
					if (coloredDMD) // RGB data (maybe sRGB, but this is handled by the hardware sampler)
					{
						litLum += light * texFetch(displayTex, dotUv, dmdSize).rgb;
					}
					else // linear brightness data
					{
						litLum += light * texFetch(displayTex, dotUv, dmdSize).rrr;
					}
				}
			}
		}

	#elif defined(CRT)
		float unlitLum = 0.0;
		vec3 litLum;
		// On-screen size of the display in pixels, which both CRT filters scale their scanlines and mask to.
		// The uv gradient gives the exact rasterized pixel density for any projection: playfield or ancillary
		// window, 2D backdrop or perspective 3D (where it rightfully varies over the surface), stereo, VR, and
		// supersampling. Both derivatives are used per axis so that rotated displays stay correct
		vec2 dUvDx = dFdx(displayUv);
		vec2 dUvDy = dFdy(displayUv);
		vec2 inv_outSize = max(vec2(length(vec2(dUvDx.x, dUvDy.x)), length(vec2(dUvDx.y, dUvDy.y))), vec2_splat(1e-8)); // guard against a degenerate (edge on) display
		vec2 outSize = 1.0 / inv_outSize;
	#if CRT_OVERSAMPLE > 1
		// When downscaled: the input grid, the scanlines and the shadow mask are all undersampled, which shows up as moiree. Supersample the emitter over the pixel footprint
		// in this situation. Only needed for CRT, which point samples and synthesizes patterns:
		// the pixelated and smoothed modes are filtered by implicitly (mipmapping/anisotropy) when downscaled.
		float ratio = max(crtSize.x * inv_outSize.x, crtSize.y * inv_outSize.y);
		BRANCH if ((crtMode == 2.0) && (ratio > 1.0)) //!! magic, 'should' be 1.0, but not sufficient due to the approximate/whacky pattern generators
		{
			// Korobov lattice, randomized per pixel (for now constant over time, otherwise temporal noise)
			const vec2 offs = hash22(gl_FragCoord.xy);
			litLum = vec3_splat(0.0);
			UNROLL for (int i = 0; i < CRT_OVERSAMPLE; ++i)
			{
				const float i_float = float(i);
				const vec2 xi = vec2(fract(i_float * (1.0 / float(CRT_OVERSAMPLE)) + offs.x), fract(i_float * (float(CRT_OVERSAMPLE_KOROBOV) / float(CRT_OVERSAMPLE)) + offs.y));
				litLum += CrtEmitter(displayUv + triangularPDF(xi.x) * dUvDx + triangularPDF(xi.y) * dUvDy, outSize, dUvDx, dUvDy);
			}
			litLum *= 1.0 / float(CRT_OVERSAMPLE);
		}
		else BRANCH if ((crtMode == 2.0) && (ratio > 0.31)) //!! 0.31 = magic, 'should' be 1.0, but not sufficient due to the approximate/whacky pattern generators
		{
			// Korobov lattice, randomized per pixel (for now constant over time, otherwise temporal noise)
			const vec2 offs = hash22(gl_FragCoord.xy);
			litLum = vec3_splat(0.0);
			UNROLL for (int i = 0; i < CRT_OVERSAMPLE2; ++i)
			{
				const float i_float = float(i);
				const vec2 xi = vec2(fract(i_float * (1.0 / float(CRT_OVERSAMPLE2)) + offs.x), fract(i_float * (float(CRT_OVERSAMPLE_KOROBOV2) / float(CRT_OVERSAMPLE2)) + offs.y));
				litLum += CrtEmitter(displayUv + triangularPDF(xi.x) * dUvDx + triangularPDF(xi.y) * dUvDy, outSize, dUvDx, dUvDy);
			}
			litLum *= 1.0 / float(CRT_OVERSAMPLE2);
		}
		else
	#endif
		{
			litLum = CrtEmitter(displayUv, outSize, dUvDx, dUvDy);
		}

	#endif

	// Shading is a mix of basic shading (just tinted texture ambient if any) and transmitted (tinted emitter ambient + light)
	vec3 lum = unlitLum * unlit + litLum * lit;
	if (hasGlass)
		lum = mix(lum, glassAmbient, 0.5 * glass.a);
	lum *= glass.rgb;

	// Convert to output color space
	BRANCH if (displayOutputMode == 0.0) // No tonemap, linear Color space
		gl_FragColor = vec4(lum, 1.0);
	else BRANCH if (displayOutputMode == 1.0) // Reinhard tonemapping, linear colorspace
		gl_FragColor = vec4(ReinhardToneMap(lum), 1.0);
	else BRANCH if (displayOutputMode == 2.0) // Reinhard tonemapping, sRGB colorspace
		gl_FragColor = vec4(FBGamma(ReinhardToneMap(lum)), 1.0);
}
