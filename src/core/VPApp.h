// license:GPLv3+

#pragma once

#include "Settings.h"
#include "FileLocator.h"

class AppMsgLoop
{
public:
   virtual ~AppMsgLoop() = default;
   virtual void Initialize() = 0;
   virtual bool StepMsgLoop() = 0;
   virtual int MainMsgLoop() = 0;
};

#ifndef __STANDALONE__

class WinMsgLoop final : public CWinApp, public AppMsgLoop
{
public:
   WinMsgLoop();
   ~WinMsgLoop() override = default;
   void Initialize() override;
   bool StepMsgLoop() override;
   int MainMsgLoop() override;

protected:
   BOOL OnIdle(LONG count) override;
   BOOL PreTranslateMessage(MSG& msg) override;

private:
   int m_idleIndex = 0;
   class VPinball* m_vpxEditor;
};

#else

class StandaloneMsgLoop final : public AppMsgLoop
{
public:
   ~StandaloneMsgLoop() override = default;
   void Initialize() override;
   bool StepMsgLoop() override;
   int MainMsgLoop() override;

private:
   class VPinball* m_vpxEditor;
};

#endif



class VPApp final
{
public:
   VPApp();
   ~VPApp();

   void ProcessCommandLine();
   void ProcessCommandLine(int argc, const char* argv[]);
   void SetSettingsFileName(const string& path) { m_iniFileName = path; } // Must be defined before InitInstance() is called, otherwise it will be ignored
   void InitInstance();

   int Run();

   std::unique_ptr<AppMsgLoop> m_msgLoop;

   // overall app settings
   Settings m_settings;

   // command line parameters
   int m_disEnableTrueFullscreen = -1;
   int m_captureAttract = 0; // Number of frames to capture for attract mode capture, 0 = disabled
   int m_captureAttractFPS = 0;
   bool m_captureAttractLoop = true;
   bool m_open_minimized = false;
   bool m_disable_pause_menu = false;
   bool m_povEdit = false; // table should be run in camera mode to change the POV (and then export that on exit), nothing else
   bool m_table_played_via_command_line = false;
   volatile bool m_table_played_via_SelectTableOnStart = false;
   bool m_bgles = false; // override global emission scale by m_fgles below?
   float m_fgles = 0.f;
   wstring m_customParameters[MAX_CUSTOM_PARAM_INDEX];


   FileLocator m_fileLocator;

   // The Win32 VPX Editor
   VPinball m_vpxEditor;

   int GetLogicalNumberOfProcessors() const;

private:
   string GetPathFromArg(const string& arg, bool setCurrentPath);
   static string GetCommandLineHelp();
   static void OnCommandLineError(const string& title, const string& message);

   // command line parameters
   bool m_run = true; // Should we run the main win32 UI or Player or just exit ?
   bool m_play          = false;
   bool m_liveedit      = false;
   bool m_extractPov    = false;
   bool m_extractScript = false;
   bool m_audit         = false;
   string m_iniFileName;
   string m_tableFileName;
   string m_tableIniFileName;
   string m_tournamentFileName; // if not empty, tournament mode is/can be active

   int m_logicalNumberOfProcessors = -1;
};
