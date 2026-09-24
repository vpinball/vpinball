// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <vector>

// xbrz.h defines FORCE_INLINE which mmpx relies on, so keep it first like UpscaleDMD.cpp does
#include "upscaledmd/xbrz/xbrz.h"
#include "upscaledmd/mmpx/mmpx.h"
#include "upscaledmd/scalefx/scalefx.h"
#include "upscaledmd/super-xbr/super-xbr.h"

namespace
{

constexpr uint32_t kBlack = 0xFF000000u;
constexpr uint32_t kWhite = 0xFFFFFFFFu;
constexpr uint32_t kMidGray = 0xFF808080u;
constexpr uint32_t kRed = 0xFFFF0000u; // packed as 0xAARRGGBB like UpscaleDMD frames

std::vector<uint32_t> UniformImage(int w, int h, uint32_t color) { return std::vector<uint32_t>(static_cast<size_t>(w) * h, color); }

// Left half dark, right half bright
std::vector<uint32_t> SplitImage(int w, int h)
{
   std::vector<uint32_t> img(static_cast<size_t>(w) * h);
   for (int y = 0; y < h; ++y)
      for (int x = 0; x < w; ++x)
         img[y * w + x] = (x < w / 2) ? kBlack : kWhite;
   return img;
}

bool AllEqual(const std::vector<uint32_t>& img, uint32_t color)
{
   return std::ranges::all_of(img, [color](uint32_t px) { return px == color; });
}

} // namespace

TEST_CASE("UpscaleDMD scaling algorithms")
{
   constexpr int srcW = 8, srcH = 4;
   const std::vector<uint32_t> uniform = UniformImage(srcW, srcH, kMidGray);
   const std::vector<uint32_t> split = SplitImage(srcW, srcH);

   SUBCASE("xbrz scales by the requested factor and preserves flats")
   {
      for (size_t factor = 2; factor <= xbrz::SCALE_FACTOR_MAX; ++factor)
      {
         std::vector<uint32_t> out(uniform.size() * factor * factor, 0);
         xbrz::scale(factor, uniform.data(), out.data(), srcW, srcH, xbrz::ColorFormat::rgb);
         CHECK(AllEqual(out, kMidGray));

         std::vector<uint32_t> outSplit(split.size() * factor * factor, 0);
         xbrz::scale(factor, split.data(), outSplit.data(), srcW, srcH, xbrz::ColorFormat::rgb);
         const int outW = srcW * static_cast<int>(factor);
         const int outH = srcH * static_cast<int>(factor);
         CHECK(outSplit[0] == kBlack); // top left stays dark
         CHECK(outSplit[outW - 1] == kWhite); // top right stays bright
         CHECK(outSplit[(outH - 1) * outW] == kBlack); // bottom left
         CHECK(outSplit[outH * outW - 1] == kWhite); // bottom right
      }
   }

   SUBCASE("xbrz is deterministic")
   {
      std::vector<uint32_t> a(split.size() * 4), b(split.size() * 4);
      xbrz::scale(2, split.data(), a.data(), srcW, srcH, xbrz::ColorFormat::rgb);
      xbrz::scale(2, split.data(), b.data(), srcW, srcH, xbrz::ColorFormat::rgb);
      CHECK(a == b);
   }

   SUBCASE("xbrz nearest neighbor replicates pixels exactly")
   {
      const std::vector<uint32_t> src = { kBlack, kWhite, kRed, kMidGray }; // 2x2
      std::vector<uint32_t> out(4 * 4);
      xbrz::nearestNeighborScale(src.data(), 2, 2, out.data(), 4, 4);
      CHECK(out[0] == kBlack);
      CHECK(out[1] == kBlack);
      CHECK(out[2] == kWhite);
      CHECK(out[3] == kWhite);
      CHECK(out[2 * 4] == kRed);
      CHECK(out[3 * 4 + 3] == kMidGray);
   }

   SUBCASE("xbrz bilinear preserves flats")
   {
      std::vector<uint32_t> out(7 * 5);
      xbrz::bilinearScale(uniform.data(), srcW, srcH, out.data(), 7, 5);
      CHECK(AllEqual(out, kMidGray));
   }

   SUBCASE("mmpx 2x preserves flats and edges")
   {
      mmpx::MMPXAlgorithm scaler;
      std::vector<uint32_t> out(uniform.size() * 4, 0);
      scaler.run(uniform.data(), out.data(), srcW, srcH);
      CHECK(AllEqual(out, kMidGray));

      std::vector<uint32_t> outSplit(split.size() * 4, 0);
      scaler.run(split.data(), outSplit.data(), srcW, srcH);
      CHECK(outSplit[0] == kBlack);
      CHECK(outSplit[2 * srcW - 1] == kWhite);
      CHECK(outSplit[(2 * srcH - 1) * 2 * srcW] == kBlack);
      CHECK(outSplit[2 * srcH * 2 * srcW - 1] == kWhite);
   }

   SUBCASE("super-xbr 2x preserves flats and edges")
   {
      std::vector<uint32_t> out(uniform.size() * 4, 0);
      superxbr::scale<2, false>(uniform.data(), out.data(), srcW, srcH);
      CHECK(AllEqual(out, kMidGray));

      std::vector<uint32_t> outSplit(split.size() * 4, 0);
      superxbr::scale<2, false>(split.data(), outSplit.data(), srcW, srcH);
      CHECK(outSplit[0] == kBlack);
      CHECK(outSplit[2 * srcW - 1] == kWhite);
      CHECK(outSplit[(2 * srcH - 1) * 2 * srcW] == kBlack);
      CHECK(outSplit[2 * srcH * 2 * srcW - 1] == kWhite);
   }

   SUBCASE("scalefx 3x preserves flats and edges")
   {
      std::vector<uint32_t> out(uniform.size() * 9, 0);
      scalefx::upscale<true>(uniform.data(), out.data(), srcW, srcH, false);
      CHECK(AllEqual(out, kMidGray));

      std::vector<uint32_t> outSplit(split.size() * 9, 0);
      scalefx::upscale<true>(split.data(), outSplit.data(), srcW, srcH, false);
      CHECK(outSplit[0] == kBlack);
      CHECK(outSplit[3 * srcW - 1] == kWhite);
      CHECK(outSplit[(3 * srcH - 1) * 3 * srcW] == kBlack);
      CHECK(outSplit[3 * srcH * 3 * srcW - 1] == kWhite);
   }

   SUBCASE("scalefx anti-aliased mode keeps the source resolution")
   {
      std::vector<uint32_t> out(uniform.size(), 0);
      scalefx::upscale<false>(uniform.data(), out.data(), srcW, srcH, false);
      CHECK(AllEqual(out, kMidGray));
   }

   SUBCASE("isolated single pixel input stays in bounds")
   {
      std::vector<uint32_t> dot = UniformImage(srcW, srcH, kBlack);
      dot[2 * srcW + 3] = kRed;

      std::vector<uint32_t> outXbrz(dot.size() * 4, 0);
      xbrz::scale(2, dot.data(), outXbrz.data(), srcW, srcH, xbrz::ColorFormat::rgb);
      // xBRZ blends the dot with its neighbors instead of copying it verbatim, so check
      // that the output carries a clearly red dominant pixel rather than an exact match
      const auto isReddish = [](uint32_t px)
      {
         const uint32_t r = (px >> 16) & 0xFF, g = (px >> 8) & 0xFF, b = px & 0xFF;
         return r > 0x40 && r > g * 2 && r > b * 2;
      };
      CHECK(std::ranges::any_of(outXbrz, isReddish));

      mmpx::MMPXAlgorithm scaler;
      std::vector<uint32_t> outMmpx(dot.size() * 4, 0);
      scaler.run(dot.data(), outMmpx.data(), srcW, srcH);
      CHECK(std::ranges::find(outMmpx, kRed) != outMmpx.end());
   }
}
