#include "core/stdafx.h"
#include "vpx-test.h"
#include "doctest.h"

enum class VPXBackends : int
{
   DX9         = 0x01,
   GL          = 0x02,
   BGFX_DX11   = 0x04,
   BGFX_DX12   = 0x08,
   BGFX_VULKAN = 0x10,
   BGFX_GL     = 0x20,
   BGFX_GLES   = 0x40,

   ALL         = 0x7F,
   ALL_BUT_DX9 = 0x7E
};

static void TestRender(const string& tableFile, const string& imageName, VPXBackends backends)
{

   #if defined(ENABLE_DX9)
   if ((int)backends & (int)VPXBackends::DX9) SUBCASE("Direct3D9")
   {
      // DX9 rendering does not match other backends
      CaptureRender(tableFile, imageName + "-DX9-Test.webp");
      CHECK(CheckMatchingBitmaps(imageName + "-DX9-Test.webp", imageName + "-Ref.webp"));
   }

   #elif defined(ENABLE_OPENGL)

   if ((int)backends & (int)VPXBackends::GL) SUBCASE("OpenGL")
   {
      CaptureRender(tableFile, imageName + "-GL-Test.webp");
      CHECK(CheckMatchingBitmaps(imageName + "-GL-Test.webp", imageName + "-Ref.webp"));
   }

   #elif defined(ENABLE_BGFX)

   if ((int)backends & (int)VPXBackends::BGFX_VULKAN) SUBCASE("BGFX-Vulkan")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend"s, "Vulkan"s);
      CaptureRender(tableFile, imageName + "-BGFX-Vulkan-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::Vulkan);
      CHECK(CheckMatchingBitmaps(imageName + "-BGFX-Vulkan-Test.webp", imageName + "-Ref.webp"));
   }

   if ((int)backends & (int)VPXBackends::BGFX_GL) SUBCASE("BGFX-OpenGL")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend"s, "OpenGL"s);
      CaptureRender(tableFile, imageName + "-BGFX-GL-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::OpenGL);
      CHECK(CheckMatchingBitmaps(imageName + "-BGFX-GL-Test.webp", imageName + "-Ref.webp"));
   }

   /* Not available on Windows desktop
   if ((int)backends & (int)VPXBackends::BGFX_GLES) SUBCASE("BGFX-OpenGLES")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend"s, "OpenGLES"s);
      CaptureRender(tableFile, imageName + "-BGFX-GLES-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::OpenGLES);
      CHECK(CheckMatchingBitmaps(imageName + "-BGFX-GLES-Test.webp", imageName + "-Ref.webp"));
   }*/

   if ((int)backends & (int)VPXBackends::BGFX_DX11) SUBCASE("BGFX-Direct3D11")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend"s, "Direct3D11"s);
      CaptureRender(tableFile, imageName + "-BGFX-DX11-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::Direct3D11);
      CHECK(CheckMatchingBitmaps(imageName + "-BGFX-DX11-Test.webp", imageName + "-Ref.webp"));
   }

   /* Not stable yet
   if ((int)backends & (int)VPXBackends::BGFX_DX12) SUBCASE("BGFX-Direct3D12")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend"s, "Direct3D12"s);
      CaptureRender(tableFile, imageName + "-BGFX-DX12-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::Direct3D12);
      CHECK(CheckMatchingBitmaps(imageName + "-BGFX-DX12-Test.webp", imageName + "-Ref.webp"));
   }*/
   
   #endif
}

TEST_CASE("Base render")
{
   ResetVPX();
   TestRender("test000-default-table.vpx"s, "test000-default-table"s, VPXBackends::ALL);
   ResetVPX();
}

TEST_CASE("Stereo top/bottom render")
{
   ResetVPX();
   Settings& settings = g_pvp->m_settings;
   settings.SaveValue(Settings::Section::Player, "Stereo3D"s, STEREO_TB);
   TestRender("test000-default-table.vpx"s, "test000-stereo-tb"s, VPXBackends::ALL_BUT_DX9);
   ResetVPX();
}

TEST_CASE("Stereo interleaved render")
{
   ResetVPX();
   Settings& settings = g_pvp->m_settings;
   settings.SaveValue(Settings::Section::Player, "Stereo3D"s, STEREO_INT);
   TestRender("test000-default-table.vpx"s, "test000-stereo-int"s, VPXBackends::ALL_BUT_DX9);
   ResetVPX();
}

TEST_CASE("Stereo side by side render")
{
   ResetVPX();
   Settings& settings = g_pvp->m_settings;
   settings.SaveValue(Settings::Section::Player, "Stereo3D"s, STEREO_SBS);
   TestRender("test000-default-table.vpx"s, "test000-stereo-sbs"s, VPXBackends::ALL_BUT_DX9);
   ResetVPX();
}

TEST_CASE("Stereo anaglyph render")
{
   ResetVPX();
   Settings& settings = g_pvp->m_settings;
   settings.SaveValue(Settings::Section::Player, "Stereo3D"s, STEREO_ANAGLYPH_1);
   TestRender("test000-default-table.vpx"s, "test000-stereo-anaglyph"s, VPXBackends::ALL_BUT_DX9);
   ResetVPX();
}
