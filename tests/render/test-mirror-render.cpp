// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "core/VPApp.h"
#include "parts/flasher.h"
#include "parts/pintable.h"
#include "renderer/Renderer.h"
#include "plugins/MsgPluginManager.h"
using namespace MsgPI;

// Playfield flasher displays (legacy DMD, new DMD, ancillary layout with an image and a CRT display) show an asymmetric pattern
// (white column, red, green from left to right). In mirror mode their content must not be mirrored: red must stay left of green.

static constexpr int patternWidth = 32;
static constexpr int patternHeight = 8;
static uint32_t pattern[patternWidth * patternHeight];

static int MSGPIAPI LayoutRender(VPXRenderContext2D* ctx, void* context)
{
   const VPXTexture tex = *static_cast<VPXTexture*>(context);
   // Image on the left half, display on the right half (Pixelated uses the CRT shader, BGFX only)
   ctx->DrawImage(ctx, tex, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, (float)patternWidth, (float)patternHeight, 0.f, 0.f, 0.f, 0.f, 0.f, 0.5f * ctx->srcWidth, ctx->srcHeight);
   ctx->DrawDisplay(ctx, VPXDMDStyle_Pixelated, nullptr, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, tex, 1.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f,
      0.55f * ctx->srcWidth, 0.f, 0.45f * ctx->srcWidth, ctx->srcHeight);
   return 1;
}

struct DisplayRect
{
   float left, top, right, bottom; // Normalized screen coordinates of the bounding box of a display
};

// Renders the default table with the 3 displays and returns their normalized screen bounding boxes
static void CaptureDisplays(const string& screenshotPath, const bool mirror, DisplayRect (&rects)[3])
{
   ResetVPX();
   Settings& settings = g_settingsService.GetAppSettings();
   settings.SetPlayer_Mirror(mirror, false);
   settings.SetDMD_ProfileLegacy(0, true, false);
   settings.SetDMD_ProfileLegacy(1, false, false);
   for (int y = 0; y < patternHeight; y++)
      for (int x = 0; x < patternWidth; x++)
         pattern[y * patternWidth + x] = x < 2 ? 0xFFFFFFFFu : x < patternWidth / 2 ? 0xFF0000FFu : 0xFF00FF00u; // ABGR: white, red, green
   struct CaptureState
   {
      std::filesystem::path path;
      bool captureRequested;
      bool mirrored;
      Flasher* flashers[3];
      DisplayRect* rects;
      VPXTexture layoutTex;
      AncillaryRendererDef renderer;
   } state = { GetAssetPath() / screenshotPath, false, false, {}, rects, nullptr, {} };
   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* state = reinterpret_cast<CaptureState*>(context);
      if (state->layoutTex == nullptr)
      {
         // Make the layout the first ScoreView renderer, the one shown by the EXT_RENDER flasher
         g_pplayer->m_pluginAPI.getAPI().UpdateTexture(&state->layoutTex, patternWidth, patternHeight, VPXTEXFMT_sRGBA8, pattern);
         state->renderer = { "MirrorTest", "MirrorTest", "Mirror test layout", &state->layoutTex, LayoutRender };
         auto& renderers = g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_ScoreView];
         renderers.insert(renderers.begin(), state->renderer);
      }
      for (int i = 0; i < 2; i++)
         BaseTexture::Update(state->flashers[i]->m_dmdFrame, patternWidth, patternHeight, BaseTexture::SRGBA, pattern);
      if (state->captureRequested || g_pplayer->m_overall_frames < 60)
         return;
      if (g_pplayer->m_renderer->IsTemporalAccumulationInProgress() && g_pplayer->m_overall_frames < 512)
         return;
      state->captureRequested = true;
      state->mirrored = g_pplayer->m_tblMirrorEnabled;
      // Project the flasher outlines with the (possibly flipped) model/view/projection used to render the screenshot
      const Matrix3D& mvp = g_pplayer->m_renderer->GetMVP().GetModelViewProj(0);
      for (int i = 0; i < 3; i++)
      {
         const Flasher* const flasher = state->flashers[i];
         DisplayRect& rect = state->rects[i];
         rect = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
         const Vertex2D& minBound = flasher->m_curve.GetMinBound();
         const Vertex2D& maxBound = flasher->m_curve.GetMaxBound();
         for (int corner = 0; corner < 4; corner++)
         {
            const Vertex3Ds ndc = mvp * Vertex3Ds((corner & 1) ? maxBound.x : minBound.x, (corner & 2) ? maxBound.y : minBound.y, flasher->m_d.m_height);
            const float x = 0.5f + 0.5f * ndc.x, y = 0.5f - 0.5f * ndc.y;
            rect = { min(rect.left, x), min(rect.top, y), max(rect.right, x), max(rect.bottom, y) };
         }
      }
      g_pplayer->m_renderer->m_renderDevice->CaptureScreenshot({ g_pplayer->m_playfieldWnd }, { state->path }, [](bool success) { g_pplayer->SetCloseState(Player::CS_STOP_PLAY); });
   };
   CComObject<PinTable>* table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   TestFileFeedback feedback;
   REQUIRE(SUCCEEDED(table->LoadGameFromFilename(GetAssetPath() / "test000-default-table.vpx"s, feedback)));
   const float cx = 0.5f * (table->m_left + table->m_right);
   constexpr float ys[] = { 500.f, 850.f, 1200.f };
   for (int i = 0; i < 3; i++)
   {
      Flasher* const flasher = Flasher::COMCreate();
      flasher->Init(cx, ys[i], false);
      flasher->SetName("MirrorTestDisplay" + std::to_string(i));
      flasher->m_curve.ClearPoints();
      flasher->m_curve.PushPoint(std::make_unique<DragPoint>(&flasher->m_curve, cx - 300.f, ys[i] - 75.f, 0.f, false));
      flasher->m_curve.PushPoint(std::make_unique<DragPoint>(&flasher->m_curve, cx - 300.f, ys[i] + 75.f, 0.f, false));
      flasher->m_curve.PushPoint(std::make_unique<DragPoint>(&flasher->m_curve, cx + 300.f, ys[i] + 75.f, 0.f, false));
      flasher->m_curve.PushPoint(std::make_unique<DragPoint>(&flasher->m_curve, cx + 300.f, ys[i] - 75.f, 0.f, false));
      flasher->m_d.m_height = 30.f;
      flasher->m_d.m_color = RGB(255, 255, 255);
      flasher->m_d.m_addBlend = FlasherData::AB_NONE;
      // Legacy DMD (profile 0), new DMD (profile 1), ScoreView layout
      flasher->m_d.m_renderMode = i < 2 ? FlasherData::DMD : FlasherData::EXT_RENDER;
      flasher->m_d.m_renderStyle = i < 2 ? i : (int)VPXWindowId::VPXWINDOW_ScoreView;
      table->AddPart(flasher);
      flasher->Release();
      state.flashers[i] = flasher;
   }
   LoadProgress loadProgress;
   auto player = std::make_unique<Player>(table, Player::PlayMode::Play, loadProgress);
   const unsigned int msgId = player->m_pluginManager.GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   player->m_pluginManager.GetMsgAPI().SubscribeMsg(player->m_pluginAPI.GetVPXEndPointId(), msgId, onPrepareFrame, &state);
   player->GameLoop();
   if (state.layoutTex)
      player->m_pluginAPI.getAPI().DeleteTexture(state.layoutTex);
   player->m_pluginManager.GetMsgAPI().UnsubscribeMsg(msgId, onPrepareFrame, &state);
   player->m_pluginManager.GetMsgAPI().ReleaseMsgID(msgId);
   player = nullptr;
   table->Release();
   ResetVPX();
   REQUIRE(state.captureRequested);
   CHECK(state.mirrored == mirror);
}

// Checks that a display is visible and shows red left of green
static void CheckDisplay(const BaseTexture& bmp, const DisplayRect& rect, const string& name)
{
   INFO(name);
   // Stay away from the borders as the projected outline is the bounding box of a perspective projected quad
   const float marginX = 0.1f * (rect.right - rect.left), marginY = 0.2f * (rect.bottom - rect.top);
   const int x0 = max(0, (int)((rect.left + marginX) * (float)bmp.width())), x1 = min((int)bmp.width(), (int)((rect.right - marginX) * (float)bmp.width()));
   const int y0 = max(0, (int)((rect.top + marginY) * (float)bmp.height())), y1 = min((int)bmp.height(), (int)((rect.bottom - marginY) * (float)bmp.height()));
   REQUIRE(x0 < x1);
   REQUIRE(y0 < y1);
   const uint8_t* const data = static_cast<const uint8_t*>(bmp.datac());
   const unsigned int bytesPerPixel = bmp.pitch() / bmp.width();
   size_t nRed = 0, nGreen = 0;
   double sumRedX = 0., sumGreenX = 0.;
   for (int y = y0; y < y1; y++)
      for (int x = x0; x < x1; x++)
      {
         const uint8_t* const rgb = data + y * bmp.pitch() + x * bytesPerPixel;
         if (rgb[0] > rgb[1] + 80 && rgb[0] > rgb[2] + 80)
         {
            nRed++;
            sumRedX += x;
         }
         else if (rgb[1] > rgb[0] + 80 && rgb[1] > rgb[2] + 80)
         {
            nGreen++;
            sumGreenX += x;
         }
      }
   const size_t area = (size_t)(x1 - x0) * (size_t)(y1 - y0);
   CHECK(nRed > area / 10);
   CHECK(nGreen > area / 10);
   if (nRed > 0 && nGreen > 0)
      CHECK(sumRedX / (double)nRed < sumGreenX / (double)nGreen);
}

TEST_CASE("Mirrored displays render [render]")
{
   for (const bool mirror : { false, true })
   {
      const string screenshot = mirror ? "test000-displays-mirror-Test.webp"s : "test000-displays-Test.webp"s;
      INFO(screenshot);
      DisplayRect rects[3];
      CaptureDisplays(screenshot, mirror, rects);
      const std::shared_ptr<BaseTexture> bmp = BaseTexture::CreateFromFile(GetAssetPath() / screenshot);
      REQUIRE(bmp != nullptr);
      REQUIRE((bmp->m_format == BaseTexture::RGB || bmp->m_format == BaseTexture::RGBA || bmp->m_format == BaseTexture::SRGB || bmp->m_format == BaseTexture::SRGBA));
      CheckDisplay(*bmp, rects[0], "Legacy DMD");
      CheckDisplay(*bmp, rects[1], "DMD");
      CheckDisplay(*bmp, rects[2], "ScoreView layout");
   }
}
