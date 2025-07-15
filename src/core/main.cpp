// license:GPLv3+

// Implementation of WinMain (Windows with UI) or main (Standalone)

#include "core/stdafx.h"

#include "vpversion.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "core/VPApp.h"

#include "ui/resource.h"
#include <initguid.h>

#define SET_CRT_DEBUG_FIELD(a) _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#include <locale>
#include <codecvt>

#ifdef __ANDROID__
#include <SDL3/SDL_main.h>
#endif

#ifdef __STANDALONE__
#include <SDL3_ttf/SDL_ttf.h>
#include <filesystem>
#endif

#ifndef OVERRIDE
#ifndef __STANDALONE__
   #define OVERRIDE override
#else
   #define OVERRIDE
#endif
#endif




#if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
// The OpenGL implementation will fail on NVIDIA drivers when Threaded Optimization is enabled so we disable it for this app
// Note: There are quite a lot of applications doing this, but I think this may hide an incorrect OpenGL call somewhere
// that the threaded optimization of NVIDIA drivers ends up to crash. This would be good to find the root cause, if any.

#include "nvapi/nvapi.h"
#include "nvapi/NvApiDriverSettings.h"
#pragma warning(push)
#pragma warning(disable : 4838)
#include "nvapi/NvApiDriverSettings.c"
#pragma warning(pop)

enum NvThreadOptimization
{
   NV_THREAD_OPTIMIZATION_AUTO = 0,
   NV_THREAD_OPTIMIZATION_ENABLE = 1,
   NV_THREAD_OPTIMIZATION_DISABLE = 2,
   NV_THREAD_OPTIMIZATION_NO_SUPPORT = 3
};

static bool NvAPI_OK_Verify(NvAPI_Status status)
{
   if (status == NVAPI_OK)
      return true;

   NvAPI_ShortString szDesc = { 0 };
   NvAPI_GetErrorMessage(status, szDesc);

   PLOGI << "NVAPI error: " << szDesc;

   return false;
}

static NvThreadOptimization GetNVIDIAThreadOptimization()
{
   NvThreadOptimization threadOptimization = NV_THREAD_OPTIMIZATION_NO_SUPPORT;

   NvAPI_Status status;
   status = NvAPI_Initialize();
   if (!NvAPI_OK_Verify(status))
      return threadOptimization;

   NvDRSSessionHandle hSession;
   status = NvAPI_DRS_CreateSession(&hSession);
   if (!NvAPI_OK_Verify(status))
      return threadOptimization;

   status = NvAPI_DRS_LoadSettings(hSession);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return threadOptimization;
   }

   NvDRSProfileHandle hProfile;
   status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return threadOptimization;
   }

   NVDRS_SETTING originalSetting = {};
   originalSetting.version = NVDRS_SETTING_VER;
   status = NvAPI_DRS_GetSetting(hSession, hProfile, OGL_THREAD_CONTROL_ID, &originalSetting);
   if (NvAPI_OK_Verify(status))
   {
      threadOptimization = (NvThreadOptimization)originalSetting.u32CurrentValue;
   }

   NvAPI_DRS_DestroySession(hSession);

   return threadOptimization;
}

static void SetNVIDIAThreadOptimization(NvThreadOptimization threadedOptimization)
{
   if (threadedOptimization == NV_THREAD_OPTIMIZATION_NO_SUPPORT)
      return;

   NvAPI_Status status;
   status = NvAPI_Initialize();
   if (!NvAPI_OK_Verify(status))
      return;

   NvDRSSessionHandle hSession;
   status = NvAPI_DRS_CreateSession(&hSession);
   if (!NvAPI_OK_Verify(status))
      return;

   status = NvAPI_DRS_LoadSettings(hSession);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return;
   }

   NvDRSProfileHandle hProfile;
   status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return;
   }

   NVDRS_SETTING setting = {};
   setting.version = NVDRS_SETTING_VER;
   setting.settingId = OGL_THREAD_CONTROL_ID;
   setting.settingType = NVDRS_DWORD_TYPE;
   setting.u32CurrentValue = (EValues_OGL_THREAD_CONTROL)threadedOptimization;

   status = NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return;
   }

   status = NvAPI_DRS_SaveSettings(hSession);
   NvAPI_OK_Verify(status);

   NvAPI_DRS_DestroySession(hSession);
}
#endif

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
   #if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
   static NvThreadOptimization s_OriginalNVidiaThreadOptimization = NV_THREAD_OPTIMIZATION_NO_SUPPORT;
   #endif

   Logger::Init();

   int retval;
   try
   {
      #if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
      s_OriginalNVidiaThreadOptimization = GetNVIDIAThreadOptimization();
      if (s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_NO_SUPPORT && s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_DISABLE)
      {
         PLOGI << "Disabling NVIDIA Threaded Optimization";
         SetNVIDIAThreadOptimization(NV_THREAD_OPTIMIZATION_DISABLE);
      }
      #endif
      // Start Win32++
      VPApp theApp(hInstance);
      theApp.ProcessCommandLine();
      theApp.InitInstance();
      MsgPluginManager::GetInstance().ScanPluginFolder(g_pvp->m_myPath + "plugins",
         [](MsgPlugin& plugin)
         {
         const char *enableDisable[] = { "Disabled", "Enabled" };
         int enabled = (int)VPXPluginAPIImpl::GetInstance().getAPI().GetOption(plugin.m_id.c_str(), 
            "Enable", VPX_OPT_SHOW_UI, "Enable plugin", 0.f, 1.f, 1.f, 0.f, VPXPluginAPI::NONE, enableDisable);
         if (enabled)
         {
            plugin.Load(&MsgPluginManager::GetInstance().GetMsgAPI());
         }
         else
         {
            PLOGI << "Plugin " << plugin.m_id << " was found but is disabled (" << plugin.m_library << ')';
         }
      });

      // Run the application
      retval = theApp.Run();
   }

   // catch all CException types
   catch (const CException &e)
   {
      // Display the exception and quit
      MessageBox(nullptr, e.GetText(), AtoT(e.what()), MB_ICONERROR);

      retval = -1;
   }

   MsgPluginManager::GetInstance().UnloadPlugins();

   #ifdef ENABLE_SDL_VIDEO
      SDL_QuitSubSystem(SDL_INIT_VIDEO);
   #endif

   #ifdef __STANDALONE__
      TTF_Quit();
   #endif

   #if defined(ENABLE_OPENGL) && !defined(__STANDALONE__) 
   if (s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_NO_SUPPORT && s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_DISABLE)
   {
      PLOGI << "Restoring NVIDIA Threaded Optimization";
      SetNVIDIAThreadOptimization(s_OriginalNVidiaThreadOptimization);
   }
   #endif

   #if defined(ENABLE_SDL_VIDEO) || defined(ENABLE_SDL_INPUT)
      SDL_Quit();
   #endif

   PLOGI << "Closing VPX...\n\n";
   #if (defined(__STANDALONE__) && (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   exit(retval);
   #endif
   return retval;
}

#ifdef __STANDALONE__
#ifdef __ANDROID__
int main(int argc, char** argv) {
   while(true)
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return 0;
}
#elif ((defined(__APPLE__) && defined(TARGET_OS_TV) && TARGET_OS_TV) || defined(__linux__))
extern int g_argc;
extern char **g_argv;
int main(int argc, char** argv) {
   g_argc = argc;
   g_argv = argv;
   return WinMain(NULL, NULL, NULL, 0);
}
#endif
#endif
