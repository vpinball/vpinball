// original implementation by Sp00kyFox, see https://web.archive.org/web/20160527015550/https://libretro.com/forums/archive/index.php?t-1655.html
// based on the incarnation from back then, the newer libretro implementation (~2018, still same in 2025) is usually worse, as it leads to a more "bubbly" look and also does not track/recreate tilted lines as well

// use via
// template <bool DMD_UPSCALE>
// void upscale(const uint32_t *const data_in, uint32_t *const data_out, const uint32_t xres, const uint32_t yres, const bool is_brightness_data)

#pragma once

#include <cmath>

namespace scalefx
{

#ifndef min
template <typename T> constexpr inline T min(const T x, const T y) { return x < y ? x : y; }
#endif
#ifndef max
template <typename T> constexpr inline T max(const T x, const T y) { return x < y ? y : x; }
#endif

template <typename T> constexpr inline T clamp(const T x, const T mn, const T mx)
{
   return max(min(x,mx),mn);
}

#include "../vector.h" // Copied from VPX

template <bool oneminus_sqrtd3>
static inline float eq_col1(const uint32_t AD, const uint32_t BD)
{
    const float A[3] = { (float)(AD & 255) * (float)(1.0/255.),(float)(AD & 65280) * (float)(1.0 / 65280.0),(float)(AD & 16711680) * (float)(1.0 / 16711680.0) };
    const float B[3] = { (float)(BD & 255) * (float)(1.0/255.),(float)(BD & 65280) * (float)(1.0 / 65280.0),(float)(BD & 16711680) * (float)(1.0 / 16711680.0) };

    const float r = 0.5f * (A[0] + B[0]);
    const float d[3] = { A[0] - B[0], A[1] - B[1], A[2] - B[2] };
    const float tmp = (2.f + r) * d[0] * d[0] + 4.f * d[1] * d[1] + (3.f - r) * d[2] * d[2]; // dot(c*d, d)
    const float res = sqrtf(tmp) * (float)(1.0/3.);
    return oneminus_sqrtd3 ? 1.f - res : res;
}

#if 0
static inline float eq_col2(const uint32_t e1, const uint32_t e2) //!! test vs above, should be the same?
{
    const int rmean = ((int)(e1 & 255) + (int)(e2 & 255)) / 2;
    const int r = (int)(e1 & 255) - (int)(e2 & 255);
    const int g = (int)((e1 >> 8) & 255) - (int)((e2 >> 8) & 255);
    const int b = (int)((e1 >> 16) & 255) - (int)((e2 >> 16) & 255);
    return 1.f - sqrtf((float)((((512 + rmean)*r*r) >> 8) + 4 * g*g + (((767 - rmean)*b*b) >> 8)));
}
#endif

template <bool oneminus>
static inline float eq_brightness(const uint32_t AD, const uint32_t BD)
{
    const float res = (float)abs((int)AD - (int)BD) * (float)(1.0 / 255.);
    return oneminus ? 1.f - res : res;
}

constexpr double SFX_CLR = 0.35;
constexpr float SFX_THR = (float)(1.0 - SFX_CLR);

// corner strength
static inline Vertex4D str(const Vertex4D &crn, const Vertex4D &ort) {
    return {(crn.x > SFX_THR) ? max(2.f * crn.x - (ort.x + ort.w), 0.f) : 0.f,
            (crn.y > SFX_THR) ? max(2.f * crn.y - (ort.y + ort.x), 0.f) : 0.f,
            (crn.z > SFX_THR) ? max(2.f * crn.z - (ort.z + ort.y), 0.f) : 0.f,
            (crn.w > SFX_THR) ? max(2.f * crn.w - (ort.w + ort.z), 0.f) : 0.f};
}

#ifdef NEW_SCALEFX // same as newest implementation from libretro, but usually looks worse, one reason could be porting error(s), but unlikely
constexpr double SFX_CLR2 = 0.5; //0.01..1.00 //org: 0.5
constexpr bool   SFX_SAA = true; // org: true
constexpr bool   SFX_SCN = false; // false in WIP version, true in newest libretro

// corner strength
static inline float str(const float d, const Vertex2D& a, const Vertex2D& b)
{
    const float diff = a.x - a.y;
    const float wght1 = max((float)SFX_CLR2 - d, 0.f) * (float)(1. / SFX_CLR2);
    const float wght2 = clamp((1.f-d) + (min(a.x, b.x) + a.x > min(a.y, b.y) + a.y ? diff : -diff), 0.f, 1.f);
    return (SFX_SAA || 2.f*d < a.x + a.y) ? (wght1 * wght2) * (a.x * a.y) : 0.f;
}

// corner dominance at junctions
static inline Vertex4D dom(const Vertex3Ds& x, const Vertex3Ds& y, const Vertex3Ds& z, const Vertex3Ds& w)
{
    return {2.f*x.y - (x.x + x.z),
            2.f*y.y - (y.x + y.z),
            2.f*z.y - (z.x + z.z),
            2.f*w.y - (w.x + w.z)};
}

static inline bool4 ambi_dom(const Vertex4D &jDx)
{
    return {
       (jDx.x >= 0.f) && (((jDx.y <= 0.f) && (jDx.w <= 0.f)) || (jDx.x + jDx.z >= jDx.y + jDx.w)),
       (jDx.y >= 0.f) && (((jDx.z <= 0.f) && (jDx.x <= 0.f)) || (jDx.y + jDx.w >= jDx.z + jDx.x)),
       (jDx.z >= 0.f) && (((jDx.w <= 0.f) && (jDx.y <= 0.f)) || (jDx.z + jDx.x >= jDx.w + jDx.y)),
       (jDx.w >= 0.f) && (((jDx.x <= 0.f) && (jDx.z <= 0.f)) || (jDx.w + jDx.y >= jDx.x + jDx.z))};
}

// necessary but not sufficient junction condition for orthogonal edges
static inline bool clear(const Vertex2D& crn, const Vertex2D& a, const Vertex2D& b)
{
    return (crn.x >= max(min(a.x, a.y), min(b.x, b.y))) && (crn.y >= max(min(a.x, b.y), min(b.x, a.y)));
}

// DMD_UPSCALE: select between 3x3 upscale, or 1x1 no-scale/anti-aliasing output
// is_brightness_data: select between brightness-only (0..255 per uint32) or full color data (0..255 per R,G,B(,A)-uint32)
template <bool DMD_UPSCALE>
void upscale(const uint32_t * const __restrict data_in, uint32_t * const __restrict data_out, const uint32_t xres, const uint32_t yres, const bool is_brightness_data)
{
    std::vector<Vertex4D> metric(xres*yres); //!! avoid constant reallocs

    uint32_t o = 0;
    if (is_brightness_data)
    {
        for (uint32_t j = 0; j < yres; ++j)
        {
            const uint32_t jm1 = (j - 1)*xres;

            for (uint32_t i = 0; i < xres; ++i, ++o)
            {
                const uint32_t ip1 = i + 1;
                const uint32_t E = data_in[o];
                const uint32_t E2 = (E & 0xFEFEFE) >> 1; // borders = half black/half border pixel
                const uint32_t A = (j == 0) || (i == 0) ? E2 : data_in[jm1 + i - 1];
                const uint32_t B = (j == 0) ? E2 : data_in[jm1 + i];
                const uint32_t C = (j == 0) || (i == xres - 1) ? E2 : data_in[jm1 + ip1];
                const uint32_t F = (i == xres - 1) ? E2 : data_in[j*xres + ip1];

                metric[o] = Vertex4D(eq_brightness<false>(E, A), eq_brightness<false>(E, B), eq_brightness<false>(E, C), eq_brightness<false>(E, F)); //!! only difference
            }
        }
    }
    else
    {
        for (uint32_t j = 0; j < yres; ++j)
        {
            const uint32_t jm1 = (j - 1)*xres;

            for (uint32_t i = 0; i < xres; ++i, ++o)
            {
                const uint32_t ip1 = i + 1;
                const uint32_t E = data_in[o];
                const uint32_t E2 = (E & 0xFEFEFEFE) >> 1; // borders = half black/half border pixel
                const uint32_t A = (j == 0) || (i == 0) ? E2 : data_in[jm1 + i - 1];
                const uint32_t B = (j == 0) ? E2 : data_in[jm1 + i];
                const uint32_t C = (j == 0) || (i == xres - 1) ? E2 : data_in[jm1 + ip1];
                const uint32_t F = (i == xres - 1) ? E2 : data_in[j*xres + ip1];

                metric[o] = Vertex4D(eq_col1<false>(E, A), eq_col1<false>(E, B), eq_col1<false>(E, C), eq_col1<false>(E, F)); //!! only difference
            }
        }
    }

    std::vector<Vertex4D> corner_strength(xres*yres); //!! avoid constant reallocs

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        const uint32_t jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const uint32_t jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;

        for (uint32_t i = 0; i < xres; ++i, ++o)
        {
            const uint32_t im1 = (i == 0) ? 0 : i - 1;
            const uint32_t ip1 = (i == xres - 1) ? xres - 1 : i + 1;

            // read metric data from 1st pass
            const Vertex4D A = metric[jm1 + im1], B = metric[jm1 + i];
            const Vertex4D D = metric[j*xres + im1], E = metric[o], F = metric[j*xres + ip1];
            const Vertex4D G = metric[jp1 + im1], H = metric[jp1 + i], I = metric[jp1 + ip1];

            // output corner strength
            corner_strength[o] = Vertex4D{str(D.z, Vertex2D{D.w, E.y}, Vertex2D{A.w, D.y}),
                                          str(F.x, Vertex2D{E.w, E.y}, Vertex2D{B.w, F.y}),
                                          str(H.z, Vertex2D{E.w, H.y}, Vertex2D{H.w, I.y}),
                                          str(H.x, Vertex2D{D.w, H.y}, Vertex2D{G.w, G.y})};
        }
    }

    std::vector<bool4> g_res (xres*yres*4); //=corner // alloc for all 4 regions together, see below //!! avoid constant reallocs
    bool4* const __restrict g_hori = g_res.data() + xres*yres;
    bool4* const __restrict g_vert = g_res.data() + xres*yres*2;
    bool4* const __restrict g_or   = g_res.data() + xres*yres*3;

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        const uint32_t jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const uint32_t jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;

        for (uint32_t i = 0; i < xres; ++i, ++o)
        {
            const uint32_t im1 = (i == 0) ? 0 : i - 1;
            const uint32_t ip1 = (i == xres - 1) ? xres - 1 : i + 1;

            // read metric data from 1st pass
            const Vertex4D A = metric[jm1 + im1], B = metric[jm1 + i];
            const Vertex4D D = metric[j*xres + im1], E = metric[o], F = metric[j*xres + ip1];
            const Vertex4D G = metric[jp1 + im1], H = metric[jp1 + i], I = metric[jp1 + ip1];

            // strength data
            const Vertex4D As = corner_strength[jm1 + im1], Bs = corner_strength[jm1 + i], Cs = corner_strength[jm1 + ip1];
            const Vertex4D Ds = corner_strength[j*xres + im1], Es = corner_strength[o], Fs = corner_strength[j*xres + ip1];
            const Vertex4D Gs = corner_strength[jp1 + im1], Hs = corner_strength[jp1 + i], Is = corner_strength[jp1 + ip1];

            // strength & dominance junctions
            const Vertex4D jSx{As.z, Bs.w, Es.x, Ds.y}, jDx = dom(Vertex3Ds{As.y,As.z,As.w}, Vertex3Ds{Bs.z,Bs.w,Bs.x}, Vertex3Ds{Es.w,Es.x,Es.y}, Vertex3Ds{Ds.x,Ds.y,Ds.z});
            const Vertex4D jSy{Bs.z, Cs.w, Fs.x, Es.y}, jDy = dom(Vertex3Ds{Bs.y,Bs.z,Bs.w}, Vertex3Ds{Cs.z,Cs.w,Cs.x}, Vertex3Ds{Fs.w,Fs.x,Fs.y}, Vertex3Ds{Es.x,Es.y,Es.z});
            const Vertex4D jSz{Es.z, Fs.w, Is.x, Hs.y}, jDz = dom(Vertex3Ds{Es.y,Es.z,Es.w}, Vertex3Ds{Fs.z,Fs.w,Fs.x}, Vertex3Ds{Is.w,Is.x,Is.y}, Vertex3Ds{Hs.x,Hs.y,Hs.z});
            const Vertex4D jSw{Ds.z, Es.w, Hs.x, Gs.y}, jDw = dom(Vertex3Ds{Ds.y,Ds.z,Ds.w}, Vertex3Ds{Es.z,Es.w,Es.x}, Vertex3Ds{Hs.w,Hs.x,Hs.y}, Vertex3Ds{Gs.x,Gs.y,Gs.z});            

            // majority vote for ambiguous dominance junctions
            const bool4 jx = ambi_dom(jDx);
            const bool4 jy = ambi_dom(jDy);
            const bool4 jz = ambi_dom(jDz);
            const bool4 jw = ambi_dom(jDw);

            // inject strength without creating new contradictions
            const bool4 res{
                     jx.z || (!jx.y && !jx.w && jSx.z >= 0.f && (jx.x || (jSx.x + jSx.z >= jSx.y + jSx.w))),
                     jy.w || (!jy.z && !jy.x && jSy.w >= 0.f && (jy.y || (jSy.y + jSy.w >= jSy.x + jSy.z))),
                     jz.x || (!jz.w && !jz.y && jSz.x >= 0.f && (jz.z || (jSz.x + jSz.z >= jSz.y + jSz.w))),
                     jw.y || (!jw.x && !jw.z && jSw.y >= 0.f && (jw.w || (jSw.y + jSw.w >= jSw.x + jSw.z)))};

            // single pixel & end of line detection (=corner)
            g_res[o] = {
               res.x && (jx.z || !(res.w && res.y)),
               res.y && (jy.w || !(res.x && res.z)),
               res.z && (jz.x || !(res.y && res.w)),
               res.w && (jw.y || !(res.z && res.x))};
            
            // output
            const bool4 clr{
                           clear(Vertex2D{D.z, E.x}, Vertex2D{D.w, E.y}, Vertex2D{A.w, D.y}),
                           clear(Vertex2D{F.x, E.z}, Vertex2D{E.w, E.y}, Vertex2D{B.w, F.y}),
                           clear(Vertex2D{H.z, I.x}, Vertex2D{E.w, H.y}, Vertex2D{H.w, I.y}),
                           clear(Vertex2D{H.x, G.z}, Vertex2D{D.w, H.y}, Vertex2D{G.w, G.y})};

            const Vertex4D h{min(D.w, A.w), min(E.w, B.w), min(E.w, H.w), min(D.w, G.w)};
            const Vertex4D v{min(E.y, D.y), min(E.y, F.y), min(H.y, I.y), min(H.y, G.y)};

            g_or[o] = {
               h.x + D.w >= v.x + E.y,
               h.y + E.w >= v.y + E.y,
               h.z + E.w >= v.z + H.y,
               h.w + D.w >= v.w + H.y}; // orientation
            g_hori[o] = {
               (h.x <= v.x) && clr.x,
               (h.y <= v.y) && clr.y,
               (h.z <= v.z) && clr.z,
               (h.w <= v.w) && clr.w}; // horizontal edges
            g_vert[o] = {
               (h.x >= v.x) && clr.x,
               (h.y >= v.y) && clr.y,
               (h.z >= v.z) && clr.z,
               (h.w >= v.w) && clr.w}; // vertical edges
        }
    }

    // metric & corner strength data no longer needed
    uint32_t* const __restrict metric_tmp = (uint32_t*)metric.data(); // abuse metric buffer for temporary storage

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        const uint32_t jm3 = (j <= 2) ? 0 : (j - 3)*xres;
        const uint32_t jm2 = (j <= 1) ? 0 : (j - 2)*xres;
        const uint32_t jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const uint32_t jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;
        const uint32_t jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;
        const uint32_t jp3 = ((j >= yres - 3) ? yres - 1 : j + 3)*xres;

        for (uint32_t i = 0; i < xres; ++i, ++o)
        {
            const uint32_t im3 = (i <= 2) ? 0 : i - 3;
            const uint32_t im2 = (i <= 1) ? 0 : i - 2;
            const uint32_t im1 = (i == 0) ? 0 : i - 1;
            const uint32_t ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const uint32_t ip2 = (i >= xres - 2) ? xres - 1 : i + 2;
            const uint32_t ip3 = (i >= xres - 3) ? xres - 1 : i + 3;

            // read data
            const uint32_t E = o;
            const uint32_t D = j*xres + im1, D0 = j*xres + im2, D1 = j*xres + im3;
            const uint32_t F = j*xres + ip1, F0 = j*xres + ip2, F1 = j*xres + ip3;
            const uint32_t B = jm1 + i, B0 = jm2 + i, B1 = jm3 + i;
            const uint32_t H = jp1 + i, H0 = jp2 + i, H1 = jp3 + i;

            //!! is this from here on the exact same as original ScaleFX code??

            // extract data
            const bool4 Ec = g_res[E], Eh = g_hori[E], Ev = g_vert[E], Eo = g_or[E];
            const bool4 Dc = g_res[D], Dh = g_hori[D], Do = g_or[D], D0c = g_res[D0], D0h = g_hori[D0], D1h = g_hori[D1];
            const bool4 Fc = g_res[F], Fh = g_hori[F], Fo = g_or[F], F0c = g_res[F0], F0h = g_hori[F0], F1h = g_hori[F1];
            const bool4 Bc = g_res[B], Bv = g_vert[B], Bo = g_or[B], B0c = g_res[B0], B0v = g_vert[B0], B1v = g_vert[B1];
            const bool4 Hc = g_res[H], Hv = g_vert[H], Ho = g_or[H], H0c = g_res[H0], H0v = g_vert[H0], H1v = g_vert[H1];

            // lvl1 corners (hori, vert)
            const bool lvl1x = Ec.x && (Dc.z || Bc.z || SFX_SCN);
            const bool lvl1y = Ec.y && (Fc.w || Bc.w || SFX_SCN);
            const bool lvl1z = Ec.z && (Fc.x || Hc.x || SFX_SCN);
            const bool lvl1w = Ec.w && (Dc.y || Hc.y || SFX_SCN);

            // lvl2 mid (left, right / up, down)
            const bool2 lvl2x{(Ec.x && Eh.y) && Dc.z, (Ec.y && Eh.x) && Fc.w};
            const bool2 lvl2y{(Ec.y && Ev.z) && Bc.w, (Ec.z && Ev.y) && Hc.x};
            const bool2 lvl2z{(Ec.w && Eh.z) && Dc.y, (Ec.z && Eh.w) && Fc.x};
            const bool2 lvl2w{(Ec.x && Ev.w) && Bc.z, (Ec.w && Ev.x) && Hc.y};

            // lvl3 corners (hori, vert)
            const bool2 lvl3x{lvl2x.y && (Dh.y && Dh.x) && Fh.z, lvl2w.y && (Bv.w && Bv.x) && Hv.z};
            const bool2 lvl3y{lvl2x.x && (Fh.x && Fh.y) && Dh.w, lvl2y.y && (Bv.z && Bv.y) && Hv.w};
            const bool2 lvl3z{lvl2z.x && (Fh.w && Fh.z) && Dh.x, lvl2y.x && (Hv.y && Hv.z) && Bv.x};
            const bool2 lvl3w{lvl2z.y && (Dh.z && Dh.w) && Fh.y, lvl2w.x && (Hv.x && Hv.w) && Bv.y};

            // lvl4 corners (hori, vert)
            const bool2 lvl4x{(Dc.x && Dh.y && Eh.x && Eh.y && Fh.x && Fh.y) && (D0c.z && D0h.w), (Bc.x && Bv.w && Ev.x && Ev.w && Hv.x && Hv.w) && (B0c.z && B0v.y)};
            const bool2 lvl4y{(Fc.y && Fh.x && Eh.y && Eh.x && Dh.y && Dh.x) && (F0c.w && F0h.z), (Bc.y && Bv.z && Ev.y && Ev.z && Hv.y && Hv.z) && (B0c.w && B0v.x)};
            const bool2 lvl4z{(Fc.z && Fh.w && Eh.z && Eh.w && Dh.z && Dh.w) && (F0c.x && F0h.y), (Hc.z && Hv.y && Ev.z && Ev.y && Bv.z && Bv.y) && (H0c.x && H0v.w)};
            const bool2 lvl4w{(Dc.w && Dh.z && Eh.w && Eh.z && Fh.w && Fh.z) && (D0c.y && D0h.x), (Hc.w && Hv.x && Ev.w && Ev.x && Bv.w && Bv.x) && (H0c.y && H0v.z)};

            // lvl5 mid (left, right / up, down)
            const bool2 lvl5x{lvl4x.x && (F0h.x && F0h.y) && (D1h.z && D1h.w), lvl4y.x && (D0h.y && D0h.x) && (F1h.w && F1h.z)};
            const bool2 lvl5y{lvl4y.y && (H0v.y && H0v.z) && (B1v.w && B1v.x), lvl4z.y && (B0v.z && B0v.y) && (H1v.x && H1v.w)};
            const bool2 lvl5z{lvl4w.x && (F0h.w && F0h.z) && (D1h.y && D1h.x), lvl4z.x && (D0h.z && D0h.w) && (F1h.x && F1h.y)};
            const bool2 lvl5w{lvl4x.y && (H0v.x && H0v.w) && (B1v.z && B1v.y), lvl4w.y && (B0v.w && B0v.x) && (H1v.y && H1v.z)};

            // lvl6 corners (hori, vert)
            const bool2 lvl6x{lvl5x.y && (D1h.y && D1h.x), lvl5w.y && (B1v.w && B1v.x)};
            const bool2 lvl6y{lvl5x.x && (F1h.x && F1h.y), lvl5y.y && (B1v.z && B1v.y)};
            const bool2 lvl6z{lvl5z.x && (F1h.w && F1h.z), lvl5y.x && (H1v.y && H1v.z)};
            const bool2 lvl6w{lvl5z.y && (D1h.z && D1h.w), lvl5w.x && (H1v.x && H1v.w)};

            // subpixels - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
            const uint32_t crn_x = (lvl1x && Eo.x || lvl3x.x && Eo.y || lvl4x.x && Do.x || lvl6x.x && Fo.y) ? 5
               : (lvl1x || lvl3x.y && !Eo.w || lvl4x.y && !Bo.x || lvl6x.y && !Ho.w)         ? 1
               : lvl3x.x                                                                     ? 3
               : lvl3x.y                                                                     ? 7
               : lvl4x.x                                                                     ? 2
               : lvl4x.y                                                                     ? 6
               : lvl6x.x                                                                     ? 4
               : lvl6x.y                                                                     ? 8
                                                                                             : 0;
            const uint32_t crn_y = (lvl1y && Eo.y || lvl3y.x && Eo.x || lvl4y.x && Fo.y || lvl6y.x && Do.x) ? 5
               : (lvl1y || lvl3y.y && !Eo.z || lvl4y.y && !Bo.y || lvl6y.y && !Ho.z)         ? 3
               : lvl3y.x                                                                     ? 1
               : lvl3y.y                                                                     ? 7
               : lvl4y.x                                                                     ? 4
               : lvl4y.y                                                                     ? 6
               : lvl6y.x                                                                     ? 2
               : lvl6y.y                                                                     ? 8
                                                                                             : 0;
            const uint32_t crn_z = (lvl1z && Eo.z || lvl3z.x && Eo.w || lvl4z.x && Fo.z || lvl6z.x && Do.w) ? 7
               : (lvl1z || lvl3z.y && !Eo.y || lvl4z.y && !Ho.z || lvl6z.y && !Bo.y)         ? 3
               : lvl3z.x                                                                     ? 1
               : lvl3z.y                                                                     ? 5
               : lvl4z.x                                                                     ? 4
               : lvl4z.y                                                                     ? 8
               : lvl6z.x                                                                     ? 2
               : lvl6z.y                                                                     ? 6
                                                                                             : 0;
            const uint32_t crn_w = (lvl1w && Eo.w || lvl3w.x && Eo.z || lvl4w.x && Do.w || lvl6w.x && Fo.z) ? 7
               : (lvl1w || lvl3w.y && !Eo.x || lvl4w.y && !Ho.w || lvl6w.y && !Bo.x)         ? 1
               : lvl3w.x                                                                     ? 3
               : lvl3w.y                                                                     ? 5
               : lvl4w.x                                                                     ? 2
               : lvl4w.y                                                                     ? 8
               : lvl6w.x                                                                     ? 4
               : lvl6w.y                                                                     ? 6
                                                                                             : 0;

            const uint32_t mid_x = (lvl2x.x && Eo.x || lvl2x.y && Eo.y || lvl5x.x && Do.x || lvl5x.y && Fo.y) ? 5
               : lvl2x.x                                                                       ? 1
               : lvl2x.y                                                                       ? 3
               : lvl5x.x                                                                       ? 2
               : lvl5x.y                                                                       ? 4
               : (Ec.x && Dc.z && Ec.y && Fc.w)                                                ? (Eo.x ? Eo.y ? 5 : 3 : 1)
                                                                                               : 0;
            const uint32_t mid_y = (lvl2y.x && !Eo.y || lvl2y.y && !Eo.z || lvl5y.x && !Bo.y || lvl5y.y && !Ho.z) ? 3
               : lvl2y.x                                                                           ? 5
               : lvl2y.y                                                                           ? 7
               : lvl5y.x                                                                           ? 6
               : lvl5y.y                                                                           ? 8
               : (Ec.y && Bc.w && Ec.z && Hc.x)                                                    ? (!Eo.y ? !Eo.z ? 3 : 7 : 5)
                                                                                                   : 0;
            const uint32_t mid_z = (lvl2z.x && Eo.w || lvl2z.y && Eo.z || lvl5z.x && Do.w || lvl5z.y && Fo.z) ? 7
               : lvl2z.x                                                                       ? 1
               : lvl2z.y                                                                       ? 3
               : lvl5z.x                                                                       ? 2
               : lvl5z.y                                                                       ? 4
               : (Ec.z && Fc.x && Ec.w && Dc.y)                                                ? (Eo.z ? Eo.w ? 7 : 1 : 3)
                                                                                               : 0;
            const uint32_t mid_w = (lvl2w.x && !Eo.x || lvl2w.y && !Eo.w || lvl5w.x && !Bo.x || lvl5w.y && !Ho.w) ? 1
               : lvl2w.x                                                                           ? 5
               : lvl2w.y                                                                           ? 7
               : lvl5w.x                                                                           ? 6
               : lvl5w.y                                                                           ? 8
               : (Ec.w && Hc.y && Ec.x && Bc.z)                                                    ? (!Eo.w ? !Eo.x ? 1 : 5 : 7)
                                                                                                   : 0;
            // ouput
            metric_tmp[o] = crn_x | (crn_y << 4) | (crn_z << 8) | (crn_w << 12) | (mid_x << 16) | (mid_y << 20) | (mid_z << 24) | (mid_w << 28);
        }
    }

    //memcpy(g_or, data_in, xres*yres * sizeof(uint32_t)); // must be done if data_in == data_out, and then replace data_in's below with g_or's .xyzw

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        uint32_t offs = j*(xres*9); // only used if DMD_UPSCALE
        for (uint32_t i = 0; i < xres; ++i, ++o,offs += 3)
        {
            const uint32_t tmp = metric_tmp[o];
            const uint32_t crn_x = tmp & 15;
            const uint32_t crn_y = (tmp >> 4) & 15;
            const uint32_t crn_z = (tmp >> 8) & 15;
            const uint32_t crn_w = (tmp >> 12) & 15;

            const uint32_t mid_x = (tmp >> 16) & 15;
            const uint32_t mid_y = (tmp >> 20) & 15;
            const uint32_t mid_z = (tmp >> 24) & 15;
            const uint32_t mid_w = (tmp >> 28);

            uint32_t r = 0, g = 0, b = 0, a = 0; // only used if !DMD_UPSCALE

            uint32_t o2 = 0;
            for (uint32_t j2 = 0; j2 < 3; ++j2)
                for (uint32_t i2 = 0; i2 < 3; ++i2,++o2)
                {
                    // determine subpixel
                    uint32_t sp = 0;
                    switch (o2)
                    {
                    case 0: sp = crn_x; break;
                    case 1: sp = mid_x; break;
                    case 2: sp = crn_y; break;
                    case 3: sp = mid_w; break;
                    case 4: sp = 0;     break;
                    case 5: sp = mid_y; break;
                    case 6: sp = crn_w; break;
                    case 7: sp = mid_z; break;
                    case 8: sp = crn_z; break;
                    }

                    // output coordinate - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
                    uint32_t res = 0;
                    switch (sp)
                    {
                    case 0: res = 0; break;
                    case 1: res = ((i == 0) ? 0xdeadbeefu : -1); break;
                    case 2: res = ((i <= 1) ? 0xdeadbeefu : -2); break;
                    case 3: res = ((i == xres - 1) ? 0xdeadbeefu : 1); break;
                    case 4: res = ((i >= xres - 2) ? 0xdeadbeefu : 2); break;
                    case 5: res = ((j == 0) ? 0xdeadbeefu : (uint32_t)(-(int)xres)); break;
                    case 6: res = ((j <= 1) ? 0xdeadbeefu : -2 * xres); break;
                    case 7: res = ((j == yres - 1) ? 0xdeadbeefu : xres); break;
                    case 8: res = ((j >= yres - 2) ? 0xdeadbeefu : 2 * xres); break;
                    }

                    const uint32_t tmp2 = (res == 0xdeadbeefu) ? ((data_in[o] & 0xFEFEFEFE)>>1) : data_in[o+res]; // borders = half black/half border pixel
                    if (DMD_UPSCALE)
                    data_out[offs + j2*(xres*3) + i2] = tmp2;
                    else {
                    r +=  tmp2      & 255;
                    g += (tmp2>>8)  & 255;
                    b += (tmp2>>16) & 255;
                    a +=  tmp2>>24;
                    }
                }

            if (!DMD_UPSCALE) {
            r /= 9;
            g /= 9;
            b /= 9;
            a /= 9;
            data_out[o] = r | (g << 8) | (b << 16) | (a << 24);
            }
        }
    }
}

#else

// corner dominance at junctions
static inline Vertex4D dom(const Vertex4D &strx, const Vertex4D &stry, const Vertex4D &strz, const Vertex4D &strw) {
    return {max(2.f * strx.z - (strx.y + strx.w), 0.f),
            max(2.f * stry.w - (stry.z + stry.x), 0.f),
            max(2.f * strz.x - (strz.w + strz.y), 0.f),
            max(2.f * strw.y - (strw.x + strw.z), 0.f)};
}

// necessary but not sufficient junction condition for orthogonal edges
static inline bool clear(const Vertex2D &crn, const Vertex4D &ort) {
    return (crn.x <= SFX_THR || crn.x <= ort.x || crn.x <= ort.w) &&
           (crn.y <= SFX_THR || crn.y <= ort.y || crn.y <= ort.x) &&
           (crn.x <= SFX_THR || crn.x <= ort.z || crn.x <= ort.y) &&
           (crn.y <= SFX_THR || crn.y <= ort.w || crn.y <= ort.z);
}

static inline bool4 ambi_dom(const Vertex4D &jDx)
{
    return {
        jDx.x != 0.f && jDx.x + jDx.z > jDx.y + jDx.w,
        jDx.y != 0.f && jDx.y + jDx.w > jDx.z + jDx.x,
        jDx.z != 0.f && jDx.z + jDx.x > jDx.w + jDx.y,
        jDx.w != 0.f && jDx.w + jDx.y > jDx.x + jDx.z};
}

// DMD_UPSCALE: select between 3x3 upscale, or 1x1 no-scale/anti-aliasing output
// is_brightness_data: select between brightness-only (0..255 per uint32) or full color data (0..255 per R,G,B(,A)-uint32)
template <bool DMD_UPSCALE>
void upscale(const uint32_t * const __restrict data_in, uint32_t * const __restrict data_out, const uint32_t xres, const uint32_t yres, const bool is_brightness_data)
{
    std::vector<Vertex4D> metric(xres*yres); //!! avoid constant reallocs?

    uint32_t o = 0;
    if (is_brightness_data)
    {
        for (uint32_t j = 0; j < yres; ++j)
        {
            const uint32_t jm1 = (j - 1)*xres;

            for (uint32_t i = 0; i < xres; ++i, ++o)
            {
                const uint32_t ip1 = i + 1;
                const uint32_t E = data_in[o];
                const uint32_t E2 = (E & 0xFEFEFE) >> 1; // borders = half black/half border pixel
                const uint32_t A = (j == 0) || (i == 0) ? E2 : data_in[jm1 + i - 1];
                const uint32_t B = (j == 0) ? E2 : data_in[jm1 + i];
                const uint32_t C = (j == 0) || (i == xres - 1) ? E2 : data_in[jm1 + ip1];
                const uint32_t F = (i == xres - 1) ? E2 : data_in[j*xres + ip1];

                metric[o] = Vertex4D(eq_brightness<true>(E, A), eq_brightness<true>(E, B), eq_brightness<true>(E, C), eq_brightness<true>(E, F));
            }
        }
    }
    else
    {
        for (uint32_t j = 0; j < yres; ++j)
        {
            const uint32_t jm1 = (j - 1)*xres;

            for (uint32_t i = 0; i < xres; ++i, ++o)
            {
                const uint32_t ip1 = i + 1;
                const uint32_t E = data_in[o];
                const uint32_t E2 = (E & 0xFEFEFEFE) >> 1; // borders = half black/half border pixel
                const uint32_t A = (j == 0) || (i == 0) ? E2 : data_in[jm1 + i - 1];
                const uint32_t B = (j == 0) ? E2 : data_in[jm1 + i];
                const uint32_t C = (j == 0) || (i == xres - 1) ? E2 : data_in[jm1 + ip1];
                const uint32_t F = (i == xres - 1) ? E2 : data_in[j*xres + ip1];

                metric[o] = Vertex4D(eq_col1<true>(E, A), eq_col1<true>(E, B), eq_col1<true>(E, C), eq_col1<true>(E, F));
            }
        }
    }

    std::vector<bool4> g_res (xres*yres*4); // alloc for all 4 regions together, see below //!! avoid constant reallocs?
    bool4* const __restrict g_hori = g_res.data() + xres*yres;
    bool4* const __restrict g_vert = g_res.data() + xres*yres*2;
    bool4* const __restrict g_or   = g_res.data() + xres*yres*3;

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        const uint32_t jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const uint32_t jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;
        const uint32_t jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;

        for (uint32_t i = 0; i < xres; ++i, ++o)
        {
            const uint32_t im2 = (i <= 1) ? 0 : i - 2;
            const uint32_t im1 = (i == 0) ? 0 : i - 1;
            const uint32_t ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const uint32_t ip2 = (i >= xres - 2) ? xres - 1 : i + 2;

            const Vertex4D M = metric[jm1 + im2];
            const Vertex4D A = metric[jm1 + im1];
            const Vertex4D B = metric[jm1 + i];
            const Vertex4D C = metric[jm1 + ip1];
            const Vertex4D P = metric[jm1 + ip2];

            const Vertex4D N = metric[j*xres + im2];
            const Vertex4D D = metric[j*xres + im1];
            const Vertex4D E = metric[o];
            const Vertex4D F = metric[j*xres + ip1];
            const Vertex4D Q = metric[j*xres + ip2];

            const Vertex4D O = metric[jp1 + im2];
            const Vertex4D G = metric[jp1 + im1];
            const Vertex4D H = metric[jp1 + i];
            const Vertex4D I = metric[jp1 + ip1];
            const Vertex4D R = metric[jp1 + ip2];

            const Vertex4D J = metric[jp2 + im1];
            const Vertex4D K = metric[jp2 + i];
            const Vertex4D L = metric[jp2 + ip1];

            const Vertex4D As = str(Vertex4D(M.z, B.x, D.z, D.x), Vertex4D(A.y, A.w, D.y, M.w));
            const Vertex4D Bs = str(Vertex4D(A.z, C.x, E.z, E.x), Vertex4D(B.y, B.w, E.y, A.w));
            const Vertex4D Cs = str(Vertex4D(B.z, P.x, F.z, F.x), Vertex4D(C.y, C.w, F.y, B.w));
            const Vertex4D Ds = str(Vertex4D(N.z, E.x, G.z, G.x), Vertex4D(D.y, D.w, G.y, N.w));
            const Vertex4D Es = str(Vertex4D(D.z, F.x, H.z, H.x), Vertex4D(E.y, E.w, H.y, D.w));
            const Vertex4D Fs = str(Vertex4D(E.z, Q.x, I.z, I.x), Vertex4D(F.y, F.w, I.y, E.w));
            const Vertex4D Gs = str(Vertex4D(O.z, H.x, J.z, J.x), Vertex4D(G.y, G.w, J.y, O.w));
            const Vertex4D Hs = str(Vertex4D(G.z, I.x, K.z, K.x), Vertex4D(H.y, H.w, K.y, G.w));
            const Vertex4D Is = str(Vertex4D(H.z, R.x, L.z, L.x), Vertex4D(I.y, I.w, L.y, H.w));

            // strength & dominance junctions
            const Vertex4D jSx = Vertex4D(As.z, Bs.w, Es.x, Ds.y), jDx = dom(As, Bs, Es, Ds);
            const Vertex4D jSy = Vertex4D(Bs.z, Cs.w, Fs.x, Es.y), jDy = dom(Bs, Cs, Fs, Es);
            const Vertex4D jSz = Vertex4D(Es.z, Fs.w, Is.x, Hs.y), jDz = dom(Es, Fs, Is, Hs);
            const Vertex4D jSw = Vertex4D(Ds.z, Es.w, Hs.x, Gs.y), jDw = dom(Ds, Es, Hs, Gs);

            // majority vote for ambiguous dominance junctions
            const bool4 jx = ambi_dom(jDx);
            const bool4 jy = ambi_dom(jDy);
            const bool4 jz = ambi_dom(jDz);
            const bool4 jw = ambi_dom(jDw);

            // inject strength without creating new contradictions
            bool4 res;
            res.x = jx.z || (!(jx.y || jx.w) && (jSx.z != 0.f) && (jx.x || (jSx.x + jSx.z > jSx.y + jSx.w)));
            res.y = jy.w || (!(jy.z || jy.x) && (jSy.w != 0.f) && (jy.y || (jSy.y + jSy.w > jSy.x + jSy.z)));
            res.z = jz.x || (!(jz.w || jz.y) && (jSz.x != 0.f) && (jz.z || (jSz.x + jSz.z > jSz.y + jSz.w)));
            res.w = jw.y || (!(jw.x || jw.z) && (jSw.y != 0.f) && (jw.w || (jSw.y + jSw.w > jSw.x + jSw.z)));

            // single pixel & end of line detection
            res.x = res.x && (jx.z || !(res.w && res.y));
            res.y = res.y && (jy.w || !(res.x && res.z));
            res.z = res.z && (jz.x || !(res.y && res.w));
            res.w = res.w && (jw.y || !(res.z && res.x));

            // output
            const bool4 clr(
                clear(Vertex2D(D.z, E.x), Vertex4D(A.w, E.y, D.w, D.y)),
                clear(Vertex2D(E.z, F.x), Vertex4D(B.w, F.y, E.w, E.y)),
                clear(Vertex2D(H.z, I.x), Vertex4D(E.w, I.y, H.w, H.y)),
                clear(Vertex2D(G.z, H.x), Vertex4D(D.w, H.y, G.w, G.y)));

            const Vertex4D low(max(E.y, SFX_THR), max(E.w, SFX_THR), max(H.y, SFX_THR), max(D.w, SFX_THR));

            const bool4 hori = bool4(low.x < max(D.w, A.w) && clr.x, low.x < max(E.w, B.w) && clr.y, low.z < max(E.w, H.w) && clr.z, low.z < max(D.w, G.w) && clr.w); // horizontal edges
            const bool4 vert = bool4(low.w < max(E.y, D.y) && clr.x, low.y < max(E.y, F.y) && clr.y, low.y < max(H.y, I.y) && clr.z, low.w < max(H.y, G.y) && clr.w); // vertical edges
            const bool4 ori = bool4(A.w < D.y, B.w <= F.y, H.w < I.y, G.w <= G.y);							                              // orientation

            g_res[o] = res;
            g_hori[o] = hori;
            g_vert[o] = vert;
            g_or[o] = ori;
        }
    }

    uint32_t* const __restrict metric_tmp = (uint32_t*)metric.data(); // abuse metric buffer for temporary storage

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        const uint32_t jm3 = (j <= 2) ? 0 : (j - 3)*xres;
        const uint32_t jm2 = (j <= 1) ? 0 : (j - 2)*xres;
        const uint32_t jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const uint32_t jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;
        const uint32_t jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;
        const uint32_t jp3 = ((j >= yres - 3) ? yres - 1 : j + 3)*xres;

        for (uint32_t i = 0; i < xres; ++i, ++o)
        {
            const uint32_t im3 = (i <= 2) ? 0 : i - 3;
            const uint32_t im2 = (i <= 1) ? 0 : i - 2;
            const uint32_t im1 = (i == 0) ? 0 : i - 1;
            const uint32_t ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const uint32_t ip2 = (i >= xres - 2) ? xres - 1 : i + 2;
            const uint32_t ip3 = (i >= xres - 3) ? xres - 1 : i + 3;

            // read data
            const uint32_t E = o;
            const uint32_t D = j*xres + im1, D0 = j*xres + im2, D1 = j*xres + im3;
            const uint32_t F = j*xres + ip1, F0 = j*xres + ip2, F1 = j*xres + ip3;
            const uint32_t B = jm1 + i, B0 = jm2 + i, B1 = jm3 + i;
            const uint32_t H = jp1 + i, H0 = jp2 + i, H1 = jp3 + i;

            // extract data
            const bool4 Ec = g_res[E], Eh = g_hori[E], Ev = g_vert[E], Eo = g_or[E];
            const bool4 Dc = g_res[D], Dh = g_hori[D], Do = g_or[D], D0c = g_res[D0], D0h = g_hori[D0], D1h = g_hori[D1];
            const bool4 Fc = g_res[F], Fh = g_hori[F], Fo = g_or[F], F0c = g_res[F0], F0h = g_hori[F0], F1h = g_hori[F1];
            const bool4 Bc = g_res[B], Bv = g_vert[B], Bo = g_or[B], B0c = g_res[B0], B0v = g_vert[B0], B1v = g_vert[B1];
            const bool4 Hc = g_res[H], Hv = g_vert[H], Ho = g_or[H], H0c = g_res[H0], H0v = g_vert[H0], H1v = g_vert[H1];

            // lvl2 mid (left, right / up, down)
            const bool2 lvl2x{(Ec.x && Eh.y) && Dc.z, (Ec.y && Eh.x) && Fc.w};
            const bool2 lvl2y{(Ec.y && Ev.z) && Bc.w, (Ec.z && Ev.y) && Hc.x};
            const bool2 lvl2z{(Ec.w && Eh.z) && Dc.y, (Ec.z && Eh.w) && Fc.x};
            const bool2 lvl2w{(Ec.x && Ev.w) && Bc.z, (Ec.w && Ev.x) && Hc.y};

            // lvl3 corners (hori, vert)
            const bool2 lvl3x{lvl2x.y && (Dh.y && Dh.x) && Fh.z, lvl2w.y && (Bv.w && Bv.x) && Hv.z};
            const bool2 lvl3y{lvl2x.x && (Fh.x && Fh.y) && Dh.w, lvl2y.y && (Bv.z && Bv.y) && Hv.w};
            const bool2 lvl3z{lvl2z.x && (Fh.w && Fh.z) && Dh.x, lvl2y.x && (Hv.y && Hv.z) && Bv.x};
            const bool2 lvl3w{lvl2z.y && (Dh.z && Dh.w) && Fh.y, lvl2w.x && (Hv.x && Hv.w) && Bv.y};

            // lvl4 corners (hori, vert)
            const bool2 lvl4x{(Dc.x && Dh.y && Eh.x && Eh.y && Fh.x && Fh.y) && (D0c.z && D0h.w), (Bc.x && Bv.w && Ev.x && Ev.w && Hv.x && Hv.w) && (B0c.z && B0v.y)};
            const bool2 lvl4y{(Fc.y && Fh.x && Eh.y && Eh.x && Dh.y && Dh.x) && (F0c.w && F0h.z), (Bc.y && Bv.z && Ev.y && Ev.z && Hv.y && Hv.z) && (B0c.w && B0v.x)};
            const bool2 lvl4z{(Fc.z && Fh.w && Eh.z && Eh.w && Dh.z && Dh.w) && (F0c.x && F0h.y), (Hc.z && Hv.y && Ev.z && Ev.y && Bv.z && Bv.y) && (H0c.x && H0v.w)};
            const bool2 lvl4w{(Dc.w && Dh.z && Eh.w && Eh.z && Fh.w && Fh.z) && (D0c.y && D0h.x), (Hc.w && Hv.x && Ev.w && Ev.x && Bv.w && Bv.x) && (H0c.y && H0v.z)};

            // lvl5 mid (left, right / up, down)
            const bool2 lvl5x{lvl4x.x && (F0h.x && F0h.y) && (D1h.z && D1h.w), lvl4y.x && (D0h.y && D0h.x) && (F1h.w && F1h.z)};
            const bool2 lvl5y{lvl4y.y && (H0v.y && H0v.z) && (B1v.w && B1v.x), lvl4z.y && (B0v.z && B0v.y) && (H1v.x && H1v.w)};
            const bool2 lvl5z{lvl4w.x && (F0h.w && F0h.z) && (D1h.y && D1h.x), lvl4z.x && (D0h.z && D0h.w) && (F1h.x && F1h.y)};
            const bool2 lvl5w{lvl4x.y && (H0v.x && H0v.w) && (B1v.z && B1v.y), lvl4w.y && (B0v.w && B0v.x) && (H1v.y && H1v.z)};

            // lvl6 corners (hori, vert)
            const bool2 lvl6x{lvl5x.y && (D1h.y && D1h.x), lvl5w.y && (B1v.w && B1v.x)};
            const bool2 lvl6y{lvl5x.x && (F1h.x && F1h.y), lvl5y.y && (B1v.z && B1v.y)};
            const bool2 lvl6z{lvl5z.x && (F1h.w && F1h.z), lvl5y.x && (H1v.y && H1v.z)};
            const bool2 lvl6w{lvl5z.y && (D1h.z && D1h.w), lvl5w.x && (H1v.x && H1v.w)};

            // subpixels - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
            const uint32_t crn_x = ((Ec.x && Eo.x) || (lvl3x.x && Eo.y) || (lvl4x.x && Do.x) || (lvl6x.x && Fo.y)) ? 5 : (Ec.x || (lvl3x.y && !Eo.w) || (lvl4x.y && !Bo.x) || (lvl6x.y && !Ho.w)) ? 1 : lvl3x.x ? 3 : lvl3x.y ? 7 : lvl4x.x ? 2 : lvl4x.y ? 6 : lvl6x.x ? 4 : lvl6x.y ? 8 : 0;
            const uint32_t crn_y = ((Ec.y && Eo.y) || (lvl3y.x && Eo.x) || (lvl4y.x && Fo.y) || (lvl6y.x && Do.x)) ? 5 : (Ec.y || (lvl3y.y && !Eo.z) || (lvl4y.y && !Bo.y) || (lvl6y.y && !Ho.z)) ? 3 : lvl3y.x ? 1 : lvl3y.y ? 7 : lvl4y.x ? 4 : lvl4y.y ? 6 : lvl6y.x ? 2 : lvl6y.y ? 8 : 0;
            const uint32_t crn_z = ((Ec.z && Eo.z) || (lvl3z.x && Eo.w) || (lvl4z.x && Fo.z) || (lvl6z.x && Do.w)) ? 7 : (Ec.z || (lvl3z.y && !Eo.y) || (lvl4z.y && !Ho.z) || (lvl6z.y && !Bo.y)) ? 3 : lvl3z.x ? 1 : lvl3z.y ? 5 : lvl4z.x ? 4 : lvl4z.y ? 8 : lvl6z.x ? 2 : lvl6z.y ? 6 : 0;
            const uint32_t crn_w = ((Ec.w && Eo.w) || (lvl3w.x && Eo.z) || (lvl4w.x && Do.w) || (lvl6w.x && Fo.z)) ? 7 : (Ec.w || (lvl3w.y && !Eo.x) || (lvl4w.y && !Ho.w) || (lvl6w.y && !Bo.x)) ? 1 : lvl3w.x ? 3 : lvl3w.y ? 5 : lvl4w.x ? 2 : lvl4w.y ? 8 : lvl6w.x ? 4 : lvl6w.y ? 6 : 0;

            const uint32_t mid_x = ((lvl2x.x &&  Eo.x) || (lvl2x.y &&  Eo.y) || (lvl5x.x &&  Do.x) || (lvl5x.y &&  Fo.y)) ? 5 : lvl2x.x ? 1 : lvl2x.y ? 3 : lvl5x.x ? 2 : lvl5x.y ? 4 : (Ec.x && Dc.z && Ec.y && Fc.w) ? ( Eo.x ?  Eo.y ? 5 : 3 : 1) : 0;
            const uint32_t mid_y = ((lvl2y.x && !Eo.y) || (lvl2y.y && !Eo.z) || (lvl5y.x && !Bo.y) || (lvl5y.y && !Ho.z)) ? 3 : lvl2y.x ? 5 : lvl2y.y ? 7 : lvl5y.x ? 6 : lvl5y.y ? 8 : (Ec.y && Bc.w && Ec.z && Hc.x) ? (!Eo.y ? !Eo.z ? 3 : 7 : 5) : 0;
            const uint32_t mid_z = ((lvl2z.x &&  Eo.w) || (lvl2z.y &&  Eo.z) || (lvl5z.x &&  Do.w) || (lvl5z.y &&  Fo.z)) ? 7 : lvl2z.x ? 1 : lvl2z.y ? 3 : lvl5z.x ? 2 : lvl5z.y ? 4 : (Ec.z && Fc.x && Ec.w && Dc.y) ? ( Eo.z ?  Eo.w ? 7 : 1 : 3) : 0;
            const uint32_t mid_w = ((lvl2w.x && !Eo.x) || (lvl2w.y && !Eo.w) || (lvl5w.x && !Bo.x) || (lvl5w.y && !Ho.w)) ? 1 : lvl2w.x ? 5 : lvl2w.y ? 7 : lvl5w.x ? 6 : lvl5w.y ? 8 : (Ec.w && Hc.y && Ec.x && Bc.z) ? (!Eo.w ? !Eo.x ? 1 : 5 : 7) : 0;

            // ouput
            metric_tmp[o] = crn_x | (crn_y << 4) | (crn_z << 8) | (crn_w << 12) | (mid_x << 16) | (mid_y << 20) | (mid_z << 24) | (mid_w << 28);
        }
    }

    //memcpy(g_or, data_in, xres*yres * sizeof(uint32_t)); // must be done if data_in == data_out, and then replace data_in's below with g_or's .xyzw

    o = 0;
    for (uint32_t j = 0; j < yres; ++j)
    {
        uint32_t offs = j*(xres*9); // only used if DMD_UPSCALE
        for (uint32_t i = 0; i < xres; ++i, ++o,offs += 3)
        {
            const uint32_t tmp = metric_tmp[o];
            const uint32_t crn_x = tmp & 15;
            const uint32_t crn_y = (tmp >> 4) & 15;
            const uint32_t crn_z = (tmp >> 8) & 15;
            const uint32_t crn_w = (tmp >> 12) & 15;

            const uint32_t mid_x = (tmp >> 16) & 15;
            const uint32_t mid_y = (tmp >> 20) & 15;
            const uint32_t mid_z = (tmp >> 24) & 15;
            const uint32_t mid_w = (tmp >> 28);

            uint32_t r = 0, g = 0, b = 0, a = 0; // only used if !DMD_UPSCALE

            uint32_t o2 = 0;
            for (uint32_t j2 = 0; j2 < 3; ++j2)
                for (uint32_t i2 = 0; i2 < 3; ++i2,++o2)
                {
                    // determine subpixel
                    uint32_t sp = 0;
                    switch (o2)
                    {
                    case 0: sp = crn_x; break;
                    case 1: sp = mid_x; break;
                    case 2: sp = crn_y; break;
                    case 3: sp = mid_w; break;
                    case 4: sp = 0;     break;
                    case 5: sp = mid_y; break;
                    case 6: sp = crn_w; break;
                    case 7: sp = mid_z; break;
                    case 8: sp = crn_z; break;
                    }

                    // output coordinate - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
                    uint32_t res = 0;
                    switch (sp)
                    {
                    case 0: res = 0; break;
                    case 1: res = ((i == 0) ? 0xdeadbeefu : -1); break;
                    case 2: res = ((i <= 1) ? 0xdeadbeefu : -2); break;
                    case 3: res = ((i == xres - 1) ? 0xdeadbeefu : 1); break;
                    case 4: res = ((i >= xres - 2) ? 0xdeadbeefu : 2); break;
                    case 5: res = ((j == 0) ? 0xdeadbeefu : (uint32_t)(-(int)xres)); break;
                    case 6: res = ((j <= 1) ? 0xdeadbeefu : -2 * xres); break;
                    case 7: res = ((j == yres - 1) ? 0xdeadbeefu : xres); break;
                    case 8: res = ((j >= yres - 2) ? 0xdeadbeefu : 2 * xres); break;
                    }

                    const uint32_t tmp2 = (res == 0xdeadbeefu) ? ((data_in[o] & 0xFEFEFEFE)>>1) : data_in[o+res]; // borders = half black/half border pixel
                    if (DMD_UPSCALE)
                    data_out[offs + j2*(xres*3) + i2] = tmp2;
                    else {
                    r +=  tmp2      & 255;
                    g += (tmp2>>8)  & 255;
                    b += (tmp2>>16) & 255;
                    a +=  tmp2>>24;
                    }
                }

            if (!DMD_UPSCALE) {
            r /= 9;
            g /= 9;
            b /= 9;
            a /= 9;
            data_out[o] = r | (g << 8) | (b << 16) | (a << 24);
            }
        }
    }
}

#endif

}
