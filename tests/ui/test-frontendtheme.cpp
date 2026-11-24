// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/VPApp.h"
#include "renderer/Texture.h"
#include "ui/live/frontend/FrontendTheme.h"

#include "doctest.h"


TEST_CASE("FrontendTheme::SelectVariant")
{
   using VPX::Frontend::FrontendTheme;
   const std::filesystem::path theme = GetTestTmpDir() / "themes"sv / "test"sv;
   const auto touch = [](const std::filesystem::path &path)
   {
      std::filesystem::create_directories(path.parent_path());
      std::ofstream file(path, std::ios::binary);
   };
   touch(theme / "layout.webp"s);
   touch(theme / "layout-16x9.webp"s);
   touch(theme / "layout-21x9.webp"s);
   touch(theme / "layout-32x9.mp4"s); // Videos are only eligible when the part allows them
   touch(theme / "underlay.webp"s);
   touch(theme / "underlay-16x9.mp4"s);
   touch(theme / "overlay-16x9.webp"s);
   touch(theme / "overlay-21x9.mkv"s);

   // Smallest variant ratio >= target is preferred
   CHECK(FrontendTheme::SelectVariant(theme, "layout"s, 16.f / 9.f).filename() == "layout-16x9.webp"s);
   CHECK(FrontendTheme::SelectVariant(theme, "layout"s, 2.f).filename() == "layout-21x9.webp"s);
   // No big enough variant: the largest ratio is taken
   CHECK(FrontendTheme::SelectVariant(theme, "layout"s, 4.f).filename() == "layout-21x9.webp"s);
   // Unsuffixed file only applies when no ratio variant exists
   CHECK(FrontendTheme::SelectVariant(theme, "underlay"s, 16.f / 9.f).filename() == "underlay.webp"s);
   CHECK(FrontendTheme::SelectVariant(theme, "overlay"s, 4.f).filename() == "overlay-16x9.webp"s);
   CHECK(FrontendTheme::SelectVariant(theme, "missing"s, 1.f).empty());
   // Videos are ignored by default but participate in the aspect ratio selection when allowed
   CHECK(FrontendTheme::SelectVariant(theme, "underlay"s, 16.f / 9.f, true).filename() == "underlay-16x9.mp4"s);
   CHECK(FrontendTheme::SelectVariant(theme, "overlay"s, 2.f, true).filename() == "overlay-21x9.mkv"s);
   CHECK(FrontendTheme::SelectVariant(theme, "layout"s, 4.f, true).filename() == "layout-32x9.mp4"s);
}

TEST_CASE("FrontendTheme::Discover")
{
   using VPX::Frontend::FrontendTheme;
   const std::filesystem::path root = GetTestTmpDir() / "themeRoots"sv;
   const auto touch = [](const std::filesystem::path &path)
   {
      std::filesystem::create_directories(path.parent_path());
      std::ofstream file(path, std::ios::binary);
   };
   touch(root / "Alpha"s / "layout-16x9.webp"s);
   touch(root / "Alpha"s / "overlay.webp"s);
   touch(root / "Beta"s / "underlay.webp"s); // No layout map: not a theme
   touch(root / "not-a-folder.txt"s);

   const auto themes = FrontendTheme::Discover({ root });
   REQUIRE(themes.size() == 1);
   CHECK(themes[0].name == "Alpha"s);
   CHECK(themes[0].folder == root / "Alpha"s);
}

TEST_CASE("FrontendTheme::ParseLayout")
{
   using VPX::Frontend::FrontendTheme;
   const int w = 16, h = 8;
   auto img = BaseTexture::Create(w, h, BaseTexture::SRGBA);
   REQUIRE(img != nullptr);
   memset(img->data(), 0, img->pitch() * h);
   const auto px = [&](const int x, const int y, const uint8_t r, const uint8_t g, const uint8_t b)
   {
      uint8_t *const p = static_cast<uint8_t *>(img->data()) + y * img->pitch() + x * 4;
      p[0] = r, p[1] = g, p[2] = b, p[3] = 255;
   };

   for (int x = 1; x <= 4; x++)
      for (int y = 1; y <= 3; y++)
         px(x, y, 0, 255, 0); // Playfield rect
   for (int x = 6; x <= 8; x++)
      for (int y = 0; y <= 1; y++)
         px(x, y, 0, 0, 255); // Backglass rect
   px(9, 0, 0, 255, 255); // Instruction card, single pixel
   for (int x = 0; x < w; x++)
      px(x, 6, 255, 0, 255); // Menu band row
   for (int x = 0; x < w; x++)
      px(x, 7, 255, 255, 255); // Info band row
   for (int x = 2; x <= 13; x++)
      if (x != 8) // Wheel band, one missing column
         for (int y = 4; y <= 5; y++)
            px(x, y, 255, 0, 0);

   const FrontendTheme::Layout layout = FrontendTheme::ParseLayout(*img);

   REQUIRE(layout.playfield.valid);
   CHECK(layout.playfield.xMin == doctest::Approx(1.f / w));
   CHECK(layout.playfield.yMin == doctest::Approx(1.f / h));
   CHECK(layout.playfield.xMax == doctest::Approx(5.f / w));
   CHECK(layout.playfield.yMax == doctest::Approx(4.f / h));

   REQUIRE(layout.backglass.valid);
   CHECK(layout.backglass.xMin == doctest::Approx(6.f / w));
   CHECK(layout.backglass.yMin == doctest::Approx(0.f));
   CHECK(layout.backglass.xMax == doctest::Approx(9.f / w));
   CHECK(layout.backglass.yMax == doctest::Approx(2.f / h));

   REQUIRE(layout.card.valid);
   CHECK(layout.card.xMin == doctest::Approx(9.f / w));
   CHECK(layout.card.yMin == doctest::Approx(0.f));

   REQUIRE(layout.menuBand.valid);
   CHECK(layout.menuBand.yMin == doctest::Approx(6.f / h));
   CHECK(layout.menuBand.yMax == doctest::Approx(7.f / h));

   REQUIRE(layout.infoBand.valid);
   CHECK(layout.infoBand.yMin == doctest::Approx(7.f / h));
   CHECK(layout.infoBand.yMax == doctest::Approx(8.f / h));

   CHECK(layout.wheelFirstColumn == 2);
   CHECK(layout.wheelLastColumn == 13);
   CHECK(layout.wheelColumns[0].first < 0.f); // Outside the band's ends: untouched
   CHECK(layout.wheelColumns[2].first == doctest::Approx(4.f / h));
   CHECK(layout.wheelColumns[2].second == doctest::Approx(6.f / h));
   // The empty column is interpolated between its neighbors (identical here)
   CHECK(layout.wheelColumns[8].first == doctest::Approx(4.f / h));
   CHECK(layout.wheelColumns[8].second == doctest::Approx(6.f / h));
}

TEST_CASE("FrontendTheme::ParseLayout bundled theme")
{
   using VPX::Frontend::FrontendTheme;
   // The bundled base theme layout is a lossy webp: zone colors must survive its compression
   // (a dark card color used to bleed into the bright backglass zone and lose its bounds)
   const std::filesystem::path path = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "frontend/base/layout-21x9.webp");
   REQUIRE(FileExists(path));
   const std::shared_ptr<BaseTexture> img = BaseTexture::CreateFromFile(path);
   REQUIRE(img != nullptr);
   const int w = static_cast<int>(img->width()), h = static_cast<int>(img->height());

   const FrontendTheme::Layout layout = FrontendTheme::ParseLayout(*img);

   REQUIRE(layout.backglass.valid);
   REQUIRE(layout.card.valid);
   // The card sits under the backglass: it used to bleed into it on the lossy webp (yMin ~0.09)
   CHECK(layout.card.yMin > layout.backglass.yMax);
   CHECK(layout.card.xMin == doctest::Approx(468.f / 3440.f).epsilon(0.1));
   CHECK(layout.card.xMax == doctest::Approx(1198.f / 3440.f).epsilon(0.1));
   CHECK(layout.card.yMin == doctest::Approx(610.f / 1440.f).epsilon(0.1));
   CHECK(layout.card.yMax == doctest::Approx(1008.f / 1440.f).epsilon(0.1));

   REQUIRE(layout.wheelFirstColumn >= 0);
   CHECK(layout.wheelFirstColumn <= 1); // The wheel band spans the full width
   CHECK(layout.wheelLastColumn >= w - 2);
}

TEST_CASE("FrontendTheme::ComputeFit")
{
   float scale, offsetX, offsetY;
   VPX::Frontend::FrontendTheme::ComputeFit(420, 180, 1600, 900, scale, offsetX, offsetY); // 21:9 image on 16:9 window
   CHECK(scale == doctest::Approx(5.f));
   CHECK(offsetX == doctest::Approx(-250.f)); // Image overflows on width: center cropped
   CHECK(offsetY == doctest::Approx(0.f));
   VPX::Frontend::FrontendTheme::ComputeFit(1600, 900, 900, 1600, scale, offsetX, offsetY); // Landscape on portrait
   CHECK(scale == doctest::Approx(1600.f / 900.f));
   CHECK(offsetX < 0.f);
}
