// license:GPLv3+

#include "core/stdafx.h"

class VPApp final : public CWinApp
{
public:
   VPApp(HINSTANCE hInstance);
   ~VPApp() OVERRIDE;
   BOOL InitInstance() override;

   void ProcessCommandLine();
   void ProcessCommandLine(int argc, const char* argv[]);

   int Run() override;

   bool StepMsgLoop();
   int MainMsgLoop();

protected:
   BOOL OnIdle(LONG count) OVERRIDE;

private:
   string GetPathFromArg(const string& arg, bool setCurrentPath);
   static string GetCommandLineHelp();
   static void OnCommandLineError(const string& title, const string& message);

   bool m_run           = true; // Should we run the main win32 UI or Player or just exit ?
   bool m_play          = false;
   bool m_liveedit      = false;
   bool m_extractPov    = false;
   bool m_extractScript = false;
   bool m_audit         = false;
   
#ifdef __STANDALONE__
   bool m_displayId = false;
   bool m_listSnd = false;
   bool m_listRes = false;
   bool m_listCtrl = false;
   bool m_listAll = false;
#endif
   string m_tableFileName;
   string m_tableIniFileName;
   string m_iniFileName;
   string m_tournamentFileName; // if not empty, tournament mode is/can be active

   VPinball m_vpinball;
   int m_idleIndex = 0;
};
