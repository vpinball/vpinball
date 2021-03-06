static inline float eq_col1(const DWORD AD, const DWORD BD)
{
    const float A[3] = { (float)(AD & 255) * (float)(1.0/255.),(float)(AD & 65280) * (float)(1.0 / 65280.0),(float)(AD & 16711680) * (float)(1.0 / 16711680.0) };
    const float B[3] = { (float)(BD & 255) * (float)(1.0/255.),(float)(BD & 65280) * (float)(1.0 / 65280.0),(float)(BD & 16711680) * (float)(1.0 / 16711680.0) };

    const float r = 0.5f * (A[0] + B[0]);
    const float d[3] = { A[0] - B[0], A[1] - B[1], A[2] - B[2] };
    const float tmp = (2.f + r) * d[0] * d[0] + 4.f * d[1] * d[1] + (3.f - r) * d[2] * d[2]; // dot(c*d, d)
    return 1.f - sqrtf(tmp) * (float)(1.0/3.);
}

#if 0
static inline float eq_col2(const DWORD e1, const DWORD e2) //!! test vs above, should be the same?
{
    const long rmean = ((long)(e1 & 255) + (long)(e2 & 255)) / 2;
    const long r = (long)(e1 & 255) - (long)(e2 & 255);
    const long g = (long)((e1 >> 8) & 255) - (long)((e2 >> 8) & 255);
    const long b = (long)((e1 >> 16) & 255) - (long)((e2 >> 16) & 255);
    return 1.f - sqrtf((float)((((512 + rmean)*r*r) >> 8) + 4 * g*g + (((767 - rmean)*b*b) >> 8)));
}
#endif

static inline float eq_brightness(const DWORD AD, const DWORD BD)
{
    return 1.f - (float)abs((int)AD - (int)BD) * (float)(1.0 / 100.);
}

static const double SFX_CLR = 0.35;
static const float SFX_THR = (float)(1.0 - SFX_CLR);

// corner strength
static inline Vertex4D str(const Vertex4D &crn, const Vertex4D &ort) {
    return Vertex4D((crn.x > SFX_THR) ? max(2.f * crn.x - (ort.x + ort.w), 0.f) : 0.f,
                    (crn.y > SFX_THR) ? max(2.f * crn.y - (ort.y + ort.x), 0.f) : 0.f,
                    (crn.z > SFX_THR) ? max(2.f * crn.z - (ort.z + ort.y), 0.f) : 0.f,
                    (crn.w > SFX_THR) ? max(2.f * crn.w - (ort.w + ort.z), 0.f) : 0.f);
}

// corner dominance at junctions
static inline Vertex4D dom(const Vertex4D &strx, const Vertex4D &stry, const Vertex4D &strz, const Vertex4D &strw) {
    return Vertex4D(max(2.f * strx.z - (strx.y + strx.w), 0.f),
                    max(2.f * stry.w - (stry.z + stry.x), 0.f),
                    max(2.f * strz.x - (strz.w + strz.y), 0.f),
                    max(2.f * strw.y - (strw.x + strw.z), 0.f));
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
    return bool4(
        jDx.x != 0.f && jDx.x + jDx.z > jDx.y + jDx.w,
        jDx.y != 0.f && jDx.y + jDx.w > jDx.z + jDx.x,
        jDx.z != 0.f && jDx.z + jDx.x > jDx.w + jDx.y,
        jDx.w != 0.f && jDx.w + jDx.y > jDx.x + jDx.z);
}

void upscale(DWORD * const data, const int2 res, const bool is_brightness_data)
{
    const unsigned int xres = res.x;
    const unsigned int yres = res.y;
    std::vector<Vertex4D> metric(xres*yres); //!! avoid constant reallocs?

    unsigned int o = 0;
    if (is_brightness_data)
    {
        for (unsigned int j = 0; j < yres; ++j)
        {
            const unsigned int jm1 = (j - 1)*xres;

            for (unsigned int i = 0; i < xres; ++i, ++o)
            {
                const unsigned int ip1 = i + 1;
                const DWORD E = data[j*xres + i];
                const DWORD E2 = (E & 0xFEFEFE) >> 1; // borders = half black/half border pixel
                const DWORD A = (j == 0) || (i == 0) ? E2 : data[jm1 + i - 1];
                const DWORD B = (j == 0) ? E2 : data[jm1 + i];
                const DWORD C = (j == 0) || (i == xres - 1) ? E2 : data[jm1 + ip1];
                const DWORD F = (i == xres - 1) ? E2 : data[j*xres + ip1];

                metric[o] = Vertex4D(eq_brightness(E, A), eq_brightness(E, B), eq_brightness(E, C), eq_brightness(E, F));
            }
        }
    }
    else
    {
        for (unsigned int j = 0; j < yres; ++j)
        {
            const unsigned int jm1 = (j - 1)*xres;

            for (unsigned int i = 0; i < xres; ++i, ++o)
            {
                const unsigned int ip1 = i + 1;
                const DWORD E = data[j*xres + i];
                const DWORD E2 = (E & 0xFEFEFEFE) >> 1; // borders = half black/half border pixel
                const DWORD A = (j == 0) || (i == 0) ? E2 : data[jm1 + i - 1];
                const DWORD B = (j == 0) ? E2 : data[jm1 + i];
                const DWORD C = (j == 0) || (i == xres - 1) ? E2 : data[jm1 + ip1];
                const DWORD F = (i == xres - 1) ? E2 : data[j*xres + ip1];

                metric[o] = Vertex4D(eq_col1(E, A), eq_col1(E, B), eq_col1(E, C), eq_col1(E, F));
            }
        }
    }

    std::vector<bool4> g_res (xres*yres); //!! avoid constant reallocs?
    std::vector<bool4> g_hori(xres*yres);
    std::vector<bool4> g_vert(xres*yres);
    std::vector<bool4> g_or  (xres*yres);

    o = 0;
    for (unsigned int j = 0; j < yres; ++j)
    {
        const unsigned int jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const unsigned int jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;
        const unsigned int jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;

        for (unsigned int i = 0; i < xres; ++i, ++o)
        {
            const unsigned int im2 = (i <= 1) ? 0 : i - 2;
            const unsigned int im1 = (i == 0) ? 0 : i - 1;
            const unsigned int ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const unsigned int ip2 = (i >= xres - 2) ? xres - 1 : i + 2;

            const Vertex4D M = metric[jm1 + im2];
            const Vertex4D A = metric[jm1 + im1];
            const Vertex4D B = metric[jm1 + i];
            const Vertex4D C = metric[jm1 + ip1];
            const Vertex4D P = metric[jm1 + ip2];

            const Vertex4D N = metric[j*xres + im2];
            const Vertex4D D = metric[j*xres + im1];
            const Vertex4D E = metric[j*xres + i];
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
            res.x = jx.z || !(jx.y || jx.w) && (jSx.z != 0.f && (jx.x || jSx.x + jSx.z > jSx.y + jSx.w));
            res.y = jy.w || !(jy.z || jy.x) && (jSy.w != 0.f && (jy.y || jSy.y + jSy.w > jSy.x + jSy.z));
            res.z = jz.x || !(jz.w || jz.y) && (jSz.x != 0.f && (jz.z || jSz.x + jSz.z > jSz.y + jSz.w));
            res.w = jw.y || !(jw.x || jw.z) && (jSw.y != 0.f && (jw.w || jSw.y + jSw.w > jSw.x + jSw.z));

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

    unsigned int* metric_tmp = (unsigned int*)&(metric[0].x);

    o = 0;
    for (unsigned int j = 0; j < yres; ++j)
    {
        const unsigned int jm3 = (j <= 2) ? 0 : (j - 3)*xres;
        const unsigned int jm2 = (j <= 1) ? 0 : (j - 2)*xres;
        const unsigned int jm1 = (j == 0) ? 0 : (j - 1)*xres;
        const unsigned int jp1 = ((j == yres - 1) ? yres - 1 : j + 1)*xres;
        const unsigned int jp2 = ((j >= yres - 2) ? yres - 1 : j + 2)*xres;
        const unsigned int jp3 = ((j >= yres - 3) ? yres - 1 : j + 3)*xres;

        for (unsigned int i = 0; i < xres; ++i, ++o)
        {
            const unsigned int im3 = (i <= 2) ? 0 : i - 3;
            const unsigned int im2 = (i <= 1) ? 0 : i - 2;
            const unsigned int im1 = (i == 0) ? 0 : i - 1;
            const unsigned int ip1 = (i == xres - 1) ? xres - 1 : i + 1;
            const unsigned int ip2 = (i >= xres - 2) ? xres - 1 : i + 2;
            const unsigned int ip3 = (i >= xres - 3) ? xres - 1 : i + 3;

            // read data
            const unsigned int E = j*xres + i;
            const unsigned int D = j*xres + im1, D0 = j*xres + im2, D1 = j*xres + im3;
            const unsigned int F = j*xres + ip1, F0 = j*xres + ip2, F1 = j*xres + ip3;
            const unsigned int B = jm1 + i, B0 = jm2 + i, B1 = jm3 + i;
            const unsigned int H = jp1 + i, H0 = jp2 + i, H1 = jp3 + i;

            // extract data
            const bool4 Ec = g_res[E], Eh = g_hori[E], Ev = g_vert[E], Eo = g_or[E];
            const bool4 Dc = g_res[D], Dh = g_hori[D], Do = g_or[D], D0c = g_res[D0], D0h = g_hori[D0], D1h = g_hori[D1];
            const bool4 Fc = g_res[F], Fh = g_hori[F], Fo = g_or[F], F0c = g_res[F0], F0h = g_hori[F0], F1h = g_hori[F1];
            const bool4 Bc = g_res[B], Bv = g_vert[B], Bo = g_or[B], B0c = g_res[B0], B0v = g_vert[B0], B1v = g_vert[B1];
            const bool4 Hc = g_res[H], Hv = g_vert[H], Ho = g_or[H], H0c = g_res[H0], H0v = g_vert[H0], H1v = g_vert[H1];

            // lvl2 mid (left, right / up, down)
            const bool2 lvl2x = bool2((Ec.x && Eh.y) && Dc.z, (Ec.y && Eh.x) && Fc.w);
            const bool2 lvl2y = bool2((Ec.y && Ev.z) && Bc.w, (Ec.z && Ev.y) && Hc.x);
            const bool2 lvl2z = bool2((Ec.w && Eh.z) && Dc.y, (Ec.z && Eh.w) && Fc.x);
            const bool2 lvl2w = bool2((Ec.x && Ev.w) && Bc.z, (Ec.w && Ev.x) && Hc.y);

            // lvl3 corners (hori, vert)
            const bool2 lvl3x = bool2(lvl2x.y && (Dh.y && Dh.x) && Fh.z, lvl2w.y && (Bv.w && Bv.x) && Hv.z);
            const bool2 lvl3y = bool2(lvl2x.x && (Fh.x && Fh.y) && Dh.w, lvl2y.y && (Bv.z && Bv.y) && Hv.w);
            const bool2 lvl3z = bool2(lvl2z.x && (Fh.w && Fh.z) && Dh.x, lvl2y.x && (Hv.y && Hv.z) && Bv.x);
            const bool2 lvl3w = bool2(lvl2z.y && (Dh.z && Dh.w) && Fh.y, lvl2w.x && (Hv.x && Hv.w) && Bv.y);

            // lvl4 corners (hori, vert)
            const bool2 lvl4x = bool2((Dc.x && Dh.y && Eh.x && Eh.y && Fh.x && Fh.y) && (D0c.z && D0h.w), (Bc.x && Bv.w && Ev.x && Ev.w && Hv.x && Hv.w) && (B0c.z && B0v.y));
            const bool2 lvl4y = bool2((Fc.y && Fh.x && Eh.y && Eh.x && Dh.y && Dh.x) && (F0c.w && F0h.z), (Bc.y && Bv.z && Ev.y && Ev.z && Hv.y && Hv.z) && (B0c.w && B0v.x));
            const bool2 lvl4z = bool2((Fc.z && Fh.w && Eh.z && Eh.w && Dh.z && Dh.w) && (F0c.x && F0h.y), (Hc.z && Hv.y && Ev.z && Ev.y && Bv.z && Bv.y) && (H0c.x && H0v.w));
            const bool2 lvl4w = bool2((Dc.w && Dh.z && Eh.w && Eh.z && Fh.w && Fh.z) && (D0c.y && D0h.x), (Hc.w && Hv.x && Ev.w && Ev.x && Bv.w && Bv.x) && (H0c.y && H0v.z));

            // lvl5 mid (left, right / up, down)
            const bool2 lvl5x = bool2(lvl4x.x && (F0h.x && F0h.y) && (D1h.z && D1h.w), lvl4y.x && (D0h.y && D0h.x) && (F1h.w && F1h.z));
            const bool2 lvl5y = bool2(lvl4y.y && (H0v.y && H0v.z) && (B1v.w && B1v.x), lvl4z.y && (B0v.z && B0v.y) && (H1v.x && H1v.w));
            const bool2 lvl5z = bool2(lvl4w.x && (F0h.w && F0h.z) && (D1h.y && D1h.x), lvl4z.x && (D0h.z && D0h.w) && (F1h.x && F1h.y));
            const bool2 lvl5w = bool2(lvl4x.y && (H0v.x && H0v.w) && (B1v.z && B1v.y), lvl4w.y && (B0v.w && B0v.x) && (H1v.y && H1v.z));

            // lvl6 corners (hori, vert)
            const bool2 lvl6x = bool2(lvl5x.y && (D1h.y && D1h.x), lvl5w.y && (B1v.w && B1v.x));
            const bool2 lvl6y = bool2(lvl5x.x && (F1h.x && F1h.y), lvl5y.y && (B1v.z && B1v.y));
            const bool2 lvl6z = bool2(lvl5z.x && (F1h.w && F1h.z), lvl5y.x && (H1v.y && H1v.z));
            const bool2 lvl6w = bool2(lvl5z.y && (D1h.z && D1h.w), lvl5w.x && (H1v.x && H1v.w));

            // subpixels - 0 = E, 1 = D, 2 = D0, 3 = F, 4 = F0, 5 = B, 6 = B0, 7 = H, 8 = H0
            const unsigned int crn_x = (Ec.x && Eo.x || lvl3x.x && Eo.y || lvl4x.x && Do.x || lvl6x.x && Fo.y) ? 5 : (Ec.x || lvl3x.y && !Eo.w || lvl4x.y && !Bo.x || lvl6x.y && !Ho.w) ? 1 : lvl3x.x ? 3 : lvl3x.y ? 7 : lvl4x.x ? 2 : lvl4x.y ? 6 : lvl6x.x ? 4 : lvl6x.y ? 8 : 0;
            const unsigned int crn_y = (Ec.y && Eo.y || lvl3y.x && Eo.x || lvl4y.x && Fo.y || lvl6y.x && Do.x) ? 5 : (Ec.y || lvl3y.y && !Eo.z || lvl4y.y && !Bo.y || lvl6y.y && !Ho.z) ? 3 : lvl3y.x ? 1 : lvl3y.y ? 7 : lvl4y.x ? 4 : lvl4y.y ? 6 : lvl6y.x ? 2 : lvl6y.y ? 8 : 0;
            const unsigned int crn_z = (Ec.z && Eo.z || lvl3z.x && Eo.w || lvl4z.x && Fo.z || lvl6z.x && Do.w) ? 7 : (Ec.z || lvl3z.y && !Eo.y || lvl4z.y && !Ho.z || lvl6z.y && !Bo.y) ? 3 : lvl3z.x ? 1 : lvl3z.y ? 5 : lvl4z.x ? 4 : lvl4z.y ? 8 : lvl6z.x ? 2 : lvl6z.y ? 6 : 0;
            const unsigned int crn_w = (Ec.w && Eo.w || lvl3w.x && Eo.z || lvl4w.x && Do.w || lvl6w.x && Fo.z) ? 7 : (Ec.w || lvl3w.y && !Eo.x || lvl4w.y && !Ho.w || lvl6w.y && !Bo.x) ? 1 : lvl3w.x ? 3 : lvl3w.y ? 5 : lvl4w.x ? 2 : lvl4w.y ? 8 : lvl6w.x ? 4 : lvl6w.y ? 6 : 0;

            const unsigned int mid_x = (lvl2x.x &&  Eo.x || lvl2x.y &&  Eo.y || lvl5x.x &&  Do.x || lvl5x.y &&  Fo.y) ? 5 : lvl2x.x ? 1 : lvl2x.y ? 3 : lvl5x.x ? 2 : lvl5x.y ? 4 : (Ec.x && Dc.z && Ec.y && Fc.w) ? (Eo.x ? Eo.y ? 5 : 3 : 1) : 0;
            const unsigned int mid_y = (lvl2y.x && !Eo.y || lvl2y.y && !Eo.z || lvl5y.x && !Bo.y || lvl5y.y && !Ho.z) ? 3 : lvl2y.x ? 5 : lvl2y.y ? 7 : lvl5y.x ? 6 : lvl5y.y ? 8 : (Ec.y && Bc.w && Ec.z && Hc.x) ? (!Eo.y ? !Eo.z ? 3 : 7 : 5) : 0;
            const unsigned int mid_z = (lvl2z.x &&  Eo.w || lvl2z.y &&  Eo.z || lvl5z.x &&  Do.w || lvl5z.y &&  Fo.z) ? 7 : lvl2z.x ? 1 : lvl2z.y ? 3 : lvl5z.x ? 2 : lvl5z.y ? 4 : (Ec.z && Fc.x && Ec.w && Dc.y) ? (Eo.z ? Eo.w ? 7 : 1 : 3) : 0;
            const unsigned int mid_w = (lvl2w.x && !Eo.x || lvl2w.y && !Eo.w || lvl5w.x && !Bo.x || lvl5w.y && !Ho.w) ? 1 : lvl2w.x ? 5 : lvl2w.y ? 7 : lvl5w.x ? 6 : lvl5w.y ? 8 : (Ec.w && Hc.y && Ec.x && Bc.z) ? (!Eo.w ? !Eo.x ? 1 : 5 : 7) : 0;

            // ouput
            metric_tmp[o] = crn_x | (crn_y << 4) | (crn_z << 8) | (crn_w << 12) | (mid_x << 16) | (mid_y << 20) | (mid_z << 24) | (mid_w << 28);
        }
    }

    memcpy(&g_or[0], data, xres*yres * sizeof(DWORD));

    o = 0;
    for (unsigned int j = 0; j < yres; ++j)
    {
#ifdef DMD_UPSCALE
        unsigned int offs = j*(xres*9);
        for (unsigned int i = 0; i < xres; ++i, ++o,offs += 3)
#else
        for (unsigned int i = 0; i < xres; ++i, ++o)
#endif
        {
            const unsigned int tmp = metric_tmp[o];
            const unsigned int crn_x = tmp & 15;
            const unsigned int crn_y = (tmp >> 4) & 15;
            const unsigned int crn_z = (tmp >> 8) & 15;
            const unsigned int crn_w = (tmp >> 12) & 15;

            const unsigned int mid_x = (tmp >> 16) & 15;
            const unsigned int mid_y = (tmp >> 20) & 15;
            const unsigned int mid_z = (tmp >> 24) & 15;
            const unsigned int mid_w = (tmp >> 28);

#ifndef DMD_UPSCALE
            unsigned int r = 0, g = 0, b = 0, a = 0;
#endif
            unsigned int o2 = 0;
            for (unsigned int j2 = 0; j2 < 3; ++j2)
                for (unsigned int i2 = 0; i2 < 3; ++i2,++o2)
                {
                    // determine subpixel
                    unsigned int sp = 0;
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
                    unsigned int res = 0;
                    switch (sp)
                    {
                    case 0: res = 0; break;
                    case 1: res = ((i == 0) ? 0xdeadbeef : -1); break;
                    case 2: res = ((i <= 1) ? 0xdeadbeef : -2); break;
                    case 3: res = ((i == xres - 1) ? 0xdeadbeef : 1); break;
                    case 4: res = ((i >= xres - 2) ? 0xdeadbeef : 2); break;
                    case 5: res = ((j == 0) ? 0xdeadbeef : (unsigned int)(-(int)xres)); break;
                    case 6: res = ((j <= 1) ? 0xdeadbeef : -2 * xres); break;
                    case 7: res = ((j == yres - 1) ? 0xdeadbeef : xres); break;
                    case 8: res = ((j >= yres - 2) ? 0xdeadbeef : 2 * xres); break;
                    }

#ifdef DMD_UPSCALE
                    data[offs + j2*(xres*3) + i2] = (res == 0xdeadbeef) ? ((g_or[o].xyzw & 0xFEFEFEFE)>>1) : g_or[o+res].xyzw; // borders = half black/half border pixel
#else
                    const unsigned int tmp2 = (res == 0xdeadbeef) ? ((g_or[o].xyzw & 0xFEFEFEFE) >> 1) : g_or[o+res].xyzw;
                    r +=  tmp2      & 255;
                    g += (tmp2>>8)  & 255;
                    b += (tmp2>>16) & 255;
                    a +=  tmp2>>24;
#endif
                }

#ifndef DMD_UPSCALE
            r /= 9;
            g /= 9;
            b /= 9;
            a /= 9;
            data[o] = r | (g << 8) | (b << 16) | (a << 24);
#endif
        }
    }
}
