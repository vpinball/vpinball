// license:GPLv3+

#include "core/stdafx.h"

class VPApp final : public CWinApp
{
public:
   VPApp(HINSTANCE hInstance);
   ~VPApp() OVERRIDE;
   BOOL InitInstance() override;

   void ProcessCommandLine();
   void ProcessCommandLine(int argc, char* argv[]);

   int Run() override;

   bool StepMsgLoop();
   int MainMsgLoop();

protected:
   BOOL OnIdle(LONG count) OVERRIDE;

private:
   string GetPathFromArg(const string& arg, bool setCurrentPath);
   static string GetCommandLineHelp();
   static void OnCommandLineError(const string& title, const string& message);

   bool m_run = true; // Should we run the interactive mode (UI/Player)
   bool m_play = false; // Should we start the player directly (with the provided file or after asking the user)
   bool m_extractPov = false;
   bool m_file = false;
   bool m_extractScript = false;
   bool m_audit = false;
   bool m_tournament = false;
   bool m_listSnd = false;    // Flag for -listsnd option to defer sound device enumeration
   bool m_listRes = false;    // Flag for -listres option to defer display resolution enumeration
#ifdef __STANDALONE__
   bool m_displayId = false;
   string m_prefPath;
#endif
   string m_tableFileName;
   string m_tableIniFileName;
   string m_iniFileName;
   string m_tournamentFileName;

   VPinball m_vpinball;
   int m_idleIndex = 0;
};
