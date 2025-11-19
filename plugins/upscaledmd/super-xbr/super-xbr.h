//// *** Super-xBR code begins here - MIT LICENSE *** ///

/*

*******  Super XBR Scaler  *******

Copyright (c) 2016 Hyllian - sergiogdb@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#pragma once

#include <cmath>

namespace superxbr
{

#ifndef min
template <typename T> constexpr T min(const T x, const T y) { return x < y ? x : y; }
#endif
#ifndef max
template <typename T> constexpr T max(const T x, const T y) { return x < y ? y : x; }
#endif

static inline float R(uint32_t col) { return (float)( col      & 0xFFu); }
static inline float G(uint32_t col) { return (float)((col>> 8) & 0xFFu); }
static inline float B(uint32_t col) { return (float)((col>>16) & 0xFFu); }
static inline float A(uint32_t col) { return (float)((col>>24) & 0xFFu); }

static constexpr double wgt1 = 0.129633;
static constexpr double wgt2 = 0.175068;
static constexpr float w1    = -wgt1;
static constexpr float w2    = wgt1+0.5;
static constexpr float w3    = -wgt2;
static constexpr float w4    = wgt2+0.5;


static inline float df(float A, float B)
{
	return fabsf(A - B);
}

static inline float min4(float a, float b, float c, float d)
{
	return min(min(a,b),min(c,d));
}

static inline float max4(float a, float b, float c, float d)
{
	return max(max(a,b),max(c,d));
}

template<class T>
inline T clamp(T x, T floor, T ceil)
{
	return max(min(x,ceil),floor);
}

/*
                         P1
|P0|B |C |P1|         C     F4          |a0|b1|c2|d3|
|D |E |F |F4|      B     F     I4       |b0|c1|d2|e3|   |e1|i1|i2|e2|
|G |H |I |I4|   P0    E  A  I     P3    |c0|d1|e2|f3|   |e3|i3|i4|e4|
|P2|H5|I5|P3|      D     H     I5       |d0|e1|f2|g3|
                      G     H5
                         P2

sx, sy
-1  -1 | -2  0   (x+y) (x-y)    -3  1  (x+y-1)  (x-y+1)
-1   0 | -1 -1                  -2  0
-1   1 |  0 -2                  -1 -1
-1   2 |  1 -3                   0 -2

 0  -1 | -1  1   (x+y) (x-y)      ...     ...     ...
 0   0 |  0  0
 0   1 |  1 -1
 0   2 |  2 -2
 
 1  -1 |  0  2   ...
 1   0 |  1  1
 1   1 |  2  0
 1   2 |  3 -1
 
 2  -1 |  1  3   ...
 2   0 |  2  2
 2   1 |  3  1
 2   2 |  4  0


*/

static inline float diagonal_edge(const float mat[][4], const float * const __restrict wp) {
	float dw1 = wp[0]*(df(mat[0][2], mat[1][1]) + df(mat[1][1], mat[2][0]) + df(mat[1][3], mat[2][2]) + df(mat[2][2], mat[3][1])) +
				wp[1]*(df(mat[0][3], mat[1][2]) + df(mat[2][1], mat[3][0])) +
				wp[2]*(df(mat[0][3], mat[2][1]) + df(mat[1][2], mat[3][0])) +
				wp[3]* df(mat[1][2], mat[2][1]) +
				wp[4]*(df(mat[0][2], mat[2][0]) + df(mat[1][3], mat[3][1])) +
				wp[5]*(df(mat[0][1], mat[1][0]) + df(mat[2][3], mat[3][2]));

	float dw2 = wp[0]*(df(mat[0][1], mat[1][2]) + df(mat[1][2], mat[2][3]) + df(mat[1][0], mat[2][1]) + df(mat[2][1], mat[3][2])) +
				wp[1]*(df(mat[0][0], mat[1][1]) + df(mat[2][2], mat[3][3])) +
				wp[2]*(df(mat[0][0], mat[2][2]) + df(mat[1][1], mat[3][3])) +
				wp[3]* df(mat[1][1], mat[2][2]) +
				wp[4]*(df(mat[1][0], mat[3][2]) + df(mat[0][1], mat[2][3])) +
				wp[5]*(df(mat[0][2], mat[1][3]) + df(mat[2][0], mat[3][1]));

	return (dw1 - dw2);
}

#if 0
static inline float cross_edge(const float mat[][4], const float* const __restrict wp) {
	float hvw1 = wp[3] * (df(mat[1][1], mat[2][1]) + df(mat[1][2], mat[2][2])) +
				 wp[0] * (df(mat[0][1], mat[1][1]) + df(mat[2][1], mat[3][1]) + df(mat[0][2], mat[1][2]) + df(mat[2][2], mat[3][2])) +
				 wp[2] * (df(mat[0][1], mat[2][1]) + df(mat[1][1], mat[3][1]) + df(mat[0][2], mat[2][2]) + df(mat[1][2], mat[3][2]));

	float hvw2 = wp[3] * (df(mat[1][1], mat[1][2]) + df(mat[2][1], mat[2][2])) +
				 wp[0] * (df(mat[1][0], mat[1][1]) + df(mat[2][0], mat[2][1]) + df(mat[1][2], mat[1][3]) + df(mat[2][2], mat[2][3])) +
				 wp[2] * (df(mat[1][0], mat[1][2]) + df(mat[1][1], mat[1][3]) + df(mat[2][0], mat[2][2]) + df(mat[2][1], mat[2][3]));

	return (hvw1 - hvw2);
}
#endif

///////////////////////// Super-xBR scaling
// perform super-xbr (fast shader version) scaling by factor f=2 only.
// alpha indicates whether an alpha channel is present in the input data, if false, alpha is ignored and set to 255.
template<int f,bool alpha>
void scale(const uint32_t* const __restrict data, uint32_t* const __restrict out, const int w, const int h) {
	const int outw = w*f, outh = h*f;

	{
	static constexpr float wp[6] = { 2.0f, 1.0f, -1.0f, 4.0f, -1.0f, 1.0f };

	// First Pass
	for (int y = 0; y < outh; y+=f)
		for (int x = 0; x < outw; x+=f) {
			float r[4][4], g[4][4], b[4][4], a[4][4], Y[4][4];
			const int cx = x / f, cy = y / f; // central pixels on original images
			// sample supporting pixels in original image
			for (int sx = -1; sx <= 2; ++sx) {
				for (int sy = -1; sy <= 2; ++sy) {
					// clamp pixel locations
					const int csy = clamp(sy + cy, 0, h - 1);
					const int csx = clamp(sx + cx, 0, w - 1);
					// sample & add weighted components
					const uint32_t sample = data[csy*w + csx];
					r[sx + 1][sy + 1] = R(sample);
					g[sx + 1][sy + 1] = G(sample);
					b[sx + 1][sy + 1] = B(sample);
					if (alpha) a[sx + 1][sy + 1] = A(sample);
					Y[sx + 1][sy + 1] = 0.2126f*r[sx + 1][sy + 1] + 0.7152f*g[sx + 1][sy + 1] + 0.0722f*b[sx + 1][sy + 1];
				}
			}
			const float min_r_sample = min4(r[1][1], r[2][1], r[1][2], r[2][2]);
			const float min_g_sample = min4(g[1][1], g[2][1], g[1][2], g[2][2]);
			const float min_b_sample = min4(b[1][1], b[2][1], b[1][2], b[2][2]);
			const float min_a_sample = alpha ? min4(a[1][1], a[2][1], a[1][2], a[2][2]) : 255.f;
			const float max_r_sample = max4(r[1][1], r[2][1], r[1][2], r[2][2]);
			const float max_g_sample = max4(g[1][1], g[2][1], g[1][2], g[2][2]);
			const float max_b_sample = max4(b[1][1], b[2][1], b[1][2], b[2][2]);
			const float max_a_sample = alpha ? max4(a[1][1], a[2][1], a[1][2], a[2][2]) : 255.f;
			const float d_edge = diagonal_edge(Y, wp);
			const float r1 = w1*(r[0][3] + r[3][0]) + w2*(r[1][2] + r[2][1]);
			const float g1 = w1*(g[0][3] + g[3][0]) + w2*(g[1][2] + g[2][1]);
			const float b1 = w1*(b[0][3] + b[3][0]) + w2*(b[1][2] + b[2][1]);
			const float a1 = alpha ? w1*(a[0][3] + a[3][0]) + w2*(a[1][2] + a[2][1]) : 255.f;
			const float r2 = w1*(r[0][0] + r[3][3]) + w2*(r[1][1] + r[2][2]);
			const float g2 = w1*(g[0][0] + g[3][3]) + w2*(g[1][1] + g[2][2]);
			const float b2 = w1*(b[0][0] + b[3][3]) + w2*(b[1][1] + b[2][2]);
			const float a2 = alpha ? w1*(a[0][0] + a[3][3]) + w2*(a[1][1] + a[2][2]) : 255.f;
			// generate and write result
			float rf, gf, bf, af;
			if (d_edge <= 0.0f) { rf = r1; gf = g1; bf = b1; af = a1; }
			else { rf = r2; gf = g2; bf = b2; af = a2; }
			// anti-ringing, clamp.
			rf = clamp(rf, min_r_sample, max_r_sample);
			gf = clamp(gf, min_g_sample, max_g_sample);
			bf = clamp(bf, min_b_sample, max_b_sample);
			af = alpha ? clamp(af, min_a_sample, max_a_sample) : 255.f;
			const int ri = static_cast<int>(ceilf(clamp(rf, 0.f, 255.f)));
			const int gi = static_cast<int>(ceilf(clamp(gf, 0.f, 255.f)));
			const int bi = static_cast<int>(ceilf(clamp(bf, 0.f, 255.f)));
			const int ai = alpha ? static_cast<int>(ceilf(clamp(af, 0.f, 255.f))) : 255;
			const int ofs = y*outw + x;
			out[ofs] = out[ofs + 1] = out[ofs + outw] = data[cy*w + cx];
			out[ofs + outw + 1] = (ai << 24) | (bi << 16) | (gi << 8) | ri;
		}
	}
	

	// Second Pass
	{
	static constexpr float wp[6] = { 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	for (int y = 0; y < outh; y+=f)
		for (int x = 0; x < outw; x+=f) {
			float r[4][4], g[4][4], b[4][4], a[4][4], Y[4][4];
			// sample supporting pixels in original image
			for (int sx = -1; sx <= 2; ++sx) {
				for (int sy = -1; sy <= 2; ++sy) {
					// clamp pixel locations
					const int csy = clamp(sx - sy + y, 0, f*h - 1);
					const int csx = clamp(sx + sy + x, 0, f*w - 1);
					// sample & add weighted components
					const uint32_t sample = out[csy*outw + csx];
					r[sx + 1][sy + 1] = R(sample);
					g[sx + 1][sy + 1] = G(sample);
					b[sx + 1][sy + 1] = B(sample);
					if (alpha) a[sx + 1][sy + 1] = A(sample);
					Y[sx + 1][sy + 1] = 0.2126f*r[sx + 1][sy + 1] + 0.7152f*g[sx + 1][sy + 1] + 0.0722f*b[sx + 1][sy + 1];
				}
			}
			const float min_r_sample = min4(r[1][1], r[2][1], r[1][2], r[2][2]);
			const float min_g_sample = min4(g[1][1], g[2][1], g[1][2], g[2][2]);
			const float min_b_sample = min4(b[1][1], b[2][1], b[1][2], b[2][2]);
			const float min_a_sample = alpha ? min4(a[1][1], a[2][1], a[1][2], a[2][2]) : 255.f;
			const float max_r_sample = max4(r[1][1], r[2][1], r[1][2], r[2][2]);
			const float max_g_sample = max4(g[1][1], g[2][1], g[1][2], g[2][2]);
			const float max_b_sample = max4(b[1][1], b[2][1], b[1][2], b[2][2]);
			const float max_a_sample = alpha ? max4(a[1][1], a[2][1], a[1][2], a[2][2]) : 255.f;
			{
			const float d_edge = diagonal_edge(Y, wp);
			const float r1 = w3*(r[0][3] + r[3][0]) + w4*(r[1][2] + r[2][1]);
			const float g1 = w3*(g[0][3] + g[3][0]) + w4*(g[1][2] + g[2][1]);
			const float b1 = w3*(b[0][3] + b[3][0]) + w4*(b[1][2] + b[2][1]);
			const float a1 = alpha ? w3*(a[0][3] + a[3][0]) + w4*(a[1][2] + a[2][1]) : 255.f;
			const float r2 = w3*(r[0][0] + r[3][3]) + w4*(r[1][1] + r[2][2]);
			const float g2 = w3*(g[0][0] + g[3][3]) + w4*(g[1][1] + g[2][2]);
			const float b2 = w3*(b[0][0] + b[3][3]) + w4*(b[1][1] + b[2][2]);
			const float a2 = alpha ? w3*(a[0][0] + a[3][3]) + w4*(a[1][1] + a[2][2]) : 255.f;
			// generate and write result
			float rf, gf, bf, af;
			if (d_edge <= 0.0f) { rf = r1; gf = g1; bf = b1; af = a1; }
			else { rf = r2; gf = g2; bf = b2; af = a2; }
			// anti-ringing, clamp.
			rf = clamp(rf, min_r_sample, max_r_sample);
			gf = clamp(gf, min_g_sample, max_g_sample);
			bf = clamp(bf, min_b_sample, max_b_sample);
			af = alpha ? clamp(af, min_a_sample, max_a_sample) : 255.f;
			const int ri = static_cast<int>(ceilf(clamp(rf, 0.f, 255.f)));
			const int gi = static_cast<int>(ceilf(clamp(gf, 0.f, 255.f)));
			const int bi = static_cast<int>(ceilf(clamp(bf, 0.f, 255.f)));
			const int ai = alpha ? static_cast<int>(ceilf(clamp(af, 0.f, 255.f))) : 255;
			out[y*outw + x + 1] = (ai << 24) | (bi << 16) | (gi << 8) | ri;
			}

			for (int sx = -1; sx <= 2; ++sx) {
				for (int sy = -1; sy <= 2; ++sy) {
					// clamp pixel locations
					const int csy = clamp(sx - sy + 1 + y, 0, f*h - 1);
					const int csx = clamp(sx + sy - 1 + x, 0, f*w - 1);
					// sample & add weighted components
					const uint32_t sample = out[csy*outw + csx];
					r[sx + 1][sy + 1] = R(sample);
					g[sx + 1][sy + 1] = G(sample);
					b[sx + 1][sy + 1] = B(sample);
					if (alpha) a[sx + 1][sy + 1] = A(sample);
					Y[sx + 1][sy + 1] = 0.2126f*r[sx + 1][sy + 1] + 0.7152f*g[sx + 1][sy + 1] + 0.0722f*b[sx + 1][sy + 1];
				}
			}
			const float d_edge = diagonal_edge(Y, wp);
			const float r1 = w3*(r[0][3] + r[3][0]) + w4*(r[1][2] + r[2][1]);
			const float g1 = w3*(g[0][3] + g[3][0]) + w4*(g[1][2] + g[2][1]);
			const float b1 = w3*(b[0][3] + b[3][0]) + w4*(b[1][2] + b[2][1]);
			const float a1 = alpha ? w3*(a[0][3] + a[3][0]) + w4*(a[1][2] + a[2][1]) : 255.f;
			const float r2 = w3*(r[0][0] + r[3][3]) + w4*(r[1][1] + r[2][2]);
			const float g2 = w3*(g[0][0] + g[3][3]) + w4*(g[1][1] + g[2][2]);
			const float b2 = w3*(b[0][0] + b[3][3]) + w4*(b[1][1] + b[2][2]);
			const float a2 = alpha ? w3*(a[0][0] + a[3][3]) + w4*(a[1][1] + a[2][2]) : 255.f;
			// generate and write result
			float rf, gf, bf, af;
			if (d_edge <= 0.0f) { rf = r1; gf = g1; bf = b1; af = a1; }
			else { rf = r2; gf = g2; bf = b2; af = a2; }
			// anti-ringing, clamp.
			rf = clamp(rf, min_r_sample, max_r_sample);
			gf = clamp(gf, min_g_sample, max_g_sample);
			bf = clamp(bf, min_b_sample, max_b_sample);
			af = alpha ? clamp(af, min_a_sample, max_a_sample) : 255.f;
			const int ri = static_cast<int>(ceilf(clamp(rf, 0.f, 255.f)));
			const int gi = static_cast<int>(ceilf(clamp(gf, 0.f, 255.f)));
			const int bi = static_cast<int>(ceilf(clamp(bf, 0.f, 255.f)));
			const int ai = alpha ? static_cast<int>(ceilf(clamp(af, 0.f, 255.f))) : 255;
			out[(y+1)*outw + x] = (ai << 24) | (bi << 16) | (gi << 8) | ri;
		}
	}

	// Third Pass
	{
	static constexpr float wp[6] = { 2.0f, 1.0f, -1.0f, 4.0f, -1.0f, 1.0f };

	for (int y = outh - 1; y >= 0; --y)
		for (int x = outw - 1; x >= 0; --x) {
			float r[4][4], g[4][4], b[4][4], a[4][4], Y[4][4];
			for (int sx = -2; sx <= 1; ++sx) {
				for (int sy = -2; sy <= 1; ++sy) {
					// clamp pixel locations
					const int csy = clamp(sy + y, 0, f*h - 1);
					const int csx = clamp(sx + x, 0, f*w - 1);
					// sample & add weighted components
					const uint32_t sample = out[csy*outw + csx];
					r[sx + 2][sy + 2] = R(sample);
					g[sx + 2][sy + 2] = G(sample);
					b[sx + 2][sy + 2] = B(sample);
					if (alpha) a[sx + 2][sy + 2] = A(sample);
					Y[sx + 2][sy + 2] = 0.2126f*r[sx + 2][sy + 2] + 0.7152f*g[sx + 2][sy + 2] + 0.0722f*b[sx + 2][sy + 2];
				}
			}
			const float min_r_sample = min4(r[1][1], r[2][1], r[1][2], r[2][2]);
			const float min_g_sample = min4(g[1][1], g[2][1], g[1][2], g[2][2]);
			const float min_b_sample = min4(b[1][1], b[2][1], b[1][2], b[2][2]);
			const float min_a_sample = alpha ? min4(a[1][1], a[2][1], a[1][2], a[2][2]) : 255.f;
			const float max_r_sample = max4(r[1][1], r[2][1], r[1][2], r[2][2]);
			const float max_g_sample = max4(g[1][1], g[2][1], g[1][2], g[2][2]);
			const float max_b_sample = max4(b[1][1], b[2][1], b[1][2], b[2][2]);
			const float max_a_sample = alpha ? max4(a[1][1], a[2][1], a[1][2], a[2][2]) : 255.f;
			const float d_edge = diagonal_edge(Y, wp);
			const float r1 = w1*(r[0][3] + r[3][0]) + w2*(r[1][2] + r[2][1]);
			const float g1 = w1*(g[0][3] + g[3][0]) + w2*(g[1][2] + g[2][1]);
			const float b1 = w1*(b[0][3] + b[3][0]) + w2*(b[1][2] + b[2][1]);
			const float a1 = alpha ? w1*(a[0][3] + a[3][0]) + w2*(a[1][2] + a[2][1]) : 255.f;
			const float r2 = w1*(r[0][0] + r[3][3]) + w2*(r[1][1] + r[2][2]);
			const float g2 = w1*(g[0][0] + g[3][3]) + w2*(g[1][1] + g[2][2]);
			const float b2 = w1*(b[0][0] + b[3][3]) + w2*(b[1][1] + b[2][2]);
			const float a2 = alpha ? w1*(a[0][0] + a[3][3]) + w2*(a[1][1] + a[2][2]) : 255.f;
			// generate and write result
			float rf, gf, bf, af;
			if (d_edge <= 0.0f) { rf = r1; gf = g1; bf = b1; af = a1; }
			else { rf = r2; gf = g2; bf = b2; af = a2; }
			// anti-ringing, clamp.
			rf = clamp(rf, min_r_sample, max_r_sample);
			gf = clamp(gf, min_g_sample, max_g_sample);
			bf = clamp(bf, min_b_sample, max_b_sample);
			af = alpha ? clamp(af, min_a_sample, max_a_sample) : 255.f;
			const int ri = static_cast<int>(ceilf(clamp(rf, 0.f, 255.f)));
			const int gi = static_cast<int>(ceilf(clamp(gf, 0.f, 255.f)));
			const int bi = static_cast<int>(ceilf(clamp(bf, 0.f, 255.f)));
			const int ai = alpha ? static_cast<int>(ceilf(clamp(af, 0.f, 255.f))) : 255;
			out[y*outw + x] = (ai << 24) | (bi << 16) | (gi << 8) | ri;
		}
	}
}

}
