// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendTheme.h"

#include "core/TableCatalog.h"
#include "renderer/Texture.h"


namespace VPX::Frontend
{

namespace
{

   // Theme part files: still images, or animated medias when the part allows them (underlay,
   // overlay and wheel-empty play in place; the layout map is always a still image)
   bool IsThemeFile(const std::filesystem::path &path, const bool animated) { return TableCatalog::IsImageFile(path) || (animated && TableCatalog::IsAnimatedMediaFile(path)); }

   // Lists the variants of 'stem' in a theme folder: files named '<stem>.<ext>' (aspect ratio -1,
   // universal fallback) or '<stem>-<W>x<H>.<ext>' (the ratio they target)
   vector<std::pair<std::filesystem::path, float>> ListVariants(const std::filesystem::path &folder, const string &stem, const bool animated = false)
   {
      vector<std::pair<std::filesystem::path, float>> variants;
      std::error_code ec;
      for (const auto &entry : std::filesystem::directory_iterator(folder, ec))
      {
         if (!entry.is_regular_file() || !IsThemeFile(entry.path(), animated))
            continue;
         const string name = PathToUTF8(entry.path().stem());
         if (name == stem)
            variants.emplace_back(entry.path(), -1.f);
         else if (name.starts_with(stem + '-'))
         {
            const string suffix = name.substr(stem.size() + 1);
            int w, h;
            if (sscanf_s(suffix.c_str(), "%dx%d", &w, &h) == 2 && w > 0 && h > 0)
               variants.emplace_back(entry.path(), static_cast<float>(w) / static_cast<float>(h));
         }
      }
      return variants;
   }

   // Classifies a pixel to the nearest layout key color, within a tolerance to survive lossy
   // compression. Returns -1 when no key color is close enough. The keys are the vertices of
   // the RGB cube (the background being the last one, black): the widest possible spacing so
   // that the nearest key stays correct even when a lossy codec shifts a pixel's chroma, which
   // it does far more than its luma (a dark or desaturated key would sit too close to its
   // neighbours, e.g. a dark blue card bleeding into the bright blue backglass zone).
   int ClassifyLayoutPixel(const uint8_t r, const uint8_t g, const uint8_t b)
   {
      static constexpr uint8_t keys[][3] = {
         { 0, 255, 0 }, // 0 playfield
         { 0, 0, 255 }, // 1 backglass
         { 0, 255, 255 }, // 2 instruction card
         { 255, 0, 255 }, // 3 menu band
         { 255, 255, 255 }, // 4 info band
         { 255, 0, 0 }, // 5 wheel band
         { 255, 255, 0 }, // 6 info pane
      };
      int best = -1;
      int bestDist = 0;
      for (int i = 0; i < static_cast<int>(std::size(keys)); i++)
      {
         const int dr = static_cast<int>(r) - keys[i][0], dg = static_cast<int>(g) - keys[i][1], db = static_cast<int>(b) - keys[i][2];
         const int dist = dr * dr + dg * dg + db * db;
         if (best < 0 || dist < bestDist)
         {
            best = i;
            bestDist = dist;
         }
      }
      return bestDist <= 150 * 150 ? best : -1;
   }

}

vector<FrontendTheme::Entry> FrontendTheme::Discover(const vector<std::filesystem::path> &roots)
{
   vector<Entry> themes;
   for (const auto &root : roots)
   {
      std::error_code ec;
      if (!std::filesystem::is_directory(root, ec))
         continue;
      for (const auto &entry : std::filesystem::directory_iterator(root, ec))
      {
         if (!entry.is_directory() || ListVariants(entry.path(), "layout"s).empty())
            continue;
         themes.push_back({ PathToUTF8(entry.path().filename()), entry.path() });
      }
   }
   return themes;
}

std::filesystem::path FrontendTheme::SelectVariant(const std::filesystem::path &folder, const string &stem, const float targetAR, const bool animated)
{
   const vector<std::pair<std::filesystem::path, float>> variants = ListVariants(folder, stem, animated);
   std::filesystem::path best, fallback;
   float bestAR = 0.f, fallbackAR = -1.f;
   for (const auto &[path, ar] : variants)
   {
      if (ar < 0.f)
      {
         if (fallback.empty()) // Universal file, only used when no ratio variant exists
            fallback = path;
         continue;
      }
      if (ar >= targetAR && (best.empty() || ar < bestAR))
      {
         best = path;
         bestAR = ar;
      }
      if (ar > fallbackAR || fallback.extension().empty()) // Track the largest ratio as a fallback
         fallbackAR = ar, fallback = path;
   }
   return best.empty() ? fallback : best;
}

FrontendTheme::Layout FrontendTheme::ParseLayout(const BaseTexture &layout)
{
   Layout result;
   std::shared_ptr<BaseTexture> img = layout.Convert(BaseTexture::SRGBA);
   if (img == nullptr)
      return result;
   const int w = static_cast<int>(img->width()), h = static_cast<int>(img->height());
   const uint8_t *const pixels = static_cast<const uint8_t *>(img->data());
   const unsigned int pitch = img->pitch();

   int minX[7], minY[7], maxX[7], maxY[7];
   bool seen[7] = {};
   for (int i = 0; i < 7; i++)
      minX[i] = minY[i] = INT_MAX, maxX[i] = maxY[i] = INT_MIN;

   result.wheelColumns.assign(w, { -1.f, -1.f });
   for (int y = 0; y < h; y++)
   {
      const uint8_t *const row = pixels + y * pitch;
      for (int x = 0; x < w; x++)
      {
         // Transparent pixels carry no zone: their RGB is undefined (encoders leave it white)
         if (row[x * 4 + 3] < 128)
            continue;
         const int zone = ClassifyLayoutPixel(row[x * 4], row[x * 4 + 1], row[x * 4 + 2]);
         if (zone < 0)
            continue;
         if (zone == 5)
         {
            auto &col = result.wheelColumns[x];
            if (col.first < 0.f)
               col = { static_cast<float>(y) / h, static_cast<float>(y + 1) / h };
            else
               col.first = std::min(col.first, static_cast<float>(y) / h), col.second = std::max(col.second, static_cast<float>(y + 1) / h);
            result.wheelFirstColumn = result.wheelFirstColumn < 0 ? x : std::min(result.wheelFirstColumn, x);
            result.wheelLastColumn = std::max(result.wheelLastColumn, x);
         }
         else
         {
            seen[zone] = true;
            minX[zone] = std::min(minX[zone], x), maxX[zone] = std::max(maxX[zone], x);
            minY[zone] = std::min(minY[zone], y), maxY[zone] = std::max(maxY[zone], y);
         }
      }
   }

   const auto region = [&](const int zone, Region &out)
   {
      if (!seen[zone])
         return;
      out = { static_cast<float>(minX[zone]) / w, static_cast<float>(minY[zone]) / h, static_cast<float>(maxX[zone] + 1) / w, static_cast<float>(maxY[zone] + 1) / h, true };
   };
   region(0, result.playfield);
   region(1, result.backglass);
   region(2, result.card);
   region(3, result.menuBand);
   region(4, result.infoBand);
   region(6, result.infoPane);

   // Interpolate the wheel band's empty columns between their nearest valid neighbors
   for (int x = result.wheelFirstColumn + 1; x < result.wheelLastColumn; x++)
   {
      if (result.wheelColumns[x].first >= 0.f)
         continue;
      int prev = x - 1;
      while (result.wheelColumns[prev].first < 0.f)
         prev--;
      int next = x + 1;
      while (result.wheelColumns[next].first < 0.f)
         next++;
      const float t = static_cast<float>(x - prev) / static_cast<float>(next - prev);
      result.wheelColumns[x].first = std::lerp(result.wheelColumns[prev].first, result.wheelColumns[next].first, t);
      result.wheelColumns[x].second = std::lerp(result.wheelColumns[prev].second, result.wheelColumns[next].second, t);
   }
   return result;
}

void FrontendTheme::ComputeFit(const int imgW, const int imgH, const int wndW, const int wndH, float &scale, float &offsetX, float &offsetY)
{
   scale = imgH > 0 ? static_cast<float>(wndH) / static_cast<float>(imgH) : 1.f;
   offsetX = (static_cast<float>(wndW) - static_cast<float>(imgW) * scale) * 0.5f; // Negative: center cropped on width
   offsetY = 0.f;
}

}
