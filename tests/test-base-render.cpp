#include "core/stdafx.h"
#include "vpx-test.h"
#include "doctest.h"

// Validate each rendering backend against a reference render (BGFX/DX11)
TEST_CASE("Base table render")
{
   ResetVPX();

   #if defined(ENABLE_DX9)

   SUBCASE("Direct3D9")
   {
      // DX9 rendering does not match other backends
      CaptureRender("test000-default-table.vpx", "test000-default-table-DX9-Test.webp");
      CHECK(CheckMatchingBitmaps("test000-default-table-DX9-Test.webp", "test000-default-table-Ref.webp"));
   }

   #elif defined(ENABLE_OPENGL)

   SUBCASE("OpenGL")
   {
      CaptureRender("test000-default-table.vpx", "test000-default-table-GL-Test.webp");
      CHECK(CheckMatchingBitmaps("test000-default-table-GL-Test.webp", "test000-default-table-Ref.webp"));
   }

   #elif defined(ENABLE_BGFX)

   SUBCASE("BGFX-Vulkan")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend", "Vulkan"s);
      CaptureRender("test000-default-table.vpx", "test000-default-table-BGFX-Vulkan-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::Vulkan);
      CHECK(CheckMatchingBitmaps("test000-default-table-BGFX-Vulkan-Test.webp", "test000-default-table-Ref.webp"));
   }

   SUBCASE("BGFX-OpenGL")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend", "OpenGL"s);
      CaptureRender("test000-default-table.vpx", "test000-default-table-BGFX-GL-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::OpenGL);
      CHECK(CheckMatchingBitmaps("test000-default-table-BGFX-GL-Test.webp", "test000-default-table-Ref.webp"));
   }

   /* Not available on Windows desktop
   SUBCASE("BGFX-OpenGLES")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend", "OpenGLES"s);
      CaptureRender("test000-default-table.vpx", "test000-default-table-BGFX-GLES-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::OpenGLES);
      CHECK(CheckMatchingBitmaps("test000-default-table-BGFX-GLES-Test.webp", "test000-default-table-Ref.webp"));
   }*/

   SUBCASE("BGFX-Direct3D11")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend", "Direct3D11"s);
      CaptureRender("test000-default-table.vpx", "test000-default-table-BGFX-DX11-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::Direct3D11);
      CHECK(CheckMatchingBitmaps("test000-default-table-BGFX-DX11-Test.webp", "test000-default-table-Ref.webp"));
   }

   /* Not stable yet
   SUBCASE("BGFX-Direct3D12")
   {
      g_pvp->m_settings.SaveValue(Settings::Section::Player, "GfxBackend", "Direct3D12"s);
      CaptureRender("test000-default-table.vpx", "test000-default-table-BGFX-DX12-Test.webp");
      CHECK(GetLastRenderer() == bgfx::RendererType::Direct3D12);
      CHECK(CheckMatchingBitmaps("test000-default-table-BGFX-DX12-Test.webp", "test000-default-table-Ref.webp"));
   }*/
   
   #endif

   ResetVPX();
}
