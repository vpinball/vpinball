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

private:
   string GetPathFromArg(const string& arg, bool setCurrentPath);
   string GetCommandLineHelp() const;
   void OnCommandLineError(const string& title, const string& message) const;

   bool m_run = true; // Should we run the interactive mode (UI/Player)
   bool m_play = false; // Should we start the player directly (with the provided file or after asking the user)
   bool m_extractPov = false;
   bool m_file = false;
   bool m_extractScript = false;
   bool m_audit = false;
   bool m_tournament = false;
#ifdef __STANDALONE__
   bool m_displayId = false;
   string m_prefPath;
#endif
   string m_tableFileName;
   string m_tableIniFileName;
   string m_iniFileName;
   string m_tournamentFileName;

   VPinball m_vpinball;
};
