// ****************************************************************************
// * This file is part of the xBRZ project. It is distributed under           *
// * GNU General Public License: https://www.gnu.org/licenses/gpl-3.0         *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved          *
// *                                                                          *
// * Additionally and as a special exception, the author gives permission     *
// * to link the code of this program with the following libraries            *
// * (or with modified versions that use the same licenses), and distribute   *
// * linked combinations including the two: MAME, FreeFileSync, Snes9x, ePSXe *
// * You must obey the GNU General Public License in all respects for all of  *
// * the code used other than MAME, FreeFileSync, Snes9x, ePSXe.              *
// * If you modify this file, you may extend this exception to your version   *
// * of the file, but you are not obligated to do so. If you do not wish to   *
// * do so, delete this exception statement from your version.                *
// ****************************************************************************

#ifndef XBRZ_TOOLS_H_825480175091875
#define XBRZ_TOOLS_H_825480175091875

#include <cassert>
#include <algorithm>
#include <type_traits>


namespace xbrz
{
template <uint32_t N> FORCE_INLINE
unsigned char getByte(uint32_t val) { return static_cast<unsigned char>((val >> (8 * N)) & 0xff); }

FORCE_INLINE unsigned char getAlpha(uint32_t pix) { return getByte<3>(pix); }
FORCE_INLINE unsigned char getRed  (uint32_t pix) { return getByte<2>(pix); }
FORCE_INLINE unsigned char getGreen(uint32_t pix) { return getByte<1>(pix); }
FORCE_INLINE unsigned char getBlue (uint32_t pix) { return getByte<0>(pix); }

FORCE_INLINE uint32_t makePixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b) { return (a << 24) | (r << 16) | (g << 8) | b; }
FORCE_INLINE uint32_t makePixel(                 unsigned char r, unsigned char g, unsigned char b) { return             (r << 16) | (g << 8) | b; }

FORCE_INLINE uint32_t rgb555to888(uint16_t pix) { return ((pix & 0x7C00) << 9) | ((pix & 0x03E0) << 6) | ((pix & 0x001F) << 3); }
FORCE_INLINE uint32_t rgb565to888(uint16_t pix) { return ((pix & 0xF800) << 8) | ((pix & 0x07E0) << 5) | ((pix & 0x001F) << 3); }

FORCE_INLINE uint16_t rgb888to555(uint32_t pix) { return static_cast<uint16_t>(((pix & 0xF80000) >> 9) | ((pix & 0x00F800) >> 6) | ((pix & 0x0000F8) >> 3)); }
FORCE_INLINE uint16_t rgb888to565(uint32_t pix) { return static_cast<uint16_t>(((pix & 0xF80000) >> 8) | ((pix & 0x00FC00) >> 5) | ((pix & 0x0000F8) >> 3)); }


template <class Pix> FORCE_INLINE
Pix* byteAdvance(Pix* ptr, int bytes)
{
    using PixNonConst = typename std::remove_cv<Pix>::type;
    using PixByte     = typename std::conditional<std::is_same<Pix, PixNonConst>::value, char, const char>::type;

    static_assert(std::is_integral<PixNonConst>::value, "Pix* is expected to be cast-able to char*");

    return reinterpret_cast<Pix*>(reinterpret_cast<PixByte*>(ptr) + bytes);
}


//fill block  with the given color
template <class Pix> inline
void fillBlock(Pix* trg, int pitch /*[bytes]*/, Pix col, int blockWidth, int blockHeight)
{
    //for (int y = 0; y < blockHeight; ++y, trg = byteAdvance(trg, pitch))
    //    std::fill(trg, trg + blockWidth, col);

    for (int y = 0; y < blockHeight; ++y, trg = byteAdvance(trg, pitch))
        for (int x = 0; x < blockWidth; ++x)
            trg[x] = col;
}


//nearest-neighbor (going over target image - slow for upscaling, since source is read multiple times missing out on cache! Fast for similar image sizes!)
template <class PixSrc, class PixTrg, class PixConverter>
void nearestNeighborScale(const PixSrc* src, int srcWidth, int srcHeight, int srcPitch /*[bytes]*/,
                          /**/  PixTrg* trg, int trgWidth, int trgHeight, int trgPitch /*[bytes]*/,
                          int yFirst, int yLast, PixConverter pixCvrt /*convert PixSrc to PixTrg*/)
{
    static_assert(std::is_integral<PixSrc>::value, "PixSrc* is expected to be cast-able to char*");
    static_assert(std::is_integral<PixTrg>::value, "PixTrg* is expected to be cast-able to char*");

    static_assert(std::is_same<decltype(pixCvrt(PixSrc())), PixTrg>::value, "PixConverter returning wrong pixel format");

    if (srcPitch < srcWidth * static_cast<int>(sizeof(PixSrc))  ||
        trgPitch < trgWidth * static_cast<int>(sizeof(PixTrg)))
    {
        assert(false);
        return;
    }

    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, trgHeight);
    if (yFirst >= yLast || srcHeight <= 0 || srcWidth <= 0) return;

    for (int y = yFirst; y < yLast; ++y)
    {
        const int ySrc = srcHeight * y / trgHeight;
        const PixSrc* const srcLine = byteAdvance(src, ySrc * srcPitch);
        PixTrg*       const trgLine = byteAdvance(trg, y    * trgPitch);

        for (int x = 0; x < trgWidth; ++x)
        {
            const int xSrc = srcWidth * x / trgWidth;
            trgLine[x] = pixCvrt(srcLine[xSrc]);
        }
    }
}


//nearest-neighbor (going over source image - fast for upscaling, since source is read only once
template <class PixSrc, class PixTrg, class PixConverter>
void nearestNeighborScaleOverSource(const PixSrc* src, int srcWidth, int srcHeight, int srcPitch /*[bytes]*/,
                                    /**/  PixTrg* trg, int trgWidth, int trgHeight, int trgPitch /*[bytes]*/,
                                    int yFirst, int yLast, PixConverter pixCvrt /*convert PixSrc to PixTrg*/)
{
    static_assert(std::is_integral<PixSrc>::value, "PixSrc* is expected to be cast-able to char*");
    static_assert(std::is_integral<PixTrg>::value, "PixTrg* is expected to be cast-able to char*");

    static_assert(std::is_same<decltype(pixCvrt(PixSrc())), PixTrg>::value, "PixConverter returning wrong pixel format");

    if (srcPitch < srcWidth * static_cast<int>(sizeof(PixSrc))  ||
        trgPitch < trgWidth * static_cast<int>(sizeof(PixTrg)))
    {
        assert(false);
        return;
    }

    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, srcHeight);
    if (yFirst >= yLast || trgWidth <= 0 || trgHeight <= 0) return;

    for (int y = yFirst; y < yLast; ++y)
    {
        //mathematically: ySrc = floor(srcHeight * yTrg / trgHeight)
        // => search for integers in: [ySrc, ySrc + 1) * trgHeight / srcHeight

        //keep within for loop to support MT input slices!
        const int yTrgFirst = ( y      * trgHeight + srcHeight - 1) / srcHeight; //=ceil(y * trgHeight / srcHeight)
        const int yTrgLast  = ((y + 1) * trgHeight + srcHeight - 1) / srcHeight; //=ceil(((y + 1) * trgHeight) / srcHeight)
        const int blockHeight = yTrgLast - yTrgFirst;

        if (blockHeight > 0)
        {
            const PixSrc* srcLine = byteAdvance(src, y         * srcPitch);
            /**/  PixTrg* trgLine = byteAdvance(trg, yTrgFirst * trgPitch);
            int xTrgFirst = 0;

            for (int x = 0; x < srcWidth; ++x)
            {
                const int xTrgLast = ((x + 1) * trgWidth + srcWidth - 1) / srcWidth;
                const int blockWidth = xTrgLast - xTrgFirst;
                if (blockWidth > 0)
                {
                    xTrgFirst = xTrgLast;

                    const auto trgPix = pixCvrt(srcLine[x]);
                    fillBlock(trgLine, trgPitch, trgPix, blockWidth, blockHeight);
                    trgLine += blockWidth;
                }
            }
        }
    }
}


template <class PixTrg, class PixConverter>
void bilinearScale(const uint32_t* src, int srcWidth, int srcHeight, int srcPitch,
                   /**/    PixTrg* trg, int trgWidth, int trgHeight, int trgPitch,
                   int yFirst, int yLast, PixConverter pixCvrt /*convert uint32_t to PixTrg*/)
{
    static_assert(std::is_integral<PixTrg>::value,                            "PixTrg* is expected to be cast-able to char*");
    static_assert(std::is_same<decltype(pixCvrt(uint32_t())), PixTrg>::value, "PixConverter returning wrong pixel format");

    if (srcPitch < srcWidth * static_cast<int>(sizeof(uint32_t)) ||
        trgPitch < trgWidth * static_cast<int>(sizeof(PixTrg)))
    {
        assert(false);
        return;
    }

    yFirst = std::max(yFirst, 0);
    yLast  = std::min(yLast, trgHeight);
    if (yFirst >= yLast || srcHeight <= 0 || srcWidth <= 0) return;

    const float scaleX = static_cast<float>(trgWidth ) / static_cast<float>(srcWidth );
    const float scaleY = static_cast<float>(trgHeight) / static_cast<float>(srcHeight);

    //perf notes:
    //    -> double-based calculation is (slightly) faster than float
    //    -> pre-calculation gives significant boost; std::vector<> memory allocation is negligible!
    struct CoeffsX
    {
        int     x1 = 0;
        int     x2 = 0;
        float  xx1 = 0;
        float  x2x = 0;
    };
    std::vector<CoeffsX> buf(trgWidth);
    for (int x = 0; x < trgWidth; ++x)
    {
        const int x1 = srcWidth * x / trgWidth;
        int x2 = x1 + 1;
        if (x2 == srcWidth) --x2;

        const float xx1 = x / scaleX - x1;
        const float x2x = 1.f - xx1;

        buf[x] = { x1, x2, xx1, x2x };
    }

    for (int y = yFirst; y < yLast; ++y)
    {
        const int y1 = srcHeight * y / trgHeight;
        int y2 = y1 + 1;
        if (y2 == srcHeight) --y2;

        const float yy1 = y / scaleY - y1;
        const float y2y = 1.f - yy1;

        const uint32_t* const srcLine     = byteAdvance(src, y1 * srcPitch);
        const uint32_t* const srcLineNext = byteAdvance(src, y2 * srcPitch);
        PixTrg*         const trgLine     = byteAdvance(trg, y  * trgPitch);

        for (int x = 0; x < trgWidth; ++x)
        {
            //perf: do NOT "simplify" the variable layout without measurement!
            const int     x1 = buf[x].x1;
            const int     x2 = buf[x].x2;
            const float  xx1 = buf[x].xx1;
            const float  x2x = buf[x].x2x;

            const float  x2xy2y = x2x * y2y;
            const float  xx1y2y = xx1 * y2y;
            const float  x2xyy1 = x2x * yy1;
            const float  xx1yy1 = xx1 * yy1;

            auto interpolate = [=](int offset)
            {
                /* https://en.wikipedia.org/wiki/Bilinear_interpolation
                     (c11(x2 - x) + c21(x - x1)) * (y2 - y ) +
                     (c12(x2 - x) + c22(x - x1)) * (y  - y1)                          */
                const auto c11 = (srcLine    [x1] >> (8 * offset)) & 0xff;
                const auto c21 = (srcLine    [x2] >> (8 * offset)) & 0xff;
                const auto c12 = (srcLineNext[x1] >> (8 * offset)) & 0xff;
                const auto c22 = (srcLineNext[x2] >> (8 * offset)) & 0xff;

                return c11 * x2xy2y + c21 * xx1y2y +
                       c12 * x2xyy1 + c22 * xx1yy1;
            };

            const float bi = interpolate(0);
            const float gi = interpolate(1);
            const float ri = interpolate(2);
            const float ai = interpolate(3);

            const auto b = static_cast<uint32_t>(bi + 0.5f);
            const auto g = static_cast<uint32_t>(gi + 0.5f);
            const auto r = static_cast<uint32_t>(ri + 0.5f);
            const auto a = static_cast<uint32_t>(ai + 0.5f);

            const uint32_t trgPix = (a << 24) | (r << 16) | (g << 8) | b;

            trgLine[x] = pixCvrt(trgPix);
        }
    }
}
}

#endif //XBRZ_TOOLS_H_825480175091875
