// license:GPLv3+

#include "core/stdafx.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "vpx-test.h"

#include <filesystem>

#include "core/VPApp.h"
#include "core/AppCommands.h"

#include "plugins/MsgPluginManager.h"

using namespace MsgPI;

static unsigned int onPrepareFrameMsgId = 0;

void AddOnPrepareFrameHandler(msgpi_msg_callback onPrepareFrame, void* context)
{
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   unsigned int vpxEndpoint = msgApi.GetPluginEndpoint("vpx");
   msgApi.SubscribeMsg(vpxEndpoint, onPrepareFrameMsgId, onPrepareFrame, context);
}

void RemoveOnPrepareFrameHandler(msgpi_msg_callback onPrepareFrame)
{
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   msgApi.UnsubscribeMsg(onPrepareFrameMsgId, onPrepareFrame);
}

#ifdef ENABLE_BGFX
bgfx::RendererType::Enum lastBgfxRenderer = bgfx::RendererType::Count;
bgfx::RendererType::Enum GetLastRenderer()
{
   return lastBgfxRenderer;
}
#endif

std::filesystem::path GetAssetPath()
{
   return g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Root, "test-assets");
}

bool CheckMatchingBitmaps(const string& filePath1, const string& filePath2)
{
   auto bmp1 = FileExists(GetAssetPath() / filePath1) ? BaseTexture::CreateFromFile(GetAssetPath() / filePath1) : nullptr;
   auto bmp2 = FileExists(GetAssetPath() / filePath2) ? BaseTexture::CreateFromFile(GetAssetPath() / filePath2) : nullptr;
   bool failed = (!bmp1 || !bmp2 || bmp1->width() != bmp2->width() || bmp1->height() != bmp2->height() || bmp1->m_format != bmp2->m_format);
   if (!failed && ((bmp1->m_format != BaseTexture::SRGB) || (bmp2->m_format != BaseTexture::SRGB)))
   {
      MESSAGE("Invalid bitmap format");
      failed = true;
   }
   if (!failed)
   {
      // This would be too strict as we are applying some fuzzy effects like dither and we save the screenshots with a lossy format
      // failed |= memcmp(bmp1->datac(), bmp2->datac(), bmp1->pitch() * bmp1->height()) != 0;
      const uint8_t* const __restrict data1 = reinterpret_cast<const uint8_t*>(bmp1->datac());
      const uint8_t* const __restrict data2 = reinterpret_cast<const uint8_t*>(bmp2->datac());
      const size_t dataSize = bmp1->height() * bmp1->pitch();
      uint8_t max = 0;
      size_t avg = 0;
      size_t nInvalid = 0;
      for (size_t i = 0; i < dataSize; ++i)
      {
         uint8_t dif = data1[i] > data2[i] ? data1[i] - data2[i] : data2[i] - data1[i];
         avg += dif;
         if (dif > max)
            max = dif;
         if (dif > 64)
            nInvalid++;
      }
      bool saveDiff = false;
      const double fAvg = static_cast<double>(avg) / static_cast<double>(bmp1->width() * bmp1->height());
      if ((fAvg > 3.0) || (nInvalid > 5 && max > 64))
      {
         MESSAGE("Bitmaps do not match: avg=", fAvg, " / max=", (int)max, " / nInvalid=", (int)nInvalid, " ('", filePath1, "' vs '", filePath2, "'), a diff has been generated");
         failed = true;
         saveDiff = true;
      }
      else if (fAvg > 1.0)
      {
         MESSAGE("Bitmaps are similar: avg=", fAvg, " / max=", (int)max, " / nInvalid=", (int)nInvalid, " ('", filePath1, "' vs '", filePath2, "'), a diff has been generated");
         saveDiff = true;
      }
      const string ext = extension_from_path(filePath1);
      const std::filesystem::path diffPath = GetAssetPath() / (filePath1.substr(0, filePath1.size() - ext.size() - 1) + "-diff." + ext);
      if (saveDiff)
      {
         std::shared_ptr<BaseTexture> diff = BaseTexture::Create(bmp1->width(), bmp1->height(), BaseTexture::SRGB);
         uint8_t* const __restrict diffData = (uint8_t*)diff->data();
         for (size_t i = 0; i < dataSize; ++i)
         {
            uint8_t dif = data1[i] > data2[i] ? data1[i] - data2[i] : data2[i] - data1[i];
            diffData[i] = dif > 64 ? 255 : dif * 4; // Scale to 0-255
         }
         diff->Save(diffPath);
      }
      else if (FileExists(diffPath))
      {
         MESSAGE("Removing previous diff file '", diffPath, "'");
         std::error_code ec;
         std::filesystem::remove(diffPath, ec);
         if (ec)
            MESSAGE("Failed to remove previous diff file '", diffPath, "': ", ec.message());
      }
   }
   return !failed;
}

void CaptureRender(const string& tablePath, const string& screenshotPath)
{
   struct CaptureState
   {
      std::filesystem::path tmpScreenshotPath;
      bool done;
   } state = { GetAssetPath() / screenshotPath, false };
   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* state = reinterpret_cast<CaptureState*>(context);
      if (g_pplayer->m_overall_frames == 25)
         g_pplayer->m_renderer->m_renderDevice->CaptureScreenshot({ g_pplayer->m_playfieldWnd }, { state->tmpScreenshotPath },
            [state](bool success)
            {
               #ifdef ENABLE_BGFX
               lastBgfxRenderer = bgfx::getRendererType();
               #endif
               g_pplayer->SetCloseState(Player::CS_STOP_PLAY);
               state->done = true;
            });
   };
   AddOnPrepareFrameHandler(onPrepareFrame, &state);

   CComObject<PinTable>* table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   table->LoadGameFromFilename((GetAssetPath() / tablePath).string());
   auto player = std::make_unique<Player>(table, Player::PlayMode::Play);
   player->GameLoop();
   player = nullptr;
   table->Release();

   RemoveOnPrepareFrameHandler(onPrepareFrame);
}

void ResetVPX()
{
   // Reset settings
   g_app->m_settings.Reset();
   Settings& settings = g_app->m_settings;
   settings.SetPlayerVR_AskToTurnOn(2, false);
   settings.SetPlayer_PlayfieldFullScreen(0, false);
   settings.SetPlayer_PlayfieldWidth(1920, false);
   settings.SetPlayer_PlayfieldHeight(1080, false);
   settings.SetPlayer_NumberOfTimesToShowTouchMessage(0, false);
   settings.SetPlayer_DisableAO(true, false);
}


extern "C" int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
   SDL_SetHint(SDL_HINT_WINDOW_ALLOW_TOPMOST, "0");
   SDL_InitSubSystem(SDL_INIT_VIDEO);

   // Setup the vpx app with blank default settings
   Logger::GetInstance()->Init();
   VPApp vpx;
   CommandLineProcessor cmdLine;
   const string iniPath = (GetAssetPath() / "VPinball.ini").string();
   const char* args[] = { "vpx-test.exe", "-ini", iniPath.c_str() };
   cmdLine.ProcessCommandLine(3, args);
   vpx.InitInstance();
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   onPrepareFrameMsgId = msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);

   // Initialize the doctest framework and run the tests
   doctest::Context context;
   context.setOption("no-breaks", true); // Disable breaks when a test fail (including crash & exceptions)
   const string outPath = (GetAssetPath() / "test_results.txt").string();
   context.setOption("out", outPath.c_str());
   context.applyCommandLine(0, nullptr); // TODO Apply command line arguments if any
   int res = context.run();

   // Clean up
   msgApi.ReleaseMsgID(onPrepareFrameMsgId);
   SDL_QuitSubSystem(SDL_INIT_VIDEO);

   if (context.shouldExit())
      return res;
   return 0;
}
