// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "renderer/Renderer.h"
#include "renderer/Texture.h"
#include "utils/color.h"

// EnvmapPrecalc bakes, on the CPU for all backends, the mip chain of the environment map for the glossy lookup and the diffuse
// irradiance map (tex_diffuse_env). Both must match the convention the material shaders sample through ray_to_equirectangular_uv():
// u = azimuth/2pi + 0.5 with +x at u = 0.5, and v = acos(z)/pi with +z at v = 0.
namespace
{
void FillSRGB(BaseTexture* const tex, const std::function<bool(unsigned int, unsigned int)>& isLit)
{
   uint8_t* const __restrict data = static_cast<uint8_t*>(tex->data());
   for (unsigned int y = 0; y < tex->height(); ++y)
      for (unsigned int x = 0; x < tex->width(); ++x)
      {
         const uint8_t v = isLit(x, y) ? 255 : 0;
         const size_t offs = (static_cast<size_t>(y) * tex->width() + x) * 3;
         data[offs] = v;
         data[offs + 1] = v;
         data[offs + 2] = v;
      }
}
}

TEST_CASE("EnvmapPrecalc irradiance")
{
   SUBCASE("uniform sRGB environment reproduces its color")
   {
      auto env = BaseTexture::Create(128, 64, BaseTexture::SRGB);
      REQUIRE(env != nullptr);
      uint8_t* const data = static_cast<uint8_t*>(env->data());
      memset(data, 128, 128u * 64u * 3u);

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32).irradiance;
      REQUIRE(rad != nullptr);
      CHECK(rad->m_format == BaseTexture::SRGBA);
      CHECK(rad->width() == 64u);
      CHECK(rad->height() == 32u);

      // A constant environment maps to a constant irradiance (normalized cosine weighted average)
      const uint8_t* const out = static_cast<const uint8_t*>(rad->datac());
      for (unsigned int i = 0; i < 64u * 32u; ++i)
         for (unsigned int c = 0; c < 3; ++c)
            CHECK(std::abs(static_cast<int>(out[i * 4 + c]) - 128) <= 1);
   }

   SUBCASE("azimuth half environment lands on the matching direction")
   {
      // White over the +x azimuth half of the equirectangular map (u in [0.25, 0.75))
      auto env = BaseTexture::Create(128, 64, BaseTexture::SRGB);
      REQUIRE(env != nullptr);
      FillSRGB(env.get(), [](unsigned int x, unsigned int) { return x >= 32 && x < 96; });

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32).irradiance;
      REQUIRE(rad != nullptr);
      const uint8_t* const out = static_cast<const uint8_t*>(rad->datac());

      // +x direction maps to u = 0.5 (column 32), -x to u = 0 (column 0), horizon row 16: the hemisphere around +x is (nearly) all white
      const unsigned int plusX = (16u * 64u + 32u) * 4u;
      const unsigned int minusX = (16u * 64u + 0u) * 4u;
      CHECK(out[plusX] > 240);
      CHECK(out[minusX] < 16);
   }

   SUBCASE("upper hemisphere environment lights the top of the map")
   {
      // White over the +z hemisphere (v < 0.5, i.e. upper half of the map)
      auto env = BaseTexture::Create(128, 64, BaseTexture::SRGB);
      REQUIRE(env != nullptr);
      FillSRGB(env.get(), [](unsigned int, unsigned int y) { return y < 32; });

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32).irradiance;
      REQUIRE(rad != nullptr);
      const uint8_t* const out = static_cast<const uint8_t*>(rad->datac());

      // +z direction maps to v = 0 (row 0), -z to v = 1 (row 31), the horizon (row 16) sees half of it
      const unsigned int upZ = (0u * 64u + 32u) * 4u;
      const unsigned int downZ = (31u * 64u + 32u) * 4u;
      CHECK(out[upZ] > 240);
      CHECK(out[downZ] < 16);
      CHECK(InvsRGB8(out[(16u * 64u + 32u) * 4u]) == doctest::Approx(0.5f).epsilon(0.1));
   }

   SUBCASE("uniform HDR environment keeps linear values")
   {
      auto env = BaseTexture::Create(128, 64, BaseTexture::RGB_FP32);
      REQUIRE(env != nullptr);
      float* const data = static_cast<float*>(env->data());
      for (unsigned int i = 0; i < 128u * 64u; ++i)
      {
         data[i * 3] = 0.5f;
         data[i * 3 + 1] = 0.25f;
         data[i * 3 + 2] = 0.125f;
      }

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32).irradiance;
      REQUIRE(rad != nullptr);
      CHECK(rad->m_format == BaseTexture::RGBA_FP32);

      const float* const out = static_cast<const float*>(rad->datac());
      for (unsigned int i = 0; i < 64u * 32u; ++i)
      {
         CHECK(out[i * 4] == doctest::Approx(0.5f).epsilon(0.001));
         CHECK(out[i * 4 + 1] == doctest::Approx(0.25f).epsilon(0.001));
         CHECK(out[i * 4 + 2] == doctest::Approx(0.125f).epsilon(0.001));
      }
   }

   SUBCASE("a bright sun is integrated without noise")
   {
      // Black HDR environment with a small very bright spot at the zenith: the irradiance must decrease smoothly with the angle to it
      auto env = BaseTexture::Create(512, 256, BaseTexture::RGB_FP32);
      REQUIRE(env != nullptr);
      float* const data = static_cast<float*>(env->data());
      memset(data, 0, 512u * 256u * 3u * sizeof(float));
      for (unsigned int y = 0; y < 2; ++y)
         for (unsigned int x = 0; x < 512; ++x)
            data[(y * 512u + x) * 3] = data[(y * 512u + x) * 3 + 1] = data[(y * 512u + x) * 3 + 2] = 10000.f;

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32).irradiance;
      REQUIRE(rad != nullptr);
      const float* const out = static_cast<const float*>(rad->datac());
      for (unsigned int y = 0; y < 32; ++y)
      {
         // Same value over each row (the spot is centered on the pole)
         for (unsigned int x = 1; x < 64; ++x)
            CHECK(out[(y * 64u + x) * 4] == doctest::Approx(out[y * 64u * 4]).epsilon(0.01));
         if (y > 0 && y < 15)
            CHECK(out[y * 64u * 4] < out[(y - 1) * 64u * 4]);
         if (y >= 17)
            CHECK(out[y * 64u * 4] == 0.f);
      }
   }
}

TEST_CASE("EnvmapPrecalc glossy mipmaps")
{
   SUBCASE("full chain of the 4 channel format, level 0 reused when already in it")
   {
      auto env = BaseTexture::Create(512, 256, BaseTexture::RGBA_FP16);
      REQUIRE(env != nullptr);
      const auto result = Renderer::EnvmapPrecalc(env, 64, 32);
      REQUIRE(result.glossyMips.size() == 10u); // 512 -> 1
      CHECK(result.glossyMips[0] == env);
      for (size_t i = 0; i < result.glossyMips.size(); ++i)
      {
         CHECK(result.glossyMips[i]->m_format == BaseTexture::RGBA_FP16);
         CHECK(result.glossyMips[i]->width() == max(1u, 512u >> i));
         CHECK(result.glossyMips[i]->height() == max(1u, 256u >> i));
      }
   }

   SUBCASE("uniform environment gives uniform levels")
   {
      auto env = BaseTexture::Create(256, 128, BaseTexture::RGB_FP32);
      REQUIRE(env != nullptr);
      float* const data = static_cast<float*>(env->data());
      for (unsigned int i = 0; i < 256u * 128u * 3u; ++i)
         data[i] = 2.f;
      const auto result = Renderer::EnvmapPrecalc(env, 32, 16);
      REQUIRE(result.glossyMips.size() == 9u);
      CHECK(result.glossyMips[0] != env); // RGB re-encoded to RGBA
      for (const auto& mip : result.glossyMips)
      {
         REQUIRE(mip->m_format == BaseTexture::RGBA_FP32);
         const float* const out = static_cast<const float*>(mip->datac());
         for (unsigned int i = 0; i < mip->width() * mip->height(); ++i)
         {
            CHECK(out[i * 4] == doctest::Approx(2.f).epsilon(0.001));
            CHECK(out[i * 4 + 3] == 1.f);
         }
      }
   }

   SUBCASE("odd sizes")
   {
      // Height 257: level 1 (height 128) is prefiltered from level 0, which is decoded although the image is above 256
      for (const unsigned int h : { 257u, 3u, 1u })
      {
         auto env = BaseTexture::Create(2 * h, h, BaseTexture::SRGB);
         REQUIRE(env != nullptr);
         memset(env->data(), 200, static_cast<size_t>(2 * h) * h * 3);
         const auto result = Renderer::EnvmapPrecalc(env, 2, 1);
         REQUIRE(result.glossyMips.size() == static_cast<size_t>(std::bit_width(2 * h)));
         REQUIRE(result.irradiance != nullptr);
         for (const auto& mip : result.glossyMips)
         {
            const uint8_t* const out = static_cast<const uint8_t*>(mip->datac());
            for (unsigned int i = 0; i < mip->width() * mip->height(); ++i)
               CHECK(std::abs(static_cast<int>(out[i * 4]) - 200) <= 1);
         }
      }
   }

   SUBCASE("unreadable image")
   {
      CHECK(Renderer::EnvmapPrecalc(nullptr, 64, 32).glossyMips.empty());
      CHECK(Renderer::EnvmapPrecalc(nullptr, 64, 32).irradiance == nullptr);
   }
}
