// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "core/VPApp.h"
#include "parts/pintable.h"
#include "renderer/Renderer.h"
#include "renderer/RenderView.h"
#include "plugins/MsgPluginManager.h"
using namespace MsgPI;

// Multiple render views attached to areas of a shared render target: 4 views of the same
// table, each covering a quadrant of the playfield backbuffer, must produce the same image.

static void CaptureMultiView(const string& screenshotPath)
{
   struct CaptureState
   {
      std::filesystem::path path;
      bool captureRequested;
   } state = { GetAssetPath() / screenshotPath, false };
   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* state = reinterpret_cast<CaptureState*>(context);
      if (state->captureRequested || g_pplayer->m_overall_frames < 60)
         return;
      if (g_pplayer->m_renderer->IsTemporalAccumulationInProgress() && g_pplayer->m_overall_frames < 512)
         return;
      state->captureRequested = true;
      g_pplayer->m_renderer->m_renderDevice->CaptureScreenshot({ g_pplayer->m_playfieldWnd }, { state->path }, [](bool success) { g_pplayer->SetCloseState(Player::CS_STOP_PLAY); });
   };
   CComObject<PinTable>* table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   TestFileFeedback feedback;
   REQUIRE(SUCCEEDED(table->LoadGameFromFilename(GetAssetPath() / "test000-default-table.vpx"s, feedback)));
   LoadProgress loadProgress;
   auto player = std::make_unique<Player>(table, Player::PlayMode::Play, loadProgress);

   // Attach 4 views to quadrants of the playfield backbuffer: the primary view (top left) plus 3 additional views
   static constexpr float half = 0.5f;
   static constexpr vec4 quadrants[4] = { { 0.f, 0.f, half, half }, { half, 0.f, 1.f, half }, { 0.f, half, half, 1.f }, { half, half, 1.f, 1.f } };
   player->m_renderer->GetPrimaryView()->SetOutputArea(quadrants[0]);
   for (int i = 1; i < 4; i++)
   {
      RenderView* const view = player->m_renderer->AddView(RenderView::Kind::Editor);
      view->m_outputWnd = player->m_playfieldWnd;
      view->SetOutputArea(quadrants[i]);
   }

   const unsigned int msgId = player->m_pluginManager.GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   player->m_pluginManager.GetMsgAPI().SubscribeMsg(player->m_pluginAPI.GetVPXEndPointId(), msgId, onPrepareFrame, &state);
   player->GameLoop();
   player->m_pluginManager.GetMsgAPI().UnsubscribeMsg(msgId, onPrepareFrame, &state);
   player->m_pluginManager.GetMsgAPI().ReleaseMsgID(msgId);
   player = nullptr;
   table->Release();
   ResetVPX();
   CHECK(state.captureRequested);
}

// Checks that a quadrant is not blank (the table render has contrast) and matches the top-left one
static void CheckQuadrant(const BaseTexture& bmp, const int quadrant, const string& name)
{
   INFO(name);
   const int qw = bmp.width() / 2, qh = bmp.height() / 2;
   const int offsetX = (quadrant & 1) * qw, offsetY = (quadrant >> 1) * qh;
   // Stay away from the quadrant borders to avoid edge bleeding artifacts
   const int x0 = (int)(0.05f * (float)qw), x1 = qw - x0, y0 = (int)(0.05f * (float)qh), y1 = qh - y0;
   const uint8_t* const data = static_cast<const uint8_t*>(bmp.datac());
   const unsigned int bytesPerPixel = bmp.pitch() / bmp.width();
   size_t avg = 0, nInvalid = 0;
   uint8_t maxDif = 0, lumMin = 255, lumMax = 0;
   for (int y = y0; y < y1; y++)
      for (int x = x0; x < x1; x++)
      {
         const uint8_t* const ref = data + y * bmp.pitch() + x * bytesPerPixel;
         const uint8_t* const pix = data + (y + offsetY) * bmp.pitch() + (x + offsetX) * bytesPerPixel;
         for (int c = 0; c < 3; c++)
         {
            const uint8_t dif = pix[c] > ref[c] ? pix[c] - ref[c] : ref[c] - pix[c];
            avg += dif;
            if (dif > maxDif)
               maxDif = dif;
            if (dif > 64)
               nInvalid++;
         }
         const uint8_t lum = (uint8_t)((pix[0] + pix[1] + pix[2]) / 3);
         lumMin = min(lumMin, lum);
         lumMax = max(lumMax, lum);
      }
   CHECK(lumMax - lumMin > 64); // The view must have rendered something (not a flat or blank area)
   const double fAvg = (double)avg / (double)((x1 - x0) * (y1 - y0) * 3);
   CHECK(fAvg <= 3.0);
   CHECK(!(nInvalid > 5 && maxDif > 64));
}

TEST_CASE("Multiple render views on one render target [render]")
{
   ResetVPX();
   const string screenshot = "test000-multiview-Test.webp"s;
   CaptureMultiView(screenshot);
   const std::shared_ptr<BaseTexture> bmp = BaseTexture::CreateFromFile(GetAssetPath() / screenshot);
   REQUIRE(bmp != nullptr);
   REQUIRE((bmp->m_format == BaseTexture::RGB || bmp->m_format == BaseTexture::RGBA || bmp->m_format == BaseTexture::SRGB || bmp->m_format == BaseTexture::SRGBA));
   CheckQuadrant(*bmp, 1, "Top right view");
   CheckQuadrant(*bmp, 2, "Bottom left view");
   CheckQuadrant(*bmp, 3, "Bottom right view");
}
