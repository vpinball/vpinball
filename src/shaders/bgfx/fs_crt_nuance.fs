// license:GPLv3+

//
// Simple CRT emulation, ported from the Nuance (Nuon emulator) 16/32 bit display shader
// (video_m32_o32_crt.fs, which itself was loosely inspired by the usual reshade CRT presets).
//
// It applies, in this order:
//  - a small per channel (RGB) offset, emulating convergence errors of the electron beams,
//  - a ghosting/echo term, emulating signal reflections of an analog cable,
//  - a contrast/gamma push,
//  - vignetting,
//  - a slight tint and overall gain,
//  - horizontal scanlines,
//  - a vertical aperture grille like mask pattern.
//
// Everything is evaluated in 'display gamma' space (like the original shader), the result is
// converted back to linear space before being returned.
//
// The caller must define CrtsNuanceFetch(uv) which returns the (non linear, i.e. as stored in
// the texture) color of the source image at the given normalized [0..1] texture coordinates.
//

#ifndef CRTS_NUANCE_FS
#define CRTS_NUANCE_FS

float CrtsNuanceSqr(float x)
{
	return x * x;
}

// uv      : normalized [0..1] position inside the display
// srcSize : source image size in pixels
// dstSize : output (screen) size of the display in pixels
vec3 CrtsNuanceFilter(vec2 uv, vec2 srcSize, vec2 dstSize)
{
	vec2 uvw = vec2(uv.x, 1.0 - uv.y) * dstSize; // output position in pixels (flipped y just to match reshade)
	vec2 uvw01 = uv;                             // output position in 0..1, 0..1

	// Limit mask size: the grille has a period of 2 output pixels, so it gets ever finer relative to the source
	// pixels it is tied to as the display is magnified. Halve its frequency (the original loops, one step at a
	// time) until a period spans at least 2/3 of a source pixel, which also keeps it an integer number of output
	// pixels, hence aligned to the pixel grid. Closed form of that loop, for an arbitrary magnification.
	uvw.x *= exp2(-max(0.0, ceil(log2(dstSize.x / (3.0 * srcSize.x)))));

	// Limit scanline size
	if (dstSize.y > srcSize.y * 2.0)
		uvw.y *= srcSize.y * 2.0 / dstSize.y;

	// Per channel convergence offsets
	vec3 col;
	vec2 offs = vec2( 0.001, 0.001);
	col.r = CrtsNuanceFetch(uv + offs).r + 0.05;
	     offs = vec2( 0.000,-0.002);
	col.g = CrtsNuanceFetch(uv + offs).g + 0.05;
	     offs = vec2(-0.002, 0.000);
	col.b = CrtsNuanceFetch(uv + offs).b + 0.05;

	// Ghosting
	     offs = 0.45 * vec2(-0.014,-0.027) + vec2( 0.001, 0.001);
	col.r += 0.03505 * CrtsNuanceSqr(clamp(3.0 * CrtsNuanceFetch(uv + offs).r, 0.0, 1.0));
	     offs = 0.45 * vec2(-0.019,-0.020) + vec2( 0.000,-0.002);
	col.g += 0.02065 * CrtsNuanceSqr(clamp(3.0 * CrtsNuanceFetch(uv + offs).g, 0.0, 1.0));
	     offs = 0.35 * vec2(-0.017,-0.003) + vec2(-0.002, 0.000);
	col.b += 0.04430 * CrtsNuanceSqr(clamp(3.0 * CrtsNuanceFetch(uv + offs).b, 0.0, 1.0));

	col = clamp(0.6 * col + 0.4 * col * col, 0.0, 1.0);

	float vignetting = 0.1 + 16.0 * (uvw01.x - uvw01.x * uvw01.x) * (uvw01.y - uvw01.y * uvw01.y);
	col *= pow(vignetting, 0.2);

	col *= vec3(0.95, 1.05, 0.95) * 1.15 * 2.8;

	col *= pow(0.35 + 0.18 * sin(uvw.y * 1.5), 0.9); // scanline

	col *= 1.0 - 0.23 * clamp(2.0 * uvw.x - (4.0 * floor(uvw.x * 0.5) + 2.0), 0.0, 1.0); // mask pattern

	return InvGamma(col); // back to linear space
}

#endif
