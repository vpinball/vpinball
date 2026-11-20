// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "core/VPApp.h"
#include "core/editablereg.h"
#include "parts/Material.h"
#include "parts/pintable.h"
#include "parts/primitive.h"
#include "renderer/Renderer.h"
#include "utils/color.h"

#include <fstream>
#include "plugins/MsgPluginManager.h"
using namespace MsgPI;

// Environment lighting over the material roughness range: spheres of each roughness from 0 (the roughest in VPX) to 1 (the sharpest),
// reflecting a generated high contrast environment map (LDR, then HDR with a bright sun). The metal spheres (full strength glossy
// reflection) must sharpen steadily with the roughness: their contrast is checked not to decrease towards the sharper ones. The
// plastic spheres (white diffuse base under the 8% dielectric glossy layer) are only rendered. The renders are kept
// (test000-envmap-*-Test.webp) for a visual review: stepping between roughnesses, blockiness, pole artifacts.
// Two more sharp dielectric spheres, alike but for their Edge (1, then 0), check the Fresnel of the glossy reflection: their rims
// must differ, their centers not.
// Point lights, ambient and bloom are disabled so the spheres only show the environment lighting.

namespace
{

constexpr unsigned int envWidth = 512, envHeight = 256;
constexpr int numSpheres = 11; // Roughness 0, 0.1, ..., 1
constexpr int spheresPerLine = 6; // Roughness 0 to 0.5 on the first line of a material, 0.6 to 1 on the second
constexpr float sphereRadius = 55.f, sphereSpacing = 140.f, sphereLeft = 150.f, sphereZ = sphereRadius + 2.f;
constexpr float materialY[2] = { 760.f, 1060.f }; // Metal, plastic
constexpr float edgeSphereY = 1360.f, edgeSphereX[2] = { sphereLeft + 2.f * sphereSpacing, sphereLeft + 3.f * sphereSpacing }; // Edge 1, Edge 0

string s_backend; // BGFX backend to render with (empty for the default one)
#ifdef ENABLE_BGFX
bgfx::RendererType::Enum s_lastRenderer = bgfx::RendererType::Count;
#endif

Vertex2D SpherePos(const int material, const int i)
{
   const int line = i / spheresPerLine, column = i % spheresPerLine;
   return { sphereLeft + sphereSpacing * ((float)column + 0.5f * (float)line), materialY[material] + sphereSpacing * (float)line };
}

// Linear radiance of the environment in a direction (colatitude theta from +Z, longitude phi, as Renderer::EnvmapPrecalc): a sky
// with bright vertical stripes of increasing width, a sun, a bright horizon line, a colored checker below and a blue spot at the nadir
Vertex3Ds EnvRadiance(const float theta, const float phi, const bool hdr)
{
   const Vertex3Ds dir(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
   const float thetaDeg = theta * (float)(180.0 / M_PI), phiDeg = phi * (float)(180.0 / M_PI);
   const float bright = hdr ? 4.f : 1.f;
   Vertex3Ds sunDir(sinf(0.7f) * cosf(2.2f), sinf(0.7f) * sinf(2.2f), cosf(0.7f));
   if (dir.Dot(sunDir) > cosf(4.f * (float)(M_PI / 180.0)))
      return hdr ? Vertex3Ds(60.f, 55.f, 45.f) : Vertex3Ds(1.f, 1.f, 1.f);
   if (fabsf(thetaDeg - 90.f) < 1.5f)
      return Vertex3Ds(1.f, 0.8f, 0.2f) * (hdr ? 2.f : 1.f);
   if (thetaDeg < 90.f)
   {
      // 12 stripes, every 30 degrees of longitude, from 1 to 7.6 degrees wide
      if (thetaDeg > 15.f && thetaDeg < 80.f)
      {
         const float p = fmodf(phiDeg + 720.f, 360.f);
         const int k = (int)(p / 30.f);
         if (p - (float)k * 30.f < 1.f + 0.6f * (float)k)
            return Vertex3Ds(bright, bright, bright);
      }
      const float z = dir.z;
      return Vertex3Ds(0.04f, 0.07f, 0.15f) * (0.5f + z);
   }
   if (thetaDeg > 168.f)
      return Vertex3Ds(0.05f, 0.05f, 0.5f);
   const int cell = (int)floorf(fmodf(phiDeg + 720.f, 360.f) / 30.f) + (int)floorf(thetaDeg / 15.f);
   return (cell & 1) ? Vertex3Ds(0.3f, 0.03f, 0.03f) : Vertex3Ds(0.03f, 0.25f, 0.05f);
}

float EnvTexelTheta(const unsigned int y) { return ((float)y + 0.5f) / (float)envHeight * (float)M_PI; }
float EnvTexelPhi(const unsigned int x) { return ((float)x + 0.5f) / (float)envWidth * (float)(2.0 * M_PI) + (float)M_PI; }

std::filesystem::path WriteLDREnvironment()
{
   const std::shared_ptr<BaseTexture> image = BaseTexture::Create(envWidth, envHeight, BaseTexture::SRGB);
   uint8_t* const data = static_cast<uint8_t*>(image->data());
   for (unsigned int y = 0; y < envHeight; y++)
      for (unsigned int x = 0; x < envWidth; x++)
      {
         const Vertex3Ds c = EnvRadiance(EnvTexelTheta(y), EnvTexelPhi(x), false);
         uint8_t* const texel = data + y * image->pitch() + x * 3;
         texel[0] = sRGB8(clamp(c.x, 0.f, 1.f));
         texel[1] = sRGB8(clamp(c.y, 0.f, 1.f));
         texel[2] = sRGB8(clamp(c.z, 0.f, 1.f));
      }
   const std::filesystem::path path = GetTestTmpDir() / "envmap-ldr.png"s;
   REQUIRE(image->Save(path));
   return path;
}

// Radiance HDR file, flat (not run length encoded) RGBE scanlines
std::filesystem::path WriteHDREnvironment()
{
   const std::filesystem::path path = GetTestTmpDir() / "envmap-hdr.hdr"s;
   std::ofstream file(path, std::ios::binary);
   REQUIRE(file.is_open());
   file << "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y " << envHeight << " +X " << envWidth << '\n';
   vector<uint8_t> scanline(envWidth * 4);
   for (unsigned int y = 0; y < envHeight; y++)
   {
      for (unsigned int x = 0; x < envWidth; x++)
      {
         const Vertex3Ds c = EnvRadiance(EnvTexelTheta(y), EnvTexelPhi(x), true);
         const float v = max(c.x, max(c.y, c.z));
         uint8_t* const rgbe = scanline.data() + x * 4;
         if (v < 1e-32f)
            rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
         else
         {
            int e;
            const float scale = frexpf(v, &e) * 256.f / v;
            rgbe[0] = static_cast<uint8_t>(c.x * scale);
            rgbe[1] = static_cast<uint8_t>(c.y * scale);
            rgbe[2] = static_cast<uint8_t>(c.z * scale);
            rgbe[3] = static_cast<uint8_t>(e + 128);
         }
      }
      // A scanline starting with 2, 2 is read as run length encoded
      if (scanline[0] == 2 && scanline[1] == 2)
         scanline[0] = 3;
      file.write(reinterpret_cast<const char*>(scanline.data()), static_cast<std::streamsize>(scanline.size()));
   }
   file.close();
   REQUIRE(!file.fail());
   return path;
}

void AddSphere(PinTable* const table, const string& name, const string& material, const float cx, const float cy)
{
   constexpr int segments = 48, rings = 24;
   Primitive* const sphere = static_cast<Primitive*>(EditableRegistry::CreateAndInit(ItemTypeEnum::eItemPrimitive, table, 0.f, 0.f));
   REQUIRE(sphere != nullptr);
   sphere->SetName(name);
   sphere->m_d.m_use3DMesh = true;
   sphere->m_d.m_toy = true;
   sphere->m_d.m_collidable = false;
   sphere->m_d.m_staticRendering = false;
   sphere->m_d.m_vSize.Set(1.f, 1.f, 1.f);
   sphere->m_d.m_szMaterial = material;
   sphere->m_d.m_szImage.clear();
   sphere->m_mesh.m_vertices.resize((segments + 1) * (rings + 1));
   for (int r = 0; r <= rings; r++)
      for (int s = 0; s <= segments; s++)
      {
         const float theta = (float)r / (float)rings * (float)M_PI, phi = (float)s / (float)segments * (float)(2.0 * M_PI);
         Vertex3D_NoTex2& v = sphere->m_mesh.m_vertices[r * (segments + 1) + s];
         v.nx = sinf(theta) * cosf(phi);
         v.ny = sinf(theta) * sinf(phi);
         v.nz = cosf(theta);
         v.x = cx + sphereRadius * v.nx;
         v.y = cy + sphereRadius * v.ny;
         v.z = sphereZ + sphereRadius * v.nz;
         v.tu = (float)s / (float)segments;
         v.tv = (float)r / (float)rings;
      }
   sphere->m_mesh.m_indices.clear();
   for (int r = 0; r < rings; r++)
      for (int s = 0; s < segments; s++)
      {
         const unsigned int a = r * (segments + 1) + s, b = a + segments + 1;
         sphere->m_mesh.m_indices.insert(sphere->m_mesh.m_indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
      }
   sphere->m_mesh.m_validBounds = false;
   table->AddPart(sphere);
   sphere->Release();
}

struct SphereCapture
{
   std::shared_ptr<BaseTexture> image;
   float center[2][numSpheres][2]; // Screen position of the sphere centers (metal, plastic), normalized
   float radius[2][numSpheres]; // Screen radius, normalized to the image width
   float edgeCenter[2][2]; // Same for the Edge 1 and Edge 0 spheres
   float edgeRadius[2];
};

SphereCapture CaptureSpheres(const string& screenshot, const bool hdr)
{
   ResetVPX();
#ifdef ENABLE_BGFX
   if (!s_backend.empty())
      g_settingsService.GetAppSettings().SetPlayer_GfxBackend(s_backend, false);
#endif
   CComObject<PinTable>* table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   TestFileFeedback feedback;
   REQUIRE(SUCCEEDED(table->LoadGameFromFilename(GetAssetPath() / "test000-default-table.vpx"s, feedback)));

   REQUIRE(table->ImportImage(hdr ? WriteHDREnvironment() : WriteLDREnvironment(), "EnvMapTest"s) != nullptr);
   table->m_envImage = "EnvMapTest"s;
   table->m_envEmissionScale = 1.f;
   table->m_lightEmissionScale = 0.f;
   table->m_lightAmbient = RGB(0, 0, 0);
   table->m_bloom_strength = 0.f;

   for (int i = 0; i < numSpheres; i++)
   {
      const float roughness = (float)i / (float)(numSpheres - 1);
      Material* const metal = new Material(Material::METAL, 0.f, roughness, 1.f, 0.05f, 1.f, 1.f, 1.f, RGB(255, 255, 255), RGB(0, 0, 0), RGB(0, 0, 0), false, 0.f, 0.f, 0.f, 0.f, RGB(255, 255, 255));
      metal->m_name = "EnvMetal" + std::to_string(i);
      table->AddMaterial(metal);
      Material* const plastic = new Material(Material::BASIC, 0.f, roughness, 1.f, 0.05f, 1.f, 1.f, 1.f, RGB(200, 200, 200), RGB(255, 255, 255), RGB(0, 0, 0), false, 0.f, 0.f, 0.f, 0.f, RGB(255, 255, 255));
      plastic->m_name = "EnvPlastic" + std::to_string(i);
      table->AddMaterial(plastic);
      const Vertex2D metalPos = SpherePos(0, i), plasticPos = SpherePos(1, i);
      AddSphere(table, "EnvSphereMetal" + std::to_string(i), metal->m_name, metalPos.x, metalPos.y);
      AddSphere(table, "EnvSpherePlastic" + std::to_string(i), plastic->m_name, plasticPos.x, plasticPos.y);
   }
   for (int i = 0; i < 2; i++)
   {
      Material* const dielectric = new Material(Material::BASIC, 0.f, 1.f, 1.f, 0.05f, i == 0 ? 1.f : 0.f, 1.f, 1.f, RGB(20, 20, 20), RGB(30, 30, 30), RGB(0, 0, 0), false, 0.f, 0.f, 0.f, 0.f, RGB(255, 255, 255));
      dielectric->m_name = "EnvEdge" + std::to_string(i);
      table->AddMaterial(dielectric);
      AddSphere(table, "EnvSphereEdge" + std::to_string(i), dielectric->m_name, edgeSphereX[i], edgeSphereY);
   }

   struct CaptureState
   {
      std::filesystem::path path;
      bool captureRequested;
      SphereCapture* capture;
   };
   SphereCapture capture {};
   CaptureState state = { GetAssetPath() / screenshot, false, &capture };
   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* const state = static_cast<CaptureState*>(context);
      if (state->captureRequested || g_pplayer->m_overall_frames < 60)
         return;
      if (g_pplayer->m_renderer->IsTemporalAccumulationInProgress() && g_pplayer->m_overall_frames < 512)
         return;
      state->captureRequested = true;
      const Matrix3D& mvp = g_pplayer->m_renderer->GetMVP().GetModelViewProj(0);
      for (int row = 0; row < 2; row++)
         for (int i = 0; i < numSpheres; i++)
         {
            const Vertex2D pos = SpherePos(row, i);
            const Vertex3Ds c = mvp * Vertex3Ds(pos.x, pos.y, sphereZ);
            const Vertex3Ds e = mvp * Vertex3Ds(pos.x + sphereRadius, pos.y, sphereZ);
            state->capture->center[row][i][0] = 0.5f + 0.5f * c.x;
            state->capture->center[row][i][1] = 0.5f - 0.5f * c.y;
            state->capture->radius[row][i] = 0.5f * fabsf(e.x - c.x);
         }
      for (int i = 0; i < 2; i++)
      {
         const Vertex3Ds c = mvp * Vertex3Ds(edgeSphereX[i], edgeSphereY, sphereZ);
         const Vertex3Ds e = mvp * Vertex3Ds(edgeSphereX[i] + sphereRadius, edgeSphereY, sphereZ);
         state->capture->edgeCenter[i][0] = 0.5f + 0.5f * c.x;
         state->capture->edgeCenter[i][1] = 0.5f - 0.5f * c.y;
         state->capture->edgeRadius[i] = 0.5f * fabsf(e.x - c.x);
      }
      g_pplayer->m_renderer->m_renderDevice->CaptureScreenshot({ g_pplayer->m_playfieldWnd }, { state->path },
         [](bool success)
         {
#ifdef ENABLE_BGFX
            s_lastRenderer = bgfx::getRendererType();
#endif
            g_pplayer->SetCloseState(Player::CS_STOP_PLAY);
         });
   };
   LoadProgress loadProgress;
   auto player = std::make_unique<Player>(table, Player::PlayMode::Play, loadProgress);
   const unsigned int msgId = player->m_pluginManager.GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   player->m_pluginManager.GetMsgAPI().SubscribeMsg(player->m_pluginAPI.GetVPXEndPointId(), msgId, onPrepareFrame, &state);
   player->GameLoop();
   player->m_pluginManager.GetMsgAPI().UnsubscribeMsg(msgId, onPrepareFrame, &state);
   player->m_pluginManager.GetMsgAPI().ReleaseMsgID(msgId);
   player = nullptr;
   table->Release();
   ResetVPX();
   REQUIRE(state.captureRequested);

   capture.image = BaseTexture::CreateFromFile(GetAssetPath() / screenshot);
   REQUIRE(capture.image != nullptr);
   return capture;
}

// Mean and standard deviation of the luminance (sRGB 8 bit values) over the ring between rInner and rOuter (fractions of the sphere
// radius) of a sphere of normalized center and radius
std::pair<double, double> SphereLuminance(const SphereCapture& capture, const float center[2], const float radius, const float rInner, const float rOuter)
{
   const BaseTexture& image = *capture.image;
   REQUIRE((image.m_format == BaseTexture::SRGB || image.m_format == BaseTexture::SRGBA));
   const unsigned int bpp = image.pitch() / image.width();
   const float w = (float)image.width(), h = (float)image.height();
   const float cx = center[0] * w, cy = center[1] * h, r0 = rInner * radius * w, r1 = rOuter * radius * w;
   REQUIRE(radius * w > 10.f);
   double sum = 0., sum2 = 0.;
   size_t n = 0;
   for (int y = max(0, (int)(cy - r1)); y < min((int)h, (int)(cy + r1) + 1); y++)
      for (int x = max(0, (int)(cx - r1)); x < min((int)w, (int)(cx + r1) + 1); x++)
      {
         const float d2 = ((float)x - cx) * ((float)x - cx) + ((float)y - cy) * ((float)y - cy);
         if (d2 > r1 * r1 || d2 < r0 * r0)
            continue;
         const uint8_t* const texel = static_cast<const uint8_t*>(image.datac()) + y * image.pitch() + x * bpp;
         const double l = 0.2126 * texel[0] + 0.7152 * texel[1] + 0.0722 * texel[2];
         sum += l;
         sum2 += l * l;
         n++;
      }
   REQUIRE(n > 50);
   const double mean = sum / (double)n;
   return { mean, sqrt(max(0., sum2 / (double)n - mean * mean)) };
}

// Standard deviation of the luminance over the inner part of a sphere (away from its silhouette)
double SphereContrast(const SphereCapture& capture, const int row, const int i)
{
   return SphereLuminance(capture, capture.center[row][i], capture.radius[row][i], 0.f, 0.8f).second;
}

void CheckEnvironmentRoughness(const string& prefix, const bool hdr)
{
   const SphereCapture capture = CaptureSpheres(prefix + (hdr ? "-hdr"s : "-ldr"s) + "-Test.webp", hdr);
   for (int row = 0; row < 2; row++)
   {
      double contrast[numSpheres];
      string values;
      for (int i = 0; i < numSpheres; i++)
      {
         contrast[i] = SphereContrast(capture, row, i);
         values += (i ? " / "s : ""s) + std::to_string(contrast[i]);
      }
      const string label = prefix + (hdr ? " HDR"s : " LDR"s) + (row == 0 ? " metal"s : " plastic"s);
      MESSAGE(label, " sphere contrast for roughness 0 to 1: ", values);
      if (row != 0)
         continue;
      // The reflection sharpens with the roughness (small tolerance for the slightly different view of each sphere)
      for (int i = 1; i < numSpheres; i++)
         CHECK(contrast[i] >= 0.9 * contrast[i - 1] - 0.5);
      CHECK(contrast[numSpheres - 1] > 2.0 * contrast[0]); // The roughness must have a visible effect for the check to be meaningful
   }

   // Fresnel: towards the silhouette, the glossy reflection rises to the Edge value (1) or falls to it (0); head on, it is the glossy color
   double rim[2], center[2];
   for (int i = 0; i < 2; i++)
   {
      rim[i] = SphereLuminance(capture, capture.edgeCenter[i], capture.edgeRadius[i], 0.85f, 0.95f).first;
      center[i] = SphereLuminance(capture, capture.edgeCenter[i], capture.edgeRadius[i], 0.f, 0.4f).first;
   }
   MESSAGE(prefix, (hdr ? " HDR"s : " LDR"s), " Edge 1 / Edge 0 sphere rim: ", rim[0], " / ", rim[1], ", center: ", center[0], " / ", center[1]);
   CHECK(rim[0] > rim[1] + 3.0); // The diffuse base dominates the rim: +7 (LDR) and +6 (HDR) with the Fresnel, +0 without
   CHECK(fabs(center[0] - center[1]) < 0.15 * max(center[0], center[1]) + 2.0);
}
}

TEST_CASE("Environment lighting roughness render [render]")
{
#if defined(ENABLE_DX9)
   SUBCASE("Direct3D9")
   {
      CheckEnvironmentRoughness("test000-envmap-DX9"s, false);
      CheckEnvironmentRoughness("test000-envmap-DX9"s, true);
   }
#elif defined(ENABLE_OPENGL)
   SUBCASE("OpenGL")
   {
      CheckEnvironmentRoughness("test000-envmap-GL"s, false);
      CheckEnvironmentRoughness("test000-envmap-GL"s, true);
   }
#elif defined(ENABLE_BGFX)
   const auto runBGFX = [](const string& backend, const bgfx::RendererType::Enum type, const string& prefix)
   {
      s_backend = backend;
      s_lastRenderer = bgfx::RendererType::Count;
      CheckEnvironmentRoughness(prefix, false);
      CheckEnvironmentRoughness(prefix, true);
      s_backend.clear();
      CHECK(s_lastRenderer == type);
   };
   SUBCASE("BGFX-Vulkan") { runBGFX("Vulkan"s, bgfx::RendererType::Vulkan, "test000-envmap-BGFX-Vulkan"s); }
   SUBCASE("BGFX-OpenGL") { runBGFX("OpenGL"s, bgfx::RendererType::OpenGL, "test000-envmap-BGFX-GL"s); }
   SUBCASE("BGFX-Direct3D11") { runBGFX("Direct3D11"s, bgfx::RendererType::Direct3D11, "test000-envmap-BGFX-DX11"s); }
   SUBCASE("BGFX-Direct3D12") { runBGFX("Direct3D12"s, bgfx::RendererType::Direct3D12, "test000-envmap-BGFX-DX12"s); }
#endif
}
