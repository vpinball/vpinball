// license:GPLv3+

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


#define dotColor      vColor_Intensity.rgb
#define dotSize       w_h_height.x
#define dotSharpness  w_h_height.y
#define dotRounding   w_h_height.z
#define coloredDMD    (w_h_height.w != 0.0)
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

// Apply dot tint and brightness scale
vec3 convertDmdToColor(vec3 samp)
{
	if (coloredDMD) // RGB data (eventually sRGB but this is handled by the hardware)
		return samp.rgb * dotColor;
	else // linear brightness data
		return samp.r * dotColor;
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
	vec3 dmd = convertDmdToColor(texNoLod(tex_dmd, nearest).rgb);
	#ifdef UNLIT
		dmd += unlitDot;
	#endif
	return dmd * dot;
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
		color += convertDmdToColor(texNoLod(dmdDotGlow, dmdUv).rgb) * dotGlow;
	#endif

	#ifdef BACKGLOW
		// add background diffuse light (very blurry from large DMD blur)
		color += convertDmdToColor(texNoLod(dmdBackGlow, dmdUv).rgb) * backGlow;
	#endif

	#ifdef GLASS
		// Apply the glass as a tinted (lighten by the DMD using large blur, and the base color) additive blend.
		vec4 glass = texture(glassTexture, glassUv);
		vec3 glassLight = glassColor.rgb + 2.5 * glassColor.a * convertDmdToColor(texture(dmdBackGlow, dmdUv).rgb);
		color += glass.rgb * glassLight.rgb;
	#endif

	#ifdef SRGB
	// sRGB conversion (tonemapping still to be implemented)
	gl_FragColor = vec4(FBGamma(color), 1.0);

	#else
	// Rendering to a surface part of larger rendering process including tonemapping and sRGB conversion
	gl_FragColor = vec4(color, 1.0);

	#endif
}
