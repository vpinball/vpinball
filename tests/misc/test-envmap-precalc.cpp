// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "renderer/Renderer.h"
#include "renderer/Texture.h"

// EnvmapPrecalc bakes the diffuse irradiance map (tex_diffuse_env) on the CPU for
// the platforms that cannot run the GPU irradiance pass (DX9, OpenGL ES, Apple,
// wasm/WebGPU, Android XR). The baked map must match the convention the material
// shaders sample through ray_to_equirectangular_uv(): u = azimuth/2pi + 0.5 with
// +x at u = 0.5, and v = acos(z)/pi with +z at v = 0.
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

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32);
      REQUIRE(rad != nullptr);
      CHECK(rad->m_format == BaseTexture::SRGB);
      CHECK(rad->width() == 64u);
      CHECK(rad->height() == 32u);

      // A constant environment maps to a constant irradiance: the sRGB round trip
      // (invGammaApprox in, gammaApprox out) must land back near the source byte.
      const uint8_t* const out = static_cast<const uint8_t*>(rad->datac());
      for (unsigned int i = 0; i < 64u * 32u * 3u; ++i)
         CHECK(std::abs(static_cast<int>(out[i]) - 128) <= 4);
   }

   SUBCASE("azimuth half environment lands on the matching direction")
   {
      // White over the +x azimuth half of the equirectangular map (u in [0.25, 0.75))
      auto env = BaseTexture::Create(128, 64, BaseTexture::SRGB);
      REQUIRE(env != nullptr);
      FillSRGB(env.get(), [](unsigned int x, unsigned int) { return x >= 32 && x < 96; });

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32);
      REQUIRE(rad != nullptr);
      const uint8_t* const out = static_cast<const uint8_t*>(rad->datac());

      // +x direction maps to u = 0.5 (column 32), -x to u = 0 (column 0), horizon row 16
      const unsigned int plusX = (16u * 64u + 32u) * 3u;
      const unsigned int minusX = (16u * 64u + 0u) * 3u;
      CHECK(out[plusX] > 200);
      CHECK(out[minusX] < 128);
   }

   SUBCASE("upper hemisphere environment lights the top of the map")
   {
      // White over the +z hemisphere (v < 0.5, i.e. upper half of the map)
      auto env = BaseTexture::Create(128, 64, BaseTexture::SRGB);
      REQUIRE(env != nullptr);
      FillSRGB(env.get(), [](unsigned int, unsigned int y) { return y < 32; });

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32);
      REQUIRE(rad != nullptr);
      const uint8_t* const out = static_cast<const uint8_t*>(rad->datac());

      // +z direction maps to v = 0 (row 0), -z to v = 1 (row 31)
      const unsigned int upZ = (0u * 64u + 32u) * 3u;
      const unsigned int downZ = (31u * 64u + 32u) * 3u;
      CHECK(out[upZ] > 200);
      CHECK(out[downZ] < 128);
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

      const auto rad = Renderer::EnvmapPrecalc(env, 64, 32);
      REQUIRE(rad != nullptr);
      CHECK(rad->m_format == BaseTexture::RGB_FP32);

      const float* const out = static_cast<const float*>(rad->datac());
      for (unsigned int i = 0; i < 64u * 32u; ++i)
      {
         CHECK(out[i * 3] == doctest::Approx(0.5f).epsilon(0.02));
         CHECK(out[i * 3 + 1] == doctest::Approx(0.25f).epsilon(0.02));
         CHECK(out[i * 3 + 2] == doctest::Approx(0.125f).epsilon(0.02));
      }
   }
}
