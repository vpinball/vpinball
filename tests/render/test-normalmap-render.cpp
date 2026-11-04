// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "core/VPApp.h"
#include "core/editablereg.h"
#include "parts/pintable.h"
#include "parts/primitive.h"
#include "renderer/Renderer.h"
#include "plugins/MsgPluginManager.h"
using namespace MsgPI;

// Tangent space normal mapping is checked against object space normal mapping, which does not depend on the tangent frame
// (TBN_trafo) evaluated by the shaders. A flat quad (u along +X, v along +Y, normal +Z) is rendered with a generated tangent
// space normal map, then with object space maps holding the same normals expressed through each candidate frame (signs of
// T and B): the expected one must match best, as a wrong frame mirrors the bumps. The expected frame is the DX9 one (normal
// maps authored for VPX 10.8): T = +dP/du and B = +dP/dv, which DX9 renders pixel identical. A render without normal map
// checks that the map has a visible effect.

namespace
{

constexpr int mapSize = 256;
constexpr float quadLeft = 250.f, quadRight = 750.f, quadTop = 800.f, quadBottom = 1300.f, quadZ = 5.f;

string s_backend; // BGFX backend to render with (empty for the default one)
#ifdef ENABLE_BGFX
bgfx::RendererType::Enum s_lastRenderer = bgfx::RendererType::Count;
#endif

// Tangent space normal of the generated map: bumps varying independently along u and v, so that a wrong sign of either T or B is visible
Vertex3Ds TangentNormal(const float u, const float v)
{
   Vertex3Ds n(0.7f * sinf(2.f * (float)M_PI * 3.f * u), 0.7f * sinf(2.f * (float)M_PI * 2.f * v + 0.5f), 1.f);
   n.Normalize();
   return n;
}

// Normal map texel encoding, the inverse of the shaders' decoding (tex * 255/127 - 128/127)
uint8_t Encode(const float c) { return static_cast<uint8_t>(clamp(roundf(c * 127.f + 128.f), 0.f, 255.f)); }

enum class NormalMap
{
   None,
   TangentSpace,
   ObjectSpace
};

struct FrameSigns
{
   float t, b; // Signs of T (+dP/du) and B (+dP/dv) used to convert the map to object space
};

// Writes the normal map (tangent space, or object space through the given frame) to a file for the table to import it
std::filesystem::path WriteNormalMap(const NormalMap type, const FrameSigns signs)
{
   // Tagged as sRGB as only those are saved; the bytes are stored as is, and normal maps are sampled without sRGB decoding
   const std::shared_ptr<BaseTexture> map = BaseTexture::Create(mapSize, mapSize, BaseTexture::SRGB);
   uint8_t* const data = static_cast<uint8_t*>(map->data());
   for (int y = 0; y < mapSize; y++)
      for (int x = 0; x < mapSize; x++)
      {
         const Vertex3Ds tn = TangentNormal(((float)x + 0.5f) / (float)mapSize, ((float)y + 0.5f) / (float)mapSize);
         // The quad spans +X along u and +Y along v and faces +Z: the object space normal is T * tn.x + B * tn.y + N * tn.z,
         // stored with a negated Z as the shaders decode object space maps as (x, y, -z) (Blender export convention)
         const Vertex3Ds n = type == NormalMap::TangentSpace ? tn : Vertex3Ds(signs.t * tn.x, signs.b * tn.y, -tn.z);
         uint8_t* const texel = data + y * map->pitch() + x * 3;
         texel[0] = Encode(n.x);
         texel[1] = Encode(n.y);
         texel[2] = Encode(n.z);
      }
   const std::filesystem::path path = GetTestTmpDir() / (type == NormalMap::TangentSpace ? "normalmap-tangent.png"s : "normalmap-object.png"s);
   REQUIRE(map->Save(path));
   return path;
}

// Uniform light grey base color: normal mapping is only applied to textured parts
std::filesystem::path WriteBaseColor()
{
   const std::shared_ptr<BaseTexture> image = BaseTexture::Create(16, 16, BaseTexture::SRGB);
   memset(image->data(), 200, image->pitch() * image->height());
   const std::filesystem::path path = GetTestTmpDir() / "normalmap-base.png"s;
   REQUIRE(image->Save(path));
   return path;
}

struct QuadCapture
{
   std::shared_ptr<BaseTexture> image;
   int x0, y0, x1, y1; // Screen area inside the quad
};

// Renders the default table with the normal mapped quad, and returns the screenshot with the screen area inside the quad
QuadCapture CaptureQuad(const string& screenshot, const NormalMap type, const FrameSigns signs = { 1.f, 1.f })
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

   Primitive* const quad = static_cast<Primitive*>(EditableRegistry::CreateAndInit(ItemTypeEnum::eItemPrimitive, table, 0.f, 0.f));
   REQUIRE(quad != nullptr);
   quad->SetName("NormalMapTestQuad"s);
   quad->m_d.m_use3DMesh = true;
   quad->m_d.m_toy = true;
   quad->m_d.m_collidable = false;
   quad->m_d.m_staticRendering = false;
   quad->m_d.m_disableLightingBelow = 1.f;
   quad->m_d.m_vSize.Set(1.f, 1.f, 1.f);
   quad->m_mesh.m_vertices.resize(4);
   for (unsigned int i = 0; i < 4; i++)
   {
      Vertex3D_NoTex2& v = quad->m_mesh.m_vertices[i];
      const bool right = (i & 1) != 0, bottom = (i & 2) != 0;
      v.x = right ? quadRight : quadLeft;
      v.y = bottom ? quadBottom : quadTop;
      v.z = quadZ;
      v.tu = right ? 1.f : 0.f;
      v.tv = bottom ? 1.f : 0.f;
      v.nx = 0.f;
      v.ny = 0.f;
      v.nz = 1.f;
   }
   quad->m_mesh.m_indices = { 0, 1, 2, 2, 1, 3 };
   quad->m_mesh.m_validBounds = false;
   REQUIRE(table->ImportImage(WriteBaseColor(), "NormalMapTestBase"s) != nullptr);
   quad->m_d.m_szImage = "NormalMapTestBase"s;
   if (type != NormalMap::None)
   {
      REQUIRE(table->ImportImage(WriteNormalMap(type, signs), "NormalMapTest"s) != nullptr);
      quad->m_d.m_szNormalMap = "NormalMapTest"s;
      quad->m_d.m_objectSpaceNormalMap = type == NormalMap::ObjectSpace;
   }
   table->AddPart(quad);
   quad->Release();

   struct CaptureState
   {
      std::filesystem::path path;
      bool captureRequested;
      float rect[4];
   } state = { GetAssetPath() / screenshot, false, { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX } };
   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* const state = static_cast<CaptureState*>(context);
      if (state->captureRequested || g_pplayer->m_overall_frames < 60)
         return;
      if (g_pplayer->m_renderer->IsTemporalAccumulationInProgress() && g_pplayer->m_overall_frames < 512)
         return;
      state->captureRequested = true;
      const Matrix3D& mvp = g_pplayer->m_renderer->GetMVP().GetModelViewProj(0);
      for (int corner = 0; corner < 4; corner++)
      {
         const Vertex3Ds ndc = mvp * Vertex3Ds((corner & 1) ? quadRight : quadLeft, (corner & 2) ? quadBottom : quadTop, quadZ);
         const float x = 0.5f + 0.5f * ndc.x, y = 0.5f - 0.5f * ndc.y;
         state->rect[0] = min(state->rect[0], x);
         state->rect[1] = min(state->rect[1], y);
         state->rect[2] = max(state->rect[2], x);
         state->rect[3] = max(state->rect[3], y);
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

   QuadCapture capture;
   capture.image = BaseTexture::CreateFromFile(GetAssetPath() / screenshot);
   REQUIRE(capture.image != nullptr);
   // Stay away from the borders: the projected corners only give the bounding box of the perspective projected quad
   const float w = (float)capture.image->width(), h = (float)capture.image->height();
   const float marginX = 0.15f * (state.rect[2] - state.rect[0]), marginY = 0.15f * (state.rect[3] - state.rect[1]);
   capture.x0 = max(0, (int)((state.rect[0] + marginX) * w));
   capture.x1 = min((int)w, (int)((state.rect[2] - marginX) * w));
   capture.y0 = max(0, (int)((state.rect[1] + marginY) * h));
   capture.y1 = min((int)h, (int)((state.rect[3] - marginY) * h));
   REQUIRE(capture.x0 + 16 < capture.x1);
   REQUIRE(capture.y0 + 16 < capture.y1);
   return capture;
}

// Mean absolute difference of the RGB channels over the quad area
double QuadDifference(const QuadCapture& a, const QuadCapture& b)
{
   REQUIRE(a.image->width() == b.image->width());
   REQUIRE(a.image->height() == b.image->height());
   // Captures may come with or without alpha (a translucent overlay icon in a corner, drawn depending on timing): only RGB is compared
   const auto isSRGB8 = [](const BaseTexture::Format f) { return f == BaseTexture::SRGB || f == BaseTexture::SRGBA; };
   REQUIRE(isSRGB8(a.image->m_format));
   REQUIRE(isSRGB8(b.image->m_format));
   const unsigned int bppA = a.image->pitch() / a.image->width(), bppB = b.image->pitch() / b.image->width();
   const uint8_t* const da = static_cast<const uint8_t*>(a.image->datac());
   const uint8_t* const db = static_cast<const uint8_t*>(b.image->datac());
   double sum = 0.;
   size_t n = 0;
   for (int y = a.y0; y < a.y1; y++)
      for (int x = a.x0; x < a.x1; x++)
         for (int c = 0; c < 3; c++)
         {
            sum += abs((int)da[y * a.image->pitch() + x * bppA + c] - (int)db[y * b.image->pitch() + x * bppB + c]);
            n++;
         }
   return sum / (double)n;
}

void CheckNormalMapping(const string& prefix)
{
   const QuadCapture flat = CaptureQuad(prefix + "-flat-Test.webp", NormalMap::None);
   const QuadCapture tangent = CaptureQuad(prefix + "-tangent-Test.webp", NormalMap::TangentSpace);
   constexpr FrameSigns expected = { 1.f, 1.f }; // DX9 / VPX 10.8 tangent frame
   constexpr FrameSigns candidates[] = { expected, { -1.f, -1.f }, { 1.f, -1.f }, { -1.f, 1.f } };
   double diffs[4];
   for (int i = 0; i < 4; i++)
   {
      const string name = prefix + "-object" + (candidates[i].t > 0.f ? "+T" : "-T") + (candidates[i].b > 0.f ? "+B" : "-B") + "-Test.webp";
      diffs[i] = QuadDifference(tangent, CaptureQuad(name, NormalMap::ObjectSpace, candidates[i]));
   }
   const double effect = QuadDifference(tangent, flat);
   MESSAGE(prefix, ": normal map effect ", effect, ", tangent vs object space through frame +T+B ", diffs[0], " / -T-B ", diffs[1], " / +T-B ", diffs[2], " / -T+B ", diffs[3]);
   CHECK(effect > 2.0); // The normal map must have a visible effect for the check to be meaningful
   for (int i = 1; i < 4; i++)
      CHECK(diffs[0] < diffs[i]); // The expected frame is the closest one
   CHECK(diffs[0] < 0.25 * effect); // and matches the tangent space render
}

}

TEST_CASE("Normal mapping render [render]")
{
#if defined(ENABLE_DX9)
   SUBCASE("Direct3D9") { CheckNormalMapping("test000-normalmap-DX9"s); }
#elif defined(ENABLE_OPENGL)
   SUBCASE("OpenGL") { CheckNormalMapping("test000-normalmap-GL"s); }
#elif defined(ENABLE_BGFX)
   const auto runBGFX = [](const string& backend, const bgfx::RendererType::Enum type, const string& prefix)
   {
      s_backend = backend;
      s_lastRenderer = bgfx::RendererType::Count;
      CheckNormalMapping(prefix);
      s_backend.clear();
      CHECK(s_lastRenderer == type);
   };
   SUBCASE("BGFX-Vulkan") { runBGFX("Vulkan"s, bgfx::RendererType::Vulkan, "test000-normalmap-BGFX-Vulkan"s); }
   SUBCASE("BGFX-OpenGL") { runBGFX("OpenGL"s, bgfx::RendererType::OpenGL, "test000-normalmap-BGFX-GL"s); }
   SUBCASE("BGFX-Direct3D11") { runBGFX("Direct3D11"s, bgfx::RendererType::Direct3D11, "test000-normalmap-BGFX-DX11"s); }
   SUBCASE("BGFX-Direct3D12") { runBGFX("Direct3D12"s, bgfx::RendererType::Direct3D12, "test000-normalmap-BGFX-DX12"s); }
#endif
}
