// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "core/VPApp.h"
#include "parts/ball.h"
#include "parts/light.h"
#include "parts/pintable.h"
#include "renderer/Renderer.h"

#include "plugins/MsgPluginManager.h"
using namespace MsgPI;

// Reflections of the lights on the ball (light selection in Ball::Render, DoBallLight in the ball shaders). The balls are held in
// place, with the environment, the table lights, the table's own lights, the ambient light and bloom disabled, so they only show
// the highlights of the test lights (and their faint diffuse lighting):
// - 3 balls, each with a bulb above and in front of it (its highlight faces the camera), at the same distance and of increasing
//   sizes: the highlight must grow with the bulb size (the bulb covers a larger angle)
// - 1 ball ringed by 10 bulbs that are off, with a lit bulb further away: the lit bulb must still be reflected (8 lights are
//   reflected, selected by their contribution, so lights that are off never take a slot)
// - 1 ball behind an insert (a light at the playfield level, below the ball center): only rendered, also with the playfield
//   reflection probe, where the insert highlight fades into the probe reflection
// The renders are kept (test000-balllights-*-Test.webp) for a visual review.

namespace
{

constexpr float ballRadius = 50.f; // Large balls, for the highlights to cover enough pixels
constexpr float bulbHeight = 150.f, bulbDistance = 100.f;
constexpr int numSizeBalls = 3;
constexpr float bulbScales[numSizeBalls] = { 5.f, 40.f, 120.f }; // Bulb mesh scales (bulb radius 0.45 times this)
constexpr int numBalls = numSizeBalls + 2; // Bulb size balls, ball among the off bulbs, ball next to the insert

string s_backend; // BGFX backend to render with (empty for the default one)
#ifdef ENABLE_BGFX
bgfx::RendererType::Enum s_lastRenderer = bgfx::RendererType::Count;
#endif

Vertex2D BallPos(const PinTable* const table, const int i)
{
   const float cx = 0.5f * (table->m_left + table->m_right);
   if (i < numSizeBalls)
      return { cx + 330.f * (float)(i - 1), 1000.f };
   return { cx + (i == numSizeBalls ? -250.f : 250.f), 1350.f };
}

void AddLight(PinTable* const table, const string& name, const float x, const float y, const float height, const bool bulb, const bool on, const float scale = 20.f)
{
   Light* const light = Light::COMCreate();
   light->Init(x, y, false);
   light->SetName(name);
   light->m_d.m_BulbLight = bulb;
   light->m_d.m_showBulbMesh = false;
   light->m_d.m_showReflectionOnBall = true;
   light->m_d.m_height = height;
   light->m_d.m_meshRadius = scale;
   light->m_d.m_intensity = 10.f;
   light->m_d.m_falloff = 50.f;
   light->m_d.m_falloff_power = 2.f;
   light->m_d.m_color = RGB(255, 255, 255);
   light->m_d.m_color2 = RGB(255, 255, 255);
   light->m_d.m_state = on ? 1.f : 0.f;
   light->m_inPlayState = light->m_d.m_state; // Set from m_d.m_state by Init and on load only
   light->m_d.m_fader = Fader::FADER_NONE;
   light->m_d.m_visible = !bulb; // Bulbs are reflected but not rendered (no halo over the balls), the insert is rendered for the probe
   table->AddPart(light);
   light->Release();
}

struct BallCapture
{
   std::shared_ptr<BaseTexture> image;
   float center[numBalls][2]; // Screen position of the ball centers, normalized
   float radius[numBalls]; // Screen radius, normalized to the image width
};

BallCapture CaptureBalls(const string& screenshot, const bool probe)
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
   table->m_envEmissionScale = 0.f;
   table->m_lightEmissionScale = 0.f;
   table->m_lightAmbient = RGB(0, 0, 0);
   table->m_bloom_strength = 0.f;
   table->m_ballPlayfieldReflectionStrength = probe ? 1.f : 0.f;
   table->m_angletiltMin = table->m_angletiltMax = 0.f;
   for (IEditable* const part : table->GetParts()) // The table's own lights are off
      if (part->GetItemType() == eItemLight)
      {
         static_cast<Light*>(part)->m_d.m_state = 0.f;
         static_cast<Light*>(part)->m_inPlayState = 0.f;
      }

   struct CaptureState
   {
      std::filesystem::path path;
      bool captureRequested;
      BallCapture* capture;
      Ball* balls[numBalls];
      Vertex2D pos[numBalls];
   };
   BallCapture capture {};
   CaptureState state = { GetAssetPath() / screenshot, false, &capture, {}, {} };
   for (int i = 0; i < numBalls; i++)
   {
      state.pos[i] = BallPos(table, i);
      Ball* const ball = Ball::COMCreate();
      ball->Init(state.pos[i].x, state.pos[i].y, false);
      ball->SetName("LightTestBall" + std::to_string(i));
      ball->m_hitBall.m_d.m_pos.z = ballRadius;
      ball->m_hitBall.m_d.m_radius = ballRadius;
      ball->m_d.m_color = RGB(255, 255, 255);
      table->AddPart(ball);
      ball->Release();
      state.balls[i] = ball;
   }
   for (int i = 0; i < numSizeBalls; i++) // Bulbs in front of the balls (towards the camera)
      AddLight(table, "SizeBulb" + std::to_string(i), state.pos[i].x, state.pos[i].y + bulbDistance, bulbHeight, true, true, bulbScales[i]);
   const Vertex2D crowded = state.pos[numSizeBalls];
   for (int i = 0; i < 10; i++)
   {
      const float a = (float)i * (float)(2.0 * M_PI / 10.0);
      AddLight(table, "OffBulb" + std::to_string(i), crowded.x + 80.f * cosf(a), crowded.y + 80.f * sinf(a), bulbHeight, true, false);
   }
   AddLight(table, "LitBulb", crowded.x, crowded.y + 200.f, bulbHeight, true, true);
   const Vertex2D insertBall = state.pos[numSizeBalls + 1];
   AddLight(table, "Insert", insertBall.x, insertBall.y + 80.f, 0.f, false, true);

   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* const state = static_cast<CaptureState*>(context);
      for (int i = 0; i < numBalls; i++) // Held in place
      {
         state->balls[i]->m_hitBall.m_d.m_pos = Vertex3Ds(state->pos[i].x, state->pos[i].y, ballRadius);
         state->balls[i]->m_hitBall.m_d.m_vel = Vertex3Ds(0.f, 0.f, 0.f);
      }
      if (state->captureRequested || g_pplayer->m_overall_frames < 60)
         return;
      if (g_pplayer->m_renderer->IsTemporalAccumulationInProgress() && g_pplayer->m_overall_frames < 512)
         return;
      state->captureRequested = true;
      const Matrix3D& mvp = g_pplayer->m_renderer->GetMVP().GetModelViewProj(0);
      for (int i = 0; i < numBalls; i++)
      {
         const Vertex3Ds c = mvp * Vertex3Ds(state->pos[i].x, state->pos[i].y, ballRadius);
         const Vertex3Ds e = mvp * Vertex3Ds(state->pos[i].x + ballRadius, state->pos[i].y, ballRadius);
         state->capture->center[i][0] = 0.5f + 0.5f * c.x;
         state->capture->center[i][1] = 0.5f - 0.5f * c.y;
         state->capture->radius[i] = 0.5f * fabsf(e.x - c.x);
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

struct HighlightStats
{
   double peak; // Largest luminance (sRGB 8 bit values) over the ball
   double median;
   int area; // Pixels above the middle between the median and the peak
};

HighlightStats BallHighlight(const BallCapture& capture, const int i)
{
   const BaseTexture& image = *capture.image;
   REQUIRE((image.m_format == BaseTexture::SRGB || image.m_format == BaseTexture::SRGBA));
   const unsigned int bpp = image.pitch() / image.width();
   const float w = (float)image.width(), h = (float)image.height();
   const float cx = capture.center[i][0] * w, cy = capture.center[i][1] * h, r = 0.9f * capture.radius[i] * w;
   REQUIRE(r > 6.f);
   vector<double> lums;
   for (int y = max(0, (int)(cy - r)); y < min((int)h, (int)(cy + r) + 1); y++)
      for (int x = max(0, (int)(cx - r)); x < min((int)w, (int)(cx + r) + 1); x++)
      {
         if (((float)x - cx) * ((float)x - cx) + ((float)y - cy) * ((float)y - cy) > r * r)
            continue;
         const uint8_t* const texel = static_cast<const uint8_t*>(image.datac()) + y * image.pitch() + x * bpp;
         lums.push_back(0.2126 * texel[0] + 0.7152 * texel[1] + 0.0722 * texel[2]);
      }
   REQUIRE(lums.size() > 100);
   HighlightStats stats {};
   std::ranges::sort(lums);
   stats.peak = lums.back();
   stats.median = lums[lums.size() / 2];
   const double threshold = 0.5 * (stats.median + stats.peak); // Half way to the peak
   stats.area = (int)std::ranges::count_if(lums, [threshold](const double l) { return l > threshold; });
   return stats;
}

void CheckBallLights(const string& prefix)
{
   {
      const BallCapture capture = CaptureBalls(prefix + "-Test.webp", false);
      HighlightStats stats[numBalls];
      string values;
      for (int i = 0; i < numBalls; i++)
      {
         stats[i] = BallHighlight(capture, i);
         values += (i ? " / "s : ""s) + std::to_string((int)stats[i].peak) + ',' + std::to_string((int)stats[i].median) + ',' + std::to_string(stats[i].area);
      }
      MESSAGE(prefix, " ball highlight peak,median,area (bulb scale 5, 40, 120, crowded, insert), ball radius ", capture.radius[0] * (float)capture.image->width(), " pixels: ", values);
      // The highlight of a bulb grows with its size (it covers a larger angle)
      for (int i = 0; i < numSizeBalls; i++)
         CHECK(stats[i].peak > stats[i].median + 40.);
      CHECK(stats[1].area > stats[0].area);
      CHECK(stats[2].area > 2 * stats[1].area);
      // The lit bulb is reflected, although 10 bulbs that are off are nearer
      CHECK(stats[numSizeBalls].peak > stats[numSizeBalls].median + 40.);
   }
   {
      const BallCapture capture = CaptureBalls(prefix + "-probe-Test.webp", true);
      const HighlightStats stats = BallHighlight(capture, numSizeBalls + 1);
      MESSAGE(prefix, " ball next to an insert, with the playfield reflection probe: peak ", stats.peak, ", median ", stats.median, ", area ", stats.area);
   }
}
}

TEST_CASE("Ball light reflections render [render]")
{
#if defined(ENABLE_DX9)
   SUBCASE("Direct3D9") { CheckBallLights("test000-balllights-DX9"s); }
#elif defined(ENABLE_OPENGL)
   SUBCASE("OpenGL") { CheckBallLights("test000-balllights-GL"s); }
#elif defined(ENABLE_BGFX)
   const auto runBGFX = [](const string& backend, const bgfx::RendererType::Enum type, const string& prefix)
   {
      s_backend = backend;
      s_lastRenderer = bgfx::RendererType::Count;
      CheckBallLights(prefix);
      s_backend.clear();
      CHECK(s_lastRenderer == type);
   };
   SUBCASE("BGFX-Vulkan") { runBGFX("Vulkan"s, bgfx::RendererType::Vulkan, "test000-balllights-BGFX-Vulkan"s); }
   SUBCASE("BGFX-OpenGL") { runBGFX("OpenGL"s, bgfx::RendererType::OpenGL, "test000-balllights-BGFX-GL"s); }
   SUBCASE("BGFX-Direct3D11") { runBGFX("Direct3D11"s, bgfx::RendererType::Direct3D11, "test000-balllights-BGFX-DX11"s); }
   SUBCASE("BGFX-Direct3D12") { runBGFX("Direct3D12"s, bgfx::RendererType::Direct3D12, "test000-balllights-BGFX-DX12"s); }
#endif
}
