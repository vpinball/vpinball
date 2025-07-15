// license:GPLv3+

#include "core/stdafx.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "vpx-test.h"

#include <filesystem>

#include "core/VPApp.h"

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

string GetAssetPath()
{
   HMODULE hm = nullptr;
   if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, _T("GetAssetPath"), &hm) == 0)
      return ""s;
   TCHAR path[MAX_PATH];
   if (GetModuleFileName(hm, path, MAX_PATH) == 0)
      return ""s;
   std::string fullpath(path);
   fullpath = fullpath.substr(0, fullpath.find_last_of(_T("\\/"))) + _T('\\');
   return fullpath + "test-assets\\";
}

bool CheckMatchingBitmaps(const string& filePath1, const string& filePath2)
{
   auto bmp1 = FileExists(GetAssetPath() + filePath1) ? BaseTexture::CreateFromFile(GetAssetPath() + filePath1) : nullptr;
   auto bmp2 = FileExists(GetAssetPath() + filePath2) ? BaseTexture::CreateFromFile(GetAssetPath() + filePath2) : nullptr;
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
      for (size_t i = 0; i < dataSize; ++i)
      {
         uint8_t dif = data1[i] > data2[i] ? data1[i] - data2[i] : data2[i] - data1[i];
         avg += dif;
         if (dif > max)
            max = dif;
      }
      avg /= bmp1->width() * bmp1->height();
      if ((avg > 2) || (max > 64))
      {
         MESSAGE("Bitmaps do not match: '", filePath1, "' vs '", filePath2, "' avg=", avg, " / max=", (int)max);
         failed = true;
      }
      else if (avg > 0)
      {
         MESSAGE("Bitmaps are similar: '", filePath1, "' vs '", filePath2, "' avg=", avg, " / max=", (int)max);
      }
      if (avg > 0)
      {
         std::shared_ptr<BaseTexture> diff = BaseTexture::Create(bmp1->width(), bmp1->height(), BaseTexture::SRGB);
         uint8_t* const __restrict diffData = diff->data();
         for (size_t i = 0; i < dataSize; ++i)
         {
            uint8_t dif = data1[i] > data2[i] ? data1[i] - data2[i] : data2[i] - data1[i];
            diffData[i] = dif > 64 ? 255 : dif * 4; // Scale to 0-255
         }
         const string ext = extension_from_path(filePath1);
         diff->Save(GetAssetPath() + filePath1.substr(0, filePath1.size() - ext.size() - 1) + "-diff." + ext);
      }
   }
   return !failed;
}

void CaptureRender(const string& tablePath, const string& screenshotPath)
{
   g_pvp->LoadFileName(GetAssetPath() + tablePath, false);
   struct CaptureState
   {
      int frameIndex;
      string tmpScreenshotPath;
      bool done;
   } state = { 0, GetAssetPath() + screenshotPath, false };
   msgpi_msg_callback onPrepareFrame = [](const unsigned int msgId, void* context, void* msgData)
   {
      CaptureState* state = reinterpret_cast<CaptureState*>(context);
      state->frameIndex++;
      if (state->frameIndex == 25)
         g_pplayer->m_renderer->m_renderDevice->CaptureScreenshot(state->tmpScreenshotPath,
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
   g_pvp->DoPlay(0);
   while (!state.done)
      g_app->StepMsgLoop();
   RemoveOnPrepareFrameHandler(onPrepareFrame);
   PostMessage(g_pvp->GetHwnd(), WM_COMMAND, IDM_CLOSE, 0); // Close the table
   while (!g_pvp->m_vtable.empty())
      g_app->StepMsgLoop();
}

void ResetVPX()
{
   // Stop player
   if (g_pplayer)
      g_pplayer->SetCloseState(Player::CS_STOP_PLAY);

   // Close all opened tables
   size_t nOpenedTables = g_pvp->m_vtable.size();
   while (nOpenedTables > 0)
   {
      PostMessage(g_pvp->GetHwnd(), WM_COMMAND, IDM_CLOSE, 0);
      while (g_pvp->m_vtable.size() == nOpenedTables)
         g_app->StepMsgLoop();
      nOpenedTables = g_pvp->m_vtable.size();
   }

   // Reset settings
   g_pvp->m_settings = Settings();
   Settings& settings = g_pvp->m_settings;
   settings.Validate(true);
   settings.SaveValue(Settings::Section::PlayerVR, "AskToTurnOn"s, 2);
   settings.SaveValue(Settings::Section::Player, "PlayfieldFullScreen"s, 0);
   settings.SaveValue(Settings::Section::Player, "PlayfieldWidth"s, 1920);
   settings.SaveValue(Settings::Section::Player, "PlayfieldHeight"s, 1080);
   settings.SaveValue(Settings::Section::Player, "EnableMouseInPlayer"s, false);
   settings.SaveValue(Settings::Section::Player, "NumberOfTimesToShowTouchMessage"s, 0);
   settings.SaveValue(Settings::Section::Player, "DisableAO"s, true);
}


extern "C" int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
   // Initialize the doctest framework
   doctest::Context context;
   context.setOption("no-breaks", true); // Disable breaks in the test output
   context.setOption("out", (GetAssetPath() + "test_results.txt").c_str());
   context.applyCommandLine(0, nullptr); // TODO Apply command line arguments if any

   // Setup the vpx app with blank default settings
   Logger::GetInstance()->Init();
   VPApp vpx(hInstance);
   vpx.InitInstance();
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   onPrepareFrameMsgId = msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);

   // Run the tests
   int res = context.run();

   // Clean up
   msgApi.ReleaseMsgID(onPrepareFrameMsgId);
   PostMessage(g_pvp->GetHwnd(), WM_CLOSE, 0, 0);
   g_app->MainMsgLoop();

   if (context.shouldExit())
      return res;
   return 0;
}
